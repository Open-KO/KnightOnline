#include <gtest/gtest.h>
#include "TestApp.h"
#include "TestUser.h"

#include "data/UserKnightsRank_test_data.h"
#include "data/UserPersonalRank_test_data.h"
#include "shared-server/utilities.h"

#include <cstdlib>

using namespace Ebenezer;

// linter skips for this test
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
/**
 * Tests CUser::HandleUserStipendResponse, which handles the result of the Aujard DB Operation
 */
class RankStipendRewardTest : public testing::Test
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

	/// \brief Sets the given charId on the packet
	/// \param packet
	/// \param charId
	void SetPacketName(StipendResponseSubPacket& packet, const std::string_view charId)
	{
		memset(packet.charId, 0, MAX_ID_SIZE + 1);
		packet.charIdLength = static_cast<uint8_t>(charId.length());
		memcpy(packet.charId, charId.data(), charId.length());
	}

	/// \brief Mocks user data based on a test packet
	/// \param packet
	void MockUserFor(const StipendResponseSubPacket& packet)
	{
		static constexpr char KarusPrefix[3] = "KA";
		static constexpr char ElmoPrefix[3]  = "EL";

		// validate params
		ASSERT_GE(packet.rank, 1);
		ASSERT_LE(packet.rank, 100);
		ASSERT_GE(packet.nation, NATION_KARUS);
		ASSERT_LE(packet.nation, NATION_ELMORAD);

		// Reset gold between tests
		_user->m_pUserData->m_iGold = 0;

		// Set name
		snprintf(_user->m_pUserData->m_id, MAX_ID_SIZE + 1, "%s_RANK_%d",
			packet.nation == NATION_KARUS ? KarusPrefix : ElmoPrefix, packet.rank);

		// Set nation
		_user->m_pUserData->m_bNation = packet.nation;
	}

	/// \brief Checks that a stipend has been claimed for a given rank
	/// \param type
	/// \param rank
	/// \return true when the stipend is claimed, false otherwise
	bool VerifyClaim(const e_StipendType type, const uint8_t rank) const
	{
		switch (type)
		{
			case STIPEND_TYPE_USER_KNIGHTS:
			{
				const auto rankInfo = _app->m_UserKnightsRankMap.GetData(rank);
				if (rankInfo == nullptr)
					return false;

				return _user->m_pUserData->m_bNation == NATION_KARUS ? rankInfo->IsClaimedKarus
																	 : rankInfo->IsClaimedElmo;
			}
			case STIPEND_TYPE_USER_PERSONAL:
			{
				const auto rankInfo = _app->m_UserPersonalRankMap.GetData(rank);
				if (rankInfo == nullptr)
					return false;

				return _user->m_pUserData->m_bNation == NATION_KARUS ? rankInfo->IsClaimedKarus
																	 : rankInfo->IsClaimedElmo;
			}
		}

		return false;
	}

	/// \brief Checks if the user gold amount equals their stipend amount
	/// \param type
	/// \param rank
	/// \return true when stipend amount equals user gold amount, false otherwise
	bool VerifyPay(const e_StipendType type, const uint8_t rank) const
	{
		return GetRewardAmount(type, rank) == _user->m_pUserData->m_iGold;
	}

	/// \brief Returns the stipend amount from the correct cache for the given rank
	/// \param type
	/// \param rank
	/// \return Stipend amount
	int32_t GetRewardAmount(const uint8_t type, const uint8_t rank) const
	{
		switch (type)
		{
			case STIPEND_TYPE_USER_KNIGHTS:
			{
				const auto rankInfo = _app->m_UserKnightsRankMap.GetData(rank);
				if (rankInfo == nullptr)
					return 0;

				return rankInfo->Money;
			}
			case STIPEND_TYPE_USER_PERSONAL:
			{
				const auto rankInfo = _app->m_UserPersonalRankMap.GetData(rank);
				if (rankInfo == nullptr)
					return 0;

				return rankInfo->Salary;
			}
			default:
				break;
		}

		return 0;
	}

	/// \brief Handles the verification of the messages sent by CUser in the cases where
	/// stipends should be paid.
	/// \param testPacket
	void AddSendCallbacks_StipendPaid(const StipendResponseSubPacket& testPacket)
	{
		// Get Reward Amount
		const int32_t stipendReward = GetRewardAmount(testPacket.stipendType, testPacket.rank);

		// GoldGain callback
		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(GoldChangePacket));
				auto packet = reinterpret_cast<const GoldChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_GOLD_CHANGE);
				EXPECT_EQ(packet->SubOpcode, GOLD_CHANGE_GAIN);
				EXPECT_EQ(packet->ChangeAmount, stipendReward);
			});

		// ItemLog callback
		// If we test m_ItemLoggerSendQ traffic in the future, uncomment and update this
		// _user->AddSendCallback(
		// 	[=](const char* pBuf, int len)
		// 	{
		// 		int index = 0;
		// 		char srcId[MAX_ID_SIZE + 1] {};
		// 		char targetId[MAX_ID_SIZE + 1] {};
		// 		ASSERT_NE(pBuf, nullptr);
		// 		ASSERT_GT(len, 0);
		// 		EXPECT_EQ(GetByte(pBuf, index), WIZ_ITEM_LOG);
		// 		// srcId
		// 		int16_t srcLen = GetShort(pBuf, index);
		// 		GetString(srcId, pBuf, srcLen, index);
		// 		EXPECT_EQ(srcId, _user->m_pUserData->m_id);
		// 		// targetId
		// 		int16_t targetLen = GetShort(pBuf, index);
		// 		GetString(targetId, pBuf, targetLen, index);
		// 		EXPECT_EQ(targetId, _user->m_pUserData->m_id);
		// 		// log type
		// 		uint8_t expectedLogType = testPacket.stipendType == STIPEND_TYPE_USER_KNIGHTS
		// 									  ? ITEM_LOG_KNIGHTS_REWARD
		// 									  : ITEM_LOG_PERSONAL_REWARD;
		// 		EXPECT_EQ(GetByte(pBuf, index), expectedLogType);
		// 		// serial
		// 		EXPECT_EQ(GetInt64(pBuf, index), 0);
		// 		// itemId
		// 		EXPECT_EQ(GetDWORD(pBuf, index), ITEM_NOAH);
		// 		// count
		// 		EXPECT_EQ(GetShort(pBuf, index), stipendReward);
		// 		// durability
		// 		EXPECT_EQ(GetDWORD(pBuf, index), 0);
		// 	});

		// SendSay callback
		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(SendSayPacket));
				auto packet = reinterpret_cast<const SendSayPacket*>(pBuf);
				EXPECT_EQ(packet->opCode, WIZ_NPC_SAY);
				EXPECT_EQ(packet->message1, MSG_STIPEND_GIVE_REWARD);
			});
	}

	/// \brief Handles the verification of the messages sent by CUser in the cases where
	/// a stipend is not paid because it was already claimed
	void AddSendCallbacks_AlreadyClaimed()
	{
		// SendSay callback
		_user->AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(SendSayPacket));
				auto packet = reinterpret_cast<const SendSayPacket*>(pBuf);
				EXPECT_EQ(packet->opCode, WIZ_NPC_SAY);
				EXPECT_EQ(packet->message1, MSG_STIPEND_ALREADY_CLAIMED);
			});
	}
};

