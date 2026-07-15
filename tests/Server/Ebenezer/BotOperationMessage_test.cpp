#include <gtest/gtest.h>

#include <Ebenezer/BotManager.h>
#include <Ebenezer/BotTypes.h>
#include <Ebenezer/OperationMessage.h>

#include <shared/Ini.h>
#include <shared/StringUtils.h>
#include <shared/packets.h>
#include <shared-server/utilities.h>

#include <boost/interprocess/ipc/message_queue.hpp>
#include "TestApp.h"

#include <spdlog/sinks/ostream_sink.h>

#include <algorithm>
#include <barrier>
#include <filesystem>
#include <future>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace Ebenezer;

namespace
{
class ScopedMessageQueue
{
public:
	explicit ScopedMessageQueue(std::string name) : _name(std::move(name))
	{
		boost::interprocess::message_queue::remove(_name.c_str());
	}

	~ScopedMessageQueue()
	{
		boost::interprocess::message_queue::remove(_name.c_str());
	}

	const char* Name() const { return _name.c_str(); }

private:
	std::string _name;
};

std::vector<char> NewCharacterRequest(std::string_view name)
{
	std::vector<char> packet(128);
	int index = 0;
	SetByte(packet.data(), 0, index);
	SetShort(packet.data(), static_cast<int16_t>(name.size()), index);
	SetString(packet.data(), name.data(), static_cast<int>(name.size()), index);
	SetByte(packet.data(), 1, index);
	SetShort(packet.data(), CLASS_KA_WARRIOR, index);
	SetByte(packet.data(), 1, index);
	SetByte(packet.data(), 1, index);
	for (int i = 0; i < 5; ++i)
		SetByte(packet.data(), 50, index);
	packet.resize(index);
	return packet;
}

std::vector<char> SelectCharacterRequest(
	std::string_view account, std::string_view character, uint8_t zone)
{
	std::vector<char> packet(128);
	int index = 0;
	SetString2(packet.data(), account.data(), static_cast<int>(account.size()), index);
	SetString2(packet.data(), character.data(), static_cast<int>(character.size()), index);
	SetByte(packet.data(), 1, index);
	SetByte(packet.data(), zone, index);
	packet.resize(index);
	return packet;
}

class BotOperationMessageTest : public testing::Test
{
protected:
	void SetUp() override
	{
		_app = std::make_unique<TestApp>();
		ASSERT_NE(_app->CreateMap(ZONE_FRONTIER, 256), nullptr);
		ASSERT_TRUE(_app->AddHomeEntry(NATION_KARUS, 120, 120));
		ASSERT_TRUE(_app->AddHomeEntry(NATION_ELMORAD, 140, 140));
		_gm = _app->AddUser();
		ASSERT_NE(_gm, nullptr);
		_gm->m_pUserData->m_bAuthority = AUTHORITY_MANAGER;
		strcpy_safe(_gm->m_pUserData->m_id, "Task6GM");
	}

	void TearDown() override
	{
		if (_app != nullptr)
		{
			_app->GetBotManager().Stop();
			_app->GetBotManager().RemoveAll();
		}
		_app.reset();
	}

	OperationMessage Operation(CUser* source = nullptr)
	{
		return OperationMessage(_app.get(), source == nullptr ? _gm.get() : source);
	}

	std::vector<std::string> BotNames() const
	{
		std::vector<std::string> names;
		for (const auto& entry : _app->GetBotRegistry().Snapshot())
			if (entry != nullptr && entry->m_pUserData != nullptr)
				names.emplace_back(entry->m_pUserData->m_id);
		std::sort(names.begin(), names.end());
		return names;
	}

