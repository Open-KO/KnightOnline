// GameSocket.cpp: implementation of the CGameSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"
#include "GameSocket.h"
#include "ServerDlg.h"
#include "User.h"
#include "Map.h"
#include "Region.h"
#include "Party.h"

#include "extern.h"

#include <shared/crc32.h>
#include <shared/lzf.h>

#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern CRITICAL_SECTION g_region_critical;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. RecvUserInfo(), RecvAttackReq(), RecvUserUpdate() 수정
*/

CGameSocket::CGameSocket()
{
	//m_pParty = nullptr;
}

CGameSocket::~CGameSocket()
{
	/*
	delete m_pParty;
	m_pParty = nullptr;
	*/
}

void CGameSocket::Initialize()
{
	m_sSocketID = -1;
	m_pMain = (CServerDlg*) AfxGetApp()->GetMainWnd();
	//m_pParty = new CParty;
	//m_pParty->Initialize();
	m_Party.Initialize();
}

void CGameSocket::CloseProcess()
{
	spdlog::info("GameSocket::CloseProcess: socketId={} sSid={}", m_sSocketID, m_Sid);
	m_pMain->DeleteAllUserList(m_sSocketID);
	Initialize();
	CIOCPSocket2::CloseProcess();
}

void CGameSocket::Parsing(int length, char* pData)
{
	int index = 0;
	uint8_t bType = GetByte(pData, index);

	switch (bType)
	{
		case AI_SERVER_CONNECT:
			RecvServerConnect(pData);
			break;

		case AG_USER_INFO:
			RecvUserInfo(pData + index);
			break;

		case AG_USER_INOUT:
			RecvUserInOut(pData + index);
			break;

		case AG_USER_MOVE:
			RecvUserMove(pData + index);
			break;

		case AG_USER_MOVEEDGE:
			RecvUserMoveEdge(pData + index);
			break;

		case AG_ATTACK_REQ:
			RecvAttackReq(pData + index);
			break;

		case AG_USER_LOG_OUT:
			RecvUserLogOut(pData + index);
			break;

		case AG_USER_REGENE:
			RecvUserRegene(pData + index);
			break;

		case AG_USER_SET_HP:
			RecvUserSetHP(pData + index);
			break;

		case AG_USER_UPDATE:
			RecvUserUpdate(pData + index);
			break;

		case AG_ZONE_CHANGE:
			RecvZoneChange(pData + index);
			break;

		case AG_USER_PARTY:
			//if(m_pParty)
			m_Party.PartyProcess(pData + index);
			break;

		case AG_MAGIC_ATTACK_REQ:
			RecvMagicAttackReq(pData + index);
			break;

		case AG_COMPRESSED_DATA:
			RecvCompressedData(pData + index);
			break;

		case AG_USER_INFO_ALL:
			RecvUserInfoAllData(pData + index);
			break;

		case AG_PARTY_INFO_ALL:
			RecvPartyInfoAllData(pData + index);
			break;

		case AG_CHECK_ALIVE_REQ:
			RecvCheckAlive(pData + index);
			break;

		case AG_HEAL_MAGIC:
			RecvHealMagic(pData + index);
			break;

		case AG_TIME_WEATHER:
			RecvTimeAndWeather(pData + index);
			break;

		case AG_USER_FAIL:
			RecvUserFail(pData + index);
			break;

		case AG_BATTLE_EVENT:
			RecvBattleEvent(pData + index);
			break;

		case AG_NPC_GATE_OPEN:
			RecvGateOpen(pData + index);
			break;
	}
}

