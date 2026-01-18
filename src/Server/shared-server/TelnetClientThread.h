#ifndef SERVER_SHAREDSERVER_TELNETCLIENTTHREAD_H
#define SERVER_SHAREDSERVER_TELNETCLIENTTHREAD_H

#pragma once

#include <shared/Thread.h>

#include <asio/ip/tcp.hpp>

class TelnetThread;
class TelnetClientThread : public Thread
{
	friend class TelnetThread;

public:
	TelnetClientThread(asio::ip::tcp::socket&& rawSocket, uint32_t socketId);
	~TelnetClientThread() override;
	void Disconnect();

protected:
	/// \brief The main thread loop for the telnet server
	void thread_loop() override;

	/// \brief Underlying client socket used for communication
	asio::ip::tcp::socket _clientSocket;

	/// \brief Socket identifier used in the TelnetThread client map
	uint32_t _socketId = 0;

	/// \brief Remote IP address of the client.
	std::string _remoteIp;

	/// \brief Authenticated account ID of the client.
	std::string _accountId;

private:
	/// \brief Writes a line to the client telnet application
	void WriteLine(const std::string& line);

	/// \brief Attempts to read a line from the client
	/// \returns Client input, or empty string on timeout/error
	std::string ReadLine();

	/// \brief Attempts to authenticate a user
	/// \returns true on the successful authentication of an account with strAuthority of AUTHORITY_MANAGER; false otherwise
	bool Authenticate(const std::string& accountId, const std::string& password);

	/// \brief Checks the current state of the AppThread and writes it to the client
	void HealthCheck();
};

#endif // SERVER_SHAREDSERVER_TELNETCLIENTTHREAD_H