	std::unique_ptr<TestApp> _app;
	std::shared_ptr<TestUser> _gm;
};

TEST(BotConfigTest, UsesExactMilestoneDefaults)
{
	TestApp app;
	CIni ini;
	ASSERT_TRUE(app.LoadBotConfig(ini));
	const BotConfig& config = app.GetBotConfig();
	EXPECT_FALSE(config.enabled);
	EXPECT_EQ(config.count, 10);
	EXPECT_EQ(config.tickMilliseconds, 200);
	EXPECT_EQ(config.respawnSeconds, 15);
	EXPECT_EQ(config.zoneId, ZONE_FRONTIER);
	EXPECT_FLOAT_EQ(config.attackRange, 2.5f);
	EXPECT_FLOAT_EQ(config.moveStep, 1.5f);
}

TEST(BotConfigTest, RejectsEveryOutOfRangeValueWithOneErrorAndDisablesBots)
{
	struct InvalidValue
	{
		const char* key;
		const char* value;
	};
	const std::vector<InvalidValue> invalidValues {
		{ "Enabled", "2" }, { "Count", "501" }, { "Count", "-1" },
		{ "TickMilliseconds", "199" }, { "RespawnSeconds", "14" },
		{ "Zone", "256" }, { "AttackRange", "0.49" }, { "AttackRange", "10.01" },
		{ "MoveStep", "0.09" }, { "MoveStep", "5.01" }, { "Count", "not-a-number" }
	};

	for (const auto& invalid : invalidValues)
	{
		TestApp app;
		CIni ini;
		ini.SetInt("BOTS", "Enabled", 1);
		ini.SetString("BOTS", invalid.key, invalid.value);
		std::ostringstream output;
		auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(output);
		auto logger = std::make_shared<spdlog::logger>("bot-config-test", sink);
		auto originalLogger = spdlog::default_logger();
		spdlog::set_default_logger(logger);
		spdlog::set_level(spdlog::level::err);

		EXPECT_FALSE(app.LoadBotConfig(ini)) << invalid.key << '=' << invalid.value;
		logger->flush();
		spdlog::set_default_logger(originalLogger);
		spdlog::set_level(spdlog::level::off);
		EXPECT_FALSE(app.GetBotConfig().enabled);
		const std::string logs = output.str();
		const std::string marker = "invalid BOTS config";
		const size_t first = logs.find(marker);
		ASSERT_NE(first, std::string::npos) << logs;
		EXPECT_EQ(logs.find(marker, first + marker.size()), std::string::npos) << logs;
	}
}

TEST(BotConfigTest, InvalidBotsSectionDoesNotFailOrdinaryLoadConfig)
{
	TestApp app;
	CIni ini;
	const auto root = std::filesystem::temp_directory_path() / "openko-task6-config";
	const auto maps = root / "MAP";
	const auto quests = root / "QUESTS";
	std::filesystem::create_directories(maps);
	std::filesystem::create_directories(quests);
	ini.SetString("PATH", "MAP_DIR", maps.string());
	ini.SetString("PATH", "QUESTS_DIR", quests.string());
	ini.SetInt("BOTS", "Enabled", 1);
	ini.SetInt("BOTS", "TickMilliseconds", 100);

	EXPECT_TRUE(app.LoadConfigForTest(ini));
	EXPECT_FALSE(app.GetBotConfig().enabled);
	std::filesystem::remove_all(root);
}

TEST_F(BotOperationMessageTest, ResolvesAllSupportedNationAndClassTokens)
{
	EXPECT_EQ(ResolveBotNation("karus"), NATION_KARUS);
	EXPECT_EQ(ResolveBotNation("ELMORAD"), NATION_ELMORAD);
	EXPECT_EQ(ResolveBotNation("unknown"), 0);
	EXPECT_EQ(ResolveBotClass(NATION_KARUS, "warrior"), CLASS_KA_WARRIOR);
	EXPECT_EQ(ResolveBotClass(NATION_KARUS, "rogue"), CLASS_KA_ROGUE);
	EXPECT_EQ(ResolveBotClass(NATION_KARUS, "mage"), CLASS_KA_WIZARD);
	EXPECT_EQ(ResolveBotClass(NATION_KARUS, "priest"), CLASS_KA_PRIEST);
	EXPECT_EQ(ResolveBotClass(NATION_ELMORAD, "warrior"), CLASS_EL_WARRIOR);
	EXPECT_EQ(ResolveBotClass(NATION_ELMORAD, "rogue"), CLASS_EL_ROGUE);
	EXPECT_EQ(ResolveBotClass(NATION_ELMORAD, "mage"), CLASS_EL_WIZARD);
	EXPECT_EQ(ResolveBotClass(NATION_ELMORAD, "priest"), CLASS_EL_PRIEST);
	EXPECT_EQ(ResolveBotClass(NATION_KARUS, "invalid"), CLASS_UNKNOWN);
}

TEST(BotReservedNameTest, MatchesExactBotPrefixesCaseInsensitively)
{
	EXPECT_TRUE(IsReservedBotName("Bot_K_000"));
	EXPECT_TRUE(IsReservedBotName("bOt_e_Legacy"));
	EXPECT_TRUE(IsReservedBotName("BOT_K_"));
	EXPECT_FALSE(IsReservedBotName("Bot_X_000"));
	EXPECT_FALSE(IsReservedBotName("BotK_000"));
	EXPECT_FALSE(IsReservedBotName("Player_Bot_K_000"));
}

TEST(BotReservedNameTest, CreationRejectsReservedNameWithoutAujardRequestAndAllowsNormalName)
{
	TestApp app;
	ASSERT_TRUE(app.AddCoefficientEntry(CLASS_KA_WARRIOR));
	auto user = app.AddUser();
	ASSERT_NE(user, nullptr);
	strcpy_safe(user->m_strAccountID, "CreateAccount");
	ScopedMessageQueue queue("ko_t6_nc");
	ASSERT_TRUE(app.m_LoggerSendQueue.Create(queue.Name()));
	user->AddSendCallback([](const char* packet, int length)
	{
		ASSERT_EQ(length, 2);
		EXPECT_EQ(static_cast<uint8_t>(packet[0]), WIZ_NEW_CHAR);
		EXPECT_EQ(static_cast<uint8_t>(packet[1]), 0x05);
	});

	auto reserved = NewCharacterRequest("bOt_K_Real");
	user->NewCharToAgent(reserved.data());
	char queued[SharedMemoryQueue::MAX_MSG_SIZE] {};
	EXPECT_EQ(app.m_LoggerSendQueue.GetData(queued), SMQ_EMPTY);
	EXPECT_STREQ(user->m_strAccountID, "CreateAccount");

	user->ResetSend();
	auto normal = NewCharacterRequest("NormalHero");
	user->NewCharToAgent(normal.data());
	const int queuedLength = app.m_LoggerSendQueue.GetData(queued);
	ASSERT_GT(queuedLength, 0);
	EXPECT_EQ(static_cast<uint8_t>(queued[0]), WIZ_NEW_CHAR);
	EXPECT_EQ(user->GetPacketsSent(), 0u);
}

TEST(BotReservedNameTest, CharacterSelectionRejectsReservedNameBeforeAujardRequest)
{
	TestApp app;
	ASSERT_NE(app.CreateMap(ZONE_FRONTIER, 256), nullptr);
	auto user = app.AddUser();
	ASSERT_NE(user, nullptr);
	strcpy_safe(user->m_strAccountID, "SelectAccount");
	app.m_nServerNo = 0;
	ScopedMessageQueue queue("ko_t6_sc");
	ASSERT_TRUE(app.m_LoggerSendQueue.Create(queue.Name()));
	user->AddSendCallback([](const char* packet, int length)
	{
		ASSERT_EQ(length, 2);
		EXPECT_EQ(static_cast<uint8_t>(packet[0]), WIZ_SEL_CHAR);
		EXPECT_EQ(static_cast<uint8_t>(packet[1]), 0x00);
	});

	auto reserved = SelectCharacterRequest(
		"SelectAccount", "BOT_E_Legacy", ZONE_FRONTIER);
	const int packetCountBefore = app.m_iPacketCount;
	user->SelCharToAgent(reserved.data());
	char queued[SharedMemoryQueue::MAX_MSG_SIZE] {};
	EXPECT_EQ(app.m_LoggerSendQueue.GetData(queued), SMQ_EMPTY);
	EXPECT_EQ(app.m_iPacketCount, packetCountBefore);
	EXPECT_STREQ(user->m_strAccountID, "SelectAccount");

	user->ResetSend();
	user->AddSendCallback([](const char*, int) { ADD_FAILURE() << "normal name was rejected"; });
	auto normal = SelectCharacterRequest(
		"SelectAccount", "NormalHero", ZONE_FRONTIER);
	user->SelCharToAgent(normal.data());
	const int queuedLength = app.m_LoggerSendQueue.GetData(queued);
	ASSERT_GT(queuedLength, 0);
	EXPECT_EQ(static_cast<uint8_t>(queued[0]), WIZ_SEL_CHAR);
	EXPECT_EQ(app.m_iPacketCount, packetCountBefore + 1);
	EXPECT_EQ(user->GetPacketsSent(), 0u);
}

TEST(BotReservedNameTest, SelectCharacterRejectsReservedLoadedIdentityWithoutStateMutation)
{
	TestApp app;
	auto user = app.AddUser();
	ASSERT_NE(user, nullptr);
	strcpy_safe(user->m_pUserData->m_id, "Bot_K_Legacy");
	user->m_pUserData->m_bZone = ZONE_BATTLE;
	user->SetState(CONNECTION_STATE_CONNECTED);
	user->AddSendCallback([](const char* packet, int length)
	{
		ASSERT_EQ(length, 2);
		EXPECT_EQ(static_cast<uint8_t>(packet[0]), WIZ_SEL_CHAR);
		EXPECT_EQ(static_cast<uint8_t>(packet[1]), 0x00);
	});
	const auto stateBefore = user->GetState();
	const int receivedBefore = app.m_iRecvPacketCount;
	char response[] { 1, 1 };

	user->SelectCharacter(response);

	EXPECT_EQ(user->GetState(), stateBefore);
	EXPECT_EQ(app.m_iRecvPacketCount, receivedBefore);
	EXPECT_EQ(user->GetPacketsSent(), 1u);
}

TEST_F(BotOperationMessageTest, OnlineReservedRealNameRejectsWholeBotBatch)
{
	auto real = _app->AddUser();
	ASSERT_NE(real, nullptr);
	strcpy_safe(real->m_pUserData->m_id, "bOt_K_000");
	real->SetState(CONNECTION_STATE_CONNECTED);

	auto operation = Operation();
	EXPECT_TRUE(operation.Process("+bot_add karus warrior 2"));
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
}

TEST_F(BotOperationMessageTest, AddsDeterministicallyNamedBotsAndRemovesThem)
{
	auto operation = Operation();
	EXPECT_TRUE(operation.Process("+bot_add KARUS WARRIOR 5"));
	EXPECT_TRUE(operation.Process("+bot_add elmorad priest 2"));
	EXPECT_EQ(_app->GetBotManager().Status().total, 7u);
	EXPECT_EQ(BotNames(), (std::vector<std::string> { "Bot_E_000", "Bot_E_001",
		"Bot_K_000", "Bot_K_001", "Bot_K_002", "Bot_K_003", "Bot_K_004" }));
	EXPECT_TRUE(operation.Process("+bot_remove_all"));
	EXPECT_EQ(_app->GetBotManager().Status().total, 0u);
}

TEST_F(BotOperationMessageTest, RejectsInvalidArgumentsWithoutChangingRegistry)
{
	auto operation = Operation();
	ASSERT_TRUE(operation.Process("+bot_add karus warrior 1"));
	const size_t before = _app->GetBotManager().Status().total;
	for (const char* command : { "+bot_add", "+bot_add orc warrior 1",
		"+bot_add karus bard 1", "+bot_add karus warrior 0",
		"+bot_add karus warrior 501", "+bot_add karus warrior nope" })
	{
		EXPECT_TRUE(operation.Process(command));
		EXPECT_EQ(_app->GetBotManager().Status().total, before) << command;
	}
}

TEST_F(BotOperationMessageTest, CapacityFailureLeavesAllPreExistingEntriesUntouched)
{
	for (int i = 0; i < MAX_BOT_USER - 1; ++i)
	{
		auto entry = std::make_shared<TestUser>();
		ASSERT_GE(_app->GetBotRegistry().Register(entry), 0);
	}
	const size_t before = _app->GetBotRegistry().Size();
	auto operation = Operation();
	EXPECT_TRUE(operation.Process("+bot_add karus warrior 2"));
	EXPECT_EQ(_app->GetBotRegistry().Size(), before);
}

TEST_F(BotOperationMessageTest, MissingZoneAndInvalidHomePositionLeaveRegistryUnchanged)
{
	CIni ini;
	ini.SetInt("BOTS", "Zone", ZONE_BATTLE);
	ASSERT_TRUE(_app->LoadBotConfig(ini));
	auto operation = Operation();
	EXPECT_TRUE(operation.Process("+bot_add karus warrior 1"));
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);

