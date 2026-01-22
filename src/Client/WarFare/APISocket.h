// APISocket.h: interface for the CAPISocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_)
#define AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_

#pragma once

#include "GameDef.h"
#include "PacketDef.h"

#include <N3Base/N3Base.h>
#include <shared/ExpandableCircularBuffer.h>

#define _CRYPTION // 암호화 사용
#ifdef _CRYPTION
#include <shared/JvCryption.h>
#endif

#include <queue>
#include <string>
#include <vector>

inline constexpr int WM_SOCKETMSG  = (WM_USER + 1);
inline constexpr int SEND_BUF_SIZE = 262144; // 최대 버퍼..
inline constexpr int RECV_BUF_SIZE = 262144; // 최대 버퍼..

class DataPack
{
public:
	int m_Size;
	uint8_t* m_pData;

public:
	DataPack()
	{
		m_Size  = 0;
		m_pData = nullptr;
	}

	DataPack(int size, uint8_t* pData)
	{
		__ASSERT(size, "size is 0");
		m_Size  = size;
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
	uint32_t dwTime = 0;
	int iSize       = 0;
};
#endif

class CAPISocket
{
protected:
	SOCKET m_hSocket;

	HWND m_hWndTarget;
	std::string m_szIP;
	uint32_t m_dwPort;

	std::vector<char> m_SendBuf;
	std::vector<char> m_RecvBuf;

	BOOL m_bConnected;

	ExpandableCircularBuffer m_CB;

#ifdef _DEBUG
	__SocketStatisics m_Statistics_Send_Sum[256] = {};
	__SocketStatisics m_Statistics_Recv_Sum[256] = {};
#endif

public:
	static int s_nInstanceCount;

	int m_iSendByteCount;
	std::queue<Packet*> m_qRecvPkt;

	BOOL m_bEnableSend; // 보내기 가능..?

public:
	int Connect(HWND hWnd, const std::string& szIP, uint32_t port);
	void Disconnect();
	BOOL IsConnected()
	{
		return m_bConnected;
	}
	int ReConnect();

	std::string GetCurrentIP()
	{
		return m_szIP;
	}
	uint32_t GetCurrentPort()
	{
		return m_dwPort;
	}

	void Release();
	void Receive();
	BOOL ReceiveProcess();
	void Send(uint8_t* pData, int nSize);

#ifdef _CRYPTION
protected:
	static BOOL s_bCryptionFlag; //0 : 비암호화 , 1 : 암호화
	static CJvCryption s_JvCrypt;
	static uint32_t s_wSendVal;
	static uint32_t s_wRcvVal;

public:
	static void InitCrypt(int64_t PublicKey)
	{
		s_JvCrypt.SetPublicKey(PublicKey);
		s_JvCrypt.Init();

		s_wSendVal = 0;
		s_wRcvVal  = 0;
		if (0 != PublicKey)
			s_bCryptionFlag = TRUE;
		else
			s_bCryptionFlag = FALSE;
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
