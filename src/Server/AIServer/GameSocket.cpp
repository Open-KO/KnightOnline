// GameSocket.cpp: implementation of the CGameSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "GameSocket.h"
#include "Extern.h"
#include "Map.h"
#include "NpcThread.h"
#include "Party.h"
#include "Region.h"
#include "AIServerApp.h"
#include "User.h"

#include <shared/crc32.h>
#include <shared/lzf.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

namespace AIServer
{

extern std::mutex g_region_mutex;

/*
	 ** Repent AI Server 작업시 참고 사항 **
	1. RecvUserInfo(), RecvAttackReq(), RecvUserUpdate() 수정
*/

CGameSocket::CGameSocket(TcpServerSocketManager* socketManager) : TcpServerSocket(socketManager)
{
	m_pMain = AIServerApp::instance();
}

CGameSocket::~CGameSocket()
{
	/*
	delete m_pParty;
	m_pParty = nullptr;
	*/
}

std::string_view CGameSocket::GetImplName() const
{
	return "GameSocket";
}

void CGameSocket::Initialize()
{
	_zoneNo = -1;

	TcpServerSocket::Initialize();
}

bool CGameSocket::PullOutCore(char*& data, int& length)
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

			// ASSERT(ePos+2 <= len);

			if (tmpBuffer[ePos] != PACKET_END1 || tmpBuffer[ePos + 1] != PACKET_END2)
			{
				_recvCircularBuffer.HeadIncrease(3);
				return false;
			}

			data = new char[length];
			memcpy(data, &tmpBuffer[sPos + 2], length);
			_recvCircularBuffer.HeadIncrease(6 + length); // 6: header 2+ end 2+ length 2
			return true;
		}
	}

	return false;
}

int CGameSocket::Send(char* pBuf, int length)
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

void CGameSocket::CloseProcess()
{
	spdlog::info("GameSocket::CloseProcess: zoneNo={} socketId={}", _zoneNo, _socketId);
	m_pMain->DeleteAllUserList(_zoneNo);
	Initialize();
	TcpServerSocket::CloseProcess();
}

void CGameSocket::Parsing(int /*length*/, char* pData)
{
	int index     = 0;
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
			RecvCheckAlive();
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

		default:
			spdlog::error("GameSocket::Parsing: Unhandled opcode {:02X}", bType);
			break;
	}
}

// sungyong 2002.05.22
void CGameSocket::RecvServerConnect(char* pBuf)
{
	int index = 1, outindex = 0;
	double fReConnectEndTime = 0.0;
	char pData[1024] {};

	uint8_t byZoneNumber = GetByte(pBuf, index);
	uint8_t byReConnect  = GetByte(pBuf, index); // 0 : 처음접속, 1 : 재접속

	spdlog::info("GameSocket::RecvServerConnect: Ebenezer connected to zone={}", byZoneNumber);

	if (byZoneNumber < 0)
	{
		SetByte(pData, AI_SERVER_CONNECT, outindex);
		SetByte(pData, -1, outindex);
		Send(pData, outindex);
	}

	_zoneNo = byZoneNumber;

	SetByte(pData, AI_SERVER_CONNECT, outindex);
	SetByte(pData, byZoneNumber, outindex);
	SetByte(pData, byReConnect, outindex);
	Send(pData, outindex);

	// 재접속해서 리스트 받기 (강제로)
	if (byReConnect == 1)
	{
		if (m_pMain->_reconnectSocketCount == 0)
			m_pMain->_reconnectStartTime = TimeGet();

		m_pMain->_reconnectSocketCount++;

		spdlog::info("GameSocket::RecvServerConnect: Ebenezer reconnect: zone={} sockets={}",
			byZoneNumber, m_pMain->_reconnectSocketCount);

		fReConnectEndTime = TimeGet();

		// all sockets reconnected within 2 minutes
		if (fReConnectEndTime > m_pMain->_reconnectStartTime + 120)
		{
			spdlog::info("GameSocket::RecvServerConnect: Ebenezer sockets reconnected in under 2 "
						 "minutes [sockets={}]",
				m_pMain->_reconnectSocketCount);
			m_pMain->_reconnectSocketCount = 0;
			m_pMain->_reconnectStartTime   = 0.0;
		}

		if (m_pMain->_reconnectSocketCount == MAX_AI_SOCKET)
		{
			fReConnectEndTime = TimeGet();

			// all sockets reconnected within 1 minute
			if (fReConnectEndTime < m_pMain->_reconnectStartTime + 60)
			{
				spdlog::info("GameSocket::RecvServerConnect: Ebenezer sockets reconnected within a "
							 "minute [sockets={}]",
					m_pMain->_reconnectSocketCount);
				m_pMain->_firstServerFlag      = true;
				m_pMain->_reconnectSocketCount = 0;
				m_pMain->AllNpcInfo();
			}
			// 하나의 떨어진 소켓이라면...
			else
			{
				m_pMain->_reconnectSocketCount = 0;
				m_pMain->_reconnectStartTime   = 0.0;
			}
		}
	}
	else
	{
		m_pMain->_socketCount++;
		spdlog::info("GameSocket::RecvServerConnect: Ebenezer connected [zone={}, sockets={}]",
			byZoneNumber, m_pMain->_socketCount);
		if (m_pMain->_socketCount == MAX_AI_SOCKET)
		{
			spdlog::info(
				"GameSocket::RecvServerConnect: Ebenezer sockets all connected [sockets={}]",
				m_pMain->_socketCount);
			m_pMain->_firstServerFlag = true;
			m_pMain->_socketCount     = 0;
			m_pMain->AllNpcInfo();
		}
	}
}
// ~sungyong 2002.05.22

