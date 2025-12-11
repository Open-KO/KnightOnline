// APISocket.h: interface for the CAPISocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_)
#define AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Base.h>
#include "GameDef.h"
#include "PacketDef.h"

#include <queue>
#include <string>

#define WM_SOCKETMSG	(WM_USER+1)
#define RECEIVE_BUF_SIZE	262144 // 최대 버퍼..

#define _CRYPTION		// 암호화 사용
#ifdef _CRYPTION
#include <shared/JvCryption.h>
#endif

class BB_CircularBuffer  
{
public:
	BB_CircularBuffer(int size=RECEIVE_BUF_SIZE)
	{
		__ASSERT(size>0, "1");
		m_iBufSize = size;
		m_pBuffer = new uint8_t[m_iBufSize];

		m_iHeadPos = 0;
		m_iTailPos = 0;
	}

	virtual ~BB_CircularBuffer()
	{
		__ASSERT(m_pBuffer,"1");
		if (m_pBuffer)
			delete[] m_pBuffer, m_pBuffer = nullptr;
	}

	void	PutData(uint8_t *pData, int len)
	{
		if (len<=0)
		{
	#ifdef _DEBUG
			//OutputDebugString("BB_CircularBuffer::PutData len is <= 0\n");
	#endif
			return;
		}
		while (IsOverFlowCondition(len)) BufferResize();
		if (IsIndexOverFlow(len))
		{
			int FirstCopyLen = m_iBufSize-m_iTailPos;
			int SecondCopyLen = len - FirstCopyLen;
			__ASSERT(FirstCopyLen, "1");
			CopyMemory(m_pBuffer+m_iTailPos, pData, FirstCopyLen);
			if (SecondCopyLen)
			{
				CopyMemory(m_pBuffer, pData+FirstCopyLen, SecondCopyLen);
				m_iTailPos = SecondCopyLen;
			}
			else m_iTailPos = 0;
		}
		else
		{
			CopyMemory(m_pBuffer+m_iTailPos, pData, len);
			m_iTailPos += len;
		}
	}
	void	GetData(uint8_t *pData, int len)
	{
		__ASSERT(len>0&&len<=GetValidCount(),"GetData error");
		if (len < m_iBufSize-m_iHeadPos)
			CopyMemory(pData, m_pBuffer+m_iHeadPos, len);
		else
		{
			int fc, sc;
			fc = m_iBufSize-m_iHeadPos;
			sc = len - fc;
			CopyMemory(pData, m_pBuffer+m_iHeadPos, fc);
			if (sc) CopyMemory(pData+fc, m_pBuffer, sc);
		}
	}
	int		GetOutData(uint8_t *pData) //HeadPos, 변화
	{
		int len = GetValidCount();
		int fc, sc;
		fc = m_iBufSize-m_iHeadPos;
		if (len>fc)
		{
			sc = len - fc;
			CopyMemory(pData, m_pBuffer+m_iHeadPos, fc);
			CopyMemory(pData+fc, m_pBuffer, sc);
			m_iHeadPos = sc;
			__ASSERT(m_iHeadPos==m_iTailPos, "1");
		}
		else
		{
			CopyMemory(pData, m_pBuffer+m_iHeadPos, len);
			m_iHeadPos += len;
			if (m_iHeadPos==m_iBufSize) m_iHeadPos = 0;
		}
		return len;
	}
	void	PutData(uint8_t& data)
	{
		int len = 1;
		while (IsOverFlowCondition(len)) BufferResize();
		m_pBuffer[m_iTailPos++] = data;
		if (m_iTailPos==m_iBufSize) m_iTailPos = 0;
	}
	uint8_t&	GetHeadData(){return m_pBuffer[m_iHeadPos];}
	//1 Byte Operation;
	//false : 모든데이터 다빠짐, TRUE: 정상적으로 진행중
	BOOL	HeadIncrease(int increasement=1)
	{
		__ASSERT(increasement<=GetValidCount(),"1");
		m_iHeadPos += increasement;
		m_iHeadPos %= m_iBufSize;
		return m_iHeadPos!=m_iTailPos;
	}
	void	SetEmpty() {m_iHeadPos=0; m_iTailPos=0;}

