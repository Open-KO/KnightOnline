// IOCPort.h: interface for the CIOCPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOCPORT_H__1555441D_142E_4C26_B889_D9DCFC5E54E8__INCLUDED_)
#define AFX_IOCPORT_H__1555441D_142E_4C26_B889_D9DCFC5E54E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// sungyong 2002.05.22
typedef struct _SEND_DATA
{
	int16_t	sCurZone;		// 현재의 존
	int16_t	sLength;		// 패킷의 길이
	char	pBuf[2048];		// 패킷의 내용..
} SEND_DATA;
// ~sungyong 2002.05.22

#include <list>

typedef  std::list<int>  SidList;
typedef  std::list<_SEND_DATA*>  SendDataList;	// sungyong~ 2002.05.22

class CIOCPSocket2;
class CIOCPort
{
public:
	void CreateSendThread();	// sungyong~ 2002.05.22
	void CreateAcceptThread();
	void RidIOCPSocket(int index, CIOCPSocket2* pSock);
	CIOCPSocket2* GetIOCPSocket(int index);
	void CreateReceiveWorkerThread(int workernum);
	void PutOldSid(int sid);
	int GetNewSid();
	bool Associate(CIOCPSocket2* pIocpSock, HANDLE hPort);
	bool Listen(int port);
	void Init(int serversocksize, int workernum = 0);
	void DeleteAllArray();
	CIOCPort();
	virtual ~CIOCPort();

	SOCKET m_ListenSocket;
	HANDLE m_hListenEvent;
	HANDLE m_hServerIOCPort;
	HANDLE m_hAcceptThread;
	HANDLE m_hSendIOCP;					// sungyong~ 2002.05.22

	int m_SocketArraySize;
	int m_AiSocketCount;				// sungyong~ 2002.05.22

	SidList m_SidList;
	SendDataList m_SendDataList;		// sungyong~ 2002.05.22
	CIOCPSocket2** m_SockArray;
	CIOCPSocket2** m_SockArrayInActive;

	CRITICAL_SECTION	m_critSendData;	// sungyong~ 2002.05.22

protected:
	uint32_t m_dwNumberOfWorkers;
	uint32_t m_dwConcurrency;
};

#endif // !defined(AFX_IOCPORT_H__1555441D_142E_4C26_B889_D9DCFC5E54E8__INCLUDED_)