// sungyong 2002.05.22
void CGameSocket::RecvServerConnect(char* pBuf)
{
	int index = 1;
	int outindex = 0, zone_index = 0;
	float fReConnectEndTime = 0.0f;
	char pData[1024] = {};
	uint8_t byZoneNumber = GetByte(pBuf, index);
	uint8_t byReConnect = GetByte(pBuf, index);	// 0 : 처음접속, 1 : 재접속

	std::string logstr = fmt::format("Ebenezer connected to zone={}", byZoneNumber);
	m_pMain->AddOutputMessage(logstr);
	spdlog::info("GameSocket::RecvServerConnect: {}", logstr);

	if (byZoneNumber < 0)
	{
		SetByte(pData, AI_SERVER_CONNECT, outindex);
		SetByte(pData, -1, outindex);
		Send(pData, outindex);
	}

	m_sSocketID = byZoneNumber;

	SetByte(pData, AI_SERVER_CONNECT, outindex);
	SetByte(pData, byZoneNumber, outindex);
	SetByte(pData, byReConnect, outindex);
	Send(pData, outindex);

	// 재접속해서 리스트 받기 (강제로)
	if (byReConnect == 1)
	{
		if (m_pMain->m_sReSocketCount == 0)
			m_pMain->m_fReConnectStart = TimeGet();

		m_pMain->m_sReSocketCount++;

		spdlog::info("GameSocket::RecvServerConnect: Ebenezer reconnect: zone={} sockets={}",
			byZoneNumber, m_pMain->m_sReSocketCount);

		fReConnectEndTime = TimeGet();

		// all sockets reconnected within 2 minutes
		if (fReConnectEndTime > m_pMain->m_fReConnectStart + 120)
		{
			spdlog::info("GameSocket::RecvServerConnect: Ebenezer sockets reconnected in under 2 minutes [sockets={}]",
				m_pMain->m_sReSocketCount);
			m_pMain->m_sReSocketCount = 0;
			m_pMain->m_fReConnectStart = 0.0f;
		}

		if (m_pMain->m_sReSocketCount == MAX_AI_SOCKET)
		{
			fReConnectEndTime = TimeGet();

			// all sockets reconnected within 1 minute
			if (fReConnectEndTime < m_pMain->m_fReConnectStart + 60)
			{
				spdlog::info("GameSocket::RecvServerConnect: Ebenezer sockets reconnected within a minute [sockets={}]",
					m_pMain->m_sReSocketCount);
				m_pMain->m_bFirstServerFlag = true;
				m_pMain->m_sReSocketCount = 0;
				m_pMain->AllNpcInfo();
			}
			// 하나의 떨어진 소켓이라면...
			else
			{
				m_pMain->m_sReSocketCount = 0;
				m_pMain->m_fReConnectStart = 0.0f;
			}
		}
	}
	else
	{
		//m_pMain->PostMessage( WM_GAMESERVER_LOGIN, (LONG)byZoneNumber );
		m_pMain->m_sSocketCount++;
		spdlog::info("GameSocket::RecvServerConnect: Ebenezer connected [zone={}, sockets={}]",
			byZoneNumber, m_pMain->m_sSocketCount);
		if (m_pMain->m_sSocketCount == MAX_AI_SOCKET)
		{
			spdlog::info("GameSocket::RecvServerConnect: Ebenezer sockets all connected [sockets={}]",
				m_pMain->m_sSocketCount);
			m_pMain->m_bFirstServerFlag = true;
			m_pMain->m_sSocketCount = 0;
			m_pMain->AllNpcInfo();
		}
	}
}
// ~sungyong 2002.05.22

void CGameSocket::RecvUserInfo(char* pBuf)
{
//	TRACE(_T("RecvUserInfo()\n"));
	int index = 0;
	int16_t uid = -1, sHp, sMp, sZoneIndex, sLength = 0;
	uint8_t bNation, bLevel, bZone, bAuthority = 1;
	int16_t sDamage, sAC;
	float fHitAgi, fAvoidAgi;
	char strName[MAX_ID_SIZE + 1] = {};
//
	int16_t sItemAC, sAmountLeft, sAmountRight;
	uint8_t bTypeLeft, bTypeRight;
//
	uid = GetShort(pBuf, index);
	sLength = GetShort(pBuf, index);
	if (sLength > MAX_ID_SIZE
		|| sLength <= 0)
	{
		spdlog::error("GameSocket::RecvUserInfo: charId len={} overflow for userId={}", sLength, uid);
		return;
	}

	GetString(strName, pBuf, sLength, index);
	bZone = GetByte(pBuf, index);
	sZoneIndex = GetShort(pBuf, index);
	bNation = GetByte(pBuf, index);
	bLevel = GetByte(pBuf, index);
	sHp = GetShort(pBuf, index);
	sMp = GetShort(pBuf, index);
	sDamage = GetShort(pBuf, index);
	sAC = GetShort(pBuf, index);
	fHitAgi = Getfloat(pBuf, index);
	fAvoidAgi = Getfloat(pBuf, index);
//
	sItemAC = GetShort(pBuf, index);
	bTypeLeft = GetByte(pBuf, index);
	bTypeRight = GetByte(pBuf, index);
	sAmountLeft = GetShort(pBuf, index);
	sAmountRight = GetShort(pBuf, index);
	bAuthority = GetByte(pBuf, index);
//

	//CUser* pUser = m_pMain->GetActiveUserPtr(uid);
	//if( pUser == nullptr )		return;
	CUser* pUser = new CUser;
	pUser->Initialize();

	pUser->m_iUserId = uid;
	strcpy(pUser->m_strUserID, strName);
	pUser->m_curZone = bZone;
	pUser->m_sZoneIndex = sZoneIndex;
	pUser->m_bNation = bNation;
	pUser->m_byLevel = bLevel;
	pUser->m_sHP = sHp;
	pUser->m_sMP = sMp;
	//pUser->m_sSP = sSp;
	pUser->m_sHitDamage = sDamage;
	pUser->m_fHitrate = fHitAgi;
	pUser->m_fAvoidrate = fAvoidAgi;
	pUser->m_sAC = sAC;
	pUser->m_bLive = USER_LIVE;
//
	pUser->m_sItemAC = sItemAC;
	pUser->m_bMagicTypeLeftHand = bTypeLeft;
	pUser->m_bMagicTypeRightHand = bTypeRight;
	pUser->m_sMagicAmountLeftHand = sAmountLeft;
	pUser->m_sMagicAmountRightHand = sAmountRight;
	pUser->m_byIsOP = bAuthority;
//

	spdlog::debug("GameSocket::RecvUserInfo: userId={} charId={}", uid, strName);

	if (uid >= USER_BAND
		&& uid < MAX_USER)
		m_pMain->m_pUser[uid] = pUser;

	_USERLOG* pUserLog = nullptr;
	pUserLog = new _USERLOG;
	pUserLog->t = CTime::GetCurrentTime();
	pUserLog->byFlag = USER_LOGIN;
	pUserLog->byLevel = pUser->m_byLevel;
	strcpy(pUserLog->strUserID, pUser->m_strUserID);
	pUser->m_UserLogList.push_back(pUserLog);
}

