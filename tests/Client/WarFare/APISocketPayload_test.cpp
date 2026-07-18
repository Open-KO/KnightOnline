#include <gtest/gtest.h>

#include "Client/WarFare/APISocketPayload.h"

#include <array>

TEST(APISocketPayloadTest, PadsOneBytePayloadToServerMinimum)
{
	const std::array<uint8_t, 1> input { 0xF5 };
	std::array<uint8_t, 2> storage {};

	const api_socket::PayloadView payload
		= api_socket::ApplyMinimumUnencryptedPayloadCompatibility(
			input.data(), static_cast<int>(input.size()), storage);

	ASSERT_EQ(payload.Size, 2);
	EXPECT_EQ(payload.Data[0], 0xF5);
	EXPECT_EQ(payload.Data[1], 0x00);
}

TEST(APISocketPayloadTest, LeavesLargerPayloadUnchanged)
{
	const std::array<uint8_t, 2> input { 0xF5, 0x7A };
	std::array<uint8_t, 2> storage { 0xCC, 0xCC };

	const api_socket::PayloadView payload
		= api_socket::ApplyMinimumUnencryptedPayloadCompatibility(
			input.data(), static_cast<int>(input.size()), storage);

	EXPECT_EQ(payload.Data, input.data());
	EXPECT_EQ(payload.Size, static_cast<int>(input.size()));
	EXPECT_EQ(payload.Data[0], 0xF5);
	EXPECT_EQ(payload.Data[1], 0x7A);
}