	int&	GetBufferSize() {return m_iBufSize;}
	int&	GetHeadPos() {return m_iHeadPos;}
	int&	GetTailPos() {return m_iTailPos;}
	int		GetValidCount()
	{
		int count = m_iTailPos-m_iHeadPos;
		if (count<0) count = m_iBufSize+count;
		return count;
	}
protected:
	//over flow 먼저 점검한 후 IndexOverFlow 점검
	BOOL	IsOverFlowCondition(int &len) {return (len >= m_iBufSize-GetValidCount()) ? TRUE: FALSE;}
	BOOL	IsIndexOverFlow(int &len) {return (len+m_iTailPos>=m_iBufSize) ? TRUE:FALSE;}
	void	BufferResize() //overflow condition 일때 size를 현재의 두배로 늘림
	{
		int prevBufSize = m_iBufSize;
		m_iBufSize <<= 1;
		uint8_t *pNewData = new uint8_t[m_iBufSize];
		CopyMemory(pNewData, m_pBuffer, prevBufSize);
		if (m_iTailPos<m_iHeadPos) 
		{
			CopyMemory(pNewData+prevBufSize, m_pBuffer, m_iTailPos);
			m_iTailPos += prevBufSize;
		}
		delete [] m_pBuffer;
		m_pBuffer = pNewData;
	}
protected:
	int		m_iBufSize;
	uint8_t	*m_pBuffer;

	int		m_iHeadPos;
	int		m_iTailPos;
};

class DataPack
{
public:
	int		m_Size;
	uint8_t*	m_pData;

public:
	DataPack()
	{
		m_Size = 0;
		m_pData = nullptr;
	}

	DataPack(int size, uint8_t *pData)
	{
		__ASSERT(size, "size is 0");		
		m_Size = size;
		m_pData = new uint8_t[size];
		memcpy(m_pData, pData, size);
	}

	virtual ~DataPack()
	{
		delete[] m_pData;
	}
};

#ifdef _DEBUG
struct __SocketStatisics
{
	uint32_t dwTime;
	int iSize;
};
#include <vector>
#include <string>
#endif

class CAPISocket  
{
protected:
	void *	m_hSocket;
	
	HWND		m_hWndTarget;
	std::string m_szIP;
	uint32_t		m_dwPort;

	uint8_t	m_RecvBuf[RECEIVE_BUF_SIZE];
	BOOL	m_bConnected;

	BB_CircularBuffer	m_CB;

#ifdef _DEBUG
	__SocketStatisics m_Statistics_Send_Sum[256];
	__SocketStatisics m_Statistics_Recv_Sum[256];
#endif

public:
	static int			s_nInstanceCount;
	
	int					m_iSendByteCount;
	std::queue<Packet *> m_qRecvPkt;

	BOOL	m_bEnableSend; // 보내기 가능..?
public:
	int		Connect(HWND hWnd, const char* pszIP, uint32_t port);
	void	Disconnect();
	BOOL	IsConnected() { return m_bConnected; }
	int		ReConnect();

	std::string GetCurrentIP() { return m_szIP; }
	uint32_t		GetCurrentPort() { return m_dwPort; }

	void	Release();	
	void	Receive();
	BOOL	ReceiveProcess();
	void	Send(uint8_t* pData, int nSize);

#ifdef _CRYPTION
protected:
	static BOOL			s_bCryptionFlag;			//0 : 비암호화 , 1 : 암호화
	static CJvCryption	s_JvCrypt;
	static uint32_t		s_wSendVal;
	static uint32_t		s_wRcvVal;
public:
	static void			InitCrypt(int64_t PublicKey)
	{
		s_JvCrypt.SetPublicKey(PublicKey);
		s_JvCrypt.Init();

		s_wSendVal = 0;
		s_wRcvVal = 0;
		if (0 != PublicKey) s_bCryptionFlag = TRUE;
		else s_bCryptionFlag = FALSE;
	}
#endif

	//패킷 만들기 함수
	static void MP_AddByte(uint8_t* dest, int& iOffset, uint8_t byte)
	{
		memcpy(dest + iOffset, &byte, 1);
		++iOffset;
	}

	static void MP_AddShort(uint8_t* dest, int& iOffset, int16_t value)
	{
		memcpy(dest + iOffset, &value, 2);
		iOffset += 2;
	}

	static void MP_AddWord(uint8_t* dest, int& offset, uint16_t value)
	{
		memcpy(dest + offset, &value, 2);
		offset += 2;
	}

	static void MP_AddDword(uint8_t* dest, int& iOffset, uint32_t dword)
	{
		memcpy(dest + iOffset, &dword, 4);
		iOffset += 4;
	}

	static void MP_AddFloat(uint8_t* dest, int& iOffset, float value)
	{
		memcpy(dest + iOffset, &value, 4);
		iOffset += 4;
	}

	static void MP_AddString(uint8_t* dest, int& iOffset, const std::string& szString)
	{
		if (szString.empty())
			return;

		memcpy(dest + iOffset, &szString[0], szString.size());
		iOffset += static_cast<int>(szString.size());
	}

	CAPISocket();
	virtual ~CAPISocket();
};

#endif // !defined(AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_)