void CGameSocket::RecvUserInOut(char* pBuf)
{
	int index = 0;
	uint8_t bType = -1;
	int16_t uid = -1, len = 0;
	char strName[MAX_ID_SIZE + 1] = {};
	float fX = -1, fZ = -1;

	bType = GetByte(pBuf, index);
	uid = GetShort(pBuf, index);
	len = GetShort(pBuf, index);
	GetString(strName, pBuf, len, index);
	fX = Getfloat(pBuf, index);
	fZ = Getfloat(pBuf, index);

	if (fX < 0
		|| fZ < 0)
	{
		spdlog::error("RecvUserInOut: invalid position charId={} fX={} fZ={}", strName, fX, fZ);
		return;
	}

//	TRACE(_T("RecvUserInOut(),, uid = %d\n"), uid);

	int region_x = 0, region_z = 0;
	int x1 = (int) fX / TILE_SIZE;
	int z1 = (int) fZ / TILE_SIZE;
	region_x = (int) fX / VIEW_DIST;
	region_z = (int) fZ / VIEW_DIST;

	// 수정할것,,, : 지금 존 번호를 0으로 했는데.. 유저의 존 정보의 번호를 읽어야,, 함,,
	MAP* pMap = nullptr;

	CUser* pUser = m_pMain->GetUserPtr(uid);

//	TRACE(_T("^^& RecvUserInOut( type=%d )-> User(%hs, %d),, zone=%d, index=%d, region_x=%d, y=%d\n"), bType, pUser->m_strUserID, pUser->m_iUserId, pUser->m_curZone, pUser->m_sZoneIndex, region_x, region_z);

	if (pUser != nullptr)
	{
	//	TRACE(_T("##### Fail : ^^& RecvUserInOut() [name = %hs]. state=%d, hp=%d\n"), pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		bool bFlag = false;

		if (pUser->m_bLive == USER_DEAD
			|| pUser->m_sHP <= 0)
		{
			if (pUser->m_sHP > 0)
			{
				pUser->m_bLive = 1;
			}
			
			spdlog::warn("GameSocket::RecvUserInOut: UserHeal error[charId={} isAlive={} hp={} fX={} fZ={}]",
					pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP, fX, fZ);
		}

		pMap = m_pMain->GetMapByIndex(pUser->m_sZoneIndex);
		if (pMap == nullptr)
		{
			spdlog::error("GameSocket::RecvUserInOut: Map not found for zoneIndex={} [charId={} x1={} z1={}]",
				pUser->m_sZoneIndex, pUser->m_strUserID, x1, z1);
			return;
		}

		if (x1 < 0
			|| z1 < 0
			|| x1 > pMap->m_sizeMap.cx
			|| z1 > pMap->m_sizeMap.cy)
		{
			spdlog::error("GameSocket::RecvUserInOut: Character position out of bounds [charId={} x1=%d, z1=%d]",
				pUser->m_strUserID, x1, z1);
			return;
		}
		// map 이동이 불가능이면 User등록 실패..
		//if(pMap->m_pMap[x1][z1].m_sEvent == 0) return;
		if (region_x > pMap->GetXRegionMax()
			|| region_z > pMap->GetZRegionMax())
		{
			spdlog::error("GameSocket::RecvUserInOut: region out of bounds [charId={} nRX={} nRZ={}]",
				pUser->m_strUserID, region_x, region_z);
			return;
		}

		//strcpy(pUser->m_strUserID, strName);
		pUser->m_curx = pUser->m_fWill_x = fX;
		pUser->m_curz = pUser->m_fWill_z = fZ;

		//bFlag = pUser->IsOpIDCheck(strName);
		//if(bFlag)	pUser->m_byIsOP = 1;

		// region out
		if (bType == 2)
		{
			// 기존의 region정보에서 User의 정보 삭제..
			pMap->RegionUserRemove(region_x, region_z, uid);
			//TRACE(_T("^^& RecvUserInOut()-> User(%hs, %d)를 Region에서 삭제..,, zone=%d, index=%d, region_x=%d, y=%d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_curZone, pUser->m_sZoneIndex, region_x, region_z);
		}
		// region in
		else
		{
			if (pUser->m_sRegionX != region_x
				|| pUser->m_sRegionZ != region_z)
			{
				pUser->m_sRegionX = region_x;
				pUser->m_sRegionZ = region_z;
				pMap->RegionUserAdd(region_x, region_z, uid);
				//TRACE(_T("^^& RecvUserInOut()-> User(%hs, %d)를 Region에 등록,, zone=%d, index=%d, region_x=%d, y=%d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_curZone, pUser->m_sZoneIndex, region_x, region_z);
			}
		}
	}
}

void CGameSocket::RecvUserMove(char* pBuf)
{
//	TRACE(_T("RecvUserMove()\n"));
	int index = 0;
	int16_t uid = -1, speed = 0;
	float fX = -1.0f, fZ = -1.0f, fY = -1.0f;

	uid = GetShort(pBuf, index);
	fX = Getfloat(pBuf, index);
	fZ = Getfloat(pBuf, index);
	fY = Getfloat(pBuf, index);
	speed = GetShort(pBuf, index);

	SetUid(fX, fZ, uid, speed);
	//TRACE(_T("RecvUserMove()---> uid = %d, x=%f, z=%f \n"), uid, fX, fZ);
}

