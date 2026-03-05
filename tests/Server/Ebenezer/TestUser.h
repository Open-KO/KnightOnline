#ifndef TESTS_SERVER_EBENEZER_TESTUSER_H
#define TESTS_SERVER_EBENEZER_TESTUSER_H

#pragma once

#include "packet_structs.h"

#include <Ebenezer/User.h>

#include <functional>
#include <queue>
#include <stdexcept>

class UnhandledSendCallbackException : public std::runtime_error
{
public:
	explicit UnhandledSendCallbackException() : runtime_error("unhandled send callback")
	{
	}
};

class TestUser : public Ebenezer::CUser
{
public:
	using SendCallback = std::function<void(const char*, int)>;

	TestUser() : CUser(test_tag {})
	{
		m_pUserData               = &_userDataStore;
		m_pUserData->m_bAuthority = AUTHORITY_USER;
	}

	size_t GetPacketsSent() const
	{
		return _packetsSent;
	}

	void ResetSend()
	{
		_packetsSent = 0;

		while (!_sendCallbacks.empty())
			_sendCallbacks.pop();
	}

	void AddSendCallback(const SendCallback& callback)
	{
		_sendCallbacks.push(callback);
	}

	int Send(char* pBuf, int length) override
	{
		++_packetsSent;

		if (_sendCallbacks.empty())
			throw UnhandledSendCallbackException();

		const auto& sendCallback = _sendCallbacks.front();
		if (sendCallback != nullptr)
			sendCallback(pBuf, length);

		_sendCallbacks.pop();
		return length;
	}

	// Calls GiveItem while handling packet callbacks inline
	void TestGiveItem(const int32_t itemId, const int16_t count)
	{
		if (itemId <= 0 || count <= 0)
			return;

		AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(WeightChangePacket));
				auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
			});
		AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(ItemCountChangePacket));
				auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
				EXPECT_EQ(packet->ItemId, itemId);
				EXPECT_EQ(packet->ItemCount, count);
			});
		EXPECT_TRUE(GiveItem(itemId, count));
	}

	// Calls GiveItem while handling packet callbacks inline
	void TestRobItem(const int32_t itemId, const int16_t count)
	{
		if (itemId <= 0 || count <= 0)
			return;

		AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(WeightChangePacket));
				auto packet = reinterpret_cast<const WeightChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_WEIGHT_CHANGE);
			});
		AddSendCallback(
			[=](const char* pBuf, int len)
			{
				ASSERT_EQ(len, sizeof(ItemCountChangePacket));
				auto packet = reinterpret_cast<const ItemCountChangePacket*>(pBuf);
				EXPECT_EQ(packet->Opcode, WIZ_ITEM_COUNT_CHANGE);
				EXPECT_EQ(packet->ItemId, itemId);
			});
		EXPECT_TRUE(RobItem(itemId, count));
	}

	void ClearInventory()
	{
		for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
		{
			m_pUserData->m_sItemArray[i].nNum           = 0;
			m_pUserData->m_sItemArray[i].sCount         = 0;
			m_pUserData->m_sItemArray[i].sDuration      = 0;
			m_pUserData->m_sItemArray[i].nSerialNum     = 0;
			m_pUserData->m_sItemArray[i].byFlag         = 0;
			m_pUserData->m_sItemArray[i].sTimeRemaining = 0;
		}
	}

private:
	_USER_DATA _userDataStore = {};
	size_t _packetsSent       = 0;
	std::queue<SendCallback> _sendCallbacks;
};

#endif // TESTS_SERVER_EBENEZER_TESTUSER_H