	ini.SetInt("BOTS", "Zone", ZONE_FRONTIER);
	ASSERT_TRUE(_app->LoadBotConfig(ini));
	auto* home = _app->m_HomeTableMap.GetData(NATION_KARUS);
	ASSERT_NE(home, nullptr);
	home->FreeZoneX = 5000;
	home->FreeZoneZ = 5000;
	EXPECT_TRUE(operation.Process("+bot_add karus warrior 1"));
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
}

TEST_F(BotOperationMessageTest, BatchFailureRollsBackOnlyBotsCreatedByThatBatch)
{
	BotSpawnRequest existing { "Existing", NATION_KARUS, CLASS_KA_WARRIOR, 60,
		{ ZONE_FRONTIER, 120.0f, 0.0f, 120.0f } };
	ASSERT_GE(_app->GetBotManager().Spawn(existing), 0);
	BotSpawnRequest valid { "BatchOne", NATION_KARUS, CLASS_KA_WARRIOR, 60,
		{ ZONE_FRONTIER, 121.0f, 0.0f, 121.0f } };
	BotSpawnRequest invalid { "BatchTwo", NATION_KARUS, CLASS_KA_WARRIOR, 60,
		{ ZONE_FRONTIER, 5000.0f, 0.0f, 5000.0f } };

	EXPECT_TRUE(_app->GetBotManager().SpawnBatch({ valid, invalid }).empty());
	ASSERT_EQ(_app->GetBotRegistry().Size(), 1u);
	EXPECT_STREQ(_app->GetBotRegistry().Snapshot().front()->m_pUserData->m_id, "Existing");
}

