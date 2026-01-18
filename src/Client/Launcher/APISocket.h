// APISocket.h: interface for the CAPISocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_)
#define AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_

#pragma once

#include <winsock.h>

#include <queue>
#include <string>

#include "BB_CircularBuffer.h"

#define WM_SOCKETMSG     (WM_USER + 1)
#define RECEIVE_BUF_SIZE 4096

class DataPack
{
public:
	int m_Size;
	BYTE* m_pData;

public:
	DataPack()
	{
		m_Size  = 0;
		m_pData = nullptr;
	}
	DataPack(int size, BYTE* pData)
	{
		ASSERT(size);
		m_Size  = size;
		m_pData = new BYTE[size];
		CopyMemory(m_pData, pData, size);
	}
	virtual ~DataPack()
	{
		delete[] m_pData;
	}
};

class CAPISocket
{
protected:
	SOCKET m_hSocket;
	HWND m_hWndTarget;
	BYTE m_RecvBuf[RECEIVE_BUF_SIZE];

	BB_CircularBuffer m_CB;

public:
	static int s_nInstanceCount;
	static WSADATA s_WSData;

	int m_iSendByteCount;
	std::queue<DataPack*> m_qRecvPkt;

public:
	BOOL Connect(HWND hWnd, const char* pszIP, DWORD port);
	BOOL Disconnect();

	void Release();
	void Receive();
	BOOL ReceiveProcess();
	void Send(BYTE* pData, int nSize);

	//패킷 만들기 함수
	static void MP_AddByte(BYTE* dest, int& iOffset, BYTE byte)
	{
		CopyMemory(dest + iOffset, &byte, 1);
		iOffset++;
	}
	static void MP_AddShort(BYTE* dest, int& iOffset, short value)
	{
		CopyMemory(dest + iOffset, &value, 2);
		iOffset += 2;
	}
	static void MP_AddWord(BYTE* dest, int& offset, WORD value)
	{
		CopyMemory(dest + offset, &value, 2);
		offset += 2;
	}
	static void MP_AddDword(BYTE* dest, int& iOffset, DWORD dword)
	{
		CopyMemory(dest + iOffset, &dword, 4);
		iOffset += 4;
	}
	static void MP_AddFloat(BYTE* dest, int& iOffset, float value)
	{
		CopyMemory(dest + iOffset, &value, 4);
		iOffset += 4;
	}
	//	static	void	MP_AddString(BYTE *dest, int &iOffset, BYTE *sor, int len) { CopyMemory(dest+iOffset, sor, len); iOffset += len; }
	static void MP_AddString(BYTE* dest, int& iOffset, const std::string& szString)
	{
		if (!szString.empty())
		{
			CopyMemory(dest + iOffset, &szString[0], szString.size());
			iOffset += static_cast<int>(szString.size());
		}
	}

	//패킷 Parsing 함수
	static BYTE& Parse_GetByte(const BYTE* buf, int& iOffset)
	{
		iOffset++;
		return *(BYTE*) (buf + iOffset - 1);
	}
	static short& Parse_GetShort(const BYTE* buf, int& iOffset)
	{
		iOffset += 2;
		return *(short*) (buf + iOffset - 2);
	}
	static WORD& Parse_GetWord(const BYTE* buf, int& iOffset)
	{
		iOffset += 2;
		return *(PWORD) (buf + iOffset - 2);
	}
	static DWORD& Parse_GetDword(const BYTE* buf, int& iOffset)
	{
		iOffset += 4;
		return *(DWORD*) (buf + iOffset - 4);
	}
	static float& Parse_GetFloat(const BYTE* buf, int& iOffset)
	{
		iOffset += 4;
		return *(float*) (buf + iOffset - 4);
	}
	//	static	void	Parse_GetString(const BYTE* buf, int &iOffset, char *dest, int len) { if (len) { CopyMemory(dest, buf+iOffset, len); iOffset += len; } dest[len] = 0; }
	static void Parse_GetString(const BYTE* buf, int& iOffset, std::string& szString, int len)
	{
		if (len > 0)
		{
			szString.assign(len, ' ');
			memcpy(&(szString[0]), buf + iOffset, len);
			iOffset += len;
		}
		else
		{
			szString = "";
		}
	}
	//static	DWORD&	Parse_GetDword(char *buf, int &iOffset);

	CAPISocket();
	virtual ~CAPISocket();
};

#endif // !defined(AFX_APISOCKET_H__31D58152_3B8D_4CBD_BEB9_6BE23C4F0FFB__INCLUDED_)
