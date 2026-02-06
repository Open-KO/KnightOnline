#ifndef TESTS_SERVER_EBENEZER_TESTUSER_H
#define TESTS_SERVER_EBENEZER_TESTUSER_H

#pragma once

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

private:
	_USER_DATA _userDataStore = {};
	size_t _packetsSent       = 0;
	std::queue<SendCallback> _sendCallbacks;
};

#endif // TESTS_SERVER_EBENEZER_TESTUSER_H
