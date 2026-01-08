#ifndef SERVER_EBENEZER_UDPSOCKET_H
#define SERVER_EBENEZER_UDPSOCKET_H

#pragma once

#include "Define.h"
#include "RecvUDPThread.h"

namespace Ebenezer
{

class RecvUDPThread;
class EbenezerApp;
class CUdpSocket
{
	friend class RecvUDPThread;

public:
	CUdpSocket(EbenezerApp* main = nullptr);
	virtual ~CUdpSocket();

	bool CreateSocket();
	void AsyncReceive();
	int SendUDPPacket(const std::string& strAddress, char* pBuf, int len);
	bool PacketProcess(int len);
	void Parsing(char* pBuf, int len);
	void ServerChat(char* pBuf);
	void RecvBattleEvent(char* pBuf);
	void ReceiveKnightsProcess(char* pBuf);
	void RecvCreateKnights(char* pBuf);
	void RecvJoinKnights(char* pBuf, uint8_t command);
	void RecvModifyFame(char* pBuf, uint8_t command);
	void RecvDestroyKnights(char* pBuf);
	void RecvBattleZoneCurrentUsers(char* pBuf);

protected:
	static constexpr int UDP_SOCKET_BUFFER_SIZE = (1024 * 32);

	RecvUDPThread _recvUdpThread;
	asio::io_context _io;
	asio::ip::udp::socket _socket;
	asio::ip::udp::endpoint _sender;

	char _recvBuff[UDP_SOCKET_BUFFER_SIZE] = {};
	EbenezerApp* _main                     = nullptr;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_UDPSOCKET_H
