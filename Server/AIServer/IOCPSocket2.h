// IOCPSocket2.h: interface for the CIOCPSocket2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOCPSOCKET2_H__36499609_63DD_459C_B4D0_1686FEEC67C2__INCLUDED_)
#define AFX_IOCPSOCKET2_H__36499609_63DD_459C_B4D0_1686FEEC67C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOCPort.h"
#include "Define.h"

#define receives				0
#define sends					1
#define both					2 

class CCircularBuffer;

class CIOCPSocket2
{
public:
	void InitSocket(CIOCPort* pIOCPort);
	void Close();
	bool AsyncSelect(long lEvent = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
	bool SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel = SOL_SOCKET);
	bool ShutDown(int nHow = sends);
	bool PullOutCore(char*& data, int& length);
	void ReceivedData(int length);
	int  Receive();
	int  Send(char* pBuf, long length, int dwFlag = 0);
	bool Connect(CIOCPort* pIocp, const char*  lpszHostAddress, UINT nHostPort);
	bool Create(UINT nSocketPort = 0,
				 int nSocketType = SOCK_STREAM,
				 long lEvent = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,
				 const char* lpszSocketAddress = nullptr);
	bool Accept(SOCKET listensocket, struct sockaddr* addr, int* len);

	int GetSocketID() const {
		return m_Sid;
	}

	void SetSocketID(int sid) {
		m_Sid = sid;
	}

	HANDLE GetSocketHandle() const {
		return (HANDLE) m_Socket;
	}

	BYTE GetState() const {
		return m_State;
	}

	BYTE GetSockType() const {
		return m_Type;
	}

	virtual void CloseProcess();
	virtual void Parsing(int length, char* pData);
	virtual void Initialize();

	CIOCPSocket2();
	virtual ~CIOCPSocket2();

	short				m_nSocketErr;
	short				m_nPending;
	short				m_nWouldblock;

protected:
	CIOCPort*			m_pIOCPort;
	CCircularBuffer*	m_pBuffer;

	SOCKET				m_Socket;

	char				m_pRecvBuff[MAX_PACKET_SIZE];
	char				m_pSendBuff[MAX_PACKET_SIZE];

	HANDLE				m_hSockEvent;

	OVERLAPPED			m_RecvOverlapped;
	OVERLAPPED			m_SendOverlapped;

	BYTE				m_Type;
	BYTE				m_State;
	int					m_Sid;
	std::string			m_ConnectAddress;

	DWORD				m_wPacketSerial;

};

#endif // !defined(AFX_IOCPSOCKET2_H__36499609_63DD_459C_B4D0_1686FEEC67C2__INCLUDED_)