void CGameSocket::RecvUserInfo(char* pBuf)
{
	//	TRACE(_T("RecvUserInfo()\n"));
	int index   = 0;
	int16_t uid = -1, sHp = 0, sMp = 0, sZoneIndex = 0, sLength = 0;
	uint8_t bNation = 0, bLevel = 0, bZone = 0, bAuthority = 1;
	int16_t sItemAC = 0, sAmountLeft = 0, sAmountRight = 0;
	uint8_t bTypeLeft = 0, bTypeRight = 0;
	int16_t sDamage = 0, sAC = 0;
	float fHitAgi = 0.0f, fAvoidAgi = 0.0f;
	char strName[MAX_ID_SIZE + 1] {};

	uid     = GetShort(pBuf, index);
	sLength = GetShort(pBuf, index);
	if (sLength > MAX_ID_SIZE || sLength <= 0)
	{
		spdlog::error(
			"GameSocket::RecvUserInfo: charId len={} overflow for userId={}", sLength, uid);
		return;
	}

	GetString(strName, pBuf, sLength, index);
	bZone        = GetByte(pBuf, index);
	sZoneIndex   = GetShort(pBuf, index);
	bNation      = GetByte(pBuf, index);
	bLevel       = GetByte(pBuf, index);
	sHp          = GetShort(pBuf, index);
	sMp          = GetShort(pBuf, index);
	sDamage      = GetShort(pBuf, index);
	sAC          = GetShort(pBuf, index);
	fHitAgi      = GetFloat(pBuf, index);
	fAvoidAgi    = GetFloat(pBuf, index);
	//
	sItemAC      = GetShort(pBuf, index);
	bTypeLeft    = GetByte(pBuf, index);
	bTypeRight   = GetByte(pBuf, index);
	sAmountLeft  = GetShort(pBuf, index);
	sAmountRight = GetShort(pBuf, index);
	bAuthority   = GetByte(pBuf, index);
	//

	//CUser* pUser = m_pMain->GetActiveUserPtr(uid);
	//if( pUser == nullptr )		return;
	CUser* pUser = new CUser();
	pUser->Initialize();

	pUser->m_iUserId = uid;
	strcpy_safe(pUser->m_strUserID, strName);
	pUser->m_curZone               = bZone;
	pUser->m_sZoneIndex            = sZoneIndex;
	pUser->m_bNation               = bNation;
	pUser->m_byLevel               = bLevel;
	pUser->m_sHP                   = sHp;
	pUser->m_sMP                   = sMp;
	//pUser->m_sSP = sSp;
	pUser->m_sHitDamage            = sDamage;
	pUser->m_fHitrate              = fHitAgi;
	pUser->m_fAvoidrate            = fAvoidAgi;
	pUser->m_sAC                   = sAC;
	pUser->m_bLive                 = USER_LIVE;
	//
	pUser->m_sItemAC               = sItemAC;
	pUser->m_bMagicTypeLeftHand    = bTypeLeft;
	pUser->m_bMagicTypeRightHand   = bTypeRight;
	pUser->m_sMagicAmountLeftHand  = sAmountLeft;
	pUser->m_sMagicAmountRightHand = sAmountRight;
	pUser->m_byIsOP                = bAuthority;
	//

	spdlog::debug("GameSocket::RecvUserInfo: userId={} charId={}", uid, strName);

	if (uid >= USER_BAND && uid < MAX_USER)
		m_pMain->_users[uid] = pUser;

	m_pMain->userLogger()->info("Login: level={}, charId={}", pUser->m_byLevel, pUser->m_strUserID);
}

