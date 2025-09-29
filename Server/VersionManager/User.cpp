// User.cpp: implementation of the CUser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VersionManager.h"
#include "VersionManagerdlg.h"
#include "User.h"

#include <shared/packets.h>

#include <set>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUser::CUser(CVersionManagerDlg* main, SocketManager* socketManager)
	: TcpServerSocket(socketManager), _main(main)
{
}

bool CUser::PullOutCore(char*& data, int& length)
{
	uint8_t*	pTmp;
	int			len;
	bool		foundCore;
	MYSHORT		slen;

	len = _recvCircularBuffer.GetValidCount();

	if (len == 0
		|| len < 0)
		return false;

	pTmp = new uint8_t[len];

	_recvCircularBuffer.GetData((char*) pTmp, len);

	foundCore = false;

	int	sPos = 0, ePos = 0;

	for (int i = 0; i < len && !foundCore; i++)
	{
		if (i + 2 >= len)
			break;

		if (pTmp[i] == PACKET_START1
			&& pTmp[i + 1] == PACKET_START2)
		{
			sPos = i + 2;

			slen.b[0] = pTmp[sPos];
			slen.b[1] = pTmp[sPos + 1];

			length = slen.i;

			if (length < 0)
				goto cancelRoutine;

			if (length > len)
				goto cancelRoutine;

			ePos = sPos + length + 2;

			if ((ePos + 2) > len)
				goto cancelRoutine;
//			ASSERT(ePos+2 <= len);

			if (pTmp[ePos] == PACKET_END1
				&& pTmp[ePos + 1] == PACKET_END2)
			{
				data = new char[length + 1];
				memcpy(data, (pTmp + sPos + 2), length);
				data[length] = 0;
				foundCore = true;
//				int head = _recvCircularBuffer.GetHeadPos(), tail = _recvCircularBuffer.GetTailPos();
//				TRACE("data : %s, len : %d\n", data, length);
//				TRACE("head : %d, tail : %d\n", head, tail );
				break;
			}
			else
			{
				_recvCircularBuffer.HeadIncrease(3);
				break;
			}
		}
	}

	if (foundCore)
		_recvCircularBuffer.HeadIncrease(6 + length); // 6: header 2+ end 2+ length 2

cancelRoutine:
	delete[] pTmp;
	return foundCore;
}

int CUser::Send(char* pBuf, int length)
{
	constexpr int PacketHeaderSize = 6;

	_ASSERT(length >= 0);
	_ASSERT((length + PacketHeaderSize) <= MAX_PACKET_SIZE);

	if (length < 0
		|| (length + PacketHeaderSize) > MAX_PACKET_SIZE)
		return -1;

	char sendBuffer[MAX_PACKET_SIZE];
	int index = 0;
	SetByte(sendBuffer, PACKET_START1, index);
	SetByte(sendBuffer, PACKET_START2, index);
	SetShort(sendBuffer, length, index);
	SetString(sendBuffer, pBuf, length, index);
	SetByte(sendBuffer, PACKET_END1, index);
	SetByte(sendBuffer, PACKET_END2, index);
	return QueueAndSend(sendBuffer, index);
}

void CUser::Parsing(int len, char* pData)
{
	int index = 0, send_index = 0, client_version = 0;
	char buff[2048] = {};
	uint8_t command = GetByte(pData, index);

	switch (command)
	{
		case LS_VERSION_REQ:
			SetByte(buff, LS_VERSION_REQ, send_index);
			SetShort(buff, _main->LastVersion(), send_index);
			Send(buff, send_index);
			break;

		case LS_SERVERLIST:
			// 기범이가 ^^;
			_main->DbProcess.LoadUserCountList();

			SetByte(buff, LS_SERVERLIST, send_index);
			SetByte(buff, static_cast<uint8_t>(_main->ServerList.size()), send_index);

			for (const _SERVER_INFO* pInfo : _main->ServerList)
			{
				SetString2(buff, pInfo->strServerIP, (int16_t) strlen(pInfo->strServerIP), send_index);
				SetString2(buff, pInfo->strServerName, (int16_t) strlen(pInfo->strServerName), send_index);

				if (pInfo->sUserCount <= pInfo->sUserLimit)
					SetShort(buff, pInfo->sUserCount, send_index);   // 기범이가 ^^;
				else
					SetShort(buff, -1, send_index);
			}

			Send(buff, send_index);
			break;

		case LS_DOWNLOADINFO_REQ:
			client_version = GetShort(pData, index);
			SendDownloadInfo(client_version);
			break;

		case LS_LOGIN_REQ:
			LogInReq(pData + index);
			break;

		case LS_NEWS:
			NewsReq(pData + index);
			break;
	}
}

