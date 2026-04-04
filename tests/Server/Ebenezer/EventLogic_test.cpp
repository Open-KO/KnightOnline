#include <gtest/gtest.h>
#include "TestApp.h"
#include "TestUser.h"

#include "data/UserKnightsRank_test_data.h"
#include "data/UserPersonalRank_test_data.h"

#include <cstdlib>

using namespace Ebenezer;

// linter skips for this test
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
/**
 * Tests CUser .evt logic events (A)
 */
class EventLogicTest : public testing::Test
{
protected:
	std::unique_ptr<TestApp> _app   = nullptr;
	std::shared_ptr<TestUser> _user = nullptr;

	void SetUp() override
	{
		_app = std::make_unique<TestApp>();
		ASSERT_TRUE(_app != nullptr);

		// Load test tables
		for (const auto& userKnightsRankModel : s_userKnightsRankData)
			_app->AddUserKnightsRankEntry(userKnightsRankModel);

		for (const auto& userPersonalRankModel : s_userPersonalRankData)
			_app->AddUserPersonalRankEntry(userPersonalRankModel);

		// Setup user
		_user = _app->AddUser();
		ASSERT_TRUE(_user != nullptr);

		// Mark player as ingame
		_user->SetState(CONNECTION_STATE_GAMESTART);
	}

	void TearDown() override
	{
		_user.reset();
		_app.reset();
	}
};

// This test covers CUser::CheckManner.  Simple range test
TEST_F(EventLogicTest, CheckManner)
{
	_user->ResetSend();
	_user->m_pUserData->m_iMannerPoint = 100;
	EXPECT_FALSE(_user->CheckManner(0, 99));
	EXPECT_TRUE(_user->CheckManner(0, 100));
	EXPECT_TRUE(_user->CheckManner(100, 500));
	EXPECT_FALSE(_user->CheckManner(101, 500));
}

// This test covers CUser::CheckUserKnightsRanking
TEST_F(EventLogicTest, CheckUserRanking)
{
	_user->ResetSend();

	// Bound/sanity checks
	// Would be bad evt config; Logger would WARN
	EXPECT_FALSE(_user->CheckUserRanking(-1, 50));
	// function should only check the first 100 ranks
	EXPECT_FALSE(_user->CheckUserRanking(101, 50000));
	// max < min is invalid
	EXPECT_FALSE(_user->CheckUserRanking(50, 1));

	// Test individual ranks
	for (uint8_t type = 0; type < 2; type++)
	{
		for (int i = 1; i < 101; i++)
		{
			// Test Karus Ranks
			_user->m_pUserData->m_bNation = NATION_KARUS;
			snprintf(_user->m_pUserData->m_id, MAX_ID_SIZE + 1, "KA_RANK_%d", i);
			EXPECT_TRUE(_user->CheckUserRanking(1, 100, static_cast<e_StipendType>(type)));

			// Test Elmo Ranks
			_user->m_pUserData->m_bNation = NATION_ELMORAD;
			snprintf(_user->m_pUserData->m_id, MAX_ID_SIZE + 1, "EL_RANK_%d", i);
			EXPECT_TRUE(_user->CheckUserRanking(1, 100, static_cast<e_StipendType>(type)));
		}
	}

	// Always false on overflow servers
	_app->m_nServerGroup = SERVER_GROUP_OVERFLOW;
	EXPECT_FALSE(_user->CheckUserRanking());
	_app->m_nServerGroup = SERVER_GROUP_NONE;

	// unranked chars
	for (uint8_t type = 0; type < 2; type++)
	{
		_user->m_pUserData->m_bNation = NATION_KARUS;
		snprintf(_user->m_pUserData->m_id, MAX_ID_SIZE + 1, "KA_UNRANKED");
		EXPECT_FALSE(_user->CheckUserRanking(1, 100, static_cast<e_StipendType>(type)));

		_user->m_pUserData->m_bNation = NATION_ELMORAD;
		snprintf(_user->m_pUserData->m_id, MAX_ID_SIZE + 1, "EL_UNRANKED");
		EXPECT_FALSE(_user->CheckUserRanking(1, 100, static_cast<e_StipendType>(type)));
	}
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