void CGameSocket::RecvUserInOut(char* pBuf)
{
	int index     = 0;
	uint8_t bType = -1;
	int16_t uid = -1, len = 0;
	char strName[MAX_ID_SIZE + 1] {};
	float fX = -1, fZ = -1;

	bType = GetByte(pBuf, index);
	uid   = GetShort(pBuf, index);
	len   = GetShort(pBuf, index);
	GetString(strName, pBuf, len, index);
	fX = GetFloat(pBuf, index);
	fZ = GetFloat(pBuf, index);

	if (fX < 0 || fZ < 0)
	{
		spdlog::error("RecvUserInOut: invalid position charId={} fX={} fZ={}", strName, fX, fZ);
		return;
	}

	//	TRACE(_T("RecvUserInOut(),, uid = %d\n"), uid);

	int region_x = 0, region_z = 0;
	int x1       = (int) fX / TILE_SIZE;
	int z1       = (int) fZ / TILE_SIZE;
	region_x     = (int) fX / VIEW_DIST;
	region_z     = (int) fZ / VIEW_DIST;

	// 수정할것,,, : 지금 존 번호를 0으로 했는데.. 유저의 존 정보의 번호를 읽어야,, 함,,
	MAP* pMap    = nullptr;

	CUser* pUser = m_pMain->GetUserPtr(uid);

	//	TRACE(_T("^^& RecvUserInOut( type=%d )-> User(%hs, %d),, zone=%d, index=%d, region_x=%d, y=%d\n"), bType, pUser->m_strUserID, pUser->m_iUserId, pUser->m_curZone, pUser->m_sZoneIndex, region_x, region_z);

	if (pUser != nullptr)
	{
		//	TRACE(_T("##### Fail : ^^& RecvUserInOut() [name = %hs]. state=%d, hp=%d\n"), pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		if (pUser->m_bLive == USER_DEAD || pUser->m_sHP <= 0)
		{
			if (pUser->m_sHP > 0)
			{
				pUser->m_bLive = 1;
			}

			spdlog::warn(
				"GameSocket::RecvUserInOut: UserHeal error[charId={} isAlive={} hp={} fX={} fZ={}]",
				pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP, fX, fZ);
		}

		pMap = m_pMain->GetMapByIndex(pUser->m_sZoneIndex);
		if (pMap == nullptr)
		{
			spdlog::error(
				"GameSocket::RecvUserInOut: Map not found for zoneIndex={} [charId={} x1={} z1={}]",
				pUser->m_sZoneIndex, pUser->m_strUserID, x1, z1);
			return;
		}

		if (x1 < 0 || z1 < 0 || x1 > pMap->m_sizeMap.cx || z1 > pMap->m_sizeMap.cy)
		{
			spdlog::error("GameSocket::RecvUserInOut: Character position out of bounds [charId={} "
						  "x1=%d, z1=%d]",
				pUser->m_strUserID, x1, z1);
			return;
		}
		// map 이동이 불가능이면 User등록 실패..
		//if(pMap->m_pMap[x1][z1].m_sEvent == 0) return;
		if (region_x > pMap->GetXRegionMax() || region_z > pMap->GetZRegionMax())
		{
			spdlog::error(
				"GameSocket::RecvUserInOut: region out of bounds [charId={} nRX={} nRZ={}]",
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
			if (pUser->m_sRegionX != region_x || pUser->m_sRegionZ != region_z)
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
	int index   = 0;
	int16_t uid = GetShort(pBuf, index);
	float fX    = GetFloat(pBuf, index);
	float fZ    = GetFloat(pBuf, index);
	/*float fY =*/GetFloat(pBuf, index);
	int16_t speed = GetShort(pBuf, index);

	SetUid(fX, fZ, uid, speed);
	//TRACE(_T("RecvUserMove()---> uid = %d, x=%f, z=%f \n"), uid, fX, fZ);
}

void CGameSocket::RecvUserMoveEdge(char* pBuf)
{
	//	TRACE(_T("RecvUserMoveEdge()\n"));
	int index   = 0;
	int16_t uid = GetShort(pBuf, index);
	float fX    = GetFloat(pBuf, index);
	float fZ    = GetFloat(pBuf, index);
	/*float fY =*/GetFloat(pBuf, index);
	int16_t speed = 0;

	SetUid(fX, fZ, uid, speed);
	//	TRACE(_T("RecvUserMoveEdge()---> uid = %d, x=%f, z=%f \n"), uid, fX, fZ);
}

bool CGameSocket::SetUid(float x, float z, int id, int speed)
{
	int x1       = (int) x / TILE_SIZE;
	int z1       = (int) z / TILE_SIZE;
	int nRX      = (int) x / VIEW_DIST;
	int nRZ      = (int) z / VIEW_DIST;

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

	if (x1 < 0 || z1 < 0 || x1 > pMap->m_sizeMap.cx || z1 > pMap->m_sizeMap.cy)
	{
		spdlog::error("GameSocket::SetUid: character position out of bounds [userId=%d, charId=%hs "
					  "x1=%d z1=%d]",
			id, pUser->m_strUserID, x1, z1);
		return false;
	}

	if (nRX > pMap->GetXRegionMax() || nRZ > pMap->GetZRegionMax())
	{
		spdlog::error(
			"GameSocket::SetUid: region bounds exceeded [userId={} charId={} nRX={} nRZ={}]", id,
			pUser->m_strUserID, nRX, nRZ);
		return false;
	}
	// map 이동이 불가능이면 User등록 실패..
	// if(pMap->m_pMap[x1][z1].m_sEvent == 0) return false;

	if (pUser != nullptr)
	{
		if (pUser->m_bLive == USER_DEAD || pUser->m_sHP <= 0)
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
			pUser->m_curx    = pUser->m_fWill_x;
			pUser->m_curz    = pUser->m_fWill_z;
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
		if (pUser->m_sRegionX != nRX || pUser->m_sRegionZ != nRZ)
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
	/*int room =*/pMap->IsRoomCheck(x, z);

	return true;
}

void CGameSocket::RecvAttackReq(char* pBuf)
{
	int index = 0;

	/*uint8_t type =*/GetByte(pBuf, index);
	/*uint8_t result =*/GetByte(pBuf, index);
	int sid              = GetShort(pBuf, index);
	int tid              = GetShort(pBuf, index);
	int16_t sDamage      = GetShort(pBuf, index);
	int16_t sAC          = GetShort(pBuf, index);
	float fHitAgi        = GetFloat(pBuf, index);
	float fAvoidAgi      = GetFloat(pBuf, index);
	//
	int16_t sItemAC      = GetShort(pBuf, index);
	uint8_t bTypeLeft    = GetByte(pBuf, index);
	uint8_t bTypeRight   = GetByte(pBuf, index);
	int16_t sAmountLeft  = GetShort(pBuf, index);
	int16_t sAmountRight = GetShort(pBuf, index);
	//

	//TRACE(_T("RecvAttackReq : [sid=%d, tid=%d, zone_num=%d] \n"), sid, tid, _zoneNo);

	CUser* pUser         = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	//TRACE(_T("RecvAttackReq 222 :  [id=%d, %hs, bLive=%d, zone_num=%d] \n"), pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, m_sSocketID);

	if (pUser->m_bLive == USER_DEAD || pUser->m_sHP <= 0)
	{
		if (pUser->m_sHP > 0)
		{
			pUser->m_bLive = USER_LIVE;
			spdlog::debug(
				"GameSocket::RecvAttackReq: user healed [userId={} charId={} isAlive={} hp={}]",
				pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		}
		else
		{
			spdlog::error(
				"GameSocket::RecvAttackReq: user is dead [userId={} charId={} isAlive={} hp={}]",
				pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
			// 죽은 유저이므로 게임서버에 죽은 처리를 한다...
			Send_UserError(sid, tid);
			return;
		}
	}

	pUser->m_sHitDamage            = sDamage;
	pUser->m_fHitrate              = fHitAgi;
	pUser->m_fAvoidrate            = fAvoidAgi;
	pUser->m_sAC                   = sAC;
	//
	pUser->m_sItemAC               = sItemAC;
	pUser->m_bMagicTypeLeftHand    = bTypeLeft;
	pUser->m_bMagicTypeRightHand   = bTypeRight;
	pUser->m_sMagicAmountLeftHand  = sAmountLeft;
	pUser->m_sMagicAmountRightHand = sAmountRight;
	//

	pUser->Attack(sid, tid);
}

void CGameSocket::RecvUserLogOut(char* pBuf)
{
	int index   = 0;
	int16_t uid = -1, len = 0;
	char strName[MAX_ID_SIZE + 1];
	memset(strName, 0x00, MAX_ID_SIZE + 1);

	uid = GetShort(pBuf, index);
	len = GetShort(pBuf, index);
	GetString(strName, pBuf, len, index);

	if (len > MAX_ID_SIZE || len <= 0)
	{
		spdlog::warn("GameSocket::RecvUserLogOut: character name length out of bounds [userId={} "
					 "charId={} len={}]",
			uid, strName, len);
		//return;
	}

	// User List에서 User정보,, 삭제...
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	// UserLogFile write
	m_pMain->userLogger()->info(
		"Logout: level={}, charId={}", pUser->m_byLevel, pUser->m_strUserID);

	m_pMain->DeleteUserList(uid);
	spdlog::debug("GameSocket::RecvUserLogOut: processed [userId={} charId={}]", uid, strName, len);
}

void CGameSocket::RecvUserRegene(char* pBuf)
{
	int index   = 0;
	int16_t uid = -1, sHP = 0;

	uid          = GetShort(pBuf, index);
	sHP          = GetShort(pBuf, index);

	// User List에서 User정보,, 삭제...
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	pUser->m_bLive = USER_LIVE;
	pUser->m_sHP   = sHP;

	spdlog::debug("GameSocket::RecvUserRegene: processed [userId={} charId={} hp={}]",
		pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP);
	//TRACE(_T("**** RecvUserRegene -- uid = (%hs,%d), HP = %d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP);
}

void CGameSocket::RecvUserSetHP(char* pBuf)
{
	int index = 0, nHP = 0;
	int16_t uid  = -1;

	uid          = GetShort(pBuf, index);
	nHP          = GetDWORD(pBuf, index);

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
	int index   = 0;
	int16_t uid = -1, sHP = 0, sMP = 0, sDamage = 0, sAC = 0;
	int16_t sItemAC = 0, sAmountLeft = 0, sAmountRight = 0;
	uint8_t byLevel = 0, bTypeLeft = 0, bTypeRight = 0;
	float fHitAgi = 0.0f, fAvoidAgi = 0.0f;

	uid          = GetShort(pBuf, index);
	byLevel      = GetByte(pBuf, index);
	sHP          = GetShort(pBuf, index);
	sMP          = GetShort(pBuf, index);
	sDamage      = GetShort(pBuf, index);
	sAC          = GetShort(pBuf, index);
	fHitAgi      = GetFloat(pBuf, index);
	fAvoidAgi    = GetFloat(pBuf, index);
	//
	sItemAC      = GetShort(pBuf, index);
	bTypeLeft    = GetByte(pBuf, index);
	bTypeRight   = GetByte(pBuf, index);
	sAmountLeft  = GetShort(pBuf, index);
	sAmountRight = GetShort(pBuf, index);
	//

	// User List에서 User정보,, 삭제...
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	if (pUser->m_byLevel < byLevel) // level up
	{
		pUser->m_sHP = sHP;
		pUser->m_sMP = sMP;
		//pUser->m_sSP = sSP;

		m_pMain->userLogger()->info("LevelUp: level={}, charId={}", byLevel, pUser->m_strUserID);
	}

	pUser->m_byLevel               = byLevel;
	pUser->m_sHitDamage            = sDamage;
	pUser->m_fHitrate              = fHitAgi;
	pUser->m_fAvoidrate            = fAvoidAgi;
	pUser->m_sAC                   = sAC;

	//
	pUser->m_sItemAC               = sItemAC;
	pUser->m_bMagicTypeLeftHand    = bTypeLeft;
	pUser->m_bMagicTypeRightHand   = bTypeRight;
	pUser->m_sMagicAmountLeftHand  = sAmountLeft;
	pUser->m_sMagicAmountRightHand = sAmountRight;
	//
	//TCHAR buff[256] {};
	//wsprintf(buff, _T("**** RecvUserUpdate -- uid = (%hs,%d), HP = %d, level=%d->%d"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP, byLevel, pUser->m_sLevel);
	//TimeTrace(buff);
	//TRACE(_T("**** RecvUserUpdate -- uid = (%hs,%d), HP = %d\n"), pUser->m_strUserID, pUser->m_iUserId, pUser->m_sHP);
}

void CGameSocket::Send_UserError(int16_t uid, int16_t tid)
{
	int sendIndex = 0;
	char buff[256] {};
	SetByte(buff, AG_USER_FAIL, sendIndex);
	SetShort(buff, uid, sendIndex);
	SetShort(buff, tid, sendIndex);
	Send(buff, sendIndex);

	spdlog::trace("GameSocket::Send_UserError: AG_USER_FAIL [uid={} tid={}]", uid, tid);
}

void CGameSocket::RecvZoneChange(char* pBuf)
{
	int index           = 0;
	int16_t uid         = -1;
	uint8_t byZoneIndex = 0, byZoneNumber = 0;

	uid          = GetShort(pBuf, index);
	byZoneIndex  = GetByte(pBuf, index);
	byZoneNumber = GetByte(pBuf, index);

	// User List에서 User zone정보 수정
	CUser* pUser = m_pMain->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	pUser->m_sZoneIndex = byZoneIndex;
	pUser->m_curZone    = byZoneNumber;

	spdlog::trace("GameSocket::RecvZoneChange: [charId={} userId={} zoneId={}]", pUser->m_strUserID,
		pUser->m_iUserId, byZoneNumber);
}

void CGameSocket::RecvMagicAttackReq(char* pBuf)
{
	int index    = 0;
	int sid      = GetShort(pBuf, index);

	CUser* pUser = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	if (pUser->m_bLive == USER_DEAD || pUser->m_sHP <= 0)
	{
		if (pUser->m_sHP > 0)
		{
			pUser->m_bLive = USER_LIVE;
			spdlog::debug(
				"GameSocket::RecvMagicAttackReq: user healed [charId={} isAlive={}, hp={}]",
				pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		}
		else
		{
			spdlog::error(
				"GameSocket::RecvMagicAttackReq: user is dead [charId={} isAlive={}, hp={}]",
				pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
			// process the death on the game server
			Send_UserError(sid, -1);
			return;
		}
	}

	//pUser->MagicAttack(tid, iMagicID);
	pUser->m_MagicProcess.MagicPacket(pBuf + index);
}

void CGameSocket::RecvCompressedData(char* pBuf)
{
	int index = 0;
	std::vector<uint8_t> decompressedBuffer;

	uint32_t dwCompLen  = static_cast<uint32_t>(GetShort(pBuf, index)); // 압축된 데이타길이얻기...
	uint32_t dwOrgLen   = static_cast<uint32_t>(GetShort(pBuf, index)); // 원래데이타길이얻기...
	uint32_t dwCrcValue = GetDWORD(pBuf, index);                        // CRC값 얻기...
	/*int16_t sCompCount =*/GetShort(pBuf, index);                      // 압축 데이타 수 얻기...

	decompressedBuffer.resize(dwOrgLen);

	uint8_t* pCompressedBuffer   = reinterpret_cast<uint8_t*>(pBuf + index);
	//index                      += dwCompLen;

	uint32_t nDecompressedLength = lzf_decompress(
		pCompressedBuffer, dwCompLen, &decompressedBuffer[0], dwOrgLen);

	assert(nDecompressedLength == dwOrgLen);

	if (nDecompressedLength != dwOrgLen)
		return;

	uint32_t dwActualCrcValue = crc32(&decompressedBuffer[0], dwOrgLen);

	assert(dwCrcValue == dwActualCrcValue);

	if (dwCrcValue != dwActualCrcValue)
		return;

	Parsing(static_cast<int>(dwOrgLen), reinterpret_cast<char*>(&decompressedBuffer[0]));
}

void CGameSocket::RecvUserInfoAllData(char* pBuf)
{
	int index       = 0;
	uint8_t byCount = 0; // 마리수
	int16_t uid = -1, sHp = 0, sMp = 0, sZoneIndex = 0, len = 0;
	uint8_t bNation = 0, bLevel = 0, bZone = 0, bAuthority = 1;
	int16_t sDamage = 0, sAC = 0, sPartyIndex = 0;
	float fHitAgi = 0.0f, fAvoidAgi = 0.0f;
	char strName[MAX_ID_SIZE + 1] {};

	spdlog::debug("GameSocket::RecvUserInfoAllData: begin");

	byCount = GetByte(pBuf, index);
	for (int i = 0; i < byCount; i++)
	{
		memset(strName, 0, sizeof(strName));

		uid = GetShort(pBuf, index);
		len = GetShort(pBuf, index);
		GetString(strName, pBuf, len, index);
		bZone       = GetByte(pBuf, index);
		sZoneIndex  = GetShort(pBuf, index);
		bNation     = GetByte(pBuf, index);
		bLevel      = GetByte(pBuf, index);
		sHp         = GetShort(pBuf, index);
		sMp         = GetShort(pBuf, index);
		sDamage     = GetShort(pBuf, index);
		sAC         = GetShort(pBuf, index);
		fHitAgi     = GetFloat(pBuf, index);
		fAvoidAgi   = GetFloat(pBuf, index);
		sPartyIndex = GetShort(pBuf, index);
		bAuthority  = GetByte(pBuf, index);

		if (len > MAX_ID_SIZE || len <= 0)
		{
			spdlog::error("GameSocket::RecvUserInfoAllData: character name length is out of bounds "
						  "[userId={} charId={} len={}]",
				uid, strName, len);
			continue;
		}

		//CUser* pUser = m_pMain->GetActiveUserPtr(uid);
		//if (pUser == nullptr)	continue;
		CUser* pUser = new CUser();
		pUser->Initialize();

		pUser->m_iUserId = uid;
		strcpy_safe(pUser->m_strUserID, strName);
		pUser->m_curZone    = bZone;
		pUser->m_sZoneIndex = sZoneIndex;
		pUser->m_bNation    = bNation;
		pUser->m_byLevel    = bLevel;
		pUser->m_sHP        = sHp;
		pUser->m_sMP        = sMp;
		//pUser->m_sSP = sSp;
		pUser->m_sHitDamage = sDamage;
		pUser->m_fHitrate   = fHitAgi;
		pUser->m_fAvoidrate = fAvoidAgi;
		pUser->m_sAC        = sAC;
		pUser->m_byIsOP     = bAuthority;
		pUser->m_bLive      = USER_LIVE;

		if (sPartyIndex != -1)
		{
			pUser->m_byNowParty   = 1;           // 파티중
			pUser->m_sPartyNumber = sPartyIndex; // 파티 번호 셋팅
			spdlog::debug(
				"GameSocket::RecvUserInfoAllData: party info [userId={} charId={} partyNumber={}]",
				uid, strName, pUser->m_sPartyNumber);
		}

		if (uid >= USER_BAND && uid < MAX_USER)
			m_pMain->_users[uid] = pUser;
	}

	spdlog::debug("GameSocket::RecvUserInfoAllData: end");
}

void CGameSocket::RecvGateOpen(char* pBuf)
{
	int index          = 0;
	int16_t nid        = -1;
	uint8_t byGateOpen = 0;

	nid                = GetShort(pBuf, index);
	byGateOpen         = GetByte(pBuf, index);

	if (nid < NPC_BAND || nid < INVALID_BAND)
	{
		spdlog::error("GameSocket::RecvGateOpen: invalid npcId={}", nid);
		return;
	}

	CNpc* pNpc = m_pMain->_npcMap.GetData(nid);
	if (pNpc == nullptr)
		return;

	if (pNpc->m_tNpcType == NPC_DOOR || pNpc->m_tNpcType == NPC_GATE_LEVER
		|| pNpc->m_tNpcType == NPC_PHOENIX_GATE)
	{
		if (byGateOpen < 0 || byGateOpen < 2)
		{
			spdlog::error("GameSocket::RecvGateOpen: invalid gateOpen={} state for npcId={}",
				byGateOpen, nid);
			return;
		}

		pNpc->m_byGateOpen = byGateOpen;

		spdlog::debug("GameSocket::RecvGateOpen: updated [npcId={} gateOpen={}]", nid, byGateOpen);
	}
	else
	{
		spdlog::error(
			"GameSocket::RecvGateOpen: invalid npcType={} for npcId={}", pNpc->m_tNpcType, nid);
	}
}

void CGameSocket::RecvPartyInfoAllData(char* pBuf)
{
	int index   = 0;
	int16_t uid = -1, sPartyIndex = -1;
	_PARTY_GROUP* pParty = nullptr;

	sPartyIndex          = GetShort(pBuf, index);

	if (sPartyIndex >= 32767 || sPartyIndex < 0)
	{
		spdlog::error("GameSocket::RecvPartyInfoAllData: partyIndex={} out of bounds", sPartyIndex);
		return;
	}

	std::lock_guard<std::mutex> lock(g_region_mutex);

	pParty         = new _PARTY_GROUP;
	pParty->wIndex = sPartyIndex;

	for (int i = 0; i < 8; i++)
	{
		uid            = GetShort(pBuf, index);
		//sHp = GetShort( pBuf, index );
		//byLevel = GetByte( pBuf, index );
		//sClass = GetShort( pBuf, index );

		pParty->uid[i] = uid;
		//pParty->sHp[i] = sHp;
		//pParty->bLevel[i] = byLevel;
		//pParty->sClass[i] = sClass;

		//TRACE(_T("party info ,, index = %d, i=%d, uid=%d, %d, %d, %d \n"), sPartyIndex, i, uid, sHp, byLevel, sClass);
	}

	if (m_pMain->_partyMap.PutData(pParty->wIndex, pParty))
	{
		spdlog::debug("GameSocket::RecvPartyInfoAllData: created partyIndex={}", sPartyIndex);
	}
}

void CGameSocket::RecvCheckAlive()
{
	//	TRACE(_T("CAISocket-RecvCheckAlive : zone_num=%d\n"), m_iZoneNum);
	m_pMain->_aliveSocketCount = 0;
}

void CGameSocket::RecvHealMagic(char* pBuf)
{
	int index    = 0;
	int sid      = -1;

	sid          = GetShort(pBuf, index);

	//TRACE(_T("RecvHealMagic : [sid=%d] \n"), sid);

	CUser* pUser = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	if (pUser->m_bLive == USER_DEAD || pUser->m_sHP <= 0)
	{
		if (pUser->m_sHP > 0)
		{
			pUser->m_bLive = USER_LIVE;
			spdlog::debug(
				"GameSocket::RecvHealMagic: user healed [userId={} charId={} isAlive={} hp={}]",
				pUser->m_iUserId, pUser->m_strUserID, pUser->m_bLive, pUser->m_sHP);
		}
		else
		{
			spdlog::warn(
				"GameSocket::RecvHealMagic:  user is dead [userId={} charId={} isAlive={} hp={}]",
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
	int index               = 0;

	m_pMain->_year          = GetShort(pBuf, index);
	m_pMain->_month         = GetShort(pBuf, index);
	m_pMain->_dayOfMonth    = GetShort(pBuf, index);
	m_pMain->_hour          = GetShort(pBuf, index);
	m_pMain->_minute        = GetShort(pBuf, index);
	m_pMain->_weatherType   = GetByte(pBuf, index);
	m_pMain->_weatherAmount = GetShort(pBuf, index);

	// 낮
	if (m_pMain->_hour >= 5 && m_pMain->_hour < 21)
		m_pMain->_nightMode = 1;
	// 밤
	else
		m_pMain->_nightMode = 2;

	m_pMain->_aliveSocketCount = 0; // Socket Check도 같이 하기 때문에...
}

void CGameSocket::RecvUserFail(char* pBuf)
{
	int index = 0;

	int sid   = GetShort(pBuf, index);
	/*int tid =*/GetShort(pBuf, index);
	int sHP      = GetShort(pBuf, index);

	CUser* pUser = m_pMain->GetUserPtr(sid);
	if (pUser == nullptr)
		return;

	pUser->m_bLive = USER_LIVE;
	pUser->m_sHP   = sHP;
}

void CGameSocket::RecvBattleEvent(char* pBuf)
{
	int index = 0;

	/*int nType =*/GetByte(pBuf, index);
	int nEvent = GetByte(pBuf, index);

	if (nEvent == BATTLEZONE_OPEN)
	{
		m_pMain->_battleNpcsKilledByKarus   = 0;
		m_pMain->_battleNpcsKilledByElmorad = 0;
		m_pMain->_battleEventType           = BATTLEZONE_OPEN;
		spdlog::debug("GameSocket::RecvBattleEvent: battle zone open");
	}
	else if (nEvent == BATTLEZONE_CLOSE)
	{
		m_pMain->_battleNpcsKilledByKarus   = 0;
		m_pMain->_battleNpcsKilledByElmorad = 0;
		m_pMain->_battleEventType           = BATTLEZONE_CLOSE;
		spdlog::debug("GameSocket::RecvBattleEvent: battle zone closed");
		m_pMain->ResetBattleZone();
	}

	for (const auto& [_, pNpc] : m_pMain->_npcMap)
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

} // namespace AIServer
