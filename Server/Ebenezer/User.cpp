// User.cpp: implementation of the CUser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ebenezer.h"
#include "EbenezerDlg.h"
#include "Map.h"
#include "OperationMessage.h"
#include "User.h"
#include "db_resources.h"

#include <shared/globals.h>
#include <shared/packets.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern CRITICAL_SECTION g_region_critical;
extern CRITICAL_SECTION g_LogFile_critical;
extern bool g_serverdown_flag;

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
	m_pMain = (CEbenezerDlg*) AfxGetApp()->GetMainWnd();

	// Cryption
	jct.GenerateKey();
	///~

	m_MagicProcess.m_pMain = m_pMain;
	m_MagicProcess.m_pSrcUser = this;

	m_RegionX = -1;
	m_RegionZ = -1;

	m_sBodyAc = 0;
	m_sTotalHit = 0;
	m_sTotalAc = 0;
	m_sTotalHitrate = 0;
	m_sTotalEvasionrate = 0;

	m_sItemMaxHp = 0;
	m_sItemMaxMp = 0;
	m_iItemWeight = 0;
	m_sItemHit = 0;
	m_sItemAc = 0;
	m_sItemStr = 0;
	m_sItemSta = 0;
	m_sItemDex = 0;
	m_sItemIntel = 0;
	m_sItemCham = 0;
	m_sItemHitrate = 100;
	m_sItemEvasionrate = 100;

	m_sSpeed = 0;

	m_iMaxHp = 0;
	m_iMaxMp = 1;
	m_iMaxExp = 0;
	m_iMaxWeight = 0;

	m_bFireR = 0;
	m_bColdR = 0;
	m_bLightningR = 0;
	m_bMagicR = 0;
	m_bDiseaseR = 0;
	m_bPoisonR = 0;

	m_sDaggerR = 0;
	m_sSwordR = 0;
	m_sAxeR = 0;
	m_sMaceR = 0;
	m_sSpearR = 0;
	m_sBowR = 0;

	m_bMagicTypeLeftHand = 0;		// For weapons and shields with special power.		
	m_bMagicTypeRightHand = 0;
	m_sMagicAmountLeftHand = 0;
	m_sMagicAmountRightHand = 0;

	m_iZoneIndex = 0;
	m_bResHpType = USER_STANDING;
	m_bWarp = 0x00;

	m_sPartyIndex = -1;
	m_sExchangeUser = -1;
	m_bExchangeOK = 0x00;
	m_sPrivateChatUser = -1;
	m_bNeedParty = 0x01;

	m_fHPLastTimeNormal = 0.0f;		// For Automatic HP recovery. 
	m_fHPStartTimeNormal = 0.0f;
	m_bHPAmountNormal = 0;
	m_bHPDurationNormal = 0;
	m_bHPIntervalNormal = 5;

	m_fAreaLastTime = 0.0f;		// For Area Damage spells Type 3.
	m_fAreaStartTime = 0.0f;
	m_bAreaInterval = 5;
	m_iAreaMagicID = 0;

	m_sFriendUser = -1;

	InitType3();	 // Initialize durational type 3 stuff :)
	InitType4();	 // Initialize durational type 4 stuff :)

	m_fSpeedHackClientTime = 0.0f;
	m_fSpeedHackServerTime = 0.0f;
	m_bSpeedHackCheck = 0;

	m_fBlinkStartTime = 0.0f;

	m_sAliveCount = 0;

	m_bAbnormalType = 1;	// User starts out in normal size.

	m_sWhoKilledMe = -1;
	m_iLostExp = 0;

	m_fLastTrapAreaTime = 0.0f;

	memset(m_strAccountID, 0, sizeof(m_strAccountID));
/*
	m_iSelMsgEvent[0] = -1;		// 이밴트 관련 초기화 ^^;
	m_iSelMsgEvent[1] = -1;
	m_iSelMsgEvent[2] = -1;
	m_iSelMsgEvent[3] = -1;
	m_iSelMsgEvent[4] = -1;
*/
	for (int i = 0; i < MAX_MESSAGE_EVENT; i++)
		m_iSelMsgEvent[i] = -1;

	m_sEventNid = -1;

	m_bZoneChangeFlag = false;

	m_bRegeneType = 0;

	m_fLastRegeneTime = 0.0f;

	m_bZoneChangeSameZone = false;

	memset(m_strCouponId, 0, sizeof(m_strCouponId));
	m_iEditBoxEvent = -1;

	for (int j = 0; j < MAX_CURRENT_EVENT; j++)
		m_sEvent[j] = -1;

	while (!m_arUserEvent.empty())
		m_arUserEvent.pop_back();

	m_bIsPartyLeader = false;
	m_byInvisibilityState = 0;
	m_sDirection = 0;
	m_bIsChicken = false;
	m_byKnightsRank = 0;
	m_byPersonalRank = 0;

	CIOCPSocket2::Initialize();
}

void CUser::CloseProcess()
{
	UserInOut(USER_OUT);

	if (m_sPartyIndex != -1)
		PartyRemove(m_Sid);

	if (m_sExchangeUser != -1)
		ExchangeCancel();

/* 부디 잘 작동하길 ㅠ.ㅠ
	if (!m_bZoneChangeFlag) {
		if (m_pUserData->m_bZone == ZONE_BATTLE || (m_pUserData->m_bZone != m_pUserData->m_bNation && m_pUserData->m_bZone < 3) ) {
			model::Home* pHomeInfo = nullptr;	// Send user back home in case it was the battlezone.
			pHomeInfo = m_pMain->m_HomeTableMap.GetData(m_pUserData->m_bNation);
			if (!pHomeInfo) return;

			m_pUserData->m_bZone = m_pUserData->m_bNation;

			if (m_pUserData->m_bNation == KARUS) {
				m_pUserData->m_curx = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
				m_pUserData->m_curz = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
			}
			else {
				m_pUserData->m_curx = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
				m_pUserData->m_curz = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
			}
		}
		TRACE(_T("본국으로 잘 저장되었을거야. 걱정마!!!\r\n"));
	}
*/

	MarketBBSUserDelete();
	LogOut();
	Initialize();
	CIOCPSocket2::CloseProcess();
}

void CUser::Parsing(int len, char* pData)
{
	int index = 0;
	float currenttime;

	BYTE command = GetByte(pData, index);

	spdlog::trace("User::Parsing: userId={} charId={} command={:02X} len={}",
		GetSocketID(), m_pUserData->m_id, command, len);

	switch (command)
	{
		case WIZ_LOGIN:
			LoginProcess(pData + index);
			break;

		case WIZ_SEL_NATION:
			SelNationToAgent(pData + index);
			break;

		case WIZ_NEW_CHAR:
			NewCharToAgent(pData + index);
			break;

		case WIZ_DEL_CHAR:
			DelCharToAgent(pData + index);
			break;

		case WIZ_SEL_CHAR:
			SelCharToAgent(pData + index);
			break;

		case WIZ_GAMESTART:
			if (m_State != STATE_GAMESTART)
				GameStart(pData + index);
			break;

		case WIZ_MOVE:
			MoveProcess(pData + index);
			break;

		case WIZ_ROTATE:
			Rotate(pData + index);
			break;

		case WIZ_ATTACK:
			Attack(pData + index);
			break;

		case WIZ_ALLCHAR_INFO_REQ:
			AllCharInfoToAgent();
			break;

		case WIZ_CHAT:
			Chat(pData + index);
			break;

		case WIZ_CHAT_TARGET:
			ChatTargetSelect(pData + index);
			break;

		case WIZ_REGENE:
			InitType3();	// Init Type 3.....
			InitType4();	// Init Type 4.....
	//		Corpse();
			Regene(pData + index);
	//		InitType3();	// Init Type 3.....
	//		InitType4();	// Init Type 4.....
			break;

		case WIZ_REQ_USERIN:
			RequestUserIn(pData + index);
			break;

		case WIZ_REQ_NPCIN:
			RequestNpcIn(pData + index);
			break;

		case WIZ_WARP:
			if (m_pUserData->m_bAuthority == AUTHORITY_MANAGER)
				Warp(pData + index);
			break;

		case WIZ_ITEM_MOVE:
			ItemMove(pData + index);
			break;

		case WIZ_NPC_EVENT:
			NpcEvent(pData + index);
			break;

		case WIZ_ITEM_TRADE:
			ItemTrade(pData + index);
			break;

		case WIZ_TARGET_HP:
		{
			int uid = GetShort(pData, index);
			BYTE echo = GetByte(pData, index);
			SendTargetHP(echo, uid);
		}
		break;

		case WIZ_BUNDLE_OPEN_REQ:
			BundleOpenReq(pData + index);
			break;

		case WIZ_ITEM_GET:
			ItemGet(pData + index);
			break;

		case WIZ_ZONE_CHANGE:
			RecvZoneChange(pData + index);
			break;

		case WIZ_POINT_CHANGE:
			PointChange(pData + index);
			break;

		case WIZ_STATE_CHANGE:
			StateChange(pData + index);
			break;

		case WIZ_VERSION_CHECK:
			VersionCheck();
			break;

		//case WIZ_SPEEDHACK_USED:
		//	SpeedHackUser();
		//	break;

		case WIZ_PARTY:
			PartyProcess(pData + index);
			break;

		case WIZ_EXCHANGE:
			ExchangeProcess(pData + index);
			break;

		case WIZ_MAGIC_PROCESS:
			m_MagicProcess.MagicPacket(pData + index, len);
			break;

		case WIZ_SKILLPT_CHANGE:
			SkillPointChange(pData + index);
			break;

		case WIZ_OBJECT_EVENT:
			ObjectEvent(pData + index);
			break;

		case WIZ_WEATHER:
		case WIZ_TIME:
			UpdateGameWeather(pData + index, command);
			break;

		case WIZ_CLASS_CHANGE:
			ClassChange(pData + index);
			break;

		case WIZ_CONCURRENTUSER:
			CountConcurrentUser();
			break;

		case WIZ_DATASAVE:
			UserDataSaveToAgent();
			break;

		case WIZ_ITEM_REPAIR:
			ItemRepair(pData + index);
			break;

		case WIZ_KNIGHTS_PROCESS:
			m_pMain->m_KnightsManager.PacketProcess(this, pData + index);
			break;

		case WIZ_ITEM_REMOVE:
			ItemRemove(pData + index);
			break;

		case WIZ_OPERATOR:
			OperatorCommand(pData + index);
			break;

		case WIZ_SPEEDHACK_CHECK:
			SpeedHackTime(pData + index);
			m_sAliveCount = 0;
			break;

		case WIZ_SERVER_CHECK:
			ServerStatusCheck();
			break;

		case WIZ_WAREHOUSE:
			WarehouseProcess(pData + index);
			break;

		case WIZ_REPORT_BUG:
			ReportBug(pData + index);
			break;

		case WIZ_HOME:
			Home();
			break;

		case WIZ_FRIEND_PROCESS:
#if 0 // outdated
			FriendReport(pData + index);
	//		Friend(pData+index);
#endif
			break;

		case WIZ_WARP_LIST:
			SelectWarpList(pData + index);
			break;

		case WIZ_ZONE_CONCURRENT:
			ZoneConCurrentUsers(pData + index);
			break;

		// 인원체크가 끝나고 해당 서버로 가도 좋다는 허락을 받았다
		case WIZ_VIRTUAL_SERVER:
			ServerChangeOk(pData + index);
			break;

		case WIZ_PARTY_BBS:
			PartyBBS(pData + index);
			break;

		case WIZ_MARKET_BBS:
			MarketBBS(pData + index);
			break;

		case WIZ_KICKOUT:
			KickOut(pData + index);
			break;

		case WIZ_CLIENT_EVENT:
			ClientEvent(pData + index);
			break;

		case WIZ_TEST_PACKET:
			TestPacket(pData + index);
			break;

		case WIZ_SELECT_MSG:
			RecvSelectMsg(pData + index);
			break;

		case WIZ_EDIT_BOX:
			RecvEditBox(pData + index);
			break;
	}

	currenttime = TimeGet();

	if (command == WIZ_GAMESTART)
	{
		m_fHPLastTimeNormal = currenttime;

		for (int h = 0; h < MAX_TYPE3_REPEAT; h++)
			m_fHPLastTime[h] = currenttime;
	}

	// For Sitdown/Standup HP restoration.
	if (m_fHPLastTimeNormal != 0.0f
		&& (currenttime - m_fHPLastTimeNormal) > m_bHPIntervalNormal
		&& m_bAbnormalType != ABNORMAL_BLINKING)
		HPTimeChange(currenttime);

	// For Type 3 HP Duration.
	if (m_bType3Flag)
	{
		for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
		{
			if (m_fHPLastTime[i] != 0.0f
				&& (currenttime - m_fHPLastTime[i]) > m_bHPInterval[i])
			{
				HPTimeChangeType3(currenttime);
				break;
			}
		}
	}

	// For Type 4 Stat Duration.
	if (m_bType4Flag)
		Type4Duration(currenttime);

	// Should you stop blinking?
	if (m_bAbnormalType == ABNORMAL_BLINKING)
		BlinkTimeCheck(currenttime);
}

void CUser::VersionCheck()
{
	int index = 0, send_index = 0;
	char send_buff[128] = {};

	SetByte(send_buff, WIZ_VERSION_CHECK, send_index);
	SetShort(send_buff, __VERSION, send_index);
	// Cryption
	SetInt64(send_buff, jct.GetPublicKey(), send_index);
	///~
	Send(send_buff, send_index);
	// Cryption
	m_CryptionFlag = 1;
	///~
}

void CUser::LoginProcess(char* pBuf)
{
	int index = 0, idlen = 0, send_index = 0, retvalue = 0;
	int pwdlen = 0;
	char accountid[MAX_ID_SIZE + 1] = {},
		password[MAX_PW_SIZE + 1] = {},
		send_buff[256] = {};

	CUser* pUser = nullptr;
	CTime t = CTime::GetCurrentTime();

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
		goto fail_return;

	GetString(accountid, pBuf, idlen, index);

	pwdlen = GetShort(pBuf, index);
	if (pwdlen > MAX_PW_SIZE
		|| pwdlen <= 0)
		goto fail_return;

	GetString(password, pBuf, pwdlen, index);

	pUser = m_pMain->GetUserPtr(accountid, NameType::Account);
	if (pUser != nullptr
		&& pUser->m_Sid != m_Sid)
	{
		pUser->UserDataSaveToAgent();
		pUser->Close();
		goto fail_return;
	}

	SetByte(send_buff, WIZ_LOGIN, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, accountid, idlen, send_index);
	SetShort(send_buff, pwdlen, send_index);
	SetString(send_buff, password, pwdlen, send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"LoginProcess: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
		goto fail_return;
	}

	strcpy(m_strAccountID, accountid);
	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_LOGIN, send_index);
	SetByte(send_buff, 0xFF, send_index);		 // 성공시 국가 정보... FF 실패
	Send(send_buff, send_index);
}

void CUser::NewCharToAgent(char* pBuf)
{
	int index = 0, idlen = 0, send_index = 0, retvalue = 0;
	int charindex = 0, race = 0, Class = 0, hair = 0, face = 0, str = 0, sta = 0, dex = 0, intel = 0, cha = 0;
	char charid[MAX_ID_SIZE + 1] = {},
		send_buff[256] = {};
	BYTE result;
	int sum = 0;
	model::Coefficient* p_TableCoefficient = nullptr;

	charindex = GetByte(pBuf, index);
	idlen = GetShort(pBuf, index);

	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
	{
		result = 0x05;
		goto fail_return;
	}

	GetString(charid, pBuf, idlen, index);
	race = GetByte(pBuf, index);
	Class = GetShort(pBuf, index);
	face = GetByte(pBuf, index);
	hair = GetByte(pBuf, index);
	str = GetByte(pBuf, index);
	sta = GetByte(pBuf, index);
	dex = GetByte(pBuf, index);
	intel = GetByte(pBuf, index);
	cha = GetByte(pBuf, index);

	if (charindex > 4
		|| charindex < 0)
	{
		result = 0x01;
		goto fail_return;
	}

	if (!IsValidName(charid))
	{
		result = 0x05;
		goto fail_return;
	}

	p_TableCoefficient = m_pMain->m_CoefficientTableMap.GetData(Class);
	if (p_TableCoefficient == nullptr)
	{
		result = 0x02;
		goto fail_return;
	}

	sum = str + sta + dex + intel + cha;
	if (sum > 300)
	{
		result = 0x02;
		goto fail_return;
	}

	if (str < 50
		|| sta < 50
		|| dex < 50
		|| intel < 50
		|| cha < 50)
	{
		result = 0x11;
		goto fail_return;
	}

	SetByte(send_buff, WIZ_NEW_CHAR, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);
	SetByte(send_buff, charindex, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, charid, idlen, send_index);
	SetByte(send_buff, race, send_index);
	SetShort(send_buff, Class, send_index);
	SetByte(send_buff, face, send_index);
	SetByte(send_buff, hair, send_index);
	SetByte(send_buff, str, send_index);
	SetByte(send_buff, sta, send_index);
	SetByte(send_buff, dex, send_index);
	SetByte(send_buff, intel, send_index);
	SetByte(send_buff, cha, send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"NewCharToAgent: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
		goto fail_return;
	}

	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_NEW_CHAR, send_index);
	SetByte(send_buff, result, send_index);
	Send(send_buff, send_index);
}

void CUser::DelCharToAgent(char* pBuf)
{
	int index = 0, idlen = 0, send_index = 0, retvalue = 0;
	int charindex = 0, soclen = 0;
	char charid[MAX_ID_SIZE + 1] = {},
		socno[15] = {},
		send_buff[256] = {};

	charindex = GetByte(pBuf, index);
	if (charindex > 4)
		goto fail_return;

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
		goto fail_return;

	GetString(charid, pBuf, idlen, index);
	soclen = GetShort(pBuf, index);

	// sungyong tw
	// if (soclen != 14)
	//	goto fail_return;

	if (soclen > 14
		|| soclen <= 0) goto fail_return;

	// ~sungyong tw
	GetString(socno, pBuf, soclen, index);

	// 단장은 캐릭 삭제가 안되게, 먼저 클랜을 탈퇴 후 삭제가 되도록,,
	if (m_pUserData->m_bKnights > 0
		&& m_pUserData->m_bFame == CHIEF)
		goto fail_return;

	SetByte(send_buff, WIZ_DEL_CHAR, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);
	SetByte(send_buff, charindex, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, charid, idlen, send_index);
	SetShort(send_buff, soclen, send_index);
	SetString(send_buff, socno, soclen, send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"DelCharToAgent: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
		goto fail_return;
	}

	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_DEL_CHAR, send_index);
	SetByte(send_buff, 0x00, send_index);
	SetByte(send_buff, 0xFF, send_index);
	Send(send_buff, send_index);
}

void CUser::SelNationToAgent(char* pBuf)
{
	int index = 0, send_index = 0, retvalue = 0;
	int nation = 0;
	char send_buff[256] = {};

	nation = GetByte(pBuf, index);
	if (nation > 2)
		goto fail_return;

	SetByte(send_buff, WIZ_SEL_NATION, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);
	SetByte(send_buff, nation, send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"SelNationToAgent: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
		goto fail_return;
	}

	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_SEL_NATION, send_index);
	SetByte(send_buff, 0x00, send_index);
	Send(send_buff, send_index);
}

void CUser::SelCharToAgent(char* pBuf)
{
	int index = 0, idlen1 = 0, idlen2 = 0, send_index = 0, retvalue = 0, zoneId = 0;
	char charId[MAX_ID_SIZE + 1] = {},
		accountId[MAX_ID_SIZE + 1] = {},
		send_buff[256] = {};
	CUser* pUser = nullptr;
	C3DMap* pMap = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;
	CTime t = CTime::GetCurrentTime();
	BYTE bInit = 0x01;

	idlen1 = GetShort(pBuf, index);
	if (idlen1 > MAX_ID_SIZE
		|| idlen1 <= 0)
		goto fail_return;

	GetString(accountId, pBuf, idlen1, index);

	idlen2 = GetShort(pBuf, index);
	if (idlen2 > MAX_ID_SIZE
		|| idlen2 <= 0)
		goto fail_return;

	GetString(charId, pBuf, idlen2, index);
	bInit = GetByte(pBuf, index);
	zoneId = GetByte(pBuf, index);

	if (_strnicmp(accountId, m_strAccountID, MAX_ID_SIZE) != 0)
	{
		pUser = m_pMain->GetUserPtr(accountId, NameType::Account);
		if (pUser != nullptr
			&& pUser->m_Sid != m_Sid)
		{
			pUser->Close();
			goto fail_return;
		}

		strcpy(m_strAccountID, accountId);	// 존이동 한 경우는 로그인 프로시져가 없으므로...
	}

	pUser = m_pMain->GetUserPtr(charId, NameType::Character);
	if (pUser != nullptr
		&& pUser->m_Sid != m_Sid)
	{
		pUser->Close();
		goto fail_return;
	}

	// 음냥,, 여기서 존을 비교,,,
	if (zoneId <= 0)
	{
		spdlog::error("User::SelCharToAgent: invalid zoneId={}", zoneId);
		goto fail_return;
	}

	pMap = m_pMain->GetMapByID(zoneId);
	if (pMap == nullptr)
	{
		spdlog::error("User::SelCharToAgent: no map found for zoneId={}", zoneId);
		goto fail_return;
	}

	if (m_pMain->m_nServerNo != pMap->m_nServerNo)
	{
		pInfo = m_pMain->m_ServerArray.GetData(pMap->m_nServerNo);
		if (pInfo == nullptr)
		{
			spdlog::error("User::SelCharToAgent: serverId={} not registered [zoneId={}]",
				pMap->m_nServerNo, zoneId);
			goto fail_return;
		}

		SetByte(send_buff, WIZ_SERVER_CHANGE, send_index);
		SetShort(send_buff, strlen(pInfo->strServerIP), send_index);
		SetString(send_buff, pInfo->strServerIP, strlen(pInfo->strServerIP), send_index);
		SetShort(send_buff, pInfo->sPort, send_index);
		SetByte(send_buff, bInit, send_index);
		SetByte(send_buff, zoneId, send_index);
		SetByte(send_buff, m_pMain->m_byOldVictory, send_index);
		Send(send_buff, send_index);
		spdlog::error("User::SelCharToAgent: server change [charId={} ip={} init={}]",
			charId, pInfo->strServerIP, bInit);
		return;
	}

	SetByte(send_buff, WIZ_SEL_CHAR, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);
	SetShort(send_buff, idlen2, send_index);
	SetString(send_buff, charId, idlen2, send_index);
	SetByte(send_buff, bInit, send_index);
	SetDWORD(send_buff, m_pMain->m_iPacketCount, send_index);

	{
		spdlog::debug("User::SelCharToAgent: accountId={} charId={} packetCount={} threadId={} rearPtr={}",
			m_strAccountID, charId, m_pMain->m_iPacketCount, GetCurrentThreadId(),
			m_pMain->m_LoggerSendQueue.GetRearPointer());
	}

	m_pMain->m_iPacketCount++;

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"SelCharToAgent: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
		goto fail_return;
	}

	m_pMain->m_iSendPacketCount++;
	//TRACE(_T(" select char to agent ,, acname=%hs, userid=%hs\n"), m_strAccountID, userid);

	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_SEL_CHAR, send_index);
	SetByte(send_buff, 0x00, send_index);
	Send(send_buff, send_index);
}

void CUser::SelectCharacter(char* pBuf)
{
	int index = 0, send_index = 0, retvalue = 0;
	char send_buff[256] = {};
	BYTE result, bInit;
	C3DMap* pMap = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;
	CKnights* pKnights = nullptr;

	result = GetByte(pBuf, index);
	bInit = GetByte(pBuf, index);

	m_pMain->m_iRecvPacketCount++;

	if (result == 0)
		goto fail_return;

	if (m_pUserData->m_bZone == 0)
		goto fail_return;

	pMap = m_pMain->GetMapByID(m_pUserData->m_bZone);
	if (pMap == nullptr)
		goto fail_return;

	if (m_pMain->m_nServerNo != pMap->m_nServerNo)
	{
		pInfo = m_pMain->m_ServerArray.GetData(pMap->m_nServerNo);
		if (pInfo == nullptr)
			goto fail_return;

		SetByte(send_buff, WIZ_SERVER_CHANGE, send_index);
		SetShort(send_buff, strlen(pInfo->strServerIP), send_index);
		SetString(send_buff, pInfo->strServerIP, strlen(pInfo->strServerIP), send_index);
		SetShort(send_buff, pInfo->sPort, send_index);
		SetByte(send_buff, bInit, send_index);
		SetByte(send_buff, m_pUserData->m_bZone, send_index);
		SetByte(send_buff, m_pMain->m_byOldVictory, send_index);
		Send(send_buff, send_index);
		return;
	}

	if (m_pUserData->m_bAuthority == AUTHORITY_BLOCK_USER)
	{
		Close();
		return;
	}

	// 전쟁중이 아닌상태에서는 대장유저가->일반단장으로
	if (m_pMain->m_byBattleOpen == NO_BATTLE
		&& m_pUserData->m_bFame == COMMAND_CAPTAIN)
		m_pUserData->m_bFame = CHIEF;

	if (m_pUserData->m_bZone != m_pUserData->m_bNation
		&& m_pUserData->m_bZone < 3
		&& !m_pMain->m_byBattleOpen)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	if (m_pUserData->m_bZone == ZONE_BATTLE
		&& m_pMain->m_byBattleOpen != NATION_BATTLE)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE
		&& m_pMain->m_byBattleOpen != SNOW_BATTLE)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	if (m_pUserData->m_bZone == ZONE_FRONTIER
		&& m_pMain->m_byBattleOpen)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	SetLogInInfoToDB(bInit);	// Write User Login Info To DB for Kicking out or Billing

	SetByte(send_buff, WIZ_SEL_CHAR, send_index);
	SetByte(send_buff, result, send_index);
	SetByte(send_buff, m_pUserData->m_bZone, send_index);
	SetShort(send_buff, (WORD) m_pUserData->m_curx * 10, send_index);
	SetShort(send_buff, (WORD) m_pUserData->m_curz * 10, send_index);
	SetShort(send_buff, (short) m_pUserData->m_cury * 10, send_index);
	SetByte(send_buff, m_pMain->m_byOldVictory, send_index);
	Send(send_buff, send_index);

	SetDetailData();	// 디비에 없는 데이터 셋팅...

	//TRACE(_T("SelectCharacter 111 - id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);

	// sungyong ,, zone server : 카루스와 전쟁존을 합치므로 인해서,,
	// 전쟁존일때 ... 
	if (m_pUserData->m_bZone > 2)
	{
		// 나의 기사단 리스트에서 내가 기사단 정보에 있는지를 검색해서 만약 없으면 
		// 추가한다(다른존에서 기사단에 가입된 경우)

		// 추방된 유저
		if (m_pUserData->m_bKnights == -1)
		{
			m_pUserData->m_bKnights = 0;
			m_pUserData->m_bFame = 0;
			//TRACE(_T("SelectCharacter - id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
			return;
		}
		
		if (m_pUserData->m_bKnights != 0)
		{
		/*	memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte( send_buff, WIZ_KNIGHTS_PROCESS, send_index );
			SetByte( send_buff, KNIGHTS_LIST_REQ+0x10, send_index );
			SetShort( send_buff, GetSocketID(), send_index );
			SetShort( send_buff, m_pUserData->m_bKnights, send_index );
			retvalue = m_pMain->m_LoggerSendQueue.PutData( send_buff, send_index );
			if( retvalue >= SMQ_FULL ) {
				//goto fail_return;
				m_pMain->m_StatusList.AddString(_T("KNIGHTS_LIST_REQ Packet Drop!!!"));
			}	*/

			pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
			if (pKnights != nullptr)
			{
				m_pMain->m_KnightsManager.SetKnightsUser(m_pUserData->m_bKnights, m_pUserData->m_id);
			}
			else
			{
				//TRACE(_T("SelectCharacter - 기사단 리스트 요청,, id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
				SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
				SetByte(send_buff, KNIGHTS_LIST_REQ + 0x10, send_index);
				SetShort(send_buff, GetSocketID(), send_index);
				SetShort(send_buff, m_pUserData->m_bKnights, send_index);
				retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
				if (retvalue >= SMQ_FULL)
				{
					//goto fail_return;
					m_pMain->AddOutputMessage(_T("KNIGHTS_LIST_REQ Packet Drop!!!"));
				}

				pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
				if (pKnights != nullptr)
				{
					//TRACE(_T("SelectCharacter - 기사단 리스트 추가,, id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
					m_pMain->m_KnightsManager.SetKnightsUser(m_pUserData->m_bKnights, m_pUserData->m_id);
				}
			}
		}
	}
	else
	{
		// 나의 기사단 리스트에서 내가 기사단 정보에 있는지를 검색해서 만약 없으면 
		// 추가한다(다른존에서 기사단에 가입된 경우)

		// 추방된 유저
		if (m_pUserData->m_bKnights == -1)
		{
			m_pUserData->m_bKnights = 0;
			m_pUserData->m_bFame = 0;
			//TRACE(_T("SelectCharacter - id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
			return;
		}
		
		if (m_pUserData->m_bKnights != 0)
		{
			pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
			if (pKnights != nullptr)
			{
				m_pMain->m_KnightsManager.SetKnightsUser(m_pUserData->m_bKnights, m_pUserData->m_id);
			}
			// 기사단이 파괴되어 있음으로..
			else
			{
				m_pUserData->m_bKnights = 0;
				m_pUserData->m_bFame = 0;
			}
		}
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_DATASAVE, send_index);
	SetShort(send_buff, strlen(m_pUserData->m_Accountid), send_index);
	SetString(send_buff, m_pUserData->m_Accountid, strlen(m_pUserData->m_Accountid), send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
	SetByte(send_buff, 0x01, send_index);	// login...
	SetByte(send_buff, m_pUserData->m_bLevel, send_index);
	SetDWORD(send_buff, m_pUserData->m_iExp, send_index);
	SetDWORD(send_buff, m_pUserData->m_iLoyalty, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);

	retvalue = m_pMain->m_ItemLoggerSendQ.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"SelectCharacter: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
	}
	//TRACE(_T("SelectCharacter - id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);

	return;

fail_return:
	SetByte(send_buff, WIZ_SEL_CHAR, send_index);
	SetByte(send_buff, 0x00, send_index);
	Send(send_buff, send_index);
}

void CUser::AllCharInfoToAgent()
{
	int send_index = 0, retvalue = 0;
	char send_buff[256] = {};

	SetByte(send_buff, WIZ_ALLCHAR_INFO_REQ, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_ALLCHAR_INFO_REQ, send_index);
		SetByte(send_buff, 0xFF, send_index);
		
		std::wstring logstr = std::format(L"AllCharInfoToAgent: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
	}
}

void CUser::UserDataSaveToAgent()
{
	int send_index = 0, retvalue = 0;
	char send_buff[256] = {};

	if (strlen(m_pUserData->m_id) == 0
		|| strlen(m_pUserData->m_Accountid) == 0)
		return;

	SetByte(send_buff, WIZ_DATASAVE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_pUserData->m_Accountid), send_index);
	SetString(send_buff, m_pUserData->m_Accountid, strlen(m_pUserData->m_Accountid), send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"UserDataSaveToAgent: send error: {}", retvalue);
			m_pMain->AddOutputMessage(logstr);
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_DATASAVE, send_index);
	SetShort(send_buff, strlen(m_pUserData->m_Accountid), send_index);
	SetString(send_buff, m_pUserData->m_Accountid, strlen(m_pUserData->m_Accountid), send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
	SetByte(send_buff, 0x02, send_index);
	SetByte(send_buff, m_pUserData->m_bLevel, send_index);
	SetDWORD(send_buff, m_pUserData->m_iExp, send_index);
	SetDWORD(send_buff, m_pUserData->m_iLoyalty, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);

	retvalue = m_pMain->m_ItemLoggerSendQ.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"UserDataSaveToAgent: item send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
	}
}

void CUser::LogOut()
{
	int index = 0, idlen = 0, idindex = 0, send_index = 0, count = 0;
	CUser* pUser = nullptr;
	char send_buff[256] = {};
	
	spdlog::debug("User::LogOut: accountId={} charId={}",
		m_pUserData->m_Accountid, m_pUserData->m_id);

	pUser = m_pMain->GetUserPtr(m_pUserData->m_Accountid, NameType::Account);
	if (pUser != nullptr
		&& pUser->m_Sid != m_Sid)
	{
		spdlog::error("User::LogOut: got a pointer to a duplicate user socket [accountId={} charId={}]",
			m_pUserData->m_Accountid, m_pUserData->m_id);
		return;
	}

	// 이미 유저가 빠진 경우.. 
	if (strlen(m_pUserData->m_id) == 0)
		return;

	SetByte(send_buff, WIZ_LOGOUT, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_pUserData->m_Accountid), send_index);
	SetString(send_buff, m_pUserData->m_Accountid, strlen(m_pUserData->m_Accountid), send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);

	do
	{
		if (m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index) == 1)
			break;

		count++;
	}
	while (count < 30);

	if (count > 29)
	{
		std::string logstr = fmt::format("LogOut: send error: accountId={} charId={}",
			m_pUserData->m_Accountid, m_pUserData->m_id);
		m_pMain->AddOutputMessage(logstr);
	}

	SetByte(send_buff, AG_USER_LOG_OUT, index);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_DATASAVE, send_index);
	SetShort(send_buff, strlen(m_pUserData->m_Accountid), send_index);
	SetString(send_buff, m_pUserData->m_Accountid, strlen(m_pUserData->m_Accountid), send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
	SetByte(send_buff, 0x03, send_index);		// logout
	SetByte(send_buff, m_pUserData->m_bLevel, send_index);
	SetDWORD(send_buff, m_pUserData->m_iExp, send_index);
	SetDWORD(send_buff, m_pUserData->m_iLoyalty, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);

	int retvalue = m_pMain->m_ItemLoggerSendQ.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"LogOut: item send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
	}

//	if (m_pUserData->m_bKnights > 0)
//		m_pMain->m_KnightsManager.ModifyKnightsUser(m_pUserData->m_bKnights, m_pUserData->m_id, m_pUserData->m_bFame, m_pUserData->m_bLevel, m_pUserData->m_sClass, 0);

//	TRACE(_T("Send Logout Result - %hs\n"), m_pUserData->m_id);
}

void CUser::MoveProcess(char* pBuf)
{
	if (m_bWarp)
		return;

	int index = 0, send_index = 0, region = 0;
	WORD will_x, will_z;
	short will_y, speed = 0;
	float real_x, real_z, real_y;
	BYTE echo;
	char send_buff[1024] = {};
	C3DMap* pMap = nullptr;

	will_x = GetShort(pBuf, index);
	will_z = GetShort(pBuf, index);
	will_y = GetShort(pBuf, index);

	speed = GetShort(pBuf, index);
	echo = GetByte(pBuf, index);

	real_x = will_x / 10.0f;
	real_z = will_z / 10.0f;
	real_y = will_y / 10.0f;

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	if (!pMap->IsValidPosition(real_x, real_z, real_y))
		return;

//	real_y = pMap->GetHeight(	real_x, real_y, real_z );

//	if( speed > 60 ) {	// client 에서 이수치보다 크게 보낼때가 많음...
//		if( m_bSpeedAmount == 100 )
//			speed = 0;
//	}

	if (m_bResHpType == USER_DEAD
		|| m_pUserData->m_sHp == 0)
	{
		if (speed != 0)
		{
			spdlog::warn("User::MoveProcess: dead user is moving [charId={} socketId={} resHpType={} hp={} speed={} x={} z={}]",
				m_pUserData->m_id, m_Sid, m_bResHpType, m_pUserData->m_sHp, speed,
				static_cast<int32_t>(m_pUserData->m_curx), static_cast<int32_t>(m_pUserData->m_curz));
		}
	}

	if (speed != 0)
	{
		m_pUserData->m_curx = m_fWill_x;	// 가지고 있던 다음좌표를 현재좌표로 셋팅...
		m_pUserData->m_curz = m_fWill_z;
		m_pUserData->m_cury = m_fWill_y;

		m_fWill_x = will_x / 10.0f;	// 다음좌표를 기억....
		m_fWill_z = will_z / 10.0f;
		m_fWill_y = will_y / 10.0f;
	}
	else
	{
		m_pUserData->m_curx = m_fWill_x = will_x / 10.0f;	// 다음좌표 == 현재 좌표...
		m_pUserData->m_curz = m_fWill_z = will_z / 10.0f;
		m_pUserData->m_cury = m_fWill_y = will_y / 10.0f;
	}

	SetByte(send_buff, WIZ_MOVE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, will_x, send_index);
	SetShort(send_buff, will_z, send_index);
	SetShort(send_buff, will_y, send_index);
	SetShort(send_buff, speed, send_index);
	SetByte(send_buff, echo, send_index);

	RegisterRegion();
	m_pMain->Send_Region(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	pMap->CheckEvent(real_x, real_z, this);

	int  ai_send_index = 0;
	char ai_send_buff[256] = {};

	SetByte(ai_send_buff, AG_USER_MOVE, ai_send_index);
	SetShort(ai_send_buff, m_Sid, ai_send_index);
	Setfloat(ai_send_buff, m_fWill_x, ai_send_index);
	Setfloat(ai_send_buff, m_fWill_z, ai_send_index);
	Setfloat(ai_send_buff, m_fWill_y, ai_send_index);
	SetShort(ai_send_buff, speed, ai_send_index);

	m_pMain->Send_AIServer(m_pUserData->m_bZone, ai_send_buff, ai_send_index);
}

void CUser::UserInOut(BYTE Type)
{
	int send_index = 0;
	char send_buff[256] = {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	if (Type == USER_OUT)
		pMap->RegionUserRemove(m_RegionX, m_RegionZ, m_Sid);
	else
		pMap->RegionUserAdd(m_RegionX, m_RegionZ, m_Sid);

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_USER_INOUT, send_index);
	SetByte(send_buff, Type, send_index);
	SetShort(send_buff, m_Sid, send_index);

	if (Type == USER_OUT)
	{
		m_pMain->Send_Region(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, this);

		// AI Server쪽으로 정보 전송..
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_USER_INOUT, send_index);
		SetByte(send_buff, Type, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
		SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
		Setfloat(send_buff, m_pUserData->m_curx, send_index);
		Setfloat(send_buff, m_pUserData->m_curz, send_index);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
		return;
	}

	GetUserInfo(send_buff, send_index);

//	TRACE(_T("USERINOUT - %d, %hs\n"), m_Sid, m_pUserData->m_id);
	m_pMain->Send_Region(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, this);

	// AI Server쪽으로 정보 전송..
	// 이거 않되도 너무 미워하지 마세요 ㅜ.ㅜ
	if (m_bAbnormalType != ABNORMAL_BLINKING)
	{
		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));
		SetByte(send_buff, AG_USER_INOUT, send_index);
		SetByte(send_buff, Type, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
		SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
		Setfloat(send_buff, m_pUserData->m_curx, send_index);
		Setfloat(send_buff, m_pUserData->m_curz, send_index);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
	}
//
}

void CUser::Rotate(char* pBuf)
{
	int index = 0, send_index = 0;
	char send_buff[256] = {};

	m_sDirection = GetShort(pBuf, index);

	SetByte(send_buff, WIZ_ROTATE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, m_sDirection, send_index);

	m_pMain->Send_Region(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
}

void CUser::Attack(char* pBuf)
{
	int index = 0, send_index = 0;
	int sid = -1, tid = -1, damage = 0;
	float delaytime = 0.0f, distance = 0.0f;
	BYTE type, result;
	char send_buff[256] = {};

//	CUser* pUser = nullptr;
	CUser* pTUser = nullptr;
	CNpc* pNpc = nullptr;
	model::Item* pTable = nullptr;

	type = GetByte(pBuf, index);
	result = GetByte(pBuf, index);
//	sid = GetShort(pBuf, index);
	tid = GetShort(pBuf, index);
// 비러머글 해킹툴 유저 --;
	delaytime = GetShort(pBuf, index);
	distance = GetShort(pBuf, index);
//

//	delaytime = delaytime / 100.0f;
//	distance = distance / 10.0f;	// 'Coz the server multiplies it by 10 before they send it to you.

/*
	if (sid < 0
		|| sid >= MAX_USER
		|| tid < 0)
		return;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[sid];
	if (pUser == nullptr
		|| pUser->m_Sid != m_Sid
		|| pUser->m_bResHpType == USER_BLINKING)
		return;
*/

	if (m_bAbnormalType == ABNORMAL_BLINKING)
		return;

	if (m_bResHpType == USER_DEAD
		|| m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::Attack: dead user cannot attack [charId={} resHpType={} hp={}]",
			m_pUserData->m_id, m_Sid, m_bResHpType, m_pUserData->m_sHp);
		return;
	}

// 비러머글 해킹툴 유저 --;
	// This checks if such an item exists.
	pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
	if (pTable == nullptr
		&& m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
		return;

	// If you're holding a weapon, do a delay check.
	if (pTable != nullptr)
	{
//		TRACE(_T("Delay time : %f  ,  Table Delay Time : %f \r\n"), delaytime, pTable->m_sDelay / 100.0f);
//		if (delaytime + 0.01f < (pTable->Delay / 100.0f)) {
		if (delaytime < pTable->Delay)
			return;
	}
	// Empty handed.
	else
	{
//		if (delaytime + 0.01f < 1.0f) 
		if (delaytime < 100)
			return;
	}
//
	// USER
	if (tid < NPC_BAND)
	{
		if (tid >= MAX_USER
			|| tid < 0)
			return;

		pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];

		if (pTUser == nullptr
			|| pTUser->m_bResHpType == USER_DEAD
			|| pTUser->m_bAbnormalType == ABNORMAL_BLINKING
			|| pTUser->m_pUserData->m_bNation == m_pUserData->m_bNation)
		{
			result = 0x00;
		}
		else
		{
// 비러머글 해킹툴 유저 --;
			// Check if the user is holding a weapon!!! No null pointers allowed!!!
			if (pTable != nullptr)
			{
//				TRACE(_T("Distance : %f  , Table Distance : %f  \r\n"), distance, pTable->Range / 10.0f);
//				if (distance > (pTable->Range / 10.0f))
				if (distance > pTable->Range)
					return;
			}
//
			damage = GetDamage(tid, 0);

			// 눈싸움전쟁존에서 눈싸움중이라면 공격은 눈을 던지는 것만 가능하도록,,,
			if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE
				&& m_pMain->m_byBattleOpen == SNOW_BATTLE)
				damage = 0;

			if (damage <= 0)
			{
				result = 0;
			}
			else
			{
				pTUser->HpChange(-damage, 0, true);
				ItemWoreOut(DURABILITY_TYPE_ATTACK, damage);
				pTUser->ItemWoreOut(DURABILITY_TYPE_DEFENCE, damage);

				//TRACE(_T("%hs - HP:%d, (damage-%d, t_hit-%d)\n"), pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_sHp, damage, m_sTotalHit);

				if (pTUser->m_pUserData->m_sHp == 0)
				{
					result = 0x02;
					pTUser->m_bResHpType = USER_DEAD;

				// sungyong work : loyalty		
					// Something regarding loyalty points.
					if (m_sPartyIndex == -1)
						LoyaltyChange(tid);
					else
						LoyaltyDivide(tid);

					GoldChange(tid, 0);

					// 기범이의 완벽한 보호 코딩!!!
					pTUser->InitType3();	// Init Type 3.....
					pTUser->InitType4();	// Init Type 4.....

					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;

					// 지휘권한이 있는 유저가 죽는다면,, 지휘 권한 박탈
					if (pTUser->m_pUserData->m_bFame == COMMAND_CAPTAIN)
					{
						pTUser->m_pUserData->m_bFame = CHIEF;

						SetByte(send_buff, WIZ_AUTHORITY_CHANGE, send_index);
						SetByte(send_buff, COMMAND_AUTHORITY, send_index);
						SetShort(send_buff, pTUser->GetSocketID(), send_index);
						SetByte(send_buff, pTUser->m_pUserData->m_bFame, send_index);
						m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ);

						Send(send_buff, send_index);

						//TRACE(_T("---> UserAttack Dead Captain Deprive - %hs\n"), pTUser->m_pUserData->m_id);

						if (pTUser->m_pUserData->m_bNation == KARUS)
							m_pMain->Announcement(KARUS_CAPTAIN_DEPRIVE_NOTIFY, KARUS);
						else if (pTUser->m_pUserData->m_bNation == ELMORAD)
							m_pMain->Announcement(ELMORAD_CAPTAIN_DEPRIVE_NOTIFY, ELMORAD);
					}

					pTUser->m_sWhoKilledMe = m_Sid;		// You killed me, you.....
//
					if (pTUser->m_pUserData->m_bZone != pTUser->m_pUserData->m_bNation
						&& pTUser->m_pUserData->m_bZone < 3)
					{
						pTUser->ExpChange(-pTUser->m_iMaxExp / 100);
						//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
					}
//
				}

				SendTargetHP(0, tid, -damage);
			}
		}
	}
	// NPC
	else if (tid >= NPC_BAND)
	{
		// 포인터 참조하면 안됨
		if (!m_pMain->m_bPointCheckFlag)
			return;

		pNpc = m_pMain->m_NpcMap.GetData(tid);

		// Npc 상태 체크..
		if (pNpc != nullptr
			&& pNpc->m_NpcState != NPC_DEAD
			&& pNpc->m_iHP > 0)
		{
// 비러머글 해킹툴 유저 --;
			// Check if the user is holding a weapon!!! No null pointers allowed!!!
			if (pTable != nullptr)
			{
//				TRACE(_T("Distance : %f  , Table Distance : %f  \r\n"), distance, pTable->Range / 10.0f);
//				if (distance > (pTable->Range / 10.0f))
				if (distance > pTable->Range)
					return;

				// TRACE(_T("Success!!! \r\n"));
			}
//
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, AG_ATTACK_REQ, send_index);
			SetByte(send_buff, type, send_index);
			SetByte(send_buff, result, send_index);
//			SetShort( send_buff, sid, send_index );
			SetShort(send_buff, m_Sid, send_index);
			SetShort(send_buff, tid, send_index);
			SetShort(send_buff, m_sTotalHit * m_bAttackAmount / 100, send_index);   // 표시
			SetShort(send_buff, m_sTotalAc + m_sACAmount, send_index);   // 표시
			Setfloat(send_buff, m_sTotalHitrate, send_index);
			Setfloat(send_buff, m_sTotalEvasionrate, send_index);
			SetShort(send_buff, m_sItemAc, send_index);
			SetByte(send_buff, m_bMagicTypeLeftHand, send_index);
			SetByte(send_buff, m_bMagicTypeRightHand, send_index);
			SetShort(send_buff, m_sMagicAmountLeftHand, send_index);
			SetShort(send_buff, m_sMagicAmountRightHand, send_index);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);	// AI Server쪽으로 정보 전송..
			return;
		}
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_ATTACK, send_index);
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, result, send_index);
//	SetShort( send_buff, sid, send_index );
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, tid, send_index);
	m_pMain->Send_Region(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	if (tid < NPC_BAND)
	{
		if (result == 0x02)
		{
			// 유저에게는 바로 데드 패킷을 날림... (한 번 더 보냄, 유령을 없애기 위해서)
			if (pTUser != nullptr)
			{
				pTUser->Send(send_buff, send_index);
				memset(send_buff, 0, sizeof(send_buff));
				
				spdlog::debug("User::Attack: user attack dead [charId={} result={} resHpType={} Hp={}]",
					pTUser->m_pUserData->m_id, result, pTUser->m_bResHpType,
					pTUser->m_pUserData->m_sHp);
			}
		}
	}
}

void CUser::SendMyInfo(int type)
{
	// TODO:
	if (type != 0)
		return;

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	CKnights* pKnights = nullptr;

	int send_index = 0;
	char send_buff[2048] = {};

	int x = 0, z = 0;
//	int map_size = (pMap->m_nMapSize - 1) * pMap->m_fUnitDist ;		// Are you within the map limits?
//	if (m_pUserData->m_curx >= map_size || m_pUserData->m_curz >= map_size) {

	if (!pMap->IsValidPosition(m_pUserData->m_curx, m_pUserData->m_curz, 0.0f))
	{
		model::Home* pHomeInfo = m_pMain->m_HomeTableMap.GetData(m_pUserData->m_bNation);
		if (pHomeInfo == nullptr)
			return;

		// Battle Zone...
		if (m_pUserData->m_bNation != m_pUserData->m_bZone
			&& m_pUserData->m_bZone > 200)
		{
			x = pHomeInfo->FreeZoneX + myrand(0, pHomeInfo->FreeZoneLX);
			z = pHomeInfo->FreeZoneZ + myrand(0, pHomeInfo->FreeZoneLZ);
		}
		else if (m_pUserData->m_bNation != m_pUserData->m_bZone && m_pUserData->m_bZone < 3)
		{	// Specific Lands...
			if (m_pUserData->m_bNation == KARUS)
			{
				x = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
				z = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
			}
			else if (m_pUserData->m_bNation == ELMORAD)
			{
				x = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
				z = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
			}
			else
			{
				return;
			}
		}
		else
		{	// Your own nation...
			if (m_pUserData->m_bNation == KARUS)
			{
				x = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
				z = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
			}
			else if (m_pUserData->m_bNation == ELMORAD)
			{
				x = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
				z = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
			}
			else
			{
				return;
			}
		}

		m_pUserData->m_curx = x;
		m_pUserData->m_curz = z;
	}

	SetByte(send_buff, WIZ_MYINFO, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetString1(send_buff, m_pUserData->m_id, static_cast<BYTE>(strlen(m_pUserData->m_id)), send_index);

	SetShort(send_buff, (WORD) m_pUserData->m_curx * 10, send_index);
	SetShort(send_buff, (WORD) m_pUserData->m_curz * 10, send_index);
	SetShort(send_buff, (short) m_pUserData->m_cury * 10, send_index);

	SetByte(send_buff, m_pUserData->m_bNation, send_index);
	SetByte(send_buff, m_pUserData->m_bRace, send_index);
	SetShort(send_buff, m_pUserData->m_sClass, send_index);
	SetByte(send_buff, m_pUserData->m_bFace, send_index);
	SetByte(send_buff, m_pUserData->m_bHairColor, send_index);
	SetByte(send_buff, m_pUserData->m_bRank, send_index);
	SetByte(send_buff, m_pUserData->m_bTitle, send_index);
	SetByte(send_buff, m_pUserData->m_bLevel, send_index);
	SetByte(send_buff, m_pUserData->m_bPoints, send_index);
	SetDWORD(send_buff, m_iMaxExp, send_index);
	SetDWORD(send_buff, m_pUserData->m_iExp, send_index);
	SetDWORD(send_buff, m_pUserData->m_iLoyalty, send_index);
	SetDWORD(send_buff, m_pUserData->m_iLoyaltyMonthly, send_index);
	SetByte(send_buff, m_pUserData->m_bCity, send_index);
	SetShort(send_buff, m_pUserData->m_bKnights, send_index);
	SetByte(send_buff, m_pUserData->m_bFame, send_index);

	if (m_pUserData->m_bKnights != 0)
		pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);

	if (pKnights != nullptr)
	{
		SetShort(send_buff, pKnights->m_sAllianceKnights, send_index);
		SetByte(send_buff, pKnights->m_byFlag, send_index);
		SetString1(send_buff, pKnights->m_strName, static_cast<BYTE>(strlen(pKnights->m_strName)), send_index);
		SetByte(send_buff, pKnights->m_byGrade, send_index); // Knights grade
		SetByte(send_buff, pKnights->m_byRanking, send_index);
		SetShort(send_buff, pKnights->m_sMarkVersion, send_index);
		SetShort(send_buff, pKnights->m_sCape, send_index);
		//TRACE(_T("sendmyinfo knights index = %d, kname=%hs, name=%hs\n") , iLength, pKnights->strName, m_pUserData->m_id);
	}
	else
	{
		SetShort(send_buff, 0, send_index);		// m_sAllianceKnights
		SetByte(send_buff, 0, send_index);		// m_byFlag
		SetByte(send_buff, 0, send_index);		// m_strName
		SetByte(send_buff, 0, send_index);		// m_byGrade
		SetByte(send_buff, 0, send_index);		// m_byRanking
		SetShort(send_buff, 0, send_index);		// m_sMarkVerison
		SetShort(send_buff, -1, send_index);	// m_sCape
	}

	SetShort(send_buff, m_iMaxHp, send_index);
	SetShort(send_buff, m_pUserData->m_sHp, send_index);
	SetShort(send_buff, m_iMaxMp, send_index);
	SetShort(send_buff, m_pUserData->m_sMp, send_index);
	SetShort(send_buff, GetMaxWeightForClient(), send_index);
	SetShort(send_buff, GetCurrentWeightForClient(), send_index);
	SetByte(send_buff, m_pUserData->m_bStr, send_index);
	SetByte(send_buff, static_cast<BYTE>(m_sItemStr), send_index);
	SetByte(send_buff, m_pUserData->m_bSta, send_index);
	SetByte(send_buff, static_cast<BYTE>(m_sItemSta), send_index);
	SetByte(send_buff, m_pUserData->m_bDex, send_index);
	SetByte(send_buff, static_cast<BYTE>(m_sItemDex), send_index);
	SetByte(send_buff, m_pUserData->m_bIntel, send_index);
	SetByte(send_buff, static_cast<BYTE>(m_sItemIntel), send_index);
	SetByte(send_buff, m_pUserData->m_bCha, send_index);
	SetByte(send_buff, static_cast<BYTE>(m_sItemCham), send_index);
	SetShort(send_buff, m_sTotalHit, send_index);
	SetShort(send_buff, m_sTotalAc, send_index);
//	SetShort( send_buff, m_sBodyAc+m_sItemAc, send_index );		<- 누가 이렇게 해봤어? --;	
	SetByte(send_buff, m_bFireR, send_index);
	SetByte(send_buff, m_bColdR, send_index);
	SetByte(send_buff, m_bLightningR, send_index);
	SetByte(send_buff, m_bMagicR, send_index);
	SetByte(send_buff, m_bDiseaseR, send_index);
	SetByte(send_buff, m_bPoisonR, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
// 이거 나중에 꼭 주석해 --;
	SetByte(send_buff, m_pUserData->m_bAuthority, send_index);
//

	SetByte(send_buff, m_byKnightsRank, send_index);
	SetByte(send_buff, m_byPersonalRank, send_index);

	for (int i = 0; i < 9; i++)
		SetByte(send_buff, m_pUserData->m_bstrSkill[i], send_index);

	for (int i = 0; i < SLOT_MAX; i++)
	{
		SetDWORD(send_buff, m_pUserData->m_sItemArray[i].nNum, send_index);
		SetShort(send_buff, m_pUserData->m_sItemArray[i].sDuration, send_index);
		SetShort(send_buff, m_pUserData->m_sItemArray[i].sCount, send_index);
		SetByte(send_buff, m_pUserData->m_sItemArray[i].byFlag, send_index);
		SetShort(send_buff, m_pUserData->m_sItemArray[i].sTimeRemaining, send_index);
	}

	for (int i = 0; i < HAVE_MAX; i++)
	{
		SetDWORD(send_buff, m_pUserData->m_sItemArray[SLOT_MAX + i].nNum, send_index);
		SetShort(send_buff, m_pUserData->m_sItemArray[SLOT_MAX + i].sDuration, send_index);
		SetShort(send_buff, m_pUserData->m_sItemArray[SLOT_MAX + i].sCount, send_index);
		SetByte(send_buff, m_pUserData->m_sItemArray[SLOT_MAX + i].byFlag, send_index);
		SetShort(send_buff, m_pUserData->m_sItemArray[SLOT_MAX + i].sTimeRemaining, send_index);
	}

	SetByte(send_buff, 0, send_index); // account status (0 = none, 1 = normal prem with expiry in hours, 2 = pc room)
	SetByte(send_buff, m_pUserData->m_byPremiumType, send_index);
	SetShort(send_buff, m_pUserData->m_sPremiumTime, send_index);
	SetByte(send_buff, static_cast<BYTE>(m_bIsChicken), send_index);
	SetDWORD(send_buff, m_pUserData->m_iMannerPoint, send_index);

	Send(send_buff, send_index);

	SetZoneAbilityChange(m_pUserData->m_bZone);

	// AI Server쪽으로 정보 전송..
	int ai_send_index = 0;
	char ai_send_buff[256] = {};

	SetByte(ai_send_buff, AG_USER_INFO, ai_send_index);
	SetShort(ai_send_buff, m_Sid, ai_send_index);
	SetString2(ai_send_buff, m_pUserData->m_id, static_cast<short>(strlen(m_pUserData->m_id)), ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bZone, ai_send_index);
	SetShort(ai_send_buff, m_iZoneIndex, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bNation, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bLevel, ai_send_index);
	SetShort(ai_send_buff, m_pUserData->m_sHp, ai_send_index);
	SetShort(ai_send_buff, m_pUserData->m_sMp, ai_send_index);
	SetShort(ai_send_buff, m_sTotalHit * m_bAttackAmount / 100, ai_send_index);  // 표시
	SetShort(ai_send_buff, m_sTotalAc + m_sACAmount, ai_send_index);  // 표시
	Setfloat(ai_send_buff, m_sTotalHitrate, ai_send_index);
	Setfloat(ai_send_buff, m_sTotalEvasionrate, ai_send_index);

// Yookozuna
	SetShort(ai_send_buff, m_sItemAc, ai_send_index);
	SetByte(ai_send_buff, m_bMagicTypeLeftHand, ai_send_index);
	SetByte(ai_send_buff, m_bMagicTypeRightHand, ai_send_index);
	SetShort(ai_send_buff, m_sMagicAmountLeftHand, ai_send_index);
	SetShort(ai_send_buff, m_sMagicAmountRightHand, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bAuthority, ai_send_index);
//
	m_pMain->Send_AIServer(m_pUserData->m_bZone, ai_send_buff, ai_send_index);

//	if( m_pUserData->m_bKnights > 0 )	{
//		m_pMain->m_KnightsManager.ModifyKnightsUser( m_pUserData->m_bKnights, m_pUserData->m_id, m_pUserData->m_bFame, m_pUserData->m_bLevel, m_pUserData->m_sClass, 1);
//	}
}

void CUser::Chat(char* pBuf)
{
	int index = 0, chatlen = 0, send_index = 0, tid = -1;
	BYTE type;
	CUser* pUser = nullptr;
	char chatstr[1024] = {},
		send_buff[1024] = {};
	std::string finalstr;

	// this user refused chatting
	if (m_pUserData->m_bAuthority == AUTHORITY_NOCHAT)
		return;

	type = GetByte(pBuf, index);
	chatlen = GetShort(pBuf, index);
	if (chatlen > 512
		|| chatlen <= 0)
		return;

	GetString(chatstr, pBuf, chatlen, index);

	if (m_pUserData->m_bAuthority == AUTHORITY_MANAGER
		&& chatstr[0] == '+')
	{
		OperationMessage opMessage(m_pMain, this);
		opMessage.Process(chatstr);
		return;
	}

	if (type == PUBLIC_CHAT
		|| type == ANNOUNCEMENT_CHAT)
	{
		if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
			return;

		finalstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);
	}
	else
	{
		finalstr.assign(chatstr, chatlen);
	}

	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, m_pUserData->m_bNation, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetString1(send_buff, m_pUserData->m_id, static_cast<BYTE>(strlen(m_pUserData->m_id)), send_index);
	SetString2(send_buff, finalstr, send_index);

	switch (type)
	{
		case GENERAL_CHAT:
			m_pMain->Send_NearRegion(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, m_pUserData->m_curx, m_pUserData->m_curz);
			break;

		case PRIVATE_CHAT:
			if (m_sPrivateChatUser < 0
				|| m_sPrivateChatUser >= MAX_USER)
				break;

			// 이건 내가 추가했지롱 :P
			if (m_sPrivateChatUser == m_Sid)
				break;

			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sPrivateChatUser];
			if (pUser == nullptr
				|| pUser->GetState() != STATE_GAMESTART)
				break;

			pUser->Send(send_buff, send_index);
			Send(send_buff, send_index);
			break;

		case PARTY_CHAT:
			m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
			break;

		case FORCE_CHAT:
			break;

		case SHOUT_CHAT:
			if (m_pUserData->m_sMp < (m_iMaxMp / 5))
				break;

			MSpChange(-(m_iMaxMp / 5));
			m_pMain->Send_Region(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
			break;

		case KNIGHTS_CHAT:
			m_pMain->Send_KnightsMember(m_pUserData->m_bKnights, send_buff, send_index, m_pUserData->m_bZone);
			break;

		case PUBLIC_CHAT:
			m_pMain->Send_All(send_buff, send_index);
			break;

		case COMMAND_CHAT:
			if (m_pUserData->m_bFame == COMMAND_CAPTAIN)		// 지휘권자만 채팅이 되도록
				m_pMain->Send_CommandChat(send_buff, send_index, m_pUserData->m_bNation, this);
			break;

		//case WAR_SYSTEM_CHAT:
		//	m_pMain->Send_All( send_buff, send_index );
		//	break;
	}
}

void CUser::SetMaxHp(int iFlag)
{
	model::Coefficient* p_TableCoefficient
		= m_pMain->m_CoefficientTableMap.GetData(m_pUserData->m_sClass);
	if (p_TableCoefficient == nullptr)
		return;

	int temp_sta = 0;
	temp_sta = m_pUserData->m_bSta + m_sItemSta + m_bStaAmount;
//	if (temp_sta > 255)
//		temp_sta = 255;

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE
		&& iFlag == 0)
	{
		m_iMaxHp = 100;
		//TRACE(_T("--> SetMaxHp - name=%hs, max=%d, hp=%d\n"), m_pUserData->m_id, m_iMaxHp, m_pUserData->m_sHp);
	}
	else
	{
		m_iMaxHp = (short) (((p_TableCoefficient->HitPoint * m_pUserData->m_bLevel * m_pUserData->m_bLevel * temp_sta)
			+ (0.1 * m_pUserData->m_bLevel * temp_sta) + (temp_sta / 5)) + m_sMaxHPAmount + m_sItemMaxHp);

		if (iFlag == 1)
			m_pUserData->m_sHp = m_iMaxHp + 20;		// 조금 더 hp를 주면 자동으로 hpchange()함수가 실행됨,, 꽁수^^*
		else if (iFlag == 2)
			m_iMaxHp = 100;

		//TRACE(_T("<-- SetMaxHp - name=%hs, max=%d, hp=%d\n"), m_pUserData->m_id, m_iMaxHp, m_pUserData->m_sHp);
	}

	if (m_iMaxHp < m_pUserData->m_sHp)
	{
		m_pUserData->m_sHp = m_iMaxHp;
		HpChange(m_pUserData->m_sHp);
	}

	if (m_pUserData->m_sHp < 5)
		m_pUserData->m_sHp = 5;
}

void CUser::SetMaxMp()
{
	model::Coefficient* p_TableCoefficient
		= m_pMain->m_CoefficientTableMap.GetData(m_pUserData->m_sClass);
	if (p_TableCoefficient == nullptr)
		return;

	int temp_intel = 0, temp_sta = 0;
	temp_intel = m_pUserData->m_bIntel + m_sItemIntel + m_bIntelAmount + 30;
//	if (temp_intel > 255)
//		temp_intel = 255;

	temp_sta = m_pUserData->m_bSta + m_sItemSta + m_bStaAmount;
//	if (temp_sta > 255)
//		temp_sta = 255;

	if (p_TableCoefficient->ManaPoint != 0)
	{
		m_iMaxMp = (short) ((p_TableCoefficient->ManaPoint * m_pUserData->m_bLevel * m_pUserData->m_bLevel * temp_intel)
				  + (0.1f * m_pUserData->m_bLevel * 2 * temp_intel) + (temp_intel / 5));
		m_iMaxMp += m_sItemMaxMp;
		m_iMaxMp += 20;		 // 성래씨 요청
	}
	else if (p_TableCoefficient->Sp != 0)
	{
		m_iMaxMp = (short) ((p_TableCoefficient->Sp * m_pUserData->m_bLevel * m_pUserData->m_bLevel * temp_sta)
			  + (0.1f * m_pUserData->m_bLevel * temp_sta) + (temp_sta / 5));
		m_iMaxMp += m_sItemMaxMp;
	}

	if (m_iMaxMp < m_pUserData->m_sMp)
	{
		m_pUserData->m_sMp = m_iMaxMp;
		MSpChange(m_pUserData->m_sMp);
	}
}

// 너무 개판이라 나중에 반드시 수정해야 할 함수.... 
void CUser::Regene(char* pBuf, int magicid)
{
//	Corpse();		// Get rid of the corpse ~ 또 사고칠뻔 했자나 이 바보야!!!

	InitType3();
	InitType4();

	CUser* pUser = nullptr;
	_OBJECT_EVENT* pEvent = nullptr;
	model::Home* pHomeInfo = nullptr;
	model::MagicType5* pType = nullptr;
	C3DMap* pMap = nullptr;

	int index = 0;
	BYTE regene_type = 0;

	regene_type = GetByte(pBuf, index);

	if (regene_type != 1
		&& regene_type != 2)
		regene_type = 1;

	if (regene_type == 2)
	{
		magicid = 490041;	// The Stone of Resurrection magic ID

		// Subtract resurrection stones.
		if (ItemCountChange(379006000, 1, 3 * m_pUserData->m_bLevel) < 2)
			return;

		// 5 level minimum.
		if (m_pUserData->m_bLevel <= 5)
			return;
	}

	pHomeInfo = m_pMain->m_HomeTableMap.GetData(m_pUserData->m_bNation);
	if (pHomeInfo == nullptr)
		return;

	int send_index = 0;
	char send_buff[1024] = {};

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	UserInOut(USER_OUT);	// 원래는 이 한줄밖에 없었음 --;

	float x = (float) (myrand(0, 400) / 100.0f);
	float z = (float) (myrand(0, 400) / 100.0f);

	if (x < 2.5f)
		x += 1.5f;

	if (z < 2.5f)
		z += 1.5f;

	pEvent = pMap->GetObjectEvent(m_pUserData->m_sBind);

	if (magicid == 0)
	{
		// Bind Point
		if (pEvent != nullptr
			&& pEvent->byLife == 1)
		{
			m_pUserData->m_curx = m_fWill_x = pEvent->fPosX + x;
			m_pUserData->m_curz = m_fWill_z = pEvent->fPosZ + z;
			m_pUserData->m_cury = 0;
		}
		// Free Zone or Opposite Zone
		else if (m_pUserData->m_bNation != m_pUserData->m_bZone)
		{
			// Frontier Zone...
			if (m_pUserData->m_bZone > 200)
			{
				x = pHomeInfo->FreeZoneX + myrand(0, pHomeInfo->FreeZoneLX);
				z = pHomeInfo->FreeZoneZ + myrand(0, pHomeInfo->FreeZoneLZ);
			}
//
			// Battle Zone...
			else if (m_pUserData->m_bZone > 100
				&& m_pUserData->m_bZone < 200)
			{
/*
				m_bResHpType = USER_STANDING;
				HpChange( m_iMaxHp );
				KickOutZoneUser();	// Go back to your own zone!
				return;
*/
				x = pHomeInfo->BattleZoneX + myrand(0, pHomeInfo->BattleZoneLX);
				z = pHomeInfo->BattleZoneZ + myrand(0, pHomeInfo->BattleZoneLZ);
// 비러머글 개척존 바꾸어치기 >.<
				if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
				{
					x = pHomeInfo->FreeZoneX + myrand(0, pHomeInfo->FreeZoneLX);
					z = pHomeInfo->FreeZoneZ + myrand(0, pHomeInfo->FreeZoneLZ);
				}
//
			}
// 비러머글 뉴존 >.<
			else if (m_pUserData->m_bZone > 10 && m_pUserData->m_bZone < 20)
			{
				x = 527 + myrand(0, 10);
				z = 543 + myrand(0, 10);
			}
//
			// Specific Lands...
			else if (m_pUserData->m_bZone < 3)
			{
				if (m_pUserData->m_bNation == KARUS)
				{
					x = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
					z = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
				}
				else if (m_pUserData->m_bNation == ELMORAD)
				{
					x = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
					z = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
				}
				else return;
			}

			m_pUserData->m_curx = x;
			m_pUserData->m_curz = z;
		}
		//  추후에 Warp 랑 합쳐야 할것 같음...
		else
		{
			if (m_pUserData->m_bNation == KARUS)
			{
				x = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
				z = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
			}
			else if (m_pUserData->m_bNation == ELMORAD)
			{
				x = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
				z = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
			}
			else
			{
				return;
			}

			m_pUserData->m_curx = x;
			m_pUserData->m_curz = z;
		}
	}

	SetByte(send_buff, WIZ_REGENE, send_index);
//	SetShort( send_buff, m_Sid, send_index );    //
	SetShort(send_buff, (WORD) m_pUserData->m_curx * 10, send_index);
	SetShort(send_buff, (WORD) m_pUserData->m_curz * 10, send_index);
	SetShort(send_buff, (short) m_pUserData->m_cury * 10, send_index);
	Send(send_buff, send_index);
//	m_pMain->Send_Region( send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false ); //

// Clerical Resurrection.
	if (magicid > 0)
	{
		pType = m_pMain->m_MagicType5TableMap.GetData(magicid);
		if (pType == nullptr)
			return;

		m_bAbnormalType = ABNORMAL_BLINKING;
		m_bResHpType = USER_STANDING;
		m_fBlinkStartTime = TimeGet();
		MSpChange(-m_iMaxMp);					// Empty out MP.

		if (m_sWhoKilledMe == -1
			&& regene_type == 1)
			ExpChange((m_iLostExp * pType->ExpRecover) / 100);		// Restore Target Experience.

		m_bRegeneType = REGENE_MAGIC;
	}
	// Normal Regene.
	else
	{
		m_bAbnormalType = ABNORMAL_BLINKING;
		m_fBlinkStartTime = TimeGet();
//
		m_bResHpType = USER_STANDING;
//		HpChange( m_iMaxHp );
		m_bRegeneType = REGENE_NORMAL;
	}

//	비러머글 클랜 소환!!!
	m_fLastRegeneTime = TimeGet();
//
	m_sWhoKilledMe = -1;
	m_iLostExp = 0;
//	
	if (m_bAbnormalType != ABNORMAL_BLINKING)
	{
		// AI_server로 regene정보 전송...	
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_USER_REGENE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetShort(send_buff, m_pUserData->m_sHp, send_index);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
	}
//

	// 이 send_index는 왜 없었을까??? --;
#if defined(_DEBUG)
	{
		//TCHAR logstr[1024] = {};
		//_stprintf(logstr, _T("<------ User Regene ,, nid=%d, name=%hs, type=%d ******"), m_Sid, m_pUserData->m_id, m_bResHpType);
		//TimeTrace(logstr);
	}
#endif

	// 이거 확인사살로 추가했어요!!!!
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;

	m_RegionX = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	m_RegionZ = (int) (m_pUserData->m_curz / VIEW_DISTANCE);

	UserInOut(USER_REGENE);

	m_pMain->RegionUserInOutForMe(this);
	m_pMain->RegionNpcInfoForMe(this);

	// Send Blinking State Change.....
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, 3, send_index);
	SetByte(send_buff, m_bAbnormalType, send_index);
	StateChange(send_buff);
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;

	// Send Party Packet.....
	if (m_sPartyIndex != -1
		&& !m_bType3Flag)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetByte(send_buff, 1, send_index);
		SetByte(send_buff, 0x00, send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
	}
	//  end of Send Party Packet.....  //
//
//
	// Send Party Packet for Type 4
	if (m_sPartyIndex != -1
		&& !m_bType4Flag)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetByte(send_buff, 2, send_index);
		SetByte(send_buff, 0x00, send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
	}
	//  end of Send Party Packet.....  //
//
}

void CUser::ZoneChange(int zone, float x, float z)
{
	m_bZoneChangeFlag = true;

	int send_index = 0, zoneindex = 0;
	char send_buff[128] = {};
	C3DMap* pMap = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;

	if (g_serverdown_flag)
		return;

	zoneindex = m_pMain->GetZoneIndex(zone);

	pMap = m_pMain->GetMapByIndex(zoneindex);
	if (pMap == nullptr)
		return;

	// If Target zone is frontier zone.
	if (pMap->m_bType == 2)
	{
		if (m_pUserData->m_bLevel < 20
			&& m_pMain->m_byBattleOpen != SNOW_BATTLE)
			return;
	}

	// Battle zone open
	if (m_pMain->m_byBattleOpen == NATION_BATTLE)
	{
		if (m_pUserData->m_bZone == BATTLE_ZONE)
		{
			// 상대방 국가로 못넘어 가게..
			if (pMap->m_bType == 1
				&& m_pUserData->m_bNation != zone)
			{
				if (m_pUserData->m_bNation == KARUS
					&& !m_pMain->m_byElmoradOpenFlag)
				{
					spdlog::error("User::ZoneChange: zone not open for invasion [charId={} nation={} enemyNationOpen={}]",
						m_pUserData->m_id, m_pUserData->m_bNation, m_pMain->m_byElmoradOpenFlag);
					return;
				}
				
				if (m_pUserData->m_bNation == ELMORAD
					&& !m_pMain->m_byKarusOpenFlag)
				{
					spdlog::error("User::ZoneChange: zone not open for invasion [charId={} nation={} enemyNationOpen={}]",
						m_pUserData->m_id, m_pUserData->m_bNation, m_pMain->m_byKarusOpenFlag);
					return;
				}
			}
		}
		// 상대방 국가로 못넘어 가게..
		else if (pMap->m_bType == 1
			&& m_pUserData->m_bNation != zone)
		{
			return;
		}
//
		// You can't go to frontier zone when Battlezone is open.
		else if (pMap->m_bType == 2
			&& zone == ZONE_FRONTIER)
		{
			// 비러머글 마을 도착 공지....
			int temp_index = 0;
			char temp_buff[128] = {};

			SetByte(temp_buff, WIZ_WARP_LIST, temp_index);
			SetByte(temp_buff, 2, temp_index);
			SetByte(temp_buff, 0, temp_index);
			Send(temp_buff, temp_index);
//
			return;
		}
//
	}
	// Snow Battle zone open
	else if (m_pMain->m_byBattleOpen == SNOW_BATTLE)
	{
		// 상대방 국가로 못넘어 가게..
		if (pMap->m_bType == 1
			&& m_pUserData->m_bNation != zone)
			return;

		// You can't go to frontier zone when Battlezone is open.
		if (pMap->m_bType == 2
			&& (zone == ZONE_FRONTIER || zone == ZONE_BATTLE))
			return;
	}
	// Battle zone close
	else
	{
		// 상대방 국가로 못넘어 가게..
		if (pMap->m_bType == 1)
		{
			if (m_pUserData->m_bNation != zone
				&& zone > ZONE_MORADON
				&& zone != ZONE_ARENA)
				return;

			if (m_pUserData->m_bNation != zone
				&& zone < 3)
				return;
		}
	}

	m_bWarp = 0x01;

	UserInOut(USER_OUT);

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
	{
		//TRACE(_T("ZoneChange - name=%hs\n"), m_pUserData->m_id);
		SetMaxHp(1);
	}

	if (m_pUserData->m_bZone != zone)
		SetZoneAbilityChange(zone);

	m_iZoneIndex = zoneindex;
	m_pUserData->m_bZone = zone;
	m_pUserData->m_curx = m_fWill_x = x;
	m_pUserData->m_curz = m_fWill_z = z;

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
	{
		//TRACE(_T("ZoneChange - name=%hs\n"), m_pUserData->m_id);
		SetMaxHp();
	}

	PartyRemove(m_Sid);	// 파티에서 탈퇴되도록 처리

	//TRACE(_T("ZoneChange ,,, id=%hs, nation=%d, zone=%d, x=%.2f, z=%.2f\n"), m_pUserData->m_id, m_pUserData->m_bNation, zone, x, z);

	if (m_pMain->m_nServerNo != pMap->m_nServerNo)
	{
		pInfo = m_pMain->m_ServerArray.GetData(pMap->m_nServerNo);
		if (pInfo == nullptr)
			return;

		UserDataSaveToAgent();
		
		spdlog::debug("User::ZoneChange: [userId={} accountId={} charId={} zoneId={} x={} z={}]",
			m_Sid, m_strAccountID, m_pUserData->m_id, zone,
			static_cast<int32_t>(x), static_cast<int32_t>(z));
		
		m_pUserData->m_bLogout = 2;	// server change flag

		SetByte(send_buff, WIZ_SERVER_CHANGE, send_index);
		SetShort(send_buff, strlen(pInfo->strServerIP), send_index);
		SetString(send_buff, pInfo->strServerIP, strlen(pInfo->strServerIP), send_index);
		SetShort(send_buff, pInfo->sPort, send_index);
		SetByte(send_buff, 0x02, send_index);				// 중간에 서버가 바뀌는 경우...
		SetByte(send_buff, m_pUserData->m_bZone, send_index);
		SetByte(send_buff, m_pMain->m_byOldVictory, send_index);
		Send(send_buff, send_index);
		return;
	}

	m_pUserData->m_sBind = -1;		// Bind Point Clear...

	m_RegionX = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	m_RegionZ = (int) (m_pUserData->m_curz / VIEW_DISTANCE);

	SetByte(send_buff, WIZ_ZONE_CHANGE, send_index);
	SetByte(send_buff, ZONE_CHANGE_TELEPORT, send_index);
	SetByte(send_buff, m_pUserData->m_bZone, send_index);
	SetByte(send_buff, 0, send_index); // subzone
	SetShort(send_buff, (WORD) m_pUserData->m_curx * 10, send_index);
	SetShort(send_buff, (WORD) m_pUserData->m_curz * 10, send_index);
	SetShort(send_buff, (short) m_pUserData->m_cury * 10, send_index);
	SetByte(send_buff, m_pMain->m_byOldVictory, send_index);
	Send(send_buff, send_index);

	// 비러머글 순간이동 >.<
	if (!m_bZoneChangeSameZone)
	{
		m_sWhoKilledMe = -1;
		m_iLostExp = 0;
		m_bRegeneType = 0;
		m_fLastRegeneTime = 0.0f;
		m_pUserData->m_sBind = -1;
		InitType3();
		InitType4();
	}

	m_bZoneChangeSameZone = false;
//
	int ai_send_index = 0;
	char ai_send_buff[256] = {};
	SetByte(ai_send_buff, AG_ZONE_CHANGE, ai_send_index);
	SetShort(ai_send_buff, m_Sid, ai_send_index);
	SetByte(ai_send_buff, (BYTE) m_iZoneIndex, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bZone, ai_send_index);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, ai_send_buff, ai_send_index);

	m_bZoneChangeFlag = false;
}

void CUser::Warp(char* pBuf)
{
	if (m_bWarp)
		return;

//	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
//		return;

	int index = 0, send_index = 0;
	WORD warp_x, warp_z;
	float real_x, real_z;
	char send_buff[128] = {};
	C3DMap* pMap = nullptr;

	warp_x = GetShort(pBuf, index);
	warp_z = GetShort(pBuf, index);

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	real_x = warp_x / 10.0f;
	real_z = warp_z / 10.0f;

	if (!pMap->IsValidPosition(real_x, real_z, 0.0f))
		return;

	SetByte(send_buff, WIZ_WARP, send_index);
	SetShort(send_buff, warp_x, send_index);
	SetShort(send_buff, warp_z, send_index);
	Send(send_buff, send_index);

	UserInOut(USER_OUT);

	m_pUserData->m_curx = m_fWill_x = real_x;
	m_pUserData->m_curz = m_fWill_z = real_z;

	m_RegionX = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	m_RegionZ = (int) (m_pUserData->m_curz / VIEW_DISTANCE);

	//TRACE(_T(" Warp ,, name=%hs, x=%.2f, z=%.2f\n"), m_pUserData->m_id, m_pUserData->m_curx, m_pUserData->m_curz);

	//UserInOut( USER_IN );
	UserInOut(USER_WARP);
	m_pMain->RegionUserInOutForMe(this);
	m_pMain->RegionNpcInfoForMe(this);
}

void CUser::SendTimeStatus()
{
	char send_buff[256] = {};
	int send_index = 0;

	SetByte(send_buff, WIZ_TIME, send_index);
	SetShort(send_buff, m_pMain->m_nYear, send_index);
	SetShort(send_buff, m_pMain->m_nMonth, send_index);
	SetShort(send_buff, m_pMain->m_nDate, send_index);
	SetShort(send_buff, m_pMain->m_nHour, send_index);
	SetShort(send_buff, m_pMain->m_nMin, send_index);
	Send(send_buff, send_index);

	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, WIZ_WEATHER, send_index);
	SetByte(send_buff, (BYTE) m_pMain->m_nWeather, send_index);
	SetShort(send_buff, m_pMain->m_nAmount, send_index);
	Send(send_buff, send_index);
}

void CUser::SetDetailData()
{
	SetSlotItemValue();
	SetUserAbility();

	if (m_pUserData->m_bLevel > MAX_LEVEL)
	{
		spdlog::error("User::SetDetailData: user exceeds max level [accountId={} charId={} level={}]",
			m_pUserData->m_Accountid, m_pUserData->m_id, m_pUserData->m_bLevel);

		Close();
		return;
	}

	m_iMaxExp = m_pMain->m_LevelUpTableArray[m_pUserData->m_bLevel - 1]->RequiredExp;
	m_iMaxWeight = (m_pUserData->m_bStr + m_sItemStr) * 50;

	m_iZoneIndex = m_pMain->GetZoneIndex(m_pUserData->m_bZone);

	// 이 서버에 없는 존....
	if (m_iZoneIndex == -1)
		Close();

	m_fWill_x = m_pUserData->m_curx;
	m_fWill_z = m_pUserData->m_curz;
	m_fWill_y = m_pUserData->m_cury;

	m_RegionX = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	m_RegionZ = (int) (m_pUserData->m_curz / VIEW_DISTANCE);
}

void CUser::RegisterRegion()
{
	int iRegX = 0, iRegZ = 0, old_region_x = 0, old_region_z = 0;

	iRegX = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	iRegZ = (int) (m_pUserData->m_curz / VIEW_DISTANCE);

	if (m_RegionX != iRegX
		|| m_RegionZ != iRegZ)
	{
		C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
		if (pMap == nullptr)
			return;

		old_region_x = m_RegionX;
		old_region_z = m_RegionZ;

		pMap->RegionUserRemove(m_RegionX, m_RegionZ, m_Sid);
		m_RegionX = iRegX;	
		m_RegionZ = iRegZ;
		pMap->RegionUserAdd(m_RegionX, m_RegionZ, m_Sid);

		if (m_State == STATE_GAMESTART)
		{
			// delete user 는 계산 방향이 진행방향의 반대...
			RemoveRegion(old_region_x - m_RegionX, old_region_z - m_RegionZ);

			// add user 는 계산 방향이 진행방향...
			InsertRegion(m_RegionX - old_region_x, m_RegionZ - old_region_z);

			m_pMain->RegionNpcInfoForMe(this);
			m_pMain->RegionUserInOutForMe(this);
		}

		// TRACE(_T("User를 Region에 등록,, region_x=%d, y=%d\n"), m_RegionX, m_RegionZ);
	}
}

void CUser::RemoveRegion(int del_x, int del_z)
{
	int send_index = 0;
	char send_buff[256] = {};
	CUser* pUser = nullptr;

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	SetByte(send_buff, WIZ_USER_INOUT, send_index);
	SetByte(send_buff, USER_OUT, send_index);
	SetShort(send_buff, m_Sid, send_index);

	// x 축으로 이동되었을때...
	if (del_x != 0)
	{
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x * 2, m_RegionZ + del_z - 1);
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x * 2, m_RegionZ + del_z);
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x * 2, m_RegionZ + del_z + 1);

		// TRACE(_T("Remove : (%d %d), (%d %d), (%d %d)\n"), m_RegionX+del_x*2, m_RegionZ+del_z-1, m_RegionX+del_x*2, m_RegionZ+del_z, m_RegionX+del_x*2, m_RegionZ+del_z+1 );
	}

	// z 축으로 이동되었을때...
	if (del_z != 0)
	{
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x, m_RegionZ + del_z * 2);

		// x, z 축 둘다 이동되었을때 겹치는 부분 한번만 보낸다..
		if (del_x < 0)
		{
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x + 1, m_RegionZ + del_z * 2);
		}
		else if (del_x > 0)
		{
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x - 1, m_RegionZ + del_z * 2);
		}
		else
		{
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x - 1, m_RegionZ + del_z * 2);
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x + 1, m_RegionZ + del_z * 2);

			// TRACE(_T("Remove : (%d %d), (%d %d), (%d %d)\n"), m_RegionX+del_x-1, m_RegionZ+del_z*2, m_RegionX+del_x, m_RegionZ+del_z*2, m_RegionX+del_x+1, m_RegionZ+del_z*2 );
		}
	}
}

void CUser::InsertRegion(int del_x, int del_z)
{
	int send_index = 0;
	char send_buff[256] = {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	SetByte(send_buff, WIZ_USER_INOUT, send_index);
	SetByte(send_buff, USER_IN, send_index);
	SetShort(send_buff, m_Sid, send_index);
	GetUserInfo(send_buff, send_index);

	// x 축으로 이동되었을때...
	if (del_x != 0)
	{
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x, m_RegionZ - 1);
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x, m_RegionZ);
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + del_x, m_RegionZ + 1);

		// TRACE(_T("Insert : (%d %d), (%d %d), (%d %d)\n"), m_RegionX+del_x, m_RegionZ-1, m_RegionX+del_x, m_RegionZ, m_RegionX+del_x, m_RegionZ+1 );
	}

	// z 축으로 이동되었을때...
	if (del_z != 0)
	{
		m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX, m_RegionZ + del_z);
		
		// x, z 축 둘다 이동되었을때 겹치는 부분 한번만 보낸다..
		if (del_x < 0)
		{
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + 1, m_RegionZ + del_z);
		}
		else if (del_x > 0)
		{
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX - 1, m_RegionZ + del_z);
		}
		else
		{
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX - 1, m_RegionZ + del_z);
			m_pMain->Send_UnitRegion(pMap, send_buff, send_index, m_RegionX + 1, m_RegionZ + del_z);

			// TRACE(_T("Insert : (%d %d), (%d %d), (%d %d)\n"), m_RegionX-1, m_RegionZ+del_z, m_RegionX, m_RegionZ+del_z, m_RegionX+1, m_RegionZ+del_z );
		}
	}
}

void CUser::RequestUserIn(char* pBuf)
{
	int index = 0, user_count = 0, send_index = 0, t_count = 0;
	char send_buff[40960] = {};

	send_index = 3;	// packet command 와 user_count 는 나중에 셋팅한다...
	user_count = GetShort(pBuf, index);
	for (int i = 0; i < user_count; i++)
	{
		short uid = GetShort(pBuf, index);
		if (uid < 0
			|| uid >= MAX_USER)
			continue;

		if (i > 1000)
			break;

		CUser* pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[uid];
		if (pUser == nullptr
			|| pUser->GetState() != STATE_GAMESTART)
			continue;

		SetShort(send_buff, pUser->GetSocketID(), send_index);
		pUser->GetUserInfo(send_buff, send_index);

		++t_count;
	}

	int temp_index = 0;
	SetByte(send_buff, WIZ_REQ_USERIN, temp_index);
	SetShort(send_buff, t_count, temp_index);

	if (send_index < 500)
		Send(send_buff, send_index);
	else
		SendCompressingPacket(send_buff, send_index);
}

void CUser::RequestNpcIn(char* pBuf)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return;

	int index = 0, npc_count = 0, send_index = 0, t_count = 0;
	char send_buff[20480] = {};

	send_index = 3;	// packet command 와 user_count 는 나중에 셋팅한다...
	npc_count = GetShort(pBuf, index);
	for (int i = 0; i < npc_count; i++)
	{
		short nid = GetShort(pBuf, index);
		if (nid < 0
			|| nid > NPC_BAND + NPC_BAND)
			continue;

		if (i > 1000)
			break;

		CNpc* pNpc = m_pMain->m_NpcMap.GetData(nid);
		if (pNpc == nullptr)
			continue;

		// 음냥,,
		// if (pNpc->m_NpcState != NPC_LIVE)
		//	continue;

		SetShort(send_buff, pNpc->m_sNid, send_index);
		pNpc->GetNpcInfo(send_buff, send_index);

		++t_count;
	}

	int temp_index = 0;
	SetByte(send_buff, WIZ_REQ_NPCIN, temp_index);
	SetShort(send_buff, t_count, temp_index);

	if (send_index < 500)
		Send(send_buff, send_index);
	else
		SendCompressingPacket(send_buff, send_index);
}

BYTE CUser::GetHitRate(float rate)
{
	BYTE result = FAIL;
	int random = myrand(1, 10000);

	if (rate >= 5.0f)
	{
		if (random >= 1
			&& random <= 3500)
			result = GREAT_SUCCESS;
		else if (random >= 3501
			&& random <= 7500)
			result = SUCCESS;
		else if (random >= 7501
			&& random <= 9800)
			result = NORMAL;
	}
	else if (rate >= 3.0f)
	{
		if (random >= 1
			&& random <= 2500)
			result = GREAT_SUCCESS;
		else if (random >= 2501
			&& random <= 6000)
			result = SUCCESS;
		else if (random >= 6001
			&& random <= 9600)
			result = NORMAL;
	}
	else if (rate >= 2.0f)
	{
		if (random >= 1
			&& random <= 2000)
			result = GREAT_SUCCESS;
		else if (random >= 2001
			&& random <= 5000)
			result = SUCCESS;
		else if (random >= 5001
			&& random <= 9400)
			result = NORMAL;
	}
	else if (rate >= 1.25f)
	{
		if (random >= 1
			&& random <= 1500)
			result = GREAT_SUCCESS;
		else if (random >= 1501
			&& random <= 4000)
			result = SUCCESS;
		else if (random >= 4001
			&& random <= 9200)
			result = NORMAL;
	}
	else if (rate >= 0.8f)
	{
		if (random >= 1
			&& random <= 1000)
			result = GREAT_SUCCESS;
		else if (random >= 1001
			&& random <= 3000)
			result = SUCCESS;
		else if (random >= 3001
			&& random <= 9000)
			result = NORMAL;
	}
	else if (rate >= 0.5f)
	{
		if (random >= 1
			&& random <= 800)
			result = GREAT_SUCCESS;
		else if (random >= 801
			&& random <= 2500)
			result = SUCCESS;
		else if (random >= 2501
			&& random <= 8000)
			result = NORMAL;
	}
	else if (rate >= 0.33f)
	{
		if (random >= 1
			&& random <= 600)
			result = GREAT_SUCCESS;
		else if (random >= 601
			&& random <= 2000)
			result = SUCCESS;
		else if (random >= 2001
			&& random <= 7000)
			result = NORMAL;
	}
	else if (rate >= 0.2f)
	{
		if (random >= 1
			&& random <= 400)
			result = GREAT_SUCCESS;
		else if (random >= 401
			&& random <= 1500)
			result = SUCCESS;
		else if (random >= 1501
			&& random <= 6000)
			result = NORMAL;
	}
	else
	{
		if (random >= 1
			&& random <= 200)
			result = GREAT_SUCCESS;
		else if (random >= 201
			&& random <= 1000)
			result = SUCCESS;
		else if (random >= 1001
			&& random <= 5000)
			result = NORMAL;
	}

	return result;
}

// 착용한 아이템의 값(타격률, 회피율, 데미지)을 구한다.
void CUser::SetSlotItemValue()
{
	model::Item* pTable = nullptr;
	int item_hit = 0, item_ac = 0;

	m_sItemMaxHp = m_sItemMaxMp = 0;
	m_sItemHit = m_sItemAc = m_sItemStr = m_sItemSta = m_sItemDex = m_sItemIntel = m_sItemCham = 0;
	m_sItemHitrate = m_sItemEvasionrate = 100;
	m_iItemWeight = 0;

	m_bFireR = m_bColdR = m_bLightningR = m_bMagicR = m_bDiseaseR = m_bPoisonR = 0;
	m_sDaggerR = m_sSwordR = m_sAxeR = m_sMaceR = m_sSpearR = m_sBowR = 0;

	m_bMagicTypeLeftHand = m_bMagicTypeRightHand = 0;
	m_sMagicAmountLeftHand = m_sMagicAmountRightHand = 0;

	for (int i = 0; i < SLOT_MAX; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum <= 0)
			continue;

		pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[i].nNum);
		if (pTable == nullptr)
			continue;

		if (m_pUserData->m_sItemArray[i].sDuration == 0)
		{
			item_hit = pTable->Damage / 2;
			item_ac = pTable->Armor / 2;
		}
		else
		{
			item_hit = pTable->Damage;
			item_ac = pTable->Armor;
		}

		if (i == RIGHTHAND) 	// ItemHit Only Hands
			m_sItemHit += item_hit;

		if (i == LEFTHAND)
		{
			if ((m_pUserData->m_sClass == BERSERKER
				|| m_pUserData->m_sClass == BLADE))
	//			m_sItemHit += item_hit * (double) (m_pUserData->m_bstrSkill[PRO_SKILL1] / 60.0);    // 성래씨 요청 ^^;
				m_sItemHit += item_hit * 0.5f;
		}

		m_sItemMaxHp += pTable->MaxHpBonus;
		m_sItemMaxMp += pTable->MaxMpBonus;
		m_sItemAc += item_ac;
		m_sItemStr += pTable->StrengthBonus;
		m_sItemSta += pTable->StaminaBonus;
		m_sItemDex += pTable->DexterityBonus;
		m_sItemIntel += pTable->IntelligenceBonus;
		m_sItemCham += pTable->CharismaBonus;
		m_sItemHitrate += pTable->HitRate;
		m_sItemEvasionrate += pTable->EvasionRate;
//		m_iItemWeight += pTable->Weight;

		m_bFireR += pTable->FireResist;
		m_bColdR += pTable->ColdResist;
		m_bLightningR += pTable->LightningResist;
		m_bMagicR += pTable->MagicResist;
		m_bDiseaseR += pTable->CurseResist;
		m_bPoisonR += pTable->PoisonResist;

		m_sDaggerR += pTable->DaggerArmor;
		m_sSwordR += pTable->SwordArmor;
		m_sAxeR += pTable->AxeArmor;
		m_sMaceR += pTable->MaceArmor;
		m_sSpearR += pTable->SpearArmor;
		m_sBowR += pTable->BowArmor;
	}

	// Also add the weight of items in the inventory....
	for (int i = 0; i < HAVE_MAX + SLOT_MAX; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum <= 0)
			continue;

		pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[i].nNum);
		if (pTable == nullptr)
			continue;

		// Non-countable items.
		if (pTable->Countable == 0)
			m_iItemWeight += pTable->Weight;
		// Countable items.
		else
			m_iItemWeight += pTable->Weight * m_pUserData->m_sItemArray[i].sCount;
	}

	if (m_sItemHit < 3)
		m_sItemHit = 3;

	// For magical items..... by Yookozuna 2002.7.10

	// Get item info for left hand.
	model::Item* pLeftHand
		= m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[LEFTHAND].nNum);
	if (pLeftHand != nullptr)
	{
		if (pLeftHand->FireDamage != 0)
		{
			m_bMagicTypeLeftHand = 1;
			m_sMagicAmountLeftHand = pLeftHand->FireDamage;
		}

		if (pLeftHand->IceDamage != 0)
		{
			m_bMagicTypeLeftHand = 2;
			m_sMagicAmountLeftHand = pLeftHand->IceDamage;
		}

		if (pLeftHand->LightningDamage != 0)
		{
			m_bMagicTypeLeftHand = 3;
			m_sMagicAmountLeftHand = pLeftHand->LightningDamage;
		}

		if (pLeftHand->PoisonDamage != 0)
		{
			m_bMagicTypeLeftHand = 4;
			m_sMagicAmountLeftHand = pLeftHand->PoisonDamage;
		}

		if (pLeftHand->HpDrain != 0)
		{
			m_bMagicTypeLeftHand = 5;
			m_sMagicAmountLeftHand = pLeftHand->HpDrain;
		}

		if (pLeftHand->MpDamage != 0)
		{
			m_bMagicTypeLeftHand = 6;
			m_sMagicAmountLeftHand = pLeftHand->MpDamage;
		}

		if (pLeftHand->MpDrain != 0)
		{
			m_bMagicTypeLeftHand = 7;
			m_sMagicAmountLeftHand = pLeftHand->MpDrain;
		}

		if (pLeftHand->MirrorDamage != 0)
		{
			m_bMagicTypeLeftHand = 8;
			m_sMagicAmountLeftHand = pLeftHand->MirrorDamage;
		}
	}

	// Get item info for right hand.
	model::Item* pRightHand
		= m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
	if (pRightHand != nullptr)
	{
		if (pRightHand->FireDamage != 0)
		{
			m_bMagicTypeRightHand = 1;
			m_sMagicAmountRightHand = pRightHand->FireDamage;
		}

		if (pRightHand->IceDamage != 0)
		{
			m_bMagicTypeRightHand = 2;
			m_sMagicAmountRightHand = pRightHand->IceDamage;
		}

		if (pRightHand->LightningDamage != 0)
		{
			m_bMagicTypeRightHand = 3;
			m_sMagicAmountRightHand = pRightHand->LightningDamage;
		}

		if (pRightHand->PoisonDamage != 0)
		{
			m_bMagicTypeRightHand = 4;
			m_sMagicAmountRightHand = pRightHand->PoisonDamage;
		}

		if (pRightHand->HpDrain != 0)
		{
			m_bMagicTypeRightHand = 5;
			m_sMagicAmountRightHand = pRightHand->HpDrain;
		}

		if (pRightHand->MpDamage != 0)
		{
			m_bMagicTypeRightHand = 6;
			m_sMagicAmountRightHand = pRightHand->MpDamage;
		}

		if (pRightHand->MpDrain != 0)
		{
			m_bMagicTypeRightHand = 7;
			m_sMagicAmountRightHand = pRightHand->MpDrain;
		}

		if (pRightHand->MirrorDamage != 0)
		{
			m_bMagicTypeRightHand = 8;
			m_sMagicAmountRightHand = pRightHand->MirrorDamage;
		}
	}
}

short CUser::GetDamage(short tid, int magicid)
{
	short damage = 0;
	int random = 0;
	short common_damage = 0, temp_hit = 0, temp_ac = 0, temp_hit_B = 0;
	BYTE result = FAIL;

	model::Magic* pTable = nullptr;
	model::MagicType1* pType1 = nullptr;
	model::MagicType2* pType2 = nullptr;

	// Check if target id is valid.
	if (tid < 0
		|| tid >= MAX_USER)
		return -1;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];	   // Get target info.
	if (pTUser == nullptr
		|| pTUser->m_bResHpType == USER_DEAD)
		return -1;

	temp_ac = pTUser->m_sTotalAc + pTUser->m_sACAmount;    // 표시   
	temp_hit_B = (m_sTotalHit * m_bAttackAmount * 200 / 100) / (temp_ac + 240);   // 표시

	// Skill/Arrow hit.    
	if (magicid > 0)
	{
		pTable = m_pMain->m_MagicTableMap.GetData(magicid);		// Get main magic table.
		if (pTable == nullptr)
			return -1;

		// SKILL HIT!
		if (pTable->Type1 == 1)
		{
			pType1 = m_pMain->m_MagicType1TableMap.GetData(magicid);	// Get magic skill table type 1.
			if (pType1 == nullptr)
				return -1;

			// Non-relative hit.
			if (pType1->Type)
			{
				random = myrand(0, 100);
				if (pType1->HitRateMod <= random)
					result = FAIL;
				else
					result = SUCCESS;
			}
			// Relative hit.
			else
			{
				result = GetHitRate((m_sTotalHitrate / pTUser->m_sTotalEvasionrate) * (pType1->HitRateMod / 100.0f));
			}

			temp_hit = temp_hit_B * (pType1->DamageMod / 100.0f);
		}
		// ARROW HIT!
		else if (pTable->Type1 == 2)
		{
			pType2 = m_pMain->m_MagicType2TableMap.GetData(magicid);	// Get magic skill table type 1.
			if (pType2 == nullptr)
				return -1;

			// Non-relative/Penetration hit.
			if (pType2->HitType == 1
				|| pType2->HitType == 2)
			{
				random = myrand(0, 100);

				if (pType2->HitRateMod <= random)
					result = FAIL;
				else
					result = SUCCESS;
			}
			// Relative hit/Arc hit.
			else
			{
				result = GetHitRate((m_sTotalHitrate / pTUser->m_sTotalEvasionrate) * (pType2->HitRateMod / 100.0f));
			}

			if (pType2->HitType == 1
				/*|| pType2->HitType == 2*/)
				temp_hit = m_sTotalHit * m_bAttackAmount * (pType2->DamageMod / 100.0f) / 100;   // 표시
			else
				temp_hit = temp_hit_B * (pType2->DamageMod / 100.0f);
		}
	}
	// Normal Hit.
	else
	{
		temp_hit = m_sTotalHit * m_bAttackAmount / 100;	// 표시
		result = GetHitRate(m_sTotalHitrate / pTUser->m_sTotalEvasionrate);
	}

	// 1. Magical item damage....
	switch (result)
	{
		case GREAT_SUCCESS:
		case SUCCESS:
		case NORMAL:
			// Skill Hit.
			if (magicid > 0)
			{
				random = myrand(0, temp_hit);
				if (pTable->Type1 == 1)
					damage = static_cast<int16_t>((temp_hit + 0.3f * random) + 0.99f);
				else
					damage = static_cast<int16_t>(((temp_hit * 0.6f) + 1.0f * random) + 0.99f);
			}
			// Normal Hit.	
			else
			{
				random = myrand(0, temp_hit_B);
				damage = static_cast<int16_t>((0.85f * temp_hit_B) + 0.3f * random);
			}

			break;

		case FAIL:
		default:
			damage = 0;
	}

	damage = GetMagicDamage(damage, tid);	// 2. Magical item damage....	
	damage = GetACDamage(damage, tid);		// 3. Additional AC calculation....	
//	damage = damage / 2;	// 성래씨 추가 요청!!!!
	damage = damage / 3;	// 성래씨 추가 요청!!!!  

	return damage;
}

short CUser::GetMagicDamage(int damage, short tid)
{
	short total_r = 0;
	short temp_damage = 0;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];	   // Get target info.
	if (pTUser == nullptr
		|| pTUser->m_bResHpType == USER_DEAD)
		return damage;

	// RIGHT HAND!!! by Yookozuna
	if (m_bMagicTypeRightHand > 4
		&& m_bMagicTypeRightHand < 8)
		temp_damage = damage * m_sMagicAmountRightHand / 100;

	// RIGHT HAND!!!
	switch (m_bMagicTypeRightHand)
	{
		// Fire Damage
		case ITEM_TYPE_FIRE:
			total_r = pTUser->m_bFireR + pTUser->m_bFireRAmount;
			break;

		// Ice Damage
		case ITEM_TYPE_COLD:
			total_r = pTUser->m_bColdR + pTUser->m_bColdRAmount;
			break;

		// Lightning Damage
		case ITEM_TYPE_LIGHTNING:
			total_r = pTUser->m_bLightningR + pTUser->m_bLightningRAmount;
			break;

		// Poison Damage
		case ITEM_TYPE_POISON:
			total_r = pTUser->m_bPoisonR + pTUser->m_bPoisonRAmount;
			break;

		// HP Drain
		case ITEM_TYPE_HP_DRAIN:
			HpChange(temp_damage, 0);
			break;

		// MP Damage
		case ITEM_TYPE_MP_DAMAGE:
			pTUser->MSpChange(-temp_damage);
			break;

		// MP Drain
		case ITEM_TYPE_MP_DRAIN:
			MSpChange(temp_damage);
			break;
	}

	if (m_bMagicTypeRightHand > 0
		&& m_bMagicTypeRightHand < 5)
	{
		if (total_r > 200)
			total_r = 200;

		temp_damage = m_sMagicAmountRightHand - m_sMagicAmountRightHand * total_r / 200;
		damage = damage + temp_damage;
	}

	// Reset all temporary data.
	total_r = 0;
	temp_damage = 0;

	// LEFT HAND!!! by Yookozuna
	if (m_bMagicTypeLeftHand > 4
		&& m_bMagicTypeLeftHand < 8)
		temp_damage = damage * m_sMagicAmountLeftHand / 100;

	// LEFT HAND!!!
	switch (m_bMagicTypeLeftHand)
	{
		// Fire Damage
		case ITEM_TYPE_FIRE:
			total_r = pTUser->m_bFireR + pTUser->m_bFireRAmount;
			break;

		// Ice Damage
		case ITEM_TYPE_COLD:
			total_r = pTUser->m_bColdR + pTUser->m_bColdRAmount;
			break;

		// Lightning Damage
		case ITEM_TYPE_LIGHTNING:
			total_r = pTUser->m_bLightningR + pTUser->m_bLightningRAmount;
			break;

		// Poison Damage
		case ITEM_TYPE_POISON:
			total_r = pTUser->m_bPoisonR + pTUser->m_bPoisonRAmount;
			break;

		// HP Drain
		case ITEM_TYPE_HP_DRAIN:
			HpChange(temp_damage, 0);
			break;

		// MP Damage
		case ITEM_TYPE_MP_DAMAGE:
			pTUser->MSpChange(-temp_damage);
			break;

		// MP Drain
		case ITEM_TYPE_MP_DRAIN:
			MSpChange(temp_damage);
			break;
	}

	if (m_bMagicTypeLeftHand > 0
		&& m_bMagicTypeLeftHand < 5)
	{
		if (total_r > 200)
			total_r = 200;

		temp_damage = m_sMagicAmountLeftHand - m_sMagicAmountLeftHand * total_r / 200;
		damage = damage + temp_damage;
	}

	// Reset all temporary data.
	total_r = 0;
	temp_damage = 0;

	// Mirror Attack Check routine.
	if (pTUser->m_bMagicTypeLeftHand == ITEM_TYPE_MIRROR_DAMAGE)
	{
		temp_damage = damage * pTUser->m_sMagicAmountLeftHand / 100;
		HpChange(-temp_damage);		// Reflective Hit.
	}

	return damage;
}

short CUser::GetACDamage(int damage, short tid)
{
	model::Item* pLeftHand = nullptr;
	model::Item* pRightHand = nullptr;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];	   // Get target info.
	if (pTUser == nullptr
		|| pTUser->m_bResHpType == USER_DEAD)
		return damage;

	if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
	{
		pRightHand = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
		if (pRightHand != nullptr)
		{
			// Weapon Type Right Hand....
			switch (pRightHand->Kind / 10)
			{
				case WEAPON_DAGGER:
					damage -= damage * pTUser->m_sDaggerR / 200;
					break;

				case WEAPON_SWORD:
					damage -= damage * pTUser->m_sSwordR / 200;
					break;

				case WEAPON_AXE:
					damage -= damage * pTUser->m_sAxeR / 200;
					break;

				case WEAPON_MACE:
					damage -= damage * pTUser->m_sMaceR / 200;
					break;

				case WEAPON_SPEAR:
					damage -= damage * pTUser->m_sSpearR / 200;
					break;

				case WEAPON_BOW:
					damage -= damage * pTUser->m_sBowR / 200;
					break;
			}
		}
	}

	if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0)
	{
		pLeftHand = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[LEFTHAND].nNum);
		if (pLeftHand != nullptr)
		{
			// Weapon Type Right Hand....
			switch (pLeftHand->Kind / 10)
			{
				case WEAPON_DAGGER:
					damage -= damage * pTUser->m_sDaggerR / 200;
					break;

				case WEAPON_SWORD:
					damage -= damage * pTUser->m_sSwordR / 200;
					break;

				case WEAPON_AXE:
					damage -= damage * pTUser->m_sAxeR / 200;
					break;

				case WEAPON_MACE:
					damage -= damage * pTUser->m_sMaceR / 200;
					break;

				case WEAPON_SPEAR:
					damage -= damage * pTUser->m_sSpearR / 200;
					break;

				case WEAPON_BOW:
					damage -= damage * pTUser->m_sBowR / 200;
					break;
			}
		}
	}

	return damage;
}

void CUser::ExpChange(int iExp)
{
	char send_buff[256] = {};
	int send_index = 0;

	if (m_pUserData->m_bLevel < 6
		&& iExp < 0)
		return;

	if (m_pUserData->m_bZone == ZONE_BATTLE
		&& iExp < 0)
		return;

/*
	if (m_pUserData->m_bZone != m_pUserData->m_bNation
		&& m_pUserData->m_bZone < 3
		&& iExp < 0)
		iExp = iExp / 10;
*/

	m_pUserData->m_iExp += iExp;

	if (m_pUserData->m_iExp < 0)
	{
		if (m_pUserData->m_bLevel > 5)
		{
			--m_pUserData->m_bLevel;
			m_pUserData->m_iExp += m_pMain->m_LevelUpTableArray[m_pUserData->m_bLevel - 1]->RequiredExp;
			LevelChange(m_pUserData->m_bLevel, false);
			return;
		}
	}
	else if (m_pUserData->m_iExp >= m_iMaxExp)
	{
		if (m_pUserData->m_bLevel >= MAX_LEVEL)
		{
			m_pUserData->m_iExp = m_iMaxExp;
			return;
		}

		m_pUserData->m_iExp = m_pUserData->m_iExp - m_iMaxExp;
		++m_pUserData->m_bLevel;

		LevelChange(m_pUserData->m_bLevel);
		return;
	}

	SetByte(send_buff, WIZ_EXP_CHANGE, send_index);
	SetDWORD(send_buff, m_pUserData->m_iExp, send_index);
	Send(send_buff, send_index);

	if (iExp < 0)
		m_iLostExp = -iExp;
}

void CUser::LevelChange(short level, bool bLevelUp)
{
	if (level < 1
		|| level > MAX_LEVEL)
		return;

	char send_buff[256] = {};
	int send_index = 0;

	if (bLevelUp)
	{
		if ((m_pUserData->m_bPoints + m_pUserData->m_bSta + m_pUserData->m_bStr + m_pUserData->m_bDex + m_pUserData->m_bIntel + m_pUserData->m_bCha) < (300 + 3 * (level - 1)))
			m_pUserData->m_bPoints += 3;

		if (level > 9
			&& (m_pUserData->m_bstrSkill[0] + m_pUserData->m_bstrSkill[1] + m_pUserData->m_bstrSkill[2] + m_pUserData->m_bstrSkill[3] + m_pUserData->m_bstrSkill[4]
			+ m_pUserData->m_bstrSkill[5] + m_pUserData->m_bstrSkill[6] + m_pUserData->m_bstrSkill[7] + m_pUserData->m_bstrSkill[8]) < (2 * (level - 9)))
			m_pUserData->m_bstrSkill[0] += 2;	// Skill Points up
	}

	m_iMaxExp = m_pMain->m_LevelUpTableArray[level - 1]->RequiredExp;

	SetSlotItemValue();
	SetUserAbility();

	m_pUserData->m_sMp = m_iMaxMp;
	HpChange(m_iMaxHp);

	Send2AI_UserUpdateInfo();

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_LEVEL_CHANGE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetByte(send_buff, m_pUserData->m_bLevel, send_index);
	SetByte(send_buff, m_pUserData->m_bPoints, send_index);
	SetByte(send_buff, m_pUserData->m_bstrSkill[0], send_index);
	SetDWORD(send_buff, m_iMaxExp, send_index);
	SetDWORD(send_buff, m_pUserData->m_iExp, send_index);
	SetShort(send_buff, m_iMaxHp, send_index);
	SetShort(send_buff, m_pUserData->m_sHp, send_index);
	SetShort(send_buff, m_iMaxMp, send_index);
	SetShort(send_buff, m_pUserData->m_sMp, send_index);
	SetShort(send_buff, GetMaxWeightForClient(), send_index);
	SetShort(send_buff, GetCurrentWeightForClient(), send_index);
	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ);

	if (m_sPartyIndex != -1)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_LEVELCHANGE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetByte(send_buff, m_pUserData->m_bLevel, send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
	}
}

void CUser::PointChange(char* pBuf)
{
	int index = 0, send_index = 0, value = 0;
	BYTE type = 0x00;
	char send_buff[128] = {};

	type = GetByte(pBuf, index);
	value = GetShort(pBuf, index);

	if (type > 5
		|| abs(value) > 1)
		return;

	if (m_pUserData->m_bPoints < 1)
		return;

	switch (type)
	{
		case STAT_TYPE_STR:
			if (m_pUserData->m_bStr == 255)
				return;
			break;

		case STAT_TYPE_STA:
			if (m_pUserData->m_bSta == 255)
				return;
			break;

		case STAT_TYPE_DEX:
			if (m_pUserData->m_bDex == 255)
				return;
			break;

		case STAT_TYPE_INTEL:
			if (m_pUserData->m_bIntel == 255)
				return;
			break;

		case STAT_TYPE_CHA:
			if (m_pUserData->m_bCha == 255)
				return;
			break;
	}

	m_pUserData->m_bPoints -= value;

	SetByte(send_buff, WIZ_POINT_CHANGE, send_index);
	SetByte(send_buff, type, send_index);
	switch (type)
	{
		case STAT_TYPE_STR:
			++m_pUserData->m_bStr;
			SetShort(send_buff, m_pUserData->m_bStr, send_index);
			SetUserAbility();
			break;

		case STAT_TYPE_STA:
			++m_pUserData->m_bSta;
			SetShort(send_buff, m_pUserData->m_bSta, send_index);
			SetMaxHp();
			SetMaxMp();
			break;

		case STAT_TYPE_DEX:
			++m_pUserData->m_bDex;
			SetShort(send_buff, m_pUserData->m_bDex, send_index);
			SetUserAbility();
			break;

		case STAT_TYPE_INTEL:
			++m_pUserData->m_bIntel;
			SetShort(send_buff, m_pUserData->m_bIntel, send_index);
			SetMaxMp();
			break;

		case STAT_TYPE_CHA:
			++m_pUserData->m_bCha;
			SetShort(send_buff, m_pUserData->m_bCha, send_index);
			break;
	}

	SetShort(send_buff, m_iMaxHp, send_index);
	SetShort(send_buff, m_iMaxMp, send_index);
	SetShort(send_buff, m_sTotalHit, send_index);
	SetShort(send_buff, GetMaxWeightForClient(), send_index);
	Send(send_buff, send_index);
}

// type : Received From AIServer -> 1, The Others -> 0
// attack : Direct Attack(true) or Other Case(false)
void CUser::HpChange(int amount, int type, bool attack)
{
	char send_buff[256] = {};
	int send_index = 0;

	m_pUserData->m_sHp += amount;
	if (m_pUserData->m_sHp < 0)
		m_pUserData->m_sHp = 0;
	else if (m_pUserData->m_sHp > m_iMaxHp)
		m_pUserData->m_sHp = m_iMaxHp;

	SetByte(send_buff, WIZ_HP_CHANGE, send_index);
	SetShort(send_buff, m_iMaxHp, send_index);
	SetShort(send_buff, m_pUserData->m_sHp, send_index);
	Send(send_buff, send_index);

	if (type == 0)
	{
		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));

		SetByte(send_buff, AG_USER_SET_HP, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetDWORD(send_buff, m_pUserData->m_sHp, send_index);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
	}

	if (m_sPartyIndex != -1)
	{
		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));

		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_HPCHANGE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetShort(send_buff, m_iMaxHp, send_index);
		SetShort(send_buff, m_pUserData->m_sHp, send_index);
		SetShort(send_buff, m_iMaxMp, send_index);
		SetShort(send_buff, m_pUserData->m_sMp, send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
	}

	// 직접 가격해서 죽는경우는 Dead Packet 없음
	if (m_pUserData->m_sHp == 0
		&& !attack)
		Dead();
}

void CUser::MSpChange(int amount)
{
	char send_buff[256] = {};
	int send_index = 0;

	m_pUserData->m_sMp += amount;
	if (m_pUserData->m_sMp < 0)
		m_pUserData->m_sMp = 0;
	else if (m_pUserData->m_sMp > m_iMaxMp)
		m_pUserData->m_sMp = m_iMaxMp;

	SetByte(send_buff, WIZ_MSP_CHANGE, send_index);
	SetShort(send_buff, m_iMaxMp, send_index);
	SetShort(send_buff, m_pUserData->m_sMp, send_index);
	Send(send_buff, send_index);

	if (m_sPartyIndex != -1)
	{
		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));

		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_HPCHANGE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetShort(send_buff, m_iMaxHp, send_index);
		SetShort(send_buff, m_pUserData->m_sHp, send_index);
		SetShort(send_buff, m_iMaxMp, send_index);
		SetShort(send_buff, m_pUserData->m_sMp, send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
	}
}

void CUser::Send2AI_UserUpdateInfo()
{
	int send_index = 0;
	char send_buff[1024];

	SetByte(send_buff, AG_USER_UPDATE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetByte(send_buff, m_pUserData->m_bLevel, send_index);
	SetShort(send_buff, m_pUserData->m_sHp, send_index);
	SetShort(send_buff, m_pUserData->m_sMp, send_index);
	SetShort(send_buff, m_sTotalHit * m_bAttackAmount / 100, send_index); // 표시
	SetShort(send_buff, m_sTotalAc + m_sACAmount, send_index);  // 표시
	Setfloat(send_buff, m_sTotalHitrate, send_index);
	Setfloat(send_buff, m_sTotalEvasionrate, send_index);

//
	SetShort(send_buff, m_sItemAc, send_index);
	SetByte(send_buff, m_bMagicTypeLeftHand, send_index);
	SetByte(send_buff, m_bMagicTypeRightHand, send_index);
	SetShort(send_buff, m_sMagicAmountLeftHand, send_index);
	SetShort(send_buff, m_sMagicAmountRightHand, send_index);
//

	m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
}

void CUser::SetUserAbility()
{
	model::Coefficient* p_TableCoefficient = nullptr;
	model::Item* pItem = nullptr;
	bool bHaveBow = false;

	p_TableCoefficient = m_pMain->m_CoefficientTableMap.GetData(m_pUserData->m_sClass);
	if (p_TableCoefficient == nullptr)
		return;

	float hitcoefficient = 0.0f;
	if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
	{
		pItem = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
		if (pItem != nullptr)
		{
			// 무기 타입....
			switch (pItem->Kind / 10)
			{
				case WEAPON_DAGGER:
					hitcoefficient = p_TableCoefficient->ShortSword;
					break;

				case WEAPON_SWORD:
					hitcoefficient = p_TableCoefficient->Sword;
					break;

				case WEAPON_AXE:
					hitcoefficient = p_TableCoefficient->Axe;
					break;

				case WEAPON_MACE:
					hitcoefficient = p_TableCoefficient->Club;
					break;

				case WEAPON_SPEAR:
					hitcoefficient = p_TableCoefficient->Spear;
					break;

				case WEAPON_BOW:
				case WEAPON_LONGBOW:
				case WEAPON_LAUNCHER:
					hitcoefficient = p_TableCoefficient->Bow;
					bHaveBow = true;
					break;

				case WEAPON_STAFF:
					hitcoefficient = p_TableCoefficient->Staff;
					break;
			}
		}
	}

	if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0
		&& hitcoefficient == 0.0f)
	{
		// 왼손 무기 : 활 적용을 위해
		pItem = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[LEFTHAND].nNum);
		if (pItem != nullptr)
		{
			// 무기 타입....
			switch (pItem->Kind / 10)
			{
				case WEAPON_BOW:
				case WEAPON_LONGBOW:
				case WEAPON_LAUNCHER:
					hitcoefficient = p_TableCoefficient->Bow;
					bHaveBow = true;
					break;
			}
		}
	}

	int temp_str = 0, temp_dex = 0;

	temp_str = m_pUserData->m_bStr + m_bStrAmount + m_sItemStr;
//	if (temp_str > 255)
//		temp_str = 255;

	temp_dex = m_pUserData->m_bDex + m_bDexAmount + m_sItemDex;
//	if (temp_dex > 255)
//		temp_dex = 255;

	m_sBodyAc = m_pUserData->m_bLevel;
	m_iMaxWeight = (m_pUserData->m_bStr + m_sItemStr) * 50;
/*
	궁수 공격력 = 0.005 * 활 공격력 * (Dex + 40) + 직업계수 * 활 공격력 * 화살공격력 * 레벨 * Dex
	전사 공격력 = 0.005 * 무기 공격력 * (Str + 40) + 직업계수 * 활 공격력 * 화살공격력 * 레벨 * Dex
*/
	if (bHaveBow)
	{
		m_sTotalHit = (short) ((((0.005 * pItem->Damage * (temp_dex + 40)) + (hitcoefficient * pItem->Damage * m_pUserData->m_bLevel * temp_dex)) + 3));
	}
	else
	{
		m_sTotalHit = (short) ((((0.005f * m_sItemHit * (temp_str + 40)) + (hitcoefficient * m_sItemHit * m_pUserData->m_bLevel * temp_str)) + 3));
	}

	// 토탈 AC = 테이블 코에피션트 * (레벨 + 아이템 AC + 테이블 4의 AC)
	m_sTotalAc = (short) (p_TableCoefficient->Armor * (m_sBodyAc + m_sItemAc));
	m_sTotalHitrate = ((1 + p_TableCoefficient->HitRate * m_pUserData->m_bLevel * temp_dex) * m_sItemHitrate / 100) * (m_bHitRateAmount / 100);

	m_sTotalEvasionrate = ((1 + p_TableCoefficient->Evasionrate * m_pUserData->m_bLevel * temp_dex) * m_sItemEvasionrate / 100) * (m_sAvoidRateAmount / 100);

	SetMaxHp();
	SetMaxMp();
}

void CUser::ItemMove(char* pBuf)
{
	int index = 0, itemid = 0, srcpos = -1, destpos = -1;
	int send_index = 0;
	char send_buff[128] = {};
	model::Item* pTable = nullptr;
	BYTE dir;

	dir = GetByte(pBuf, index);
	itemid = GetDWORD(pBuf, index);
	srcpos = GetByte(pBuf, index);
	destpos = GetByte(pBuf, index);

	if (m_sExchangeUser != -1)
		goto fail_return;

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		goto fail_return;

	// if (dir == ITEM_INVEN_SLOT && ((pTable->Weight + m_iItemWeight) > m_iMaxWeight))
	//		goto fail_return;

	if (dir > 0x04
		|| srcpos >= SLOT_MAX + HAVE_MAX
		|| destpos >= SLOT_MAX + HAVE_MAX)
		goto fail_return;

	if (destpos > SLOT_MAX)
	{
		if ((dir == ITEM_MOVE_INVEN_SLOT
			|| dir == ITEM_MOVE_SLOT_SLOT))
			goto fail_return;
	}

	if (dir == ITEM_MOVE_SLOT_INVEN
		&& srcpos > SLOT_MAX)
		goto fail_return;

	if (dir == ITEM_MOVE_INVEN_SLOT
		&& destpos == RESERVED)
		goto fail_return;

	if (dir == ITEM_MOVE_SLOT_INVEN
		&& srcpos == RESERVED)
		goto fail_return;

	if (dir == ITEM_MOVE_INVEN_SLOT
		|| dir == ITEM_MOVE_SLOT_SLOT)
	{
		if (pTable->Race != 0)
		{
			if (pTable->Race != m_pUserData->m_bRace)
				goto fail_return;
		}

		if (!ItemEquipAvailable(pTable))
			goto fail_return;
	}

	switch (dir)
	{
		case ITEM_MOVE_INVEN_SLOT:
			if (itemid != m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum)
				goto fail_return;

			if (!IsValidSlotPos(pTable, destpos))
				goto fail_return;
			
			// 오른손전용 무기(또는 양손쓸수 있고 장착하려는 위치가 오른손) 인데 다른손에 두손쓰는 경우 체크
			if (pTable->Slot == 0x01
				|| (pTable->Slot == 0x00 && destpos == RIGHTHAND))
			{
				if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0)
				{
					model::Item* pTable2 = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[LEFTHAND].nNum);
					if (pTable2 != nullptr)
					{
						if (pTable2->Slot == 0x04)
						{
							// 오른손에 넣구..
							m_pUserData->m_sItemArray[RIGHTHAND].nNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum;
							m_pUserData->m_sItemArray[RIGHTHAND].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
							m_pUserData->m_sItemArray[RIGHTHAND].sCount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;
							m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

							if (m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum == 0)
								m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum = m_pMain->GenerateItemSerial();

							// 왼손무기를 인벤으로 넣어준다.
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[LEFTHAND].nNum;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[LEFTHAND].sDuration;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[LEFTHAND].sCount;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[LEFTHAND].nSerialNum;

							if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
								m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

							// 왼손은 비어있게 되지...
							m_pUserData->m_sItemArray[LEFTHAND].nNum = 0;
							m_pUserData->m_sItemArray[LEFTHAND].sDuration = 0;
							m_pUserData->m_sItemArray[LEFTHAND].sCount = 0;
							m_pUserData->m_sItemArray[LEFTHAND].nSerialNum = 0;
						}
						else
						{
							short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
							int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

							// Swapping
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

							if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != 0
								&& m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
								m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

							m_pUserData->m_sItemArray[destpos].nNum = itemid;
							m_pUserData->m_sItemArray[destpos].sDuration = duration;
							m_pUserData->m_sItemArray[destpos].sCount = 1;
							m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

							if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
								m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
						}
					}
				}
				else
				{
					short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
					int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

					// Swapping
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

					if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != 0
						&& m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[destpos].nNum = itemid;
					m_pUserData->m_sItemArray[destpos].sDuration = duration;
					m_pUserData->m_sItemArray[destpos].sCount = 1;
					m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

					if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			// 왼손전용 무기(또는 양손쓸수 있고 장착하려는 위치가 왼손) 인데 다른손에 두손쓰는 경우 체크
			else if (pTable->Slot == 0x02
				|| (pTable->Slot == 0x00 && destpos == LEFTHAND))
			{
				if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
				{
					model::Item* pTable2 = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
					if (pTable2 != nullptr)
					{
						if (pTable2->Slot == 0x03)
						{
							m_pUserData->m_sItemArray[LEFTHAND].nNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum;
							m_pUserData->m_sItemArray[LEFTHAND].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
							m_pUserData->m_sItemArray[LEFTHAND].sCount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;
							m_pUserData->m_sItemArray[LEFTHAND].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

							if (m_pUserData->m_sItemArray[LEFTHAND].nSerialNum == 0)
								m_pUserData->m_sItemArray[LEFTHAND].nSerialNum = m_pMain->GenerateItemSerial();

							// 오른손무기를 인벤으로 넣어준다.
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[RIGHTHAND].nNum;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[RIGHTHAND].sDuration;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[RIGHTHAND].sCount;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum;

							if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
								m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

							m_pUserData->m_sItemArray[RIGHTHAND].nNum = 0;
							m_pUserData->m_sItemArray[RIGHTHAND].sDuration = 0;
							m_pUserData->m_sItemArray[RIGHTHAND].sCount = 0;
							m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum = 0;
						}
						else
						{
							short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
							int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

							// Swapping
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
							m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

							if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != 0
								&& m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
								m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

							m_pUserData->m_sItemArray[destpos].nNum = itemid;
							m_pUserData->m_sItemArray[destpos].sDuration = duration;
							m_pUserData->m_sItemArray[destpos].sCount = 1;
							m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

							if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
								m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
						}
					}
				}
				else
				{
					short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
					int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

					// Swapping
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

					if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != 0
						&& m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[destpos].nNum = itemid;
					m_pUserData->m_sItemArray[destpos].sDuration = duration;
					m_pUserData->m_sItemArray[destpos].sCount = 1;
					m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

					if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			// 두손 사용하고 오른손 무기
			else if (pTable->Slot == 0x03)
			{
				if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0
					&& m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
					goto fail_return;

				if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0)
				{
					m_pUserData->m_sItemArray[RIGHTHAND].nNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum;
					m_pUserData->m_sItemArray[RIGHTHAND].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
					m_pUserData->m_sItemArray[RIGHTHAND].sCount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;
					m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

					if (m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum == 0)
						m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[LEFTHAND].nNum;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[LEFTHAND].sDuration;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[LEFTHAND].sCount;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[LEFTHAND].nSerialNum;

					if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[LEFTHAND].nNum = 0;
					m_pUserData->m_sItemArray[LEFTHAND].sDuration = 0;
					m_pUserData->m_sItemArray[LEFTHAND].sCount = 0;
					m_pUserData->m_sItemArray[LEFTHAND].nSerialNum = 0;
				}
				else
				{
					short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
					int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

					// Swapping
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

					if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != 0
						&& m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[destpos].nNum = itemid;
					m_pUserData->m_sItemArray[destpos].sDuration = duration;
					m_pUserData->m_sItemArray[destpos].sCount = 1;
					m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

					if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			// 두손 사용하고 왼손 무기
			else if (pTable->Slot == 0x04)
			{
				if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0
					&& m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
					goto fail_return;

				if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
				{
					m_pUserData->m_sItemArray[LEFTHAND].nNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum;
					m_pUserData->m_sItemArray[LEFTHAND].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
					m_pUserData->m_sItemArray[LEFTHAND].sCount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;
					m_pUserData->m_sItemArray[LEFTHAND].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

					if (m_pUserData->m_sItemArray[LEFTHAND].nSerialNum == 0)
						m_pUserData->m_sItemArray[LEFTHAND].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[RIGHTHAND].nNum;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[RIGHTHAND].sDuration;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[RIGHTHAND].sCount;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum;

					if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[RIGHTHAND].nNum = 0;
					m_pUserData->m_sItemArray[RIGHTHAND].sDuration = 0;
					m_pUserData->m_sItemArray[RIGHTHAND].sCount = 0;
					m_pUserData->m_sItemArray[RIGHTHAND].nSerialNum = 0;
				}
				else
				{
					short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
					int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

					// Swapping
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

					if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != 0
						&& m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[destpos].nNum = itemid;
					m_pUserData->m_sItemArray[destpos].sDuration = duration;
					m_pUserData->m_sItemArray[destpos].sCount = 1;
					m_pUserData->m_sItemArray[destpos].nSerialNum = serial;
					if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			else
			{
				short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
				int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

				// Swapping
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

				if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != 0
					&& m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();

				m_pUserData->m_sItemArray[destpos].nNum = itemid;
				m_pUserData->m_sItemArray[destpos].sDuration = duration;
				m_pUserData->m_sItemArray[destpos].sCount = 1;
				m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

				if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
					m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
			}
			break;

		case ITEM_MOVE_SLOT_INVEN:
			if (itemid != m_pUserData->m_sItemArray[srcpos].nNum)
				goto fail_return;

			if (m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum != 0)
				goto fail_return;

			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum = m_pUserData->m_sItemArray[srcpos].nNum;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration = m_pUserData->m_sItemArray[srcpos].sDuration;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount = m_pUserData->m_sItemArray[srcpos].sCount;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = m_pUserData->m_sItemArray[srcpos].nSerialNum;

			if (m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum == 0)
				m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = m_pMain->GenerateItemSerial();

			m_pUserData->m_sItemArray[srcpos].nNum = 0;
			m_pUserData->m_sItemArray[srcpos].sDuration = 0;
			m_pUserData->m_sItemArray[srcpos].sCount = 0;
			m_pUserData->m_sItemArray[srcpos].nSerialNum = 0;
			break;

		case ITEM_MOVE_INVEN_INVEN:
		{
			if (itemid != m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum)
				goto fail_return;

			short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
			short itemcount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;
			int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;
			model::Item* pTable2 = nullptr;

			m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum;
			m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration;
			m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount;
			m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum;

			if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum == 0)
			{
				pTable2 = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum);
				if (pTable2 != nullptr
					&& pTable2->Countable == 0)
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pMain->GenerateItemSerial();
			}

			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum = itemid;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration = duration;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount = itemcount;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = serial;

			if (m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum == 0)
			{
				pTable2 = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum);
				if (pTable2 != nullptr
					&& pTable2->Countable == 0)
					m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = m_pMain->GenerateItemSerial();
			}
		}
		break;

		case ITEM_MOVE_SLOT_SLOT:
			if (itemid != m_pUserData->m_sItemArray[srcpos].nNum)
				goto fail_return;
			if (!IsValidSlotPos(pTable, destpos))
				goto fail_return;

			if (m_pUserData->m_sItemArray[destpos].nNum != 0)
			{
				// dest slot exist some item
				model::Item* pTable2 = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[destpos].nNum);
				if (pTable2 != nullptr)
				{
					if (pTable2->Slot != 0x00)
						goto fail_return;

					short duration = m_pUserData->m_sItemArray[srcpos].sDuration;
					short count = m_pUserData->m_sItemArray[srcpos].sCount;
					int64_t serial = m_pUserData->m_sItemArray[srcpos].nSerialNum;

					// Swapping
					m_pUserData->m_sItemArray[srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
					m_pUserData->m_sItemArray[srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
					m_pUserData->m_sItemArray[srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
					m_pUserData->m_sItemArray[srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

					if (m_pUserData->m_sItemArray[srcpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[srcpos].nSerialNum = m_pMain->GenerateItemSerial();

					m_pUserData->m_sItemArray[destpos].nNum = itemid;
					m_pUserData->m_sItemArray[destpos].sDuration = duration;
					m_pUserData->m_sItemArray[destpos].sCount = count;
					m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

					if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
						m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			else
			{
				short duration = m_pUserData->m_sItemArray[srcpos].sDuration;
				short count = m_pUserData->m_sItemArray[srcpos].sCount;
				int64_t serial = m_pUserData->m_sItemArray[srcpos].nSerialNum;

				// Swapping
				m_pUserData->m_sItemArray[srcpos].nNum = m_pUserData->m_sItemArray[destpos].nNum;
				m_pUserData->m_sItemArray[srcpos].sDuration = m_pUserData->m_sItemArray[destpos].sDuration;
				m_pUserData->m_sItemArray[srcpos].sCount = m_pUserData->m_sItemArray[destpos].sCount;
				m_pUserData->m_sItemArray[srcpos].nSerialNum = m_pUserData->m_sItemArray[destpos].nSerialNum;

				m_pUserData->m_sItemArray[destpos].nNum = itemid;
				m_pUserData->m_sItemArray[destpos].sDuration = duration;
				m_pUserData->m_sItemArray[destpos].sCount = count;
				m_pUserData->m_sItemArray[destpos].nSerialNum = serial;

				if (m_pUserData->m_sItemArray[destpos].nSerialNum == 0)
					m_pUserData->m_sItemArray[destpos].nSerialNum = m_pMain->GenerateItemSerial();
			}
			break;
	}

	// 장착이 바뀌는 경우에만 계산..
	if (dir != ITEM_MOVE_INVEN_INVEN)
	{
		SetSlotItemValue();
		SetUserAbility();
	}

//	비러머글 사자의 힘 >.<
	SetByte(send_buff, WIZ_ITEM_MOVE, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetShort(send_buff, m_sTotalHit, send_index);
	SetShort(send_buff, m_sTotalAc, send_index);
	SetShort(send_buff, GetMaxWeightForClient(), send_index);
	SetShort(send_buff, m_iMaxHp, send_index);
	SetShort(send_buff, m_iMaxMp, send_index);
	SetShort(send_buff, m_sItemStr + m_bStrAmount, send_index);
	SetShort(send_buff, m_sItemSta + m_bStaAmount, send_index);
	SetShort(send_buff, m_sItemDex + m_bDexAmount, send_index);
	SetShort(send_buff, m_sItemIntel + m_bIntelAmount, send_index);
	SetShort(send_buff, m_sItemCham + m_bChaAmount, send_index);
	SetShort(send_buff, m_bFireR, send_index);
	SetShort(send_buff, m_bColdR, send_index);
	SetShort(send_buff, m_bLightningR, send_index);
	SetShort(send_buff, m_bMagicR, send_index);
	SetShort(send_buff, m_bDiseaseR, send_index);
	SetShort(send_buff, m_bPoisonR, send_index);
	Send(send_buff, send_index);
//
	SendItemWeight();

	if (dir == ITEM_MOVE_INVEN_SLOT)
	{
		// 장착
		if (destpos == HEAD
			|| destpos == BREAST
			|| destpos == SHOULDER
			|| destpos == LEFTHAND
			|| destpos == RIGHTHAND
			|| destpos == LEG
			|| destpos == GLOVE
			|| destpos == FOOT)
			UserLookChange(destpos, itemid, m_pUserData->m_sItemArray[destpos].sDuration);
	}

	if (dir == ITEM_MOVE_SLOT_INVEN)
	{
		// 해제
		if (srcpos == HEAD
			|| srcpos == BREAST
			|| srcpos == SHOULDER
			|| srcpos == LEFTHAND
			|| srcpos == RIGHTHAND
			|| srcpos == LEG
			|| srcpos == GLOVE
			|| srcpos == FOOT)
			UserLookChange(srcpos, 0, 0);		
	}

	// AI Server에 바끤 데이타 전송....
	Send2AI_UserUpdateInfo();
	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_ITEM_MOVE, send_index);
	SetByte(send_buff, 0x00, send_index);
	Send(send_buff, send_index);
}

bool CUser::IsValidSlotPos(model::Item* pTable, int destpos) const
{
	if (pTable == nullptr)
		return false;

	switch (pTable->Slot)
	{
		case 0:
			if (destpos != RIGHTHAND
				&& destpos != LEFTHAND)
				return false;
			break;

		case 1:
		case 3:
			if (destpos != RIGHTHAND)
				return false;
			break;

		case 2:
		case 4:
			if (destpos != LEFTHAND)
				return false;
			break;

		case 5:
			if (destpos != BREAST)
				return false;
			break;

		case 6:
			if (destpos != LEG)
				return false;
			break;

		case 7:
			if (destpos != HEAD)
				return false;
			break;

		case 8:
			if (destpos != GLOVE)
				return false;
			break;

		case 9:
			if (destpos != FOOT)
				return false;
			break;

		case 10:
			if (destpos != RIGHTEAR
				&& destpos != LEFTEAR)
				return false;
			break;

		case 11:
			if (destpos != NECK)
				return false;
			break;

		case 12:
			if (destpos != RIGHTRING
				&& destpos != LEFTRING)
				return false;
			break;

		case 13:
			if (destpos != SHOULDER)
				return false;
			break;

		case 14:
			if (destpos != WAIST)
				return false;
			break;
	}

	return true;
}

void CUser::NpcEvent(char* pBuf)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return;

	int index = 0, send_index = 0, nid = 0, i = 0, temp_index = 0;
	char send_buff[2048] = {};
	CNpc* pNpc = nullptr;

	nid = GetShort(pBuf, index);

	pNpc = m_pMain->m_NpcMap.GetData(nid);
	if (pNpc == nullptr)
		return;

	switch (pNpc->m_tNpcType)
	{
		case NPC_MERCHANT:
			SetByte(send_buff, WIZ_TRADE_NPC, send_index);
			SetDWORD(send_buff, pNpc->m_iSellingGroup, send_index);
			Send(send_buff, send_index);
			break;

		case NPC_TINKER:
			SetByte(send_buff, WIZ_REPAIR_NPC, send_index);
			SetDWORD(send_buff, pNpc->m_iSellingGroup, send_index);
			Send(send_buff, send_index);
			break;

		case NPC_OFFICER:
			SetShort(send_buff, 0, send_index);	// default 0 page
			m_pMain->m_KnightsManager.AllKnightsList(this, send_buff);
			break;

		case NPC_WAREHOUSE:
			SetByte(send_buff, WIZ_WAREHOUSE, send_index);
			SetByte(send_buff, WAREHOUSE_REQ, send_index);
			Send(send_buff, send_index);
			break;

		case NPC_RENTAL:
			SetByte(send_buff, WIZ_RENTAL, send_index);
			SetByte(send_buff, RENTAL_NPC, send_index);
			// 1 = enabled, -1 = disabled
			SetShort(send_buff, 1, send_index);
			SetDWORD(send_buff, pNpc->m_iSellingGroup, send_index);
			Send(send_buff, send_index);
			break;

#if 0 // not typically available
		case NPC_COUPON:
			if (m_pMain->m_byNationID == 1
				|| m_pMain->m_byNationID == 4)
				return;

			SetShort(send_buff, nid, send_index);
			ClientEvent(send_buff);
			break;
#endif

		// 비러머글 퀘스트 관련 NPC들 >.<....
		case NPC_SELITH:
		case NPC_CLAN_MATCH_ADVISOR:
		case NPC_TELEPORT_GATE:
		case NPC_OPERATOR:
		case NPC_KISS:
		case NPC_ISAAC:
		case NPC_KAISHAN:
		case NPC_CAPTAIN:
		case NPC_CLERIC:
		case NPC_LADY:
		case NPC_ATHIAN:
		case NPC_ARENA:
		case NPC_TRAINER_KATE:
		case NPC_GENERIC:
		case NPC_SENTINEL_PATRICK:
		case NPC_TRADER_KIM:
		case NPC_PRIEST_IRIS:
		case NPC_MONK_ELMORAD:
		case NPC_MONK_KARUS:
		case NPC_MASTER_WARRIOR:
		case NPC_MASTER_ROGUE:
		case NPC_MASTER_MAGE:
		case NPC_MASTER_PRIEST:
		case NPC_BLACKSMITH:
		case NPC_NPC_1:
		case NPC_NPC_2:
		case NPC_NPC_3:
		case NPC_NPC_4:
		case NPC_NPC_5:
		case NPC_HERO_STATUE_1:
		case NPC_HERO_STATUE_2:
		case NPC_HERO_STATUE_3:
		case NPC_KARUS_HERO_STATUE:
		case NPC_ELMORAD_HERO_STATUE:
		case NPC_KEY_QUEST_1:
		case NPC_KEY_QUEST_2:
		case NPC_KEY_QUEST_3:
		case NPC_KEY_QUEST_4:
		case NPC_KEY_QUEST_5:
		case NPC_KEY_QUEST_6:
		case NPC_KEY_QUEST_7:
		case NPC_ROBOS:
		case NPC_SERVER_TRANSFER:
		case NPC_RANKING:
		case NPC_LYONI:
		case NPC_BEGINNER_HELPER_1:
		case NPC_BEGINNER_HELPER_2:
		case NPC_BEGINNER_HELPER_3:
		case NPC_FT_1:
		case NPC_FT_2:
		case NPC_FT_3:
		case NPC_PREMIUM_PC:
		case NPC_KJWAR:
		case NPC_SIEGE_2:
		case NPC_CRAFTSMAN:
		case NPC_COLISEUM_ARTES:
		case NPC_MANAGER_BARREL:
		case NPC_UNK_138:
		case NPC_LOVE_AGENT:
		case NPC_SPY:
		case NPC_ROYAL_GUARD:
		case NPC_ROYAL_CHEF:
		case NPC_ESLANT_WOMAN:
		case NPC_FARMER:
		case NPC_NAMELESS_WARRIOR:
		case NPC_UNK_147:
		case NPC_GATE_GUARD:
		case NPC_ROYAL_ADVISOR:
		case NPC_BIFROST_GATE:
		case NPC_SANGDUF:
		case NPC_UNK_152:
		case NPC_ADELIA:
		case NPC_BIFROST_MONUMENT:
			SetShort(send_buff, nid, send_index);
			ClientEvent(send_buff);
			break;
	}
}

void CUser::ItemTrade(char* pBuf)
{
	int index = 0, send_index = 0, itemid = 0, money = 0, count = 0, group = 0, npcid = 0;
	model::Item* pTable = nullptr;
	char send_buff[128] = {};
	CNpc* pNpc = nullptr;
	BYTE type = 0, pos = 0, destpos = 0, result = 0;

	// 상거래 안되게...
	if (m_bResHpType == USER_DEAD
		|| m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::ItemTrade: dead user cannot trade [charId={} socketId={} resHpType={} hp={} x={} z={}]",
			m_pUserData->m_id, m_Sid, m_bResHpType, m_pUserData->m_sHp,
			static_cast<int32_t>(m_pUserData->m_curx), static_cast<int32_t>(m_pUserData->m_curz));
		result = 0x01;
		goto fail_return;
	}

	type = GetByte(pBuf, index);

	// item buy
	if (type == 0x01)
	{
		group = GetDWORD(pBuf, index);
		npcid = GetShort(pBuf, index);
	}

	itemid = GetDWORD(pBuf, index);
	pos = GetByte(pBuf, index);

	// item move only
	if (type == 0x03) 
		destpos = GetByte(pBuf, index);
	else
		count = GetShort(pBuf, index);

	// 비러머글 크리스마스 이밴트 >.<
	if (itemid >= ITEM_NO_TRADE)
		goto fail_return;

	// item inven to inven move
	if (type == 0x03)
	{
		if (pos >= HAVE_MAX
			|| destpos >= HAVE_MAX)
		{
			SetByte(send_buff, WIZ_ITEM_TRADE, send_index);
			SetByte(send_buff, 0x04, send_index);
			Send(send_buff, send_index);
			return;
		}

		if (itemid != m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum)
		{
			SetByte(send_buff, WIZ_ITEM_TRADE, send_index);
			SetByte(send_buff, 0x04, send_index);
			Send(send_buff, send_index);
			return;
		}

		short duration = m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration;
		short itemcount = m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount;

		m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount;
		m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum = itemid;
		m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration = duration;
		m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount = itemcount;

		SetByte(send_buff, WIZ_ITEM_TRADE, send_index);
		SetByte(send_buff, 0x03, send_index);
		Send(send_buff, send_index);
		return;
	}

	if (m_sExchangeUser != -1)
		goto fail_return;

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
	{
		result = 0x01;
		goto fail_return;
	}

	if (pos >= HAVE_MAX)
	{
		result = 0x02;
		goto fail_return;
	}

	if (count <= 0
		|| count > MAX_ITEM_COUNT)
	{
		result = 0x02;
		goto fail_return;
	}

	// buy sequence
	if (type == 0x01)
	{
		if (!m_pMain->m_bPointCheckFlag)
		{
			result = 0x01;
			goto fail_return;
		}

		pNpc = m_pMain->m_NpcMap.GetData(npcid);
		if (pNpc == nullptr)
		{
			result = 0x01;
			goto fail_return;
		}

		if (pNpc->m_iSellingGroup != group)
		{
			result = 0x01;
			goto fail_return;
		}

		// Non-stackable items should only ever have a stack of 1.
		if (pTable->Countable == 0
			&& count != 1)
		{
			result = 2;
			goto fail_return;
		}

		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != 0)
		{
			if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum == itemid)
			{
				if (pTable->Countable == 0
					|| count <= 0)
				{
					result = 0x02;
					goto fail_return;
				}

				if (pTable->Countable != 0
					&& (count + m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount) > MAX_ITEM_COUNT)
				{
					result = 0x04;
					goto fail_return;
				}
			}
			else
			{
				result = 0x02;
				goto fail_return;
			}
		}

		int64_t buyPrice = static_cast<int64_t>(pTable->BuyPrice) * count;
		if (buyPrice < 0
			|| buyPrice > MAX_GOLD)
		{
			result = 3;
			goto fail_return;
		}

		if (m_pUserData->m_iGold < buyPrice)
		{
			result = 3;
			goto fail_return;
		}

		// Check weight of countable item.
		if (pTable->Countable)
		{
			if (((pTable->Weight * count) + m_iItemWeight) > m_iMaxWeight)
			{
				result = 0x04;
				goto fail_return;
			}
		}
		// Check weight of non-countable item.
		else
		{
			if ((pTable->Weight + m_iItemWeight) > m_iMaxWeight)
			{
				result = 0x04;
				goto fail_return;
			}
		}

		m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum = itemid;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = pTable->Durability;

		m_pUserData->m_iGold -= static_cast<int>(buyPrice);

		// count 개념이 있는 아이템
		if (pTable->Countable
			&& count > 0)
		{
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount += count;
		}
		else
		{
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = 1;
			m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum = m_pMain->GenerateItemSerial();
		}

		SendItemWeight();
		ItemLogToAgent(m_pUserData->m_id, pNpc->m_strName, ITEM_LOG_MERCHANT_BUY, m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum, itemid, count, pTable->Durability);
	}
	// sell sequence
	else
	{
		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != itemid)
		{
			result = 0x02;
			goto fail_return;
		}

		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount < count)
		{
			result = 0x03;
			goto fail_return;
		}

		int durability = m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration;

		if (pTable->Countable != 0
			&& count > 0)
		{
			int64_t salePrice = static_cast<int64_t>(pTable->BuyPrice) * count;
			if (pTable->SellPrice != SALE_TYPE_FULL)
			{
				if (m_pUserData->m_byPremiumType != 0)
					salePrice /= 6;
				else
					salePrice /= 4;
			}

			if (salePrice < 0
				|| salePrice > MAX_GOLD)
			{
				result = 3;
				goto fail_return;
			}

			m_pUserData->m_iGold += static_cast<int>(salePrice);

			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount -= count;

			if (m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount <= 0)
			{
				m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = 0;
			}
		}
		else
		{
			int64_t salePrice = static_cast<int64_t>(pTable->BuyPrice);
			if (pTable->SellPrice != SALE_TYPE_FULL)
			{
				if (m_pUserData->m_byPremiumType != 0)
					salePrice /= 6;
				else
					salePrice /= 4;
			}

			if (salePrice < 0
				|| salePrice > MAX_GOLD)
			{
				result = 3;
				goto fail_return;
			}

			m_pUserData->m_iGold += static_cast<int>(salePrice);

			m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum = 0;
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = 0;
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = 0;
		}

		SendItemWeight();
		ItemLogToAgent(m_pUserData->m_id, "MERCHANT SELL", ITEM_LOG_MERCHANT_SELL, 0, itemid, count, durability);
	}

	SetByte(send_buff, WIZ_ITEM_TRADE, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	Send(send_buff, send_index);
	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_ITEM_TRADE, send_index);
	SetByte(send_buff, 0x00, send_index);
	SetByte(send_buff, result, send_index);
	Send(send_buff, send_index);
}

void CUser::SendTargetHP(BYTE echo, int tid, int damage)
{
	int send_index = 0, hp = 0, maxhp = 0;
	char send_buff[256] = {};
	CUser* pTUser = nullptr;
	CNpc* pNpc = nullptr;

	if (tid < 0)
		return;

	if (tid >= NPC_BAND)
	{
		// 포인터 참조하면 안됨
		if (!m_pMain->m_bPointCheckFlag)
			return;

		pNpc = m_pMain->m_NpcMap.GetData(tid);
		if (pNpc == nullptr)
			return;

		hp = pNpc->m_iHP;
		maxhp = pNpc->m_iMaxHP;
	}
	else
	{
		if (tid >= MAX_USER)
			return;

		pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];
		if (pTUser == nullptr
			|| pTUser->m_bResHpType == USER_DEAD)
			return;

		hp = pTUser->m_pUserData->m_sHp;
		maxhp = pTUser->m_iMaxHp;
	}

	SetByte(send_buff, WIZ_TARGET_HP, send_index);
	SetShort(send_buff, tid, send_index);
	SetByte(send_buff, echo, send_index);
	SetDWORD(send_buff, maxhp, send_index);
	SetDWORD(send_buff, hp, send_index);
	SetShort(send_buff, damage, send_index);
	Send(send_buff, send_index);
}

void CUser::BundleOpenReq(char* pBuf)
{
	int index = 0, send_index = 0, bundle_index = 0;
	char send_buff[256] = {};
	_ZONE_ITEM* pItem = nullptr;
	C3DMap* pMap = nullptr;
	CRegion* pRegion = nullptr;

	bundle_index = GetDWORD(pBuf, index);
	if (bundle_index < 1)
		return;

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	if (m_RegionX < 0
		|| m_RegionZ < 0
		|| m_RegionX > pMap->GetXRegionMax()
		|| m_RegionZ > pMap->GetZRegionMax())
		return;

	pRegion = &pMap->m_ppRegion[m_RegionX][m_RegionZ];
	if (pRegion == nullptr)
		return;

	pItem = pRegion->m_RegionItemArray.GetData(bundle_index);
	if (pItem == nullptr)
		return;

	SetByte(send_buff, WIZ_BUNDLE_OPEN_REQ, send_index);

	for (int i = 0; i < 6; i++)
	{
		SetDWORD(send_buff, pItem->itemid[i], send_index);
		SetShort(send_buff, pItem->count[i], send_index);
	}

	Send(send_buff, send_index);
}

bool CUser::IsValidName(const char* name)
{
	// sungyong tw
	const char* szInvalids[] =
	{
		"~", "`", "!", "@", "#", "$", "%", "^", "&", "*",
		"(", ")", "-", "+", "=", "|", "\\", "<", ">", ",",
		".", "?", "/", "{", "[", "}", "]", "\"", "\'", " ", "　",
		"운영자", "나이트", "도우미", "Knight", "Noahsystem", "Wizgate", "Mgame",
		"노아시스템", "위즈게이트", "엠게임"
	};

	// taiwan version
	/*const char* szInvalids[] =
	{
		"~", "`", "!", "@", "#", "$", "%", "^", "&", "*",
		"(", ")", "-", "+", "=", "|", "\\", "<", ">", ",",
		".", "?", "/", "{", "[", "}", "]", "\"", "\'", " ",	"　"
	};*/

	for (int i = 0; i < _countof(szInvalids); i++)
	{
		if (strstr(name, szInvalids[i]) != nullptr)
			return false;
	}

	return true;
}

void CUser::ItemGet(char* pBuf)
{
	int index = 0, send_index = 0, bundle_index = 0, itemid = 0, count = 0, i = 0;
	BYTE pos;
	model::Item* pTable = nullptr;
	char send_buff[256] = {};
	_ZONE_ITEM* pItem = nullptr;
	C3DMap* pMap = nullptr;
	CRegion* pRegion = nullptr;
	CUser* pUser = nullptr;
	CUser* pGetUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;

	bundle_index = GetDWORD(pBuf, index);
	if (bundle_index < 1)
		goto fail_return;

	if (m_sExchangeUser != -1)
		goto fail_return;

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		goto fail_return;

	if (m_RegionX < 0
		|| m_RegionZ < 0
		|| m_RegionX > pMap->GetXRegionMax()
		|| m_RegionZ > pMap->GetZRegionMax())
		goto fail_return;

	pRegion = &pMap->m_ppRegion[m_RegionX][m_RegionZ];
	if (pRegion == nullptr)
		goto fail_return;

	pItem = pRegion->m_RegionItemArray.GetData(bundle_index);
	if (pItem == nullptr)
		goto fail_return;

	itemid = GetDWORD(pBuf, index);

	for (i = 0; i < 6; i++)
	{
		if (pItem->itemid[i] == itemid)
			break;
	}

	if (i == 6)
		goto fail_return;

	count = pItem->count[i];

	if (!pMap->RegionItemRemove(m_RegionX, m_RegionZ, bundle_index, pItem->itemid[i], pItem->count[i]))
		goto fail_return;

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		goto fail_return;

	if (m_sPartyIndex != -1
		&& itemid != ITEM_GOLD)
		pGetUser = GetItemRoutingUser(itemid, count);
	else
		pGetUser = this;

	if (pGetUser == nullptr)
		goto fail_return;

	pos = pGetUser->GetEmptySlot(itemid, pTable->Countable);

	// Common Item
	if (pos != 0xFF)
	{
		if (pos >= HAVE_MAX)
			goto fail_return;

		if (pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != 0)
		{
			if (pTable->Countable != 1)
				goto fail_return;

			if (pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != itemid)
				goto fail_return;
		}

		// Check weight of countable item.
		if (pTable->Countable)
		{
			if ((pTable->Weight * count + pGetUser->m_iItemWeight) > pGetUser->m_iMaxWeight)
			{
				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));
				SetByte(send_buff, WIZ_ITEM_GET, send_index);
				SetByte(send_buff, 0x06, send_index);
				pGetUser->Send(send_buff, send_index);
				return;
			}
		}
		// Check weight of non-countable item.
		else
		{
			if ((pTable->Weight + pGetUser->m_iItemWeight) > pGetUser->m_iMaxWeight)
			{
				send_index = 0;
				memset(send_buff, 0, sizeof(send_buff));
				SetByte(send_buff, WIZ_ITEM_GET, send_index);
				SetByte(send_buff, 0x06, send_index);
				pGetUser->Send(send_buff, send_index);
				return;
			}
		}

		// Add item to inventory. 
		pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum = itemid;

		// Apply number of item.
		if (pTable->Countable)
		{
			pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount += count;
			if (pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount > MAX_ITEM_COUNT)
				pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = MAX_ITEM_COUNT;
		}
		else
		{
			pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = 1;
			pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum = m_pMain->GenerateItemSerial();
		}

		pGetUser->SendItemWeight();
		pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = pTable->Durability;
		pGetUser->ItemLogToAgent(pGetUser->m_pUserData->m_id, "MONSTER", ITEM_LOG_MONSTER_GET, pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum, itemid, count, pTable->Durability);
	}
	// Gold
	else
	{
		if (itemid != ITEM_GOLD)
			goto fail_return;

		if (count > 0
			&& count < 32767)
		{
			if (m_sPartyIndex == -1)
			{
				m_pUserData->m_iGold += count;

				SetByte(send_buff, WIZ_ITEM_GET, send_index);
				SetByte(send_buff, 0x01, send_index);
				SetByte(send_buff, pos, send_index);
				SetDWORD(send_buff, itemid, send_index);
				SetShort(send_buff, count, send_index);
				SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
				Send(send_buff, send_index);
			}
			else
			{
				pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
				if (pParty == nullptr)
					goto fail_return;

				int usercount = 0, money = 0, levelsum = 0;
				for (int i = 0; i < 8; i++)
				{
					if (pParty->uid[i] != -1)
					{
						usercount++;
						levelsum += pParty->bLevel[i];
					}
				}

				if (usercount == 0)
					goto fail_return;

				for (i = 0; i < 8; i++)
				{
					if (pParty->uid[i] != -1
						|| pParty->uid[i] >= MAX_USER)
					{
						pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[pParty->uid[i]];
						if (pUser == nullptr)
							continue;

						money = count * (float) (pUser->m_pUserData->m_bLevel / (float) levelsum);
						pUser->m_pUserData->m_iGold += money;

						send_index = 0;
						memset(send_buff, 0, sizeof(send_buff));
						SetByte(send_buff, WIZ_ITEM_GET, send_index);
						SetByte(send_buff, 0x02, send_index);
						SetByte(send_buff, 0xff, send_index);			// gold -> pos : 0xff
						SetDWORD(send_buff, itemid, send_index);
						SetDWORD(send_buff, pUser->m_pUserData->m_iGold, send_index);
						pUser->Send(send_buff, send_index);
					}
				}
			}
		}
		return;
	}

	SetByte(send_buff, WIZ_ITEM_GET, send_index);
	if (pGetUser == this)
		SetByte(send_buff, 0x01, send_index);
	else
		SetByte(send_buff, 0x05, send_index);
	SetByte(send_buff, pos, send_index);
	SetDWORD(send_buff, itemid, send_index);
	SetShort(send_buff, pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount, send_index);
	SetDWORD(send_buff, pGetUser->m_pUserData->m_iGold, send_index);
	pGetUser->Send(send_buff, send_index);

	if (m_sPartyIndex != -1)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_ITEM_GET, send_index);
		SetByte(send_buff, 0x03, send_index);
		SetDWORD(send_buff, itemid, send_index);
		SetShort(send_buff, strlen(pGetUser->m_pUserData->m_id), send_index);
		SetString(send_buff, pGetUser->m_pUserData->m_id, strlen(pGetUser->m_pUserData->m_id), send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);

		if (pGetUser != this)
		{
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_ITEM_GET, send_index);
			SetByte(send_buff, 0x04, send_index);
			Send(send_buff, send_index);
		}
	}
	return;

fail_return:
	send_index = 0;
	SetByte(send_buff, WIZ_ITEM_GET, send_index);
	SetByte(send_buff, 0x00, send_index);
	Send(send_buff, send_index);
}

void CUser::StateChange(char* pBuf)
{
	int index = 0, send_index = 0;
	BYTE type = 0, buff = 0;
	char send_buff[128] = {};

	type = GetByte(pBuf, index);
	buff = GetByte(pBuf, index);

	if (type > 5)
		return;

	// Operators only!!!
	if (type == 5
		&& m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	if (type == 1)
		m_bResHpType = buff;
	else if (type == 2)
		m_bNeedParty = buff;
	else if (type == 3)
		m_bAbnormalType = buff;

	SetByte(send_buff, WIZ_STATE_CHANGE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetByte(send_buff, type, send_index);

	uint32_t nResult = 0;
	if (type == 1)
		nResult = m_bResHpType;
	else if (type == 2)
		nResult = m_bNeedParty;
	else if (type == 3)
		nResult = m_bAbnormalType;
	// Just plain echo :)
	//		N3_SP_STATE_CHANGE_ACTION = 0x04,			// 1 - 인사, 11 - 도발
	//		N3_SP_STATE_CHANGE_VISIBLE = 0x05 };		// 투명 0 ~ 255
	else
		nResult = buff;
		
	SetDWORD(send_buff, nResult, send_index);

	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ);
}

void CUser::LoyaltyChange(short tid)
{
	int send_index = 0;
	char send_buff[256] = {};
	short level_difference = 0, loyalty_source = 0, loyalty_target = 0;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.  

	// Check if target exists.
	if (pTUser == nullptr)
		return;

	// Different nations!!!
	if (pTUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
	{
		// Calculate difference!
		level_difference = pTUser->m_pUserData->m_bLevel - m_pUserData->m_bLevel;

		// No cheats allowed...
		if (pTUser->m_pUserData->m_iLoyalty <= 0)
		{
			loyalty_source = 0;
			loyalty_target = 0;
		}
		// Target at least six levels lower...
		else if (level_difference > 5)
		{
			loyalty_source = 50;
			loyalty_target = -25;
		}
		// Target at least six levels higher...
		else if (level_difference < -5)
		{
			loyalty_source = 10;
			loyalty_target = -5;
		}
		// Target within the 5 and -5 range...
		else
		{
			loyalty_source = 30;
			loyalty_target = -15;
		}
	}
	// Same Nations!!!
	else
	{
		if (pTUser->m_pUserData->m_iLoyalty >= 0)
		{
			loyalty_source = -1000;
			loyalty_target = -15;
		}
		else
		{
			loyalty_source = 100;
			loyalty_target = -15;
		}
	}

	if (m_pUserData->m_bZone != m_pUserData->m_bNation
		&& m_pUserData->m_bZone < 3)
		loyalty_source = 2 * loyalty_source;

	//TRACE(_T("LoyaltyChange 222 - user1=%hs, %d,, user2=%hs, %d\n"), m_pUserData->m_id,  m_pUserData->m_iLoyalty, pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_iLoyalty);

	m_pUserData->m_iLoyalty += loyalty_source;			// Recalculations of Loyalty...
	pTUser->m_pUserData->m_iLoyalty += loyalty_target;

	// Cannot be less than zero.
	if (m_pUserData->m_iLoyalty < 0)
		m_pUserData->m_iLoyalty = 0;

	if (pTUser->m_pUserData->m_iLoyalty < 0)
		pTUser->m_pUserData->m_iLoyalty = 0;

	//TRACE(_T("LoyaltyChange 222 - user1=%hs, %d,, user2=%hs, %d\n"), m_pUserData->m_id,  m_pUserData->m_iLoyalty, pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_iLoyalty);

	SetByte(send_buff, WIZ_LOYALTY_CHANGE, send_index);	// Send result to source.
	SetDWORD(send_buff, m_pUserData->m_iLoyalty, send_index);
	Send(send_buff, send_index);

	// Send result to target.
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_LOYALTY_CHANGE, send_index);
	SetDWORD(send_buff, pTUser->m_pUserData->m_iLoyalty, send_index);
	pTUser->Send(send_buff, send_index);

	// This is for the Event Battle on Wednesday :(
	if (m_pMain->m_byBattleOpen != 0)
	{
		if (m_pUserData->m_bZone == ZONE_BATTLE
			// || m_pUserData->m_bZone == ZONE_SNOW_BATTLE
			)
		{ 
			if (pTUser->m_pUserData->m_bNation == KARUS)
			{
				++m_pMain->m_sKarusDead;
				//TRACE(_T("++ LoyaltyChange - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
			}
			else if (pTUser->m_pUserData->m_bNation == ELMORAD)
			{
				++m_pMain->m_sElmoradDead;
				//TRACE(_T("++ LoyaltyChange - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
			}
		}
	}
//
}

void CUser::SpeedHackUser()
{
	if (strlen(m_pUserData->m_id) == 0)
		return;
	
	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
	{
		spdlog::warn("User::SpeedHackUser: speed hack detected, banning charId={}", m_pUserData->m_id);
		m_pUserData->m_bAuthority = AUTHORITY_BLOCK_USER;
	}

	Close();
}

void CUser::UserLookChange(int pos, int itemid, int durability)
{
	int send_index = 0;
	char send_buff[256] = {};

	if (pos >= SLOT_MAX)
		return;

	SetByte(send_buff, WIZ_USERLOOK_CHANGE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetByte(send_buff, (BYTE) pos, send_index);
	SetDWORD(send_buff, itemid, send_index);
	SetShort(send_buff, durability, send_index);
	m_pMain->Send_Region(send_buff, send_index, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, this);
}

void CUser::SendNotice()
{
	int send_index = 0, temp_index = 0, count = 0;
	char send_buff[2048] = {};

	send_index = 2;
	for (int i = 0; i < 20; i++)
	{
		if (strlen(m_pMain->m_ppNotice[i]) == 0)
			continue;

		SetString1(send_buff, m_pMain->m_ppNotice[i], send_index);
		count++;
	}

	temp_index = 0;
	SetByte(send_buff, WIZ_NOTICE, temp_index);
	SetByte(send_buff, count, temp_index);
	Send(send_buff, send_index);
}

void CUser::PartyProcess(char* pBuf)
{
	int index = 0, idlength = 0, memberid = -1;
	char strid[MAX_ID_SIZE + 1] = {};
	BYTE subcommand, result;
	CUser* pUser = nullptr;

	subcommand = GetByte(pBuf, index);
	switch (subcommand)
	{
		case PARTY_CREATE:
			idlength = GetShort(pBuf, index);
			if (idlength <= 0
				|| idlength > MAX_ID_SIZE)
				return;

			GetString(strid, pBuf, idlength, index);

			pUser = m_pMain->GetUserPtr(strid, NameType::Character);
			if (pUser != nullptr)
			{
				memberid = pUser->GetSocketID();
				PartyRequest(memberid, true);
			}
			break;

		case PARTY_PERMIT:
			result = GetByte(pBuf, index);
			if (result != 0)
				PartyInsert();
			// 거절한 경우
			else
				PartyCancel();
			break;

		case PARTY_INSERT:
			idlength = GetShort(pBuf, index);
			if (idlength <= 0
				|| idlength > MAX_ID_SIZE)
				return;

			GetString(strid, pBuf, idlength, index);

			pUser = m_pMain->GetUserPtr(strid, NameType::Character);
			if (pUser != nullptr)
			{
				memberid = pUser->GetSocketID();
				PartyRequest(memberid, false);
			}
			break;

		case PARTY_REMOVE:
			memberid = GetShort(pBuf, index);
			PartyRemove(memberid);
			break;

		case PARTY_DELETE:
			PartyDelete();
			break;
	}
}

// 거절한 사람한테 온다... 리더를 찾아서 알려주는 함수
void CUser::PartyCancel()
{
	int send_index = 0, leader_id = -1, count = 0;
	CUser* pUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;
	char send_buff[256] = {};

	if (m_sPartyIndex == -1)
		return;

	pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);

	// 이상한 경우
	if (pParty == nullptr)
	{
		m_sPartyIndex = -1;
		return;
	}

	m_sPartyIndex = -1;

	leader_id = pParty->uid[0];
	if (leader_id < 0
		|| leader_id >= MAX_USER)
		return;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[leader_id];
	if (pUser == nullptr)
		return;

	// 파티 생성시 취소한거면..파티를 뽀개야쥐...
	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] >= 0)
			count++;
	}

	// 리더 혼자이면 파티 깨짐
	if (count == 1)
		pUser->PartyDelete();

	SetByte(send_buff, WIZ_PARTY, send_index);
	SetByte(send_buff, PARTY_INSERT, send_index);
	SetShort(send_buff, -1, send_index);
	pUser->Send(send_buff, send_index);
}

//리더에게 패킷이 온거다..
void CUser::PartyRequest(int memberid, bool bCreate)
{
	int index = 0, send_index = 0, result = -1, i = 0;
	CUser* pUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;
	char send_buff[256] = {};

	if (memberid < 0
		|| memberid >= MAX_USER)
		goto fail_return;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[memberid];
	if (pUser == nullptr)
		goto fail_return;

	//이미 파티 가입되어 있으면 안되징...
	if (pUser->m_sPartyIndex != -1)
		goto fail_return;

	if (m_pUserData->m_bNation != pUser->m_pUserData->m_bNation)
	{
		result = -3;
		goto fail_return;
	}

	if (!((pUser->m_pUserData->m_bLevel <= (int) (m_pUserData->m_bLevel * 1.5) && pUser->m_pUserData->m_bLevel >= (int) (m_pUserData->m_bLevel * 1.5))
		|| (pUser->m_pUserData->m_bLevel <= (m_pUserData->m_bLevel + 8) && pUser->m_pUserData->m_bLevel >= ((int) (m_pUserData->m_bLevel) - 8))))
	{
		result = -2;
		goto fail_return;
	}

	// 기존의 파티에 추가하는 경우
	if (!bCreate)
	{
		pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
		if (pParty == nullptr)
			goto fail_return;

		for (i = 0; i < 8; i++)
		{
			if (pParty->uid[i] < 0)
				break;
		}

		// 파티 인원 Full
		if (i == 8)
			goto fail_return;
	}

	if (bCreate)
	{
		// (생성자)리더가 이미 파티 가입되어 있으면 안되징...
		if (m_sPartyIndex != -1)
			goto fail_return;

		m_sPartyIndex = m_pMain->m_sPartyIndex++;
		if (m_pMain->m_sPartyIndex == 32767)
			m_pMain->m_sPartyIndex = 0;

		EnterCriticalSection(&g_region_critical);

		pParty = new _PARTY_GROUP;
		pParty->wIndex = m_sPartyIndex;
		pParty->uid[0] = m_Sid;
		pParty->sMaxHp[0] = m_iMaxHp;
		pParty->sHp[0] = m_pUserData->m_sHp;
		pParty->bLevel[0] = m_pUserData->m_bLevel;
		pParty->sClass[0] = m_pUserData->m_sClass;

		if (!m_pMain->m_PartyMap.PutData(pParty->wIndex, pParty))
		{
			delete pParty;
			m_sPartyIndex = -1;
			LeaveCriticalSection(&g_region_critical);
			goto fail_return;
		}

		LeaveCriticalSection(&g_region_critical);

		// AI Server
		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));
		SetByte(send_buff, AG_USER_PARTY, send_index);
		SetByte(send_buff, PARTY_CREATE, send_index);
		SetShort(send_buff, pParty->wIndex, send_index);
		SetShort(send_buff, pParty->uid[0], send_index);
		//SetShort( send_buff, pParty->sHp[0], send_index );
		//SetByte( send_buff, pParty->bLevel[0], send_index );
		//SetShort( send_buff, pParty->sClass[0], send_index );
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
	}

	pUser->m_sPartyIndex = m_sPartyIndex;

/*	파티 BBS를 위해 추가...
	if (pUser->m_bNeedParty == 2 && pUser->m_sPartyIndex != -1) {
		pUser->m_bNeedParty = 1;	// 난 더 이상 파티가 필요하지 않아 ^^;
		memset( send_buff, 0x00, 256 ); send_index = 0;
		SetByte(send_buff, 2, send_index);
		SetByte(send_buff, pUser->m_bNeedParty, send_index);
		pUser->StateChange(send_buff);
	}

	if (m_bNeedParty == 2 && m_sPartyIndex != -1) {
		m_bNeedParty = 1;	// 난 더 이상 파티가 필요하지 않아 ^^;
		memset( send_buff, 0x00, 256 ); send_index = 0;
		SetByte(send_buff, 2, send_index);
		SetByte(send_buff, m_bNeedParty, send_index);
		StateChange(send_buff);
	}
*/
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, WIZ_PARTY, send_index);
	SetByte(send_buff, PARTY_PERMIT, send_index);
	SetShort(send_buff, m_Sid, send_index);
// 원거리가 않된데자나 씨~
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);	// Create packet.
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
//
	pUser->Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_PARTY, send_index);
	SetByte(send_buff, PARTY_INSERT, send_index);
	SetShort(send_buff, result, send_index);
	Send(send_buff, send_index);
}

void CUser::PartyInsert()	// 본인이 추가 된다.  리더에게 패킷이 가는것이 아님
{
	int send_index = 0;
	CUser* pUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;
	char send_buff[256] = {};

	if (m_sPartyIndex == -1)
		return;

	pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);

	// 이상한 경우
	if (pParty == nullptr)
	{
		m_sPartyIndex = -1;
		return;
	}

	// Send your info to the rest of the party members.
	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] == m_Sid)
			continue;

		if (pParty->uid[i] < 0
			|| pParty->uid[i] >= MAX_USER)
			continue;

		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[pParty->uid[i]];
		if (pUser == nullptr)
			continue;

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_INSERT, send_index);
		SetShort(send_buff, pParty->uid[i], send_index);
		SetShort(send_buff, strlen(pUser->m_pUserData->m_id), send_index);
		SetString(send_buff, pUser->m_pUserData->m_id, strlen(pUser->m_pUserData->m_id), send_index);
		SetShort(send_buff, pParty->sMaxHp[i], send_index);
		SetShort(send_buff, pParty->sHp[i], send_index);
		SetByte(send_buff, pParty->bLevel[i], send_index);
		SetShort(send_buff, pParty->sClass[i], send_index);
		SetShort(send_buff, pUser->m_iMaxMp, send_index);
		SetShort(send_buff, pUser->m_pUserData->m_sMp, send_index);
		Send(send_buff, send_index);	// 추가된 사람에게 기존 인원 다 받게함..
	}

	int i;
	for (i = 0; i < 8; i++)
	{
		// Party Structure 에 추가
		if (pParty->uid[i] != -1)
			continue;

		pParty->uid[i] = m_Sid;
		pParty->sMaxHp[i] = m_iMaxHp;
		pParty->sHp[i] = m_pUserData->m_sHp;
		pParty->bLevel[i] = m_pUserData->m_bLevel;
		pParty->sClass[i] = m_pUserData->m_sClass;
		break;
	}

// 파티 BBS를 위해 추가...	대장판!!!
	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[pParty->uid[0]];
	if (pUser == nullptr)
		return;

	// 이건 파티장 것...
	if (pUser->m_bNeedParty == 2
		&& pUser->m_sPartyIndex != -1)
	{
		// 난 더 이상 파티가 필요하지 않아 ^^;
		pUser->m_bNeedParty = 1;

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, 2, send_index);
		SetByte(send_buff, pUser->m_bNeedParty, send_index);
		pUser->StateChange(send_buff);
	}
// 추가 끝................

// 파티 BBS를 위해 추가...	쫄따구판!!!
	// 이건 실제로 추가된 사람 것...
	if (m_bNeedParty == 2
		&& m_sPartyIndex != -1)
	{
		// 난 더 이상 파티가 필요하지 않아 ^^;
		m_bNeedParty = 1;

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, 2, send_index);
		SetByte(send_buff, m_bNeedParty, send_index);
		StateChange(send_buff);
	}
// 추가 끝................

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_PARTY, send_index);
	SetByte(send_buff, PARTY_INSERT, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
	SetShort(send_buff, m_iMaxHp, send_index);
	SetShort(send_buff, m_pUserData->m_sHp, send_index);
	SetByte(send_buff, m_pUserData->m_bLevel, send_index);
	SetShort(send_buff, m_pUserData->m_sClass, send_index);
	SetShort(send_buff, m_iMaxMp, send_index);
	SetShort(send_buff, m_pUserData->m_sMp, send_index);
	m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);	// 추가된 인원을 브로드캐스팅..

	// AI Server
	BYTE byIndex = i;
	send_index = 0; memset(send_buff, 0x00, 256);
	SetByte(send_buff, AG_USER_PARTY, send_index);
	SetByte(send_buff, PARTY_INSERT, send_index);
	SetShort(send_buff, pParty->wIndex, send_index);
	SetByte(send_buff, byIndex, send_index);
	SetShort(send_buff, pParty->uid[byIndex], send_index);
	//SetShort( send_buff, pParty->sHp[byIndex], send_index );
	//SetByte( send_buff, pParty->bLevel[byIndex], send_index );
	//SetShort( send_buff, pParty->sClass[byIndex], send_index );
	m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
}

void CUser::PartyRemove(int memberid)
{
	int index = 0, send_index = 0, count = 0;
	CUser* pUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;

	if (m_sPartyIndex == -1)
		return;

	if (memberid < 0
		|| memberid >= MAX_USER)
		return;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[memberid];	// 제거될 사람...
	if (pUser == nullptr)
		return;

	pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);

	// 이상한 경우
	if (pParty == nullptr)
	{
		pUser->m_sPartyIndex = -1;
		m_sPartyIndex = -1;
		return;
	}

	// 자기자신 탈퇴가 아닌경우
	if (memberid != m_Sid)
	{
		// 리더만 멤버 삭제 할수 있음..
		if (pParty->uid[0] != m_Sid)
			return;
	}
	else
	{
		// 리더가 탈퇴하면 파티 깨짐
		if (pParty->uid[0] == memberid)
		{
			PartyDelete();
			return;
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] != -1
			&& pParty->uid[i] != memberid)
			++count;
	}

	// 리더 혼자 남은 경우 파티 깨짐
	if (count == 1)
	{
		PartyDelete();
		return;
	}

	// 삭제된 인원을 브로드캐스팅..제거될 사람한테두 패킷이 간다.
	char send_buff[256] = {};
	SetByte(send_buff, WIZ_PARTY, send_index);
	SetByte(send_buff, PARTY_REMOVE, send_index);
	SetShort(send_buff, memberid, send_index);
	m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);

	// 파티가 유효한 경우 에는 파티 리스트에서 뺀다.
	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] != -1
			&& pParty->uid[i] == memberid)
		{
			pParty->uid[i] = -1;
			pParty->sHp[i] = 0;
			pParty->bLevel[i] = 0;
			pParty->sClass[i] = 0;
			pUser->m_sPartyIndex = -1;
		}
	}

	// AI Server
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, AG_USER_PARTY, send_index);
	SetByte(send_buff, PARTY_REMOVE, send_index);
	SetShort(send_buff, pParty->wIndex, send_index);
	SetShort(send_buff, memberid, send_index);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
}

void CUser::PartyDelete()
{
	int send_index = 0;
	CUser* pUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;
	if (m_sPartyIndex == -1)
		return;

	pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
	if (pParty == nullptr)
	{
		m_sPartyIndex = -1;
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] < 0
			|| pParty->uid[i] >= MAX_USER)
			continue;

		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[pParty->uid[i]];
		if (pUser != nullptr)
			pUser->m_sPartyIndex = -1;
	}

	// 삭제된 인원을 브로드캐스팅..
	char send_buff[256] = {};
	SetByte(send_buff, WIZ_PARTY, send_index);
	SetByte(send_buff, PARTY_DELETE, send_index);
	m_pMain->Send_PartyMember(pParty->wIndex, send_buff, send_index);

	// AI Server
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, AG_USER_PARTY, send_index);
	SetByte(send_buff, PARTY_DELETE, send_index);
	SetShort(send_buff, pParty->wIndex, send_index);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);

	EnterCriticalSection(&g_region_critical);

	m_pMain->m_PartyMap.DeleteData(pParty->wIndex);

	LeaveCriticalSection(&g_region_critical);
}

void CUser::ExchangeProcess(char* pBuf)
{
	int index = 0;
	BYTE subcommand = GetByte(pBuf, index);
	switch (subcommand)
	{
		case EXCHANGE_REQ:
			ExchangeReq(pBuf + index);
			break;

		case EXCHANGE_AGREE:
			ExchangeAgree(pBuf + index);
			break;

		case EXCHANGE_ADD:
			ExchangeAdd(pBuf + index);
			break;

		case EXCHANGE_DECIDE:
			ExchangeDecide();
			break;

		case EXCHANGE_CANCEL:
			ExchangeCancel();
			break;
	}
}

void CUser::ExchangeReq(char* pBuf)
{
	int index = 0, destid = -1, send_index = 0, type = 0;
	CUser* pUser = nullptr;
	char send_buff[256] = {};

	destid = GetShort(pBuf, index);
	if (destid < 0
		|| destid >= MAX_USER)
		goto fail_return;

	// 교환 안되게.....
	if (m_bResHpType == USER_DEAD
		|| m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::ExchangeReq: dead user cannot exchange [charId={} socketId={} resHpType={} hp={} x={} z={}]",
			m_pUserData->m_id, m_Sid, m_bResHpType, m_pUserData->m_sHp,
			static_cast<int32_t>(m_pUserData->m_curx), static_cast<int32_t>(m_pUserData->m_curz));

		goto fail_return;
	}

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[destid];
	if (pUser == nullptr)
		goto fail_return;

	if (pUser->m_sExchangeUser != -1)
		goto fail_return;

	if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
		goto fail_return;

	m_sExchangeUser = destid;
	pUser->m_sExchangeUser = m_Sid;

	SetByte(send_buff, WIZ_EXCHANGE, send_index);
	SetByte(send_buff, EXCHANGE_REQ, send_index);
	SetShort(send_buff, m_Sid, send_index);
	pUser->Send(send_buff, send_index);

	return;

fail_return:
	SetByte(send_buff, WIZ_EXCHANGE, send_index);
	SetByte(send_buff, EXCHANGE_CANCEL, send_index);
	Send(send_buff, send_index);
}

void CUser::ExchangeAgree(char* pBuf)
{
	int index = 0, destid = -1, send_index = 0;
	CUser* pUser = nullptr;
	char send_buff[256] = {};

	BYTE result = GetByte(pBuf, index);

	if (m_sExchangeUser < 0
		|| m_sExchangeUser >= MAX_USER)
	{
		m_sExchangeUser = -1;
		return;
	}

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sExchangeUser];
	if (pUser == nullptr)
	{
		m_sExchangeUser = -1;
		return;
	}

	// 거절
	if (result == 0)
	{
		m_sExchangeUser = -1;
		pUser->m_sExchangeUser = -1;
	}
	else
	{
		InitExchange(true);
		pUser->InitExchange(true);
	}

	SetByte(send_buff, WIZ_EXCHANGE, send_index);
	SetByte(send_buff, EXCHANGE_AGREE, send_index);
	SetShort(send_buff, result, send_index);
	pUser->Send(send_buff, send_index);
}

void CUser::ExchangeAdd(char* pBuf)
{
	int index = 0, send_index = 0, count = 0, itemid = 0, duration = 0;
	CUser* pUser = nullptr;
	_EXCHANGE_ITEM* pItem = nullptr;
	model::Item* pTable = nullptr;
	char send_buff[256] = {};
	BYTE pos;
	bool bAdd = true, bGold = false;

	if (m_sExchangeUser < 0
		|| m_sExchangeUser >= MAX_USER)
	{
		ExchangeCancel();
		return;
	}

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sExchangeUser];
	if (pUser == nullptr)
	{
		ExchangeCancel();
		return;
	}

	pos = GetByte(pBuf, index);
	itemid = GetDWORD(pBuf, index);
	count = GetDWORD(pBuf, index);

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		goto add_fail;

	if (itemid != ITEM_GOLD
		&& pos >= HAVE_MAX)
		goto add_fail;

	if (m_bExchangeOK)
		goto add_fail;

	if (itemid == ITEM_GOLD)
	{
		if (count > m_pUserData->m_iGold)
			goto add_fail;

		if (count <= 0)
			goto add_fail;

		for (_EXCHANGE_ITEM* pExchangeItem : m_ExchangeItemList)
		{
			if (pExchangeItem->itemid == ITEM_GOLD)
			{
				pExchangeItem->count += count;
				m_pUserData->m_iGold -= count;
				bAdd = false;
				break;
			}
		}

		if (bAdd)
			m_pUserData->m_iGold -= count;
	}
	else if (m_MirrorItem[pos].nNum == itemid)
	{
		if (m_MirrorItem[pos].sCount < count)
			goto add_fail;

		// 중복허용 아이템
		if (pTable->Countable)
		{
			for (_EXCHANGE_ITEM* pExchangeItem : m_ExchangeItemList)
			{
				if (pExchangeItem->itemid == itemid)
				{
					pExchangeItem->count += count;
					m_MirrorItem[pos].sCount -= count;
					bAdd = false;
					break;
				}
			}
		}

		if (bAdd)
			m_MirrorItem[pos].sCount -= count;

		duration = m_MirrorItem[pos].sDuration;

		if (m_MirrorItem[pos].sCount <= 0
			|| pTable->Countable == 0)
		{
			m_MirrorItem[pos].nNum = 0;
			m_MirrorItem[pos].sDuration = 0;
			m_MirrorItem[pos].sCount = 0;
			m_MirrorItem[pos].nSerialNum = 0;
		}
	}
	else
	{
		goto add_fail;
	}

	for (_EXCHANGE_ITEM* pExchangeItem : m_ExchangeItemList)
	{
		if (pExchangeItem->itemid == ITEM_GOLD)
		{
			bGold = true;
			break;
		}
	}

	if (static_cast<int>(m_ExchangeItemList.size()) > ((bGold) ? 13 : 12))
		goto add_fail;

	// Gold 가 중복되면 추가하지 않는댜..
	if (bAdd)
	{
		pItem = new _EXCHANGE_ITEM;
		pItem->itemid = itemid;
		pItem->duration = duration;
		pItem->count = count;
		pItem->nSerialNum = m_MirrorItem[pos].nSerialNum;
		m_ExchangeItemList.push_back(pItem);
	}

	SetByte(send_buff, WIZ_EXCHANGE, send_index);
	SetByte(send_buff, EXCHANGE_ADD, send_index);
	SetByte(send_buff, 0x01, send_index);
	Send(send_buff, send_index);

	send_index = 0;
	SetByte(send_buff, WIZ_EXCHANGE, send_index);
	SetByte(send_buff, EXCHANGE_OTHERADD, send_index);
	SetDWORD(send_buff, itemid, send_index);
	SetDWORD(send_buff, count, send_index);
	SetShort(send_buff, duration, send_index);
	pUser->Send(send_buff, send_index);

	return;

add_fail:
	SetByte(send_buff, WIZ_EXCHANGE, send_index);
	SetByte(send_buff, EXCHANGE_ADD, send_index);
	SetByte(send_buff, 0x00, send_index);
	Send(send_buff, send_index);
}

void CUser::ExchangeDecide()
{
	int send_index = 0, getmoney = 0, putmoney = 0;
	CUser* pUser = nullptr;
	_EXCHANGE_ITEM* pItem = nullptr;
	char send_buff[256] = {};
	bool bSuccess = true;

	if (m_sExchangeUser < 0
		|| m_sExchangeUser >= MAX_USER)
	{
		ExchangeCancel();
		return;
	}

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sExchangeUser];
	if (pUser == nullptr)
	{
		ExchangeCancel();
		return;
	}

	if (!pUser->m_bExchangeOK)
	{
		m_bExchangeOK = 0x01;
		SetByte(send_buff, WIZ_EXCHANGE, send_index);
		SetByte(send_buff, EXCHANGE_OTHERDECIDE, send_index);
		pUser->Send(send_buff, send_index);
	}
	// 둘다 승인한 경우
	else
	{
		// 교환 실패
		if (!ExecuteExchange()
			|| !pUser->ExecuteExchange())
		{
			for (_EXCHANGE_ITEM* pExchangeItem : m_ExchangeItemList)
			{
				if (pExchangeItem->itemid == ITEM_GOLD)
				{
					// 돈만 백업
					m_pUserData->m_iGold += pExchangeItem->count;
					break;
				}
			}

			for (_EXCHANGE_ITEM* pExchangeItem : pUser->m_ExchangeItemList)
			{
				if (pExchangeItem->itemid == ITEM_GOLD)
				{
					// 돈만 백업
					pUser->m_pUserData->m_iGold += pExchangeItem->count;
					break;
				}
			}

			bSuccess = false;
		}

		if (bSuccess)
		{
			// 실제 데이터 교환...
			getmoney = ExchangeDone();
			putmoney = pUser->ExchangeDone();

			if (getmoney > 0)
			{
				ItemLogToAgent(
					m_pUserData->m_id,
					pUser->m_pUserData->m_id,
					ITEM_LOG_EXCHANGE_GET,
					0,
					ITEM_GOLD,
					getmoney,
					0);
			}

			if (putmoney > 0)
			{
				ItemLogToAgent(
					m_pUserData->m_id,
					pUser->m_pUserData->m_id,
					ITEM_LOG_EXCHANGE_PUT,
					0,
					ITEM_GOLD,
					putmoney,
					0);
			}

			SetByte(send_buff, WIZ_EXCHANGE, send_index);
			SetByte(send_buff, EXCHANGE_DONE, send_index);
			SetByte(send_buff, 0x01, send_index);
			SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
			SetShort(send_buff, pUser->m_ExchangeItemList.size(), send_index);

			for (_EXCHANGE_ITEM* pExchangeItem : pUser->m_ExchangeItemList)
			{
				// 새로 들어갈 인벤토리 위치
				SetByte(send_buff, pExchangeItem->pos, send_index);
				SetDWORD(send_buff, pExchangeItem->itemid, send_index);
				SetShort(send_buff, pExchangeItem->count, send_index);
				SetShort(send_buff, pExchangeItem->duration, send_index);

				ItemLogToAgent(
					m_pUserData->m_id,
					pUser->m_pUserData->m_id,
					ITEM_LOG_EXCHANGE_GET,
					pExchangeItem->nSerialNum,
					pExchangeItem->itemid,
					pExchangeItem->count,
					pExchangeItem->duration);
			}
			Send(send_buff, send_index);		// 나한테 보내고...

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_EXCHANGE, send_index);
			SetByte(send_buff, EXCHANGE_DONE, send_index);
			SetByte(send_buff, 1, send_index);
			SetDWORD(send_buff, pUser->m_pUserData->m_iGold, send_index);
			SetShort(send_buff, m_ExchangeItemList.size(), send_index);

			for (_EXCHANGE_ITEM* pExchangeItem : m_ExchangeItemList)
			{
				// 새로 들어갈 인벤토리 위치
				SetByte(send_buff, pExchangeItem->pos, send_index);
				SetDWORD(send_buff, pExchangeItem->itemid, send_index);
				SetShort(send_buff, pExchangeItem->count, send_index);
				SetShort(send_buff, pExchangeItem->duration, send_index);

				ItemLogToAgent(
					m_pUserData->m_id,
					pUser->m_pUserData->m_id,
					ITEM_LOG_EXCHANGE_PUT,
					pExchangeItem->nSerialNum,
					pExchangeItem->itemid,
					pExchangeItem->count,
					pExchangeItem->duration);
			}

			pUser->Send(send_buff, send_index);	// 상대방도 보내준다. 

			SendItemWeight();
			pUser->SendItemWeight();
		}
		else
		{
			SetByte(send_buff, WIZ_EXCHANGE, send_index);
			SetByte(send_buff, EXCHANGE_DONE, send_index);
			SetByte(send_buff, 0, send_index);
			Send(send_buff, send_index);
			pUser->Send(send_buff, send_index);
		}

		InitExchange(false);
		pUser->InitExchange(false);
	}
}

void CUser::ExchangeCancel()
{
	int send_index = 0;
	char send_buff[256] = {};
	CUser* pUser = nullptr;
	bool bFind = true;

	if (m_sExchangeUser < 0
		|| m_sExchangeUser >= MAX_USER)
		bFind = false;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sExchangeUser];
	if (pUser == nullptr)
		bFind = false;

	for (_EXCHANGE_ITEM* pExchangeItem : m_ExchangeItemList)
	{
		if (pExchangeItem->itemid == ITEM_GOLD)
		{
			// 돈만 백업
			m_pUserData->m_iGold += pExchangeItem->count;
			break;
		}
	}

	InitExchange(false);

	if (bFind)
	{
		pUser->ExchangeCancel();

		SetByte(send_buff, WIZ_EXCHANGE, send_index);
		SetByte(send_buff, EXCHANGE_CANCEL, send_index);
		pUser->Send(send_buff, send_index);
	}
}

void CUser::InitExchange(bool bStart)
{
	while (!m_ExchangeItemList.empty())
	{
		delete m_ExchangeItemList.front();
		m_ExchangeItemList.pop_front();
	}

	// 교환 시작시 백업
	if (bStart)
	{
		for (int i = 0; i < HAVE_MAX; i++)
		{
			m_MirrorItem[i].nNum = m_pUserData->m_sItemArray[SLOT_MAX + i].nNum;
			m_MirrorItem[i].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + i].sDuration;
			m_MirrorItem[i].sCount = m_pUserData->m_sItemArray[SLOT_MAX + i].sCount;
			m_MirrorItem[i].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum;
		}
	}
	// 교환 종료시 클리어
	else
	{
		m_sExchangeUser = -1;
		m_bExchangeOK = 0;

		for (int i = 0; i < HAVE_MAX; i++)
		{
			m_MirrorItem[i].nNum = 0;
			m_MirrorItem[i].sDuration = 0;
			m_MirrorItem[i].sCount = 0;
			m_MirrorItem[i].nSerialNum = 0;
		}
	}
}

bool CUser::ExecuteExchange()
{
	model::Item* pTable = nullptr;
	CUser* pUser = nullptr;
	DWORD money = 0;
	short weight = 0, i = 0;

	if (m_sExchangeUser < 0
		|| m_sExchangeUser >= MAX_USER)
		return false;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sExchangeUser];
	if (pUser == nullptr)
		return false;

	int iCount = pUser->m_ExchangeItemList.size();
	auto Iter = pUser->m_ExchangeItemList.begin();
	for (; Iter != pUser->m_ExchangeItemList.end(); ++Iter)
	{
		// 비러머글 크리스마스 이밴트 >.<
		if ((*Iter)->itemid >= ITEM_NO_TRADE)
			return false;

		if ((*Iter)->itemid == ITEM_GOLD)
		{
			money = (*Iter)->count;
		}
		else
		{
			pTable = m_pMain->m_ItemTableMap.GetData((*Iter)->itemid);
			if (pTable == nullptr)
				continue;

			for (i = 0; i < HAVE_MAX; i++)
			{
				// 증복허용 않되는 아이템!!!
				if (m_MirrorItem[i].nNum == 0
					&& pTable->Countable == 0)
				{
					m_MirrorItem[i].nNum = (*Iter)->itemid;
					m_MirrorItem[i].sDuration = (*Iter)->duration;
					m_MirrorItem[i].sCount = (*Iter)->count;
					m_MirrorItem[i].nSerialNum = (*Iter)->nSerialNum;

					// 패킷용 데이터...
					(*Iter)->pos = i;
					weight += pTable->Weight;
					break;
				}
				// 증복허용 아이템!!!				
				else if (m_MirrorItem[i].nNum == (*Iter)->itemid
					&& pTable->Countable == 1)
				{
					m_MirrorItem[i].sCount += (*Iter)->count;

					if (m_MirrorItem[i].sCount > MAX_ITEM_COUNT)
						m_MirrorItem[i].sCount = MAX_ITEM_COUNT;

					// 패킷용 데이터...
					(*Iter)->pos = i;
					weight += (pTable->Weight * (*Iter)->count);
					break;
				}
			}

			// 중복 허용 아이템인데 기존에 가지고 있지 않은 경우 빈곳에 추가
			if (i == HAVE_MAX
				&& pTable->Countable == 1)
			{
				for (i = 0; i < HAVE_MAX; i++)
				{
					if (m_MirrorItem[i].nNum != 0)
						continue;

					m_MirrorItem[i].nNum = (*Iter)->itemid;
					m_MirrorItem[i].sDuration = (*Iter)->duration;
					m_MirrorItem[i].sCount = (*Iter)->count;

					// 패킷용 데이터...
					(*Iter)->pos = i;
					weight += (pTable->Weight * (*Iter)->count);
					break;
				}
			}

			// 인벤토리 공간 부족...
			if (Iter != pUser->m_ExchangeItemList.end()
				&& i == HAVE_MAX)
				return false;
		}
	}

	// Too much weight! 
	if ((weight + m_iItemWeight) > m_iMaxWeight)
		return false;

	return true;
}

int CUser::ExchangeDone()
{
	int money = 0;
	CUser* pUser = nullptr;
	model::Item* pTable = nullptr;

	if (m_sExchangeUser < 0
		|| m_sExchangeUser >= MAX_USER)
		return 0;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sExchangeUser];
	if (pUser == nullptr)
		return 0;

	for (auto Iter = pUser->m_ExchangeItemList.begin(); Iter != pUser->m_ExchangeItemList.end(); )
	{
		if ((*Iter)->itemid == ITEM_GOLD)
		{
			money = (*Iter)->count;
			delete (*Iter);
			Iter = pUser->m_ExchangeItemList.erase(Iter);
			continue;
		}

		++Iter;
	}

	// 상대방이 준 돈.
	if (money > 0)
		m_pUserData->m_iGold += money;

	// 성공시 리스토어..
	for (int i = 0; i < HAVE_MAX; i++)
	{
		m_pUserData->m_sItemArray[SLOT_MAX + i].nNum = m_MirrorItem[i].nNum;
		m_pUserData->m_sItemArray[SLOT_MAX + i].sDuration = m_MirrorItem[i].sDuration;
		m_pUserData->m_sItemArray[SLOT_MAX + i].sCount = m_MirrorItem[i].sCount;
		m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum = m_MirrorItem[i].nSerialNum;

		pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[SLOT_MAX + i].nNum);
		if (pTable == nullptr)
			continue;

		if (pTable->Countable == 0
			&& m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum == 0)
			m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum = m_pMain->GenerateItemSerial();
	}

	return money;
}

void CUser::SkillPointChange(char* pBuf)
{
	int index = 0, send_index = 0, value = 0;
	BYTE type = 0;
	char send_buff[128] = {};

	type = GetByte(pBuf, index);
	if (type > 8)
		return; // goto fail_return;

	if (m_pUserData->m_bstrSkill[0] < 1)
		goto fail_return;

	if ((m_pUserData->m_bstrSkill[type] + 1) > m_pUserData->m_bLevel)
		goto fail_return;

	m_pUserData->m_bstrSkill[0] -= 1;
	m_pUserData->m_bstrSkill[type] += 1;

	return;

fail_return:
	SetByte(send_buff, WIZ_SKILLPT_CHANGE, send_index);
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, m_pUserData->m_bstrSkill[type], send_index);
	Send(send_buff, send_index);
}

void CUser::UpdateGameWeather(char* pBuf, BYTE type)
{
	int index = 0, send_index = 0, year = 0, month = 0, date = 0;
	char send_buff[128] = {};

	// is this user administrator?
	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	if (type == WIZ_WEATHER)
	{
		m_pMain->m_nWeather = GetByte(pBuf, index);
		m_pMain->m_nAmount = GetShort(pBuf, index);

		SetByte(send_buff, WIZ_WEATHER, send_index);
		SetByte(send_buff, (BYTE) m_pMain->m_nWeather, send_index);
		SetShort(send_buff, m_pMain->m_nAmount, send_index);
		m_pMain->Send_All(send_buff, send_index);
	}
	else if (type == WIZ_TIME)
	{
		year = GetShort(pBuf, index);
		month = GetShort(pBuf, index);
		date = GetShort(pBuf, index);
		m_pMain->m_nHour = GetShort(pBuf, index);
		m_pMain->m_nMin = GetShort(pBuf, index);

		SetByte(send_buff, WIZ_TIME, send_index);
		SetShort(send_buff, year, send_index);
		SetShort(send_buff, month, send_index);
		SetShort(send_buff, date, send_index);
		SetShort(send_buff, m_pMain->m_nHour, send_index);
		SetShort(send_buff, m_pMain->m_nMin, send_index);
		m_pMain->Send_All(send_buff, send_index);
	}
}

void CUser::ClassChange(char* pBuf)
{
	int index = 0, classcode = 0, send_index = 0, type = 0, sub_type = 0, money = 0, old_money = 0;
	char send_buff[128] = {};
	bool bSuccess = false;

	type = GetByte(pBuf, index);

	// 전직요청
	if (type == CLASS_CHANGE_REQ)
	{
		ClassChangeReq();
		return;
	}
	
	// 포인트 초기화
	if (type == ALL_POINT_CHANGE)
	{
		AllPointChange();
		return;
	}
	
	// 스킬 초기화
	if (type == ALL_SKILLPT_CHANGE)
	{
		AllSkillPointChange();
		return;
	}

	// 포인트 & 스킬 초기화에 돈이 얼마인지를 묻는 서브 패킷
	if (type == CHANGE_MONEY_REQ)
	{
		sub_type = GetByte(pBuf, index);

		money = pow((m_pUserData->m_bLevel * 2), 3.4);
		money = (money / 100) * 100;

		if (m_pUserData->m_bLevel < 30)
			money = static_cast<int>(money * 0.4);
#if 0
		else if (m_pUserData->m_bLevel >= 30
			&& m_pUserData->m_bLevel < 60)
			money = static_cast<int>(money * 1);
#endif
		else if (m_pUserData->m_bLevel >= 60
			&& m_pUserData->m_bLevel <= 90)
			money = static_cast<int>(money * 1.5);

		// 능력치 포인트
		if (sub_type == 1)
		{
			// 할인시점이고 승리국이라면
			if (m_pMain->m_sDiscount == 1
				&& m_pMain->m_byOldVictory == m_pUserData->m_bNation)
			{
				old_money = money;
				money = static_cast<int>(money * 0.5);
				//TRACE(_T("^^ ClassChange -  point Discount ,, money=%d->%d\n"), old_money, money);
			}

			if (m_pMain->m_sDiscount == 2)
			{
				old_money = money;
				money = static_cast<int>(money * 0.5);
			}

			SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
			SetByte(send_buff, CHANGE_MONEY_REQ, send_index);
			SetDWORD(send_buff, money, send_index);
			Send(send_buff, send_index);
		}
		// skill 포인트
		else if (sub_type == 2)
		{
			// 스킬은 한번 더
			money = static_cast<int>(money * 1.5);

			// 할인시점이고 승리국이라면
			if (m_pMain->m_sDiscount == 1
				&& m_pMain->m_byOldVictory == m_pUserData->m_bNation)
			{
				old_money = money;
				money = static_cast<int>(money * 0.5);
				//TRACE(_T("^^ ClassChange -  skillpoint Discount ,, money=%d->%d\n"), old_money, money);
			}

			if (m_pMain->m_sDiscount == 2)
			{
				old_money = money;
				money = static_cast<int>(money * 0.5);
			}

			SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
			SetByte(send_buff, CHANGE_MONEY_REQ, send_index);
			SetDWORD(send_buff, money, send_index);
			Send(send_buff, send_index);
		}

		return;
	}

	classcode = GetByte(pBuf, index);

	switch (m_pUserData->m_sClass)
	{
		case KARUWARRRIOR:
			if (classcode == BERSERKER
				|| classcode == GUARDIAN)
				bSuccess = true;
			break;

		case KARUROGUE:
			if (classcode == HUNTER
				|| classcode == PENETRATOR)
				bSuccess = true;
			break;

		case KARUWIZARD:
			if (classcode == SORSERER
				|| classcode == NECROMANCER)
				bSuccess = true;
			break;

		case KARUPRIEST:
			if (classcode == SHAMAN
				|| classcode == DARKPRIEST)
				bSuccess = true;
			break;

		case ELMORWARRRIOR:
			if (classcode == BLADE
				|| classcode == PROTECTOR)
				bSuccess = true;
			break;

		case ELMOROGUE:
			if (classcode == RANGER
				|| classcode == ASSASSIN)
				bSuccess = true;
			break;

		case ELMOWIZARD:
			if (classcode == MAGE
				|| classcode == ENCHANTER)
				bSuccess = true;
			break;

		case ELMOPRIEST:
			if (classcode == CLERIC
				|| classcode == DRUID)
				bSuccess = true;
			break;
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	if (!bSuccess)
	{
		SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
		SetByte(send_buff, CLASS_CHANGE_RESULT, send_index);
		SetByte(send_buff, 0, send_index);
		Send(send_buff, send_index);
	}
	else
	{
		m_pUserData->m_sClass = classcode;

		if (m_sPartyIndex != -1)
		{
			SetByte(send_buff, WIZ_PARTY, send_index);
			SetByte(send_buff, PARTY_CLASSCHANGE, send_index);
			SetShort(send_buff, m_Sid, send_index);
			SetShort(send_buff, m_pUserData->m_sClass, send_index);
			m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
		}
	}
}

bool CUser::ItemEquipAvailable(const model::Item* pTable) const
{
	if (pTable == nullptr)
		return false;

	// if (pTable->m_bReqLevel > m_pUserData->m_bLevel)
	//	return false;

	if (pTable->RequiredRank > m_pUserData->m_bRank)
		return false;

	if (pTable->RequiredTitle > m_pUserData->m_bTitle)
		return false;

	if (pTable->RequiredStrength > m_pUserData->m_bStr)
		return false;

	if (pTable->RequiredStamina > m_pUserData->m_bSta)
		return false;

	if (pTable->RequiredDexterity > m_pUserData->m_bDex)
		return false;

	if (pTable->RequiredIntelligence > m_pUserData->m_bIntel)
		return false;

	if (pTable->RequiredCharisma > m_pUserData->m_bCha)
		return false;

	return true;
}

void CUser::ChatTargetSelect(char* pBuf)
{
	int index = 0, send_index = 0, idlen = 0;
	CUser* pUser = nullptr;
	char chatid[MAX_ID_SIZE + 1] = {},
		send_buff[128] = {};

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE
		|| idlen < 0)
		return;

	GetString(chatid, pBuf, idlen, index);

	int i;
	for (i = 0; i < MAX_USER; i++)
	{
		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];
		if (pUser != nullptr
			&& pUser->GetState() == STATE_GAMESTART
			&& _strnicmp(chatid, pUser->m_pUserData->m_id, MAX_ID_SIZE) == 0)
		{
			m_sPrivateChatUser = i;
			break;
		}
	}

	SetByte(send_buff, WIZ_CHAT_TARGET, send_index);
	if (i == MAX_USER)
		SetShort(send_buff, 0, send_index);
	else
	{
		SetShort(send_buff, strlen(chatid), send_index);
		SetString(send_buff, chatid, strlen(chatid), send_index);
	}
	Send(send_buff, send_index);
}

// AI server에 User정보를 전부 전송...
void CUser::SendUserInfo(char* temp_send, int& index)
{
	SetShort(temp_send, m_Sid, index);
	SetShort(temp_send, strlen(m_pUserData->m_id), index);
	SetString(temp_send, m_pUserData->m_id, strlen(m_pUserData->m_id), index);
	SetByte(temp_send, m_pUserData->m_bZone, index);
	SetShort(temp_send, m_iZoneIndex, index);
	SetByte(temp_send, m_pUserData->m_bNation, index);
	SetByte(temp_send, m_pUserData->m_bLevel, index);
	SetShort(temp_send, m_pUserData->m_sHp, index);
	SetShort(temp_send, m_pUserData->m_sMp, index);
	SetShort(temp_send, m_sTotalHit * m_bAttackAmount / 100, index);    // 표시
	SetShort(temp_send, m_sTotalAc + m_sACAmount, index);	// 표시
	Setfloat(temp_send, m_sTotalHitrate, index);
	Setfloat(temp_send, m_sTotalEvasionrate, index);
	SetShort(temp_send, m_sPartyIndex, index);
	SetByte(temp_send, m_pUserData->m_bAuthority, index);
}

void CUser::CountConcurrentUser()
{
	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	int usercount = 0, send_index = 0;
	char send_buff[128] = {};

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];
		if (pUser != nullptr
			&& pUser->GetState() == STATE_GAMESTART)
			++usercount;
	}

	SetByte(send_buff, WIZ_CONCURRENTUSER, send_index);
	SetShort(send_buff, usercount, send_index);
	Send(send_buff, send_index);
}

void CUser::LoyaltyDivide(short tid)
{
	int send_index = 0;
	char send_buff[256] = {};

	int levelsum = 0, individualvalue = 0;
	short temp_loyalty = 0, level_difference = 0,
		loyalty_source = 0, loyalty_target = 0,
		average_level = 0;
	BYTE total_member = 0;

	CUser* pUser = nullptr;

	if (m_sPartyIndex < 0)
		return;

	_PARTY_GROUP* pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
	if (pParty == nullptr)
		return;

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];     // Get target info.  

	// Check if target exists.
	if (pTUser == nullptr)
		return;

	// Get total level and number of members in party.
	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] != -1)
		{
			levelsum += pParty->bLevel[i];
			++total_member;
		}
	}

	// Protection codes.
	if (levelsum <= 0)
		return;

	if (total_member <= 0)
		return;

	// Calculate average level.
	average_level = levelsum / total_member;

	// This is for the Event Battle on Wednesday :(
	if (m_pMain->m_byBattleOpen != 0)
	{
		if (m_pUserData->m_bZone == ZONE_BATTLE)
		{
			if (pTUser->m_pUserData->m_bNation == KARUS)
			{
				++m_pMain->m_sKarusDead;
				//TRACE(_T("++ LoyaltyDivide - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
			}
			else if (pTUser->m_pUserData->m_bNation == ELMORAD)
			{
				++m_pMain->m_sElmoradDead;
				//TRACE(_T("++ LoyaltyDivide - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
			}
		}
	}

	// Different nations!!!
	if (pTUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
	{
		// Calculate difference!
		level_difference = pTUser->m_pUserData->m_bLevel - average_level;

		// No cheats allowed...
		if (pTUser->m_pUserData->m_iLoyalty <= 0)
		{
			loyalty_source = 0;
			loyalty_target = 0;
		}
		// At least six levels higher...
		else if (level_difference > 5)
		{
			loyalty_source = 50;
			loyalty_target = -25;
		}
		// At least six levels lower...
		else if (level_difference < -5)
		{
			loyalty_source = 10;
			loyalty_target = -5;
		}
		// Within the 5 and -5 range...
		else
		{
			loyalty_source = 30;
			loyalty_target = -15;
		}
	}
	// Same Nation!!! 
	else
	{
		individualvalue = -1000;

		// Distribute loyalty amongst party members.
		for (int j = 0; j < 8; j++)
		{
			if (pParty->uid[j] != -1
				|| pParty->uid[j] >= MAX_USER)
			{
				pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[pParty->uid[j]];
				if (pUser == nullptr)
					continue;

				//TRACE(_T("LoyaltyDivide 111 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);

				pUser->m_pUserData->m_iLoyalty += individualvalue;

				// Cannot be less than zero.
				if (pUser->m_pUserData->m_iLoyalty < 0)
					pUser->m_pUserData->m_iLoyalty = 0;

				//TRACE(_T("LoyaltyDivide 222 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);

				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;
				SetByte(send_buff, WIZ_LOYALTY_CHANGE, send_index);	// Send result to source.
				SetDWORD(send_buff, pUser->m_pUserData->m_iLoyalty, send_index);
				pUser->Send(send_buff, send_index);
			}
		}

		return;
	}

	if (m_pUserData->m_bZone != m_pUserData->m_bNation
		&& m_pUserData->m_bZone < 3)
		loyalty_source = 2 * loyalty_source;

	// Distribute loyalty amongst party members.
	for (int j = 0; j < 8; j++)
	{
		if (pParty->uid[j] != -1
			|| pParty->uid[j] >= MAX_USER)
		{
			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[pParty->uid[j]];
			if (pUser == nullptr)
				continue;

			//TRACE(_T("LoyaltyDivide 333 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);
			individualvalue = pUser->m_pUserData->m_bLevel * loyalty_source / levelsum;
			pUser->m_pUserData->m_iLoyalty += individualvalue;

			if (pUser->m_pUserData->m_iLoyalty < 0)
				pUser->m_pUserData->m_iLoyalty = 0;

			//TRACE(_T("LoyaltyDivide 444 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_LOYALTY_CHANGE, send_index);	// Send result to source.
			SetDWORD(send_buff, pUser->m_pUserData->m_iLoyalty, send_index);
			pUser->Send(send_buff, send_index);

			individualvalue = 0;
		}
	}

	pTUser->m_pUserData->m_iLoyalty += loyalty_target;	// Recalculate target loyalty.

	if (pTUser->m_pUserData->m_iLoyalty < 0)
		pTUser->m_pUserData->m_iLoyalty = 0;

	//TRACE(_T("LoyaltyDivide 555 - user1=%hs, %d\n"), pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_iLoyalty);

	// Send result to target.
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_LOYALTY_CHANGE, send_index);
	SetDWORD(send_buff, pTUser->m_pUserData->m_iLoyalty, send_index);
	pTUser->Send(send_buff, send_index);
}

void CUser::Dead()
{
	int send_index = 0;
	char send_buff[1024] = {},
		strKnightsName[MAX_ID_SIZE + 1] = {};
	CKnights* pKnights = nullptr;

	SetByte(send_buff, WIZ_DEAD, send_index);
	SetShort(send_buff, m_Sid, send_index);
	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ);

	m_bResHpType = USER_DEAD;

	// 유저에게는 바로 데드 패킷을 날림... (한 번 더 보냄, 유령을 없애기 위해서)
	Send(send_buff, send_index);

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;

	// 지휘권한이 있는 유저가 죽는다면,, 지휘 권한 박탈
	if (m_pUserData->m_bFame == COMMAND_CAPTAIN)
	{
		m_pUserData->m_bFame = CHIEF;
		SetByte(send_buff, WIZ_AUTHORITY_CHANGE, send_index);
		SetByte(send_buff, COMMAND_AUTHORITY, send_index);
		SetShort(send_buff, GetSocketID(), send_index);
		SetByte(send_buff, m_pUserData->m_bFame, send_index);
		m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ);
		Send(send_buff, send_index);

		pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
		if (pKnights != nullptr)
			strcpy(strKnightsName, pKnights->m_strName);
		else
			strcpy(strKnightsName, "*");

		std::string chatstr;

		//TRACE(_T("---> Dead Captain Deprive - %hs\n"), m_pUserData->m_id);
		if (m_pUserData->m_bNation == KARUS)
			chatstr = fmt::format_db_resource(IDS_KARUS_CAPTAIN_DEPRIVE, strKnightsName, m_pUserData->m_id);
		else if (m_pUserData->m_bNation == ELMORAD)
			chatstr = fmt::format_db_resource(IDS_ELMO_CAPTAIN_DEPRIVE, strKnightsName, m_pUserData->m_id);

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;

		chatstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);
		SetByte(send_buff, WIZ_CHAT, send_index);
		SetByte(send_buff, WAR_SYSTEM_CHAT, send_index);
		SetByte(send_buff, 1, send_index);
		SetShort(send_buff, -1, send_index);
		SetByte(send_buff, 0, send_index);			// sender name length
		SetString2(send_buff, chatstr, send_index);
		m_pMain->Send_All(send_buff, send_index, nullptr, m_pUserData->m_bNation);
	}
}

void CUser::ItemWoreOut(int type, int damage)
{
	model::Item* pTable = nullptr;
	int worerate = sqrt(damage / 10.0);
	if (worerate == 0)
		return;

	if (type == DURABILITY_TYPE_ATTACK)
	{
		if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[RIGHTHAND].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
			if (pTable != nullptr
				// 2 == DEFENCE ITEM
				&& pTable->Slot != 2)
			{
				m_pUserData->m_sItemArray[RIGHTHAND].sDuration -= worerate;
				ItemDurationChange(RIGHTHAND, pTable->Durability, m_pUserData->m_sItemArray[RIGHTHAND].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[LEFTHAND].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[LEFTHAND].nNum);
			if (pTable != nullptr
				// 2 == DEFENCE ITEM
				&& pTable->Slot != 2)
			{
				m_pUserData->m_sItemArray[LEFTHAND].sDuration -= worerate;
				ItemDurationChange(LEFTHAND, pTable->Durability, m_pUserData->m_sItemArray[LEFTHAND].sDuration, worerate);
			}
		}
	}
	else if (type == DURABILITY_TYPE_DEFENCE)
	{
		if (m_pUserData->m_sItemArray[HEAD].nNum != 0
			&& m_pUserData->m_sItemArray[HEAD].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[HEAD].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[HEAD].sDuration -= worerate;
				ItemDurationChange(HEAD, pTable->Durability, m_pUserData->m_sItemArray[HEAD].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[BREAST].nNum != 0
			&& m_pUserData->m_sItemArray[BREAST].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[BREAST].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[BREAST].sDuration -= worerate;
				ItemDurationChange(BREAST, pTable->Durability, m_pUserData->m_sItemArray[BREAST].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[LEG].nNum != 0
			&& m_pUserData->m_sItemArray[LEG].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[LEG].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[LEG].sDuration -= worerate;
				ItemDurationChange(LEG, pTable->Durability, m_pUserData->m_sItemArray[LEG].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[GLOVE].nNum != 0
			&& m_pUserData->m_sItemArray[GLOVE].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[GLOVE].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[GLOVE].sDuration -= worerate;
				ItemDurationChange(GLOVE, pTable->Durability, m_pUserData->m_sItemArray[GLOVE].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[FOOT].nNum != 0
			&& m_pUserData->m_sItemArray[FOOT].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[FOOT].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[FOOT].sDuration -= worerate;
				ItemDurationChange(FOOT, pTable->Durability, m_pUserData->m_sItemArray[FOOT].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[RIGHTHAND].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
			if (pTable != nullptr
				// 방패?
				&& pTable->Slot == 2)
			{
				m_pUserData->m_sItemArray[RIGHTHAND].sDuration -= worerate;
				ItemDurationChange(RIGHTHAND, pTable->Durability, m_pUserData->m_sItemArray[RIGHTHAND].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[LEFTHAND].sDuration != 0)
		{
			pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[LEFTHAND].nNum);
			if (pTable
				// 방패?
				&& pTable->Slot == 2)
			{
				m_pUserData->m_sItemArray[LEFTHAND].sDuration -= worerate;
				ItemDurationChange(LEFTHAND, pTable->Durability, m_pUserData->m_sItemArray[LEFTHAND].sDuration, worerate);
			}
		}
	}
}

void CUser::ItemDurationChange(int slot, int maxvalue, int curvalue, int amount)
{
	if (maxvalue <= 0)
		return;

	if (slot < 0
		|| slot > SLOT_MAX)
		return;

	int curpercent = 0, beforepercent = 0, curbasis = 0, beforebasis = 0;
	int send_index = 0;
	char send_buff[128] = {};

	if (m_pUserData->m_sItemArray[slot].sDuration <= 0)
	{
		m_pUserData->m_sItemArray[slot].sDuration = 0;

		SetByte(send_buff, WIZ_DURATION, send_index);
		SetByte(send_buff, slot, send_index);
		SetShort(send_buff, 0, send_index);
		Send(send_buff, send_index);

		SetSlotItemValue();
		SetUserAbility();

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_ITEM_MOVE, send_index);	// durability 변경에 따른 수치 재조정...
		SetByte(send_buff, 0x01, send_index);
		SetShort(send_buff, m_sTotalHit, send_index);
		SetShort(send_buff, m_sTotalAc, send_index);
		SetShort(send_buff, GetCurrentWeightForClient(), send_index);
		SetShort(send_buff, m_iMaxHp, send_index);
		SetShort(send_buff, m_iMaxMp, send_index);
		SetShort(send_buff, m_sItemStr + m_bStrAmount, send_index);
		SetShort(send_buff, m_sItemSta + m_bStaAmount, send_index);
		SetShort(send_buff, m_sItemDex + m_bDexAmount, send_index);
		SetShort(send_buff, m_sItemIntel + m_bIntelAmount, send_index);
		SetShort(send_buff, m_sItemCham + m_bChaAmount, send_index);
		SetShort(send_buff, m_bFireR, send_index);
		SetShort(send_buff, m_bColdR, send_index);
		SetShort(send_buff, m_bLightningR, send_index);
		SetShort(send_buff, m_bMagicR, send_index);
		SetShort(send_buff, m_bDiseaseR, send_index);
		SetShort(send_buff, m_bPoisonR, send_index);
		Send(send_buff, send_index);
		return;
	}

	curpercent = (curvalue / (double) maxvalue) * 100;
	beforepercent = ((curvalue + amount) / (double) maxvalue) * 100;

	curbasis = curpercent / 5;
	beforebasis = beforepercent / 5;

	if (curbasis != beforebasis)
	{
		SetByte(send_buff, WIZ_DURATION, send_index);
		SetByte(send_buff, slot, send_index);
		SetShort(send_buff, curvalue, send_index);
		Send(send_buff, send_index);

		if (curpercent >= 65
			&& curpercent < 70)
			UserLookChange(slot, m_pUserData->m_sItemArray[slot].nNum, curvalue);

		if (curpercent >= 25
			&& curpercent < 30)
			UserLookChange(slot, m_pUserData->m_sItemArray[slot].nNum, curvalue);
	}
}

void CUser::HPTimeChange(float currenttime)
{
	bool bFlag = false;

	m_fHPLastTimeNormal = currenttime;

	if (m_bResHpType == USER_DEAD)
		return;

	//char logstr[128] = {};
	//wsprintf(logstr, "HPTimeChange ,, nid=%d, name=%hs, hp=%d, type=%d ******", m_Sid, m_pUserData->m_id, m_pUserData->m_sHp, m_bResHpType);
	//TimeTrace(logstr);

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE
		&& m_pMain->m_byBattleOpen == SNOW_BATTLE)
	{
		if (m_pUserData->m_sHp < 1)
			return;

		HpChange(5);
		return;
	}

	if (m_bResHpType == USER_STANDING)
	{
		if (m_pUserData->m_sHp < 1)
			return;

		if (m_iMaxHp != m_pUserData->m_sHp)
			HpChange((int) ((m_pUserData->m_bLevel * (1 + m_pUserData->m_bLevel / 60.0) + 1) * 0.2) + 3);

		if (m_iMaxMp != m_pUserData->m_sMp)
			MSpChange((int) ((m_pUserData->m_bLevel * (1 + m_pUserData->m_bLevel / 60.0) + 1) * 0.2) + 3);
	}
	else if (m_bResHpType == USER_SITDOWN)
	{
		if (m_pUserData->m_sHp < 1)
			return;

		if (m_iMaxHp != m_pUserData->m_sHp)
			HpChange((int) (m_pUserData->m_bLevel * (1 + m_pUserData->m_bLevel / 30.0)) + 3);

		if (m_iMaxMp != m_pUserData->m_sMp)
			MSpChange((int) ((m_iMaxMp * 5) / ((m_pUserData->m_bLevel - 1) + 30)) + 3);
	}

	/*
	나중에 또 고칠것에 대비해서 여기에 두기로 했습니다 :

	HP(MP)가 모두 차는 데 걸리는 시간 A = (레벨 - 1) + 30
	HP(MP)가 모두 차는 데 걸리는 횟수 B = A/5
	한번에 차는 HP(MP)의 양 = Max HP / B
	*/
}

void CUser::HPTimeChangeType3(float currenttime)
{
	int send_index = 0;
	char send_buff[128] = {};

	// Get the current time for all the last times...
	for (int g = 0; g < MAX_TYPE3_REPEAT; g++)
		m_fHPLastTime[g] = currenttime;

	// Make sure the user is not dead first!!!
	if (m_bResHpType == USER_DEAD)
		return;

	for (int h = 0; h < MAX_TYPE3_REPEAT; h++)
	{
		HpChange(m_bHPAmount[h]);	// Reduce HP...

		CUser* pUser = nullptr;

		// Send report to the source...
		if (m_sSourceID[h] >= 0
			&& m_sSourceID[h] < MAX_USER)
		{
			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sSourceID[h]];
			if (pUser != nullptr)
				pUser->SendTargetHP(0, m_Sid, m_bHPAmount[h]);
		}

		// Check if the target is dead.	
		if (m_pUserData->m_sHp == 0)
		{
			m_bResHpType = USER_DEAD;	// Officially declare the user DEAD!!!!!

			// If the killer was a NPC
			if (m_sSourceID[h] >= NPC_BAND)
			{
				if (m_pUserData->m_bZone != m_pUserData->m_bNation
					&& m_pUserData->m_bZone < 3)
				{
					ExpChange(-m_iMaxExp / 100);
					//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
				}
				else
				{
					ExpChange(-m_iMaxExp / 20);     // Reduce target experience.		
				}
			}
			// You got killed by another player
			else
			{
				// (No more pointer mistakes....)
				if (pUser != nullptr)
				{
					// Something regarding loyalty points.
					if (pUser->m_sPartyIndex == -1)
						pUser->LoyaltyChange(m_Sid);
					else
						pUser->LoyaltyDivide(m_Sid);

					pUser->GoldChange(m_Sid, 0);
				}
			}
			// 기범이의 완벽한 보호 코딩!!!
			InitType3();	// Init Type 3.....
			InitType4();	// Init Type 4.....

			if (m_sSourceID[h] >= 0 && m_sSourceID[h] < MAX_USER)
			{
				m_sWhoKilledMe = m_sSourceID[h];	// Who the hell killed me?
//
				if (m_pUserData->m_bZone != m_pUserData->m_bNation
					&& m_pUserData->m_bZone < 3)
				{
					ExpChange(-m_iMaxExp / 100);
					//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
				}
//
			}

			break;	// Exit the for loop :)
		}
	}

	// Type 3 Cancellation Process.
	for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
	{
		if (m_bHPDuration[i] > 0)
		{
			if (((currenttime - m_fHPStartTime[i]) >= m_bHPDuration[i])
				|| m_bResHpType == USER_DEAD)
			{
				/*	Send Party Packet.....
				if (m_sPartyIndex != -1)
				{
					SetByte(send_buff, WIZ_PARTY, send_index );
					SetByte(send_buff, PARTY_STATUSCHANGE, send_index );
					SetShort(send_buff, m_Sid, send_index );
					SetByte(send_buff, 1, send_index );
					SetByte(send_buff, 0, send_index);
					m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
				}
				//  end of Send Party Packet.....*/

				SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
				SetByte(send_buff, MAGIC_TYPE3_END, send_index);

				if (m_bHPAmount[i] > 0)
					SetByte(send_buff, 100, send_index);
				else
					SetByte(send_buff, 200, send_index);

				Send(send_buff, send_index);
				memset(send_buff, 0, sizeof(send_buff));
				send_index = 0;

				m_fHPStartTime[i] = 0.0f;
				m_fHPLastTime[i] = 0.0f;
				m_bHPAmount[i] = 0;
				m_bHPDuration[i] = 0;
				m_bHPInterval[i] = 5;
				m_sSourceID[i] = -1;
			}
		}
	}

	int buff_test = 0;
	for (int j = 0; j < MAX_TYPE3_REPEAT; j++)
		buff_test += m_bHPDuration[j];

	if (buff_test == 0)
		m_bType3Flag = false;

	bool bType3Test = true;
	for (int k = 0; k < MAX_TYPE3_REPEAT; k++)
	{
		if (m_bHPAmount[k] < 0)
		{
			bType3Test = false;
			break;
		}
	}

	// Send Party Packet.....
	if (m_sPartyIndex != -1
		&& bType3Test)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetByte(send_buff, 1, send_index);
		SetByte(send_buff, 0, send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
	}
	//  end of Send Party Packet.....  //
//
}

void CUser::ItemRepair(char* pBuf)
{
	int index = 0, send_index = 0, money = 0, quantity = 0;
	int itemid = 0, pos = 0, slot = -1, durability = 0;
	char send_buff[128] = {};
	model::Item* pTable = nullptr;

	pos = GetByte(pBuf, index);
	slot = GetByte(pBuf, index);
	itemid = GetDWORD(pBuf, index);

	// SLOT
	if (pos == 1)
	{
		if (slot >= SLOT_MAX)
			goto fail_return;

		if (m_pUserData->m_sItemArray[slot].nNum != itemid)
			goto fail_return;
	}
	// INVEN
	else if (pos == 2)
	{
		if (slot >= HAVE_MAX)
			goto fail_return;

		if (m_pUserData->m_sItemArray[SLOT_MAX + slot].nNum != itemid)
			goto fail_return;
	}

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		goto fail_return;

	durability = pTable->Durability;

	if (durability == 1)
		goto fail_return;

	if (pos == 1)
		quantity = pTable->Durability - m_pUserData->m_sItemArray[slot].sDuration;
	else if (pos == 2)
		quantity = pTable->Durability - m_pUserData->m_sItemArray[SLOT_MAX + slot].sDuration;

	money = (int) (((pTable->BuyPrice - 10) / 10000.0f) + pow(pTable->BuyPrice, 0.75)) * quantity / (double) durability;
	if (money > m_pUserData->m_iGold)
		goto fail_return;

	m_pUserData->m_iGold -= money;
	if (pos == 1)
		m_pUserData->m_sItemArray[slot].sDuration = durability;
	else if (pos == 2)
		m_pUserData->m_sItemArray[SLOT_MAX + slot].sDuration = durability;

	SetByte(send_buff, WIZ_ITEM_REPAIR, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_ITEM_REPAIR, send_index);
	SetByte(send_buff, 0x00, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	Send(send_buff, send_index);
}

void CUser::Type4Duration(float currenttime)
{
	int send_index = 0;
	char send_buff[128] = {};
	BYTE buff_type = 0;

	if (m_sDuration1 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime1 + m_sDuration1))
		{
			m_sDuration1 = 0;
			m_fStartTime1 = 0.0f;
			m_sMaxHPAmount = 0;
			buff_type = 1;
		}
	}

	if (m_sDuration2 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime2 + m_sDuration2))
		{
			m_sDuration2 = 0;
			m_fStartTime2 = 0.0f;
			m_sACAmount = 0;
			buff_type = 2;
		}
	}

	if (m_sDuration3 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime3 + m_sDuration3))
		{
			m_sDuration3 = 0;
			m_fStartTime3 = 0.0f;
			buff_type = 3;

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, 3, send_index);	// You are now normal again!!!
			SetByte(send_buff, ABNORMAL_NORMAL, send_index);
			StateChange(send_buff);
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
		}
	}

	if (m_sDuration4 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime4 + m_sDuration4))
		{
			m_sDuration4 = 0;
			m_fStartTime4 = 0.0f;
			m_bAttackAmount = 100;
			buff_type = 4;
		}
	}

	if (m_sDuration5 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime5 + m_sDuration5))
		{
			m_sDuration5 = 0;
			m_fStartTime5 = 0.0f;
			m_bAttackSpeedAmount = 100;
			buff_type = 5;
		}
	}

	if (m_sDuration6 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime6 + m_sDuration6))
		{
			m_sDuration6 = 0;
			m_fStartTime6 = 0.0f;
			m_bSpeedAmount = 100;
			buff_type = 6;
		}
	}

	if (m_sDuration7 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime7 + m_sDuration7))
		{
			m_sDuration7 = 0;
			m_fStartTime7 = 0.0f;
			m_bStrAmount = 0;
			m_bStaAmount = 0;
			m_bDexAmount = 0;
			m_bIntelAmount = 0;
			m_bChaAmount = 0;
			buff_type = 7;
		}
	}

	if (m_sDuration8 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime8 + m_sDuration8))
		{
			m_sDuration8 = 0;
			m_fStartTime8 = 0.0f;
			m_bFireRAmount = 0;
			m_bColdRAmount = 0;
			m_bLightningRAmount = 0;
			m_bMagicRAmount = 0;
			m_bDiseaseRAmount = 0;
			m_bPoisonRAmount = 0;
			buff_type = 8;
		}
	}

	if (m_sDuration9 != 0
		&& buff_type == 0)
	{
		if (currenttime > (m_fStartTime9 + m_sDuration9))
		{
			m_sDuration9 = 0;
			m_fStartTime9 = 0.0f;
			m_bHitRateAmount = 100;
			m_sAvoidRateAmount = 100;
			buff_type = 9;
		}
	}

	if (buff_type != 0)
	{
		m_bType4Buff[buff_type - 1] = 0;

		SetSlotItemValue();
		SetUserAbility();
		Send2AI_UserUpdateInfo();	// AI Server에 바끤 데이타 전송....

		/*	Send Party Packet.....
		if (m_sPartyIndex != -1)
		{
			SetByte(send_buff, WIZ_PARTY, send_index);
			SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
			SetShort(send_buff, m_Sid, send_index);
//			if (buff_type != 5 && buff_type != 6)
//				SetByte(send_buff, 3, send_index);
//			else
				SetByte(send_buff, 2, send_index);
			SetByte(send_buff, 0, send_index);
			m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
		}
		//  end of Send Party Packet.....  */

		SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);
		SetByte(send_buff, MAGIC_TYPE4_END, send_index);
		SetByte(send_buff, buff_type, send_index);
		Send(send_buff, send_index);
	}

	int buff_test = 0;
	for (int i = 0; i < MAX_TYPE4_BUFF; i++)
		buff_test += m_bType4Buff[i];

	if (buff_test == 0)
		m_bType4Flag = false;

	bool bType4Test = true;
	for (int j = 0; j < MAX_TYPE4_BUFF; j++)
	{
		if (m_bType4Buff[j] == 1)
		{
			bType4Test = false;
			break;
		}
	}
//
	// Send Party Packet.....
	if (m_sPartyIndex != -1
		&& bType4Test)
	{
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_PARTY, send_index);
		SetByte(send_buff, PARTY_STATUSCHANGE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetByte(send_buff, 2, send_index);
		SetByte(send_buff, 0, send_index);
		m_pMain->Send_PartyMember(m_sPartyIndex, send_buff, send_index);
	}
	//  end of Send Party Packet.....  //
//
}

// Returns:
// 0: Requested item not available.
// 1: Amount is greater than current item count.
// 2: Success. Current item count updated or deleted.
BYTE CUser::ItemCountChange(int itemid, int type, int amount)
{
	int send_index = 0, result = 0, slot = -1;				
	char send_buff[128] = {};

	// This checks if such an item exists.
	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
	{
		result = 0;
		return result;
	}

	for (int i = SLOT_MAX * type; i < SLOT_MAX + HAVE_MAX * type; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum != itemid)
			continue;

		if (pTable->RequiredDexterity > (m_pUserData->m_bDex + m_sItemDex + m_bDexAmount)
			&& pTable->RequiredDexterity != 0)
			return result;

		if (pTable->RequiredStrength > (m_pUserData->m_bStr + m_sItemStr + m_bStrAmount)
			&& pTable->RequiredStrength != 0)
			return result;

		if (pTable->RequiredStamina > (m_pUserData->m_bSta + m_sItemSta + m_bStaAmount)
			&& pTable->RequiredStamina != 0)
			return result;

		if (pTable->RequiredIntelligence > (m_pUserData->m_bIntel + m_sItemIntel + m_bIntelAmount)
			&& pTable->RequiredIntelligence != 0)
			return result;

		if (pTable->RequiredCharisma > (m_pUserData->m_bCha + m_sItemCham + m_bChaAmount)
			&& pTable->RequiredCharisma != 0)
			return result;

		// This checks if the user actually has that item.
		if (pTable->Countable == 0)
		{
			result = 2;
			return result;
		}

		slot = i;
		m_pUserData->m_sItemArray[i].sCount -= amount;

		// You have just ran out of items.
		if (m_pUserData->m_sItemArray[i].sCount == 0)
		{
			m_pUserData->m_sItemArray[i].nNum = 0;
			result = 2;
			break;
		}
		// Insufficient number of items.
		else if (m_pUserData->m_sItemArray[i].sCount < 0)
		{
			m_pUserData->m_sItemArray[i].sCount += amount;
			result = 1;
			break;
		}

		// No error detected....
		result = 2;
	}

	// Something happened :(
	if (result < 2)
		return result;

	SendItemWeight();

	SetByte(send_buff, WIZ_ITEM_COUNT_CHANGE, send_index);
	SetShort(send_buff, 1, send_index);	// The number of for-loops
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, slot - type * SLOT_MAX, send_index);
	SetDWORD(send_buff, itemid, send_index);	// The ID of item.
	SetDWORD(send_buff, m_pUserData->m_sItemArray[slot].sCount, send_index);

	Send(send_buff, send_index);

	return result;		// Success :) 
}

void CUser::SendAllKnightsID()
{
	int send_index = 0, count = 0, buff_index = 0;
	char send_buff[4096] = {},
		temp_buff[4096] = {};

	for (const auto& [_, pKnights] : m_pMain->m_KnightsMap)
	{
		if (pKnights == nullptr)
			continue;

		//if (pKnights->bFlag != KNIGHTS_TYPE)
		//	continue;

		SetShort(temp_buff, pKnights->m_sIndex, buff_index);
		SetShort(temp_buff, strlen(pKnights->m_strName), buff_index);
		SetString(temp_buff, pKnights->m_strName, strlen(pKnights->m_strName), buff_index);
		++count;
	}

	SetByte(send_buff, WIZ_KNIGHTS_LIST, send_index);
	SetByte(send_buff, 0x01, send_index);					// All List 
	SetShort(send_buff, count, send_index);
	SetString(send_buff, temp_buff, buff_index, send_index);
	SendCompressingPacket(send_buff, send_index);
	//Send( send_buff, send_index );
}

void CUser::ItemRemove(char* pBuf)
{
	int index = 0, send_index = 0, slot = 0, pos = 0, itemid = 0, count = 0, durability = 0;
	int64_t serial = 0;
	char send_buff[128] = {};

	slot = GetByte(pBuf, index);
	pos = GetByte(pBuf, index);
	itemid = GetDWORD(pBuf, index);

	if (slot == 1)
	{
		if (pos > SLOT_MAX)
			goto fail_return;

		if (m_pUserData->m_sItemArray[pos].nNum != itemid)
			goto fail_return;

		count = m_pUserData->m_sItemArray[pos].sCount;
		durability = m_pUserData->m_sItemArray[pos].sDuration;
		serial = m_pUserData->m_sItemArray[pos].nSerialNum;

		m_pUserData->m_sItemArray[pos].nNum = 0;
		m_pUserData->m_sItemArray[pos].sCount = 0;
		m_pUserData->m_sItemArray[pos].sDuration = 0;
		m_pUserData->m_sItemArray[pos].nSerialNum = 0;
	}
	else
	{
		if (pos > HAVE_MAX)
			goto fail_return;

		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != itemid)
			goto fail_return;

		count = m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount;
		durability = m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration;
		serial = m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum;

		m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum = 0;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = 0;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = 0;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum = 0;
	}

	SendItemWeight();

	SetByte(send_buff, WIZ_ITEM_REMOVE, send_index);
	SetByte(send_buff, 0x01, send_index);
	Send(send_buff, send_index);

	ItemLogToAgent(
		m_pUserData->m_id,
		"DESTROY",
		ITEM_LOG_DESTROY,
		serial,
		itemid,
		count,
		durability);
	return;

fail_return:
	SetByte(send_buff, WIZ_ITEM_REMOVE, send_index);
	SetByte(send_buff, 0, send_index);
	Send(send_buff, send_index);
}

void CUser::OperatorCommand(char* pBuf)
{
	int index = 0, idlen = 0;
	char userid[MAX_ID_SIZE + 1] = {};
	CUser* pUser = nullptr;

	// Is this user's authority operator?
	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	BYTE command = GetByte(pBuf, index);
	idlen = GetShort(pBuf, index);

	if (idlen < 0
		|| idlen > MAX_ID_SIZE)
		return;

	GetString(userid, pBuf, idlen, index);

	pUser = m_pMain->GetUserPtr(userid, NameType::Character);
	if (pUser == nullptr)
		return;

	switch (command)
	{
		case OPERATOR_ARREST:
			ZoneChange(pUser->m_pUserData->m_bZone, pUser->m_pUserData->m_curx, pUser->m_pUserData->m_curz);
			break;

		case OPERATOR_FORBID_CONNECT:
			pUser->m_pUserData->m_bAuthority = AUTHORITY_BLOCK_USER;
			pUser->Close();
			break;

		case OPERATOR_CHAT_FORBID:
			pUser->m_pUserData->m_bAuthority = AUTHORITY_NOCHAT;
			break;

		case OPERATOR_CHAT_PERMIT:
			pUser->m_pUserData->m_bAuthority = AUTHORITY_USER;
			break;
	}
}

void CUser::SpeedHackTime(char* pBuf)
{
	BYTE b_first = 0x00;
	int index = 0;
	float servertime = 0.0f, clienttime = 0.0f, client_gap = 0.0f, server_gap = 0.0f;

	b_first = GetByte(pBuf, index);
	clienttime = Getfloat(pBuf, index);

	if (b_first)
	{
		m_fSpeedHackClientTime = clienttime;
		m_fSpeedHackServerTime = TimeGet();
	}
	else
	{
		servertime = TimeGet();

		server_gap = servertime - m_fSpeedHackServerTime;
		client_gap = clienttime - m_fSpeedHackClientTime;

		if ((client_gap - server_gap) > 10.0f)
		{
			spdlog::debug("User::SpeedHackTime: speed hack check performed on charId={}", m_pUserData->m_id);
			Close();
		}
		else if (client_gap - server_gap < 0.0f)
		{
			m_fSpeedHackClientTime = clienttime;
			m_fSpeedHackServerTime = TimeGet();
		}
	}

/*	float currenttime;
	if (m_fSpeedHackTime == 0.0f)
		m_fSpeedHackTime = TimeGet();
	else
	{
		currenttime = TimeGet();
		if ((currenttime - m_fSpeedHackTime) < 48.0f)
		{
			char logstr[256] = {};
			sprintf(logstr, "%s SpeedHack User Checked By Server Time\r\n", m_pUserData->m_id);
			LogFileWrite( logstr );

//			if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
//				m_pUserData->m_bAuthority = AUTHORITY_BLOCK_USER;

			Close();
		}
	}

	m_fSpeedHackTime = TimeGet();
*/
}

// server의 상태를 체크..
void CUser::ServerStatusCheck()
{
	int send_index = 0;
	char send_buff[256] = {};
	SetByte(send_buff, WIZ_SERVER_CHECK, send_index);
	SetShort(send_buff, m_pMain->m_sErrorSocketCount, send_index);
	Send(send_buff, send_index);
}

void CUser::Type3AreaDuration(float currenttime)
{
	int send_index = 0;
	char send_buff[128] = {};

	CMagicProcess magic_process;

	model::MagicType3* pType = m_pMain->m_MagicType3TableMap.GetData(m_iAreaMagicID);      // Get magic skill table type 3.
	if (pType == nullptr)
		return;

	// Did one second pass?
	if (m_fAreaLastTime != 0.0f
		&& (currenttime - m_fAreaLastTime) > m_bAreaInterval)
	{
		m_fAreaLastTime = currenttime;
		if (m_bResHpType == USER_DEAD)
			return;

		// Actual damage procedure.
		for (int i = 0; i < MAX_USER; i++)
		{
			// Region check.
			if (!magic_process.UserRegionCheck(m_Sid, i, m_iAreaMagicID, pType->Radius))
				continue;

			CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];
			if (pTUser == nullptr)
				continue;

			SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);	// Set packet.
			SetByte(send_buff, MAGIC_EFFECTING, send_index);
			SetDWORD(send_buff, m_iAreaMagicID, send_index);
			SetShort(send_buff, m_Sid, send_index);
			SetShort(send_buff, i, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
		}

		// Did area duration end?
		if (((currenttime - m_fAreaStartTime) >= pType->Duration)
			|| m_bResHpType == USER_DEAD)
		{
			m_bAreaInterval = 5;
			m_fAreaStartTime = 0.0f;
			m_fAreaLastTime = 0.0f;
			m_iAreaMagicID = 0;
		}
	}


	SetByte(send_buff, WIZ_MAGIC_PROCESS, send_index);	// Set packet.
	SetByte(send_buff, MAGIC_EFFECTING, send_index);
	SetDWORD(send_buff, m_iAreaMagicID, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);
	SetShort(send_buff, 0, send_index);

	// Send packet to region.
	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
}

void CUser::WarehouseProcess(char* pBuf)
{
	int index = 0, send_index = 0, itemid = 0, srcpos = -1, destpos = -1, page = -1, reference_pos = -1, count = 0;
	char send_buff[2048] = {};
	model::Item* pTable = nullptr;

	BYTE command = GetByte(pBuf, index);

	// 창고 안되게...
	if (m_bResHpType == USER_DEAD
		|| m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::WarehouseProcess: dead user cannot use warehouse [charId={} socketId={} resHpType={} hp={} x={} z={}]",
			m_pUserData->m_id, m_Sid, m_bResHpType, m_pUserData->m_sHp,
			static_cast<int32_t>(m_pUserData->m_curx), static_cast<int32_t>(m_pUserData->m_curz));
		return;
	}

	if (m_sExchangeUser != -1)
		goto fail_return;

	if (command == WAREHOUSE_OPEN)
	{
		SetByte(send_buff, WIZ_WAREHOUSE, send_index);
		SetByte(send_buff, WAREHOUSE_OPEN, send_index);
		SetByte(send_buff, 1, send_index); /* success */
		SetDWORD(send_buff, m_pUserData->m_iBank, send_index);

		for (int i = 0; i < WAREHOUSE_MAX; i++)
		{
			SetDWORD(send_buff, m_pUserData->m_sWarehouseArray[i].nNum, send_index);
			SetShort(send_buff, m_pUserData->m_sWarehouseArray[i].sDuration, send_index);
			SetShort(send_buff, m_pUserData->m_sWarehouseArray[i].sCount, send_index);
		}

		SendCompressingPacket(send_buff, send_index);
		return;
	}

	itemid = GetDWORD(pBuf, index);
	page = GetByte(pBuf, index);
	srcpos = GetByte(pBuf, index);
	destpos = GetByte(pBuf, index);
	pTable = m_pMain->m_ItemTableMap.GetData(itemid);

	if (pTable == nullptr)
		goto fail_return;

	reference_pos = 24 * page;

	switch (command)
	{
		case WAREHOUSE_INPUT:
			count = GetDWORD(pBuf, index);

			if (itemid == ITEM_GOLD)
			{
				if ((m_pUserData->m_iBank + count) > 2'100'000'000)
					goto fail_return;

				if ((m_pUserData->m_iGold - count) < 0)
					goto fail_return;

				m_pUserData->m_iBank += count;
				m_pUserData->m_iGold -= count;
				break;
			}

			if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum != itemid)
				goto fail_return;

			if (reference_pos + destpos > WAREHOUSE_MAX)
				goto fail_return;

			if (m_pUserData->m_sWarehouseArray[reference_pos + destpos].nNum
				&& pTable->Countable == 0)
				goto fail_return;

			if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount < count)
				goto fail_return;

			m_pUserData->m_sWarehouseArray[reference_pos + destpos].nNum = itemid;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

			if (pTable->Countable == 0
				&& m_pUserData->m_sWarehouseArray[reference_pos + destpos].nSerialNum == 0)
				m_pUserData->m_sWarehouseArray[reference_pos + destpos].nSerialNum = m_pMain->GenerateItemSerial();

			if (pTable->Countable != 0)
				m_pUserData->m_sWarehouseArray[reference_pos + destpos].sCount += count;
			else
				m_pUserData->m_sWarehouseArray[reference_pos + destpos].sCount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;

			if (!pTable->Countable)
			{
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = 0;
			}
			else
			{
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount -= count;

				if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount <= 0)
				{
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = 0;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = 0;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = 0;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = 0;
				}
			}

			SendItemWeight();
			ItemLogToAgent(
				m_pUserData->m_Accountid,
				m_pUserData->m_id,
				ITEM_LOG_WAREHOUSE_PUT,
				0,
				itemid,
				count,
				m_pUserData->m_sWarehouseArray[reference_pos + destpos].sDuration);
			break;

		case WAREHOUSE_OUTPUT:
			count = GetDWORD(pBuf, index);

			if (itemid == ITEM_GOLD)
			{
				if ((m_pUserData->m_iGold + count) > 2'100'000'000)
					goto fail_return;

				if ((m_pUserData->m_iBank - count) < 0)
					goto fail_return;

				m_pUserData->m_iGold += count;
				m_pUserData->m_iBank -= count;
				break;
			}

			// Check weight of countable item.
			if (pTable->Countable != 0)
			{
				if (((pTable->Weight * count) + m_iItemWeight) > m_iMaxWeight)
					goto fail_return;
			}
			// Check weight of non-countable item.
			else
			{
				if ((pTable->Weight + m_iItemWeight) > m_iMaxWeight)
					goto fail_return;
			}

			if ((reference_pos + srcpos) > WAREHOUSE_MAX)
				goto fail_return;

			if (m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum != itemid)
				goto fail_return;

			if (m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum
				&& pTable->Countable == 0)
				goto fail_return;

			if (m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount < count)
				goto fail_return;

			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum = itemid;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum;

			if (pTable->Countable != 0)
			{
				m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount += count;
			}
			else
			{
				if (m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum == 0)
					m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = m_pMain->GenerateItemSerial();

				m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount;
			}

			if (pTable->Countable == 0)
			{
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum = 0;
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration = 0;
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount = 0;
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum = 0;
			}
			else
			{
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount -= count;

				if (m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount <= 0)
				{
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum = 0;
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration = 0;
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount = 0;
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum = 0;
				}
			}

			SendItemWeight();
			ItemLogToAgent(
				m_pUserData->m_id,
				m_pUserData->m_Accountid,
				ITEM_LOG_WAREHOUSE_GET,
				0,
				itemid,
				count,
				m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration);
			//TRACE(_T("WARE OUTPUT : %hs %hs %d %d %d %d %d"), m_pUserData->m_id, m_pUserData->m_Accountid, ITEM_WAREHOUSE_GET, 0, itemid, count, m_pUserData->m_sItemArray[SLOT_MAX+destpos].sDuration );
			break;

		case WAREHOUSE_MOVE:
			if ((reference_pos + srcpos) > WAREHOUSE_MAX)
				goto fail_return;

			if (m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum != itemid)
				goto fail_return;

			if (m_pUserData->m_sWarehouseArray[reference_pos + destpos].nNum)
				goto fail_return;

			m_pUserData->m_sWarehouseArray[reference_pos + destpos].nNum = itemid;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos].sDuration = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos].sCount = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos].nSerialNum = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum;

			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum = 0;
			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration = 0;
			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount = 0;
			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum = 0;
			break;

		case WAREHOUSE_INVENMOVE:
		{
			if (itemid != m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum)
				goto fail_return;

				short duration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
				short itemcount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;
				int64_t serial = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum;

				m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum = itemid;
				m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration = duration;
				m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount = itemcount;
				m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = serial;
			}
			break;
	}

	m_pUserData->m_bWarehouse = 1;

	SetByte(send_buff, WIZ_WAREHOUSE, send_index);
	SetByte(send_buff, command, send_index);
	SetByte(send_buff, 0x01, send_index);
	Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_WAREHOUSE, send_index);
	SetByte(send_buff, command, send_index);
	SetByte(send_buff, 0x00, send_index);
	Send(send_buff, send_index);
}

void CUser::InitType4()
{
	m_bAttackSpeedAmount = 100;		// this is for the duration spells Type 4
	m_bSpeedAmount = 100;
	m_sACAmount = 0;
	m_bAttackAmount = 100;
	m_sMaxHPAmount = 0;
	m_bHitRateAmount = 100;
	m_sAvoidRateAmount = 100;
	m_bStrAmount = 0;
	m_bStaAmount = 0;
	m_bDexAmount = 0;
	m_bIntelAmount = 0;
	m_bChaAmount = 0;
	m_bFireRAmount = 0;
	m_bColdRAmount = 0;
	m_bLightningRAmount = 0;
	m_bMagicRAmount = 0;
	m_bDiseaseRAmount = 0;
	m_bPoisonRAmount = 0;
// 비러머글 수능
	m_bAbnormalType = 1;
//
	m_sDuration1 = 0;  m_fStartTime1 = 0.0f;		// Used for Type 4 Durational Spells.
	m_sDuration2 = 0;  m_fStartTime2 = 0.0f;
	m_sDuration3 = 0;  m_fStartTime3 = 0.0f;
	m_sDuration4 = 0;  m_fStartTime4 = 0.0f;
	m_sDuration5 = 0;  m_fStartTime5 = 0.0f;
	m_sDuration6 = 0;  m_fStartTime6 = 0.0f;
	m_sDuration7 = 0;  m_fStartTime7 = 0.0f;
	m_sDuration8 = 0;  m_fStartTime8 = 0.0f;
	m_sDuration9 = 0;  m_fStartTime9 = 0.0f;

	for (int h = 0; h < MAX_TYPE4_BUFF; h++)
		m_bType4Buff[h] = 0;

	m_bType4Flag = false;
}

int CUser::GetNumberOfEmptySlots() const
{
	int emptySlotCount = 0;

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		const _ITEM_DATA& item = m_pUserData->m_sItemArray[i];
		if (item.nNum == 0)
			++emptySlotCount;
	}

	return emptySlotCount;
}

bool CUser::CheckExistEvent(int16_t questId, uint8_t questState) const
{
	for (const _USER_QUEST& quest : m_pUserData->m_quests)
	{
		if (quest.sQuestID != questId)
			continue;

		// Quest found - state must match
		return (quest.byQuestState == questState);
	}

	// Quest not found - only return true if we're checking if it doesn't exist
	return (questState == QUEST_STATE_NOT_STARTED);
}

// item 먹을때 비어잇는 슬롯을 찾아야되...
int CUser::GetEmptySlot(int itemid, int bCountable) const
{
	int pos = 255, i = 0;

	model::Item* pTable = nullptr;

	if (bCountable == -1)
	{
		pTable = m_pMain->m_ItemTableMap.GetData(itemid);
		if (pTable == nullptr)
			return pos;

		bCountable = pTable->Countable;
	}

	if (itemid == ITEM_GOLD)
		return pos;

	for (i = 0; i < HAVE_MAX; i++)
	{
		if (m_pUserData->m_sItemArray[SLOT_MAX + i].nNum != 0)
			continue;

		pos = i;
		break;
	}

	if (bCountable)
	{
		for (i = 0; i < HAVE_MAX; i++)
		{
			if (m_pUserData->m_sItemArray[SLOT_MAX + i].nNum == itemid)
				return i;
		}

		if (i == HAVE_MAX)
			return pos;
	}

	return pos;
}

void CUser::ReportBug(char* pBuf)
{
	// Beep(3000, 200);	// Let's hear a beep from the speaker.

	int index = 0, chatlen = 0, send_index = 0;
	char chatMsg[1024] = {};

	chatlen = GetShort(pBuf, index);
	if (chatlen > 512
		|| chatlen <= 0)
		return;

	GetString(chatMsg, pBuf, chatlen, index);

//	TRACE( " Short : %d   String : %hs  \n ", chatlen, chatstr);
	if (strlen(m_pUserData->m_id) == 0)
		return;

	spdlog::info("User::ReportBug: [charId={} chatMsg={}]", m_pUserData->m_id, chatMsg);
}

void CUser::Home()
{
	int send_index = 0;
	char send_buff[128] = {};

	short x = 0, z = 0;		// The point where you will be warped to.
	if (!GetStartPosition(&x, &z))
		return;

	SetShort(send_buff, (WORD) (x * 10), send_index);
	SetShort(send_buff, (WORD) (z * 10), send_index);
	Warp(send_buff);
}

bool CUser::GetStartPosition(short* x, short* z)
{
	model::StartPosition* startPosition = m_pMain->m_StartPositionTableMap.GetData(m_pUserData->m_bZone);
	if (startPosition == nullptr)
		return false;

	if (m_pUserData->m_bNation == KARUS)
	{
		*x = startPosition->KarusX + myrand(0, startPosition->RangeX);
		*z = startPosition->KarusZ + myrand(0, startPosition->RangeZ);
		return true;
	}
	else if (m_pUserData->m_bNation == ELMORAD)
	{
		*x = startPosition->ElmoX + myrand(0, startPosition->RangeX);
		*z = startPosition->ElmoZ + myrand(0, startPosition->RangeZ);
		return true;
	}

	return false;
}

CUser* CUser::GetItemRoutingUser(int itemid, short itemcount)
{
	if (m_sPartyIndex == -1)
		return nullptr;

	CUser* pUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;
	int select_user = -1, count = 0;

	pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
	if (pParty == nullptr)
		return nullptr;

	if (pParty->bItemRouting > 7)
		return nullptr;

	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return nullptr;

	while (count < 8)
	{
		select_user = pParty->uid[pParty->bItemRouting];
		if (select_user >= 0
			&& select_user < MAX_USER)
		{
			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[select_user];
			if (pUser != nullptr)
			{
//	이거 않되도 저를 너무 미워하지 마세요 ㅠ.ㅠ
				// Check weight of countable item.
				if (pTable->Countable)
				{
					if ((pTable->Weight * count + pUser->m_iItemWeight) <= pUser->m_iMaxWeight)
					{
						pParty->bItemRouting++;
						if (pParty->bItemRouting > 6)
							pParty->bItemRouting = 0;

						// 즉, 유저의 포인터를 리턴한다 :)
						return pUser;
					}
				}
				// Check weight of non-countable item.
				else
				{
					if ((pTable->Weight + pUser->m_iItemWeight) <= pUser->m_iMaxWeight)
					{
						pParty->bItemRouting++;

						if (pParty->bItemRouting > 6)
							pParty->bItemRouting = 0;

						// 즉, 유저의 포인터를 리턴한다 :)
						return pUser;
					}
				}
//

/*
				pParty->bItemRouting++;
				if (pParty->bItemRouting > 6)
					pParty->bItemRouting = 0;

				// 즉, 유저의 포인터를 리턴한다 :)
				return pUser;
*/
			}
		}

		if (pParty->bItemRouting > 6)
			pParty->bItemRouting = 0;
		else
			pParty->bItemRouting++;

		count++;
	}

	return nullptr;
}

void CUser::FriendReport(char* pBuf)
{
	int index = 0, send_index = 0;
	short usercount = 0, idlen = 0;
	char send_buff[256] = {},
		userid[MAX_ID_SIZE + 1] = {};
	CUser* pUser = nullptr;

	usercount = GetShort(pBuf, index);	// Get usercount packet.
	if (usercount >= 30
		|| usercount < 0)
		return;

	SetByte(send_buff, WIZ_FRIEND_PROCESS, send_index);
	SetShort(send_buff, usercount, send_index);

	for (int k = 0; k < usercount; k++)
	{
		idlen = GetShort(pBuf, index);
		if (idlen > MAX_ID_SIZE)
		{
			SetShort(send_buff, strlen(userid), send_index);
			SetString(send_buff, userid, strlen(userid), send_index);
			SetShort(send_buff, -1, send_index);
			SetByte(send_buff, 0, send_index);
			continue;
		}

		GetString(userid, pBuf, idlen, index);

		pUser = m_pMain->GetUserPtr(userid, NameType::Character);

		SetShort(send_buff, idlen, send_index);
		SetString(send_buff, userid, idlen, send_index);

		// No such user
		if (pUser == nullptr)
		{
			SetShort(send_buff, -1, send_index);
			SetByte(send_buff, 0, send_index);
		}
		else
		{
			SetShort(send_buff, pUser->m_Sid, send_index);
			if (pUser->m_sPartyIndex >= 0)
				SetByte(send_buff, 3, send_index);
			else
				SetByte(send_buff, 1, send_index);
		}
	}

	Send(send_buff, send_index);
}

void CUser::ClassChangeReq()
{
	char send_buff[128] = {};
	int send_index = 0;

	SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
	SetByte(send_buff, CLASS_CHANGE_RESULT, send_index);

	if (m_pUserData->m_bLevel < 10)
		SetByte(send_buff, 2, send_index);
	else if ((m_pUserData->m_sClass % 100) > 4)
		SetByte(send_buff, 3, send_index);
	else
		SetByte(send_buff, 1, send_index);
	Send(send_buff, send_index);
}

void CUser::AllSkillPointChange()
{
	// 돈을 먼저 깍고.. 만약,, 돈이 부족하면.. 에러...
	int index = 0, send_index = 0, skill_point = 0, money = 0, i = 0, j = 0, temp_value = 0, old_money = 0;
	BYTE type = 0;    // 0:돈이 부족, 1:성공, 2:초기화할 스킬이 없을때..
	char send_buff[128] = {};

	temp_value = pow((m_pUserData->m_bLevel * 2), 3.4);
	temp_value = (temp_value / 100) * 100;

	if (m_pUserData->m_bLevel < 30)	
		temp_value = static_cast<int>(temp_value * 0.4);
#if 0
	else if (m_pUserData->m_bLevel >= 30
		&& m_pUserData->m_bLevel < 60)
		temp_value = temp_value * 1;
#endif
	else if (m_pUserData->m_bLevel >= 60
		&& m_pUserData->m_bLevel <= 90)
		temp_value = static_cast<int>(temp_value * 1.5);

	// 스킬은 한번 더 
	temp_value = static_cast<int>(temp_value * 1.5);

	// 할인시점이고 승리국이라면
	if (m_pMain->m_sDiscount == 1
		&& m_pMain->m_byOldVictory == m_pUserData->m_bNation)
	{
		old_money = temp_value;
		temp_value = static_cast<int>(temp_value * 0.5);
		//TRACE(_T("^^ AllSkillPointChange - Discount ,, money=%d->%d\n"), old_money, temp_value);
	}

	if (m_pMain->m_sDiscount == 2)
	{
		old_money = temp_value;
		temp_value = static_cast<int>(temp_value * 0.5);
		//TRACE(_T("^^ AllSkillPointChange - Discount ,, money=%d->%d\n"), old_money, temp_value);
	}

	money = m_pUserData->m_iGold - temp_value;
	//money = m_pUserData->m_iGold - 100;

	if (money < 0)
		goto fail_return;

	if (m_pUserData->m_bLevel < 10)
		goto fail_return;

	for (i = 1; i < 9; i++)
		skill_point += m_pUserData->m_bstrSkill[i];

	if (skill_point <= 0)
	{
		type = 2;
		goto fail_return;
	}

	// 문제될 소지가 많음 : 가용스킬이 255을 넘는 상황이 발생할 확율이 높음..
	//m_pUserData->m_bstrSkill[0] += skill_point;		
	m_pUserData->m_bstrSkill[0] = (m_pUserData->m_bLevel - 9) * 2;
	for (j = 1; j < 9; j++)
		m_pUserData->m_bstrSkill[j] = 0;
	m_pUserData->m_iGold = money;
	type = 1;

	SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
	SetByte(send_buff, ALL_SKILLPT_CHANGE, send_index);
	SetByte(send_buff, type, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	SetByte(send_buff, m_pUserData->m_bstrSkill[0], send_index);
	Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
	SetByte(send_buff, ALL_SKILLPT_CHANGE, send_index);
	SetByte(send_buff, type, send_index);
	SetDWORD(send_buff, temp_value, send_index);
	Send(send_buff, send_index);
}

void CUser::AllPointChange()
{
	// 돈을 먼저 깍고.. 만약,, 돈이 부족하면.. 에러...
	int index = 0, send_index = 0, total_point = 0, money = 0, classcode = 0, temp_money = 0, old_money = 0;
	double dwMoney = 0;
	BYTE type = 0;
	char send_buff[128] = {};
	int i = 0;

	if (m_pUserData->m_bLevel > 80)
		goto fail_return;

	temp_money = pow((m_pUserData->m_bLevel * 2), 3.4);
	temp_money = (temp_money / 100) * 100;
	if (m_pUserData->m_bLevel < 30)
		temp_money = static_cast<int>(temp_money * 0.4);
#if 0
	else if (m_pUserData->m_bLevel >= 30
		&& m_pUserData->m_bLevel < 60)
		temp_money = static_cast<int>(temp_money * 1);
#endif
	else if (m_pUserData->m_bLevel >= 60
		&& m_pUserData->m_bLevel <= 90)
		temp_money = static_cast<int>(temp_money * 1.5);

	// 할인시점이고 승리국이라면
	if (m_pMain->m_sDiscount == 1
		&& m_pMain->m_byOldVictory == m_pUserData->m_bNation)
	{
		temp_money = static_cast<int>(temp_money * 0.5);
		//TRACE(_T("^^ AllPointChange - Discount ,, money=%d->%d\n"), old_money, temp_money);
	}

	if (m_pMain->m_sDiscount == 2)
		temp_money = static_cast<int>(temp_money * 0.5);

	money = m_pUserData->m_iGold - temp_money;
	if (money < 0)
		goto fail_return;

	// 장착아이템이 하나라도 있으면 에러처리 
	for (i = 0; i < SLOT_MAX; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum != 0)
		{
			type = 0x04;
			goto fail_return;
		}
	}

	switch (m_pUserData->m_bRace)
	{
		case KARUS_BIG:
			if (m_pUserData->m_bStr == 65
				&& m_pUserData->m_bSta == 65
				&& m_pUserData->m_bDex == 60
				&& m_pUserData->m_bIntel == 50
				&& m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr = 65;
			m_pUserData->m_bSta = 65;
			m_pUserData->m_bDex = 60;
			m_pUserData->m_bIntel = 50;
			m_pUserData->m_bCha = 50;
			break;

		case KARUS_MIDDLE:
			if (m_pUserData->m_bStr == 65
				&& m_pUserData->m_bSta == 65
				&& m_pUserData->m_bDex == 60
				&& m_pUserData->m_bIntel == 50
				&& m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr = 65;
			m_pUserData->m_bSta = 65;
			m_pUserData->m_bDex = 60;
			m_pUserData->m_bIntel = 50;
			m_pUserData->m_bCha = 50;
			break;

		case KARUS_SMALL:
			if (m_pUserData->m_bStr == 50
				&& m_pUserData->m_bSta == 50
				&& m_pUserData->m_bDex == 70
				&& m_pUserData->m_bIntel == 70
				&& m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr = 50;
			m_pUserData->m_bSta = 50;
			m_pUserData->m_bDex = 70;
			m_pUserData->m_bIntel = 70;
			m_pUserData->m_bCha = 50;
			break;

		case KARUS_WOMAN:
			if (m_pUserData->m_bStr == 50
				&& m_pUserData->m_bSta == 60
				&& m_pUserData->m_bDex == 60
				&& m_pUserData->m_bIntel == 70
				&& m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr = 50;
			m_pUserData->m_bSta = 60;
			m_pUserData->m_bDex = 60;
			m_pUserData->m_bIntel = 70;
			m_pUserData->m_bCha = 50;
			break;
	
		case BABARIAN:
			if (m_pUserData->m_bStr == 65
				&& m_pUserData->m_bSta == 65
				&& m_pUserData->m_bDex == 60
				&& m_pUserData->m_bIntel == 50
				&& m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr = 65;
			m_pUserData->m_bSta = 65;
			m_pUserData->m_bDex = 60;
			m_pUserData->m_bIntel = 50;
			m_pUserData->m_bCha = 50;
			break;

		case ELMORAD_MAN:
			if (m_pUserData->m_bStr == 60
				&& m_pUserData->m_bSta == 60
				&& m_pUserData->m_bDex == 70
				&& m_pUserData->m_bIntel == 50
				&& m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr = 60;
			m_pUserData->m_bSta = 60;
			m_pUserData->m_bDex = 70;
			m_pUserData->m_bIntel = 50;
			m_pUserData->m_bCha = 50;
			break;

		case ELMORAD_WOMAN:
			if (m_pUserData->m_bStr == 50
				&& m_pUserData->m_bSta == 50
				&& m_pUserData->m_bDex == 70
				&& m_pUserData->m_bIntel == 70
				&& m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr = 50;
			m_pUserData->m_bSta = 50;
			m_pUserData->m_bDex = 70;
			m_pUserData->m_bIntel = 70;
			m_pUserData->m_bCha = 50;
			break;
	}

	m_pUserData->m_bPoints = (m_pUserData->m_bLevel - 1) * 3 + 10;
	m_pUserData->m_iGold = money;

	SetUserAbility();
	Send2AI_UserUpdateInfo();

	type = 1;
	SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
	SetByte(send_buff, ALL_POINT_CHANGE, send_index);
	SetByte(send_buff, type, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	SetShort(send_buff, m_pUserData->m_bStr, send_index);
	SetShort(send_buff, m_pUserData->m_bSta, send_index);
	SetShort(send_buff, m_pUserData->m_bDex, send_index);
	SetShort(send_buff, m_pUserData->m_bIntel, send_index);
	SetShort(send_buff, m_pUserData->m_bCha, send_index);
	SetShort(send_buff, m_iMaxHp, send_index);
	SetShort(send_buff, m_iMaxMp, send_index);
	SetShort(send_buff, m_sTotalHit, send_index);
	SetShort(send_buff, GetMaxWeightForClient(), send_index);
	SetShort(send_buff, m_pUserData->m_bPoints, send_index);
	Send(send_buff, send_index);

fail_return:
	SetByte(send_buff, WIZ_CLASS_CHANGE, send_index);
	SetByte(send_buff, ALL_POINT_CHANGE, send_index);
	SetByte(send_buff, type, send_index);
	SetDWORD(send_buff, temp_money, send_index);
	Send(send_buff, send_index);

}

void CUser::GoldChange(short tid, int gold)
{

	// Money only changes in Frontier zone and Battle zone!!!
	if (m_pUserData->m_bZone < 3)
		return;

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
		return;

	// Users ONLY!!!
	if (tid >= MAX_USER
		|| tid < 0)
		return;

	int s_temp_gold = 0, t_temp_gold = 0, send_index = 0;
	BYTE s_type = 0, t_type = 0;    // 1 -> Get gold    2 -> Lose gold

	char send_buff[256] = {};

	CUser* pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[tid];
	if (pTUser == nullptr)
		return;

	if (pTUser->m_pUserData->m_iGold <= 0)
		return;

	// Reward money in battle zone!!!
	if (gold == 0)
	{
		// Source is NOT in a party.
		if (m_sPartyIndex == -1)
		{
			s_type = GOLD_CHANGE_GAIN;
			t_type = GOLD_CHANGE_LOSE;

			s_temp_gold = (pTUser->m_pUserData->m_iGold * 4) / 10;
			t_temp_gold = pTUser->m_pUserData->m_iGold / 2;

			m_pUserData->m_iGold += s_temp_gold;
			pTUser->m_pUserData->m_iGold -= t_temp_gold;
		}
		// When the source is in a party.
		else
		{
			_PARTY_GROUP* pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
			if (pParty == nullptr)
				return;

			s_type = GOLD_CHANGE_GAIN;
			t_type = GOLD_CHANGE_LOSE;

			s_temp_gold = (pTUser->m_pUserData->m_iGold * 4) / 10;
			t_temp_gold = pTUser->m_pUserData->m_iGold / 2;

			pTUser->m_pUserData->m_iGold -= t_temp_gold;

			SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);	// First the victim...
			SetByte(send_buff, t_type, send_index);
			SetDWORD(send_buff, t_temp_gold, send_index);
			SetDWORD(send_buff, pTUser->m_pUserData->m_iGold, send_index);
			pTUser->Send(send_buff, send_index);

			// For the loot sharing procedure...
			int usercount = 0, money = 0, levelsum = 0, count = 0;
			count = s_temp_gold;

			for (int i = 0; i < 8; i++)
			{
				if (pParty->uid[i] != -1)
				{
					usercount++;
					levelsum += pParty->bLevel[i];
				}
			}

			if (usercount == 0)
				return;

			for (int i = 0; i < 8; i++)
			{
				if (pParty->uid[i] != -1
					|| pParty->uid[i] >= MAX_USER)
				{
					CUser* pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[pParty->uid[i]];
					if (pUser == nullptr)
						continue;

					money = count * (float) (pUser->m_pUserData->m_bLevel / (float) levelsum);
					pUser->m_pUserData->m_iGold += money;

					// Now the party members...
					send_index = 0;
					memset(send_buff, 0, sizeof(send_buff));
					SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
					SetByte(send_buff, GOLD_CHANGE_GAIN, send_index);
					SetDWORD(send_buff, money, send_index);
					SetDWORD(send_buff, pUser->m_pUserData->m_iGold, send_index);
					pUser->Send(send_buff, send_index);
				}
			}

			return;
		}
	}
	// When actual values are provided.
	else
	{
		// Source gains money.
		if (gold > 0)
		{
			s_type = GOLD_CHANGE_GAIN;
			t_type = GOLD_CHANGE_LOSE;

			s_temp_gold = gold;
			t_temp_gold = gold;

			m_pUserData->m_iGold += s_temp_gold;
			pTUser->m_pUserData->m_iGold -= t_temp_gold;
		}
		// Source loses money.
		else
		{
			s_type = GOLD_CHANGE_LOSE;
			t_type = GOLD_CHANGE_GAIN;

			s_temp_gold = gold;
			t_temp_gold = gold;

			m_pUserData->m_iGold -= s_temp_gold;
			pTUser->m_pUserData->m_iGold += t_temp_gold;
		}
	}

	SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
	SetByte(send_buff, s_type, send_index);
	SetDWORD(send_buff, s_temp_gold, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	Send(send_buff, send_index);

	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));

	// Now the target
	SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
	SetByte(send_buff, t_type, send_index);
	SetDWORD(send_buff, t_temp_gold, send_index);
	SetDWORD(send_buff, pTUser->m_pUserData->m_iGold, send_index);
	pTUser->Send(send_buff, send_index);
}

void CUser::SelectWarpList(char* pBuf)
{
	int index = 0, send_index = 0, warpid = 0;
	_WARP_INFO* pWarp = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;
	C3DMap* pCurrentMap = nullptr, *pTargetMap = nullptr;
	char send_buff[128] = {};

// 비러머글 순간이동 >.<
	BYTE type = 2;
//		
	warpid = GetShort(pBuf, index);

	pCurrentMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pCurrentMap == nullptr)
		return;

	pWarp = pCurrentMap->m_WarpArray.GetData(warpid);
	if (pWarp == nullptr)
		return;

	// We cannot use warp gates when invading.
	if (m_pUserData->m_bNation != pWarp->sZone
		&& pWarp->sZone <= ZONE_ELMORAD)
		return;

	// We cannot use warp gates belonging to another nation.
	if (pWarp->sNation != 0
		&& pWarp->sNation != m_pUserData->m_bNation)
		return;

	pTargetMap = m_pMain->GetMapByID(pWarp->sZone);
	if (pTargetMap == nullptr)
		return;

	pInfo = m_pMain->m_ServerArray.GetData(pTargetMap->m_nServerNo);
	if (pInfo == nullptr)
		return;

	float rx = (float) myrand(0, (int) pWarp->fR * 2);
	if (rx < pWarp->fR)
		rx = -rx;

	float rz = (float) myrand(0, (int) pWarp->fR * 2);
	if (rz < pWarp->fR)
		rz = -rz;

// 비러머글 순간이동 >.<
/*
	SetByte(send_buff, WIZ_WARP_LIST, send_index);
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, 1, send_index);
	Send(send_buff, send_index);
*/

	if (m_pUserData->m_bZone == pWarp->sZone)
	{
		m_bZoneChangeSameZone = true;

		SetByte(send_buff, WIZ_WARP_LIST, send_index);
		SetByte(send_buff, type, send_index);
		SetByte(send_buff, 1, send_index);
		Send(send_buff, send_index);
	}
//
	ZoneChange(pWarp->sZone, pWarp->fX + rx, pWarp->fZ + rz);

/*	SetByte(send_buff, WIZ_VIRTUAL_SERVER, send_index);
	SetShort(send_buff, strlen(pInfo->strServerIP), send_index);
	SetString(send_buff, pInfo->strServerIP, strlen(pInfo->strServerIP), send_index);
	SetShort(send_buff, pInfo->sPort, send_index);
	Send(send_buff, send_index);*/
}

void CUser::ZoneConCurrentUsers(char* pBuf)
{
	int index = 0, send_index = 0, zone = 0, usercount = 0, nation = 0;
	char send_buff[128] = {};

	zone = GetShort(pBuf, index);
	nation = GetByte(pBuf, index);

	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bZone == zone
			&& pUser->m_pUserData->m_bNation == nation)
			usercount++;
	}

	SetByte(send_buff, WIZ_ZONE_CONCURRENT, send_index);
	SetShort(send_buff, usercount, send_index);
	Send(send_buff, send_index);
}

void CUser::ServerChangeOk(char* pBuf)
{
	int index = 0, warpid = 0;
	_WARP_INFO* pWarp = nullptr;
	C3DMap* pMap = nullptr;
	float rx = 0.0f, rz = 0.0f;
/* 비러머글 순간이동 >.<
	int send_index = 0;
	char send_buff[128] = {};
	BYTE type = 2 ;
*/
	warpid = GetShort(pBuf, index);

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	pWarp = pMap->m_WarpArray.GetData(warpid);
	if (pWarp == nullptr)
		return;

	rx = (float) myrand(0, (int) pWarp->fR * 2);
	if (rx < pWarp->fR) rx = -rx;
	rz = (float) myrand(0, (int) pWarp->fR * 2);
	if (rz < pWarp->fR) rz = -rz;

/* 비러머글 순간이동 >.<
	SetByte(send_buff, WIZ_WARP_LIST, send_index);
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, 1, send_index);
	Send(send_buff, send_index);
*/
	ZoneChange(pWarp->sZone, pWarp->fX + rx, pWarp->fZ + rz);
}

bool CUser::GetWarpList(int warp_group)
{
	int warpid = 0, send_index = 0;	// 헤더와 카운트를 나중에 패킹...
	int zoneindex = -1, temp_index = 0, count = 0;
	char buff[8192] = {};
	char send_buff[8192] = {};
// 비러머글 마을 이름 표시 >.<
	BYTE type = 1;		// 1이면 일반, 2이면 워프 성공했는지 않했는지 ^^;
//

	C3DMap* pCurrentMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pCurrentMap == nullptr)
		return false;

	for (const auto& [_, pWarp] : pCurrentMap->m_WarpArray)
	{
		if (pWarp == nullptr)
			continue;

		if ((pWarp->sWarpID / 10) != warp_group)
			continue;

		SetShort(buff, pWarp->sWarpID, send_index);

		SetString2(buff, pWarp->strWarpName, static_cast<short>(strlen(pWarp->strWarpName)), send_index);
		SetString2(buff, pWarp->strAnnounce, static_cast<short>(strlen(pWarp->strAnnounce)), send_index);
		SetShort(buff, pWarp->sZone, send_index);

		C3DMap* pTargetMap = m_pMain->GetMapByID(pWarp->sZone);
		if (pTargetMap != nullptr)
			SetShort(buff, pTargetMap->m_sMaxUser, send_index);
		else
			SetShort(buff, 0, send_index);

		SetDWORD(buff, pWarp->dwPay, send_index);
		SetShort(buff, (short) (pWarp->fX * 10), send_index);
		SetShort(buff, (short) (pWarp->fZ * 10), send_index);
		SetShort(buff, (short) (pWarp->fY * 10), send_index);
		count++;
	}

	SetByte(send_buff, WIZ_WARP_LIST, temp_index);
// 비러머글 마을 이름 표시 >.<
	SetByte(send_buff, type, temp_index);
//
	SetShort(send_buff, count, temp_index);
	SetString(send_buff, buff, send_index, temp_index);
	Send(send_buff, temp_index);

	return true;
}

void CUser::InitType3()
{
	// This is for the duration spells Type 3.
	for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
	{
		m_fHPStartTime[i] = 0.0f;
		m_fHPLastTime[i] = 0.0f;
		m_bHPAmount[i] = 0;
		m_bHPDuration[i] = 0;
		m_bHPInterval[i] = 5;
		m_sSourceID[i] = -1;
	}

	m_bType3Flag = false;
}

bool CUser::BindObjectEvent(short objectindex, short nid)
{
	int send_index = 0, result = 0;
	char send_buff[128] = {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	if (pEvent->sBelong != 0
		&& pEvent->sBelong != m_pUserData->m_bNation)
	{
		result = 0;
	}
	else
	{
		m_pUserData->m_sBind = pEvent->sIndex;
		result = 1;
	}

	SetByte(send_buff, WIZ_OBJECT_EVENT, send_index);
	SetByte(send_buff, static_cast<uint8_t>(pEvent->sType), send_index);
	SetByte(send_buff, result, send_index);
	Send(send_buff, send_index);

	return true;
}

bool CUser::GateObjectEvent(short objectindex, short nid)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return false;

	int send_index = 0, result = 0;
	char send_buff[128] = {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	CNpc* pNpc = m_pMain->m_NpcMap.GetData(nid);
	if (pNpc == nullptr)
		return false;

	if (pNpc->m_tNpcType == NPC_GATE
		|| pNpc->m_tNpcType == NPC_PHOENIX_GATE
		|| pNpc->m_tNpcType == NPC_SPECIAL_GATE)
	{
		// NOTE: This can have other states; they should ideally be defined.
		pNpc->m_byGateOpen = pNpc->m_byGateOpen == 0 ? true : false;
		result = 1;

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_NPC_GATE_OPEN, send_index);
		SetShort(send_buff, nid, send_index);
		SetByte(send_buff, pNpc->m_byGateOpen, send_index);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
	}
	else
	{
		result = 0;
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_OBJECT_EVENT, send_index);
	SetByte(send_buff, static_cast<uint8_t>(pEvent->sType), send_index);
	SetByte(send_buff, result, send_index);
	SetShort(send_buff, nid, send_index);
	SetByte(send_buff, pNpc->m_byGateOpen, send_index);
	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	return true;
}

bool CUser::GateLeverObjectEvent(short objectindex, short nid)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return false;

	int send_index = 0, result = 0;
	char send_buff[128] = {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	CNpc* pNpc = m_pMain->m_NpcMap.GetData(nid);
	if (pNpc == nullptr)
		return false;

	_OBJECT_EVENT* pGateEvent = pMap->GetObjectEvent(pEvent->sControlNpcID);
	if (pGateEvent == nullptr)
		return false;

	CNpc* pGateNpc = m_pMain->GetNpcPtr(pEvent->sControlNpcID, m_pUserData->m_bZone);
	if (pGateNpc == nullptr)
	{
		result = 0;
	}
	else
	{
		if (pGateNpc->m_tNpcType == NPC_GATE
			|| pGateNpc->m_tNpcType == NPC_PHOENIX_GATE
			|| pGateNpc->m_tNpcType == NPC_SPECIAL_GATE)
		{
			if (pNpc->m_byGroup != m_pUserData->m_bNation
				&& pNpc->m_byGroup != 0)
			{
				if (pNpc->m_byGateOpen == 0)
					return false;
			}

			// NOTE: This can have other states; ideally they should be defined.
			pNpc->m_byGateOpen = pNpc->m_byGateOpen == 0 ? true : false;
			result = 1;
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, AG_NPC_GATE_OPEN, send_index);
			SetShort(send_buff, nid, send_index);
			SetByte(send_buff, pNpc->m_byGateOpen, send_index);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);

			pGateNpc->m_byGateOpen = !pGateNpc->m_byGateOpen;
			result = 1;
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, AG_NPC_GATE_OPEN, send_index);
			SetShort(send_buff, pGateNpc->m_sNid, send_index);
			SetByte(send_buff, pGateNpc->m_byGateOpen, send_index);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_OBJECT_EVENT, send_index);
			SetByte(send_buff, static_cast<uint8_t>(pGateEvent->sType), send_index);
			SetByte(send_buff, result, send_index);
			SetShort(send_buff, pGateNpc->m_sNid, send_index);
			SetByte(send_buff, pGateNpc->m_byGateOpen, send_index);
			m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
		}
		else
		{
			result = 0;
		}
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_OBJECT_EVENT, send_index);
	SetByte(send_buff, static_cast<uint8_t>(pEvent->sType), send_index);
	SetByte(send_buff, result, send_index);
	SetShort(send_buff, nid, send_index);
	SetByte(send_buff, pNpc->m_byGateOpen, send_index);
	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	return true;
}

bool CUser::FlagObjectEvent(short objectindex, short nid)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return false;

	int send_index = 0, result = 0;
	char send_buff[128] = {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	CNpc* pNpc = m_pMain->m_NpcMap.GetData(nid);
	if (pNpc == nullptr)
		return false;

	_OBJECT_EVENT* pFlagEvent = pMap->GetObjectEvent(pEvent->sControlNpcID);
	if (pFlagEvent == nullptr)
		return false;

	CNpc* pFlagNpc = m_pMain->GetNpcPtr(pEvent->sControlNpcID, m_pUserData->m_bZone);
	if (pFlagNpc == nullptr)
	{
		result = 0;
	}
	else
	{
		if (pFlagNpc->m_tNpcType == NPC_GATE
			|| pFlagNpc->m_tNpcType == NPC_PHOENIX_GATE
			|| pFlagNpc->m_tNpcType == NPC_SPECIAL_GATE)
		{
			if (m_pMain->m_bVictory > 0)
				return false;

			if (pNpc->m_byGateOpen == 0)
				return false;

			// if (pNpc->m_byGroup != 0
			//	&& pFlagNpc->m_byGroup != 0)
			//	goto fail_return;

			result = 1;

			// pNpc->m_byGroup = m_pUserData->m_bNation;		
			// pNpc->m_byGateOpen = !pNpc->m_byGateOpen;	
			pNpc->m_byGateOpen = 0;	// LEVER !!!

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, AG_NPC_GATE_OPEN, send_index);
			SetShort(send_buff, nid, send_index);
			SetByte(send_buff, pNpc->m_byGateOpen, send_index);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;

			// pFlagNpc->m_byGroup = m_pUserData->m_bNation;		
			// pFlagNpc->m_byGateOpen = !pFlagNpc->m_byGateOpen;	// FLAG !!!
			pFlagNpc->m_byGateOpen = 0;

			SetByte(send_buff, AG_NPC_GATE_OPEN, send_index);		// (Send to AI....)
			SetShort(send_buff, pFlagNpc->m_sNid, send_index);
			SetByte(send_buff, pFlagNpc->m_byGateOpen, send_index);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;

			SetByte(send_buff, WIZ_OBJECT_EVENT, send_index);		// (Send to Region...)
			SetByte(send_buff, static_cast<uint8_t>(pFlagEvent->sType), send_index);
			SetByte(send_buff, result, send_index);
			SetShort(send_buff, pFlagNpc->m_sNid, send_index);
			SetByte(send_buff, pFlagNpc->m_byGateOpen, send_index);
			m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

			// ADD FLAG SCORE !!!
			if (m_pUserData->m_bNation == KARUS)
				++m_pMain->m_bKarusFlag;
			else if (m_pUserData->m_bNation == ELMORAD)
				++m_pMain->m_bElmoradFlag;

			// Did one of the teams win?
			m_pMain->BattleZoneVictoryCheck();
		}
		else
		{
			result = 0;
		}
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_OBJECT_EVENT, send_index);
	SetByte(send_buff, static_cast<uint8_t>(pEvent->sType), send_index);
	SetByte(send_buff, result, send_index);
	SetShort(send_buff, nid, send_index);
	SetByte(send_buff, pNpc->m_byGateOpen, send_index);
	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	return true;
}

bool CUser::WarpListObjectEvent(short objectindex, short nid)
{
	int send_index = 0, result = 0;
	char send_buff[128] = {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	// We cannot use warp gates belonging to another nation.
	if (pEvent->sBelong != 0
		&& pEvent->sBelong != m_pUserData->m_bNation)
		return false;

	// We cannot use warp gates when invading.
	if (m_pUserData->m_bNation != m_pUserData->m_bZone
		&& m_pUserData->m_bZone <= ZONE_ELMORAD)
		return false;

	if (!GetWarpList(pEvent->sControlNpcID))
		return false;

	return true;
}

void CUser::ObjectEvent(char* pBuf)
{
	int index = 0, objectindex = 0, send_index = 0, result = 0, nid = 0;
	char send_buff[128] = {};
	uint8_t objectType = 0;

	C3DMap* pMap = nullptr;
	_OBJECT_EVENT* pEvent = nullptr;

	objectindex = GetShort(pBuf, index);
	nid = GetShort(pBuf, index);

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		goto fail_return;

	pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		goto fail_return;

	objectType = static_cast<uint8_t>(pEvent->sType);

	switch (pEvent->sType)
	{
		// Bind Point
		case 0:

		// Destory Bind Point
		case 7:
			if (!BindObjectEvent(objectindex, nid))
				goto fail_return;
			break;

		// Gate Object : 사용치 않음 : 2002.12.23
		case 1:
		case 2:
			//if (!GateObjectEvent(objectindex, nid))
			//	goto fail_return; 
			break;

		// Gate lever Object
		case 3:
			if (!GateLeverObjectEvent(objectindex, nid))
				goto fail_return;
			break;

		// Flag Lever Object
		case 4:
			if (!FlagObjectEvent(objectindex, nid))
				goto fail_return;
			break;

		// Warp List
		case 5:
			if (!WarpListObjectEvent(objectindex, nid))
				goto fail_return;
			break;
	}
	return;

fail_return:
	SetByte(send_buff, WIZ_OBJECT_EVENT, send_index);
	SetByte(send_buff, objectType, send_index);
	SetByte(send_buff, 0, send_index);
	Send(send_buff, send_index);
}

#if 0 // outdated
void CUser::Friend(char* pBuf)
{
	int index = 0;
	BYTE subcommand = GetByte(pBuf, index);

	switch (subcommand)
	{
		case FRIEND_REQUEST:
			FriendRequest(pBuf + index);
			break;

		case FRIEND_ACCEPT:
			FriendAccept(pBuf + index);
			break;

		case FRIEND_REPORT:
			FriendReport(pBuf + index);
			break;
	}
}

void CUser::FriendRequest(char* pBuf)
{
	int index = 0, destid = -1, send_index = 0;

	CUser* pUser = nullptr;
	char send_buff[256] = {};

	destid = GetShort(pBuf, index);
	if (destid < 0
		|| destid >= MAX_USER)
		goto fail_return;

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[destid];

	if (pUser == nullptr)
		goto fail_return;

	if (pUser->m_sFriendUser != -1)
		goto fail_return;

	if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
		goto fail_return;

	m_sFriendUser = destid;
	pUser->m_sFriendUser = m_Sid;

	SetByte(send_buff, WIZ_FRIEND_REPORT, send_index);
	SetByte(send_buff, FRIEND_REQUEST, send_index);
	SetShort(send_buff, m_Sid, send_index);
	pUser->Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_FRIEND_REPORT, send_index);
	SetByte(send_buff, FRIEND_CANCEL, send_index);
	Send(send_buff, send_index);
}

void CUser::FriendAccept(char* pBuf)
{
	int index = 0, destid = -1, send_index = 0;
	CUser* pUser = nullptr;
	char send_buff[256] = {};

	BYTE result = GetByte(pBuf, index);

	if (m_sFriendUser < 0
		|| m_sFriendUser >= MAX_USER)
	{
		m_sFriendUser = -1;
		return;
	}

	pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_sFriendUser];

	if (pUser == nullptr)
	{
		m_sFriendUser = -1;
		return;
	}

	m_sFriendUser = -1;
	pUser->m_sFriendUser = -1;

	SetByte(send_buff, WIZ_FRIEND_REPORT, send_index);
	SetByte(send_buff, FRIEND_ACCEPT, send_index);
	SetByte(send_buff, result, send_index);
	pUser->Send(send_buff, send_index);
}
#endif

void CUser::Corpse()
{
	int send_index = 0;
	char send_buff[256] = {};

	SetByte(send_buff, WIZ_CORPSE, send_index);
	SetShort(send_buff, m_Sid, send_index);
	m_pMain->Send_Region(send_buff, send_index, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
}

void CUser::PartyBBS(char* pBuf)
{
	int index = 0;
	BYTE subcommand = GetByte(pBuf, index);
	switch (subcommand)
	{
		// When you register a message on the Party BBS.
		case PARTY_BBS_REGISTER:
			PartyBBSRegister(pBuf + index);
			break;

		// When you delete your message on the Party BBS.
		case PARTY_BBS_DELETE:
			PartyBBSDelete(pBuf + index);
			break;

		// Get the 'needed' messages from the Party BBS.
		case PARTY_BBS_NEEDED:
			PartyBBSNeeded(pBuf + index, PARTY_BBS_NEEDED);
			break;
	}
}

void CUser::PartyBBSRegister(char* pBuf)
{
	CUser* pUser = nullptr;
	int index = 0, send_index = 0;	// Basic Initializations. 			
	BYTE result = 0;
	short bbs_len = 0;
	char send_buff[256] = {};
	int i = 0, counter = 0;

	// You are already in a party!
	if (m_sPartyIndex != -1)
		goto fail_return;

	// You are already on the BBS!
	if (m_bNeedParty == 2)
		goto fail_return;

	// Success! Now you officially need a party!!!
	m_bNeedParty = 2;
	result = 1;

	// Send new 'Need Party Status' to region!!!
	SetByte(send_buff, 2, send_index);
	SetByte(send_buff, m_bNeedParty, send_index);
	StateChange(send_buff);

	// Now, let's find out which page the user is on.
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	for (i = 0; i < MAX_USER; i++)
	{
		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];

		// Protection codes.
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
			continue;

		if (pUser->m_bNeedParty == 1)
			continue;

		if (!((pUser->m_pUserData->m_bLevel <= (int) (m_pUserData->m_bLevel * 1.5) && pUser->m_pUserData->m_bLevel >= (int) (m_pUserData->m_bLevel * 1.5))
			|| (pUser->m_pUserData->m_bLevel <= (m_pUserData->m_bLevel + 8) && pUser->m_pUserData->m_bLevel >= ((int) (m_pUserData->m_bLevel) - 8))))
			continue;

		if (pUser->m_Sid == m_Sid)
			break;

		++counter;
	}

	SetShort(send_buff, counter / MAX_BBS_PAGE, send_index);
	PartyBBSNeeded(send_buff, PARTY_BBS_REGISTER);
	return;

fail_return:
	SetByte(send_buff, WIZ_PARTY_BBS, send_index);
	SetByte(send_buff, PARTY_BBS_REGISTER, send_index);
	SetByte(send_buff, result, send_index);
	Send(send_buff, send_index);
}

void CUser::PartyBBSDelete(char* pBuf)
{
	int send_index = 0;	// Basic Initializations. 			
	BYTE result = 0;
	char send_buff[256] = {};

	// You don't need anymore 
	if (m_bNeedParty == 1)
		goto fail_return;

	// Success! You no longer need a party !!!
	m_bNeedParty = 1;
	result = 1;

	// Send new 'Need Party Status' to region!!!
	SetByte(send_buff, 2, send_index);
	SetByte(send_buff, m_bNeedParty, send_index);
	StateChange(send_buff);

	// Now, let's find out which page the user is on.
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetShort(send_buff, 0, send_index);
	PartyBBSNeeded(send_buff, PARTY_BBS_DELETE);
	return;

fail_return:
	SetByte(send_buff, WIZ_PARTY_BBS, send_index);
	SetByte(send_buff, PARTY_BBS_DELETE, send_index);
	SetByte(send_buff, result, send_index);
	Send(send_buff, send_index);
}

void CUser::PartyBBSNeeded(char* pBuf, BYTE type)
{
	CUser* pUser = nullptr;	// Basic Initializations. 	
	int index = 0, send_index = 0, i = 0, j = 0;
	short page_index = 0, start_counter = 0, bbs_len = 0, BBS_Counter = 0;
	BYTE result = 0, valid_counter = 0;
	char send_buff[256] = {};

	page_index = GetShort(pBuf, index);
	start_counter = page_index * MAX_BBS_PAGE;

	if (start_counter < 0)
		goto fail_return;

	if (start_counter > MAX_USER)
		goto fail_return;

	result = 1;

	SetByte(send_buff, WIZ_PARTY_BBS, send_index);
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, result, send_index);

	for (i = 0; i < MAX_USER; i++)
	{
		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];

		// Protection codes.
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
			continue;

		if (pUser->m_bNeedParty == 1)
			continue;

		if (!((pUser->m_pUserData->m_bLevel <= (int) (m_pUserData->m_bLevel * 1.5) && pUser->m_pUserData->m_bLevel >= (int) (m_pUserData->m_bLevel * 1.5))
			|| (pUser->m_pUserData->m_bLevel <= (m_pUserData->m_bLevel + 8) && pUser->m_pUserData->m_bLevel >= ((int) (m_pUserData->m_bLevel) - 8))))
			continue;

		BBS_Counter++;

		// Range check codes.
		if (i < start_counter)
			continue;

		if (valid_counter >= MAX_BBS_PAGE)
			continue;

		// Create packet.
		SetShort(send_buff, strlen(pUser->m_pUserData->m_id), send_index);
		SetString(send_buff, pUser->m_pUserData->m_id, strlen(pUser->m_pUserData->m_id), send_index);
		SetByte(send_buff, pUser->m_pUserData->m_bLevel, send_index);
		SetShort(send_buff, pUser->m_pUserData->m_sClass, send_index);

		// Increment counters.
		++valid_counter;
//		++BBS_Counter;		
	}

	// You still need to fill up ten slots.
	if (valid_counter < MAX_BBS_PAGE)
	{
//		for (j = 0; j < MAX_BBS_PAGE; j++)
		for (j = valid_counter; j < MAX_BBS_PAGE; j++)
		{
			SetShort(send_buff, 0, send_index);
			SetByte(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
		}
	}

	SetShort(send_buff, page_index, send_index);
	SetShort(send_buff, BBS_Counter, send_index);
	Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_PARTY_BBS, send_index);
	SetByte(send_buff, PARTY_BBS_NEEDED, send_index);
	SetByte(send_buff, result, send_index);
	Send(send_buff, send_index);
}

void CUser::MarketBBS(char* pBuf)
{
	int index = 0;
	BYTE subcommand = GetByte(pBuf, index);

	MarketBBSBuyPostFilter();		// Get rid of empty slots.
	MarketBBSSellPostFilter();

	switch (subcommand)
	{
		// When you register a message on the Market BBS.
		case MARKET_BBS_REGISTER:
			MarketBBSRegister(pBuf + index);
			break;

		// When you delete your message on the Market BBS.
		case MARKET_BBS_DELETE:
			MarketBBSDelete(pBuf + index);
			break;

		// Get the 'needed' messages from the Market BBS.
		case MARKET_BBS_REPORT:
			MarketBBSReport(pBuf + index, MARKET_BBS_REPORT);
			break;

		// When you first open the Market BBS.
		case MARKET_BBS_OPEN:
			MarketBBSReport(pBuf + index, MARKET_BBS_OPEN);
			break;

		// When you agree to spend money on remote bartering.
		case MARKET_BBS_REMOTE_PURCHASE:
			MarketBBSRemotePurchase(pBuf + index);
			break;

		// USE ONLY IN EMERGENCY!!!
		case MARKET_BBS_MESSAGE:
			MarketBBSMessage(pBuf + index);
			break;
	}
}

void CUser::MarketBBSRegister(char* pBuf)
{
	CUser* pUser = nullptr;	// Basic Initializations.
	int index = 0, send_index = 0, i = 0, j = 0, page_index = 0;
	short title_len = 0, message_len = 0;
	BYTE result = 0, sub_result = 1, buysell_index = 0;
	char send_buff[256] = {};

	buysell_index = GetByte(pBuf, index);	// Buy or sell?

	if (buysell_index == MARKET_BBS_BUY)
	{
		if (m_pUserData->m_iGold < BUY_POST_PRICE)
		{
			sub_result = 2;
			goto fail_return;
		}
	}
	else if (buysell_index == MARKET_BBS_SELL)
	{
		if (m_pUserData->m_iGold < SELL_POST_PRICE)
		{
			sub_result = 2;
			goto fail_return;
		}
	}

	for (i = 0; i < MAX_BBS_POST; i++)
	{
		// Buy
		if (buysell_index == MARKET_BBS_BUY)
		{
			if (m_pMain->m_sBuyID[i] == -1)
			{
				m_pMain->m_sBuyID[i] = m_Sid;

				title_len = GetShort(pBuf, index);
				GetString(m_pMain->m_strBuyTitle[i], pBuf, title_len, index);

				message_len = GetShort(pBuf, index);
				GetString(m_pMain->m_strBuyMessage[i], pBuf, message_len, index);

				m_pMain->m_iBuyPrice[i] = GetDWORD(pBuf, index);
				m_pMain->m_fBuyStartTime[i] = TimeGet();

				result = 1;
				break;
			}
		}
		// Sell
		else if (buysell_index == MARKET_BBS_SELL)
		{
			if (m_pMain->m_sSellID[i] == -1)
			{
				m_pMain->m_sSellID[i] = m_Sid;

				title_len = GetShort(pBuf, index);
				GetString(m_pMain->m_strSellTitle[i], pBuf, title_len, index);

				message_len = GetShort(pBuf, index);
				GetString(m_pMain->m_strSellMessage[i], pBuf, message_len, index);

				m_pMain->m_iSellPrice[i] = GetDWORD(pBuf, index);
				m_pMain->m_fSellStartTime[i] = TimeGet();

				result = 1;
				break;
			}
		}
		// Error
		else
		{
			goto fail_return;
		}
	}

	// No spaces available
	if (result == 0)
		goto fail_return;

	SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
	SetByte(send_buff, GOLD_CHANGE_LOSE, send_index);

	if (buysell_index == MARKET_BBS_BUY)
	{
		m_pUserData->m_iGold -= BUY_POST_PRICE;
		SetDWORD(send_buff, BUY_POST_PRICE, send_index);
	}
	else if (buysell_index == MARKET_BBS_SELL)
	{
		m_pUserData->m_iGold -= SELL_POST_PRICE;
		SetDWORD(send_buff, SELL_POST_PRICE, send_index);
	}

	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	Send(send_buff, send_index);

	page_index = i / MAX_BBS_PAGE;
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, buysell_index, send_index);
	SetShort(send_buff, page_index, send_index);
	MarketBBSReport(send_buff, MARKET_BBS_REGISTER);
	return;

fail_return:
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, WIZ_MARKET_BBS, send_index);
	SetByte(send_buff, MARKET_BBS_REGISTER, send_index);
	SetByte(send_buff, buysell_index, send_index);
	SetByte(send_buff, result, send_index);
	SetByte(send_buff, sub_result, send_index);
	Send(send_buff, send_index);
}

void CUser::MarketBBSDelete(char* pBuf)
{
	CUser* pUser = nullptr;	// Basic Initializations. 	
	int index = 0, send_index = 0, i = 0, j = 0;
	short delete_id = 0;
	BYTE result = 0, sub_result = 1, buysell_index = 0;
	char send_buff[256] = {};

	buysell_index = GetByte(pBuf, index);	// Buy or sell?
	delete_id = GetShort(pBuf, index);		// Which message should I delete? 

	if (delete_id < 0
		|| delete_id >= MAX_BBS_POST)
		goto fail_return;

	// Buy 
	if (buysell_index == MARKET_BBS_BUY)
	{
		if (m_pMain->m_sBuyID[delete_id] != m_Sid
			&& m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
			goto fail_return;

		MarketBBSBuyDelete(delete_id);
		result = 1;
	}
	// Sell
	else if (buysell_index == MARKET_BBS_SELL)
	{
		if (m_pMain->m_sSellID[delete_id] != m_Sid
			&& m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
			goto fail_return;

		MarketBBSSellDelete(delete_id);
		result = 1;	
	}
	// Error
	else
	{
		goto fail_return;
	}

	SetShort(send_buff, buysell_index, send_index);
	SetShort(send_buff, 0, send_index);
	MarketBBSReport(send_buff, MARKET_BBS_DELETE);
	return;

fail_return:
	SetByte(send_buff, WIZ_MARKET_BBS, send_index);
	SetByte(send_buff, MARKET_BBS_DELETE, send_index);
	SetByte(send_buff, buysell_index, send_index);
	SetByte(send_buff, result, send_index);
	SetByte(send_buff, sub_result, send_index);
	Send(send_buff, send_index);
}

void CUser::MarketBBSReport(char* pBuf, BYTE type)
{
	CUser* pUser = nullptr;	// Basic Initializations. 	
	int index = 0, send_index = 0, i = 0, j = 0;
	short bbs_len = 0, page_index = 0, start_counter = 0, valid_counter = 0, BBS_Counter = 0,
		title_length = 0, message_length = 0;
	BYTE result = 0, sub_result = 1, buysell_index = 0;
	char send_buff[8192] = {};

	buysell_index = GetByte(pBuf, index);	// Buy or sell?
	page_index = GetShort(pBuf, index);		// Which message should I delete? 

	start_counter = page_index * MAX_BBS_PAGE;

	if (type == MARKET_BBS_OPEN)
	{
		start_counter = 0;
		page_index = 0;
	}

	if (start_counter < 0)
		goto fail_return;

	if (start_counter > MAX_BBS_POST)
		goto fail_return;

	result = 1;

	SetByte(send_buff, WIZ_MARKET_BBS, send_index);
	SetByte(send_buff, type, send_index);
	SetByte(send_buff, buysell_index, send_index);
	SetByte(send_buff, result, send_index);

	for (i = 0; i < MAX_BBS_POST; i++)
	{
		if (buysell_index == MARKET_BBS_BUY)
		{
			if (m_pMain->m_sBuyID[i] == -1)
				continue;

			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_pMain->m_sBuyID[i]];

			// Delete info!!!
			if (pUser == nullptr)
			{
				MarketBBSBuyDelete(i);
				continue;
			}

			// Increment number of total messages.
			++BBS_Counter;

			// Range check codes.
			if (i < start_counter)
				continue;

			if (valid_counter >= MAX_BBS_PAGE)
				continue;

			SetShort(send_buff, m_pMain->m_sBuyID[i], send_index);

			SetShort(send_buff, strlen(pUser->m_pUserData->m_id), send_index);
			SetString(send_buff, pUser->m_pUserData->m_id, strlen(pUser->m_pUserData->m_id), send_index);

			title_length = static_cast<short>(strlen(m_pMain->m_strBuyTitle[i]));
			if (title_length > MAX_BBS_TITLE)
				title_length = MAX_BBS_TITLE;

			SetString2(send_buff, m_pMain->m_strBuyTitle[i], title_length, send_index);
//			SetShort(send_buff, strlen(m_pMain->m_strBuyTitle[i]), send_index);
//			SetString(send_buff, m_pMain->m_strBuyTitle[i], strlen(m_pMain->m_strBuyTitle[i]), send_index);

			message_length = static_cast<short>(strlen(m_pMain->m_strBuyMessage[i]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(send_buff, m_pMain->m_strBuyMessage[i], message_length, send_index);
//			SetShort(send_buff, strlen(m_pMain->m_strBuyMessage[i]), send_index);
//			SetString(send_buff, m_pMain->m_strBuyMessage[i], strlen(m_pMain->m_strBuyMessage[i]), send_index);

			SetDWORD(send_buff, m_pMain->m_iBuyPrice[i], send_index);
			SetShort(send_buff, i, send_index);

			++valid_counter;
		}
		else if (buysell_index == MARKET_BBS_SELL)
		{
			if (m_pMain->m_sSellID[i] == -1)
				continue;

			// Delete info!!!
			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_pMain->m_sSellID[i]];
			if (pUser == nullptr)
			{
				MarketBBSSellDelete(i);
				continue;
			}

			++BBS_Counter;

			// Range check codes.
			if (i < start_counter)
				continue;

			if (valid_counter >= MAX_BBS_PAGE)
				continue;

			SetShort(send_buff, m_pMain->m_sSellID[i], send_index);
			SetString2(send_buff, pUser->m_pUserData->m_id, send_index);

			title_length = static_cast<short>(strlen(m_pMain->m_strSellTitle[i]));
			if (title_length > MAX_BBS_TITLE)
				title_length = MAX_BBS_TITLE;

			SetString2(send_buff, m_pMain->m_strSellTitle[i], title_length, send_index);
//			SetShort(send_buff, strlen(m_pMain->m_strSellTitle[i]), send_index);
//			SetString(send_buff, m_pMain->m_strSellTitle[i], strlen(m_pMain->m_strSellTitle[i]), send_index);

			message_length = static_cast<short>(strlen(m_pMain->m_strSellMessage[i]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(send_buff, m_pMain->m_strSellMessage[i], message_length, send_index);
//			SetShort(send_buff, strlen(m_pMain->m_strSellMessage[i]), send_index);
//			SetString(send_buff, m_pMain->m_strSellMessage[i], strlen(m_pMain->m_strSellMessage[i]), send_index);

			SetDWORD(send_buff, m_pMain->m_iSellPrice[i], send_index);
			SetShort(send_buff, i, send_index);

			++valid_counter;	// Increment number of messages on the requested page			
		}
	}

	if (valid_counter == 0
		&& page_index > 0)
		goto fail_return1;

	// You still need to fill up slots.
	if (valid_counter < MAX_BBS_PAGE)
	{
		for (j = valid_counter; j < MAX_BBS_PAGE; j++)
		{
			SetShort(send_buff, -1, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetShort(send_buff, 0, send_index);
			SetDWORD(send_buff, 0, send_index);
			SetShort(send_buff, -1, send_index);

			++valid_counter;
		}
	}

	SetShort(send_buff, page_index, send_index);
	SetShort(send_buff, BBS_Counter, send_index);
	Send(send_buff, send_index);
	return;

fail_return:
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, WIZ_MARKET_BBS, send_index);
	SetByte(send_buff, MARKET_BBS_REPORT, send_index);
	SetByte(send_buff, buysell_index, send_index);
	SetByte(send_buff, result, send_index);
	SetByte(send_buff, sub_result, send_index);
	Send(send_buff, send_index);
	return;

fail_return1:
	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetShort(send_buff, buysell_index, send_index);
	SetShort(send_buff, page_index - 1, send_index);
	MarketBBSReport(send_buff, type);
}

void CUser::MarketBBSRemotePurchase(char* pBuf)
{
	CUser* pUser = nullptr;
	int send_index = 0, index = 0, i = 0;
	short message_index = -1, tid = -1;
	BYTE result = 0, sub_result = 1, buysell_index = 0;

	char send_buff[256] = {};

	buysell_index = GetByte(pBuf, index);		// Buy or sell?
	message_index = GetShort(pBuf, index);		// Which message should I retrieve? 

	if (buysell_index != MARKET_BBS_BUY
		&& buysell_index != MARKET_BBS_SELL)
		goto fail_return;

	if (buysell_index == MARKET_BBS_BUY)
	{
		if (m_pMain->m_sBuyID[message_index] == -1)
		{
			sub_result = 3;
			goto fail_return;
		}

		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_pMain->m_sBuyID[message_index]];

		// Something wrong with the target ID.
		if (pUser == nullptr)
		{
			sub_result = 1;
			goto fail_return;
		}
	}
	else if (buysell_index == MARKET_BBS_SELL)
	{
		if (m_pMain->m_sSellID[message_index] == -1)
		{
			sub_result = 3;
			goto fail_return;
		}

		pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_pMain->m_sSellID[message_index]];

		// Something wrong with the target ID.
		if (pUser == nullptr)
		{
			sub_result = 1;
			goto fail_return;
		}
	}

	// Check if user has gold.
	if (m_pUserData->m_iGold >= REMOTE_PURCHASE_PRICE)
	{
		m_pUserData->m_iGold -= REMOTE_PURCHASE_PRICE;

		SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
		SetByte(send_buff, GOLD_CHANGE_LOSE, send_index);
		SetDWORD(send_buff, REMOTE_PURCHASE_PRICE, send_index);
		SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
		Send(send_buff, send_index);

		result = 1;
	}
	// User does not have gold.
	else
	{
		sub_result = 2;
	}

fail_return:
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_MARKET_BBS, send_index);
	SetByte(send_buff, MARKET_BBS_REMOTE_PURCHASE, send_index);
	SetByte(send_buff, buysell_index, send_index);
	SetByte(send_buff, result, send_index);

	// Only on errors!!!
	if (result == 0)
		SetByte(send_buff, sub_result, send_index);

	Send(send_buff, send_index);
}

void CUser::MarketBBSTimeCheck()
{
	CUser* pUser = nullptr;	// Basic Initializations. 	
	int send_index = 0, price = 0;
	char send_buff[256] = {};
	float currenttime = TimeGet();

	for (int i = 0; i < MAX_BBS_POST; i++)
	{
		// BUY!!!
		if (m_pMain->m_sBuyID[i] != -1)
		{
			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_pMain->m_sBuyID[i]];
			if (pUser == nullptr)
			{
				MarketBBSBuyDelete(i);
				continue;
			}

			if (m_pMain->m_fBuyStartTime[i] + BBS_CHECK_TIME < currenttime)
			{
				if (pUser->m_pUserData->m_iGold >= BUY_POST_PRICE)
				{
					pUser->m_pUserData->m_iGold -= BUY_POST_PRICE;
					m_pMain->m_fBuyStartTime[i] = TimeGet();

					// Now the target
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
					SetByte(send_buff, GOLD_CHANGE_LOSE, send_index);
					SetDWORD(send_buff, BUY_POST_PRICE, send_index);
					SetDWORD(send_buff, pUser->m_pUserData->m_iGold, send_index);
					pUser->Send(send_buff, send_index);
				}
				else
				{
					MarketBBSBuyDelete(i);
				}
			}
		}

		// SELL!!!
		if (m_pMain->m_sSellID[i] != -1)
		{
			pUser = (CUser*) m_pMain->m_Iocport.m_SockArray[m_pMain->m_sSellID[i]];
			if (pUser == nullptr)
			{
				MarketBBSSellDelete(i);
				continue;
			}

			if (m_pMain->m_fSellStartTime[i] + BBS_CHECK_TIME < currenttime)
			{
				if (pUser->m_pUserData->m_iGold >= SELL_POST_PRICE)
				{
					pUser->m_pUserData->m_iGold -= SELL_POST_PRICE;
					m_pMain->m_fSellStartTime[i] = TimeGet();

					// Now the target
					memset(send_buff, 0, sizeof(send_buff));
					send_index = 0;
					SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
					SetByte(send_buff, GOLD_CHANGE_LOSE, send_index);
					SetDWORD(send_buff, SELL_POST_PRICE, send_index);
					SetDWORD(send_buff, pUser->m_pUserData->m_iGold, send_index);
					pUser->Send(send_buff, send_index);
				}
				else
				{
					MarketBBSSellDelete(i);
				}
			}
		}
	}
}

void CUser::MarketBBSUserDelete()
{
	for (int i = 0; i < MAX_BBS_POST; i++)
	{
		// BUY!!!
		if (m_pMain->m_sBuyID[i] == m_Sid)
			MarketBBSBuyDelete(i);

		// SELL!!
		if (m_pMain->m_sSellID[i] == m_Sid)
			MarketBBSSellDelete(i);
	}
}

void CUser::MarketBBSBuyDelete(short index)
{
	m_pMain->m_sBuyID[index] = -1;
	memset(m_pMain->m_strBuyTitle[index], 0, sizeof(m_pMain->m_strBuyTitle[index]));
	memset(m_pMain->m_strBuyMessage[index], 0, sizeof(m_pMain->m_strBuyMessage[index]));
	m_pMain->m_iBuyPrice[index] = 0;
	m_pMain->m_fBuyStartTime[index] = 0.0f;
}

void CUser::MarketBBSSellDelete(short index)
{
	m_pMain->m_sSellID[index] = -1;
	memset(m_pMain->m_strSellTitle[index], 0, sizeof(m_pMain->m_strSellTitle[index]));
	memset(m_pMain->m_strSellMessage[index], 0, sizeof(m_pMain->m_strSellMessage[index]));
	m_pMain->m_iSellPrice[index] = 0;
	m_pMain->m_fSellStartTime[index] = 0.0f;
}

void CUser::MarketBBSMessage(char* pBuf)
{
	int index = 0, send_index = 0;
	short message_index = 0, message_length = 0;
	BYTE result = 0, sub_result = 1, buysell_index = 0;
	char send_buff[256] = {};

	buysell_index = GetByte(pBuf, index);		// Buy or sell?
	message_index = GetShort(pBuf, index);		// Which message should I retrieve? 

	if (buysell_index != MARKET_BBS_BUY
		&& buysell_index != MARKET_BBS_SELL)
		goto fail_return;

	SetByte(send_buff, WIZ_MARKET_BBS, send_index);
	SetByte(send_buff, MARKET_BBS_MESSAGE, send_index);
	SetByte(send_buff, result, send_index);

	switch (buysell_index)
	{
		case MARKET_BBS_BUY:
			if (m_pMain->m_sBuyID[message_index] == -1)
				goto fail_return;

			message_length = static_cast<short>(strlen(m_pMain->m_strBuyMessage[message_index]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(send_buff, m_pMain->m_strBuyMessage[message_index], message_length, send_index);
			break;

		case MARKET_BBS_SELL:
			if (m_pMain->m_sSellID[message_index] == -1)
				goto fail_return;

			message_length = static_cast<short>(strlen(m_pMain->m_strSellMessage[message_index]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(send_buff, m_pMain->m_strSellMessage[message_index], message_length, send_index);
			break;
	}

	Send(send_buff, send_index);
	return;

fail_return:
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_MARKET_BBS, send_index);
	SetByte(send_buff, MARKET_BBS_MESSAGE, send_index);
	SetByte(send_buff, result, send_index);
	SetByte(send_buff, sub_result, send_index);
	Send(send_buff, send_index);
}

void CUser::MarketBBSBuyPostFilter()
{
	int empty_counter = 0;

	for (int i = 0; i < MAX_BBS_POST; i++)
	{
		// BUY!!!
		if (m_pMain->m_sBuyID[i] == -1)
		{
			empty_counter++;
			continue;
		}

		if (empty_counter > 0)
		{
			if (m_pMain->m_sBuyID[i] != -1)
			{
				m_pMain->m_sBuyID[i - empty_counter] = m_pMain->m_sBuyID[i];
				strcpy(m_pMain->m_strBuyTitle[i - empty_counter], m_pMain->m_strBuyTitle[i]);
				strcpy(m_pMain->m_strBuyMessage[i - empty_counter], m_pMain->m_strBuyMessage[i]);
				m_pMain->m_iBuyPrice[i - empty_counter] = m_pMain->m_iBuyPrice[i];
				m_pMain->m_fBuyStartTime[i - empty_counter] = m_pMain->m_fBuyStartTime[i];

				MarketBBSBuyDelete(i);
			}
		}
	}
}

void CUser::MarketBBSSellPostFilter()
{
	int empty_counter = 0;

	for (int i = 0; i < MAX_BBS_POST; i++)
	{
		// BUY!!!
		if (m_pMain->m_sSellID[i] == -1)
		{
			empty_counter++;
			continue;
		}

		if (empty_counter > 0)
		{
			if (m_pMain->m_sSellID[i] != -1)
			{
				m_pMain->m_sSellID[i - empty_counter] = m_pMain->m_sSellID[i];
				strcpy(m_pMain->m_strSellTitle[i - empty_counter], m_pMain->m_strSellTitle[i]);
				strcpy(m_pMain->m_strSellMessage[i - empty_counter], m_pMain->m_strSellMessage[i]);
				m_pMain->m_iSellPrice[i - empty_counter] = m_pMain->m_iSellPrice[i];
				m_pMain->m_fSellStartTime[i - empty_counter] = m_pMain->m_fSellStartTime[i];

				MarketBBSSellDelete(i);
			}
		}
	}
}

void CUser::BlinkTimeCheck(float currenttime)
{
	int send_index = 0;
	char send_buff[256] = {};

	if (BLINK_TIME < (currenttime - m_fBlinkStartTime))
	{
		m_fBlinkStartTime = 0.0f;

		m_bAbnormalType = ABNORMAL_NORMAL;

		if (m_bRegeneType == REGENE_MAGIC)
			HpChange(m_iMaxHp / 2);					// Refill HP.	
		else
			HpChange(m_iMaxHp);

		m_bRegeneType = REGENE_NORMAL;

		SetByte(send_buff, 3, send_index);
		SetByte(send_buff, m_bAbnormalType, send_index);
		StateChange(send_buff);

		//TRACE(_T("?? BlinkTimeCheck : name=%hs(%d), type=%d ??\n"), m_pUserData->m_id, m_Sid, m_bAbnormalType);
//
		// AI_server로 regene정보 전송...	
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_USER_REGENE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetShort(send_buff, m_pUserData->m_sHp, send_index);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
//
//
		// To AI Server....
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, AG_USER_INOUT, send_index);
		SetByte(send_buff, USER_REGENE, send_index);
		SetShort(send_buff, m_Sid, send_index);
		SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
		SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
		Setfloat(send_buff, m_pUserData->m_curx, send_index);
		Setfloat(send_buff, m_pUserData->m_curz, send_index);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, send_buff, send_index);
//
	}
}

void CUser::SetLogInInfoToDB(BYTE bInit)
{
	int index = 0, send_index = 0, retvalue = 0, addrlen = 20;
	char send_buff[256] = {}, strClientIP[20] = {};
	_ZONE_SERVERINFO* pInfo = nullptr;
	sockaddr_in addr = {};

	pInfo = m_pMain->m_ServerArray.GetData(m_pMain->m_nServerNo);
	if (pInfo == nullptr)
	{
		spdlog::error("User::SetLogInInfoToDB: invalid serverId={}, closing user",
			m_pMain->m_nServerNo);
		Close();
		return;
	}

	getpeername(m_Socket, (sockaddr*) &addr, &addrlen);
	strcpy(strClientIP, inet_ntoa(addr.sin_addr));

	SetByte(send_buff, WIZ_LOGIN_INFO, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
	SetShort(send_buff, strlen(pInfo->strServerIP), send_index);
	SetString(send_buff, pInfo->strServerIP, strlen(pInfo->strServerIP), send_index);
	SetShort(send_buff, pInfo->sPort, send_index);
	SetShort(send_buff, strlen(strClientIP), send_index);
	SetString(send_buff, strClientIP, strlen(strClientIP), send_index);
	SetByte(send_buff, bInit, send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"SetLogInInfoToDb: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
	}
}

void CUser::KickOut(char* pBuf)
{
	int idlen = 0, index = 0, send_index = 0;
	char accountid[MAX_ID_SIZE + 1] = {};
	char send_buff[256] = {};
	CUser* pUser = nullptr;

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
		return;

	GetString(accountid, pBuf, idlen, index);

	pUser = m_pMain->GetUserPtr(accountid, NameType::Account);
	if (pUser != nullptr)
	{
		pUser->UserDataSaveToAgent();
		pUser->Close();
	}
	else
	{
		SetByte(send_buff, WIZ_KICKOUT, send_index);
		SetShort(send_buff, idlen, send_index);
		SetString(send_buff, accountid, idlen, send_index);
		m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	}
}
// 여기서 부터 정애씨가 고생하면서 해주신 퀘스트 부분....
// The main function for the quest procedures!!!
// (actually, this only takes care of the first event :(  )
void CUser::ClientEvent(char* pBuf)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return;

	int index = 0;
	CNpc* pNpc = nullptr;
	int nid = 0, eventnum = 0;
	BYTE code = 0;
	EVENT* pEvent = nullptr;
	EVENT_DATA* pEventData = nullptr;
	int eventid = -1;

	nid = GetShort(pBuf, index);
	pNpc = m_pMain->m_NpcMap.GetData(nid);

	// Better to check than not to check ;)
	if (pNpc == nullptr)
		return;

	m_sEventNid = nid;     // 이건 나중에 내가 추가한 거였슴....

	// 이거 일단 주석 처리 절대 빼지마!!
	// if (pNpc->m_byEvent < 0)
	//	return;

	pEvent = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);
	if (pEvent == nullptr)
		return;

//	pEventData = pEvent->m_arEvent.GetData(pNpc->m_byEvent);

	switch (pNpc->m_tNpcType)
	{
		case NPC_SELITH:
			eventid = 30001;
			break;

		case NPC_ANVIL:
			eventid = 8030;
			break;

		case NPC_CLAN_MATCH_ADVISOR:
			eventid = 31001;
			break;

		case NPC_TELEPORT_GATE:
			eventid = m_pMain->GetEventTrigger(pNpc->m_tNpcType, pNpc->m_byTrapNumber);
			if (eventid == -1)
				return;
			break;

		case NPC_OPERATOR:
			eventid = 35201;
			break;

		case NPC_ISAAC:
			eventid = 35001;
			break;

		case NPC_KAISHAN:
		case NPC_NPC_5:
			eventid = 21001;
			break;

		case NPC_CAPTAIN:
			eventid = 15002;
			break;

		case NPC_CLAN:
		case NPC_MONK_ELMORAD:
			eventid = EVENT_LOGOS_ELMORAD;
			break;

		case NPC_CLERIC:
		case NPC_SIEGE_2:
			eventid = EVENT_POTION;
			break;

		case NPC_LADY:
		case NPC_PRIEST_IRIS:
			eventid = 20501;
			break;

		case NPC_ATHIAN:
		case NPC_MANAGER_BARREL:
			eventid = 22001;
			break;

		case NPC_ARENA:
			eventid = 15951;
			break;

		case NPC_TRAINER_KATE:
		case NPC_NPC_2:
			eventid = 20701;
			break;

		case NPC_GENERIC:
		case NPC_NPC_4:
			eventid = 20901;
			break;

		case NPC_SENTINEL_PATRICK:
		case NPC_NPC_3:
			eventid = 20801;
			break;

		case NPC_TRADER_KIM:
		case NPC_NPC_1:
			eventid = 20601;
			break;

		case NPC_MONK_KARUS:
			eventid = EVENT_LOGOS_KARUS;
			break;

		case NPC_MASTER_WARRIOR:
			eventid = 11001;
			break;

		case NPC_MASTER_ROGUE:
			eventid = 12001;
			break;

		case NPC_MASTER_MAGE:
			eventid = 13001;
			break;

		case NPC_MASTER_PRIEST:
			eventid = 14001;
			break;

		case NPC_BLACKSMITH:
			eventid = 7001;
			break;

		case NPC_COUPON:
			eventid = EVENT_COUPON;
			break;

		case NPC_HERO_STATUE_1:
		case NPC_KARUS_HERO_STATUE:
			eventid = 31101;
			break;

		case NPC_HERO_STATUE_2:
			eventid = 31131;
			break;

		case NPC_HERO_STATUE_3:
			eventid = 31161;
			break;

		case NPC_ELMORAD_HERO_STATUE:
			eventid = 31171;
			break;

		case NPC_KEY_QUEST_1:
			eventid = 15801;
			break;

		case NPC_KEY_QUEST_2:
			eventid = 15821;
			break;

		case NPC_KEY_QUEST_3:
			eventid = 15841;
			break;

		case NPC_KEY_QUEST_4:
			eventid = 15861;
			break;

		case NPC_KEY_QUEST_5:
			eventid = 15881;
			break;

		case NPC_KEY_QUEST_6:
			eventid = 15901;
			break;

		case NPC_KEY_QUEST_7:
			eventid = 15921;
			break;

		case NPC_ROBOS:
			eventid = 35480;
			break;

		case NPC_SERVER_TRANSFER:
			eventid = 35541;
			break;

		case NPC_RANKING:
			eventid = 35560;
			break;

		case NPC_LYONI:
			eventid = 35553;
			break;

		case NPC_BEGINNER_HELPER_1:
			eventid = 35563;
			break;

		case NPC_BEGINNER_HELPER_2:
			eventid = 35594;
			break;

		case NPC_BEGINNER_HELPER_3:
			eventid = 35615;
			break;

		case NPC_FT_1:
			eventid = EVENT_FT_1;
			break;

		case NPC_FT_2:
			eventid = EVENT_FT_2;
			break;

		case NPC_FT_3:
			eventid = EVENT_FT_3;
			break;

		case NPC_PREMIUM_PC:
			eventid = 35550;
			break;

		case NPC_KJWAR:
			eventid = 35624;
			break;

		case NPC_CRAFTSMAN:
			eventid = 32000;
			break;

		case NPC_COLISEUM_ARTES:
			eventid = 35640;
			break;

		case NPC_UNK_138:
			eventid = 35650;
			break;

		case NPC_LOVE_AGENT:
			eventid = 35662;
			break;

		case NPC_SPY:
			eventid = 1100;
			break;

		case NPC_ROYAL_GUARD:
			eventid = 17000;
			break;

		case NPC_ROYAL_CHEF:
			eventid = 17550;
			break;

		case NPC_ESLANT_WOMAN:
			eventid = 17590;
			break;

		case NPC_FARMER:
			eventid = 17600;
			break;

		case NPC_NAMELESS_WARRIOR:
			eventid = 17630;
			break;

		case NPC_UNK_147:
			eventid = 17100;
			break;

		case NPC_GATE_GUARD:
			eventid = 17570;
			break;

		case NPC_ROYAL_ADVISOR:
			eventid = 17520;
			break;

		case NPC_BIFROST_GATE:
			eventid = 17681;
			break;

		case NPC_SANGDUF:
			eventid = 15310;
			break;

		case NPC_UNK_152:
			eventid = 2901;
			break;

		case NPC_ADELIA:
			eventid = 35212;
			break;

		case NPC_BIFROST_MONUMENT:
			eventid = 0;
			break;
	}

	// Make sure you change this later!!!
	pEventData = pEvent->m_arEvent.GetData(eventid);
	if (pEventData == nullptr)
		return;

	// Check if all 'A's meet the requirements in Event #1
	if (!CheckEventLogic(pEventData))
		return;

	// Execute the 'E' events in Event #1
	for (EXEC* pExec : pEventData->m_arExec)
	{
		if (!RunNpcEvent(pNpc, pExec))
			return;
	}
}

// This part reads all the 'A' parts and checks if the
// requirements for the 'E' parts are met.
bool CUser::CheckEventLogic(const EVENT_DATA* pEventData)
{
	if (pEventData == nullptr)
		return false;

	bool bExact = true;

	for (LOGIC_ELSE* pLE : pEventData->m_arLogicElse)
	{
		bExact = false;

		if (pLE == nullptr)
			return false;

		switch (pLE->m_LogicElse)
		{
			case LOGIC_CHECK_UNDER_WEIGHT:
				if (pLE->m_LogicElseInt[0] + m_iItemWeight >= m_iMaxWeight)
					bExact = true;
				break;

			case LOGIC_CHECK_OVER_WEIGHT:
				if (pLE->m_LogicElseInt[0] + m_iItemWeight < m_iMaxWeight)
					bExact = true;
				break;

			case LOGIC_CHECK_SKILL_POINT:
				if (CheckSkillPoint(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1], pLE->m_LogicElseInt[2]))
					bExact = true;
				break;

			case LOGIC_CHECK_EXIST_ITEM:
				if (CheckExistItem(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_NOEXIST_ITEM:
				if (!CheckExistItem(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_CLASS:
				if (CheckClass(
					pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1], pLE->m_LogicElseInt[2],
					pLE->m_LogicElseInt[3], pLE->m_LogicElseInt[4], pLE->m_LogicElseInt[5]))
					bExact = true;
				break;

			case LOGIC_CHECK_NOCLASS:
				if (!CheckClass(
					pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1], pLE->m_LogicElseInt[2],
					pLE->m_LogicElseInt[3], pLE->m_LogicElseInt[4], pLE->m_LogicElseInt[5]))
					bExact = true;
				break;

			case LOGIC_CHECK_WEIGHT:
				if (!CheckWeight(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;
	// 비러머글 복권 >.<		
			case LOGIC_CHECK_EDITBOX:
				if (!CheckEditBox())
					bExact = true;
				break;

			case LOGIC_RAND:
				if (CheckRandom(pLE->m_LogicElseInt[0]))
					bExact = true;
				break;
	//
	// 비러머글 엑셀 >.<
			case LOGIC_CHECK_LV:
				if (m_pUserData->m_bLevel >= pLE->m_LogicElseInt[0]
					&& m_pUserData->m_bLevel <= pLE->m_LogicElseInt[1])
					bExact = true;
				break;

			case LOGIC_NOEXIST_COM_EVENT:
				if (!ExistComEvent(pLE->m_LogicElseInt[0]))
					bExact = true;
				break;

			case LOGIC_EXIST_COM_EVENT:
				if (ExistComEvent(pLE->m_LogicElseInt[0]))
					bExact = true;
				break;

			case LOGIC_HOWMUCH_ITEM:
				if (CheckItemCount(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1], pLE->m_LogicElseInt[2]))
					bExact = true;
				break;

			case LOGIC_CHECK_NOAH:
				if (m_pUserData->m_iGold >= pLE->m_LogicElseInt[0] && m_pUserData->m_iGold <= pLE->m_LogicElseInt[1])
					bExact = true;
				break;

			case LOGIC_CHECK_NATION:
				if(m_pUserData->m_bNation == pLE->m_LogicElseInt[0])
					bExact = true;
				break;

			case LOGIC_CHECK_LOYALTY:
				if (m_pUserData->m_iLoyalty >= pLE->m_LogicElseInt[0] && m_pUserData->m_iLoyalty <= pLE->m_LogicElseInt[1])
					bExact = true;
				break;

			case LOGIC_CHECK_CHIEF:
				if (m_pUserData->m_bFame == CHIEF)
					bExact = true;
				break;

			case LOGIC_CHECK_NO_CHIEF:
				if (m_pUserData->m_bFame != CHIEF)
					bExact = true;
				break;

			case LOGIC_CHECK_CLAN_GRADE:
				if (CheckClanGrade(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_EMPTY_SLOT:
				if (GetNumberOfEmptySlots() >= pLE->m_LogicElseInt[0])
					bExact = true;
				break;

			case LOGIC_CHECK_EXIST_EVENT:
				if (CheckExistEvent(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_NOEXIST_EVENT:
				if (!CheckExistEvent(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			default:
				return false;
		}

		// OR 조건일 경우 bExact가 TRUE이면 전체가 TRUE이다
		if (!pLE->m_bAnd)
		{
			if (bExact)
				return true;
		}
		// AND 조건일 경우 bExact가 FALSE이면 전체가 FALSE이다
		else
		{
			if (!bExact)
				return false;
		}
	}

	return bExact;
}

// This part executes all the 'E' lines!
bool CUser::RunNpcEvent(CNpc* pNpc, const EXEC* pExec)
{
	switch (pExec->m_Exec)
	{
		case EXEC_SAY:
			SendNpcSay(pExec);
			break;

		case EXEC_SELECT_MSG:
			SelectMsg(pExec);
			break;

		case EXEC_RUN_EVENT:
		{
			EVENT* pEvent = nullptr;
			EVENT_DATA* pEventData = nullptr;

			pEvent = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);
			if (pEvent == nullptr)
				break;

			pEventData = pEvent->m_arEvent.GetData(pExec->m_ExecInt[0]);
			if (pEventData == nullptr)
				break;

			if (!CheckEventLogic(pEventData))
				break;

			for (EXEC* pExec : pEventData->m_arExec)
			{
				if (!RunNpcEvent(pNpc, pExec))
					return false;
			}
		}
		break;

		case EXEC_GIVE_ITEM:
			if (!GiveItem(pExec->m_ExecInt[0], pExec->m_ExecInt[1]))
				return false;
			break;

		case EXEC_ROB_ITEM:
			if (!RobItem(pExec->m_ExecInt[0], pExec->m_ExecInt[1]))
				return false;
			break;

		//	비러머글 복권 >.<
		case EXEC_OPEN_EDITBOX:
			OpenEditBox(pExec->m_ExecInt[1], pExec->m_ExecInt[2]);
			break;

		case EXEC_GIVE_NOAH:
			GoldGain(pExec->m_ExecInt[0]);
			break;

		case EXEC_LOG_COUPON_ITEM:
			LogCoupon(pExec->m_ExecInt[0], pExec->m_ExecInt[1]);
			break;
	//
	// 비러머글 엑셀 >.<
		case EXEC_SAVE_COM_EVENT:
			SaveComEvent(pExec->m_ExecInt[0]);
			break;
	//
		case EXEC_RETURN:
			return false;
	}

	return true;
}

bool CUser::RunEvent(const EVENT_DATA* pEventData)
{
	for (EXEC* pExec : pEventData->m_arExec)
	{
		if (pExec == nullptr)
			break;

		switch (pExec->m_Exec)
		{
			case EXEC_SAY:
				SendNpcSay(pExec);
				break;

			case EXEC_SELECT_MSG:
				SelectMsg(pExec);
				break;

			case EXEC_RUN_EVENT:
			{
				EVENT* pEvent = nullptr;
				EVENT_DATA* pEventData = nullptr;

				pEvent = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);
				if (pEvent == nullptr)
					break;

				pEventData = pEvent->m_arEvent.GetData(pExec->m_ExecInt[0]);
				if (pEventData == nullptr)
					break;

				if (!CheckEventLogic(pEventData))
					break;

				if (!RunEvent(pEventData))
					return false;
			}
			break;

			case EXEC_GIVE_ITEM:
				if (!GiveItem(pExec->m_ExecInt[0], pExec->m_ExecInt[1]))
					return false;
				break;

			case EXEC_ROB_ITEM:
				if (!RobItem(pExec->m_ExecInt[0], pExec->m_ExecInt[1]))
					return false;
				break;

//	비러머글 복권 >.<
			case EXEC_OPEN_EDITBOX:
				OpenEditBox(pExec->m_ExecInt[1], pExec->m_ExecInt[2]);
				break;

			case EXEC_GIVE_NOAH:
				GoldGain(pExec->m_ExecInt[0]);
				break;

			case EXEC_LOG_COUPON_ITEM:
				LogCoupon(pExec->m_ExecInt[0], pExec->m_ExecInt[1]);
				break;
//
// 비러머글 엑셀 >.<
			case EXEC_SAVE_COM_EVENT:
				SaveComEvent(pExec->m_ExecInt[0]);
				break;

			case EXEC_ROB_NOAH:
				GoldLose(pExec->m_ExecInt[0]);
				break;
//
			case EXEC_RETURN:
				return false;
		}
	}

	return true;
}

// 정애씨가 고생하면서 해주신 퀘스트 부분 끝
void CUser::TestPacket(char* pBuf)
{
	// npc의 리스트를 재 요청하는 군,,,,,
	m_pMain->RegionNpcInfoForMe(this, 1);
	m_pMain->SyncTest(2);
}

void CUser::ItemLogToAgent(const char* srcid, const char* tarid, int type, int64_t serial, int itemid, int count, int durability)
{
	int send_index = 0, retvalue = 0;
	char send_buff[1024] = {};

	SetByte(send_buff, WIZ_ITEM_LOG, send_index);
	SetShort(send_buff, strlen(srcid), send_index);
	SetString(send_buff, (char*) srcid, strlen(srcid), send_index);
	SetShort(send_buff, strlen(tarid), send_index);
	SetString(send_buff, (char*) tarid, strlen(tarid), send_index);
	SetByte(send_buff, type, send_index);
	SetInt64(send_buff, serial, send_index);
	SetDWORD(send_buff, itemid, send_index);
	SetShort(send_buff, count, send_index);
	SetShort(send_buff, durability, send_index);

	retvalue = m_pMain->m_ItemLoggerSendQ.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"ItemLogToAgent: item send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
	}
}

void CUser::SendItemWeight()
{
	int send_index = 0;
	char send_buff[256] = {};

	SetSlotItemValue();
	SetByte(send_buff, WIZ_WEIGHT_CHANGE, send_index);
	SetShort(send_buff, GetCurrentWeightForClient(), send_index);
	Send(send_buff, send_index);
}

void CUser::GoldGain(int gold)
{
	int send_index = 0;
	char send_buff[256] = {};
	int64_t iTotalGold = 0;

	if (m_pUserData->m_iGold < 0)
	{
		spdlog::error("User::GoldGain: user has negative gold [charId={} existingGold={}]",
			m_pUserData->m_id, m_pUserData->m_iGold);
		return;
	}	

	if (gold < 0)
		gold = 0;
	
	iTotalGold = static_cast<int64_t>(m_pUserData->m_iGold) + static_cast<int64_t>(gold);

	if (iTotalGold > MAX_GOLD)
		iTotalGold = MAX_GOLD;

	// set user gold as iTotalGold
	m_pUserData->m_iGold = static_cast<int>(iTotalGold);

	SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
	SetByte(send_buff, GOLD_CHANGE_GAIN, send_index);
	SetDWORD(send_buff, gold, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	Send(send_buff, send_index);
}

bool CUser::GoldLose(int gold)
{
	int send_index = 0;
	char send_buff[256] = {};

	if (m_pUserData->m_iGold < 0)
	{
		spdlog::error("User::GoldLose: user has negative gold [charId={} existingGold={}]",
			m_pUserData->m_id, m_pUserData->m_iGold);
		return false;
	}

	if (gold < 0)
		gold = 0;

	// Insufficient gold!
	if (m_pUserData->m_iGold < gold)
		return false;

	m_pUserData->m_iGold -= gold;	// Subtract gold.

	SetByte(send_buff, WIZ_GOLD_CHANGE, send_index);
	SetByte(send_buff, GOLD_CHANGE_LOSE, send_index);
	SetDWORD(send_buff, gold, send_index);
	SetDWORD(send_buff, m_pUserData->m_iGold, send_index);
	Send(send_buff, send_index);

	return true;
}

bool CUser::CheckSkillPoint(BYTE skillnum, BYTE min, BYTE max) const
{
	if (skillnum < 5
		|| skillnum > 8)
		return false;

	if (m_pUserData->m_bstrSkill[skillnum] < min
		|| m_pUserData->m_bstrSkill[skillnum] > max)
		return false;

	return true;
}

bool CUser::CheckWeight(int itemid, short count) const
{
	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return false;

	if (pTable->Countable == 0)
	{
		// Check weight first!
		if ((m_iItemWeight + pTable->Weight) <= m_iMaxWeight)
		{
			// Now check empty slots :P
			if (GetEmptySlot(itemid, 0) != 0xFF)
				return true;
		}
	}
	else
	{
		// Check weight first!
		if (((pTable->Weight * count) + m_iItemWeight) <= m_iMaxWeight)
		{
			// Now check empty slots :P
			if (GetEmptySlot(itemid, pTable->Countable) != 0xFF)
				return true;
		}
	}

	return false;
}

bool CUser::CheckExistItem(int itemid, short count) const
{
	// This checks if such an item exists.
	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return false;

	// Check every slot in this case.....
	for (int i = 0; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum != itemid)
			continue;

		// Non-countable item. Automatically return true
		if (pTable->Countable == 0)
			return true;

		// Countable items. Make sure the amount is same or higher.
		if (m_pUserData->m_sItemArray[i].sCount >= count)
			return true;

		return false;
	}

	return false;
}

bool CUser::RobItem(int itemid, short count)
{
	int send_index = 0;
	char send_buff[256] = {};
	BYTE type = 1;

	// This checks if such an item exists.
	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return false;

	int i;
	for (i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX * type; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum != itemid)
			continue;

		// Remove item from inventory (Non-countable items)
		if (pTable->Countable == 0)
		{
			m_pUserData->m_sItemArray[i].nNum = 0;
			m_pUserData->m_sItemArray[i].sCount = 0;
			m_pUserData->m_sItemArray[i].sDuration = 0;
			goto success_return;
		}

		// Remove the number of items from the inventory (Countable Items)
		if (m_pUserData->m_sItemArray[i].sCount < count)
			return false;

		m_pUserData->m_sItemArray[i].sCount -= count;
		if (m_pUserData->m_sItemArray[i].sCount == 0)
		{
			m_pUserData->m_sItemArray[i].nNum = 0;
			m_pUserData->m_sItemArray[i].sCount = 0;
			m_pUserData->m_sItemArray[i].sDuration = 0;
		}
		goto success_return;
	}

	return false;

success_return:
	SendItemWeight();	// Change weight first :)
	SetByte(send_buff, WIZ_ITEM_COUNT_CHANGE, send_index);
	SetShort(send_buff, 1, send_index);	// The number of for-loops
	SetByte(send_buff, 1, send_index);
	SetByte(send_buff, i - SLOT_MAX, send_index);
	SetDWORD(send_buff, itemid, send_index);	// The ID of item.
	SetDWORD(send_buff, m_pUserData->m_sItemArray[i].sCount, send_index);
	Send(send_buff, send_index);
	return true;
}

bool CUser::GiveItem(int itemid, short count)
{
	int pos = 255;
	int send_index = 0;
	char send_buff[128] = {};

	// This checks if such an item exists.
	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return false;

	pos = GetEmptySlot(itemid, pTable->Countable);

	// No empty slots.
	if (pos == 0xFF)
		return false;

	// Common Item
	if (pos >= HAVE_MAX)
		return false;

	if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != 0)
	{
		if (pTable->Countable != 1)
			return false;
			
		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != itemid)
			return false;
	}

	/*	이건 필요할 때 주석 빼도록....
	// Check weight of countable item.
	if (pTable->Countable != 0)
	{
		if (((pTable->Weight * count) + m_iItemWeight) > m_iMaxWeight)
			return false;
	}
	// Check weight of non-countable item.
	else
	{
		if ((pTable->Weight + m_iItemWeight) > m_iMaxWeight)
			return false;
	}*/

	// Add item to inventory.
	m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum = itemid;

	// Apply number of items to a countable item.
	if (pTable->Countable != 0)
	{
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount += count;
		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount > MAX_ITEM_COUNT)
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = MAX_ITEM_COUNT;
	}
	// Just add uncountable item to inventory.
	else
	{
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount = 1;
	}

	// Apply duration to item.
	m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = pTable->Durability;

	SendItemWeight();	// Change weight first :)
	SetByte(send_buff, WIZ_ITEM_COUNT_CHANGE, send_index);
	SetShort(send_buff, 1, send_index);	// The number of for-loops
	SetByte(send_buff, 1, send_index);
	SetByte(send_buff, pos, send_index);
	SetDWORD(send_buff, itemid, send_index);	// The ID of item.
	SetDWORD(send_buff, m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount, send_index);
	Send(send_buff, send_index);
	return true;
}

bool CUser::CheckClass(short class1, short class2, short class3, short class4, short class5, short class6) const
{
	if (JobGroupCheck(class1))
		return true;

	if (JobGroupCheck(class2))
		return true;

	if (JobGroupCheck(class3))
		return true;

	if (JobGroupCheck(class4))
		return true;

	if (JobGroupCheck(class5))
		return true;

	if (JobGroupCheck(class6))
		return true;

	return false;
}

// Receive menu reply from client.
void CUser::RecvSelectMsg(char* pBuf)
{
	int index = 0, selevent, selnum;
	EVENT* pEvent = nullptr;
	EVENT_DATA* pEventData = nullptr;

	selnum = GetByte(pBuf, index);

	// 비러머글 퀘스트 >.<
	if (selnum < 0
		|| selnum >= MAX_MESSAGE_EVENT)
		goto fail_return;

	// Get the event number that needs to be processed next.
	selevent = m_iSelMsgEvent[selnum];

	pEvent = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);
	if (pEvent == nullptr)
		goto fail_return;

	pEventData = pEvent->m_arEvent.GetData(selevent);
	if (pEventData == nullptr)
		goto fail_return;

	if (!CheckEventLogic(pEventData))
		goto fail_return;

	if (!RunEvent(pEventData))
		goto fail_return;

	return;

fail_return:
// 비러머글 퀘스트 >.<
	for (int i = 0; i < MAX_MESSAGE_EVENT; i++)
		m_iSelMsgEvent[i] = -1;
//
}

void CUser::SendNpcSay(const EXEC* pExec)
{
	int send_index = 0;
	char send_buff[128] = {};

	if (pExec == nullptr)
		return;

	SetByte(send_buff, WIZ_NPC_SAY, send_index);

	// It will be TEN for now!!!
	for (int i = 0; i < 10; i++)
		SetDWORD(send_buff, pExec->m_ExecInt[i], send_index);

	Send(send_buff, send_index);
}

void CUser::SelectMsg(const EXEC* pExec)
{
	int i, chat, send_index = 0;
	char send_buff[128] = {};

	if (pExec == nullptr)
		return;

	SetByte(send_buff, WIZ_SELECT_MSG, send_index);
	SetShort(send_buff, m_sEventNid, send_index);
//	SetByte(send_buff, (BYTE) pExec->m_ExecInt[0], send_index);		// NPC 직업
	SetDWORD(send_buff, pExec->m_ExecInt[1], send_index);			// 지문 번호

	chat = 2;

	// 비러머글 퀘스트 >.<
	for (i = 0; i < MAX_MESSAGE_EVENT; i++)
	{
		SetDWORD(send_buff, pExec->m_ExecInt[chat], send_index);
		chat += 2;
	}

	Send(send_buff, send_index);

/*
	m_iSelMsgEvent[0] = pExec->m_ExecInt[3];
	m_iSelMsgEvent[1] = pExec->m_ExecInt[5];
	m_iSelMsgEvent[2] = pExec->m_ExecInt[7];
	m_iSelMsgEvent[3] = pExec->m_ExecInt[9];
	m_iSelMsgEvent[4] = pExec->m_ExecInt[11];
*/

// 비러머글 퀘스트 >.<
	for (int j = 0; j < MAX_MESSAGE_EVENT; j++)
		m_iSelMsgEvent[j] = pExec->m_ExecInt[(2 * j) + 3];
//
}

bool CUser::JobGroupCheck(short jobgroupid) const
{
	// Check job groups
	if (jobgroupid < 100)
	{
		switch (jobgroupid)
		{
			case JOB_GROUP_WARRIOR:
				if (m_pUserData->m_sClass == 101
					|| m_pUserData->m_sClass == 105
					|| m_pUserData->m_sClass == 106
					|| m_pUserData->m_sClass == 201
					|| m_pUserData->m_sClass == 205
					|| m_pUserData->m_sClass == 206)
					return true;
				break;

			case JOB_GROUP_ROGUE:
				if (m_pUserData->m_sClass == 102
					|| m_pUserData->m_sClass == 107
					|| m_pUserData->m_sClass == 108
					|| m_pUserData->m_sClass == 202
					|| m_pUserData->m_sClass == 207
					|| m_pUserData->m_sClass == 208)
					return true;
				break;

			case JOB_GROUP_MAGE:
				if (m_pUserData->m_sClass == 103
					|| m_pUserData->m_sClass == 109
					|| m_pUserData->m_sClass == 110 
					|| m_pUserData->m_sClass == 203
					|| m_pUserData->m_sClass == 209
					|| m_pUserData->m_sClass == 210)
					return true;
				break;

			case JOB_GROUP_CLERIC:
				if (m_pUserData->m_sClass == 104
					|| m_pUserData->m_sClass == 111
					|| m_pUserData->m_sClass == 112
					|| m_pUserData->m_sClass == 204
					|| m_pUserData->m_sClass == 211
					|| m_pUserData->m_sClass == 212)
					return true;
				break;

			case JOB_GROUP_ATTACK_WARRIOR:
				if (m_pUserData->m_sClass == 105
					|| m_pUserData->m_sClass == 205)
					return true;
				break;

			case JOB_GROUP_DEFENSE_WARRIOR:
				if (m_pUserData->m_sClass == 106
					|| m_pUserData->m_sClass == 206)
					return true;
				break;

			case JOB_GROUP_ARCHERER:
				if (m_pUserData->m_sClass == 107
					|| m_pUserData->m_sClass == 207)
					return true;
				break;

			case JOB_GROUP_ASSASSIN:
				if (m_pUserData->m_sClass == 108
					|| m_pUserData->m_sClass == 208)
					return true;
				break;

			case JOB_GROUP_ATTACK_MAGE:
				if (m_pUserData->m_sClass == 109
					|| m_pUserData->m_sClass == 209)
					return true;
				break;

			case JOB_GROUP_PET_MAGE:
				if (m_pUserData->m_sClass == 110
					|| m_pUserData->m_sClass == 210)
					return true;
				break;

			case JOB_GROUP_HEAL_CLERIC:
				if (m_pUserData->m_sClass == 111
					|| m_pUserData->m_sClass == 211)
					return true;
				break;

			case JOB_GROUP_CURSE_CLERIC:
				if (m_pUserData->m_sClass == 112
					|| m_pUserData->m_sClass == 212)
					return true;
				break;
		}
	}
	// Just check if the class is the same as the player's class
	else
	{
		if (m_pUserData->m_sClass == jobgroupid)
			return true;
	}

	return false;
}

// 잉...성용씨 미워!!! 흑흑흑 ㅠ.ㅠ
void CUser::TrapProcess()
{
	float currenttime = TimeGet();

	// Time interval has passed :)
	if (ZONE_TRAP_INTERVAL < (currenttime - m_fLastTrapAreaTime))
	{
		// Reduce target health point.
		HpChange(-ZONE_TRAP_DAMAGE);

		// Check if the target is dead.
		if (m_pUserData->m_sHp == 0)
		{
			// Target status is officially dead now.
			m_bResHpType = USER_DEAD;

			InitType3();	// Init Type 3.....
			InitType4();	// Init Type 4.....

			m_sWhoKilledMe = -1;		// Who the hell killed me?
		}
	}

	// Update Last Trap Area time :)
	m_fLastTrapAreaTime = currenttime;
}

void CUser::KickOutZoneUser(bool home)
{
	int regene_event = 0;

	int zoneindex = m_pMain->GetZoneIndex(m_pUserData->m_bNation);
	if (zoneindex < 0)
		return;

	C3DMap* pMap = m_pMain->GetMapByIndex(zoneindex);
	if (pMap == nullptr)
		return;

	if (home)
	{
		// ZoneChange(pMap->m_nZoneNumber, pMap->m_fInitX, pMap->m_fInitZ);

// 비러머글 버퍼
		int random = myrand(0, 9000);
		if (random < 3000)
			regene_event = 0;
		else if (random < 6000)
			regene_event = 1;
		else if (random < 9001)
			regene_event = 2;

		_REGENE_EVENT* pRegene = pMap->GetRegeneEvent(regene_event);
		if (pRegene == nullptr)
		{
			spdlog::error("User::KickOutZoneUser: no regene event found [charId={} regeneEventId={} zoneIndex={}]",
				m_pUserData->m_id, regene_event, zoneindex);
			KickOutZoneUser();
			return;
		}

		//TRACE(_T("KickOutZoneUser - user=%hs, regene_event=%d\n"), m_pUserData->m_id, regene_event);

		int delta_x = myrand(0, pRegene->fRegeneAreaX);
		int delta_z = myrand(0, pRegene->fRegeneAreaZ);

		int x = pRegene->fRegenePosX + delta_x;
		int y = pRegene->fRegenePosZ + delta_z;

		ZoneChange(pMap->m_nZoneNumber, x, y);
	}
	else
	{
		// Move user to native zone.
		if (m_pUserData->m_bNation == KARUS)
			ZoneChange(pMap->m_nZoneNumber, 1335, 83);
		else
			ZoneChange(pMap->m_nZoneNumber, 445, 1950);
	}
}

void CUser::EventMoneyItemGet(int itemid, int count)
{
/*
	int index = 0, send_index = 0, bundle_index = 0, itemid = 0, count = 0, i = 0;
	BYTE pos;
	char send_buff[256] = {};
	C3DMap* pMap = nullptr;
	CUser* pUser = nullptr;
	CUser* pGetUser = nullptr;
	_PARTY_GROUP* pParty = nullptr;

	if  m_sExchangeUser != -1)
	goto fail_return;

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		goto fail_return;

	pGetUser = this;
*/
}

void CUser::NativeZoneReturn()
{
	// Send user back home in case it was the battlezone.
	model::Home* pHomeInfo = m_pMain->m_HomeTableMap.GetData(m_pUserData->m_bNation);
	if (pHomeInfo == nullptr)
		return;

	m_pUserData->m_bZone = m_pUserData->m_bNation;

	if (m_pUserData->m_bNation == KARUS)
	{
		m_pUserData->m_curx = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
		m_pUserData->m_curz = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
	}
	else
	{
		m_pUserData->m_curx = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
		m_pUserData->m_curz = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
	}
}

bool CUser::CheckEditBox() const
{
	std::string id = fmt::format_db_resource(IDS_COUPON_NOTEPAD_ID);
	if (id == m_strCouponId)
		return true;

	id = fmt::format_db_resource(IDS_COUPON_POSTIT_ID);
	if (id == m_strCouponId)
		return true;

	return false;
}

void CUser::OpenEditBox(int message, int event)
{
	// 이것은 기술지원 필요함 ㅠ.ㅠ
	// if (!CheckCouponUsed())
	//	return;

	// 이것은 기술지원 필요함 ㅠ.ㅠ
	int send_index = 0, retvalue = 0;
	char send_buff[256] = {};

	SetByte(send_buff, DB_COUPON_EVENT, send_index);
	SetByte(send_buff, CHECK_COUPON_EVENT, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);
	SetDWORD(send_buff, event, send_index);
//	비러머글 대사문 >.<
	SetDWORD(send_buff, message, send_index);
//	
	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"OpenEditBox: coupon send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
		//goto fail_return;
	}
/*
	return true;

fail_return:
	send_index = 0;
	return false;

	m_iEditBoxEvent = event;	// What will the next event be when an answer is given?

	send_index = 0;
	memset(send_buff, 0, sizeof(send_buff));
	SetByte(send_buff, WIZ_EDIT_BOX, send_index);
	Send(send_buf, send_index);*/
}

bool CUser::CheckRandom(short percent) const
{
	if (percent < 0
		|| percent > 1000)
		return false;

	int rand = myrand(0, 1000);
	return (percent > rand);
}

void CUser::RecvEditBox(char* pBuf)
{
	EVENT* pEvent = nullptr;
	EVENT_DATA* pEventData = nullptr;

	int index = 0, selevent = -1;
	short coupon_length = 0;
	char send_buff[256] = {}; // , coupon_id[MAX_COUPON_ID_LENGTH];

	coupon_length = GetShort(pBuf, index);		// Get length of coupon number.
	if (coupon_length < 0
		|| coupon_length > MAX_COUPON_ID_LENGTH)
		return;

	GetString(m_strCouponId, pBuf, coupon_length, index);		// Get actual coupon number.

	selevent = m_iEditBoxEvent;

	pEvent = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);	// 여기서 부터 중요함 --;
	if (pEvent == nullptr)
		goto fail_return;

	pEventData = pEvent->m_arEvent.GetData(selevent);
	if (pEventData == nullptr)
		goto fail_return;

	if (!CheckEventLogic(pEventData))
		goto fail_return;

	if (!RunEvent(pEventData))
		goto fail_return;

	return;

fail_return:
	m_iEditBoxEvent = -1;
	memset(m_strCouponId, 0, sizeof(m_strCouponId));
}

bool CUser::CheckCouponUsed() const
{
	// 이것은 기술지원 필요함 ㅠ.ㅠ
	return true;
}

void CUser::LogCoupon(int itemid, int count)
{
	// 참고로 쿠폰 번호는 : m_iEditboxEvent
	// 이것은 기술지원 필요함 ㅠ.ㅠ	
	int send_index = 0, retvalue = 0;
	char send_buff[256] = {};

	SetByte(send_buff, DB_COUPON_EVENT, send_index);
	SetByte(send_buff, UPDATE_COUPON_EVENT, send_index);
	SetShort(send_buff, m_Sid, send_index);
	SetShort(send_buff, strlen(m_strAccountID), send_index);
	SetString(send_buff, m_strAccountID, strlen(m_strAccountID), send_index);
	SetShort(send_buff, strlen(m_pUserData->m_id), send_index);
	SetString(send_buff, m_pUserData->m_id, strlen(m_pUserData->m_id), send_index);
	SetShort(send_buff, strlen(m_strCouponId), send_index);
	SetString(send_buff, m_strCouponId, strlen(m_strCouponId), send_index);
	SetDWORD(send_buff, itemid, send_index);
	SetDWORD(send_buff, count, send_index);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	if (retvalue >= SMQ_FULL)
	{
		std::wstring logstr = std::format(L"LogCoupon: send error: {}", retvalue);
		m_pMain->AddOutputMessage(logstr);
		//goto fail_return;
	}
}

void CUser::CouponEvent(char* pBuf)
{
	int index = 0, nEventNum = 0, nItemCount = 0, nResult = 0, nMessageNum = 0;
	nResult = GetByte(pBuf, index);
	nEventNum = GetDWORD(pBuf, index);
// 비러머글 대사 >.<
	nMessageNum = GetDWORD(pBuf, index);
//

	if (nResult == 0)
		return;

	// 알아서 사용
	int send_index = 0;
	char send_buff[128] = {};
	m_iEditBoxEvent = nEventNum;	// What will the next event be when an answer is given?
	SetByte(send_buff, WIZ_EDIT_BOX, send_index);
// 비러머글 대사 >.<
	SetDWORD(send_buff, nMessageNum, send_index);
//
	Send(send_buff, send_index);
}

bool CUser::CheckItemCount(int itemid, short min, short max) const
{
	// This checks if such an item exists.
	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return false;

	// Scan the inventory (exclude equipped items).
	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum != itemid)
			continue;

		// Non-countable item.
		if (pTable->Countable == 0)
		{
			// Caller expected some quantity.
			// We do have an item already, it's just not stackable, so we should allow it.
			if (min != 0 || max != 0)
				return true;

			// This is somewhat of a special case since the item does exist, but this effectively
			// just restores the old behaviour where non-stackable items always failed this check.
			// In this case, it only fails when the caller doesn't ask for a specific quantity.
			return false;
		}

		// Countable items. Make sure the amount is within the range.
		if (m_pUserData->m_sItemArray[i].sCount < min
			|| m_pUserData->m_sItemArray[i].sCount > max)
		{
			// This check is a bit wonky, but it's official behaviour.
			// If the min check failed, the stack size is negative, which is clearly a bad case and
			// we can fail.
			// Otherwise, it's probably just intending to detect a misconfiguration here; the caller
			// typically expects to supply 0 for both min and max at the same time, so it exceeding
			// the max (implying non-zero) but having a min of 0 seems like a misconfiguration.
			// At least, that's my best guess for this scenario.
			if (min == 0)
				return false;

			// Only bother to enforce the max check if the caller specified it.
			// Otherwise, it should be ignored.
			if (max != 0)
				return false;
		}

		return true;
	}

	// Item not found in inventory
	// Succeed only if we essentially don't require any to exist.
	if (min == 0
		|| max == 0)
		return true;

	// Player doesn't have any; caller expected some quantity.
	return false;
}

bool CUser::CheckClanGrade(int min, int max) const
{
	if (m_pUserData->m_bKnights == 0)
		return false;

	CKnights* pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
	if (pKnights == nullptr)
		return false;

	if (pKnights->m_byGrade < min || pKnights->m_byGrade > max)
		return false;

	return true;
}

void CUser::SaveComEvent(int eventid)
{
	for (int i = 0; i < MAX_CURRENT_EVENT; i++)
	{
		if (m_sEvent[i] != eventid)
		{
			m_sEvent[i] = eventid;
			break;
		}
	}
}

bool CUser::ExistComEvent(int eventid) const
{
	for (int i = 0; i < MAX_CURRENT_EVENT; i++)
	{
		if (m_sEvent[i] == eventid)
			return true;
	}

	return false;
}

void CUser::RecvDeleteChar(char* pBuf)
{
	int nResult = 0, nLen = 0, index = 0, send_index = 0, char_index = 0, knightsId = 0;
	char charId[MAX_ID_SIZE + 1] = {};
	char send_buff[256] = {};

	nResult = GetByte(pBuf, index);
	char_index = GetByte(pBuf, index);
	knightsId = GetShort(pBuf, index);
	nLen = GetShort(pBuf, index);
	GetString(charId, pBuf, nLen, index);

	if (nResult == 1
		&& knightsId != 0)
	{
		m_pMain->m_KnightsManager.RemoveKnightsUser(knightsId, charId);
		spdlog::debug("User::RecvDeleteChar: removed [charId={} knightsId={}]",
			charId, knightsId);

		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, UDP_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_WITHDRAW, send_index);
		SetShort(send_buff, knightsId, send_index);
		SetShort(send_buff, nLen, send_index);
		SetString(send_buff, charId, nLen, send_index);

		if (m_pMain->m_nServerGroup == 0)
			m_pMain->Send_UDP_All(send_buff, send_index);
		else
			m_pMain->Send_UDP_All(send_buff, send_index, 1);
	}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_DEL_CHAR, send_index);
	SetByte(send_buff, nResult, send_index);					// 성공시 국가 정보
	SetByte(send_buff, char_index, send_index);

	Send(send_buff, send_index);
}

void CUser::GetUserInfo(char* buff, int& buff_index)
{
	CKnights* pKnights = nullptr;

	SetString1(buff, m_pUserData->m_id, static_cast<BYTE>(strlen(m_pUserData->m_id)), buff_index);
	SetByte(buff, m_pUserData->m_bNation, buff_index);
	SetShort(buff, m_pUserData->m_bKnights, buff_index);
	SetByte(buff, m_pUserData->m_bFame, buff_index);

	if (m_pUserData->m_bKnights != 0)
		pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);

	if (pKnights != nullptr)
	{
		SetShort(buff, pKnights->m_sAllianceKnights, buff_index);
		SetString1(buff, pKnights->m_strName, static_cast<BYTE>(strlen(pKnights->m_strName)), buff_index);
		SetByte(buff, pKnights->m_byGrade, buff_index);  // knights grade
		SetByte(buff, pKnights->m_byRanking, buff_index);
		SetShort(buff, pKnights->m_sMarkVersion, buff_index);
		SetShort(buff, pKnights->m_sCape, buff_index);
	}
	else
	{
		SetShort(buff, 0, buff_index);		// m_sAllianceKnights
		SetByte(buff, 0, buff_index);		// m_strName
		SetByte(buff, 0, buff_index);		// m_byGrade
		SetByte(buff, 0, buff_index);		// m_byRanking
		SetShort(buff, 0, buff_index);		// m_sMarkVerison
		SetShort(buff, -1, buff_index);		// m_sCape
	}

	SetByte(buff, m_pUserData->m_bLevel, buff_index);
	SetByte(buff, m_pUserData->m_bRace, buff_index);
	SetShort(buff, m_pUserData->m_sClass, buff_index);
	SetShort(buff, (WORD) m_pUserData->m_curx * 10, buff_index);
	SetShort(buff, (WORD) m_pUserData->m_curz * 10, buff_index);
	SetShort(buff, (short) m_pUserData->m_cury * 10, buff_index);
	SetByte(buff, m_pUserData->m_bFace, buff_index);
	SetByte(buff, m_pUserData->m_bHairColor, buff_index);
	SetByte(buff, m_bResHpType, buff_index);
// 비러머글 수능...
	SetDWORD(buff, m_bAbnormalType, buff_index);
//
	SetByte(buff, m_bNeedParty, buff_index);
	SetByte(buff, m_pUserData->m_bAuthority, buff_index);

	SetByte(buff, static_cast<BYTE>(m_bIsPartyLeader), buff_index);
	SetByte(buff, m_byInvisibilityState, buff_index);
	SetShort(buff, m_sDirection, buff_index);
	SetByte(buff, static_cast<BYTE>(m_bIsChicken), buff_index);
	SetByte(buff, m_pUserData->m_bRank, buff_index);
	SetByte(buff, m_byKnightsRank, buff_index);
	SetByte(buff, m_byPersonalRank, buff_index);

	for (const int slot : { BREAST, LEG, HEAD, GLOVE, FOOT, SHOULDER, RIGHTHAND, LEFTHAND })
	{
		SetDWORD(buff, m_pUserData->m_sItemArray[slot].nNum, buff_index);
		SetShort(buff, m_pUserData->m_sItemArray[slot].sDuration, buff_index);
		SetShort(buff, m_pUserData->m_sItemArray[slot].byFlag, buff_index);
	}
}

void CUser::GameStart(
	char* pBuf)
{
	int index = 0, send_index = 0;
	char send_buff[512];

	int opcode = GetByte(pBuf, index);

	// Started loading
	if (opcode == 1)
	{
		SendMyInfo(0);
		m_pMain->UserInOutForMe(this);
		m_pMain->NpcInOutForMe(this);
		SendNotice();
		SendTimeStatus();

		spdlog::debug("User::GameStart: loading [charId={} socketId={}]",
			m_pUserData->m_id, m_Sid);

		SetByte(send_buff, WIZ_GAMESTART, send_index);
		Send(send_buff, send_index);
	}
	// Finished loading
	else if (opcode == 2)
	{
		// NOTE: This behaviour is flipped as compared to official to give it a more meaningful name.
		bool bRecastSavedMagic = true;

		m_State = STATE_GAMESTART;

		spdlog::debug("User::GameStart: in game [charId={} socketId={}]",
			m_pUserData->m_id, m_Sid);

		UserInOut(USER_REGENE);

		if (m_pUserData->m_bCity == 0
			&& m_pUserData->m_sHp <= 0)
			m_pUserData->m_bCity = 0xff;

		if (m_pUserData->m_bCity == 0
			|| m_pUserData->m_bCity == 0xFF)
		{
			m_iLostExp = 0;
		}
		else
		{
			int level = m_pUserData->m_bLevel;
			if (m_pUserData->m_bCity <= 100)
				--level;

			m_iLostExp = m_pMain->m_LevelUpTableArray[level]->RequiredExp;
			m_iLostExp = m_iLostExp * (m_pUserData->m_bCity % 10) / 100;

			if (((m_pUserData->m_bCity % 100) / 10) == 1)
				m_iLostExp /= 2;
		}


		if (m_iLostExp > 0
			|| m_pUserData->m_bCity == 0xff)
		{
			HpChange(-m_iMaxHp);

			// NOTE: This behaviour is flipped as compared to official to give it a more meaningful name.
			bRecastSavedMagic = false;
		}

		SendMyInfo(2);

		// TODO:
		// BlinkStart();

		SetUserAbility();

		// If there is a permanent chat available!!!
		if (m_pMain->m_bPermanentChatMode)
		{
			SetByte(send_buff, WIZ_CHAT, send_index);
			SetByte(send_buff, PERMANENT_CHAT, send_index);
			SetByte(send_buff, KARUS, send_index);
			SetShort(send_buff, -1, send_index);		// sid
			SetByte(send_buff, 0, send_index);			// sender name length
			SetString2(send_buff, m_pMain->m_strPermanentChat, static_cast<short>(strlen(m_pMain->m_strPermanentChat)), send_index);
			Send(send_buff, send_index);
		}

#if 0 // TODO
		if (bRecastSavedMagic)
			ItemMallMagicRecast(false);
#endif
	}
}

void CUser::RecvZoneChange(char* pBuf)
{
	int index = 0;
	BYTE opcode = GetByte(pBuf, index);
	if (opcode == ZONE_CHANGE_LOADING)
	{
		m_pMain->UserInOutForMe(this);
		m_pMain->NpcInOutForMe(this);

		char send_buff[128];
		int send_index = 0;
		SetByte(send_buff, WIZ_ZONE_CHANGE, send_index);
		SetByte(send_buff, ZONE_CHANGE_LOADED, send_index);
		Send(send_buff, send_index);
	}
	else if (opcode == ZONE_CHANGE_LOADED)
	{
		// UserInOut(USER_IN);
		UserInOut(USER_REGENE);
		m_bWarp = 0;

		if (!m_bZoneChangeSameZone)
		{
#if 0 // TODO:
			BlinkStart();
			ItemMallMagicRecast(false);
#endif
		}
		else
		{
			m_bZoneChangeSameZone = false;
		}
	}
}

int16_t CUser::GetCurrentWeightForClient() const
{
	return std::min(m_iItemWeight, SHRT_MAX);
}

int16_t CUser::GetMaxWeightForClient() const
{
	return std::min(m_iMaxWeight, SHRT_MAX);
}

void CUser::SetZoneAbilityChange(int zone)
{
	constexpr int16_t TARIFF_BASE = 10;

	char send_buff[128];
	int send_index = 0;
	bool bCanTradeWithOtherNation = false, bCanTalkToOtherNation = false;
	uint8_t byZoneAbilityType = ZONE_ABILITY_NEUTRAL;
	int16_t sTariff = TARIFF_BASE;

	SetByte(send_buff, WIZ_ZONEABILITY, send_index);
	SetByte(send_buff, ZONE_ABILITY_UPDATE, send_index);

	if (zone == ZONE_MORADON)
	{
		bCanTradeWithOtherNation = true;
		byZoneAbilityType = ZONE_ABILITY_NEUTRAL;
		bCanTalkToOtherNation = true;
	}
	else if ((zone / 10) == 5
		&& zone != ZONE_CAITHAROS_ARENA)
	{
		bCanTradeWithOtherNation = true;
		byZoneAbilityType = ZONE_ABILITY_NEUTRAL;
		bCanTalkToOtherNation = true;
	}
	else
	{
		switch (zone)
		{
			case ZONE_ARENA:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType = ZONE_ABILITY_NEUTRAL;
				bCanTalkToOtherNation = true;
				break;

			case ZONE_CAITHAROS_ARENA:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType = ZONE_ABILITY_CAITHAROS_ARENA;
				bCanTalkToOtherNation = true;
				break;

			case ZONE_DESPERATION_ABYSS:
			case ZONE_HELL_ABYSS:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType = ZONE_ABILITY_PVP_NEUTRAL_NPCS;
				bCanTalkToOtherNation = true;
				break;

			case ZONE_FRONTIER:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType = ZONE_ABILITY_PVP;
				bCanTalkToOtherNation = false;
				sTariff = TARIFF_BASE + 10;
				break;

			case ZONE_DELOS:
				bCanTradeWithOtherNation = true;
				byZoneAbilityType = ZONE_ABILITY_SIEGE_DISABLED;
				bCanTalkToOtherNation = true;
				break;

			default:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType = ZONE_ABILITY_PVP;
				bCanTalkToOtherNation = false;
				break;
		}
	}

	SetByte(send_buff, bCanTradeWithOtherNation, send_index);
	SetByte(send_buff, byZoneAbilityType, send_index);
	SetByte(send_buff, bCanTalkToOtherNation, send_index);
	SetShort(send_buff, sTariff, send_index);
	Send(send_buff, send_index);
}
