#include <gtest/gtest.h>

#include <Ebenezer/BotCommandFacade.h>
#include <Ebenezer/BotManager.h>
#include <Ebenezer/BotMovement.h>
#include <Ebenezer/BotUser.h>
#include <Ebenezer/EbenezerApp.h>

#include <shared/StringUtils.h>
#include <shared/packets.h>
#include <shared-server/utilities.h>

#include "TestApp.h"

#include <chrono>
#include <condition_variable>
#include <cmath>
#include <cstdlib>
#include <future>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <vector>

using namespace Ebenezer;
using namespace std::chrono_literals;

namespace
{

struct BlockingTimerState
{
	std::mutex mutex;
	std::condition_variable cv;
	bool shutdownEntered = false;
	bool allowShutdown = false;
	size_t factoryCalls = 0;
	size_t starts = 0;
	size_t callbackCalls = 0;
	std::function<void()> callback;
};

class BlockingBotTimer final : public IBotTimer
{
public:
	explicit BlockingBotTimer(std::shared_ptr<BlockingTimerState> state) : _state(std::move(state)) {}

	void Start() override
	{
		std::lock_guard lock(_state->mutex);
		++_state->starts;
	}

	void Shutdown() override
	{
		std::unique_lock lock(_state->mutex);
		_state->shutdownEntered = true;
		_state->cv.notify_all();
		_state->cv.wait(lock, [this]() { return _state->allowShutdown; });
	}

private:
	std::shared_ptr<BlockingTimerState> _state;
};

class ThrowingShutdownBotTimer final : public IBotTimer
{
public:
	void Start() override {}
	void Shutdown() override
	{
		throw std::runtime_error("timer shutdown failure");
	}
};

class BotManagerTest : public testing::Test
{
protected:
	void SetUp() override
	{
		_originalRandom = myrand;
		myrand          = [](int min, int) { return min; };

		_app            = std::make_unique<TestApp>();
		_map            = _app->CreateMap(ZONE_FRONTIER, 256);
		ASSERT_NE(_map, nullptr);
		ASSERT_EQ(_app->GetBotManager().Status().total, 0u);
	}

	void TearDown() override
	{
		if (_app != nullptr)
		{
			_app->GetBotManager().Stop();
			_app->GetBotManager().RemoveAll();
		}
		_app.reset();
		myrand = _originalRandom;
	}

	BotSpawnRequest Request(
		std::string name, uint8_t nation, float x = 120.0f, float z = 120.0f) const
	{
		BotSpawnRequest request;
		request.name           = std::move(name);
		request.nation         = nation;
		request.characterClass = nation == NATION_KARUS ? CLASS_KA_WARRIOR : CLASS_EL_WARRIOR;
		request.level          = 60;
		request.spawn          = { ZONE_FRONTIER, x, 0.0f, z };
		return request;
	}

	std::shared_ptr<CBotUser> Spawn(
		std::string name, uint8_t nation, float x = 120.0f, float z = 120.0f)
	{
		const int id = _app->GetBotManager().Spawn(Request(std::move(name), nation, x, z));
		if (id < 0)
			return nullptr;
		return std::dynamic_pointer_cast<CBotUser>(_app->GetBotManager().FindUser(id));
	}

	bool RegionContains(int regionX, int regionZ, int userId) const
	{
		return _map->m_ppRegion[regionX][regionZ].m_RegionUserArray.GetData(userId) != nullptr;
	}

	size_t RegionOccurrenceCount(int userId) const
	{
		size_t count = 0;
		for (int x = 0; x <= _map->GetXRegionMax(); ++x)
		{
			for (int z = 0; z <= _map->GetZRegionMax(); ++z)
			{
				if (RegionContains(x, z, userId))
					++count;
			}
		}
		return count;
	}

