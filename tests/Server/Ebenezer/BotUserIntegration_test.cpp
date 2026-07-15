#include <gtest/gtest.h>

#include <Ebenezer/BotUser.h>
#include <Ebenezer/EbenezerApp.h>

#include <shared/packets.h>
#include <shared/StringUtils.h>
#include <shared-server/utilities.h>

#include "TestApp.h"

#include <memory>

using namespace Ebenezer;

namespace
{

class BotUserIntegrationTest : public testing::Test
{
protected:
	void SetUp() override
	{
		_app = std::make_unique<TestApp>();
		_map = _app->CreateMap(ZONE_FRONTIER);
		ASSERT_NE(_map, nullptr);
	}

	BotSpawnRequest MakeSpawnRequest(uint8_t nation = NATION_KARUS) const
	{
		BotSpawnRequest request;
		request.name           = nation == NATION_KARUS ? "Bot_K_000" : "Bot_E_000";
		request.nation         = nation;
		request.characterClass = nation == NATION_KARUS ? CLASS_KA_WARRIOR : CLASS_EL_WARRIOR;
		request.level          = 60;
		request.spawn          = { ZONE_FRONTIER, 100.0f, 0.0f, 100.0f };
		return request;
	}

	std::shared_ptr<CBotUser> CreateBot(uint8_t nation = NATION_KARUS)
	{
		auto bot = std::make_shared<CBotUser>();
		if (!bot->InitializeBot(MakeSpawnRequest(nation)))
			return nullptr;

		const int botId = _app->GetBotRegistry().Register(bot);
		if (botId < 0)
			return nullptr;
		return bot;
	}

	std::shared_ptr<TestUser> CreateRealUser(uint8_t nation)
	{
		auto user = _app->AddUser();
		if (user == nullptr)
			return nullptr;

		strcpy_safe(user->m_pUserData->m_id, nation == NATION_KARUS ? "Real_K" : "Real_E");
		user->m_pUserData->m_bNation = nation;
		user->m_pUserData->m_sClass =
			nation == NATION_KARUS ? CLASS_KA_WARRIOR : CLASS_EL_WARRIOR;
		user->m_pUserData->m_bLevel = 60;
		user->m_pUserData->m_bZone  = ZONE_FRONTIER;
		user->m_pUserData->m_curx   = 100.0f;
		user->m_pUserData->m_cury   = 0.0f;
		user->m_pUserData->m_curz   = 100.0f;
		user->m_iMaxHp              = 1500;
		user->m_pUserData->m_sHp    = user->m_iMaxHp;
		user->m_sTotalHit           = 180;
		user->m_sTotalAc            = 120;
		user->m_fTotalHitRate       = 1.0f;
		user->m_fTotalEvasionRate   = 1.0f;
		user->m_bAttackAmount       = 100;
		user->SetState(CONNECTION_STATE_GAMESTART);
		_map->Add(user.get(), 2, 2);
		return user;
	}

	static void Attack(CUser& attacker, int targetId)
	{
		char attack[16] {};
		int index = 0;
		SetByte(attack, DIRECT_ATTACK, index);
		SetByte(attack, 1, index);
		SetShort(attack, targetId, index);
		SetShort(attack, 100, index);
		SetShort(attack, 0, index);
		attacker.Attack(attack);
	}