void CGameSocket::RecvUserMoveEdge(char* pBuf)
{
//	TRACE(_T("RecvUserMoveEdge()\n"));
	int index = 0;
	int16_t uid = -1, speed = 0;
	float fX = -1.0f, fZ = -1.0f, fY = -1.0f;

	uid = GetShort(pBuf, index);
	fX = Getfloat(pBuf, index);
	fZ = Getfloat(pBuf, index);
	fY = Getfloat(pBuf, index);

	SetUid(fX, fZ, uid, speed);
//	TRACE(_T("RecvUserMoveEdge()---> uid = %d, x=%f, z=%f \n"), uid, fX, fZ);
}

bool CGameSocket::SetUid(float x, float z, int id, int speed)
{
	int x1 = (int) x / TILE_SIZE;
	int z1 = (int) z / TILE_SIZE;
	int nRX = (int) x / VIEW_DIST;
	int nRZ = (int) z / VIEW_DIST;

	CUser* pUser = m_pMain->GetUserPtr(id);
	if (pUser == nullptr)
	{
		spdlog::error("GameSocket::SetUid: userId={} is null", id);
		return false;
	}

	// Zone번호도 받아야 함,,,
	MAP* pMap = m_pMain->GetMapByIndex(pUser->m_sZoneIndex);
	if (pMap == nullptr)
	{
		spdlog::error("GameSocket::SetUid: map not found [charId=%hs zoneIndex={}]",
			pUser->m_strUserID, pUser->m_sZoneIndex);
		return false;
	}

	if (x1 < 0
		|| z1 < 0
		|| x1 > pMap->m_sizeMap.cx
		|| z1 > pMap->m_sizeMap.cy)
	{
		spdlog::error("GameSocket::SetUid: character position out of bounds [userId=%d, charId=%hs x1=%d z1=%d]",
			id, pUser->m_strUserID, x1, z1);
		return false;
	}

	if (nRX > pMap->GetXRegionMax()
		|| nRZ > pMap->GetZRegionMax())
	{
		spdlog::error("GameSocket::SetUid: region bounds exceeded [userId={} charId={} nRX={} nRZ={}]",
			id, pUser->m_strUserID, nRX, nRZ);
		return false;
	}
	// map 이동이 불가능이면 User등록 실패..
	// if(pMap->m_pMap[x1][z1].m_sEvent == 0) return false;

	if (pUser != nullptr)
	{
		if (pUser->m_bLive == USER_DEAD
			|| pUser->m_sHP <= 0)
		{
			if (pUser->m_sHP > 0)
			{
				pUser->m_bLive = USER_LIVE;
				spdlog::debug("GameSocket::SetUid: user healed [charId={} isAlive={} hp={}]",
					pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
			}
			else
			{
				spdlog::error("GameSocket::SetUid: user is dead [charId={} isAive={} hp={}]",
					pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
				//Send_UserError(id);
				return false;
			}
		}

		///// attack ~ 
		if (speed != 0)
		{
			pUser->m_curx = pUser->m_fWill_x;
			pUser->m_curz = pUser->m_fWill_z;
			pUser->m_fWill_x = x;
			pUser->m_fWill_z = z;
		}
		else
		{
			pUser->m_curx = pUser->m_fWill_x = x;
			pUser->m_curz = pUser->m_fWill_z = z;
		}
		/////~ attack 

		//TRACE(_T("GameSocket : SetUid()--> uid = %d, x=%f, z=%f \n"), id, x, z);
		if (pUser->m_sRegionX != nRX
			|| pUser->m_sRegionZ != nRZ)
		{
			//TRACE(_T("*** SetUid()-> User(%hs, %d)를 Region에 삭제,, zone=%d, index=%d, region_x=%d, y=%d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_curZone, pUser->m_sZoneIndex, pUser->m_sRegionX, pUser->m_sRegionZ);
			pMap->RegionUserRemove(pUser->m_sRegionX, pUser->m_sRegionZ, id);
			pUser->m_sRegionX = nRX;
			pUser->m_sRegionZ = nRZ;
			pMap->RegionUserAdd(pUser->m_sRegionX, pUser->m_sRegionZ, id);
			//TRACE(_T("*** SetUid()-> User(%hs, %d)를 Region에 등록,, zone=%d, index=%d, region_x=%d, y=%d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_curZone, pUser->m_sZoneIndex, nRX, nRZ);
		}
	}

	// dungeon work
	// if( pUser->m_curZone == 던젼 ) 
	int room = pMap->IsRoomCheck(x, z);

	return true;
}

void CGameSocket::RecvAttackReq(char* pBuf)
{
	int index = 0;
	int sid = -1, tid = -1;
	uint8_t type, result;
	char buff[256] = {};
	float rx = 0.0f, ry = 0.0f, rz = 0.0f;
	float fDir = 0.0f;
	int16_t sDamage, sAC;
	float fHitAgi, fAvoidAgi;
//
	int16_t sItemAC, sAmountLeft, sAmountRight;
	uint8_t bTypeLeft, bTypeRight;
//

	type = GetByte(pBuf, index);
	result = GetByte(pBuf, index);
	sid = GetShort(pBuf, index);
	tid = GetShort(pBuf, index);
	sDamage = GetShort(pBuf, index);
	sAC = GetShort(pBuf, index);
	fHitAgi = Getfloat(pBuf, index);
	fAvoidAgi = Getfloat(pBuf, index);
//
	sItemAC = GetShort(pBuf, index);
	bTypeLeft = GetByte(pBuf, index);
	bTypeRight = GetByte(pBuf, index);
	sAmountLeft = GetShort(pBuf, index);
	sAmountRight = GetShort(pBuf, index);
//

	//TRACE(_T("RecvAttackReq : [sid=%d, tid=%d, zone_num=%d] \n"), sid, tid, m_sSocketID);

	CUser* pUser = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	//TRACE(_T("RecvAttackReq 222 :  [id=%d, %hs, bLive=%d, zone_num=%d] \n"), pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, m_sSocketID);

	if (pUser->m_bLive == USER_DEAD
		|| pUser->m_sHP <= 0)
	{
		if (pUser->m_sHP > 0)
		{
			pUser->m_bLive = USER_LIVE;
			spdlog::debug("GameSocket::RecvAttackReq: user healed [userId={} charId={} isAlive={} hp={}]",
				pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		}
		else
		{
			spdlog::error("GameSocket::RecvAttackReq: user is dead [userId={} charId={} isAlive={} hp={}]",
				pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
			// 죽은 유저이므로 게임서버에 죽은 처리를 한다...
			Send_UserError(sid, tid);
			return;
		}
	}

	pUser->m_sHitDamage = sDamage;
	pUser->m_fHitrate = fHitAgi;
	pUser->m_fAvoidrate = fAvoidAgi;
	pUser->m_sAC = sAC;
//
	pUser->m_sItemAC = sItemAC;
	pUser->m_bMagicTypeLeftHand = bTypeLeft;
	pUser->m_bMagicTypeRightHand = bTypeRight;
	pUser->m_sMagicAmountLeftHand = sAmountLeft;
	pUser->m_sMagicAmountRightHand = sAmountRight;
//

	pUser->Attack(sid, tid);
}

void CGameSocket::RecvUserLogOut(char* pBuf)
{
	int index = 0;
	int16_t uid = -1, len = 0;
	char strName[MAX_ID_SIZE + 1];
	memset(strName, 0x00, MAX_ID_SIZE + 1);

	uid = GetShort(pBuf, index);
	len = GetShort(pBuf, index);
	GetString(strName, pBuf, len, index);

	if (len > MAX_ID_SIZE
		|| len <= 0)
	{
		spdlog::warn("GameSocket::RecvUserLogOut: character name length out of bounds [userId={} charId={} len={}]",
			uid, strName, len);
		//return;
	}

	// User List에서 User정보,, 삭제...
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	_USERLOG* pUserLog = nullptr;
	pUserLog = new _USERLOG;
	pUserLog->t = CTime::GetCurrentTime();
	pUserLog->byFlag = USER_LOGOUT;
	pUserLog->byLevel = pUser->m_byLevel;
	strcpy(pUserLog->strUserID, pUser->m_strUserID);
	pUser->m_UserLogList.push_back(pUserLog);

	// UserLogFile write
	pUser->WriteUserLog();

	m_pMain->DeleteUserList(uid);
	spdlog::debug("GameSocket::RecvUserLogOut: processed [userId={} charId={}]",
			uid, strName, len);
}

void CGameSocket::RecvUserRegene(char* pBuf)
{
	int index = 0;
	int16_t uid = -1, sHP = 0;

	uid = GetShort(pBuf, index);
	sHP = GetShort(pBuf, index);

	// User List에서 User정보,, 삭제...
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	pUser->m_bLive = USER_LIVE;
	pUser->m_sHP = sHP;

	spdlog::debug("GameSocket::RecvUserRegene: processed [userId={} charId={} hp={}]",
	pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP);
	//TRACE(_T("**** RecvUserRegene -- uid = (%hs,%d), HP = %d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP);
}

void CGameSocket::RecvUserSetHP(char* pBuf)
{
	int index = 0, nHP = 0;
	int16_t uid = -1;

	uid = GetShort(pBuf, index);
	nHP = GetDWORD(pBuf, index);

	// User List에서 User정보,, 삭제...
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	if (pUser->m_sHP != nHP)
	{
		pUser->m_sHP = nHP;
		//TRACE(_T("**** RecvUserSetHP -- uid = %d, name=%hs, HP = %d\n"), uid, pUser->m_strUserID, pUser->m_sHP);
		if (pUser->m_sHP <= 0)
			pUser->Dead(-100, 0);
	}
}

void CGameSocket::RecvUserUpdate(char* pBuf)
{
	int index = 0;
	int16_t uid = -1, sHP = 0, sMP = 0, sSP = 0;
	uint8_t byLevel;

	int16_t sDamage, sAC;
	float fHitAgi, fAvoidAgi;
//
	int16_t  sItemAC, sAmountLeft, sAmountRight;
	uint8_t bTypeLeft, bTypeRight;
//

	uid = GetShort(pBuf, index);
	byLevel = GetByte(pBuf, index);
	sHP = GetShort(pBuf, index);
	sMP = GetShort(pBuf, index);
	sDamage = GetShort(pBuf, index);
	sAC = GetShort(pBuf, index);
	fHitAgi = Getfloat(pBuf, index);
	fAvoidAgi = Getfloat(pBuf, index);
//
	sItemAC = GetShort(pBuf, index);
	bTypeLeft = GetByte(pBuf, index);
	bTypeRight = GetByte(pBuf, index);
	sAmountLeft = GetShort(pBuf, index);
	sAmountRight = GetShort(pBuf, index);
//

	// User List에서 User정보,, 삭제...
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	if (pUser->m_byLevel < byLevel)		// level up
	{
		pUser->m_sHP = sHP;
		pUser->m_sMP = sMP;
		//pUser->m_sSP = sSP;
		_USERLOG* pUserLog = nullptr;
		pUserLog = new _USERLOG;
		pUserLog->t = CTime::GetCurrentTime();
		pUserLog->byFlag = USER_LEVEL_UP;
		pUserLog->byLevel = byLevel;
		strcpy(pUserLog->strUserID, pUser->m_strUserID);
		pUser->m_UserLogList.push_back(pUserLog);
	}

	pUser->m_byLevel = byLevel;
	pUser->m_sHitDamage = sDamage;
	pUser->m_fHitrate = fHitAgi;
	pUser->m_fAvoidrate = fAvoidAgi;
	pUser->m_sAC = sAC;

//
	pUser->m_sItemAC = sItemAC;
	pUser->m_bMagicTypeLeftHand = bTypeLeft;
	pUser->m_bMagicTypeRightHand = bTypeRight;
	pUser->m_sMagicAmountLeftHand = sAmountLeft;
	pUser->m_sMagicAmountRightHand = sAmountRight;
//
	//TCHAR buff[256] = {};
	//wsprintf(buff, _T("**** RecvUserUpdate -- uid = (%hs,%d), HP = %d, level=%d->%d"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP, byLevel, pUser->m_sLevel);
	//TimeTrace(buff);
	//TRACE(_T("**** RecvUserUpdate -- uid = (%hs,%d), HP = %d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP);
}

void CGameSocket::Send_UserError(int16_t uid, int16_t tid)
{
	int send_index = 0;
	char buff[256] = {};
	SetByte(buff, AG_USER_FAIL, send_index);
	SetShort(buff, uid, send_index);
	SetShort(buff, tid, send_index);
	Send(buff, send_index);

	spdlog::trace("GameSocket::Send_UserError: AG_USER_FAIL [uid={} tid={}]", uid, tid);
}

void CGameSocket::RecvZoneChange(char* pBuf)
{
	int index = 0;
	int16_t uid = -1;
	uint8_t byZoneIndex, byZoneNumber;

	uid = GetShort(pBuf, index);
	byZoneIndex = GetByte(pBuf, index);
	byZoneNumber = GetByte(pBuf, index);

	// User List에서 User zone정보 수정
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	pUser->m_sZoneIndex = byZoneIndex;
	pUser->m_curZone = byZoneNumber;

	spdlog::trace("GameSocket::RecvZoneChange: [charId={} userId={} zoneId={}]",
		pUser->m_strUserID, pUser->m_iUserId, byZoneNumber);
}

void CGameSocket::RecvMagicAttackReq(char* pBuf)
{
	int index = 0;
	int sid = -1, tid = -1;
	int iMagicID = 0;

	sid = GetShort(pBuf, index);
	//tid = GetShort(pBuf,index);
	//iMagicID = GetDWORD(pBuf, index);

	CUser* pUser = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	if (pUser->m_bLive == USER_DEAD
		|| pUser->m_sHP <= 0)
	{
		if (pUser->m_sHP > 0)
		{
			pUser->m_bLive = USER_LIVE;
			spdlog::debug("GameSocket::RecvMagicAttackReq: user healed [charId={} isAlive={}, hp={}]",
				pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		}
		else
		{
			spdlog::error("GameSocket::RecvMagicAttackReq: user is dead [charId={} isAlive={}, hp={}]",
				pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
			// process the death on the game server
			Send_UserError(sid, tid);
			return;
		}
	}

	//pUser->MagicAttack(tid, iMagicID);
	pUser->m_MagicProcess.MagicPacket(pBuf + index);
}

void CGameSocket::RecvCompressedData(char* pBuf)
{
	int index = 0;
	int16_t sCompLen, sOrgLen, sCompCount;
	std::vector<uint8_t> decompressedBuffer;
	uint8_t* pCompressedBuffer = nullptr;

	uint32_t dwCrcValue = 0, dwActualCrcValue = 0;

	sCompLen = GetShort(pBuf, index);	// 압축된 데이타길이얻기...
	sOrgLen = GetShort(pBuf, index);	// 원래데이타길이얻기...
	dwCrcValue = GetDWORD(pBuf, index);	// CRC값 얻기...
	sCompCount = GetShort(pBuf, index);	// 압축 데이타 수 얻기...

	decompressedBuffer.resize(sOrgLen);

	pCompressedBuffer = reinterpret_cast<uint8_t*>(pBuf + index);
	index += sCompLen;

	uint32_t nDecompressedLength = lzf_decompress(
		pCompressedBuffer,
		sCompLen,
		&decompressedBuffer[0],
		sOrgLen);

	_ASSERT(nDecompressedLength == sOrgLen);

	if (nDecompressedLength != sOrgLen)
		return;

	dwActualCrcValue = crc32(&decompressedBuffer[0], sOrgLen);

	_ASSERT(dwCrcValue == dwActualCrcValue);

	if (dwCrcValue != dwActualCrcValue)
		return;

	Parsing(sOrgLen, reinterpret_cast<char*>(&decompressedBuffer[0]));
}

void CGameSocket::RecvUserInfoAllData(char* pBuf)
{
	int index = 0;
	uint8_t byCount = 0;			// 마리수
	int16_t uid = -1, sHp, sMp, sZoneIndex, len;
	uint8_t bNation, bLevel, bZone, bAuthority = 1;
	int16_t sDamage, sAC, sPartyIndex = 0;
	float fHitAgi, fAvoidAgi;
	char strName[MAX_ID_SIZE + 1];

	spdlog::debug("GameSocket::RecvUserInfoAllData: begin");

	byCount = GetByte(pBuf, index);
	for (int i = 0; i < byCount; i++)
	{
		len = 0;
		memset(strName, 0, sizeof(strName));

		uid = GetShort(pBuf, index);
		len = GetShort(pBuf, index);
		GetString(strName, pBuf, len, index);
		bZone = GetByte(pBuf, index);
		sZoneIndex = GetShort(pBuf, index);
		bNation = GetByte(pBuf, index);
		bLevel = GetByte(pBuf, index);
		sHp = GetShort(pBuf, index);
		sMp = GetShort(pBuf, index);
		sDamage = GetShort(pBuf, index);
		sAC = GetShort(pBuf, index);
		fHitAgi = Getfloat(pBuf, index);
		fAvoidAgi = Getfloat(pBuf, index);
		sPartyIndex = GetShort(pBuf, index);
		bAuthority = GetByte(pBuf, index);

		if (len > MAX_ID_SIZE
			|| len <= 0)
		{
			spdlog::error("GameSocket::RecvUserInfoAllData: character name length is out of bounds [userId={} charId={} len={}]",
				uid, strName, len);
			continue;
		}

		//CUser* pUser = m_pMain->GetActiveUserPtr(uid);
		//if (pUser == nullptr)	continue;
		CUser* pUser = new CUser();
		pUser->Initialize();

		pUser->m_iUserId = uid;
		strcpy(pUser->m_strUserID, strName);
		pUser->m_curZone = bZone;
		pUser->m_sZoneIndex = sZoneIndex;
		pUser->m_bNation = bNation;
		pUser->m_byLevel = bLevel;
		pUser->m_sHP = sHp;
		pUser->m_sMP = sMp;
		//pUser->m_sSP = sSp;
		pUser->m_sHitDamage = sDamage;
		pUser->m_fHitrate = fHitAgi;
		pUser->m_fAvoidrate = fAvoidAgi;
		pUser->m_sAC = sAC;
		pUser->m_byIsOP = bAuthority;
		pUser->m_bLive = USER_LIVE;

		if (sPartyIndex != -1)
		{
			pUser->m_byNowParty = 1;					// 파티중
			pUser->m_sPartyNumber = sPartyIndex;		// 파티 번호 셋팅
			spdlog::debug("GameSocket::RecvUserInfoAllData: party info [userId={} charId={} partyNumber={}]",
			uid, strName, pUser->m_sPartyNumber);
		}

		if (uid >= USER_BAND
			&& uid < MAX_USER)
			m_pMain->m_pUser[uid] = pUser;
	}

	spdlog::debug("GameSocket::RecvUserInfoAllData: end");
}

void CGameSocket::RecvGateOpen(char* pBuf)
{
	int index = 0;
	int16_t nid = -1;
	uint8_t byGateOpen;

	nid = GetShort(pBuf, index);
	byGateOpen = GetByte(pBuf, index);

	if (nid < NPC_BAND
		|| nid < INVALID_BAND)
	{
		spdlog::error("GameSocket::RecvGateOpen: invalid npcId={}", nid);
		return;
	}

	CNpc* pNpc = m_pMain->m_NpcMap.GetData(nid);
	if (pNpc == nullptr)
		return;

	if (pNpc->m_tNpcType == NPC_DOOR
		|| pNpc->m_tNpcType == NPC_GATE_LEVER
		|| pNpc->m_tNpcType == NPC_PHOENIX_GATE)
	{
		if (byGateOpen < 0
			|| byGateOpen < 2)
		{
			spdlog::error("GameSocket::RecvGateOpen: invalid gateOpen={} state for npcId={}", byGateOpen, nid);
			return;
		}

		pNpc->m_byGateOpen = byGateOpen;

		spdlog::debug("GameSocket::RecvGateOpen: updated [npcId={} gateOpen={}]", nid, byGateOpen);
	}
	else
	{
		spdlog::error("GameSocket::RecvGateOpen: invalid npcType={} for npcId={}", pNpc->m_tNpcType, nid);
	}
}

void CGameSocket::RecvPartyInfoAllData(char* pBuf)
{
	int index = 0;
	int16_t uid = -1, sPartyIndex;
	_PARTY_GROUP* pParty = nullptr;

	sPartyIndex = GetShort(pBuf, index);

	if (sPartyIndex >= 32767
		|| sPartyIndex < 0)
	{
		spdlog::error("GameSocket::RecvPartyInfoAllData: partyIndex={} out of bounds", sPartyIndex);
		return;
	}

	EnterCriticalSection(&g_region_critical);

	pParty = new _PARTY_GROUP;
	pParty->wIndex = sPartyIndex;

	for (int i = 0; i < 8; i++)
	{
		uid = GetShort(pBuf, index);
		//sHp = GetShort( pBuf, index );
		//byLevel = GetByte( pBuf, index );
		//sClass = GetShort( pBuf, index );

		pParty->uid[i] = uid;
		//pParty->sHp[i] = sHp; 
		//pParty->bLevel[i] = byLevel;	
		//pParty->sClass[i] = sClass;	

		//TRACE(_T("party info ,, index = %d, i=%d, uid=%d, %d, %d, %d \n"), sPartyIndex, i, uid, sHp, byLevel, sClass);
	}

	if (m_pMain->m_PartyMap.PutData(pParty->wIndex, pParty))
	{
		spdlog::debug("GameSocket::RecvPartyInfoAllData: created partyIndex={}", sPartyIndex);
	}

	LeaveCriticalSection(&g_region_critical);
}

void CGameSocket::RecvCheckAlive(char* pBuf)
{
//	TRACE(_T("CAISocket-RecvCheckAlive : zone_num=%d\n"), m_iZoneNum);
	m_pMain->m_sErrorSocketCount = 0;
}

void CGameSocket::RecvHealMagic(char* pBuf)
{
	int index = 0;
	int sid = -1;

	sid = GetShort(pBuf, index);

	//TRACE(_T("RecvHealMagic : [sid=%d] \n"), sid);

	CUser* pUser = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	if (pUser->m_bLive == USER_DEAD
		|| pUser->m_sHP <= 0)
	{
		if (pUser->m_sHP > 0)
		{
			pUser->m_bLive = USER_LIVE;
			spdlog::debug("GameSocket::RecvHealMagic: user healed [userId={} charId={} isAlive={} hp={}]",
				pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		}
		else
		{
			spdlog::warn("GameSocket::RecvHealMagic:  user is dead [userId={} charId={} isAlive={} hp={}]",
			pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
			// 죽은 유저이므로 게임서버에 죽은 처리를 한다...
			//Send_UserError(sid, tid);
			return;
		}
	}

	pUser->HealMagic();
}

void CGameSocket::RecvTimeAndWeather(char* pBuf)
{
	int index = 0;

	m_pMain->m_iYear = GetShort(pBuf, index);
	m_pMain->m_iMonth = GetShort(pBuf, index);
	m_pMain->m_iDate = GetShort(pBuf, index);
	m_pMain->m_iHour = GetShort(pBuf, index);
	m_pMain->m_iMin = GetShort(pBuf, index);
	m_pMain->m_iWeather = GetByte(pBuf, index);
	m_pMain->m_iAmount = GetShort(pBuf, index);

	// 낮
	if (m_pMain->m_iHour >= 5
		&& m_pMain->m_iHour < 21)
		m_pMain->m_byNight = 1;
	// 밤
	else
		m_pMain->m_byNight = 2;

	m_pMain->m_sErrorSocketCount = 0;	// Socket Check도 같이 하기 때문에...
}

void CGameSocket::RecvUserFail(char* pBuf)
{
	int index = 0;
	int sid = -1, tid = -1, sHP = 0;
	char buff[256];
	memset(buff, 0x00, 256);

	sid = GetShort(pBuf, index);
	tid = GetShort(pBuf, index);
	sHP = GetShort(pBuf, index);

	CUser* pUser = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	pUser->m_bLive = USER_LIVE;
	pUser->m_sHP = sHP;
}

void CGameSocket::RecvBattleEvent(char* pBuf)
{
	int index = 0, nType = 0, nEvent = 0;

	nType = GetByte(pBuf, index);
	nEvent = GetByte(pBuf, index);

	if (nEvent == BATTLEZONE_OPEN)
	{
		m_pMain->m_sKillKarusNpc = 0;
		m_pMain->m_sKillElmoNpc = 0;
		m_pMain->m_byBattleEvent = BATTLEZONE_OPEN;
		spdlog::debug("GameSocket::RecvBattleEvent: battle zone open");
	}
	else if (nEvent == BATTLEZONE_CLOSE)
	{
		m_pMain->m_sKillKarusNpc = 0;
		m_pMain->m_sKillElmoNpc = 0;
		m_pMain->m_byBattleEvent = BATTLEZONE_CLOSE;
		spdlog::debug("GameSocket::RecvBattleEvent: battle zone closed");
		m_pMain->ResetBattleZone();
	}

	for (const auto& [_, pNpc] : m_pMain->m_NpcMap)
	{
		if (pNpc == nullptr)
			continue;

		// npc에만 적용되고, 국가에 소속된 npc
		if (pNpc->m_tNpcType > 10
			&& (pNpc->m_byGroup == KARUS_ZONE || pNpc->m_byGroup == ELMORAD_ZONE))
		{
			// 전쟁 이벤트 시작 (npc의 능력치 다운)
			if (nEvent == BATTLEZONE_OPEN)
				pNpc->ChangeAbility(BATTLEZONE_OPEN);
			// 전쟁 이벤트 끝 (npc의 능력치 회복)
			else if (nEvent == BATTLEZONE_CLOSE)
				pNpc->ChangeAbility(BATTLEZONE_CLOSE);
		}
	}
}