// This test covers CUser::HandleUserStipendResponse cumulative rankings (USER_KNIGHTS_RANK) cases
TEST_F(RankStipendRewardTest, CumulativeRank)
{
	_user->ResetSend();

	StipendResponseSubPacket testPacket = {};
	testPacket.stipendType              = STIPEND_TYPE_USER_KNIGHTS;
	testPacket.responseCode             = STIPEND_RESPONSE_SUCCESS;
	char* packetBuffer                  = reinterpret_cast<char*>(&testPacket);

	// Test individual ranks - positive tests
	for (testPacket.rank = 1; testPacket.rank < 101; testPacket.rank++)
	{
		// Test Karus Ranks
		testPacket.nation = NATION_KARUS;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_StipendPaid(testPacket);
		EXPECT_TRUE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));
		EXPECT_TRUE(VerifyPay(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));

		// Test Elmo Ranks
		testPacket.nation = NATION_ELMORAD;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_StipendPaid(testPacket);
		EXPECT_TRUE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));
		EXPECT_TRUE(VerifyPay(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));
	}

	// Test individual ranks - already claimed
	testPacket.responseCode = STIPEND_RESPONSE_ALREADY_CLAIMED;
	for (testPacket.rank = 1; testPacket.rank < 101; testPacket.rank++)
	{
		// Test Karus Ranks
		testPacket.nation = NATION_KARUS;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_AlreadyClaimed();
		EXPECT_FALSE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));
		EXPECT_FALSE(VerifyPay(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));

		// Test Elmo Ranks
		testPacket.nation = NATION_ELMORAD;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_AlreadyClaimed();
		EXPECT_FALSE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));
		EXPECT_FALSE(VerifyPay(STIPEND_TYPE_USER_KNIGHTS, testPacket.rank));
	}
}

// This test covers CUser::HandleUserStipendResponse monthly rankings (USER_PERSONAL_RANK) cases
TEST_F(RankStipendRewardTest, MonthlyRank)
{
	_user->ResetSend();

	StipendResponseSubPacket testPacket = {};
	testPacket.stipendType              = STIPEND_TYPE_USER_PERSONAL;
	testPacket.responseCode             = STIPEND_RESPONSE_SUCCESS;
	char* packetBuffer                  = reinterpret_cast<char*>(&testPacket);

	// Test individual ranks - positive tests
	for (testPacket.rank = 1; testPacket.rank < 101; testPacket.rank++)
	{
		// Test Karus Ranks
		testPacket.nation = NATION_KARUS;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_StipendPaid(testPacket);
		EXPECT_TRUE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));
		EXPECT_TRUE(VerifyPay(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));

		// Test Elmo Ranks
		testPacket.nation = NATION_ELMORAD;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_StipendPaid(testPacket);
		EXPECT_TRUE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));
		EXPECT_TRUE(VerifyPay(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));
	}

	// Test individual ranks - already claimed
	testPacket.responseCode = STIPEND_RESPONSE_ALREADY_CLAIMED;
	for (testPacket.rank = 1; testPacket.rank < 101; testPacket.rank++)
	{
		// Test Karus Ranks
		testPacket.nation = NATION_KARUS;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_AlreadyClaimed();
		EXPECT_FALSE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));
		EXPECT_FALSE(VerifyPay(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));

		// Test Elmo Ranks
		testPacket.nation = NATION_ELMORAD;
		MockUserFor(testPacket);
		SetPacketName(testPacket, _user->m_pUserData->m_id);
		AddSendCallbacks_AlreadyClaimed();
		EXPECT_FALSE(_user->HandleUserStipendResponse(packetBuffer));
		EXPECT_TRUE(VerifyClaim(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));
		EXPECT_FALSE(VerifyPay(STIPEND_TYPE_USER_PERSONAL, testPacket.rank));
	}
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