TEST_F(BotOperationMessageTest, ConcurrentSameNameBatchesAllowExactlyOneAtomicWinner)
{
	BotSpawnRequest request { "Bot_K_Race", NATION_KARUS, CLASS_KA_WARRIOR, 60,
		{ ZONE_FRONTIER, 121.0f, 0.0f, 121.0f } };
	std::barrier start(3);
	auto spawn = [&]()
	{
		start.arrive_and_wait();
		return _app->GetBotManager().SpawnBatch({ request });
	};
	auto first = std::async(std::launch::async, spawn);
	auto second = std::async(std::launch::async, spawn);
	start.arrive_and_wait();
	const auto firstResult = first.get();
	const auto secondResult = second.get();

	EXPECT_EQ(firstResult.size() + secondResult.size(), 1u);
	ASSERT_EQ(_app->GetBotRegistry().Size(), 1u);
	EXPECT_STREQ(_app->GetBotRegistry().Snapshot().front()->m_pUserData->m_id, "Bot_K_Race");
}

TEST_F(BotOperationMessageTest, DuplicateNamesInsideOneBatchFailWithoutPartialCreation)
{
	BotSpawnRequest first { "Bot_K_Duplicate", NATION_KARUS, CLASS_KA_WARRIOR, 60,
		{ ZONE_FRONTIER, 121.0f, 0.0f, 121.0f } };
	BotSpawnRequest second = first;
	second.spawn.x = 122.0f;

	EXPECT_TRUE(_app->GetBotManager().SpawnBatch({ first, second }).empty());
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
}

