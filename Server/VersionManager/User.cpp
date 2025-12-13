#include "pch.h"
#include "VersionManagerApp.h"
#include "User.h"

#include <shared/packets.h>

#include <set>
#include <vector>
#include <memory>

CUser::CUser(SocketManager* socketManager)
	: TcpServerSocket(socketManager)
{
}

bool CUser::PullOutCore(char*& data, int& length)
{
	int len = _recvCircularBuffer.GetValidCount();

	if (len <= 0)
		return false;

	// Use RAII to ensure automatic cleanup even if exceptions occur
	std::vector<uint8_t> tmpBuffer(len);

	_recvCircularBuffer.GetData(reinterpret_cast<char*>(tmpBuffer.data()), len);

	bool foundCore = false;
	int sPos = 0;
	int ePos = 0;
	MYSHORT slen;

	for (int i = 0; i < len && !foundCore; i++)
	{
		if (i + 2 >= len)
			break;

		if (tmpBuffer[i] == PACKET_START1
			&& tmpBuffer[i + 1] == PACKET_START2)
		{
			sPos = i + 2;

			slen.b[0] = tmpBuffer[sPos];
			slen.b[1] = tmpBuffer[sPos + 1];

			length = slen.i;

			// Validate packet length before proceeding
			if (length < 0 || length > len)
			{
				// Invalid packet length, skip this start marker
				_recvCircularBuffer.HeadIncrease(3);
				break;
			}

			ePos = sPos + length + 2;

			if ((ePos + 2) > len)
			{
				// Packet not complete yet, wait for more data
				_recvCircularBuffer.HeadIncrease(3);
				break;
			}

			if (tmpBuffer[ePos] == PACKET_END1
				&& tmpBuffer[ePos + 1] == PACKET_END2)
			{
				// Allocate memory for the packet data
				// If new throws std::bad_alloc, tmpBuffer will be automatically cleaned up (RAII)
				data = new (std::nothrow) char[length + 1];
				if (data == nullptr)
				{
					// Memory allocation failed
					return false;
				}

				memcpy(data, tmpBuffer.data() + sPos + 2, length);
				data[length] = 0;
				foundCore = true;
				break;
			}
			else
			{
				// Invalid packet end marker, skip this start marker
				_recvCircularBuffer.HeadIncrease(3);
				break;
			}
		}
	}

	if (foundCore)
		_recvCircularBuffer.HeadIncrease(6 + length); // 6: header 2+ end 2+ length 2

	// tmpBuffer is automatically cleaned up here (RAII)
	return foundCore;
}

int CUser::Send(char* pBuf, int length)
{
	constexpr int PacketHeaderSize = 6;

	assert(length >= 0);
	assert((length + PacketHeaderSize) <= MAX_PACKET_SIZE);

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

void CUser::Parsing(int /*len*/, char* pData)
{
	int index = 0, send_index = 0, client_version = 0;
	char buff[2048] = {};
	uint8_t command = GetByte(pData, index);

	switch (command)
	{
		case LS_VERSION_REQ:
		{
			VersionManagerApp* appInstance = VersionManagerApp::instance();

			SetByte(buff, LS_VERSION_REQ, send_index);
			SetShort(buff, appInstance->LastVersion(), send_index);
			Send(buff, send_index);
		}
		break;

		case LS_SERVERLIST:
		{
			VersionManagerApp* appInstance = VersionManagerApp::instance();

			// 기범이가 ^^;
			appInstance->DbProcess.LoadUserCountList();

			SetByte(buff, LS_SERVERLIST, send_index);
			SetByte(buff, static_cast<uint8_t>(appInstance->ServerList.size()), send_index);

			for (const _SERVER_INFO* pInfo : appInstance->ServerList)
			{
				SetString2(buff, pInfo->strServerIP, send_index);
				SetString2(buff, pInfo->strServerName, send_index);

				if (pInfo->sUserCount <= pInfo->sUserLimit)
					SetShort(buff, pInfo->sUserCount, send_index);   // 기범이가 ^^;
				else
					SetShort(buff, -1, send_index);
			}

			Send(buff, send_index);
		}
		break;

		case LS_DOWNLOADINFO_REQ:
			client_version = GetShort(pData, index);
			SendDownloadInfo(client_version);
			break;

		case LS_LOGIN_REQ:
			LogInReq(pData + index);
			break;

		case LS_NEWS:
			NewsReq();
			break;
	}
}

void CUser::LogInReq(char* pBuf)
{
	int index = 0, idlen = 0, pwdlen = 0, send_index = 0, result = 0, serverno = 0;
	bool bCurrentuser = false;
	char send_buff[256] = {},
		accountid[MAX_ID_SIZE + 1] = {},
		pwd[MAX_PW_SIZE + 1] = {};
	std::string serverIp;
	int16_t sPremiumTimeDaysRemaining = -1;
	VersionManagerApp* appInstance = VersionManagerApp::instance();

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

	result = appInstance->DbProcess.AccountLogin(accountid, pwd);
	SetByte(send_buff, LS_LOGIN_REQ, send_index);

	if (result == AUTH_OK)
	{
		bCurrentuser = appInstance->DbProcess.IsCurrentUser(accountid, serverIp, serverno);
		if (bCurrentuser)
		{
			// Kick out
			result = AUTH_IN_GAME;

			SetByte(send_buff, result, send_index);
			SetString2(send_buff, serverIp, send_index);
			SetShort(send_buff, serverno, send_index);
		}
		else
		{
			SetByte(send_buff, result, send_index);

			if (!appInstance->DbProcess.LoadPremiumServiceUser(accountid, &sPremiumTimeDaysRemaining))
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
	VersionManagerApp* appInstance = VersionManagerApp::instance();

	for (const auto& [_, pInfo] : appInstance->VersionList)
	{
		if (pInfo->Number > version)
			downloadset.insert(pInfo->CompressName);
	}

	SetByte(buff, LS_DOWNLOADINFO_REQ, send_index);

	SetString2(buff, appInstance->FtpUrl(), send_index);
	SetString2(buff, appInstance->FtpPath(), send_index);
	SetShort(buff, static_cast<int>(downloadset.size()), send_index);

	for (const std::string& filename : downloadset)
		SetString2(buff, filename.data(), send_index);

	Send(buff, send_index);
}

void CUser::NewsReq()
{
	constexpr char szHeader[] = "Login Notice";	// this isn't really used, but it's always set to this
	constexpr char szEmpty[] = "<empty>";		// unofficial but when used, will essentially cause it to skip since it's not formatted.

	char send_buff[8192];
	int send_index = 0;
	VersionManagerApp* appInstance = VersionManagerApp::instance();

	SetByte(send_buff, LS_NEWS, send_index);
	SetString2(send_buff, szHeader, sizeof(szHeader) - 1, send_index);

	const _NEWS& news = appInstance->News;
	if (news.Size > 0)
		SetString2(send_buff, news.Content, news.Size, send_index);
	else
		SetString2(send_buff, szEmpty, sizeof(szEmpty) - 1, send_index);

	Send(send_buff, send_index);
}
