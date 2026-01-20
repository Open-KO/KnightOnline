#include "pch.h"
#include "VersionManagerApp.h"
#include "User.h"

#include <shared/packets.h>

#include <set>

namespace VersionManager
{

CUser::CUser(test_tag tag) : TcpServerSocket(tag)
{
}

CUser::CUser(TcpServerSocketManager* socketManager) : TcpServerSocket(socketManager)
{
}

std::string_view CUser::GetImplName() const
{
	return "User";
}

bool CUser::PullOutCore(char*& data, int& length)
{
	MYSHORT slen;

	int len = _recvCircularBuffer.GetValidCount();
	if (len <= 0)
		return false;

	std::vector<uint8_t> tmpBuffer(len);
	_recvCircularBuffer.GetData(reinterpret_cast<char*>(tmpBuffer.data()), len);

	int sPos = 0, ePos = 0;
	for (int i = 0; i < len; i++)
	{
		if (i + 2 >= len)
			break;

		if (tmpBuffer[i] == PACKET_START1 && tmpBuffer[i + 1] == PACKET_START2)
		{
			sPos      = i + 2;

			slen.b[0] = tmpBuffer[sPos];
			slen.b[1] = tmpBuffer[sPos + 1];

			length    = slen.i;

			if (length < 0)
				return false;

			if (length > len)
				return false;

			ePos = sPos + length + 2;

			if ((ePos + 2) > len)
				return false;

			if (tmpBuffer[ePos] != PACKET_END1 || tmpBuffer[ePos + 1] != PACKET_END2)
			{
				_recvCircularBuffer.HeadIncrease(3);
				break;
			}

			data = new char[length];
			memcpy(data, &tmpBuffer[sPos + 2], length);
			_recvCircularBuffer.HeadIncrease(6 + length); // 6: header 2+ end 2+ length 2
			return true;
		}
	}

	return false;
}

int CUser::Send(char* pBuf, int length)
{
	constexpr int PacketHeaderSize = 6;

	assert(length >= 0);
	assert((length + PacketHeaderSize) <= MAX_PACKET_SIZE);

	if (length < 0 || (length + PacketHeaderSize) > MAX_PACKET_SIZE)
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
	int index = 0, sendIndex = 0, client_version = 0;
	char buff[2048] {};
	uint8_t command = GetByte(pData, index);

	switch (command)
	{
		case LS_VERSION_REQ:
		{
			VersionManagerApp* appInstance = VersionManagerApp::instance();

			SetByte(buff, LS_VERSION_REQ, sendIndex);
			SetShort(buff, appInstance->LastVersion(), sendIndex);
			Send(buff, sendIndex);
		}
		break;

		case LS_SERVERLIST:
		{
			VersionManagerApp* appInstance = VersionManagerApp::instance();

			// 기범이가 ^^;
			appInstance->DbProcess.LoadUserCountList();

			SetByte(buff, LS_SERVERLIST, sendIndex);
			SetByte(buff, static_cast<uint8_t>(appInstance->ServerList.size()), sendIndex);

			for (const _SERVER_INFO* pInfo : appInstance->ServerList)
			{
				SetString2(buff, pInfo->strServerIP, sendIndex);
				SetString2(buff, pInfo->strServerName, sendIndex);

				if (pInfo->sUserCount <= pInfo->sUserLimit)
					SetShort(buff, pInfo->sUserCount, sendIndex); // 기범이가 ^^;
				else
					SetShort(buff, -1, sendIndex);
			}

			Send(buff, sendIndex);
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

		default:
			spdlog::error("User::Parsing: Unhandled opcode {:02X} [ip={}]", command, GetRemoteIP());
			break;
	}
}

void CUser::LogInReq(char* pBuf)
{
	VersionManagerApp* appInstance = VersionManagerApp::instance();
	int index = 0, idlen = 0, pwdlen = 0, sendIndex = 0, result = 0, serverno = 0;
	int16_t sPremiumTimeDaysRemaining = -1;
	char sendBuffer[256] {}, accountid[MAX_ID_SIZE + 1] {}, pwd[MAX_PW_SIZE + 1] {};
	std::string serverIp;

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE || idlen <= 0)
	{
		SendAuthNotFound();
		return;
	}

	GetString(accountid, pBuf, idlen, index);

	pwdlen = GetShort(pBuf, index);
	if (pwdlen > MAX_PW_SIZE || pwdlen < 0)
	{
		SendAuthNotFound();
		return;
	}

	GetString(pwd, pBuf, pwdlen, index);

	result = appInstance->DbProcess.AccountLogin(accountid, pwd);
	SetByte(sendBuffer, LS_LOGIN_REQ, sendIndex);

	if (result != AUTH_OK)
	{
		SetByte(sendBuffer, result, sendIndex);
		Send(sendBuffer, sendIndex);
		return;
	}

	bool bCurrentuser = appInstance->DbProcess.IsCurrentUser(accountid, serverIp, serverno);
	if (bCurrentuser)
	{
		// Kick out
		SetByte(sendBuffer, AUTH_IN_GAME, sendIndex);
		SetString2(sendBuffer, serverIp, sendIndex);
		SetShort(sendBuffer, serverno, sendIndex);
	}
	else
	{
		SetByte(sendBuffer, AUTH_OK, sendIndex);

		if (!appInstance->DbProcess.LoadPremiumServiceUser(accountid, &sPremiumTimeDaysRemaining))
			sPremiumTimeDaysRemaining = -1;

		SetShort(sendBuffer, sPremiumTimeDaysRemaining, sendIndex);
	}

	Send(sendBuffer, sendIndex);
}

void CUser::SendAuthNotFound()
{
	char sendBuffer[4] {};
	int sendIndex = 0;
	SetByte(sendBuffer, LS_LOGIN_REQ, sendIndex);
	SetByte(sendBuffer, AUTH_NOT_FOUND, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::SendDownloadInfo(int version)
{
	int sendIndex = 0;
	std::set<std::string> downloadset;
	char buff[2048];
	VersionManagerApp* appInstance = VersionManagerApp::instance();

	for (const auto& [_, pInfo] : appInstance->VersionList)
	{
		if (pInfo->Number > version)
			downloadset.insert(pInfo->CompressName);
	}

	SetByte(buff, LS_DOWNLOADINFO_REQ, sendIndex);

	SetString2(buff, appInstance->FtpUrl(), sendIndex);
	SetString2(buff, appInstance->FtpPath(), sendIndex);
	SetShort(buff, static_cast<int>(downloadset.size()), sendIndex);

	for (const std::string& filename : downloadset)
		SetString2(buff, filename.data(), sendIndex);

	Send(buff, sendIndex);
}

void CUser::NewsReq()
{
	// this isn't really used, but it's always set to this
	constexpr std::string_view szHeader = "Login Notice";

	// unofficial but when used, will essentially cause it to skip since it's not formatted.
	constexpr std::string_view szEmpty  = "<empty>";

	char sendBuffer[8192];
	int sendIndex                  = 0;
	VersionManagerApp* appInstance = VersionManagerApp::instance();

	SetByte(sendBuffer, LS_NEWS, sendIndex);
	SetString2(sendBuffer, szHeader, sendIndex);

	const _NEWS& news = appInstance->News;
	if (news.Size > 0)
		SetString2(sendBuffer, news.Content, news.Size, sendIndex);
	else
		SetString2(sendBuffer, szEmpty, sendIndex);

	Send(sendBuffer, sendIndex);
}

} // namespace VersionManager