	std::unique_ptr<TestApp> _app;
	TestMap* _map = nullptr;
};

TEST_F(BotUserIntegrationTest, InitializesAndRegistersSocketlessBot)
{
	auto bot = CreateBot();
	ASSERT_NE(bot, nullptr);

	char buffer[8] {};
	const int botId = bot->GetSocketID();

	EXPECT_EQ(bot->Send(buffer, sizeof(buffer)), sizeof(buffer));
	EXPECT_FALSE(bot->HasSocket());
	EXPECT_EQ(bot->GetManager(), nullptr);
	EXPECT_EQ(bot->GetState(), CONNECTION_STATE_GAMESTART);
	EXPECT_EQ(_app->GetUserPtr(botId).get(), bot.get());
	EXPECT_TRUE(_app->IsValidUserId(botId));
	EXPECT_EQ(_app->GetUserSocketCount(), MAX_SOCKET_USER);
	EXPECT_EQ(bot->m_pUserData->m_bNation, NATION_KARUS);
	EXPECT_EQ(bot->m_pUserData->m_sClass, CLASS_KA_WARRIOR);
	EXPECT_EQ(bot->m_pUserData->m_bLevel, 60);
	EXPECT_EQ(bot->m_iMaxHp, 1500);
	EXPECT_EQ(bot->m_pUserData->m_sHp, 1500);
	EXPECT_EQ(bot->m_iMaxMp, 500);
	EXPECT_EQ(bot->m_pUserData->m_sMp, 500);
	EXPECT_EQ(bot->m_sTotalHit, 180);
	EXPECT_EQ(bot->m_sTotalAc, 120);
	EXPECT_FLOAT_EQ(bot->m_fTotalHitRate, 1.0f);
	EXPECT_FLOAT_EQ(bot->m_fTotalEvasionRate, 1.0f);
	EXPECT_EQ(bot->m_sSpeed, 45);
	EXPECT_EQ(bot->m_pUserData->m_iLoyalty, 100);
	EXPECT_EQ(bot->m_pUserData->m_iGold, 0);
	EXPECT_EQ(bot->Runtime().home.zoneId, ZONE_FRONTIER);
}

TEST_F(BotUserIntegrationTest, RejectsInvalidSpawnBeforeRegistration)
{
	auto request = MakeSpawnRequest();
	auto bot     = std::make_shared<CBotUser>();

	request.name.clear();
	EXPECT_FALSE(bot->InitializeBot(request));
	request = MakeSpawnRequest();
	request.nation = 0;
	EXPECT_FALSE(bot->InitializeBot(request));
	request = MakeSpawnRequest();
	request.characterClass = CLASS_EL_WARRIOR;
	EXPECT_FALSE(bot->InitializeBot(request));
	request = MakeSpawnRequest();
	request.level = 0;
	EXPECT_FALSE(bot->InitializeBot(request));
	request = MakeSpawnRequest();
	request.spawn.zoneId = 250;
	EXPECT_FALSE(bot->InitializeBot(request));
	request = MakeSpawnRequest();
	request.spawn.x = -1.0f;
	EXPECT_FALSE(bot->InitializeBot(request));

	EXPECT_EQ(_app->GetBotRegistry().Size(), 0u);
	EXPECT_EQ(bot->GetState(), CONNECTION_STATE_DISCONNECTED);
}

TEST_F(BotUserIntegrationTest, AddsAndRemovesBotFromRegion)
{
	auto bot = CreateBot();
	ASSERT_NE(bot, nullptr);
	const int botId = bot->GetSocketID();

	bot->UserInOut(USER_IN);
	EXPECT_NE(_map->m_ppRegion[bot->m_RegionX][bot->m_RegionZ].m_RegionUserArray.GetData(botId),
		nullptr);

	bot->UserInOut(USER_OUT);
	EXPECT_EQ(_map->m_ppRegion[bot->m_RegionX][bot->m_RegionZ].m_RegionUserArray.GetData(botId),
		nullptr);
}

TEST_F(BotUserIntegrationTest, RealUserCanAttackBotId)
{
	auto real = CreateRealUser(NATION_ELMORAD);
	auto bot  = CreateBot(NATION_KARUS);
	ASSERT_NE(real, nullptr);
	ASSERT_NE(bot, nullptr);
	real->m_fTotalHitRate = 10.0f;
	for (int i = 0; i < 100; ++i)
		real->AddSendCallback([](const char*, int) {});
	const int initialHp = bot->m_pUserData->m_sHp;

	for (int i = 0; i < 100 && bot->m_pUserData->m_sHp == initialHp; ++i)
		EXPECT_NO_THROW(Attack(*real, bot->GetSocketID()));

	EXPECT_LT(bot->m_pUserData->m_sHp, initialHp);
}

TEST_F(BotUserIntegrationTest, BotCanAttackRealUserWithoutSocketManager)
{
	auto bot  = CreateBot(NATION_KARUS);
	auto real = CreateRealUser(NATION_ELMORAD);
	ASSERT_NE(bot, nullptr);
	ASSERT_NE(real, nullptr);
	bot->m_fTotalHitRate = 10.0f;
	for (int i = 0; i < 100; ++i)
		real->AddSendCallback([](const char*, int) {});
	const int initialHp = real->m_pUserData->m_sHp;

	for (int i = 0; i < 100 && real->m_pUserData->m_sHp == initialHp; ++i)
		EXPECT_NO_THROW(Attack(*bot, real->GetSocketID()));

	EXPECT_LT(real->m_pUserData->m_sHp, initialHp);
}

} // namespace
