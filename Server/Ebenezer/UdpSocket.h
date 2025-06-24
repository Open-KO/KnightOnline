// UdpSocket.h: interface for the CUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPSOCKET_H__E53802D9_5A8C_47B6_9B3B_12D2DDDACD92__INCLUDED_)
#define AFX_UDPSOCKET_H__E53802D9_5A8C_47B6_9B3B_12D2DDDACD92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEbenezerDlg;
class CUdpSocket
{
public:
	void RecvBattleZoneCurrentUsers(char* pBuf);
	void RecvDestroyKnights(char* pBuf);
	void RecvModifyFame(char* pBuf, BYTE command);
	void RecvJoinKnights(char* pBuf, BYTE command);
	void RecvCreateKnights(char* pBuf);
	void ReceiveKnightsProcess(char* pBuf);
	void RecvBattleEvent(char* pBuf);
	void ServerChat(char* pBuf);
	void Parsing(char* pBuf, int len);
	bool PacketProcess(int len);
	int  SendUDPPacket(char* strAddress, char* pBuf, int len);
	bool CreateSocket();
	CUdpSocket(CEbenezerDlg* pMain = nullptr);
	virtual ~CUdpSocket();

	SOCKET m_hUDPSocket;
	SOCKADDR_IN m_SocketAddress;
	SOCKADDR_IN m_ReplyAddress;

	HANDLE m_hUdpThread;
	char	m_pRecvBuf[8192];
	CEbenezerDlg* m_pMain;
};

#endif // !defined(AFX_UDPSOCKET_H__E53802D9_5A8C_47B6_9B3B_12D2DDDACD92__INCLUDED_)