TEST_F(BotOperationMessageTest, ExpectedRegistrySizePreconditionPreservesManualBot)
{
	BotSpawnRequest manual { "Manual", NATION_KARUS, CLASS_KA_WARRIOR, 60,
		{ ZONE_FRONTIER, 120.0f, 0.0f, 120.0f } };
	ASSERT_GE(_app->GetBotManager().Spawn(manual), 0);
	BotSpawnRequest automatic { "Bot_E_000", NATION_ELMORAD, CLASS_EL_WARRIOR, 60,
		{ ZONE_FRONTIER, 140.0f, 0.0f, 140.0f } };

	EXPECT_TRUE(_app->GetBotManager().SpawnBatch({ automatic }, 0).empty());
	ASSERT_EQ(_app->GetBotRegistry().Size(), 1u);
	EXPECT_STREQ(_app->GetBotRegistry().Snapshot().front()->m_pUserData->m_id, "Manual");
}

TEST_F(BotOperationMessageTest, RemoveBatchLeavesUnrelatedManualBotRegistered)
{
	BotSpawnRequest manual { "Manual", NATION_KARUS, CLASS_KA_WARRIOR, 60,
		{ ZONE_FRONTIER, 120.0f, 0.0f, 120.0f } };
	ASSERT_GE(_app->GetBotManager().Spawn(manual), 0);
	BotSpawnRequest automatic { "Bot_E_000", NATION_ELMORAD, CLASS_EL_WARRIOR, 60,
		{ ZONE_FRONTIER, 140.0f, 0.0f, 140.0f } };
	const auto automaticIds = _app->GetBotManager().SpawnBatch({ automatic });
	ASSERT_EQ(automaticIds.size(), 1u);

	EXPECT_EQ(_app->GetBotManager().RemoveBatch(automaticIds), 1u);
	ASSERT_EQ(_app->GetBotRegistry().Size(), 1u);
	EXPECT_STREQ(_app->GetBotRegistry().Snapshot().front()->m_pUserData->m_id, "Manual");
}