	std::function<int(int, int)> _originalRandom;
	std::unique_ptr<TestApp> _app;
	TestMap* _map = nullptr;
	const std::chrono::steady_clock::time_point _now { 10s };
};

TEST_F(BotManagerTest, SpawnRegistersBotAndAddsItsIdToMapRegion)
{
	auto bot = Spawn("Bot_K_000", NATION_KARUS);
	ASSERT_NE(bot, nullptr);

	EXPECT_EQ(bot->GetSocketID(), BOT_USER_ID_MIN);
	EXPECT_EQ(_app->GetBotManager().FindUser(bot->GetSocketID()).get(), bot.get());
	EXPECT_TRUE(RegionContains(bot->m_RegionX, bot->m_RegionZ, bot->GetSocketID()));
	EXPECT_EQ(RegionOccurrenceCount(bot->GetSocketID()), 1u);
	EXPECT_EQ(_app->GetBotManager().Status().total, 1u);
}

TEST_F(BotManagerTest, SpawnBroadcastFailureRollsBackRegistryAndRegion)
{
	auto observer = _app->AddUser();
	ASSERT_NE(observer, nullptr);
	strcpy_safe(observer->m_pUserData->m_id, "Observer");
	observer->m_pUserData->m_bNation = NATION_ELMORAD;
	observer->m_pUserData->m_bZone   = ZONE_FRONTIER;
	observer->m_pUserData->m_curx = observer->m_fWill_x = 120.0f;
	observer->m_pUserData->m_curz = observer->m_fWill_z = 120.0f;
	observer->SetState(CONNECTION_STATE_GAMESTART);
	ASSERT_TRUE(_map->Add(observer.get(), 2, 2));
	observer->AddSendCallback([](const char*, int) { throw UnhandledSendCallbackException(); });

	EXPECT_THROW(_app->GetBotManager().Spawn(Request("Bot_K_013", NATION_KARUS)),
		UnhandledSendCallbackException);
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
	EXPECT_EQ(RegionOccurrenceCount(BOT_USER_ID_MIN), 0u);
	observer->SetState(CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotManagerTest, PatrolSkipsInvalidHomeOffsetsAndNeverExceedsMaximumStep)
{
	_map = _app->CreateMap(ZONE_BATTLE, 128);
	ASSERT_NE(_map, nullptr);
	const int id = _app->GetBotManager().Spawn(BotSpawnRequest {
		"Bot_K_001", NATION_KARUS, CLASS_KA_WARRIOR, 60, { ZONE_BATTLE, 120.0f, 0.0f, 120.0f } });
	auto bot     = std::dynamic_pointer_cast<CBotUser>(_app->GetBotManager().FindUser(id));
	ASSERT_NE(bot, nullptr);

	_app->GetBotManager().Tick(_now);
	_app->GetBotManager().Tick(_now + 200ms);

	const float step = std::hypot(bot->m_fWill_x - 120.0f, bot->m_fWill_z - 120.0f);
	EXPECT_GT(step, 0.0f);
	EXPECT_LE(step, 1.5f);
	EXPECT_GE(bot->m_fWill_x, 0.0f);
	EXPECT_GE(bot->m_fWill_z, 0.0f);
	EXPECT_TRUE(_map->IsValidPosition(bot->m_fWill_x, bot->m_fWill_z));
	EXPECT_LT(bot->m_fWill_x, 120.0f);
}

TEST_F(BotManagerTest, NextStepEnforcesServerMaximumWhenCallerRequestsMore)
{
	auto bot = Spawn("Bot_K_012", NATION_KARUS, 120.0f, 120.0f);
	ASSERT_NE(bot, nullptr);

	const auto destination = BotMovement::NextStep(*bot, 220.0f, 120.0f, 99.0f);
	EXPECT_LE(std::hypot(destination.x - bot->m_pUserData->m_curx,
				  destination.z - bot->m_pUserData->m_curz),
		1.5f);
}

TEST_F(BotManagerTest, FarEnemyCausesMovementThatDecreasesTrueDistance)
{
	auto source = Spawn("Bot_K_002", NATION_KARUS, 120.0f, 120.0f);
	auto enemy  = Spawn("Bot_E_002", NATION_ELMORAD, 130.0f, 120.0f);
	ASSERT_NE(source, nullptr);
	ASSERT_NE(enemy, nullptr);
	const float initialDistance = source->GetDistance2D(
		enemy->m_pUserData->m_curx, enemy->m_pUserData->m_curz);

	_app->GetBotManager().Tick(_now);
	_app->GetBotManager().Tick(_now + 200ms);
	_app->GetBotManager().Tick(_now + 400ms);

	EXPECT_LT(source->GetDistance2D(enemy->m_pUserData->m_curx, enemy->m_pUserData->m_curz),
		initialDistance);
	EXPECT_LE(std::hypot(source->m_fWill_x - source->m_pUserData->m_curx,
				  source->m_fWill_z - source->m_pUserData->m_curz),
		1.5f);
}

TEST_F(BotManagerTest, InRangeEnemyLosesHpThroughLegacyAttack)
{
	auto source = Spawn("Bot_K_003", NATION_KARUS, 120.0f, 120.0f);
	auto enemy  = Spawn("Bot_E_003", NATION_ELMORAD, 122.0f, 120.0f);
	ASSERT_NE(source, nullptr);
	ASSERT_NE(enemy, nullptr);
	source->m_fTotalHitRate = 10.0f;
	const int initialHp     = enemy->m_pUserData->m_sHp;

	_app->GetBotManager().Tick(_now);
	_app->GetBotManager().Tick(_now + 200ms);

	EXPECT_LT(enemy->m_pUserData->m_sHp, initialHp);
}

TEST_F(BotManagerTest, BasicAttackUsesTrueDistanceAndOneSecondCooldown)
{
	auto source    = Spawn("Bot_K_004", NATION_KARUS, 120.0f, 120.0f);
	auto nearEnemy = Spawn("Bot_E_004", NATION_ELMORAD, 122.0f, 120.0f);
	auto farEnemy  = Spawn("Bot_E_005", NATION_ELMORAD, 123.0f, 120.0f);
	ASSERT_NE(source, nullptr);
	ASSERT_NE(nearEnemy, nullptr);
	ASSERT_NE(farEnemy, nullptr);
	source->m_fTotalHitRate = 10.0f;
	BotCommandFacade commands(*_app);

	const int farHp = farEnemy->m_pUserData->m_sHp;
	EXPECT_FALSE(commands.BasicAttack(*source, farEnemy->GetSocketID(), _now));
	EXPECT_EQ(farEnemy->m_pUserData->m_sHp, farHp);

	EXPECT_TRUE(commands.BasicAttack(*source, nearEnemy->GetSocketID(), _now));
	const int afterFirstAttack = nearEnemy->m_pUserData->m_sHp;
	EXPECT_LT(afterFirstAttack, nearEnemy->m_iMaxHp);
	EXPECT_FALSE(commands.BasicAttack(*source, nearEnemy->GetSocketID(), _now + 999ms));
	EXPECT_EQ(nearEnemy->m_pUserData->m_sHp, afterFirstAttack);
	EXPECT_TRUE(commands.BasicAttack(*source, nearEnemy->GetSocketID(), _now + 1s));
	EXPECT_LT(nearEnemy->m_pUserData->m_sHp, afterFirstAttack);
}

TEST_F(BotManagerTest, BasicAttackRejectsInvalidCombatStatesWithoutConsumingCooldown)
{
	auto source = Spawn("Bot_K_State", NATION_KARUS, 120.0f, 120.0f);
	auto target = Spawn("Bot_E_State", NATION_ELMORAD, 122.0f, 120.0f);
	ASSERT_NE(source, nullptr);
	ASSERT_NE(target, nullptr);
	BotCommandFacade commands(*_app);
	const auto originalCooldown = _now - 1s;

	struct RejectionCase
	{
		const char* name;
		std::function<void()> arrange;
	};
	const std::vector<RejectionCase> cases {
		{ "source zone", [&]() { source->m_pUserData->m_bZone = ZONE_BATTLE; } },
		{ "same nation", [&]() { target->m_pUserData->m_bNation = NATION_KARUS; } },
		{ "source state", [&]() { source->SetState(CONNECTION_STATE_CONNECTED); } },
		{ "target state", [&]() { target->SetState(CONNECTION_STATE_CONNECTED); } },
		{ "source hp", [&]() { source->m_pUserData->m_sHp = 0; } },
		{ "target hp", [&]() { target->m_pUserData->m_sHp = 0; } },
		{ "source blink", [&]() { source->m_bAbnormalType = ABNORMAL_BLINKING; } },
		{ "target blink", [&]() { target->m_bAbnormalType = ABNORMAL_BLINKING; } },
	};

	for (const auto& testCase : cases)
	{
		source->m_pUserData->m_bZone = target->m_pUserData->m_bZone = ZONE_FRONTIER;
		source->m_pUserData->m_bNation = NATION_KARUS;
		target->m_pUserData->m_bNation = NATION_ELMORAD;
		source->SetState(CONNECTION_STATE_GAMESTART);
		target->SetState(CONNECTION_STATE_GAMESTART);
		source->m_pUserData->m_sHp = source->m_iMaxHp;
		target->m_pUserData->m_sHp = target->m_iMaxHp;
		source->m_bResHpType = target->m_bResHpType = USER_STANDING;
		source->m_bAbnormalType = target->m_bAbnormalType = ABNORMAL_NORMAL;
		source->Runtime().nextAttackAt = originalCooldown;
		testCase.arrange();

		EXPECT_FALSE(commands.BasicAttack(*source, target->GetSocketID(), _now)) << testCase.name;
		EXPECT_EQ(source->Runtime().nextAttackAt, originalCooldown) << testCase.name;
	}
}

TEST_F(BotManagerTest, ConsecutiveMovementStepsAdvanceFromPendingEndpoint)
{
	auto bot = Spawn("Bot_K_Move", NATION_KARUS, 120.0f, 120.0f);
	ASSERT_NE(bot, nullptr);

	const auto first = BotMovement::NextStep(*bot, 130.0f, 120.0f, 1.5f);
	ASSERT_TRUE(BotMovement::Move(*bot, first, 45));
	const auto second = BotMovement::NextStep(*bot, 130.0f, 120.0f, 1.5f);
	ASSERT_TRUE(BotMovement::Move(*bot, second, 45));

	EXPECT_FLOAT_EQ(first.x, 121.5f);
	EXPECT_FLOAT_EQ(second.x, 123.0f);
	EXPECT_FLOAT_EQ(bot->m_pUserData->m_curx, first.x);
	EXPECT_FLOAT_EQ(bot->m_fWill_x, second.x);
	EXPECT_LE(std::hypot(second.x - first.x, second.z - first.z), 1.5f);
}

TEST_F(BotManagerTest, MoveRejectsInvalidCoordinatesAndSerializesValidatedPacket)
{
	auto observer = _app->AddUser();
	ASSERT_NE(observer, nullptr);
	observer->m_pUserData->m_bZone = ZONE_FRONTIER;
	observer->SetState(CONNECTION_STATE_GAMESTART);
	ASSERT_TRUE(_map->Add(observer.get(), 2, 2));
	observer->AddSendCallback([](const char*, int) {});
	auto bot = Spawn("Bot_K_Packet", NATION_KARUS);
	ASSERT_NE(bot, nullptr);

	const float originalX = bot->m_fWill_x;
	const float originalZ = bot->m_fWill_z;
	const float nan = std::numeric_limits<float>::quiet_NaN();
	const float inf = std::numeric_limits<float>::infinity();
	EXPECT_FALSE(BotMovement::Move(*bot, { ZONE_FRONTIER, nan, 0.0f, 120.0f }, 45));
	EXPECT_FALSE(BotMovement::Move(*bot, { ZONE_FRONTIER, 120.0f, 0.0f, inf }, 45));
	EXPECT_FALSE(BotMovement::Move(*bot, { ZONE_FRONTIER, 7000.0f, 0.0f, 120.0f }, 45));
	EXPECT_FALSE(BotMovement::Move(*bot, { ZONE_BATTLE, 120.0f, 0.0f, 120.0f }, 45));
	EXPECT_FLOAT_EQ(bot->m_fWill_x, originalX);
	EXPECT_FLOAT_EQ(bot->m_fWill_z, originalZ);

	EXPECT_TRUE(BotMovement::Move(*bot, { ZONE_FRONTIER, 123.5f, -2.0f, 125.5f }, 45));
	char regionPacket[1024] {};
	const int regionLength = observer->RegionPacketClear(regionPacket);
	ASSERT_GT(regionLength, 0);
	int index = 0;
	EXPECT_EQ(GetByte(regionPacket, index), WIZ_CONTINOUS_PACKET);
	EXPECT_GT(GetShort(regionPacket, index), 0);
	EXPECT_EQ(GetShort(regionPacket, index), 12);
	EXPECT_EQ(GetByte(regionPacket, index), WIZ_MOVE);
	EXPECT_EQ(GetShort(regionPacket, index), bot->GetSocketID());
	EXPECT_EQ(GetShort(regionPacket, index), 1235);
	EXPECT_EQ(GetShort(regionPacket, index), 1255);
	EXPECT_EQ(static_cast<int16_t>(GetShort(regionPacket, index)), -20);
	EXPECT_EQ(GetShort(regionPacket, index), 45);
	EXPECT_EQ(GetByte(regionPacket, index), 0);
	observer->SetState(CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotManagerTest, ZeroHpMarksDeadOnceAndSetsExactRespawnDeadline)
{
	auto bot = Spawn("Bot_K_006", NATION_KARUS);
	ASSERT_NE(bot, nullptr);
	bot->m_pUserData->m_sHp = 0;

	_app->GetBotManager().Tick(_now);
	EXPECT_EQ(bot->Runtime().state, BotState::Dead);
	EXPECT_EQ(bot->Runtime().respawnAt, _now + 15s);

	_app->GetBotManager().Tick(_now + 5s);
	EXPECT_EQ(bot->Runtime().respawnAt, _now + 15s);
}

TEST_F(BotManagerTest, RespawnsOnlyAtExactBoundaryAtHomeWithCleanRuntimeAndRegions)
{
	auto bot = Spawn("Bot_K_007", NATION_KARUS, 120.0f, 120.0f);
	ASSERT_NE(bot, nullptr);
	const int id             = bot->GetSocketID();
	bot->m_pUserData->m_sHp  = 0;
	bot->m_pUserData->m_sMp  = 0;
	bot->m_pUserData->m_curx = bot->m_fWill_x = 180.0f;
	bot->m_pUserData->m_curz = bot->m_fWill_z = 180.0f;
	bot->m_RegionX                            = 3;
	bot->m_RegionZ                            = 3;
	ASSERT_TRUE(_map->RegionUserAdd(3, 3, id));
	ASSERT_TRUE(_map->RegionUserAdd(4, 4, id));
	bot->Runtime().targetId     = 123;
	bot->Runtime().nextAttackAt = _now + 20s;

	_app->GetBotManager().Tick(_now);
	_app->GetBotManager().Tick(_now + 14999ms);
	EXPECT_EQ(bot->Runtime().state, BotState::Dead);
	EXPECT_EQ(bot->m_pUserData->m_sHp, 0);

	_app->GetBotManager().Tick(_now + 15s);
	EXPECT_EQ(bot->Runtime().state, BotState::SelectTarget);
	EXPECT_FLOAT_EQ(bot->m_pUserData->m_curx, 120.0f);
	EXPECT_FLOAT_EQ(bot->m_pUserData->m_curz, 120.0f);
	EXPECT_FLOAT_EQ(bot->m_fWill_x, 120.0f);
	EXPECT_FLOAT_EQ(bot->m_fWill_z, 120.0f);
	EXPECT_EQ(bot->m_pUserData->m_sHp, bot->m_iMaxHp);
	EXPECT_EQ(bot->m_pUserData->m_sMp, bot->m_iMaxMp);
	EXPECT_EQ(bot->m_bResHpType, USER_STANDING);
	EXPECT_EQ(bot->m_bAbnormalType, ABNORMAL_NORMAL);
	EXPECT_EQ(bot->Runtime().targetId, -1);
	EXPECT_EQ(bot->Runtime().nextAttackAt, std::chrono::steady_clock::time_point {});
	EXPECT_EQ(bot->Runtime().respawnAt, std::chrono::steady_clock::time_point {});
	EXPECT_EQ(RegionOccurrenceCount(id), 1u);
	EXPECT_TRUE(RegionContains(bot->m_RegionX, bot->m_RegionZ, id));
}

TEST_F(BotManagerTest, RespawnBroadcastsUserOutBeforeUserRegene)
{
	auto observer = _app->AddUser();
	ASSERT_NE(observer, nullptr);
	observer->m_pUserData->m_bZone = ZONE_FRONTIER;
	observer->SetState(CONNECTION_STATE_GAMESTART);
	ASSERT_TRUE(_map->Add(observer.get(), 2, 2));
	observer->AddSendCallback([](const char*, int) {});
	auto bot = Spawn("Bot_K_Order", NATION_KARUS);
	ASSERT_NE(bot, nullptr);

	std::vector<uint8_t> eventTypes;
	for (int i = 0; i < 2; ++i)
	{
		observer->AddSendCallback([&](const char* packet, int length)
		{
			ASSERT_GE(length, 4);
			int index = 0;
			EXPECT_EQ(GetByte(packet, index), WIZ_USER_INOUT);
			eventTypes.push_back(GetByte(packet, index));
			EXPECT_EQ(GetShort(packet, index), bot->GetSocketID());
		});
	}
	bot->m_pUserData->m_sHp = 0;
	BotCommandFacade commands(*_app);
	ASSERT_TRUE(commands.Respawn(*bot));
	ASSERT_EQ(eventTypes.size(), 2u);
	EXPECT_EQ(eventTypes[0], USER_OUT);
	EXPECT_EQ(eventTypes[1], USER_REGENE);
	observer->SetState(CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotManagerTest, RespawnRejectsInvalidHomeWithoutRemovingLiveRegionEntry)
{
	auto observer = _app->AddUser();
	ASSERT_NE(observer, nullptr);
	observer->m_pUserData->m_bZone = ZONE_FRONTIER;
	observer->SetState(CONNECTION_STATE_GAMESTART);
	ASSERT_TRUE(_map->Add(observer.get(), 2, 2));
	observer->AddSendCallback([](const char*, int) {});
	auto bot = Spawn("Bot_K_Home", NATION_KARUS);
	ASSERT_NE(bot, nullptr);
	const int botId = bot->GetSocketID();
	bool broadcast = false;
	observer->AddSendCallback([&](const char*, int) { broadcast = true; });
	bot->Runtime().home.x = std::numeric_limits<float>::quiet_NaN();
	BotCommandFacade commands(*_app);

	EXPECT_FALSE(commands.Respawn(*bot));
	EXPECT_FALSE(broadcast);
	EXPECT_EQ(_app->GetBotRegistry().Get(botId).get(), bot.get());
	EXPECT_EQ(RegionOccurrenceCount(botId), 1u);
	observer->SetState(CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotManagerTest, RemoveAllEmitsUserOutBeforeClearingRegistryAndRegions)
{
	auto observer = _app->AddUser();
	ASSERT_NE(observer, nullptr);
	strcpy_safe(observer->m_pUserData->m_id, "Observer");
	observer->m_pUserData->m_bNation = NATION_ELMORAD;
	observer->m_pUserData->m_bZone   = ZONE_FRONTIER;
	observer->m_pUserData->m_curx = observer->m_fWill_x = 120.0f;
	observer->m_pUserData->m_curz = observer->m_fWill_z = 120.0f;
	observer->SetState(CONNECTION_STATE_GAMESTART);
	ASSERT_TRUE(_map->Add(observer.get(), 2, 2));
	observer->AddSendCallback([](const char*, int) {});
	auto bot = Spawn("Bot_K_008", NATION_KARUS);
	ASSERT_NE(bot, nullptr);
	const int botId = bot->GetSocketID();
	bool sawUserOut = false;
	observer->AddSendCallback(
		[&](const char* packet, int length)
		{
			ASSERT_GE(length, 4);
			int index = 0;
			EXPECT_EQ(GetByte(packet, index), WIZ_USER_INOUT);
			EXPECT_EQ(GetByte(packet, index), USER_OUT);
			EXPECT_EQ(GetShort(packet, index), botId);
			sawUserOut = true;
		});

	EXPECT_EQ(_app->GetBotManager().RemoveAll(), 1u);
	EXPECT_TRUE(sawUserOut);
	EXPECT_EQ(_app->GetBotManager().Status().total, 0u);
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
	EXPECT_EQ(RegionOccurrenceCount(botId), 0u);
	observer->SetState(CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotManagerTest, TickSerializesRemoveAllAndStatusAcrossRegionCallbacks)
{
	auto observer = _app->AddUser();
	ASSERT_NE(observer, nullptr);
	observer->m_pUserData->m_bNation = NATION_ELMORAD;
	observer->m_pUserData->m_bZone = ZONE_FRONTIER;
	observer->m_pUserData->m_curx = observer->m_fWill_x = 122.0f;
	observer->m_pUserData->m_curz = observer->m_fWill_z = 120.0f;
	observer->m_iMaxHp = observer->m_pUserData->m_sHp = 1500;
	observer->m_bResHpType = USER_STANDING;
	observer->m_fTotalEvasionRate = 1.0f;
	observer->SetState(CONNECTION_STATE_GAMESTART);
	ASSERT_TRUE(_map->Add(observer.get(), 2, 2));
	observer->AddSendCallback([](const char*, int) {});
	auto bot = Spawn("Bot_K_Serial", NATION_KARUS);
	ASSERT_NE(bot, nullptr);
	bot->m_fTotalHitRate = 10.0f;
	const int botId = bot->GetSocketID();
	bot->Runtime().targetId = observer->GetSocketID();

	std::mutex barrierMutex;
	std::condition_variable barrierCv;
	bool callbackEntered = false;
	bool releaseCallback = false;
	observer->AddSendCallback([&](const char*, int)
	{
		std::unique_lock lock(barrierMutex);
		callbackEntered = true;
		barrierCv.notify_all();
		barrierCv.wait(lock, [&]() { return releaseCallback; });
	});

	auto tick = std::async(std::launch::async, [&]() { _app->GetBotManager().Tick(_now); });
	{
		std::unique_lock lock(barrierMutex);
		ASSERT_TRUE(barrierCv.wait_for(lock, 2s, [&]() { return callbackEntered; }));
	}
	auto remove = std::async(std::launch::async, [&]() { return _app->GetBotManager().RemoveAll(); });
	auto status = std::async(std::launch::async, [&]() { return _app->GetBotManager().Status(); });
	EXPECT_EQ(remove.wait_for(100ms), std::future_status::timeout);
	EXPECT_EQ(status.wait_for(100ms), std::future_status::timeout);
	{
		std::lock_guard lock(barrierMutex);
		releaseCallback = true;
	}
	barrierCv.notify_all();
	EXPECT_EQ(tick.wait_for(2s), std::future_status::ready);
	EXPECT_EQ(remove.wait_for(2s), std::future_status::ready);
	EXPECT_EQ(status.wait_for(2s), std::future_status::ready);
	EXPECT_EQ(remove.get(), 1u);
	tick.get();
	(void) status.get();
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
	EXPECT_EQ(RegionOccurrenceCount(botId), 0u);
	observer->SetState(CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotManagerTest, RegistryFullSpawnLeavesExistingEntriesAndRegionsUnchanged)
{
	for (int i = 0; i < MAX_BOT_USER; ++i)
	{
		auto entry = std::make_shared<TestUser>();
		ASSERT_GE(_app->GetBotRegistry().Register(entry), 0);
	}
	ASSERT_EQ(_app->GetBotRegistry().Size(), static_cast<size_t>(MAX_BOT_USER));

	EXPECT_EQ(_app->GetBotManager().Spawn(Request("Bot_K_Full", NATION_KARUS)), -1);
	EXPECT_EQ(_app->GetBotRegistry().Size(), static_cast<size_t>(MAX_BOT_USER));
	EXPECT_EQ(RegionOccurrenceCount(BOT_USER_ID_MIN), 0u);
}

TEST_F(BotManagerTest, RemoveAllPurgesAndUnregistersWhenUserOutBroadcastThrows)
{
	auto observer = _app->AddUser();
	ASSERT_NE(observer, nullptr);
	observer->m_pUserData->m_bZone = ZONE_FRONTIER;
	observer->SetState(CONNECTION_STATE_GAMESTART);
	ASSERT_TRUE(_map->Add(observer.get(), 2, 2));
	observer->AddSendCallback([](const char*, int) {});
	auto bot = Spawn("Bot_K_Cleanup", NATION_KARUS);
	ASSERT_NE(bot, nullptr);
	const int botId = bot->GetSocketID();
	observer->AddSendCallback([](const char*, int) { throw UnhandledSendCallbackException(); });

	EXPECT_EQ(_app->GetBotManager().RemoveAll(), 1u);
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
	EXPECT_EQ(RegionOccurrenceCount(botId), 0u);
	observer->SetState(CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotManagerTest, InvalidRegistryEntryIsRemovedWithoutPreventingAnotherBotTick)
{
	auto invalid = std::make_shared<TestUser>();
	ASSERT_GE(_app->GetBotRegistry().Register(invalid), 0);
	auto bot = Spawn("Bot_K_009", NATION_KARUS);
	ASSERT_NE(bot, nullptr);

	_app->GetBotManager().Tick(_now);

	EXPECT_EQ(_app->GetBotRegistry().Get(invalid->GetSocketID()), nullptr);
	EXPECT_EQ(bot->Runtime().state, BotState::SelectTarget);
	EXPECT_EQ(_app->GetBotRegistry().Size(), 1u);
}

TEST_F(BotManagerTest, ExceptionInOneBotDespawnsItAndAnotherBotStillTicks)
{
	auto invalid = Spawn("Bot_K_010", NATION_KARUS);
	auto valid   = Spawn("Bot_K_011", NATION_KARUS, 140.0f, 140.0f);
	ASSERT_NE(invalid, nullptr);
	ASSERT_NE(valid, nullptr);
	const int invalidId  = invalid->GetSocketID();
	invalid->m_pUserData = nullptr;

	_app->GetBotManager().Tick(_now);

	EXPECT_EQ(_app->GetBotRegistry().Get(invalidId), nullptr);
	EXPECT_EQ(valid->Runtime().state, BotState::SelectTarget);
	EXPECT_EQ(_app->GetBotRegistry().Size(), 1u);
}

TEST_F(BotManagerTest, StartAndStopAreIdempotentAndDoNotAutoSpawn)
{
	auto& manager = _app->GetBotManager();
	EXPECT_EQ(manager.Status().total, 0u);
	EXPECT_FALSE(manager.Status().running);

	manager.StartPk();
	manager.StartPk();
	EXPECT_TRUE(manager.Status().running);

	manager.Stop();
	manager.Stop();
	EXPECT_FALSE(manager.Status().running);
	EXPECT_EQ(manager.Status().total, 0u);
}

TEST_F(BotManagerTest, StartPkCannotRestartWhileStopIsJoiningTimer)
{
	auto state = std::make_shared<BlockingTimerState>();
	BotTimerFactory factory = [state](std::chrono::milliseconds, std::function<void()> callback)
	{
		std::lock_guard lock(state->mutex);
		++state->factoryCalls;
		state->callback = [state, callback = std::move(callback)]()
		{
			{
				std::lock_guard callbackLock(state->mutex);
				++state->callbackCalls;
			}
			callback();
		};
		return std::make_unique<BlockingBotTimer>(state);
	};
	BotManager manager(*_app, std::move(factory));
	manager.StartPk();
	std::function<void()> firstCallback;
	{
		std::lock_guard lock(state->mutex);
		firstCallback = state->callback;
	}
	ASSERT_TRUE(firstCallback);
	firstCallback();
	auto stop = std::async(std::launch::async, [&]() { manager.Stop(); });
	{
		std::unique_lock lock(state->mutex);
		ASSERT_TRUE(state->cv.wait_for(lock, 2s, [&]() { return state->shutdownEntered; }));
	}
	manager.StartPk();
	{
		std::lock_guard lock(state->mutex);
		EXPECT_EQ(state->factoryCalls, 1u);
		EXPECT_EQ(state->starts, 1u);
		EXPECT_EQ(state->callbackCalls, 1u);
		state->allowShutdown = true;
	}
	state->cv.notify_all();
	EXPECT_EQ(stop.wait_for(2s), std::future_status::ready);
	stop.get();
	EXPECT_FALSE(manager.Status().running);
	manager.StartPk();
	EXPECT_EQ(state->factoryCalls, 1u);
}

TEST_F(BotManagerTest, StopPublishesTerminalStateWhenTimerShutdownThrows)
{
	BotManager manager(*_app,
		[](std::chrono::milliseconds, std::function<void()>)
		{ return std::make_unique<ThrowingShutdownBotTimer>(); });
	manager.StartPk();
	EXPECT_NO_THROW(manager.Stop());
	EXPECT_FALSE(manager.Status().running);
	EXPECT_NO_THROW(manager.Stop());
}

} // namespace
