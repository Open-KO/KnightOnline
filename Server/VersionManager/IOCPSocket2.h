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

class CCircularBuffer;
class CIOCPSocket2
{
	friend class CIOCPort;

	using RawSocket_t = std::unique_ptr<asio::ip::tcp::socket>;

public:
	int GetSocketID() const
	{
		return m_Sid;
	}

	void SetSocketID(int sid)
	{
		m_Sid = sid;
	}

	uint8_t GetState() const
	{
		return m_State;
	}

	CIOCPSocket2(CIOCPort* iocport);
	virtual ~CIOCPSocket2();

	void InitSocket();
	void Close();
	bool PullOutCore(char*& data, int& length);
	void ReceivedData(int length);
	void Receive();
	int Send(char* pBuf, long length);

	virtual void CloseProcess();
	virtual void Parsing(int length, char* pData);
	virtual void Initialize();

	int16_t				m_nSocketErr;

protected:
	CIOCPort*			m_pIOCPort;
	CCircularBuffer*	m_pBuffer;

	RawSocket_t			m_Socket;

	char				m_pRecvBuff[SOCKET_BUFF_SIZE];

	uint8_t				m_State;
	int					m_Sid;
};

#endif // !defined(AFX_IOCPSOCKET2_H__36499609_63DD_459C_B4D0_1686FEEC67C2__INCLUDED_)