void CUser::LogInReq(char* pBuf)
{
	int index = 0, idlen = 0, pwdlen = 0, send_index = 0, result = 0, serverno = 0;
	bool bCurrentuser = false;
	char send_buff[256] = {},
		serverip[MAX_IP_SIZE + 1] = {},
		accountid[MAX_ID_SIZE + 1] = {},
		pwd[MAX_PW_SIZE + 1] = {};
	int16_t sPremiumTimeDaysRemaining = -1;

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
		goto fail_return;

	GetString(accountid, pBuf, idlen, index);

	pwdlen = GetShort(pBuf, index);
	if (pwdlen > MAX_PW_SIZE
		|| pwdlen < 0)
		goto fail_return;

	GetString(pwd, pBuf, pwdlen, index);

	result = _main->DbProcess.AccountLogin(accountid, pwd);
	SetByte(send_buff, LS_LOGIN_REQ, send_index);

	if (result == AUTH_OK)
	{
		bCurrentuser = _main->DbProcess.IsCurrentUser(accountid, serverip, serverno);
		if (bCurrentuser)
		{
			// Kick out
			result = AUTH_IN_GAME;

			SetByte(send_buff, result, send_index);
			SetString2(send_buff, serverip, (int16_t) strlen(serverip), send_index);
			SetShort(send_buff, serverno, send_index);
		}
		else
		{
			SetByte(send_buff, result, send_index);

			if (!_main->DbProcess.LoadPremiumServiceUser(accountid, &sPremiumTimeDaysRemaining))
				sPremiumTimeDaysRemaining = -1;

			SetShort(send_buff, sPremiumTimeDaysRemaining, send_index);
		}
	}
	else
	{
		SetByte(send_buff, result, send_index);
	}

	Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, LS_LOGIN_REQ, send_index);
	SetByte(send_buff, AUTH_NOT_FOUND, send_index);				// id, pwd 이상...
	Send(send_buff, send_index);
}

void CUser::SendDownloadInfo(int version)
{
	int send_index = 0;
	std::set<std::string> downloadset;
	char buff[2048];

	for (const auto& [_, pInfo] : _main->VersionList)
	{
		if (pInfo->Number > version)
			downloadset.insert(pInfo->CompressName);
	}

	SetByte(buff, LS_DOWNLOADINFO_REQ, send_index);

	SetString2(buff, _main->FtpUrl(), (int16_t) strlen(_main->FtpUrl()), send_index);
	SetString2(buff, _main->FtpPath(), (int16_t) strlen(_main->FtpPath()), send_index);
	SetShort(buff, static_cast<int>(downloadset.size()), send_index);

	for (const std::string& filename : downloadset)
		SetString2(buff, filename.c_str(), (int16_t) filename.size(), send_index);

	Send(buff, send_index);
}

void CUser::NewsReq(char* pBuf)
{
	char send_buff[8192];
	int send_index = 0;

	const char szHeader[]	= "Login Notice";	// this isn't really used, but it's always set to this
	const char szEmpty[]	= "<empty>";		// unofficial but when used, will essentially cause it to skip since it's not formatted.

	SetByte(send_buff, LS_NEWS, send_index);
	SetString2(send_buff, szHeader, _countof(szHeader) - 1, send_index);

	const _NEWS& news = _main->News;
	if (news.Size > 0)
		SetString2(send_buff, news.Content, news.Size, send_index);
	else
		SetString2(send_buff, szEmpty, _countof(szEmpty) - 1, send_index);

	Send(send_buff, send_index);
}
