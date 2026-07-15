#include <gtest/gtest.h>

#include <Ebenezer/Define.h>

TEST(BotCapacityTest, KeepsSocketsAndBotsInDisjointBands)
{
	EXPECT_EQ(Ebenezer::MAX_SOCKET_USER, 3000);
	EXPECT_EQ(Ebenezer::BOT_USER_ID_MIN, 3000);
	EXPECT_EQ(Ebenezer::BOT_USER_ID_MAX, 3499);
	EXPECT_EQ(Ebenezer::MAX_USER, 3500);
	EXPECT_LT(Ebenezer::BOT_USER_ID_MAX, Ebenezer::NPC_BAND);
}