TEST_F(BotOperationMessageTest, StartAndStatusCommandsWorkButCannotRestartAfterPermanentStop)
{
	auto operation = Operation();
	EXPECT_TRUE(operation.Process("+bot_start_pk"));
	EXPECT_TRUE(_app->GetBotManager().Status().running);
	EXPECT_TRUE(operation.Process("+bot_status"));
	_app->GetBotManager().Stop();
	EXPECT_TRUE(operation.Process("+bot_start_pk"));
	EXPECT_FALSE(_app->GetBotManager().Status().running);
}

TEST_F(BotOperationMessageTest, NonManagerCannotInvokeBotCommands)
{
	auto user = _app->AddUser();
	ASSERT_NE(user, nullptr);
	user->m_pUserData->m_bAuthority = AUTHORITY_USER;
	auto operation = Operation(user.get());
	EXPECT_TRUE(operation.Process("+bot_add karus warrior 1"));
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
}

TEST_F(BotOperationMessageTest, LoadedZoneValidationDisablesOnlyBots)
{
	CIni ini;
	ini.SetInt("BOTS", "Enabled", 1);
	ini.SetInt("BOTS", "Zone", ZONE_BATTLE);
	ASSERT_TRUE(_app->LoadBotConfig(ini));
	EXPECT_FALSE(_app->ValidateBotConfigZone());
	EXPECT_FALSE(_app->GetBotConfig().enabled);
	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
}

TEST_F(BotOperationMessageTest, AutoRosterCreatesExactlyFivePerNationAfterValidation)
{
	CIni ini;
	ini.SetInt("BOTS", "Enabled", 1);
	ini.SetInt("BOTS", "Count", 10);
	ASSERT_TRUE(_app->LoadBotConfig(ini));
	ASSERT_TRUE(_app->ValidateBotConfigZone());
	ASSERT_TRUE(_app->StartConfiguredBots());
	const BotStatus status = _app->GetBotManager().Status();
	EXPECT_EQ(status.total, 10u);
	EXPECT_TRUE(status.running);
	const auto names = BotNames();
	EXPECT_EQ(std::count_if(names.begin(), names.end(), [](const std::string& name)
		{ return name.starts_with("Bot_K_"); }), 5);
	EXPECT_EQ(std::count_if(names.begin(), names.end(), [](const std::string& name)
		{ return name.starts_with("Bot_E_"); }), 5);
}

TEST(BotConfiguredStartupTest, AutoRosterFailureRollsBackAndDisablesOnlyBotConfig)
{
	TestApp app;
	ASSERT_NE(app.CreateMap(ZONE_FRONTIER, 256), nullptr);
	ASSERT_TRUE(app.AddHomeEntry(NATION_KARUS, 120, 120));
	CIni ini;
	ini.SetInt("BOTS", "Enabled", 1);
	ini.SetInt("BOTS", "Count", 10);
	ASSERT_TRUE(app.LoadBotConfig(ini));
	ASSERT_TRUE(app.ValidateBotConfigZone());

	EXPECT_FALSE(app.StartConfiguredBots());
	EXPECT_FALSE(app.GetBotConfig().enabled);
	EXPECT_EQ(app.GetBotManager().Status().total, 0u);
	EXPECT_FALSE(app.GetBotManager().Status().running);
}

class BotConfiguredCountTest : public testing::TestWithParam<uint16_t>
{
};

TEST_P(BotConfiguredCountTest, ValidNonMilestoneCountDoesNotAutoSpawnOrStartTimer)
{
	TestApp app;
	ASSERT_NE(app.CreateMap(ZONE_FRONTIER, 256), nullptr);
	ASSERT_TRUE(app.AddHomeEntry(NATION_KARUS, 120, 120));
	ASSERT_TRUE(app.AddHomeEntry(NATION_ELMORAD, 140, 140));
	CIni ini;
	ini.SetInt("BOTS", "Enabled", 1);
	ini.SetInt("BOTS", "Count", GetParam());
	ASSERT_TRUE(app.LoadBotConfig(ini));
	ASSERT_TRUE(app.ValidateBotConfigZone());

	EXPECT_TRUE(app.StartConfiguredBots());
	EXPECT_EQ(app.GetBotManager().Status().total, 0u);
	EXPECT_FALSE(app.GetBotManager().Status().running);
}

INSTANTIATE_TEST_SUITE_P(NonMilestoneCounts, BotConfiguredCountTest,
	testing::Values<uint16_t>(0, 1, 9, 11, 500));
} // namespace
