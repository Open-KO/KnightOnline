// UdpSocket.h: interface for the CUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPSOCKET_H__E53802D9_5A8C_47B6_9B3B_12D2DDDACD92__INCLUDED_)
#define AFX_UDPSOCKET_H__E53802D9_5A8C_47B6_9B3B_12D2DDDACD92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Define.h"
#include "RecvUDPThread.h"

class RecvUDPThread;
class CEbenezerDlg;
class CUdpSocket
{
	friend class RecvUDPThread;

public:
	CUdpSocket(CEbenezerDlg* main = nullptr);
	virtual ~CUdpSocket();

	bool CreateSocket();
	void AsyncReceive();
	int  SendUDPPacket(char* strAddress, char* pBuf, int len);
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
	static constexpr int UDP_SOCKET_BUFFER_SIZE	= (1024*32);

	RecvUDPThread			_recvUdpThread;
	asio::io_context		_io;
	asio::ip::udp::socket	_socket;
	asio::ip::udp::endpoint _sender;

	char					_recvBuff[UDP_SOCKET_BUFFER_SIZE];
	CEbenezerDlg*			_main;
};

#endif // !defined(AFX_UDPSOCKET_H__E53802D9_5A8C_47B6_9B3B_12D2DDDACD92__INCLUDED_)
