// User.cpp: implementation of the CUser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "versionmanager.h"
#include "versionmanagerdlg.h"
#include "User.h"

#include <set>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUser::CUser()
{
}

CUser::~CUser()
{
}

void CUser::Initialize()
{
	m_pMain = (CVersionManagerDlg*) AfxGetMainWnd();

	CIOCPSocket2::Initialize();
}

void CUser::CloseProcess()
{
	CIOCPSocket2::CloseProcess();
}

void CUser::Parsing(int len, char* pData)
{
	int index = 0, send_index = 0, client_version = 0;
	char buff[2048] = {};
	BYTE command = GetByte(pData, index);

	switch (command)
	{
		case LS_VERSION_REQ:
			SetByte(buff, LS_VERSION_REQ, send_index);
			SetShort(buff, m_pMain->m_nLastVersion, send_index);
			Send(buff, send_index);
			break;

		case LS_SERVERLIST:
			// 기범이가 ^^;
			m_pMain->m_DBProcess.LoadUserCountList();

			SetByte(buff, LS_SERVERLIST, send_index);
			SetByte(buff, m_pMain->m_nServerCount, send_index);
			for (_SERVER_INFO* pInfo : m_pMain->m_ServerList)
			{
				SetShort(buff, strlen(pInfo->strServerIP), send_index);
				SetString(buff, pInfo->strServerIP, strlen(pInfo->strServerIP), send_index);
				SetShort(buff, strlen(pInfo->strServerName), send_index);
				SetString(buff, pInfo->strServerName, strlen(pInfo->strServerName), send_index);
				SetShort(buff, pInfo->sUserCount, send_index);   // 기범이가 ^^;
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

		case LS_MGAME_LOGIN:
			MgameLogin(pData + index);
			break;
	}
}

void CUser::LogInReq(char* pBuf)
{
	int index = 0, idlen = 0, pwdlen = 0, send_index = 0, result = 0, serverno = 0;
	BOOL bCurrentuser = FALSE;
	char send_buff[256] = {},
		serverip[20] = {},
		accountid[MAX_ID_SIZE + 1] = {},
		pwd[13] = {};

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
		goto fail_return;

	GetString(accountid, pBuf, idlen, index);

	pwdlen = GetShort(pBuf, index);
	if (pwdlen > 12
		|| pwdlen < 0)
		goto fail_return;

	GetString(pwd, pBuf, pwdlen, index);

	result = m_pMain->m_DBProcess.AccountLogin(accountid, pwd);
	SetByte(send_buff, LS_LOGIN_REQ, send_index);

	// success 
	if (result == 1)
	{
		bCurrentuser = m_pMain->m_DBProcess.IsCurrentUser(accountid, serverip, serverno);
		if (bCurrentuser)
		{
			result = 0x05;		// Kick out
			SetByte(send_buff, result, send_index);
			SetShort(send_buff, strlen(serverip), send_index);
			SetString(send_buff, serverip, strlen(serverip), send_index);
			SetShort(send_buff, serverno, send_index);
		}
		else
		{
			SetByte(send_buff, result, send_index);
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
	SetByte(send_buff, 0x02, send_index);				// id, pwd 이상...
	Send(send_buff, send_index);
}

void CUser::MgameLogin(char* pBuf)
{
	int index = 0, idlen = 0, pwdlen = 0, send_index = 0, result = 0;
	char send_buff[256] = {};
	char accountid[MAX_ID_SIZE + 1] = {},
		pwd[13] = {};

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
		goto fail_return;

	GetString(accountid, pBuf, idlen, index);
	pwdlen = GetShort(pBuf, index);
	if (pwdlen > 12)
		goto fail_return;

	GetString(pwd, pBuf, pwdlen, index);

	result = m_pMain->m_DBProcess.MgameLogin(accountid, pwd);
	SetByte(send_buff, LS_MGAME_LOGIN, send_index);
	SetByte(send_buff, result, send_index);
	Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, LS_MGAME_LOGIN, send_index);
	SetByte(send_buff, 0x02, send_index);				// login fail...
	Send(send_buff, send_index);
}

void CUser::SendDownloadInfo(int version)
{
	int send_index = 0;
	std::set<std::string> downloadset;
	char buff[2048];

	for (const auto& [_, pInfo] : m_pMain->m_VersionList)
	{
		if (pInfo->sVersion > version)
			downloadset.insert(pInfo->strCompName);
	}

	SetByte(buff, LS_DOWNLOADINFO_REQ, send_index);
	SetShort(buff, strlen(m_pMain->m_strFtpUrl), send_index);
	SetString(buff, m_pMain->m_strFtpUrl, strlen(m_pMain->m_strFtpUrl), send_index);
	SetShort(buff, strlen(m_pMain->m_strFilePath), send_index);
	SetString(buff, m_pMain->m_strFilePath, strlen(m_pMain->m_strFilePath), send_index);
	SetShort(buff, downloadset.size(), send_index);

	for (const std::string& filename : downloadset)
	{
		SetShort(buff, (short) filename.size(), send_index);
		SetString(buff, filename.c_str(), (int) filename.size(), send_index);
	}

	Send(buff, send_index);
}
