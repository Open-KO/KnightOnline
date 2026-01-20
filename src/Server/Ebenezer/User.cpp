#include "pch.h"
#include "EbenezerApp.h"
#include "Map.h"
#include "OperationMessage.h"
#include "User.h"
#include "db_resources.h"

#include <shared/DateTime.h>
#include <shared/globals.h>
#include <shared/lzf.h>
#include <shared/packets.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

namespace Ebenezer
{

extern std::recursive_mutex g_region_mutex;
extern bool g_serverdown_flag;

CUser::CUser(test_tag tag) : TcpServerSocket(tag)
{
}

CUser::CUser(TcpServerSocketManager* socketManager) : TcpServerSocket(socketManager)
{
}

CUser::~CUser()
{
	delete _regionBuffer;
}

std::string_view CUser::GetImplName() const
{
	return "User";
}

void CUser::Initialize()
{
	m_pMain = EbenezerApp::instance();

	if (_regionBuffer == nullptr)
		_regionBuffer = new _REGION_BUFFER();

	for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
		m_sSourceID[i] = -1;

	for (int i = 0; i < MAX_MESSAGE_EVENT; i++)
		m_iSelMsgEvent[i] = -1;

	for (int i = 0; i < MAX_CURRENT_EVENT; i++)
		m_sEvent[i] = -1;

	_regionBuffer->iLength = 0;
	memset(_regionBuffer->pDataBuff, 0, sizeof(_regionBuffer->pDataBuff));

	_sendValue         = 0;
	_recvValue         = 0;

	// Cryption
	_jvCryptionEnabled = false;
	_jvCryption.GenerateKey();
	///~

	m_MagicProcess.m_pMain    = m_pMain;
	m_MagicProcess.m_pSrcUser = this;

	m_RegionX                 = -1;
	m_RegionZ                 = -1;

	m_sBodyAc                 = 0;
	m_sTotalHit               = 0;
	m_sTotalAc                = 0;
	m_fTotalHitRate           = 0;
	m_fTotalEvasionRate       = 0;

	m_sItemMaxHp              = 0;
	m_sItemMaxMp              = 0;
	m_iItemWeight             = 0;
	m_sItemHit                = 0;
	m_sItemAc                 = 0;
	m_sItemStr                = 0;
	m_sItemSta                = 0;
	m_sItemDex                = 0;
	m_sItemIntel              = 0;
	m_sItemCham               = 0;
	m_sItemHitrate            = 100;
	m_sItemEvasionrate        = 100;

	m_sSpeed                  = 0;

	m_iMaxHp                  = 0;
	m_iMaxMp                  = 1;
	m_iMaxExp                 = 0;
	m_iMaxWeight              = 0;

	m_bFireR                  = 0;
	m_bColdR                  = 0;
	m_bLightningR             = 0;
	m_bMagicR                 = 0;
	m_bDiseaseR               = 0;
	m_bPoisonR                = 0;

	m_sDaggerR                = 0;
	m_sSwordR                 = 0;
	m_sAxeR                   = 0;
	m_sMaceR                  = 0;
	m_sSpearR                 = 0;
	m_sBowR                   = 0;

	m_bMagicTypeLeftHand      = 0; // For weapons and shields with special power.
	m_bMagicTypeRightHand     = 0;
	m_sMagicAmountLeftHand    = 0;
	m_sMagicAmountRightHand   = 0;

	m_iZoneIndex              = 0;
	m_bResHpType              = USER_STANDING;
	m_bWarp                   = 0x00;

	m_sPartyIndex             = -1;
	m_sExchangeUser           = -1;
	m_bExchangeOK             = 0x00;
	m_sPrivateChatUser        = -1;
	m_bNeedParty              = 0x01;

	m_fHPLastTimeNormal       = 0.0; // For Automatic HP recovery.
	m_bHPAmountNormal         = 0;
	m_bHPDurationNormal       = 0;
	m_bHPIntervalNormal       = 5;

	m_fAreaLastTime           = 0.0; // For Area Damage spells Type 3.
	m_fAreaStartTime          = 0.0;
	m_bAreaInterval           = 5;
	m_iAreaMagicID            = 0;

	m_sFriendUser             = -1;

	InitType3(); // Initialize durational type 3 stuff :)
	InitType4(); // Initialize durational type 4 stuff :)

	m_fSpeedHackClientTime = 0.0;
	m_fSpeedHackServerTime = 0.0;
	m_bSpeedHackCheck      = 0;

	m_fBlinkStartTime      = 0.0;

	m_sAliveCount          = 0;

	m_bAbnormalType        = 1; // User starts out in normal size.

	m_sWhoKilledMe         = -1;
	m_iLostExp             = 0;

	m_fLastTrapAreaTime    = 0.0;

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

	m_sEventNid           = -1;

	m_bZoneChangeFlag     = false;

	m_bRegeneType         = 0;

	m_fLastRegeneTime     = 0.0;

	m_bZoneChangeSameZone = false;

	memset(m_strCouponId, 0, sizeof(m_strCouponId));
	m_iEditBoxEvent = -1;

	for (int j = 0; j < MAX_CURRENT_EVENT; j++)
		m_sEvent[j] = -1;

	while (!m_arUserEvent.empty())
		m_arUserEvent.pop_back();

	m_bIsPartyLeader      = false;
	m_byInvisibilityState = 0;
	m_sDirection          = 0;
	m_bIsChicken          = false;
	m_byKnightsRank       = 0;
	m_byPersonalRank      = 0;

	m_byLastExchangeNum   = 0;

	TcpServerSocket::Initialize();
}

int CUser::Send(char* pBuf, int length)
{
	constexpr int PacketHeaderSize          = 6;
	constexpr int EncryptedPacketHeaderSize = 5;

	char sendBuffer[MAX_PACKET_SIZE];
	int index = 0;

	if (_jvCryptionEnabled)
	{
		assert(length >= 0);
		assert((length + PacketHeaderSize + EncryptedPacketHeaderSize) <= MAX_PACKET_SIZE);

		if (length < 0 || length + (PacketHeaderSize + EncryptedPacketHeaderSize) > MAX_PACKET_SIZE)
			return -1;

		uint16_t encryptedLength = static_cast<uint16_t>(length + EncryptedPacketHeaderSize);

		_sendValue++;
		_sendValue &= 0x00ffffff;

		SetByte(sendBuffer, PACKET_START1, index);
		SetByte(sendBuffer, PACKET_START2, index);
		SetShort(sendBuffer, encryptedLength, index);

		int encryptIndex = index;

		SetByte(sendBuffer, 0xfc, index); // 암호가 정확한지
		SetByte(sendBuffer, 0x1e, index);
		SetString(sendBuffer, reinterpret_cast<const char*>(&_sendValue), 3, index);
		SetString(sendBuffer, pBuf, length, index);

		// This can encrypt in-place.
		uint8_t* bufferToEncrypt = reinterpret_cast<uint8_t*>(&sendBuffer[encryptIndex]);
		_jvCryption.JvEncryptionFast(index - encryptIndex, bufferToEncrypt, bufferToEncrypt);

		SetByte(sendBuffer, PACKET_END1, index);
		SetByte(sendBuffer, PACKET_END2, index);
	}
	else
	{
		assert(length >= 0);
		assert((length + PacketHeaderSize) <= MAX_PACKET_SIZE);

		if (length < 0 || (length + PacketHeaderSize) > MAX_PACKET_SIZE)
			return -1;

		SetByte(sendBuffer, PACKET_START1, index);
		SetByte(sendBuffer, PACKET_START2, index);
		SetShort(sendBuffer, length, index);
		SetString(sendBuffer, pBuf, length, index);
		SetByte(sendBuffer, PACKET_END1, index);
		SetByte(sendBuffer, PACKET_END2, index);
	}

	return QueueAndSend(sendBuffer, index);
}

void CUser::SendCompressingPacket(const char* pData, int len)
{
	if (len <= 0 || len >= 49152)
	{
		spdlog::error("User::SendCompressingPacket: message length out of bounds [len={}]", len);
		return;
	}

	int sendIndex = 0;
	char sendBuffer[32000] {}, pBuff[32000] {};
	uint32_t out_len = lzf_compress(pData, len, pBuff, sizeof(pBuff));
	if (out_len == 0 || out_len > sizeof(pBuff))
	{
		spdlog::error("User::SendCompressingPacket: compression failed [out_len={} pBuffSize={}]",
			out_len, sizeof(pBuff));
		Send((char*) pData, len);
		return;
	}

	SetByte(sendBuffer, WIZ_COMPRESS_PACKET, sendIndex);
	SetShort(sendBuffer, (int16_t) out_len, sendIndex);
	SetShort(sendBuffer, (int16_t) len, sendIndex);
	SetDWORD(sendBuffer, 0, sendIndex); // checksum
	SetString(sendBuffer, pBuff, out_len, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::RegionPacketAdd(char* pBuf, int len)
{
	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

	SetShort(_regionBuffer->pDataBuff, len, _regionBuffer->iLength);
	SetString(_regionBuffer->pDataBuff, pBuf, len, _regionBuffer->iLength);
}

int CUser::RegionPacketClear(char* GetBuf)
{
	int index = 0;
	SetByte(GetBuf, WIZ_CONTINOUS_PACKET, index);

	{
		std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

		if (_regionBuffer->iLength <= 0)
			return 0;

		SetShort(GetBuf, _regionBuffer->iLength, index);
		SetString(GetBuf, _regionBuffer->pDataBuff, _regionBuffer->iLength, index);

		memset(_regionBuffer->pDataBuff, 0, REGION_BUFF_SIZE);
		_regionBuffer->iLength = 0;
	}

	return index;
}

bool CUser::PullOutCore(char*& data, int& length)
{
	int bufferLength = _recvCircularBuffer.GetValidCount();

	// We expect at least 7 bytes (header, length, data [at least 1 byte], tail)
	if (bufferLength < 7)
		return false; // wait for more data

	std::vector<uint8_t> tmpBuffer(bufferLength);
	_recvCircularBuffer.GetData(reinterpret_cast<char*>(tmpBuffer.data()), bufferLength);

	if (tmpBuffer[0] != PACKET_START1 && tmpBuffer[1] != PACKET_START2)
	{
		spdlog::error("User::PullOutCore: {}: failed to detect header ({:2X}, {:2X})", _socketId,
			tmpBuffer[0], tmpBuffer[1]);

		Close();
		return false;
	}

	// Find the packet's start position - this is in front of the 2 byte header.
	int startPos = 2;

	// Build the length (2 bytes, network order)
	MYSHORT slen;
	slen.b[0]          = tmpBuffer[startPos];
	slen.b[1]          = tmpBuffer[startPos + 1];

	length             = slen.w;

	int originalLength = length;

	if (length < 0)
	{
		spdlog::error("User::PullOutCore: {}: invalid length ({})", _socketId, length);

		Close();
		return false;
	}

	if (length > bufferLength)
	{
		spdlog::debug(
			"User::PullOutCore: {}: reported length ({}) is not in buffer ({}) - waiting for now",
			_socketId, length, bufferLength);
		return false; // wait for more data
	}

	// Find the end position of the packet data.
	// From the start position, that is after 2 bytes for the length,
	// then the length of the data itself.
	int endPos = startPos + 2 + length;

	// We expect a 2 byte tail after the end position.
	if ((endPos + 2) > bufferLength)
	{
		spdlog::debug("User::PullOutCore: {}: tail not in buffer - waiting for now", _socketId);
		return false; // wait for more data
	}

	if (tmpBuffer[endPos] != PACKET_END1 || tmpBuffer[endPos + 1] != PACKET_END2)
	{
		spdlog::error("User::PullOutCore: {}: failed to detect tail ({:2X}, {:2X})", _socketId,
			tmpBuffer[endPos], tmpBuffer[endPos + 1]);

		Close();
		return false;
	}

	// We've found the entire packet.
	// Do we need to decrypt it?
	if (_jvCryptionEnabled)
	{
		// Encrypted packets contain a checksum (4) and sequence number (4).
		// We should also expect at least 1 byte for its data in addition to this.
		if (length <= 8)
		{
			spdlog::error(
				"User::PullOutCore: {}: Insufficient packet length [{}] for a decrypted packet",
				_socketId, length);
			Close();
			return false;
		}

		std::vector<uint8_t> decryption_buffer(length);

		int decryptedLength = _jvCryption.JvDecryptionWithCRC32(
			length, &tmpBuffer[startPos + 2], &decryption_buffer[0]);
		if (decryptedLength < 0)
		{
			spdlog::error("User::PullOutCore: {}: Failed decryption", _socketId);
			Close();
			return false;
		}

		int index          = 0;
		uint32_t recvValue = GetDWORD(
			reinterpret_cast<const char*>(decryption_buffer.data()), index);

		// Verify the sequence number.
		// If it wraps back around, we should simply let it reset.
		if (recvValue != 0 && _recvValue > recvValue)
		{
			spdlog::error("User::PullOutCore: {}: recvValue error... len={}, recvValue={}, prev={}",
				_socketId, length, recvValue, _recvValue);

			Close();
			return false;
		}

		_recvValue = recvValue;

		// Now we need to trim out the extra data from the packet, so it's just the base packet data remaining.
		// Make sure that there is still data for this.
		length     = decryptedLength - index;
		if (length <= 0)
		{
			spdlog::error("User::PullOutCore: {}: decrypted packet length too small... len={}",
				_socketId, length);

			Close();
			return false;
		}

		data = new char[length];
		memcpy(data, &decryption_buffer[index], length);
	}
	// Packet not encrypted, we can just copy it over as-is.
	else
	{
		data = new char[length];
		memcpy(data, &tmpBuffer[startPos + 2], length);
	}

	_recvCircularBuffer.HeadIncrease(6 + originalLength); // 6: header (2) + end (2) + length (2)

	// Found a packet in this attempt.
	return true;
}

void CUser::CloseProcess()
{
	UserInOut(USER_OUT);

	if (m_sPartyIndex != -1)
		PartyRemove(_socketId);

	if (m_sExchangeUser != -1)
		ExchangeCancel();

	/* 부디 잘 작동하길 ㅠ.ㅠ
	if (!m_bZoneChangeFlag) {
		if (m_pUserData->m_bZone == ZONE_BATTLE || (m_pUserData->m_bZone != m_pUserData->m_bNation && m_pUserData->m_bZone < 3) ) {
			model::Home* pHomeInfo = nullptr;	// Send user back home in case it was the battlezone.
			pHomeInfo = m_pMain->m_HomeTableMap.GetData(m_pUserData->m_bNation);
			if (!pHomeInfo) return;

			m_pUserData->m_bZone = m_pUserData->m_bNation;

			if (m_pUserData->m_bNation == NATION_KARUS) {
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
	TcpServerSocket::CloseProcess();
}

void CUser::Parsing(int len, char* pData)
{
	int index          = 0;
	double currentTime = 0.0;

	uint8_t command    = GetByte(pData, index);

	spdlog::trace("User::Parsing: userId={} charId={} command={:02X} len={}", GetSocketID(),
		m_pUserData->m_id, command, len);

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
			if (_state != CONNECTION_STATE_GAMESTART)
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
			InitType3(); // Init Type 3.....
			InitType4(); // Init Type 4.....
			Regene(pData + index);
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
			int uid      = GetShort(pData, index);
			uint8_t echo = GetByte(pData, index);
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
			m_MagicProcess.MagicPacket(pData + index);
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
			TestPacket();
			break;

		case WIZ_SELECT_MSG:
			RecvSelectMsg(pData + index);
			break;

		case WIZ_EDIT_BOX:
			RecvEditBox(pData + index);
			break;

		default:
			spdlog::error(
				"User::Parsing: Unhandled opcode {:02X} [ip={} accountName={} characterName={}]",
				command, GetRemoteIP(), m_strAccountID, m_pUserData->m_id);

#ifndef _DEBUG
			Close();
			return;
#else
			break;
#endif
	}

	currentTime = TimeGet();

	if (command == WIZ_GAMESTART)
	{
		m_fHPLastTimeNormal = currentTime;

		for (int h = 0; h < MAX_TYPE3_REPEAT; h++)
			m_fHPLastTime[h] = currentTime;
	}

	// For Sitdown/Standup HP restoration.
	if (m_fHPLastTimeNormal != 0.0 && (currentTime - m_fHPLastTimeNormal) > m_bHPIntervalNormal
		&& m_bAbnormalType != ABNORMAL_BLINKING)
		HPTimeChange(currentTime);

	// For Type 3 HP Duration.
	if (m_bType3Flag)
	{
		for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
		{
			if (m_fHPLastTime[i] != 0.0 && (currentTime - m_fHPLastTime[i]) > m_bHPInterval[i])
			{
				HPTimeChangeType3(currentTime);
				break;
			}
		}
	}

	// For Type 4 Stat Duration.
	if (m_bType4Flag)
		Type4Duration(currentTime);

	// Should you stop blinking?
	if (m_bAbnormalType == ABNORMAL_BLINKING)
		BlinkTimeCheck(currentTime);
}

void CUser::VersionCheck()
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	SetByte(sendBuffer, WIZ_VERSION_CHECK, sendIndex);
	SetShort(sendBuffer, __VERSION, sendIndex);
	// Cryption
	SetInt64(sendBuffer, _jvCryption.GetPublicKey(), sendIndex);
	///~
	Send(sendBuffer, sendIndex);
	// Cryption
	_jvCryptionEnabled = true;
	///~
}

void CUser::LoginProcess(char* pBuf)
{
	std::shared_ptr<CUser> pUser;
	int index = 0, idlen = 0, pwdlen = 0, sendIndex = 0, retvalue = 0;
	char accountid[MAX_ID_SIZE + 1] {}, password[MAX_PW_SIZE + 1] {}, sendBuffer[256] {};

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE || idlen <= 0)
		goto fail_return;

	GetString(accountid, pBuf, idlen, index);

	pwdlen = GetShort(pBuf, index);
	if (pwdlen > MAX_PW_SIZE || pwdlen <= 0)
		goto fail_return;

	GetString(password, pBuf, pwdlen, index);

	pUser = m_pMain->GetUserPtr(accountid, NameType::Account);
	if (pUser != nullptr && pUser->_socketId != _socketId)
	{
		pUser->UserDataSaveToAgent();
		pUser->Close();
		goto fail_return;
	}

	SetByte(sendBuffer, WIZ_LOGIN, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetShort(sendBuffer, idlen, sendIndex);
	SetString(sendBuffer, accountid, idlen, sendIndex);
	SetShort(sendBuffer, pwdlen, sendIndex);
	SetString(sendBuffer, password, pwdlen, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::LoginProcess: send error: {}", retvalue);
		goto fail_return;
	}

	strcpy_safe(m_strAccountID, accountid);
	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_LOGIN, sendIndex);
	SetByte(sendBuffer, 0xFF, sendIndex); // 성공시 국가 정보... FF 실패
	Send(sendBuffer, sendIndex);
}

void CUser::NewCharToAgent(char* pBuf)
{
	model::Coefficient* p_TableCoefficient = nullptr;
	int index = 0, idlen = 0, sendIndex = 0, retvalue = 0;
	int charindex = 0, race = 0, Class = 0, hair = 0, face = 0, str = 0, sta = 0, dex = 0,
		intel = 0, cha = 0;
	char charid[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};
	uint8_t result = 0;
	int sum        = 0;

	charindex      = GetByte(pBuf, index);
	idlen          = GetShort(pBuf, index);

	if (idlen > MAX_ID_SIZE || idlen <= 0)
	{
		result = 0x05;
		goto fail_return;
	}

	GetString(charid, pBuf, idlen, index);
	race  = GetByte(pBuf, index);
	Class = GetShort(pBuf, index);
	face  = GetByte(pBuf, index);
	hair  = GetByte(pBuf, index);
	str   = GetByte(pBuf, index);
	sta   = GetByte(pBuf, index);
	dex   = GetByte(pBuf, index);
	intel = GetByte(pBuf, index);
	cha   = GetByte(pBuf, index);

	if (charindex > 4 || charindex < 0)
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

	if (str < 50 || sta < 50 || dex < 50 || intel < 50 || cha < 50)
	{
		result = 0x11;
		goto fail_return;
	}

	SetByte(sendBuffer, WIZ_NEW_CHAR, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);
	SetByte(sendBuffer, charindex, sendIndex);
	SetShort(sendBuffer, idlen, sendIndex);
	SetString(sendBuffer, charid, idlen, sendIndex);
	SetByte(sendBuffer, race, sendIndex);
	SetShort(sendBuffer, Class, sendIndex);
	SetByte(sendBuffer, face, sendIndex);
	SetByte(sendBuffer, hair, sendIndex);
	SetByte(sendBuffer, str, sendIndex);
	SetByte(sendBuffer, sta, sendIndex);
	SetByte(sendBuffer, dex, sendIndex);
	SetByte(sendBuffer, intel, sendIndex);
	SetByte(sendBuffer, cha, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::NewCharToAgent: send error: {}", retvalue);
		goto fail_return;
	}

	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_NEW_CHAR, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::DelCharToAgent(char* pBuf)
{
	int index = 0, idlen = 0, sendIndex = 0, retvalue = 0;
	int charindex = 0, soclen = 0;
	char charid[MAX_ID_SIZE + 1] {}, socno[15] {}, sendBuffer[256] {};

	charindex = GetByte(pBuf, index);
	if (charindex > 4)
		goto fail_return;

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE || idlen <= 0)
		goto fail_return;

	GetString(charid, pBuf, idlen, index);
	soclen = GetShort(pBuf, index);

	// sungyong tw
	// if (soclen != 14)
	//	goto fail_return;

	if (soclen > 14 || soclen <= 0)
		goto fail_return;

	// ~sungyong tw
	GetString(socno, pBuf, soclen, index);

	// 단장은 캐릭 삭제가 안되게, 먼저 클랜을 탈퇴 후 삭제가 되도록,,
	if (m_pUserData->m_bKnights > 0 && m_pUserData->m_bFame == KNIGHTS_DUTY_CHIEF)
		goto fail_return;

	SetByte(sendBuffer, WIZ_DEL_CHAR, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);
	SetByte(sendBuffer, charindex, sendIndex);
	SetString2(sendBuffer, charid, idlen, sendIndex);
	SetString2(sendBuffer, socno, soclen, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::DelCharToAgent: send error: {}", retvalue);
		goto fail_return;
	}

	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_DEL_CHAR, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	SetByte(sendBuffer, 0xFF, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::SelNationToAgent(char* pBuf)
{
	int index = 0, sendIndex = 0, retvalue = 0, nation = 0;
	char sendBuffer[256] {};

	nation = GetByte(pBuf, index);
	if (nation > 2)
		goto fail_return;

	SetByte(sendBuffer, WIZ_SEL_NATION, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);
	SetByte(sendBuffer, nation, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::SelNationToAgent: send error: {}", retvalue);
		goto fail_return;
	}

	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_SEL_NATION, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::SelCharToAgent(char* pBuf)
{
	std::shared_ptr<CUser> pUser;
	C3DMap* pMap            = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;
	uint8_t bInit           = 0x01;
	int index = 0, idlen1 = 0, idlen2 = 0, sendIndex = 0, retvalue = 0, zoneId = 0;
	char charId[MAX_ID_SIZE + 1] {}, accountId[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	idlen1 = GetShort(pBuf, index);
	if (idlen1 > MAX_ID_SIZE || idlen1 <= 0)
		goto fail_return;

	GetString(accountId, pBuf, idlen1, index);

	idlen2 = GetShort(pBuf, index);
	if (idlen2 > MAX_ID_SIZE || idlen2 <= 0)
		goto fail_return;

	GetString(charId, pBuf, idlen2, index);
	bInit  = GetByte(pBuf, index);
	zoneId = GetByte(pBuf, index);

	if (strnicmp(accountId, m_strAccountID, MAX_ID_SIZE) != 0)
	{
		pUser = m_pMain->GetUserPtr(accountId, NameType::Account);
		if (pUser != nullptr && pUser->_socketId != _socketId)
		{
			pUser->Close();
			goto fail_return;
		}

		strcpy_safe(m_strAccountID, accountId); // 존이동 한 경우는 로그인 프로시져가 없으므로...
	}

	pUser = m_pMain->GetUserPtr(charId, NameType::Character);
	if (pUser != nullptr && pUser->_socketId != _socketId)
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

		SetByte(sendBuffer, WIZ_SERVER_CHANGE, sendIndex);
		SetString2(sendBuffer, pInfo->strServerIP, sendIndex);
		SetShort(sendBuffer, pInfo->sPort, sendIndex);
		SetByte(sendBuffer, bInit, sendIndex);
		SetByte(sendBuffer, zoneId, sendIndex);
		SetByte(sendBuffer, m_pMain->m_byOldVictory, sendIndex);
		Send(sendBuffer, sendIndex);
		spdlog::error("User::SelCharToAgent: server change [charId={} ip={} init={}]", charId,
			pInfo->strServerIP, bInit);
		return;
	}

	SetByte(sendBuffer, WIZ_SEL_CHAR, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);
	SetString2(sendBuffer, charId, idlen2, sendIndex);
	SetByte(sendBuffer, bInit, sendIndex);
	SetDWORD(sendBuffer, m_pMain->m_iPacketCount, sendIndex);

	{
		spdlog::debug("User::SelCharToAgent: accountId={} charId={} packetCount={}", m_strAccountID,
			charId, m_pMain->m_iPacketCount);
	}

	m_pMain->m_iPacketCount++;

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::SelCharToAgent: send error: {}", retvalue);
		goto fail_return;
	}

	m_pMain->m_iSendPacketCount++;
	//TRACE(_T(" select char to agent ,, acname=%hs, userid=%hs\n"), m_strAccountID, userid);

	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_SEL_CHAR, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::SelectCharacter(const char* pBuf)
{
	C3DMap* pMap            = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;
	CKnights* pKnights      = nullptr;
	int index = 0, sendIndex = 0, retvalue = 0;
	char sendBuffer[256] {};
	uint8_t result = 0, bInit = 0;

	result = GetByte(pBuf, index);
	bInit  = GetByte(pBuf, index);

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

		SetByte(sendBuffer, WIZ_SERVER_CHANGE, sendIndex);
		SetString2(sendBuffer, pInfo->strServerIP, sendIndex);
		SetShort(sendBuffer, pInfo->sPort, sendIndex);
		SetByte(sendBuffer, bInit, sendIndex);
		SetByte(sendBuffer, m_pUserData->m_bZone, sendIndex);
		SetByte(sendBuffer, m_pMain->m_byOldVictory, sendIndex);
		Send(sendBuffer, sendIndex);
		return;
	}

	if (m_pUserData->m_bAuthority == AUTHORITY_BLOCK_USER)
	{
		Close();
		return;
	}

	// 전쟁중이 아닌상태에서는 대장유저가->일반단장으로
	if (m_pMain->m_byBattleOpen == NO_BATTLE && m_pUserData->m_bFame == KNIGHTS_DUTY_CAPTAIN)
		m_pUserData->m_bFame = KNIGHTS_DUTY_CHIEF;

	if (m_pUserData->m_bZone != m_pUserData->m_bNation && m_pUserData->m_bZone < 3
		&& !m_pMain->m_byBattleOpen)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	if (m_pUserData->m_bZone == ZONE_BATTLE && m_pMain->m_byBattleOpen != NATION_BATTLE)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE && m_pMain->m_byBattleOpen != SNOW_BATTLE)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	if (m_pUserData->m_bZone == ZONE_FRONTIER && m_pMain->m_byBattleOpen)
	{
		NativeZoneReturn();
		Close();
		return;
	}

	SetLogInInfoToDB(bInit); // Write User Login Info To DB for Kicking out or Billing

	SetByte(sendBuffer, WIZ_SEL_CHAR, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bZone, sendIndex);
	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curx * 10, sendIndex);
	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curz * 10, sendIndex);
	SetShort(sendBuffer, (int16_t) m_pUserData->m_cury * 10, sendIndex);
	SetByte(sendBuffer, m_pMain->m_byOldVictory, sendIndex);
	Send(sendBuffer, sendIndex);

	SetDetailData(); // 디비에 없는 데이터 셋팅...

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
			m_pUserData->m_bFame    = 0;
			//TRACE(_T("SelectCharacter - id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
			return;
		}

		if (m_pUserData->m_bKnights != 0)
		{
			/*	memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte( sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex );
			SetByte( sendBuffer, DB_KNIGHTS_LIST_REQ, sendIndex );
			SetShort( sendBuffer, GetSocketID(), sendIndex );
			SetShort( sendBuffer, m_pUserData->m_bKnights, sendIndex );
			retvalue = m_pMain->m_LoggerSendQueue.PutData( sendBuffer, sendIndex );
			if( retvalue >= SMQ_FULL ) {
				//goto fail_return;
				m_pMain->m_StatusList.AddString(_T("KNIGHTS_LIST_REQ Packet Drop!!!"));
			}	*/

			pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
			if (pKnights != nullptr)
			{
				m_pMain->m_KnightsManager.SetKnightsUser(
					m_pUserData->m_bKnights, m_pUserData->m_id);
			}
			else
			{
				//TRACE(_T("SelectCharacter - 기사단 리스트 요청,, id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sendIndex = 0;
				SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
				SetByte(sendBuffer, DB_KNIGHTS_LIST_REQ, sendIndex);
				SetShort(sendBuffer, GetSocketID(), sendIndex);
				SetShort(sendBuffer, m_pUserData->m_bKnights, sendIndex);
				retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
				if (retvalue >= SMQ_FULL)
				{
					//goto fail_return;
					spdlog::error("User::SelectCharacter: KNIGHTS_LIST_REQ Packet Drop!!!");
				}

				pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
				if (pKnights != nullptr)
				{
					//TRACE(_T("SelectCharacter - 기사단 리스트 추가,, id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
					m_pMain->m_KnightsManager.SetKnightsUser(
						m_pUserData->m_bKnights, m_pUserData->m_id);
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
			m_pUserData->m_bFame    = 0;
			//TRACE(_T("SelectCharacter - id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);
			return;
		}

		if (m_pUserData->m_bKnights != 0)
		{
			pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
			if (pKnights != nullptr)
			{
				m_pMain->m_KnightsManager.SetKnightsUser(
					m_pUserData->m_bKnights, m_pUserData->m_id);
			}
			// 기사단이 파괴되어 있음으로..
			else
			{
				m_pUserData->m_bKnights = 0;
				m_pUserData->m_bFame    = 0;
			}
		}
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_DATASAVE, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_Accountid, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex); // login...
	SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iExp, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iLoyalty, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);

	retvalue = m_pMain->m_ItemLoggerSendQ.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::SelectCharacter: send error: {}", retvalue);
		return;
	}

	//TRACE(_T("SelectCharacter - id=%hs, knights=%d, fame=%d\n"), m_pUserData->m_id, m_pUserData->m_bKnights, m_pUserData->m_bFame);

	return;

fail_return:
	SetByte(sendBuffer, WIZ_SEL_CHAR, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::AllCharInfoToAgent()
{
	int sendIndex = 0, retvalue = 0;
	char sendBuffer[256] {};

	SetByte(sendBuffer, WIZ_ALLCHAR_INFO_REQ, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_ALLCHAR_INFO_REQ, sendIndex);
		SetByte(sendBuffer, 0xFF, sendIndex);

		spdlog::error("User::AllCharInfoToAgent: send error: {}", retvalue);
	}
}

void CUser::UserDataSaveToAgent()
{
	int sendIndex = 0, retvalue = 0;
	char sendBuffer[256] {};

	if (strlen(m_pUserData->m_id) == 0 || strlen(m_pUserData->m_Accountid) == 0)
		return;

	SetByte(sendBuffer, WIZ_DATASAVE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_Accountid, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
		spdlog::error("User::UserDataSaveToAgent: send error: {}", retvalue);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_DATASAVE, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_Accountid, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
	SetByte(sendBuffer, 0x02, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iExp, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iLoyalty, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);

	retvalue = m_pMain->m_ItemLoggerSendQ.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
		spdlog::error("User::UserDataSaveToAgent: item send error: {}", retvalue);
}

void CUser::LogOut()
{
	int index = 0, sendIndex = 0, count = 0;
	char sendBuffer[256] {};

	spdlog::debug(
		"User::LogOut: accountId={} charId={}", m_pUserData->m_Accountid, m_pUserData->m_id);

	auto pUser = m_pMain->GetUserPtr(m_pUserData->m_Accountid, NameType::Account);
	if (pUser != nullptr && pUser->_socketId != _socketId)
	{
		spdlog::error(
			"User::LogOut: got a pointer to a duplicate user socket [accountId={} charId={}]",
			m_pUserData->m_Accountid, m_pUserData->m_id);
		return;
	}

	// 이미 유저가 빠진 경우..
	if (strlen(m_pUserData->m_id) == 0)
		return;

	SetByte(sendBuffer, WIZ_LOGOUT, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_Accountid, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);

	do
	{
		if (m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex) == 1)
			break;

		count++;
	}
	while (count < 30);

	if (count > 29)
	{
		spdlog::error("User::LogOut: send error: accountId={} charId={}", m_pUserData->m_Accountid,
			m_pUserData->m_id);
	}

	SetByte(sendBuffer, AG_USER_LOG_OUT, index);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_DATASAVE, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_Accountid, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
	SetByte(sendBuffer, 0x03, sendIndex); // logout
	SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iExp, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iLoyalty, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);

	int retvalue = m_pMain->m_ItemLoggerSendQ.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::LogOut: item send error: {}", retvalue);
		return;
	}

	//	if (m_pUserData->m_bKnights > 0)
	//		m_pMain->m_KnightsManager.ModifyKnightsUser(m_pUserData->m_bKnights, m_pUserData->m_id, m_pUserData->m_bFame, m_pUserData->m_bLevel, m_pUserData->m_sClass, 0);

	//	TRACE(_T("Send Logout Result - %hs\n"), m_pUserData->m_id);
}

void CUser::MoveProcess(char* pBuf)
{
	if (m_bWarp)
		return;

	C3DMap* pMap = nullptr;
	int index = 0, sendIndex = 0;
	uint16_t will_x = 0, will_z = 0;
	int16_t will_y = 0, speed = 0;
	float real_x = 0.0f, real_z = 0.0f, real_y = 0.0f;
	uint8_t echo = 0;
	char sendBuffer[1024] {};

	will_x = GetShort(pBuf, index);
	will_z = GetShort(pBuf, index);
	will_y = GetShort(pBuf, index);

	speed  = GetShort(pBuf, index);
	echo   = GetByte(pBuf, index);

	real_x = will_x / 10.0f;
	real_z = will_z / 10.0f;
	real_y = will_y / 10.0f;

	pMap   = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	if (!pMap->IsValidPosition(real_x, real_z))
		return;

	//	real_y = pMap->GetHeight(	real_x, real_y, real_z );

	//	if( speed > 60 ) {	// client 에서 이수치보다 크게 보낼때가 많음...
	//		if( m_bSpeedAmount == 100 )
	//			speed = 0;
	//	}

	if (m_bResHpType == USER_DEAD || m_pUserData->m_sHp == 0)
	{
		if (speed != 0)
		{
			spdlog::warn("User::MoveProcess: dead user is moving [charId={} socketId={} "
						 "resHpType={} hp={} speed={} x={} z={}]",
				m_pUserData->m_id, _socketId, m_bResHpType, m_pUserData->m_sHp, speed,
				static_cast<int32_t>(m_pUserData->m_curx),
				static_cast<int32_t>(m_pUserData->m_curz));
		}
	}

	if (speed != 0)
	{
		m_pUserData->m_curx = m_fWill_x; // 가지고 있던 다음좌표를 현재좌표로 셋팅...
		m_pUserData->m_curz = m_fWill_z;
		m_pUserData->m_cury = m_fWill_y;

		m_fWill_x           = real_x; // 다음좌표를 기억....
		m_fWill_z           = real_z;
		m_fWill_y           = real_y;
	}
	else
	{
		m_pUserData->m_curx = m_fWill_x = real_x; // 다음좌표 == 현재 좌표...
		m_pUserData->m_curz = m_fWill_z = real_z;
		m_pUserData->m_cury = m_fWill_y = real_y;
	}

	SetByte(sendBuffer, WIZ_MOVE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetShort(sendBuffer, will_x, sendIndex);
	SetShort(sendBuffer, will_z, sendIndex);
	SetShort(sendBuffer, will_y, sendIndex);
	SetShort(sendBuffer, speed, sendIndex);
	SetByte(sendBuffer, echo, sendIndex);

	RegisterRegion();
	m_pMain->Send_Region(
		sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	pMap->CheckEvent(real_x, real_z, this);

	int ai_send_index = 0;
	char ai_send_buff[256] {};

	SetByte(ai_send_buff, AG_USER_MOVE, ai_send_index);
	SetShort(ai_send_buff, _socketId, ai_send_index);
	SetFloat(ai_send_buff, m_fWill_x, ai_send_index);
	SetFloat(ai_send_buff, m_fWill_z, ai_send_index);
	SetFloat(ai_send_buff, m_fWill_y, ai_send_index);
	SetShort(ai_send_buff, speed, ai_send_index);

	m_pMain->Send_AIServer(m_pUserData->m_bZone, ai_send_buff, ai_send_index);
}

void CUser::UserInOut(uint8_t Type)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	if (Type == USER_OUT)
		pMap->RegionUserRemove(m_RegionX, m_RegionZ, _socketId);
	else
		pMap->RegionUserAdd(m_RegionX, m_RegionZ, _socketId);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_USER_INOUT, sendIndex);
	SetByte(sendBuffer, Type, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);

	if (Type == USER_OUT)
	{
		m_pMain->Send_Region(
			sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, this);

		// AI Server쪽으로 정보 전송..
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, AG_USER_INOUT, sendIndex);
		SetByte(sendBuffer, Type, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
		SetFloat(sendBuffer, m_pUserData->m_curx, sendIndex);
		SetFloat(sendBuffer, m_pUserData->m_curz, sendIndex);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
		return;
	}

	GetUserInfo(sendBuffer, sendIndex);

	//	TRACE(_T("USERINOUT - %d, %hs\n"), _socketId, m_pUserData->m_id);
	m_pMain->Send_Region(
		sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, this);

	// AI Server쪽으로 정보 전송..
	// 이거 않되도 너무 미워하지 마세요 ㅜ.ㅜ
	if (m_bAbnormalType != ABNORMAL_BLINKING)
	{
		sendIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));
		SetByte(sendBuffer, AG_USER_INOUT, sendIndex);
		SetByte(sendBuffer, Type, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
		SetFloat(sendBuffer, m_pUserData->m_curx, sendIndex);
		SetFloat(sendBuffer, m_pUserData->m_curz, sendIndex);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
	}
	//
}

void CUser::Rotate(char* pBuf)
{
	int index = 0, sendIndex = 0;
	char sendBuffer[256] {};

	m_sDirection = GetShort(pBuf, index);

	SetByte(sendBuffer, WIZ_ROTATE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetShort(sendBuffer, m_sDirection, sendIndex);

	m_pMain->Send_Region(
		sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
}

void CUser::Attack(char* pBuf)
{
	std::shared_ptr<CUser> pTUser;
	CNpc* pNpc          = nullptr;
	model::Item* pTable = nullptr;
	int index = 0, sendIndex = 0;
	int tid = -1, damage = 0;
	float delaytime = 0.0f, distance = 0.0f;
	uint8_t type = 0, result = 0;
	char sendBuffer[256] {};

	type      = GetByte(pBuf, index);
	result    = GetByte(pBuf, index);
	tid       = GetShort(pBuf, index);
	// 비러머글 해킹툴 유저 --;
	delaytime = static_cast<float>(GetShort(pBuf, index));
	distance  = static_cast<float>(GetShort(pBuf, index));
	//

	//	delaytime = delaytime / 100.0f;
	//	distance = distance / 10.0f;	// 'Coz the server multiplies it by 10 before they send it to you.

	if (m_bAbnormalType == ABNORMAL_BLINKING)
		return;

	if (m_bResHpType == USER_DEAD || m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::Attack: dead user cannot attack [charId={} resHpType={} hp={}]",
			m_pUserData->m_id, _socketId, m_bResHpType, m_pUserData->m_sHp);
		return;
	}

	// 비러머글 해킹툴 유저 --;
	// This checks if such an item exists.
	pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[RIGHTHAND].nNum);
	if (pTable == nullptr && m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0)
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
		if (!_socketManager->IsValidSocketId(tid))
			return;

		pTUser = m_pMain->GetUserPtr(tid);
		if (pTUser == nullptr || pTUser->m_bResHpType == USER_DEAD
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
			if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE && m_pMain->m_byBattleOpen == SNOW_BATTLE)
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
					result               = 0x02;
					pTUser->m_bResHpType = USER_DEAD;

					// sungyong work : loyalty
					// Something regarding loyalty points.
					if (m_sPartyIndex == -1)
						LoyaltyChange(tid);
					else
						LoyaltyDivide(tid);

					GoldChange(tid, 0);

					// 기범이의 완벽한 보호 코딩!!!
					pTUser->InitType3(); // Init Type 3.....
					pTUser->InitType4(); // Init Type 4.....

					memset(sendBuffer, 0, sizeof(sendBuffer));
					sendIndex = 0;

					// 지휘권한이 있는 유저가 죽는다면,, 지휘 권한 박탈
					if (pTUser->m_pUserData->m_bFame == KNIGHTS_DUTY_CAPTAIN)
					{
						pTUser->m_pUserData->m_bFame = KNIGHTS_DUTY_CHIEF;

						SetByte(sendBuffer, WIZ_AUTHORITY_CHANGE, sendIndex);
						SetByte(sendBuffer, COMMAND_AUTHORITY, sendIndex);
						SetShort(sendBuffer, pTUser->GetSocketID(), sendIndex);
						SetByte(sendBuffer, pTUser->m_pUserData->m_bFame, sendIndex);
						m_pMain->Send_Region(sendBuffer, sendIndex, pTUser->m_pUserData->m_bZone,
							pTUser->m_RegionX, pTUser->m_RegionZ);

						Send(sendBuffer, sendIndex);

						//TRACE(_T("---> UserAttack Dead Captain Deprive - %hs\n"), pTUser->m_pUserData->m_id);

						if (pTUser->m_pUserData->m_bNation == NATION_KARUS)
							m_pMain->Announcement(KARUS_CAPTAIN_DEPRIVE_NOTIFY, NATION_KARUS);
						else if (pTUser->m_pUserData->m_bNation == NATION_ELMORAD)
							m_pMain->Announcement(ELMORAD_CAPTAIN_DEPRIVE_NOTIFY, NATION_ELMORAD);
					}

					pTUser->m_sWhoKilledMe = _socketId; // You killed me, you.....
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
		if (pNpc != nullptr && pNpc->m_NpcState != NPC_DEAD && pNpc->m_iHP > 0)
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
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, AG_ATTACK_REQ, sendIndex);
			SetByte(sendBuffer, type, sendIndex);
			SetByte(sendBuffer, result, sendIndex);
			SetShort(sendBuffer, _socketId, sendIndex);
			SetShort(sendBuffer, tid, sendIndex);
			SetShort(sendBuffer, m_sTotalHit * m_bAttackAmount / 100, sendIndex); // 표시
			SetShort(sendBuffer, m_sTotalAc + m_sACAmount, sendIndex);            // 표시
			SetFloat(sendBuffer, m_fTotalHitRate, sendIndex);
			SetFloat(sendBuffer, m_fTotalEvasionRate, sendIndex);
			SetShort(sendBuffer, m_sItemAc, sendIndex);
			SetByte(sendBuffer, m_bMagicTypeLeftHand, sendIndex);
			SetByte(sendBuffer, m_bMagicTypeRightHand, sendIndex);
			SetShort(sendBuffer, m_sMagicAmountLeftHand, sendIndex);
			SetShort(sendBuffer, m_sMagicAmountRightHand, sendIndex);
			m_pMain->Send_AIServer(
				m_pUserData->m_bZone, sendBuffer, sendIndex); // AI Server쪽으로 정보 전송..
			return;
		}
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_ATTACK, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetShort(sendBuffer, tid, sendIndex);
	m_pMain->Send_Region(
		sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	if (tid < NPC_BAND)
	{
		if (result == 0x02)
		{
			// 유저에게는 바로 데드 패킷을 날림... (한 번 더 보냄, 유령을 없애기 위해서)
			if (pTUser != nullptr)
			{
				pTUser->Send(sendBuffer, sendIndex);
				memset(sendBuffer, 0, sizeof(sendBuffer));

				spdlog::debug(
					"User::Attack: user attack dead [charId={} result={} resHpType={} Hp={}]",
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

	int sendIndex      = 0;
	char sendBuffer[2048] {};

	int x = 0, z = 0;
	//	int map_size = (pMap->m_nMapSize - 1) * pMap->m_fUnitDist ;		// Are you within the map limits?
	//	if (m_pUserData->m_curx >= map_size || m_pUserData->m_curz >= map_size) {

	if (!pMap->IsValidPosition(m_pUserData->m_curx, m_pUserData->m_curz))
	{
		model::Home* pHomeInfo = m_pMain->m_HomeTableMap.GetData(m_pUserData->m_bNation);
		if (pHomeInfo == nullptr)
			return;

		// Battle Zone...
		if (m_pUserData->m_bNation != m_pUserData->m_bZone && m_pUserData->m_bZone > 200)
		{
			x = pHomeInfo->FreeZoneX + myrand(0, pHomeInfo->FreeZoneLX);
			z = pHomeInfo->FreeZoneZ + myrand(0, pHomeInfo->FreeZoneLZ);
		}
		// Specific Lands...
		else if (m_pUserData->m_bNation != m_pUserData->m_bZone && m_pUserData->m_bZone < 3)
		{
			if (m_pUserData->m_bNation == NATION_KARUS)
			{
				x = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
				z = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
			}
			else if (m_pUserData->m_bNation == NATION_ELMORAD)
			{
				x = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
				z = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
			}
			else
			{
				return;
			}
		}
		// Your own nation...
		else
		{
			if (m_pUserData->m_bNation == NATION_KARUS)
			{
				x = pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX);
				z = pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ);
			}
			else if (m_pUserData->m_bNation == NATION_ELMORAD)
			{
				x = pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX);
				z = pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ);
			}
			else
			{
				return;
			}
		}

		m_pUserData->m_curx = static_cast<float>(x);
		m_pUserData->m_curz = static_cast<float>(z);
	}

	SetByte(sendBuffer, WIZ_MYINFO, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString1(sendBuffer, m_pUserData->m_id, sendIndex);

	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curx * 10, sendIndex);
	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curz * 10, sendIndex);
	SetShort(sendBuffer, (int16_t) m_pUserData->m_cury * 10, sendIndex);

	SetByte(sendBuffer, m_pUserData->m_bNation, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bRace, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sClass, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bFace, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bHairColor, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bRank, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bTitle, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bPoints, sendIndex);
	SetDWORD(sendBuffer, m_iMaxExp, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iExp, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iLoyalty, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iLoyaltyMonthly, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bCity, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_bKnights, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bFame, sendIndex);

	if (m_pUserData->m_bKnights != 0)
		pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);

	if (pKnights != nullptr)
	{
		SetShort(sendBuffer, pKnights->m_sAllianceKnights, sendIndex);
		SetByte(sendBuffer, pKnights->m_byFlag, sendIndex);
		SetString1(sendBuffer, pKnights->m_strName, sendIndex);
		SetByte(sendBuffer, pKnights->m_byGrade, sendIndex); // Knights grade
		SetByte(sendBuffer, pKnights->m_byRanking, sendIndex);
		SetShort(sendBuffer, pKnights->m_sMarkVersion, sendIndex);
		SetShort(sendBuffer, pKnights->m_sCape, sendIndex);
		//TRACE(_T("sendmyinfo knights index = %d, kname=%hs, name=%hs\n") , iLength, pKnights->strName, m_pUserData->m_id);
	}
	else
	{
		SetShort(sendBuffer, 0, sendIndex);  // m_sAllianceKnights
		SetByte(sendBuffer, 0, sendIndex);   // m_byFlag
		SetByte(sendBuffer, 0, sendIndex);   // m_strName
		SetByte(sendBuffer, 0, sendIndex);   // m_byGrade
		SetByte(sendBuffer, 0, sendIndex);   // m_byRanking
		SetShort(sendBuffer, 0, sendIndex);  // m_sMarkVerison
		SetShort(sendBuffer, -1, sendIndex); // m_sCape
	}

	SetShort(sendBuffer, m_iMaxHp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
	SetShort(sendBuffer, m_iMaxMp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sMp, sendIndex);
	SetShort(sendBuffer, GetMaxWeightForClient(), sendIndex);
	SetShort(sendBuffer, GetCurrentWeightForClient(), sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bStr, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(m_sItemStr), sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bSta, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(m_sItemSta), sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bDex, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(m_sItemDex), sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bIntel, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(m_sItemIntel), sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bCha, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(m_sItemCham), sendIndex);
	SetShort(sendBuffer, m_sTotalHit, sendIndex);
	SetShort(sendBuffer, m_sTotalAc, sendIndex);
	//	SetShort( sendBuffer, m_sBodyAc+m_sItemAc, sendIndex );		<- 누가 이렇게 해봤어? --;
	SetByte(sendBuffer, m_bFireR, sendIndex);
	SetByte(sendBuffer, m_bColdR, sendIndex);
	SetByte(sendBuffer, m_bLightningR, sendIndex);
	SetByte(sendBuffer, m_bMagicR, sendIndex);
	SetByte(sendBuffer, m_bDiseaseR, sendIndex);
	SetByte(sendBuffer, m_bPoisonR, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	// 이거 나중에 꼭 주석해 --;
	SetByte(sendBuffer, m_pUserData->m_bAuthority, sendIndex);
	//

	SetByte(sendBuffer, m_byKnightsRank, sendIndex);
	SetByte(sendBuffer, m_byPersonalRank, sendIndex);

	for (int i = 0; i < 9; i++)
		SetByte(sendBuffer, m_pUserData->m_bstrSkill[i], sendIndex);

	for (int i = 0; i < SLOT_MAX; i++)
	{
		SetDWORD(sendBuffer, m_pUserData->m_sItemArray[i].nNum, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sItemArray[i].sDuration, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sItemArray[i].sCount, sendIndex);
		SetByte(sendBuffer, m_pUserData->m_sItemArray[i].byFlag, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sItemArray[i].sTimeRemaining, sendIndex);
	}

	for (int i = 0; i < HAVE_MAX; i++)
	{
		SetDWORD(sendBuffer, m_pUserData->m_sItemArray[SLOT_MAX + i].nNum, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sItemArray[SLOT_MAX + i].sDuration, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sItemArray[SLOT_MAX + i].sCount, sendIndex);
		SetByte(sendBuffer, m_pUserData->m_sItemArray[SLOT_MAX + i].byFlag, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sItemArray[SLOT_MAX + i].sTimeRemaining, sendIndex);
	}

	SetByte(sendBuffer, 0,
		sendIndex); // account status (0 = none, 1 = normal prem with expiry in hours, 2 = pc room)
	SetByte(sendBuffer, m_pUserData->m_byPremiumType, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sPremiumTime, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(m_bIsChicken), sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iMannerPoint, sendIndex);

	Send(sendBuffer, sendIndex);

	SetZoneAbilityChange(m_pUserData->m_bZone);

	// AI Server쪽으로 정보 전송..
	int ai_send_index = 0;
	char ai_send_buff[256] {};

	SetByte(ai_send_buff, AG_USER_INFO, ai_send_index);
	SetShort(ai_send_buff, _socketId, ai_send_index);
	SetString2(ai_send_buff, m_pUserData->m_id, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bZone, ai_send_index);
	SetShort(ai_send_buff, m_iZoneIndex, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bNation, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bLevel, ai_send_index);
	SetShort(ai_send_buff, m_pUserData->m_sHp, ai_send_index);
	SetShort(ai_send_buff, m_pUserData->m_sMp, ai_send_index);
	SetShort(ai_send_buff, m_sTotalHit * m_bAttackAmount / 100, ai_send_index); // 표시
	SetShort(ai_send_buff, m_sTotalAc + m_sACAmount, ai_send_index);            // 표시
	SetFloat(ai_send_buff, m_fTotalHitRate, ai_send_index);
	SetFloat(ai_send_buff, m_fTotalEvasionRate, ai_send_index);

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
	int index = 0, chatlen = 0, sendIndex = 0;
	uint8_t type = 0;
	std::shared_ptr<CUser> pUser;
	char chatstr[1024] {}, sendBuffer[1024] {};
	std::string finalstr;

	// this user refused chatting
	if (m_pUserData->m_bAuthority == AUTHORITY_NOCHAT)
		return;

	type    = GetByte(pBuf, index);
	chatlen = GetShort(pBuf, index);
	if (chatlen > 512 || chatlen <= 0)
		return;

	GetString(chatstr, pBuf, chatlen, index);

	if (m_pUserData->m_bAuthority == AUTHORITY_MANAGER && chatstr[0] == '+')
	{
		OperationMessage opMessage(m_pMain, this);
		opMessage.Process(chatstr);
		return;
	}

	if (type == PUBLIC_CHAT || type == ANNOUNCEMENT_CHAT)
	{
		if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
			return;

		finalstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);
	}
	else
	{
		finalstr.assign(chatstr, chatlen);
	}

	SetByte(sendBuffer, WIZ_CHAT, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bNation, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString1(sendBuffer, m_pUserData->m_id, sendIndex);
	SetString2(sendBuffer, finalstr, sendIndex);

	switch (type)
	{
		case GENERAL_CHAT:
			m_pMain->Send_NearRegion(sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX,
				m_RegionZ, m_pUserData->m_curx, m_pUserData->m_curz);
			break;

		case PRIVATE_CHAT:
			// 이건 내가 추가했지롱 :P
			if (m_sPrivateChatUser == _socketId)
				break;

			pUser = m_pMain->GetUserPtr(m_sPrivateChatUser);
			if (pUser == nullptr || pUser->GetState() != CONNECTION_STATE_GAMESTART)
				break;

			pUser->Send(sendBuffer, sendIndex);
			Send(sendBuffer, sendIndex);
			break;

		case PARTY_CHAT:
			m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
			break;

		case SHOUT_CHAT:
			if (m_pUserData->m_sMp < (m_iMaxMp / 5))
				break;

			MSpChange(-(m_iMaxMp / 5));
			m_pMain->Send_Region(sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX,
				m_RegionZ, nullptr, false);
			break;

		case KNIGHTS_CHAT:
			m_pMain->Send_KnightsMember(
				m_pUserData->m_bKnights, sendBuffer, sendIndex, m_pUserData->m_bZone);
			break;

		case PUBLIC_CHAT:
			m_pMain->Send_All(sendBuffer, sendIndex);
			break;

		case COMMAND_CHAT:
			if (m_pUserData->m_bFame == KNIGHTS_DUTY_CAPTAIN) // 지휘권자만 채팅이 되도록
				m_pMain->Send_CommandChat(sendBuffer, sendIndex, m_pUserData->m_bNation, this);
			break;

			//case WAR_SYSTEM_CHAT:
			//	m_pMain->Send_All( sendBuffer, sendIndex );
			//	break;

		default:
			spdlog::error(
				"User::Chat: Unhandled chat type {:02X} [accountName={} characterName={}]", type,
				m_strAccountID, m_pUserData->m_id);
			break;
	}
}

void CUser::SetMaxHp(int iFlag)
{
	model::Coefficient* p_TableCoefficient = m_pMain->m_CoefficientTableMap.GetData(
		m_pUserData->m_sClass);
	if (p_TableCoefficient == nullptr)
		return;

	int temp_sta = 0;
	temp_sta     = m_pUserData->m_bSta + m_sItemSta + m_sStaAmount;
	//	if (temp_sta > 255)
	//		temp_sta = 255;

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE && iFlag == 0)
	{
		m_iMaxHp = 100;
		//TRACE(_T("--> SetMaxHp - name=%hs, max=%d, hp=%d\n"), m_pUserData->m_id, m_iMaxHp, m_pUserData->m_sHp);
	}
	else
	{
		m_iMaxHp = static_cast<int16_t>(
			((p_TableCoefficient->HitPoint * m_pUserData->m_bLevel * m_pUserData->m_bLevel
				 * temp_sta)
				+ (0.1 * m_pUserData->m_bLevel * temp_sta) + (temp_sta / 5.0))
			+ m_sMaxHPAmount + m_sItemMaxHp);

		// 조금 더 hp를 주면 자동으로 hpchange()함수가 실행됨,, 꽁수^^*
		if (iFlag == 1)
			m_pUserData->m_sHp = m_iMaxHp + 20;
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
	model::Coefficient* p_TableCoefficient = m_pMain->m_CoefficientTableMap.GetData(
		m_pUserData->m_sClass);
	if (p_TableCoefficient == nullptr)
		return;

	int temp_intel = 0, temp_sta = 0;
	temp_intel = m_pUserData->m_bIntel + m_sItemIntel + m_sIntelAmount + 30;
	//	if (temp_intel > 255)
	//		temp_intel = 255;

	temp_sta   = m_pUserData->m_bSta + m_sItemSta + m_sStaAmount;
	//	if (temp_sta > 255)
	//		temp_sta = 255;

	if (p_TableCoefficient->ManaPoint != 0)
	{
		m_iMaxMp  = static_cast<int16_t>((p_TableCoefficient->ManaPoint * m_pUserData->m_bLevel
                                            * m_pUserData->m_bLevel * temp_intel)
										 + (0.1 * m_pUserData->m_bLevel * 2 * temp_intel)
										 + (temp_intel / 5.0));
		m_iMaxMp += m_sItemMaxMp;
		m_iMaxMp += 20; // 성래씨 요청
	}
	else if (p_TableCoefficient->Sp != 0)
	{
		m_iMaxMp = static_cast<int16_t>(
			(p_TableCoefficient->Sp * m_pUserData->m_bLevel * m_pUserData->m_bLevel * temp_sta)
			+ (0.1 * m_pUserData->m_bLevel * temp_sta) + (temp_sta / 5.0));
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

	_OBJECT_EVENT* pEvent    = nullptr;
	model::Home* pHomeInfo   = nullptr;
	model::MagicType5* pType = nullptr;
	C3DMap* pMap             = nullptr;

	int index                = 0;
	uint8_t regene_type      = GetByte(pBuf, index);

	if (regene_type != 1 && regene_type != 2)
		regene_type = 1;

	if (regene_type == 2)
	{
		magicid = 490041; // The Stone of Resurrection magic ID

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

	int sendIndex = 0;
	char sendBuffer[1024] {};

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	UserInOut(USER_OUT); // 원래는 이 한줄밖에 없었음 --;

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
		if (pEvent != nullptr && pEvent->byLife == 1)
		{
			m_pUserData->m_curx = m_fWill_x = pEvent->fPosX + x;
			m_pUserData->m_curz = m_fWill_z = pEvent->fPosZ + z;
			m_pUserData->m_cury             = 0;
		}
		// Free Zone or Opposite Zone
		else if (m_pUserData->m_bNation != m_pUserData->m_bZone)
		{
			// Frontier Zone...
			if (m_pUserData->m_bZone > 200)
			{
				x = static_cast<float>(pHomeInfo->FreeZoneX + myrand(0, pHomeInfo->FreeZoneLX));
				z = static_cast<float>(pHomeInfo->FreeZoneZ + myrand(0, pHomeInfo->FreeZoneLZ));
			}
			//
			// Battle Zone...
			else if (m_pUserData->m_bZone > 100 && m_pUserData->m_bZone < 200)
			{
				/*
				m_bResHpType = USER_STANDING;
				HpChange( m_iMaxHp );
				KickOutZoneUser();	// Go back to your own zone!
				return;
*/
				x = static_cast<float>(pHomeInfo->BattleZoneX + myrand(0, pHomeInfo->BattleZoneLX));
				z = static_cast<float>(pHomeInfo->BattleZoneZ + myrand(0, pHomeInfo->BattleZoneLZ));
				// 비러머글 개척존 바꾸어치기 >.<
				if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
				{
					x = static_cast<float>(pHomeInfo->FreeZoneX + myrand(0, pHomeInfo->FreeZoneLX));
					z = static_cast<float>(pHomeInfo->FreeZoneZ + myrand(0, pHomeInfo->FreeZoneLZ));
				}
				//
			}
			// 비러머글 뉴존 >.<
			else if (m_pUserData->m_bZone > 10 && m_pUserData->m_bZone < 20)
			{
				x = static_cast<float>(527 + myrand(0, 10));
				z = static_cast<float>(543 + myrand(0, 10));
			}
			//
			// Specific Lands...
			else if (m_pUserData->m_bZone < 3)
			{
				if (m_pUserData->m_bNation == NATION_KARUS)
				{
					x = static_cast<float>(pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX));
					z = static_cast<float>(pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ));
				}
				else if (m_pUserData->m_bNation == NATION_ELMORAD)
				{
					x = static_cast<float>(
						pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX));
					z = static_cast<float>(
						pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ));
				}
				else
				{
					return;
				}
			}
			else
			{
				int16_t spawnX = 0, spawnZ = 0;
				if (GetStartPosition(&spawnX, &spawnZ, m_pUserData->m_bZone))
				{
					x = static_cast<float>(spawnX);
					z = static_cast<float>(spawnZ);
				}
			}

			m_pUserData->m_curx = x;
			m_pUserData->m_curz = z;
		}
		//  추후에 Warp 랑 합쳐야 할것 같음...
		else
		{
			if (m_pUserData->m_bNation == NATION_KARUS)
			{
				x = static_cast<float>(pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX));
				z = static_cast<float>(pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ));
			}
			else if (m_pUserData->m_bNation == NATION_ELMORAD)
			{
				x = static_cast<float>(pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX));
				z = static_cast<float>(pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ));
			}
			else
			{
				return;
			}

			m_pUserData->m_curx = x;
			m_pUserData->m_curz = z;
		}
	}

	SetByte(sendBuffer, WIZ_REGENE, sendIndex);
	//	SetShort( sendBuffer, _socketId, sendIndex );    //
	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curx * 10, sendIndex);
	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curz * 10, sendIndex);
	SetShort(sendBuffer, (int16_t) m_pUserData->m_cury * 10, sendIndex);
	Send(sendBuffer, sendIndex);
	//	m_pMain->Send_Region( sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false ); //

	// Clerical Resurrection.
	if (magicid > 0)
	{
		pType = m_pMain->m_MagicType5TableMap.GetData(magicid);
		if (pType == nullptr)
			return;

		m_bAbnormalType   = ABNORMAL_BLINKING;
		m_bResHpType      = USER_STANDING;
		m_fBlinkStartTime = TimeGet();
		MSpChange(-m_iMaxMp);                                  // Empty out MP.

		if (m_sWhoKilledMe == -1 && regene_type == 1)
			ExpChange((m_iLostExp * pType->ExpRecover) / 100); // Restore Target Experience.

		m_bRegeneType = REGENE_MAGIC;
	}
	// Normal Regene.
	else
	{
		m_bAbnormalType   = ABNORMAL_BLINKING;
		m_fBlinkStartTime = TimeGet();
		//
		m_bResHpType      = USER_STANDING;
		HpChange(m_iMaxHp);
		m_bRegeneType = REGENE_NORMAL;
	}

	//	비러머글 클랜 소환!!!
	m_fLastRegeneTime = TimeGet();
	//
	m_sWhoKilledMe    = -1;
	m_iLostExp        = 0;
	//
	if (m_bAbnormalType != ABNORMAL_BLINKING)
	{
		// AI_server로 regene정보 전송...
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, AG_USER_REGENE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
	}
	//

	// 이 sendIndex는 왜 없었을까??? --;
#if defined(_DEBUG)
	{
		//TCHAR logstr[1024] {};
		//_stprintf(logstr, _T("<------ User Regene ,, nid=%d, name=%hs, type=%d ******"), _socketId, m_pUserData->m_id, m_bResHpType);
		//TimeTrace(logstr);
	}
#endif

	m_RegionX = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	m_RegionZ = (int) (m_pUserData->m_curz / VIEW_DISTANCE);

	UserInOut(USER_REGENE);

	m_pMain->RegionUserInOutForMe(this);
	m_pMain->RegionNpcInfoForMe(this);

	// Send Blinking State Change.....
	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, 3, sendIndex);
	SetByte(sendBuffer, m_bAbnormalType, sendIndex);
	StateChange(sendBuffer);

	// Send Party Packet.....
	if (m_sPartyIndex != -1 && !m_bType3Flag)
	{
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_STATUSCHANGE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetByte(sendBuffer, 1, sendIndex);
		SetByte(sendBuffer, 0x00, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
	}
	//  end of Send Party Packet.....  //
	//
	//
	// Send Party Packet for Type 4
	if (m_sPartyIndex != -1 && !m_bType4Flag)
	{
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_STATUSCHANGE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetByte(sendBuffer, 2, sendIndex);
		SetByte(sendBuffer, 0x00, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
	}
	//  end of Send Party Packet.....  //
	//
}

void CUser::ZoneChange(int zone, float x, float z)
{
	int sendIndex = 0, zoneindex = 0;
	char sendBuffer[128] {};
	C3DMap* pMap            = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;

	if (g_serverdown_flag)
		return;

	// This is unofficial behaviour, but official behaviour causes players
	// to be unintentionally reset back to 0,0 which is never desired, so we'll
	// ensure players get returned to the intended /town positions.
	if (static_cast<int>(x) == 0 && static_cast<int>(z) == 0)
	{
		int16_t sx = 0, sz = 0;
		if (!GetStartPosition(&sx, &sz, zone))
		{
			spdlog::warn("User::ZoneChange: failed to fetch start position for zone {} "
						 "[accountId={} characterName={}]",
				zone, m_pUserData->m_Accountid, m_pUserData->m_id);
			return;
		}

		x = static_cast<float>(sx);
		z = static_cast<float>(sz);
	}

	// Zone changes within the same zone should just warp instead.
	// This eliminates a redundant full zone reload.
	// Officially this is applied inconsistently per case (calling Warp() instead of ZoneChange()),
	// but we'll just make this consistent.
	if (zone == m_pUserData->m_bZone)
	{
		Warp(x, z);
		return;
	}

	zoneindex = m_pMain->GetZoneIndex(zone);

	pMap      = m_pMain->GetMapByIndex(zoneindex);
	if (pMap == nullptr)
		return;

	// If Target zone is frontier zone.
	if (pMap->m_bType == 2)
	{
		if (m_pUserData->m_bLevel < 20 && m_pMain->m_byBattleOpen != SNOW_BATTLE)
			return;
	}

	// Battle zone open
	if (m_pMain->m_byBattleOpen == NATION_BATTLE)
	{
		if (m_pUserData->m_bZone == ZONE_BATTLE)
		{
			// 상대방 국가로 못넘어 가게..
			if (pMap->m_bType == 1 && m_pUserData->m_bNation != zone)
			{
				if (m_pUserData->m_bNation == NATION_KARUS && !m_pMain->m_byElmoradOpenFlag)
				{
					spdlog::error("User::ZoneChange: zone not open for invasion [charId={} "
								  "nation={} enemyNationOpen={}]",
						m_pUserData->m_id, m_pUserData->m_bNation, m_pMain->m_byElmoradOpenFlag);
					return;
				}

				if (m_pUserData->m_bNation == NATION_ELMORAD && !m_pMain->m_byKarusOpenFlag)
				{
					spdlog::error("User::ZoneChange: zone not open for invasion [charId={} "
								  "nation={} enemyNationOpen={}]",
						m_pUserData->m_id, m_pUserData->m_bNation, m_pMain->m_byKarusOpenFlag);
					return;
				}
			}
		}
		// 상대방 국가로 못넘어 가게..
		else if (pMap->m_bType == 1 && m_pUserData->m_bNation != zone)
		{
			return;
		}
		//
		// You can't go to frontier zone when Battlezone is open.
		else if (pMap->m_bType == 2 && zone == ZONE_FRONTIER)
		{
			// 비러머글 마을 도착 공지....
			int temp_index = 0;
			char temp_buff[128] {};

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
		if (pMap->m_bType == 1 && m_pUserData->m_bNation != zone)
			return;

		// You can't go to frontier zone when Battlezone is open.
		if (pMap->m_bType == 2 && (zone == ZONE_FRONTIER || zone == ZONE_BATTLE))
			return;
	}
	// Battle zone close
	else
	{
		// 상대방 국가로 못넘어 가게..
		if (pMap->m_bType == 1)
		{
			if (m_pUserData->m_bNation != zone && zone > ZONE_MORADON && zone != ZONE_ARENA)
				return;

			if (m_pUserData->m_bNation != zone && zone < 3)
				return;
		}
	}

	m_bZoneChangeFlag = true;
	m_bWarp           = 0x01;

	UserInOut(USER_OUT);

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
	{
		//TRACE(_T("ZoneChange - name=%hs\n"), m_pUserData->m_id);
		SetMaxHp(1);
	}

	if (m_pUserData->m_bZone != zone)
		SetZoneAbilityChange(zone);

	m_iZoneIndex         = zoneindex;
	m_pUserData->m_bZone = zone;
	m_pUserData->m_curx = m_fWill_x = x;
	m_pUserData->m_curz = m_fWill_z = z;

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
	{
		//TRACE(_T("ZoneChange - name=%hs\n"), m_pUserData->m_id);
		SetMaxHp();
	}

	PartyRemove(_socketId); // 파티에서 탈퇴되도록 처리

	//TRACE(_T("ZoneChange ,,, id=%hs, nation=%d, zone=%d, x=%.2f, z=%.2f\n"), m_pUserData->m_id, m_pUserData->m_bNation, zone, x, z);

	if (m_pMain->m_nServerNo != pMap->m_nServerNo)
	{
		pInfo = m_pMain->m_ServerArray.GetData(pMap->m_nServerNo);
		if (pInfo == nullptr)
			return;

		UserDataSaveToAgent();

		spdlog::debug("User::ZoneChange: [userId={} accountId={} charId={} zoneId={} x={} z={}]",
			_socketId, m_strAccountID, m_pUserData->m_id, zone, static_cast<int32_t>(x),
			static_cast<int32_t>(z));

		m_pUserData->m_bLogout = 2; // server change flag

		SetByte(sendBuffer, WIZ_SERVER_CHANGE, sendIndex);
		SetString2(sendBuffer, pInfo->strServerIP, sendIndex);
		SetShort(sendBuffer, pInfo->sPort, sendIndex);
		SetByte(sendBuffer, 0x02, sendIndex); // 중간에 서버가 바뀌는 경우...
		SetByte(sendBuffer, m_pUserData->m_bZone, sendIndex);
		SetByte(sendBuffer, m_pMain->m_byOldVictory, sendIndex);
		Send(sendBuffer, sendIndex);
		return;
	}

	m_pUserData->m_sBind = -1; // Bind Point Clear...

	m_RegionX            = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	m_RegionZ            = (int) (m_pUserData->m_curz / VIEW_DISTANCE);

	SetByte(sendBuffer, WIZ_ZONE_CHANGE, sendIndex);
	SetByte(sendBuffer, ZONE_CHANGE_TELEPORT, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bZone, sendIndex);
	SetByte(sendBuffer, 0, sendIndex); // subzone
	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curx * 10, sendIndex);
	SetShort(sendBuffer, (uint16_t) m_pUserData->m_curz * 10, sendIndex);
	SetShort(sendBuffer, (int16_t) m_pUserData->m_cury * 10, sendIndex);
	SetByte(sendBuffer, m_pMain->m_byOldVictory, sendIndex);
	Send(sendBuffer, sendIndex);

	// 비러머글 순간이동 >.<
	if (!m_bZoneChangeSameZone)
	{
		m_sWhoKilledMe       = -1;
		m_iLostExp           = 0;
		m_bRegeneType        = 0;
		m_fLastRegeneTime    = 0.0;
		m_pUserData->m_sBind = -1;
		InitType3();
		InitType4();
	}

	m_bZoneChangeSameZone = false;
	//
	int ai_send_index     = 0;
	char ai_send_buff[256] {};
	SetByte(ai_send_buff, AG_ZONE_CHANGE, ai_send_index);
	SetShort(ai_send_buff, _socketId, ai_send_index);
	SetByte(ai_send_buff, (uint8_t) m_iZoneIndex, ai_send_index);
	SetByte(ai_send_buff, m_pUserData->m_bZone, ai_send_index);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, ai_send_buff, ai_send_index);

	m_bZoneChangeFlag = false;
}

void CUser::Warp(char* pBuf)
{
	int index       = 0;
	uint16_t warp_x = 0, warp_z = 0;

	warp_x = GetShort(pBuf, index);
	warp_z = GetShort(pBuf, index);

	Warp(warp_x / 10.0f, warp_z / 10.0f);
}

void CUser::Warp(float x, float z)
{
	if (m_bWarp)
		return;

	//	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
	//		return;

	C3DMap* pMap  = nullptr;
	int sendIndex = 0;
	char sendBuffer[128] {};

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	if (!pMap->IsValidPosition(x, z))
		return;

	SetByte(sendBuffer, WIZ_WARP, sendIndex);
	SetShort(sendBuffer, static_cast<uint16_t>(x * 10.0f), sendIndex);
	SetShort(sendBuffer, static_cast<uint16_t>(z * 10.0f), sendIndex);
	Send(sendBuffer, sendIndex);

	UserInOut(USER_OUT);

	m_pUserData->m_curx = x;
	m_fWill_x           = x;
	m_pUserData->m_curz = z;
	m_fWill_z           = z;

	m_RegionX           = (int) (m_pUserData->m_curx / VIEW_DISTANCE);
	m_RegionZ           = (int) (m_pUserData->m_curz / VIEW_DISTANCE);

	//TRACE(_T(" Warp ,, name=%hs, x=%.2f, z=%.2f\n"), m_pUserData->m_id, m_pUserData->m_curx, m_pUserData->m_curz);

	//UserInOut( USER_IN );
	UserInOut(USER_WARP);
	m_pMain->RegionUserInOutForMe(this);
	m_pMain->RegionNpcInfoForMe(this);
}

void CUser::SendTimeStatus()
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	SetByte(sendBuffer, WIZ_TIME, sendIndex);
	SetShort(sendBuffer, m_pMain->m_nYear, sendIndex);
	SetShort(sendBuffer, m_pMain->m_nMonth, sendIndex);
	SetShort(sendBuffer, m_pMain->m_nDate, sendIndex);
	SetShort(sendBuffer, m_pMain->m_nHour, sendIndex);
	SetShort(sendBuffer, m_pMain->m_nMin, sendIndex);
	Send(sendBuffer, sendIndex);

	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, WIZ_WEATHER, sendIndex);
	SetByte(sendBuffer, (uint8_t) m_pMain->m_nWeather, sendIndex);
	SetShort(sendBuffer, m_pMain->m_nAmount, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::SetDetailData()
{
	SetSlotItemValue();
	SetUserAbility();

	if (m_pUserData->m_bLevel > MAX_LEVEL)
	{
		spdlog::error(
			"User::SetDetailData: user exceeds max level [accountId={} charId={} level={}]",
			m_pUserData->m_Accountid, m_pUserData->m_id, m_pUserData->m_bLevel);

		Close();
		return;
	}

	m_iMaxExp    = m_pMain->m_LevelUpTableArray[m_pUserData->m_bLevel - 1]->RequiredExp;
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

	if (m_RegionX != iRegX || m_RegionZ != iRegZ)
	{
		C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
		if (pMap == nullptr)
			return;

		old_region_x = m_RegionX;
		old_region_z = m_RegionZ;

		pMap->RegionUserRemove(m_RegionX, m_RegionZ, _socketId);
		m_RegionX = iRegX;
		m_RegionZ = iRegZ;
		pMap->RegionUserAdd(m_RegionX, m_RegionZ, _socketId);

		if (_state == CONNECTION_STATE_GAMESTART)
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
	int sendIndex = 0;
	char sendBuffer[256] {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	SetByte(sendBuffer, WIZ_USER_INOUT, sendIndex);
	SetByte(sendBuffer, USER_OUT, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);

	// x 축으로 이동되었을때...
	if (del_x != 0)
	{
		m_pMain->Send_UnitRegion(
			pMap, sendBuffer, sendIndex, m_RegionX + del_x * 2, m_RegionZ + del_z - 1);
		m_pMain->Send_UnitRegion(
			pMap, sendBuffer, sendIndex, m_RegionX + del_x * 2, m_RegionZ + del_z);
		m_pMain->Send_UnitRegion(
			pMap, sendBuffer, sendIndex, m_RegionX + del_x * 2, m_RegionZ + del_z + 1);

		// TRACE(_T("Remove : (%d %d), (%d %d), (%d %d)\n"), m_RegionX+del_x*2, m_RegionZ+del_z-1, m_RegionX+del_x*2, m_RegionZ+del_z, m_RegionX+del_x*2, m_RegionZ+del_z+1 );
	}

	// z 축으로 이동되었을때...
	if (del_z != 0)
	{
		m_pMain->Send_UnitRegion(
			pMap, sendBuffer, sendIndex, m_RegionX + del_x, m_RegionZ + del_z * 2);

		// x, z 축 둘다 이동되었을때 겹치는 부분 한번만 보낸다..
		if (del_x < 0)
		{
			m_pMain->Send_UnitRegion(
				pMap, sendBuffer, sendIndex, m_RegionX + del_x + 1, m_RegionZ + del_z * 2);
		}
		else if (del_x > 0)
		{
			m_pMain->Send_UnitRegion(
				pMap, sendBuffer, sendIndex, m_RegionX + del_x - 1, m_RegionZ + del_z * 2);
		}
		else
		{
			m_pMain->Send_UnitRegion(
				pMap, sendBuffer, sendIndex, m_RegionX + del_x - 1, m_RegionZ + del_z * 2);
			m_pMain->Send_UnitRegion(
				pMap, sendBuffer, sendIndex, m_RegionX + del_x + 1, m_RegionZ + del_z * 2);

			// TRACE(_T("Remove : (%d %d), (%d %d), (%d %d)\n"), m_RegionX+del_x-1, m_RegionZ+del_z*2, m_RegionX+del_x, m_RegionZ+del_z*2, m_RegionX+del_x+1, m_RegionZ+del_z*2 );
		}
	}
}

void CUser::InsertRegion(int del_x, int del_z)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	SetByte(sendBuffer, WIZ_USER_INOUT, sendIndex);
	SetByte(sendBuffer, USER_IN, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	GetUserInfo(sendBuffer, sendIndex);

	// x 축으로 이동되었을때...
	if (del_x != 0)
	{
		m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX + del_x, m_RegionZ - 1);
		m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX + del_x, m_RegionZ);
		m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX + del_x, m_RegionZ + 1);

		// TRACE(_T("Insert : (%d %d), (%d %d), (%d %d)\n"), m_RegionX+del_x, m_RegionZ-1, m_RegionX+del_x, m_RegionZ, m_RegionX+del_x, m_RegionZ+1 );
	}

	// z 축으로 이동되었을때...
	if (del_z != 0)
	{
		m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX, m_RegionZ + del_z);

		// x, z 축 둘다 이동되었을때 겹치는 부분 한번만 보낸다..
		if (del_x < 0)
		{
			m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX + 1, m_RegionZ + del_z);
		}
		else if (del_x > 0)
		{
			m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX - 1, m_RegionZ + del_z);
		}
		else
		{
			m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX - 1, m_RegionZ + del_z);
			m_pMain->Send_UnitRegion(pMap, sendBuffer, sendIndex, m_RegionX + 1, m_RegionZ + del_z);

			// TRACE(_T("Insert : (%d %d), (%d %d), (%d %d)\n"), m_RegionX-1, m_RegionZ+del_z, m_RegionX, m_RegionZ+del_z, m_RegionX+1, m_RegionZ+del_z );
		}
	}
}

void CUser::RequestUserIn(char* pBuf)
{
	int index = 0, user_count = 0, sendIndex = 0, t_count = 0;
	char sendBuffer[40960] {};

	sendIndex  = 3; // packet command 와 user_count 는 나중에 셋팅한다...
	user_count = GetShort(pBuf, index);
	for (int i = 0; i < user_count; i++)
	{
		int16_t uid = GetShort(pBuf, index);
		if (i > 1000)
			break;

		auto pUser = m_pMain->GetUserPtr(uid);
		if (pUser == nullptr || pUser->GetState() != CONNECTION_STATE_GAMESTART)
			continue;

		SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
		pUser->GetUserInfo(sendBuffer, sendIndex);

		++t_count;
	}

	int temp_index = 0;
	SetByte(sendBuffer, WIZ_REQ_USERIN, temp_index);
	SetShort(sendBuffer, t_count, temp_index);

	if (sendIndex < 500)
		Send(sendBuffer, sendIndex);
	else
		SendCompressingPacket(sendBuffer, sendIndex);
}

void CUser::RequestNpcIn(char* pBuf)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return;

	int index = 0, npc_count = 0, sendIndex = 0, t_count = 0;
	char sendBuffer[20480] {};

	sendIndex = 3; // packet command 와 user_count 는 나중에 셋팅한다...
	npc_count = GetShort(pBuf, index);
	for (int i = 0; i < npc_count; i++)
	{
		int16_t nid = GetShort(pBuf, index);
		if (nid < 0 || nid > NPC_BAND + NPC_BAND)
			continue;

		if (i > 1000)
			break;

		CNpc* pNpc = m_pMain->m_NpcMap.GetData(nid);
		if (pNpc == nullptr)
			continue;

		// 음냥,,
		// if (pNpc->m_NpcState != NPC_LIVE)
		//	continue;

		SetShort(sendBuffer, pNpc->m_sNid, sendIndex);
		pNpc->GetNpcInfo(sendBuffer, sendIndex);

		++t_count;
	}

	int temp_index = 0;
	SetByte(sendBuffer, WIZ_REQ_NPCIN, temp_index);
	SetShort(sendBuffer, t_count, temp_index);

	if (sendIndex < 500)
		Send(sendBuffer, sendIndex);
	else
		SendCompressingPacket(sendBuffer, sendIndex);
}

uint8_t CUser::GetHitRate(float rate)
{
	uint8_t result = FAIL;
	int random     = myrand(1, 10000);

	if (rate >= 5.0f)
	{
		if (random >= 1 && random <= 3500)
			result = GREAT_SUCCESS;
		else if (random >= 3501 && random <= 7500)
			result = SUCCESS;
		else if (random >= 7501 && random <= 9800)
			result = NORMAL;
	}
	else if (rate >= 3.0f)
	{
		if (random >= 1 && random <= 2500)
			result = GREAT_SUCCESS;
		else if (random >= 2501 && random <= 6000)
			result = SUCCESS;
		else if (random >= 6001 && random <= 9600)
			result = NORMAL;
	}
	else if (rate >= 2.0f)
	{
		if (random >= 1 && random <= 2000)
			result = GREAT_SUCCESS;
		else if (random >= 2001 && random <= 5000)
			result = SUCCESS;
		else if (random >= 5001 && random <= 9400)
			result = NORMAL;
	}
	else if (rate >= 1.25f)
	{
		if (random >= 1 && random <= 1500)
			result = GREAT_SUCCESS;
		else if (random >= 1501 && random <= 4000)
			result = SUCCESS;
		else if (random >= 4001 && random <= 9200)
			result = NORMAL;
	}
	else if (rate >= 0.8f)
	{
		if (random >= 1 && random <= 1000)
			result = GREAT_SUCCESS;
		else if (random >= 1001 && random <= 3000)
			result = SUCCESS;
		else if (random >= 3001 && random <= 9000)
			result = NORMAL;
	}
	else if (rate >= 0.5f)
	{
		if (random >= 1 && random <= 800)
			result = GREAT_SUCCESS;
		else if (random >= 801 && random <= 2500)
			result = SUCCESS;
		else if (random >= 2501 && random <= 8000)
			result = NORMAL;
	}
	else if (rate >= 0.33f)
	{
		if (random >= 1 && random <= 600)
			result = GREAT_SUCCESS;
		else if (random >= 601 && random <= 2000)
			result = SUCCESS;
		else if (random >= 2001 && random <= 7000)
			result = NORMAL;
	}
	else if (rate >= 0.2f)
	{
		if (random >= 1 && random <= 400)
			result = GREAT_SUCCESS;
		else if (random >= 401 && random <= 1500)
			result = SUCCESS;
		else if (random >= 1501 && random <= 6000)
			result = NORMAL;
	}
	else
	{
		if (random >= 1 && random <= 200)
			result = GREAT_SUCCESS;
		else if (random >= 201 && random <= 1000)
			result = SUCCESS;
		else if (random >= 1001 && random <= 5000)
			result = NORMAL;
	}

	return result;
}

// 착용한 아이템의 값(타격률, 회피율, 데미지)을 구한다.
void CUser::SetSlotItemValue()
{
	model::Item* pTable = nullptr;
	int item_hit = 0, item_ac = 0;

	m_sItemHit = m_sItemAc = m_sItemStr = m_sItemSta = m_sItemDex = m_sItemIntel = m_sItemCham = 0;
	m_sItemHitrate     = 100;
	m_sItemEvasionrate = 100;
	m_iItemWeight      = 0;
	m_sItemMaxHp       = 0;
	m_sItemMaxMp       = 0;

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
			item_ac  = pTable->Armor / 2;
		}
		else
		{
			item_hit = pTable->Damage;
			item_ac  = pTable->Armor;
		}

		if (i == RIGHTHAND) // ItemHit Only Hands
			m_sItemHit += item_hit;

		if (i == LEFTHAND)
		{
			if ((m_pUserData->m_sClass == BERSERKER || m_pUserData->m_sClass == BLADE))
				//			m_sItemHit += item_hit * (double) (m_pUserData->m_bstrSkill[PRO_SKILL1] / 60.0);    // 성래씨 요청 ^^;
				m_sItemHit += static_cast<int16_t>(item_hit * 0.5f);
		}

		m_sItemMaxHp       += pTable->MaxHpBonus;
		m_sItemMaxMp       += pTable->MaxMpBonus;
		m_sItemAc          += item_ac;
		m_sItemStr         += pTable->StrengthBonus;
		m_sItemSta         += pTable->StaminaBonus;
		m_sItemDex         += pTable->DexterityBonus;
		m_sItemIntel       += pTable->IntelligenceBonus;
		m_sItemCham        += pTable->CharismaBonus;
		m_sItemHitrate     += pTable->HitRate;
		m_sItemEvasionrate += pTable->EvasionRate;
		//		m_iItemWeight += pTable->Weight;

		m_bFireR           += pTable->FireResist;
		m_bColdR           += pTable->ColdResist;
		m_bLightningR      += pTable->LightningResist;
		m_bMagicR          += pTable->MagicResist;
		m_bDiseaseR        += pTable->CurseResist;
		m_bPoisonR         += pTable->PoisonResist;

		m_sDaggerR         += pTable->DaggerArmor;
		m_sSwordR          += pTable->SwordArmor;
		m_sAxeR            += pTable->AxeArmor;
		m_sMaceR           += pTable->MaceArmor;
		m_sSpearR          += pTable->SpearArmor;
		m_sBowR            += pTable->BowArmor;
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
	model::Item* pLeftHand = m_pMain->m_ItemTableMap.GetData(
		m_pUserData->m_sItemArray[LEFTHAND].nNum);
	if (pLeftHand != nullptr)
	{
		if (pLeftHand->FireDamage != 0)
		{
			m_bMagicTypeLeftHand   = 1;
			m_sMagicAmountLeftHand = pLeftHand->FireDamage;
		}

		if (pLeftHand->IceDamage != 0)
		{
			m_bMagicTypeLeftHand   = 2;
			m_sMagicAmountLeftHand = pLeftHand->IceDamage;
		}

		if (pLeftHand->LightningDamage != 0)
		{
			m_bMagicTypeLeftHand   = 3;
			m_sMagicAmountLeftHand = pLeftHand->LightningDamage;
		}

		if (pLeftHand->PoisonDamage != 0)
		{
			m_bMagicTypeLeftHand   = 4;
			m_sMagicAmountLeftHand = pLeftHand->PoisonDamage;
		}

		if (pLeftHand->HpDrain != 0)
		{
			m_bMagicTypeLeftHand   = 5;
			m_sMagicAmountLeftHand = pLeftHand->HpDrain;
		}

		if (pLeftHand->MpDamage != 0)
		{
			m_bMagicTypeLeftHand   = 6;
			m_sMagicAmountLeftHand = pLeftHand->MpDamage;
		}

		if (pLeftHand->MpDrain != 0)
		{
			m_bMagicTypeLeftHand   = 7;
			m_sMagicAmountLeftHand = pLeftHand->MpDrain;
		}

		if (pLeftHand->MirrorDamage != 0)
		{
			m_bMagicTypeLeftHand   = 8;
			m_sMagicAmountLeftHand = pLeftHand->MirrorDamage;
		}
	}

	// Get item info for right hand.
	model::Item* pRightHand = m_pMain->m_ItemTableMap.GetData(
		m_pUserData->m_sItemArray[RIGHTHAND].nNum);
	if (pRightHand != nullptr)
	{
		if (pRightHand->FireDamage != 0)
		{
			m_bMagicTypeRightHand   = 1;
			m_sMagicAmountRightHand = pRightHand->FireDamage;
		}

		if (pRightHand->IceDamage != 0)
		{
			m_bMagicTypeRightHand   = 2;
			m_sMagicAmountRightHand = pRightHand->IceDamage;
		}

		if (pRightHand->LightningDamage != 0)
		{
			m_bMagicTypeRightHand   = 3;
			m_sMagicAmountRightHand = pRightHand->LightningDamage;
		}

		if (pRightHand->PoisonDamage != 0)
		{
			m_bMagicTypeRightHand   = 4;
			m_sMagicAmountRightHand = pRightHand->PoisonDamage;
		}

		if (pRightHand->HpDrain != 0)
		{
			m_bMagicTypeRightHand   = 5;
			m_sMagicAmountRightHand = pRightHand->HpDrain;
		}

		if (pRightHand->MpDamage != 0)
		{
			m_bMagicTypeRightHand   = 6;
			m_sMagicAmountRightHand = pRightHand->MpDamage;
		}

		if (pRightHand->MpDrain != 0)
		{
			m_bMagicTypeRightHand   = 7;
			m_sMagicAmountRightHand = pRightHand->MpDrain;
		}

		if (pRightHand->MirrorDamage != 0)
		{
			m_bMagicTypeRightHand   = 8;
			m_sMagicAmountRightHand = pRightHand->MirrorDamage;
		}
	}
}

int16_t CUser::GetDamage(int tid, int magicid)
{
	model::Magic* pTable = nullptr;
	int random           = 0;
	uint8_t result       = FAIL;
	int16_t temp_hit = 0, temp_ac = 0, temp_hit_B = 0, damage = 0;

	auto pTUser = m_pMain->GetUserPtr(tid);
	if (pTUser == nullptr || pTUser->m_bResHpType == USER_DEAD)
		return -1;

	temp_ac    = pTUser->m_sTotalAc + pTUser->m_sACAmount;                      // 표시
	temp_hit_B = (m_sTotalHit * m_bAttackAmount * 200 / 100) / (temp_ac + 240); // 표시

	// Skill/Arrow hit.
	if (magicid > 0)
	{
		// Get main magic table.
		pTable = m_pMain->m_MagicTableMap.GetData(magicid);
		if (pTable == nullptr)
			return -1;

		// SKILL HIT!
		if (pTable->Type1 == 1)
		{
			// Get magic skill table type 1.
			model::MagicType1* pType1 = m_pMain->m_MagicType1TableMap.GetData(magicid);
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
				result = GetHitRate((m_fTotalHitRate / pTUser->m_fTotalEvasionRate)
									* (pType1->HitRateMod / 100.0f));
			}

			temp_hit = static_cast<int16_t>(temp_hit_B * (pType1->DamageMod / 100.0f));
		}
		// ARROW HIT!
		else if (pTable->Type1 == 2)
		{
			// Get magic skill table type 2.
			model::MagicType2* pType2 = m_pMain->m_MagicType2TableMap.GetData(magicid);
			if (pType2 == nullptr)
				return -1;

			// Non-relative/Penetration hit.
			if (pType2->HitType == 1 || pType2->HitType == 2)
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
				result = GetHitRate((m_fTotalHitRate / pTUser->m_fTotalEvasionRate)
									* (pType2->HitRateMod / 100.0f));
			}

			if (pType2->HitType == 1
				/*|| pType2->HitType == 2*/)
				temp_hit = static_cast<int16_t>(
					m_sTotalHit * m_bAttackAmount * (pType2->DamageMod / 100.0f) / 100); // 표시
			else
				temp_hit = static_cast<int16_t>(temp_hit_B * (pType2->DamageMod / 100.0f));
		}
	}
	// Normal Hit.
	else
	{
		temp_hit = m_sTotalHit * m_bAttackAmount / 100; // 표시
		result   = GetHitRate(m_fTotalHitRate / pTUser->m_fTotalEvasionRate);
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

	damage = GetMagicDamage(damage, tid); // 2. Magical item damage....
	damage = GetACDamage(damage, tid);    // 3. Additional AC calculation....
										  //	damage = damage / 2;	// 성래씨 추가 요청!!!!
	damage = damage / 3;                  // 성래씨 추가 요청!!!!

	return damage;
}

int16_t CUser::GetMagicDamage(int damage, int tid)
{
	int16_t total_r = 0, temp_damage = 0;

	auto pTUser = m_pMain->GetUserPtr(tid);
	if (pTUser == nullptr || pTUser->m_bResHpType == USER_DEAD)
		return damage;

	// RIGHT HAND!!! by Yookozuna
	if (m_bMagicTypeRightHand > 4 && m_bMagicTypeRightHand < 8)
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

		case ITEM_TYPE_NONE:
			/* do nothing */
			break;

		default:
			spdlog::error(
				"User::GetMagicDamage: Unsupported right hand damage type {} [characterName={}]",
				m_bMagicTypeRightHand, m_pUserData->m_id);
			break;
	}

	if (m_bMagicTypeRightHand > 0 && m_bMagicTypeRightHand < 5)
	{
		if (total_r > 200)
			total_r = 200;

		temp_damage = m_sMagicAmountRightHand - m_sMagicAmountRightHand * total_r / 200;
		damage      = damage + temp_damage;
	}

	// Reset all temporary data.
	total_r     = 0;
	temp_damage = 0;

	// LEFT HAND!!! by Yookozuna
	if (m_bMagicTypeLeftHand > 4 && m_bMagicTypeLeftHand < 8)
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

		case ITEM_TYPE_NONE:
			/* do nothing */
			break;

		default:
			spdlog::error(
				"User::GetMagicDamage: Unsupported left hand damage type {} [characterName={}]",
				m_bMagicTypeLeftHand, m_pUserData->m_id);
			break;
	}

	if (m_bMagicTypeLeftHand > 0 && m_bMagicTypeLeftHand < 5)
	{
		if (total_r > 200)
			total_r = 200;

		temp_damage = m_sMagicAmountLeftHand - m_sMagicAmountLeftHand * total_r / 200;
		damage      = damage + temp_damage;
	}

	// Mirror Attack Check routine.
	if (pTUser->m_bMagicTypeLeftHand == ITEM_TYPE_MIRROR_DAMAGE)
	{
		temp_damage = damage * pTUser->m_sMagicAmountLeftHand / 100;
		HpChange(-temp_damage); // Reflective Hit.
	}

	return damage;
}

int16_t CUser::GetACDamage(int damage, int tid)
{
	model::Item *pLeftHand = nullptr, *pRightHand = nullptr;

	auto pTUser = m_pMain->GetUserPtr(tid);
	if (pTUser == nullptr || pTUser->m_bResHpType == USER_DEAD)
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

				default:
					spdlog::warn("User::GetACDamage: Unhandled right hand weapon group type {} "
								 "[itemId={} characterName={}]",
						pRightHand->Kind / 10, pRightHand->ID, m_pUserData->m_id);
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

				default:
					spdlog::warn("User::GetACDamage: Unhandled left hand weapon group type {} "
								 "[itemId={} characterName={}]",
						pLeftHand->Kind / 10, pLeftHand->ID, m_pUserData->m_id);
					break;
			}
		}
	}

	return damage;
}

void CUser::ExpChange(int iExp)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	if (m_pUserData->m_bLevel < 6 && iExp < 0)
		return;

	if (m_pUserData->m_bZone == ZONE_BATTLE && iExp < 0)
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
			m_pUserData->m_iExp += m_pMain->m_LevelUpTableArray[m_pUserData->m_bLevel - 1]
									   ->RequiredExp;
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

	SetByte(sendBuffer, WIZ_EXP_CHANGE, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iExp, sendIndex);
	Send(sendBuffer, sendIndex);

	if (iExp < 0)
		m_iLostExp = -iExp;
}

void CUser::LevelChange(int16_t level, bool bLevelUp)
{
	if (level < 1 || level > MAX_LEVEL)
		return;

	int sendIndex = 0;
	char sendBuffer[256] {};

	if (bLevelUp)
	{
		if ((m_pUserData->m_bPoints + m_pUserData->m_bSta + m_pUserData->m_bStr
				+ m_pUserData->m_bDex + m_pUserData->m_bIntel + m_pUserData->m_bCha)
			< (300 + 3 * (level - 1)))
			m_pUserData->m_bPoints += 3;

		if (level > 9
			&& (m_pUserData->m_bstrSkill[0] + m_pUserData->m_bstrSkill[1]
				   + m_pUserData->m_bstrSkill[2] + m_pUserData->m_bstrSkill[3]
				   + m_pUserData->m_bstrSkill[4] + m_pUserData->m_bstrSkill[5]
				   + m_pUserData->m_bstrSkill[6] + m_pUserData->m_bstrSkill[7]
				   + m_pUserData->m_bstrSkill[8])
				   < (2 * (level - 9)))
			m_pUserData->m_bstrSkill[0] += 2; // Skill Points up
	}

	m_iMaxExp = m_pMain->m_LevelUpTableArray[level - 1]->RequiredExp;

	SetSlotItemValue();
	SetUserAbility();

	m_pUserData->m_sMp = m_iMaxMp;
	HpChange(m_iMaxHp);

	Send2AI_UserUpdateInfo();

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_LEVEL_CHANGE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bPoints, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bstrSkill[0], sendIndex);
	SetDWORD(sendBuffer, m_iMaxExp, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iExp, sendIndex);
	SetShort(sendBuffer, m_iMaxHp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
	SetShort(sendBuffer, m_iMaxMp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sMp, sendIndex);
	SetShort(sendBuffer, GetMaxWeightForClient(), sendIndex);
	SetShort(sendBuffer, GetCurrentWeightForClient(), sendIndex);
	m_pMain->Send_Region(sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ);

	if (m_sPartyIndex != -1)
	{
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_LEVELCHANGE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
	}
}

void CUser::PointChange(char* pBuf)
{
	int index = 0, sendIndex = 0, value = 0;
	uint8_t type = 0x00;
	char sendBuffer[128] {};

	type  = GetByte(pBuf, index);
	value = GetShort(pBuf, index);

	if (type > 5 || abs(value) > 1)
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

		default:
			spdlog::error("User::PointChange: Unhandled stat type {} [characterName={}]", type,
				m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			return;
	}

	m_pUserData->m_bPoints -= value;

	SetByte(sendBuffer, WIZ_POINT_CHANGE, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	switch (type)
	{
		case STAT_TYPE_STR:
			++m_pUserData->m_bStr;
			SetShort(sendBuffer, m_pUserData->m_bStr, sendIndex);
			SetUserAbility();
			break;

		case STAT_TYPE_STA:
			++m_pUserData->m_bSta;
			SetShort(sendBuffer, m_pUserData->m_bSta, sendIndex);
			SetMaxHp();
			SetMaxMp();
			break;

		case STAT_TYPE_DEX:
			++m_pUserData->m_bDex;
			SetShort(sendBuffer, m_pUserData->m_bDex, sendIndex);
			SetUserAbility();
			break;

		case STAT_TYPE_INTEL:
			++m_pUserData->m_bIntel;
			SetShort(sendBuffer, m_pUserData->m_bIntel, sendIndex);
			SetMaxMp();
			break;

		case STAT_TYPE_CHA:
			++m_pUserData->m_bCha;
			SetShort(sendBuffer, m_pUserData->m_bCha, sendIndex);
			break;

		default:
			spdlog::error(
				"User::PointChange: Unhandled stat type in 2nd switch {} [characterName={}]", type,
				m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			return;
	}

	SetShort(sendBuffer, m_iMaxHp, sendIndex);
	SetShort(sendBuffer, m_iMaxMp, sendIndex);
	SetShort(sendBuffer, m_sTotalHit, sendIndex);
	SetShort(sendBuffer, GetMaxWeightForClient(), sendIndex);
	Send(sendBuffer, sendIndex);
}

// type : Received From AIServer -> 1, The Others -> 0
// attack : Direct Attack(true) or Other Case(false)
void CUser::HpChange(int amount, int type, bool attack)
{
	char sendBuffer[256] {};
	int sendIndex       = 0;

	m_pUserData->m_sHp += amount;
	if (m_pUserData->m_sHp < 0)
		m_pUserData->m_sHp = 0;
	else if (m_pUserData->m_sHp > m_iMaxHp)
		m_pUserData->m_sHp = m_iMaxHp;

	SetByte(sendBuffer, WIZ_HP_CHANGE, sendIndex);
	SetShort(sendBuffer, m_iMaxHp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
	Send(sendBuffer, sendIndex);

	if (type == 0)
	{
		sendIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));

		SetByte(sendBuffer, AG_USER_SET_HP, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetDWORD(sendBuffer, m_pUserData->m_sHp, sendIndex);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
	}

	if (m_sPartyIndex != -1)
	{
		sendIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));

		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_HPCHANGE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetShort(sendBuffer, m_iMaxHp, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
		SetShort(sendBuffer, m_iMaxMp, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sMp, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
	}

	// 직접 가격해서 죽는경우는 Dead Packet 없음
	if (m_pUserData->m_sHp == 0 && !attack)
		Dead();
}

void CUser::MSpChange(int amount)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	m_pUserData->m_sMp += amount;
	if (m_pUserData->m_sMp < 0)
		m_pUserData->m_sMp = 0;
	else if (m_pUserData->m_sMp > m_iMaxMp)
		m_pUserData->m_sMp = m_iMaxMp;

	SetByte(sendBuffer, WIZ_MSP_CHANGE, sendIndex);
	SetShort(sendBuffer, m_iMaxMp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sMp, sendIndex);
	Send(sendBuffer, sendIndex);

	if (m_sPartyIndex != -1)
	{
		sendIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));

		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_HPCHANGE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetShort(sendBuffer, m_iMaxHp, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
		SetShort(sendBuffer, m_iMaxMp, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sMp, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
	}
}

void CUser::Send2AI_UserUpdateInfo()
{
	int sendIndex = 0;
	char sendBuffer[1024] {};

	SetByte(sendBuffer, AG_USER_UPDATE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sMp, sendIndex);
	SetShort(sendBuffer, m_sTotalHit * m_bAttackAmount / 100, sendIndex); // 표시
	SetShort(sendBuffer, m_sTotalAc + m_sACAmount, sendIndex);            // 표시
	SetFloat(sendBuffer, m_fTotalHitRate, sendIndex);
	SetFloat(sendBuffer, m_fTotalEvasionRate, sendIndex);

	//
	SetShort(sendBuffer, m_sItemAc, sendIndex);
	SetByte(sendBuffer, m_bMagicTypeLeftHand, sendIndex);
	SetByte(sendBuffer, m_bMagicTypeRightHand, sendIndex);
	SetShort(sendBuffer, m_sMagicAmountLeftHand, sendIndex);
	SetShort(sendBuffer, m_sMagicAmountRightHand, sendIndex);
	//

	m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
}

void CUser::SetUserAbility()
{
	model::Coefficient* p_TableCoefficient = m_pMain->m_CoefficientTableMap.GetData(
		m_pUserData->m_sClass);
	if (p_TableCoefficient == nullptr)
		return;

	model::Item* pItem    = nullptr;
	bool bHaveBow         = false;
	double hitcoefficient = 0.0;
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
					bHaveBow       = true;
					break;

				case WEAPON_STAFF:
					hitcoefficient = p_TableCoefficient->Staff;
					break;

				default:
					break;
			}
		}
	}

	if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0 && hitcoefficient == 0.0)
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
					bHaveBow       = true;
					break;

				default:
					break;
			}
		}
	}

	int temp_str = 0, temp_dex = 0;

	temp_str     = m_pUserData->m_bStr + m_sStrAmount + m_sItemStr;
	//	if (temp_str > 255)
	//		temp_str = 255;

	temp_dex     = m_pUserData->m_bDex + m_sDexAmount + m_sItemDex;
	//	if (temp_dex > 255)
	//		temp_dex = 255;

	m_sBodyAc    = m_pUserData->m_bLevel;
	m_iMaxWeight = (m_pUserData->m_bStr + m_sItemStr) * 50;
	/*
	궁수 공격력 = 0.005 * 활 공격력 * (Dex + 40) + 직업계수 * 활 공격력 * 화살공격력 * 레벨 * Dex
	전사 공격력 = 0.005 * 무기 공격력 * (Str + 40) + 직업계수 * 활 공격력 * 화살공격력 * 레벨 * Dex
*/
	if (bHaveBow && pItem != nullptr)
	{
		m_sTotalHit = static_cast<int16_t>(
			((0.005 * pItem->Damage * (temp_dex + 40))
				+ (hitcoefficient * pItem->Damage * m_pUserData->m_bLevel * temp_dex))
			+ 3);
	}
	else
	{
		m_sTotalHit = static_cast<int16_t>(
			((0.005f * m_sItemHit * (temp_str + 40))
				+ (hitcoefficient * m_sItemHit * m_pUserData->m_bLevel * temp_str))
			+ 3);
	}

	// 토탈 AC = 테이블 코에피션트 * (레벨 + 아이템 AC + 테이블 4의 AC)
	m_sTotalAc      = static_cast<int16_t>(p_TableCoefficient->Armor * (m_sBodyAc + m_sItemAc));
	m_fTotalHitRate = static_cast<float>(
		((1.0 + p_TableCoefficient->HitRate * m_pUserData->m_bLevel * temp_dex) * m_sItemHitrate
			/ 100.0)
		* (m_bHitRateAmount / 100.0));

	m_fTotalEvasionRate = static_cast<float>(
		((1 + p_TableCoefficient->Evasionrate * m_pUserData->m_bLevel * temp_dex)
			* m_sItemEvasionrate / 100.0)
		* (m_sAvoidRateAmount / 100.0));

	SetMaxHp();
	SetMaxMp();
}

void CUser::ItemMove(char* pBuf)
{
	int index = 0, itemid = 0, srcpos = -1, destpos = -1, sendIndex = 0;
	model::Item* pTable = nullptr;
	uint8_t dir         = 0;
	char sendBuffer[128] {};

	dir     = GetByte(pBuf, index);
	itemid  = GetDWORD(pBuf, index);
	srcpos  = GetByte(pBuf, index);
	destpos = GetByte(pBuf, index);

	if (m_sExchangeUser != -1)
		goto fail_return;

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		goto fail_return;

	// if (dir == ITEM_INVEN_SLOT && ((pTable->Weight + m_iItemWeight) > m_iMaxWeight))
	//		goto fail_return;

	if (dir > 0x04 || srcpos >= SLOT_MAX + HAVE_MAX || destpos >= SLOT_MAX + HAVE_MAX)
		goto fail_return;

	if (destpos > SLOT_MAX)
	{
		if ((dir == ITEM_MOVE_INVEN_SLOT || dir == ITEM_MOVE_SLOT_SLOT))
			goto fail_return;
	}

	if (dir == ITEM_MOVE_SLOT_INVEN && srcpos > SLOT_MAX)
		goto fail_return;

	if (dir == ITEM_MOVE_INVEN_SLOT && destpos == RESERVED)
		goto fail_return;

	if (dir == ITEM_MOVE_SLOT_INVEN && srcpos == RESERVED)
		goto fail_return;

	if (dir == ITEM_MOVE_INVEN_SLOT || dir == ITEM_MOVE_SLOT_SLOT)
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
		{
			if (srcpos < 0 || srcpos >= HAVE_MAX)
				goto fail_return;

			if (destpos < 0 || destpos >= SLOT_MAX)
				goto fail_return;

			auto& srcItem = m_pUserData->m_sItemArray[SLOT_MAX + srcpos];
			auto& dstItem = m_pUserData->m_sItemArray[destpos];

			if (itemid != srcItem.nNum)
				goto fail_return;

			if (!IsValidSlotPos(pTable, destpos))
				goto fail_return;

			auto& leftHandItem  = m_pUserData->m_sItemArray[LEFTHAND];
			auto& rightHandItem = m_pUserData->m_sItemArray[RIGHTHAND];

			// 오른손전용 무기(또는 양손쓸수 있고 장착하려는 위치가 오른손) 인데 다른손에 두손쓰는 경우 체크
			if (pTable->Slot == 0x01 || (pTable->Slot == 0x00 && destpos == RIGHTHAND))
			{
				if (leftHandItem.nNum != 0)
				{
					model::Item* pTable2 = m_pMain->m_ItemTableMap.GetData(leftHandItem.nNum);
					if (pTable2 != nullptr)
					{
						if (pTable2->Slot == 0x04)
						{
							// 오른손에 넣구..
							rightHandItem.nNum       = srcItem.nNum;
							rightHandItem.sDuration  = srcItem.sDuration;
							rightHandItem.sCount     = srcItem.sCount;
							rightHandItem.nSerialNum = srcItem.nSerialNum;

							if (rightHandItem.nSerialNum == 0)
								rightHandItem.nSerialNum = m_pMain->GenerateItemSerial();

							// 왼손무기를 인벤으로 넣어준다.
							srcItem.nNum       = leftHandItem.nNum;
							srcItem.sDuration  = leftHandItem.sDuration;
							srcItem.sCount     = leftHandItem.sCount;
							srcItem.nSerialNum = leftHandItem.nSerialNum;

							if (srcItem.nSerialNum == 0)
								srcItem.nSerialNum = m_pMain->GenerateItemSerial();

							// 왼손은 비어있게 되지...
							leftHandItem.nNum       = 0;
							leftHandItem.sDuration  = 0;
							leftHandItem.sCount     = 0;
							leftHandItem.nSerialNum = 0;
						}
						else
						{
							int16_t duration   = srcItem.sDuration;
							int64_t serial     = srcItem.nSerialNum;

							// Swapping
							srcItem.nNum       = dstItem.nNum;
							srcItem.sDuration  = dstItem.sDuration;
							srcItem.sCount     = dstItem.sCount;
							srcItem.nSerialNum = dstItem.nSerialNum;

							if (srcItem.nNum != 0 && srcItem.nSerialNum == 0)
								srcItem.nSerialNum = m_pMain->GenerateItemSerial();

							dstItem.nNum       = itemid;
							dstItem.sDuration  = duration;
							dstItem.sCount     = 1;
							dstItem.nSerialNum = serial;

							if (dstItem.nSerialNum == 0)
								dstItem.nSerialNum = m_pMain->GenerateItemSerial();
						}
					}
				}
				else
				{
					int16_t duration   = srcItem.sDuration;
					int64_t serial     = srcItem.nSerialNum;

					// Swapping
					srcItem.nNum       = dstItem.nNum;
					srcItem.sDuration  = dstItem.sDuration;
					srcItem.sCount     = dstItem.sCount;
					srcItem.nSerialNum = dstItem.nSerialNum;

					if (srcItem.nNum != 0 && srcItem.nSerialNum == 0)
						srcItem.nSerialNum = m_pMain->GenerateItemSerial();

					dstItem.nNum       = itemid;
					dstItem.sDuration  = duration;
					dstItem.sCount     = 1;
					dstItem.nSerialNum = serial;

					if (dstItem.nSerialNum == 0)
						dstItem.nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			// 왼손전용 무기(또는 양손쓸수 있고 장착하려는 위치가 왼손) 인데 다른손에 두손쓰는 경우 체크
			else if (pTable->Slot == 0x02 || (pTable->Slot == 0x00 && destpos == LEFTHAND))
			{
				if (rightHandItem.nNum != 0)
				{
					model::Item* pTable2 = m_pMain->m_ItemTableMap.GetData(rightHandItem.nNum);
					if (pTable2 != nullptr)
					{
						if (pTable2->Slot == 0x03)
						{
							leftHandItem.nNum       = srcItem.nNum;
							leftHandItem.sDuration  = srcItem.sDuration;
							leftHandItem.sCount     = srcItem.sCount;
							leftHandItem.nSerialNum = srcItem.nSerialNum;

							if (leftHandItem.nSerialNum == 0)
								leftHandItem.nSerialNum = m_pMain->GenerateItemSerial();

							// 오른손무기를 인벤으로 넣어준다.
							srcItem.nNum       = rightHandItem.nNum;
							srcItem.sDuration  = rightHandItem.sDuration;
							srcItem.sCount     = rightHandItem.sCount;
							srcItem.nSerialNum = rightHandItem.nSerialNum;

							if (srcItem.nSerialNum == 0)
								srcItem.nSerialNum = m_pMain->GenerateItemSerial();

							rightHandItem.nNum       = 0;
							rightHandItem.sDuration  = 0;
							rightHandItem.sCount     = 0;
							rightHandItem.nSerialNum = 0;
						}
						else
						{
							int16_t duration   = srcItem.sDuration;
							int64_t serial     = srcItem.nSerialNum;

							// Swapping
							srcItem.nNum       = dstItem.nNum;
							srcItem.sDuration  = dstItem.sDuration;
							srcItem.sCount     = dstItem.sCount;
							srcItem.nSerialNum = dstItem.nSerialNum;

							if (srcItem.nNum != 0 && srcItem.nSerialNum == 0)
								srcItem.nSerialNum = m_pMain->GenerateItemSerial();

							dstItem.nNum       = itemid;
							dstItem.sDuration  = duration;
							dstItem.sCount     = 1;
							dstItem.nSerialNum = serial;

							if (dstItem.nSerialNum == 0)
								dstItem.nSerialNum = m_pMain->GenerateItemSerial();
						}
					}
				}
				else
				{
					int16_t duration   = srcItem.sDuration;
					int64_t serial     = srcItem.nSerialNum;

					// Swapping
					srcItem.nNum       = dstItem.nNum;
					srcItem.sDuration  = dstItem.sDuration;
					srcItem.sCount     = dstItem.sCount;
					srcItem.nSerialNum = dstItem.nSerialNum;

					if (srcItem.nNum != 0 && srcItem.nSerialNum == 0)
						srcItem.nSerialNum = m_pMain->GenerateItemSerial();

					dstItem.nNum       = itemid;
					dstItem.sDuration  = duration;
					dstItem.sCount     = 1;
					dstItem.nSerialNum = serial;

					if (dstItem.nSerialNum == 0)
						dstItem.nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			// 두손 사용하고 오른손 무기
			else if (pTable->Slot == 0x03)
			{
				if (leftHandItem.nNum != 0 && rightHandItem.nNum != 0)
					goto fail_return;

				if (leftHandItem.nNum != 0)
				{
					rightHandItem.nNum       = srcItem.nNum;
					rightHandItem.sDuration  = srcItem.sDuration;
					rightHandItem.sCount     = srcItem.sCount;
					rightHandItem.nSerialNum = srcItem.nSerialNum;

					if (rightHandItem.nSerialNum == 0)
						rightHandItem.nSerialNum = m_pMain->GenerateItemSerial();

					srcItem.nNum       = leftHandItem.nNum;
					srcItem.sDuration  = leftHandItem.sDuration;
					srcItem.sCount     = leftHandItem.sCount;
					srcItem.nSerialNum = leftHandItem.nSerialNum;

					if (srcItem.nSerialNum == 0)
						srcItem.nSerialNum = m_pMain->GenerateItemSerial();

					leftHandItem.nNum       = 0;
					leftHandItem.sDuration  = 0;
					leftHandItem.sCount     = 0;
					leftHandItem.nSerialNum = 0;
				}
				else
				{
					int16_t duration   = srcItem.sDuration;
					int64_t serial     = srcItem.nSerialNum;

					// Swapping
					srcItem.nNum       = dstItem.nNum;
					srcItem.sDuration  = dstItem.sDuration;
					srcItem.sCount     = dstItem.sCount;
					srcItem.nSerialNum = dstItem.nSerialNum;

					if (srcItem.nNum != 0 && srcItem.nSerialNum == 0)
						srcItem.nSerialNum = m_pMain->GenerateItemSerial();

					dstItem.nNum       = itemid;
					dstItem.sDuration  = duration;
					dstItem.sCount     = 1;
					dstItem.nSerialNum = serial;

					if (dstItem.nSerialNum == 0)
						dstItem.nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			// 두손 사용하고 왼손 무기
			else if (pTable->Slot == 0x04)
			{
				if (leftHandItem.nNum != 0 && rightHandItem.nNum != 0)
					goto fail_return;

				if (rightHandItem.nNum != 0)
				{
					leftHandItem.nNum       = srcItem.nNum;
					leftHandItem.sDuration  = srcItem.sDuration;
					leftHandItem.sCount     = srcItem.sCount;
					leftHandItem.nSerialNum = srcItem.nSerialNum;

					if (leftHandItem.nSerialNum == 0)
						leftHandItem.nSerialNum = m_pMain->GenerateItemSerial();

					srcItem.nNum       = rightHandItem.nNum;
					srcItem.sDuration  = rightHandItem.sDuration;
					srcItem.sCount     = rightHandItem.sCount;
					srcItem.nSerialNum = rightHandItem.nSerialNum;

					if (srcItem.nSerialNum == 0)
						srcItem.nSerialNum = m_pMain->GenerateItemSerial();

					rightHandItem.nNum       = 0;
					rightHandItem.sDuration  = 0;
					rightHandItem.sCount     = 0;
					rightHandItem.nSerialNum = 0;
				}
				else
				{
					int16_t duration   = srcItem.sDuration;
					int64_t serial     = srcItem.nSerialNum;

					// Swapping
					srcItem.nNum       = dstItem.nNum;
					srcItem.sDuration  = dstItem.sDuration;
					srcItem.sCount     = dstItem.sCount;
					srcItem.nSerialNum = dstItem.nSerialNum;

					if (srcItem.nNum != 0 && srcItem.nSerialNum == 0)
						srcItem.nSerialNum = m_pMain->GenerateItemSerial();

					dstItem.nNum       = itemid;
					dstItem.sDuration  = duration;
					dstItem.sCount     = 1;
					dstItem.nSerialNum = serial;
					if (dstItem.nSerialNum == 0)
						dstItem.nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			else
			{
				int16_t duration   = srcItem.sDuration;
				int64_t serial     = srcItem.nSerialNum;

				// Swapping
				srcItem.nNum       = dstItem.nNum;
				srcItem.sDuration  = dstItem.sDuration;
				srcItem.sCount     = dstItem.sCount;
				srcItem.nSerialNum = dstItem.nSerialNum;

				if (srcItem.nNum != 0 && srcItem.nSerialNum == 0)
					srcItem.nSerialNum = m_pMain->GenerateItemSerial();

				dstItem.nNum       = itemid;
				dstItem.sDuration  = duration;
				dstItem.sCount     = 1;
				dstItem.nSerialNum = serial;

				if (dstItem.nSerialNum == 0)
					dstItem.nSerialNum = m_pMain->GenerateItemSerial();
			}
		}
		break;

		case ITEM_MOVE_SLOT_INVEN:
		{
			if (srcpos < 0 || srcpos >= SLOT_MAX)
				goto fail_return;

			if (destpos < 0 || destpos >= HAVE_MAX)
				goto fail_return;

			auto& srcItem = m_pUserData->m_sItemArray[srcpos];
			auto& dstItem = m_pUserData->m_sItemArray[SLOT_MAX + destpos];

			if (itemid != srcItem.nNum)
				goto fail_return;

			if (dstItem.nNum != 0)
				goto fail_return;

			dstItem.nNum       = srcItem.nNum;
			dstItem.sDuration  = srcItem.sDuration;
			dstItem.sCount     = srcItem.sCount;
			dstItem.nSerialNum = srcItem.nSerialNum;

			if (dstItem.nSerialNum == 0)
				dstItem.nSerialNum = m_pMain->GenerateItemSerial();

			srcItem.nNum       = 0;
			srcItem.sDuration  = 0;
			srcItem.sCount     = 0;
			srcItem.nSerialNum = 0;
		}
		break;

		case ITEM_MOVE_INVEN_INVEN:
		{
			if (srcpos < 0 || srcpos >= HAVE_MAX)
				goto fail_return;

			if (destpos < 0 || destpos >= HAVE_MAX)
				goto fail_return;

			auto& srcItem = m_pUserData->m_sItemArray[SLOT_MAX + srcpos];
			auto& dstItem = m_pUserData->m_sItemArray[SLOT_MAX + destpos];

			if (itemid != srcItem.nNum)
				goto fail_return;

			int16_t duration     = srcItem.sDuration;
			int16_t itemcount    = srcItem.sCount;
			int64_t serial       = srcItem.nSerialNum;
			model::Item* pTable2 = nullptr;

			srcItem.nNum         = dstItem.nNum;
			srcItem.sDuration    = dstItem.sDuration;
			srcItem.sCount       = dstItem.sCount;
			srcItem.nSerialNum   = dstItem.nSerialNum;

			if (srcItem.nSerialNum == 0)
			{
				pTable2 = m_pMain->m_ItemTableMap.GetData(srcItem.nNum);
				if (pTable2 != nullptr && pTable2->Countable == 0)
					srcItem.nSerialNum = m_pMain->GenerateItemSerial();
			}

			dstItem.nNum       = itemid;
			dstItem.sDuration  = duration;
			dstItem.sCount     = itemcount;
			dstItem.nSerialNum = serial;

			if (dstItem.nSerialNum == 0)
			{
				pTable2 = m_pMain->m_ItemTableMap.GetData(dstItem.nNum);
				if (pTable2 != nullptr && pTable2->Countable == 0)
					dstItem.nSerialNum = m_pMain->GenerateItemSerial();
			}
		}
		break;

		case ITEM_MOVE_SLOT_SLOT:
		{
			if (srcpos < 0 || srcpos >= SLOT_MAX)
				goto fail_return;

			if (destpos < 0 || destpos >= SLOT_MAX)
				goto fail_return;

			auto& srcItem = m_pUserData->m_sItemArray[srcpos];
			auto& dstItem = m_pUserData->m_sItemArray[destpos];

			if (itemid != srcItem.nNum)
				goto fail_return;

			if (!IsValidSlotPos(pTable, destpos))
				goto fail_return;

			if (dstItem.nNum != 0)
			{
				// dest slot exist some item
				model::Item* pTable2 = m_pMain->m_ItemTableMap.GetData(dstItem.nNum);
				if (pTable2 != nullptr)
				{
					if (pTable2->Slot != 0x00)
						goto fail_return;

					int16_t duration   = srcItem.sDuration;
					int16_t count      = srcItem.sCount;
					int64_t serial     = srcItem.nSerialNum;

					// Swapping
					srcItem.nNum       = dstItem.nNum;
					srcItem.sDuration  = dstItem.sDuration;
					srcItem.sCount     = dstItem.sCount;
					srcItem.nSerialNum = dstItem.nSerialNum;

					if (srcItem.nSerialNum == 0)
						srcItem.nSerialNum = m_pMain->GenerateItemSerial();

					dstItem.nNum       = itemid;
					dstItem.sDuration  = duration;
					dstItem.sCount     = count;
					dstItem.nSerialNum = serial;

					if (dstItem.nSerialNum == 0)
						dstItem.nSerialNum = m_pMain->GenerateItemSerial();
				}
			}
			else
			{
				int16_t duration   = srcItem.sDuration;
				int16_t count      = srcItem.sCount;
				int64_t serial     = srcItem.nSerialNum;

				// Swapping
				srcItem.nNum       = dstItem.nNum;
				srcItem.sDuration  = dstItem.sDuration;
				srcItem.sCount     = dstItem.sCount;
				srcItem.nSerialNum = dstItem.nSerialNum;

				dstItem.nNum       = itemid;
				dstItem.sDuration  = duration;
				dstItem.sCount     = count;
				dstItem.nSerialNum = serial;

				if (dstItem.nSerialNum == 0)
					dstItem.nSerialNum = m_pMain->GenerateItemSerial();
			}
		}
		break;

		default:
			spdlog::error("User::ItemMove: Unhandled move type {} [characterName={}]", dir,
				m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			return;
	}

	// 장착이 바뀌는 경우에만 계산..
	if (dir != ITEM_MOVE_INVEN_INVEN)
	{
		SetSlotItemValue();
		SetUserAbility();
	}

	//	비러머글 사자의 힘 >.<
	SetByte(sendBuffer, WIZ_ITEM_MOVE, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetShort(sendBuffer, m_sTotalHit, sendIndex);
	SetShort(sendBuffer, m_sTotalAc, sendIndex);
	SetShort(sendBuffer, GetMaxWeightForClient(), sendIndex);
	SetShort(sendBuffer, m_iMaxHp, sendIndex);
	SetShort(sendBuffer, m_iMaxMp, sendIndex);
	SetShort(sendBuffer, m_sItemStr + m_sStrAmount, sendIndex);
	SetShort(sendBuffer, m_sItemSta + m_sStaAmount, sendIndex);
	SetShort(sendBuffer, m_sItemDex + m_sDexAmount, sendIndex);
	SetShort(sendBuffer, m_sItemIntel + m_sIntelAmount, sendIndex);
	SetShort(sendBuffer, m_sItemCham + m_sChaAmount, sendIndex);
	SetShort(sendBuffer, m_bFireR, sendIndex);
	SetShort(sendBuffer, m_bColdR, sendIndex);
	SetShort(sendBuffer, m_bLightningR, sendIndex);
	SetShort(sendBuffer, m_bMagicR, sendIndex);
	SetShort(sendBuffer, m_bDiseaseR, sendIndex);
	SetShort(sendBuffer, m_bPoisonR, sendIndex);
	Send(sendBuffer, sendIndex);
	//
	SendItemWeight();

	if (dir == ITEM_MOVE_INVEN_SLOT)
	{
		// 장착
		switch (destpos)
		{
			case HEAD:
			case BREAST:
			case SHOULDER:
			case LEFTHAND:
			case RIGHTHAND:
			case LEG:
			case GLOVE:
			case FOOT:
				UserLookChange(destpos, itemid, m_pUserData->m_sItemArray[destpos].sDuration);
				break;

			default:
				break;
		}
	}

	if (dir == ITEM_MOVE_SLOT_INVEN)
	{
		// 해제
		switch (srcpos)
		{
			case HEAD:
			case BREAST:
			case SHOULDER:
			case LEFTHAND:
			case RIGHTHAND:
			case LEG:
			case GLOVE:
			case FOOT:
				UserLookChange(srcpos, 0, 0);
				break;

			default:
				break;
		}
	}

	// AI Server에 바끤 데이타 전송....
	Send2AI_UserUpdateInfo();
	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_ITEM_MOVE, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	Send(sendBuffer, sendIndex);
}

bool CUser::IsValidSlotPos(model::Item* pTable, int destpos) const
{
	if (pTable == nullptr)
		return false;

	switch (pTable->Slot)
	{
		case 0:
			if (destpos != RIGHTHAND && destpos != LEFTHAND)
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
			if (destpos != RIGHTEAR && destpos != LEFTEAR)
				return false;
			break;

		case 11:
			if (destpos != NECK)
				return false;
			break;

		case 12:
			if (destpos != RIGHTRING && destpos != LEFTRING)
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

		default:
			break;
	}

	return true;
}

void CUser::NpcEvent(char* pBuf)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return;

	CNpc* pNpc = nullptr;
	int index = 0, sendIndex = 0, nid = 0;
	char sendBuffer[2048] {};

	nid  = GetShort(pBuf, index);

	pNpc = m_pMain->m_NpcMap.GetData(nid);
	if (pNpc == nullptr)
		return;

	switch (pNpc->m_tNpcType)
	{
		case NPC_MERCHANT:
			SetByte(sendBuffer, WIZ_TRADE_NPC, sendIndex);
			SetDWORD(sendBuffer, pNpc->m_iSellingGroup, sendIndex);
			Send(sendBuffer, sendIndex);
			break;

		case NPC_TINKER:
			SetByte(sendBuffer, WIZ_REPAIR_NPC, sendIndex);
			SetDWORD(sendBuffer, pNpc->m_iSellingGroup, sendIndex);
			Send(sendBuffer, sendIndex);
			break;

		case NPC_OFFICER:
			SetShort(sendBuffer, 0, sendIndex); // default 0 page
			m_pMain->m_KnightsManager.AllKnightsList(this, sendBuffer);
			break;

		case NPC_WAREHOUSE:
			SetByte(sendBuffer, WIZ_WAREHOUSE, sendIndex);
			SetByte(sendBuffer, WAREHOUSE_REQ, sendIndex);
			Send(sendBuffer, sendIndex);
			break;

		case NPC_RENTAL:
			SetByte(sendBuffer, WIZ_RENTAL, sendIndex);
			SetByte(sendBuffer, RENTAL_NPC, sendIndex);
			// 1 = enabled, -1 = disabled
			SetShort(sendBuffer, 1, sendIndex);
			SetDWORD(sendBuffer, pNpc->m_iSellingGroup, sendIndex);
			Send(sendBuffer, sendIndex);
			break;

#if 0 // not typically available
		case NPC_COUPON:
			if (m_pMain->m_byNationID == 1
				|| m_pMain->m_byNationID == 4)
				return;

			SetShort(sendBuffer, nid, sendIndex);
			ClientEvent(sendBuffer);
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
			SetShort(sendBuffer, nid, sendIndex);
			ClientEvent(sendBuffer);
			break;

		default:
			break;
	}
}

void CUser::ItemTrade(char* pBuf)
{
	model::Item* pTable = nullptr;
	CNpc* pNpc          = nullptr;
	int index = 0, sendIndex = 0, itemid = 0, count = 0, group = 0, npcid = 0;
	uint8_t type = 0, pos = 0, destpos = 0, result = 0;
	char sendBuffer[128] {};

	// 상거래 안되게...
	if (m_bResHpType == USER_DEAD || m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::ItemTrade: dead user cannot trade [charId={} socketId={} resHpType={} "
					  "hp={} x={} z={}]",
			m_pUserData->m_id, _socketId, m_bResHpType, m_pUserData->m_sHp,
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
	pos    = GetByte(pBuf, index);

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
		if (pos >= HAVE_MAX || destpos >= HAVE_MAX)
		{
			SetByte(sendBuffer, WIZ_ITEM_TRADE, sendIndex);
			SetByte(sendBuffer, 0x04, sendIndex);
			Send(sendBuffer, sendIndex);
			return;
		}

		if (itemid != m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum)
		{
			SetByte(sendBuffer, WIZ_ITEM_TRADE, sendIndex);
			SetByte(sendBuffer, 0x04, sendIndex);
			Send(sendBuffer, sendIndex);
			return;
		}

		int16_t duration  = m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration;
		int16_t itemcount = m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount;

		m_pUserData->m_sItemArray[SLOT_MAX + pos]
			.nNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum;
		m_pUserData->m_sItemArray[SLOT_MAX + pos]
			.sDuration = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration;
		m_pUserData->m_sItemArray[SLOT_MAX + pos]
			.sCount = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount;
		m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum      = itemid;
		m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration = duration;
		m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount    = itemcount;

		SetByte(sendBuffer, WIZ_ITEM_TRADE, sendIndex);
		SetByte(sendBuffer, 0x03, sendIndex);
		Send(sendBuffer, sendIndex);
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

	if (count <= 0 || count > MAX_ITEM_COUNT)
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
		if (pTable->Countable == 0 && count != 1)
		{
			result = 2;
			goto fail_return;
		}

		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != 0)
		{
			if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum == itemid)
			{
				if (pTable->Countable == 0 || count <= 0)
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
		if (buyPrice < 0 || buyPrice > MAX_GOLD)
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

		m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum       = itemid;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration  = pTable->Durability;

		m_pUserData->m_iGold                                -= static_cast<int>(buyPrice);

		// count 개념이 있는 아이템
		if (pTable->Countable && count > 0)
		{
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount += count;
		}
		else
		{
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount     = 1;
			m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum = m_pMain->GenerateItemSerial();
		}

		SendItemWeight();
		ItemLogToAgent(m_pUserData->m_id, pNpc->m_strName, ITEM_LOG_MERCHANT_BUY,
			m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum, itemid, count,
			pTable->Durability);
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

		if (pTable->Countable != 0 && count > 0)
		{
			int64_t salePrice = static_cast<int64_t>(pTable->BuyPrice) * count;
			if (pTable->SellPrice != SALE_TYPE_FULL)
			{
				if (m_pUserData->m_byPremiumType != 0)
					salePrice /= 6;
				else
					salePrice /= 4;
			}

			if (salePrice < 0 || salePrice > MAX_GOLD)
			{
				result = 3;
				goto fail_return;
			}

			m_pUserData->m_iGold                             += static_cast<int>(salePrice);

			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount -= count;

			if (m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount <= 0)
			{
				m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum      = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount    = 0;
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

			if (salePrice < 0 || salePrice > MAX_GOLD)
			{
				result = 3;
				goto fail_return;
			}

			m_pUserData->m_iGold                                += static_cast<int>(salePrice);

			m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum       = 0;
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration  = 0;
			m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount     = 0;
		}

		SendItemWeight();
		ItemLogToAgent(m_pUserData->m_id, "MERCHANT SELL", ITEM_LOG_MERCHANT_SELL, 0, itemid, count,
			durability);
	}

	SetByte(sendBuffer, WIZ_ITEM_TRADE, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	Send(sendBuffer, sendIndex);
	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_ITEM_TRADE, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::SendTargetHP(uint8_t echo, int tid, int damage)
{
	CNpc* pNpc = nullptr;
	std::shared_ptr<CUser> pTUser;
	int sendIndex = 0, hp = 0, maxhp = 0;
	char sendBuffer[256] {};

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

		hp    = pNpc->m_iHP;
		maxhp = pNpc->m_iMaxHP;
	}
	else
	{
		pTUser = m_pMain->GetUserPtr(tid);
		if (pTUser == nullptr || pTUser->m_bResHpType == USER_DEAD)
			return;

		hp    = pTUser->m_pUserData->m_sHp;
		maxhp = pTUser->m_iMaxHp;
	}

	SetByte(sendBuffer, WIZ_TARGET_HP, sendIndex);
	SetShort(sendBuffer, tid, sendIndex);
	SetByte(sendBuffer, echo, sendIndex);
	SetDWORD(sendBuffer, maxhp, sendIndex);
	SetDWORD(sendBuffer, hp, sendIndex);
	SetShort(sendBuffer, damage, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::BundleOpenReq(char* pBuf)
{
	int index = 0, sendIndex = 0, bundle_index = 0;
	_ZONE_ITEM* pItem = nullptr;
	C3DMap* pMap      = nullptr;
	CRegion* pRegion  = nullptr;
	char sendBuffer[256] {};

	bundle_index = GetDWORD(pBuf, index);
	if (bundle_index < 1)
		return;

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	if (m_RegionX < 0 || m_RegionZ < 0 || m_RegionX > pMap->GetXRegionMax()
		|| m_RegionZ > pMap->GetZRegionMax())
		return;

	pRegion = &pMap->m_ppRegion[m_RegionX][m_RegionZ];
	if (pRegion == nullptr)
		return;

	pItem = pRegion->m_RegionItemArray.GetData(bundle_index);
	if (pItem == nullptr)
		return;

	SetByte(sendBuffer, WIZ_BUNDLE_OPEN_REQ, sendIndex);

	for (int i = 0; i < 6; i++)
	{
		SetDWORD(sendBuffer, pItem->itemid[i], sendIndex);
		SetShort(sendBuffer, pItem->count[i], sendIndex);
	}

	Send(sendBuffer, sendIndex);
}

bool CUser::IsValidName(const char* name)
{
	// sungyong tw
	const char* szInvalids[] = {
		"~", "`", "!", "@", "#", "$", "%", "^", "&", "*",         //
		"(", ")", "-", "+", "=", "|", "\\", "<", ">", ",",        //
		".", "?", "/", "{", "[", "}", "]", "\"", "\'", " ", "　", //
		"Knight", "Noahsystem", "Wizgate", "Mgame"                //
	};

	// taiwan version
	/*const char* szInvalids[] =
	{
		"~", "`", "!", "@", "#", "$", "%", "^", "&", "*",
		"(", ")", "-", "+", "=", "|", "\\", "<", ">", ",",
		".", "?", "/", "{", "[", "}", "]", "\"", "\'", " ",	"　"
	};*/

	for (const char* invalidPartialName : szInvalids)
	{
		if (strstr(name, invalidPartialName) != nullptr)
			return false;
	}

	return true;
}

void CUser::ItemGet(char* pBuf)
{
	int index = 0, sendIndex = 0, bundle_index = 0, itemid = 0, count = 0, i = 0;
	uint8_t pos          = 0xff;
	model::Item* pTable  = nullptr;
	_ZONE_ITEM* pItem    = nullptr;
	C3DMap* pMap         = nullptr;
	CRegion* pRegion     = nullptr;
	_PARTY_GROUP* pParty = nullptr;
	std::shared_ptr<CUser> pUser, pGetUser;
	char sendBuffer[256] {};

	bundle_index = GetDWORD(pBuf, index);
	if (bundle_index < 1)
		goto fail_return;

	if (m_sExchangeUser != -1)
		goto fail_return;

	pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		goto fail_return;

	if (m_RegionX < 0 || m_RegionZ < 0 || m_RegionX > pMap->GetXRegionMax()
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

	if (!pMap->RegionItemRemove(
			m_RegionX, m_RegionZ, bundle_index, pItem->itemid[i], pItem->count[i]))
		goto fail_return;

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		goto fail_return;

	if (m_sPartyIndex != -1 && itemid != ITEM_GOLD)
		pGetUser = GetItemRoutingUser(itemid, count);
	else
		pGetUser = std::static_pointer_cast<CUser>(shared_from_this());

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
				sendIndex = 0;
				memset(sendBuffer, 0, sizeof(sendBuffer));
				SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
				SetByte(sendBuffer, 0x06, sendIndex);
				pGetUser->Send(sendBuffer, sendIndex);
				return;
			}
		}
		// Check weight of non-countable item.
		else
		{
			if ((pTable->Weight + pGetUser->m_iItemWeight) > pGetUser->m_iMaxWeight)
			{
				sendIndex = 0;
				memset(sendBuffer, 0, sizeof(sendBuffer));
				SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
				SetByte(sendBuffer, 0x06, sendIndex);
				pGetUser->Send(sendBuffer, sendIndex);
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
			pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos]
				.nSerialNum = m_pMain->GenerateItemSerial();
		}

		pGetUser->SendItemWeight();
		pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration = pTable->Durability;
		pGetUser->ItemLogToAgent(pGetUser->m_pUserData->m_id, "MONSTER", ITEM_LOG_MONSTER_GET,
			pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum, itemid, count,
			pTable->Durability);
	}
	// Gold
	else
	{
		if (itemid != ITEM_GOLD)
			goto fail_return;

		if (count > 0 && count < 32767)
		{
			if (m_sPartyIndex == -1)
			{
				m_pUserData->m_iGold += count;

				SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
				SetByte(sendBuffer, 0x01, sendIndex);
				SetByte(sendBuffer, pos, sendIndex);
				SetDWORD(sendBuffer, itemid, sendIndex);
				SetShort(sendBuffer, count, sendIndex);
				SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
				Send(sendBuffer, sendIndex);
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
					pUser = m_pMain->GetUserPtr(pParty->uid[i]);
					if (pUser == nullptr)
						continue;

					money = static_cast<int>(
						count * (float) (pUser->m_pUserData->m_bLevel / (float) levelsum));
					pUser->m_pUserData->m_iGold += money;

					sendIndex                    = 0;
					memset(sendBuffer, 0, sizeof(sendBuffer));
					SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
					SetByte(sendBuffer, 0x02, sendIndex);
					SetByte(sendBuffer, 0xff, sendIndex); // gold -> pos : 0xff
					SetDWORD(sendBuffer, itemid, sendIndex);
					SetDWORD(sendBuffer, pUser->m_pUserData->m_iGold, sendIndex);
					pUser->Send(sendBuffer, sendIndex);
				}
			}
		}
		return;
	}

	SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
	if (pGetUser.get() == this)
		SetByte(sendBuffer, 0x01, sendIndex);
	else
		SetByte(sendBuffer, 0x05, sendIndex);
	SetByte(sendBuffer, pos, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex);
	SetShort(sendBuffer, pGetUser->m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount, sendIndex);
	SetDWORD(sendBuffer, pGetUser->m_pUserData->m_iGold, sendIndex);
	pGetUser->Send(sendBuffer, sendIndex);

	if (m_sPartyIndex != -1)
	{
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
		SetByte(sendBuffer, 0x03, sendIndex);
		SetDWORD(sendBuffer, itemid, sendIndex);
		SetString2(sendBuffer, pGetUser->m_pUserData->m_id, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);

		if (pGetUser.get() != this)
		{
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
			SetByte(sendBuffer, 0x04, sendIndex);
			Send(sendBuffer, sendIndex);
		}
	}
	return;

fail_return:
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_ITEM_GET, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::StateChange(char* pBuf)
{
	int index = 0, sendIndex = 0;
	uint8_t type = 0, buff = 0;
	char sendBuffer[128] {};

	type = GetByte(pBuf, index);
	buff = GetByte(pBuf, index);

	if (type > 5)
		return;

	// Operators only!!!
	if (type == 5 && m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	if (type == 1)
		m_bResHpType = buff;
	else if (type == 2)
		m_bNeedParty = buff;
	else if (type == 3)
		m_bAbnormalType = buff;

	SetByte(sendBuffer, WIZ_STATE_CHANGE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetByte(sendBuffer, type, sendIndex);

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

	SetDWORD(sendBuffer, nResult, sendIndex);

	m_pMain->Send_Region(sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ);
}

void CUser::LoyaltyChange(int tid)
{
	int16_t level_difference = 0, loyalty_source = 0, loyalty_target = 0;
	int sendIndex = 0;
	char sendBuffer[256] {};

	auto pTUser = m_pMain->GetUserPtr(tid);

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

	if (m_pUserData->m_bZone != m_pUserData->m_bNation && m_pUserData->m_bZone < 3)
		loyalty_source = 2 * loyalty_source;

	//TRACE(_T("LoyaltyChange 222 - user1=%hs, %d,, user2=%hs, %d\n"), m_pUserData->m_id,  m_pUserData->m_iLoyalty, pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_iLoyalty);

	m_pUserData->m_iLoyalty         += loyalty_source; // Recalculations of Loyalty...
	pTUser->m_pUserData->m_iLoyalty += loyalty_target;

	// Cannot be less than zero.
	if (m_pUserData->m_iLoyalty < 0)
		m_pUserData->m_iLoyalty = 0;

	if (pTUser->m_pUserData->m_iLoyalty < 0)
		pTUser->m_pUserData->m_iLoyalty = 0;

	//TRACE(_T("LoyaltyChange 222 - user1=%hs, %d,, user2=%hs, %d\n"), m_pUserData->m_id,  m_pUserData->m_iLoyalty, pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_iLoyalty);

	SetByte(sendBuffer, WIZ_LOYALTY_CHANGE, sendIndex); // Send result to source.
	SetDWORD(sendBuffer, m_pUserData->m_iLoyalty, sendIndex);
	Send(sendBuffer, sendIndex);

	// Send result to target.
	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_LOYALTY_CHANGE, sendIndex);
	SetDWORD(sendBuffer, pTUser->m_pUserData->m_iLoyalty, sendIndex);
	pTUser->Send(sendBuffer, sendIndex);

	// This is for the Event Battle on Wednesday :(
	if (m_pMain->m_byBattleOpen != 0)
	{
		if (m_pUserData->m_bZone == ZONE_BATTLE
			// || m_pUserData->m_bZone == ZONE_SNOW_BATTLE
		)
		{
			if (pTUser->m_pUserData->m_bNation == NATION_KARUS)
			{
				++m_pMain->m_sKarusDead;
				//TRACE(_T("++ LoyaltyChange - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
			}
			else if (pTUser->m_pUserData->m_bNation == NATION_ELMORAD)
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
		spdlog::warn(
			"User::SpeedHackUser: speed hack detected, banning charId={}", m_pUserData->m_id);
		m_pUserData->m_bAuthority = AUTHORITY_BLOCK_USER;
	}

	Close();
}

void CUser::UserLookChange(int pos, int itemid, int durability)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	if (pos >= SLOT_MAX)
		return;

	SetByte(sendBuffer, WIZ_USERLOOK_CHANGE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetByte(sendBuffer, (uint8_t) pos, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex);
	SetShort(sendBuffer, durability, sendIndex);
	m_pMain->Send_Region(
		sendBuffer, sendIndex, (int) m_pUserData->m_bZone, m_RegionX, m_RegionZ, this);
}

void CUser::SendNotice()
{
	int sendIndex = 0, temp_index = 0, count = 0;
	char sendBuffer[2048] {};

	sendIndex = 2;
	for (int i = 0; i < 20; i++)
	{
		if (strlen(m_pMain->m_ppNotice[i]) == 0)
			continue;

		SetString1(sendBuffer, m_pMain->m_ppNotice[i], sendIndex);
		count++;
	}

	temp_index = 0;
	SetByte(sendBuffer, WIZ_NOTICE, temp_index);
	SetByte(sendBuffer, count, temp_index);
	Send(sendBuffer, sendIndex);
}

void CUser::PartyProcess(char* pBuf)
{
	int index = 0, idlength = 0, memberid = -1;
	char strid[MAX_ID_SIZE + 1] {};
	std::shared_ptr<CUser> pUser;
	uint8_t subcommand = 0, result = 0;

	subcommand = GetByte(pBuf, index);
	switch (subcommand)
	{
		case PARTY_CREATE:
			idlength = GetShort(pBuf, index);
			if (idlength <= 0 || idlength > MAX_ID_SIZE)
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
			if (idlength <= 0 || idlength > MAX_ID_SIZE)
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

		default:
			spdlog::error(
				"User::PartyProcess: Unhandled opcode {:02X} [accountName={} characterName={}]",
				subcommand, m_strAccountID, m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			break;
	}
}

// 거절한 사람한테 온다... 리더를 찾아서 알려주는 함수
void CUser::PartyCancel()
{
	_PARTY_GROUP* pParty = nullptr;
	int sendIndex = 0, leader_id = -1, count = 0;
	char sendBuffer[256] {};

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

	leader_id     = pParty->uid[0];

	auto pUser    = m_pMain->GetUserPtr(leader_id);
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

	SetByte(sendBuffer, WIZ_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_INSERT, sendIndex);
	SetShort(sendBuffer, -1, sendIndex);
	pUser->Send(sendBuffer, sendIndex);
}

//리더에게 패킷이 온거다..
void CUser::PartyRequest(int memberid, bool bCreate)
{
	int sendIndex = 0, result = -1, i = 0;
	_PARTY_GROUP* pParty = nullptr;
	bool inserted        = false;
	char sendBuffer[256] {};

	auto pUser = m_pMain->GetUserPtr(memberid);
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

	if (!((pUser->m_pUserData->m_bLevel <= (int) (m_pUserData->m_bLevel * 1.5)
			  && pUser->m_pUserData->m_bLevel >= (int) (m_pUserData->m_bLevel * 1.5))
			|| (pUser->m_pUserData->m_bLevel <= (m_pUserData->m_bLevel + 8)
				&& pUser->m_pUserData->m_bLevel >= ((int) (m_pUserData->m_bLevel) - 8))))
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

		pParty            = new _PARTY_GROUP;
		pParty->uid[0]    = _socketId;
		pParty->sMaxHp[0] = m_iMaxHp;
		pParty->sHp[0]    = m_pUserData->m_sHp;
		pParty->bLevel[0] = m_pUserData->m_bLevel;
		pParty->sClass[0] = m_pUserData->m_sClass;

		{
			std::lock_guard<std::recursive_mutex> lock(g_region_mutex);

			m_sPartyIndex = m_pMain->m_sPartyIndex++;
			if (m_pMain->m_sPartyIndex == 32767)
				m_pMain->m_sPartyIndex = 0;

			pParty->wIndex = m_sPartyIndex;
			inserted       = m_pMain->m_PartyMap.PutData(pParty->wIndex, pParty);
		}

		if (!inserted)
		{
			delete pParty;
			m_sPartyIndex = -1;
			goto fail_return;
		}

		// AI Server
		sendIndex = 0;
		memset(sendBuffer, 0, sizeof(sendBuffer));
		SetByte(sendBuffer, AG_USER_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_CREATE, sendIndex);
		SetShort(sendBuffer, pParty->wIndex, sendIndex);
		SetShort(sendBuffer, pParty->uid[0], sendIndex);
		//SetShort( sendBuffer, pParty->sHp[0], sendIndex );
		//SetByte( sendBuffer, pParty->bLevel[0], sendIndex );
		//SetShort( sendBuffer, pParty->sClass[0], sendIndex );
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
	}

	pUser->m_sPartyIndex = m_sPartyIndex;

	/*	파티 BBS를 위해 추가...
	if (pUser->m_bNeedParty == 2 && pUser->m_sPartyIndex != -1) {
		pUser->m_bNeedParty = 1;	// 난 더 이상 파티가 필요하지 않아 ^^;
		memset( sendBuffer, 0x00, 256 ); sendIndex = 0;
		SetByte(sendBuffer, 2, sendIndex);
		SetByte(sendBuffer, pUser->m_bNeedParty, sendIndex);
		pUser->StateChange(sendBuffer);
	}

	if (m_bNeedParty == 2 && m_sPartyIndex != -1) {
		m_bNeedParty = 1;	// 난 더 이상 파티가 필요하지 않아 ^^;
		memset( sendBuffer, 0x00, 256 ); sendIndex = 0;
		SetByte(sendBuffer, 2, sendIndex);
		SetByte(sendBuffer, m_bNeedParty, sendIndex);
		StateChange(sendBuffer);
	}
*/
	sendIndex            = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, WIZ_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_PERMIT, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	// 원거리가 않된데자나 씨~
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
	//
	pUser->Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_INSERT, sendIndex);
	SetShort(sendBuffer, result, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::PartyInsert() // 본인이 추가 된다.  리더에게 패킷이 가는것이 아님
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	if (m_sPartyIndex == -1)
		return;

	_PARTY_GROUP* pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);

	// 이상한 경우
	if (pParty == nullptr)
	{
		m_sPartyIndex = -1;
		return;
	}

	// Send your info to the rest of the party members.
	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] == _socketId)
			continue;

		auto pUser = m_pMain->GetUserPtr(pParty->uid[i]);
		if (pUser == nullptr)
			continue;

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_INSERT, sendIndex);
		SetShort(sendBuffer, pParty->uid[i], sendIndex);
		SetString2(sendBuffer, pUser->m_pUserData->m_id, sendIndex);
		SetShort(sendBuffer, pParty->sMaxHp[i], sendIndex);
		SetShort(sendBuffer, pParty->sHp[i], sendIndex);
		SetByte(sendBuffer, pParty->bLevel[i], sendIndex);
		SetShort(sendBuffer, pParty->sClass[i], sendIndex);
		SetShort(sendBuffer, pUser->m_iMaxMp, sendIndex);
		SetShort(sendBuffer, pUser->m_pUserData->m_sMp, sendIndex);
		Send(sendBuffer, sendIndex); // 추가된 사람에게 기존 인원 다 받게함..
	}

	int i = 0;
	for (; i < 8; i++)
	{
		// Party Structure 에 추가
		if (pParty->uid[i] != -1)
			continue;

		pParty->uid[i]    = _socketId;
		pParty->sMaxHp[i] = m_iMaxHp;
		pParty->sHp[i]    = m_pUserData->m_sHp;
		pParty->bLevel[i] = m_pUserData->m_bLevel;
		pParty->sClass[i] = m_pUserData->m_sClass;
		break;
	}

	// 파티 BBS를 위해 추가...	대장판!!!
	auto pUser = m_pMain->GetUserPtr(pParty->uid[0]);
	if (pUser == nullptr)
		return;

	// 이건 파티장 것...
	if (pUser->m_bNeedParty == 2 && pUser->m_sPartyIndex != -1)
	{
		// 난 더 이상 파티가 필요하지 않아 ^^;
		pUser->m_bNeedParty = 1;

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, 2, sendIndex);
		SetByte(sendBuffer, pUser->m_bNeedParty, sendIndex);
		pUser->StateChange(sendBuffer);
	}
	// 추가 끝................

	// 파티 BBS를 위해 추가...	쫄따구판!!!
	// 이건 실제로 추가된 사람 것...
	if (m_bNeedParty == 2 && m_sPartyIndex != -1)
	{
		// 난 더 이상 파티가 필요하지 않아 ^^;
		m_bNeedParty = 1;

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, 2, sendIndex);
		SetByte(sendBuffer, m_bNeedParty, sendIndex);
		StateChange(sendBuffer);
	}
	// 추가 끝................

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_INSERT, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
	SetShort(sendBuffer, m_iMaxHp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bLevel, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sClass, sendIndex);
	SetShort(sendBuffer, m_iMaxMp, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_sMp, sendIndex);
	m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex); // 추가된 인원을 브로드캐스팅..

	// AI Server
	uint8_t byIndex = i;
	sendIndex       = 0;
	memset(sendBuffer, 0x00, 256);
	SetByte(sendBuffer, AG_USER_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_INSERT, sendIndex);
	SetShort(sendBuffer, pParty->wIndex, sendIndex);
	SetByte(sendBuffer, byIndex, sendIndex);
	SetShort(sendBuffer, pParty->uid[byIndex], sendIndex);
	//SetShort( sendBuffer, pParty->sHp[byIndex], sendIndex );
	//SetByte( sendBuffer, pParty->bLevel[byIndex], sendIndex );
	//SetShort( sendBuffer, pParty->sClass[byIndex], sendIndex );
	m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
}

void CUser::PartyRemove(int memberid)
{
	if (m_sPartyIndex == -1)
		return;

	auto pUser = m_pMain->GetUserPtr(memberid); // 제거될 사람...
	if (pUser == nullptr)
		return;

	_PARTY_GROUP* pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);

	// 이상한 경우
	if (pParty == nullptr)
	{
		pUser->m_sPartyIndex = -1;
		m_sPartyIndex        = -1;
		return;
	}

	// 자기자신 탈퇴가 아닌경우
	if (memberid != _socketId)
	{
		// 리더만 멤버 삭제 할수 있음..
		if (pParty->uid[0] != _socketId)
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

	int count = 0;
	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] != -1 && pParty->uid[i] != memberid)
			++count;
	}

	// 리더 혼자 남은 경우 파티 깨짐
	if (count == 1)
	{
		PartyDelete();
		return;
	}

	// 삭제된 인원을 브로드캐스팅..제거될 사람한테두 패킷이 간다.
	int sendIndex = 0;
	char sendBuffer[256] {};
	SetByte(sendBuffer, WIZ_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_REMOVE, sendIndex);
	SetShort(sendBuffer, memberid, sendIndex);
	m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);

	// 파티가 유효한 경우 에는 파티 리스트에서 뺀다.
	for (int i = 0; i < 8; i++)
	{
		if (pParty->uid[i] != -1 && pParty->uid[i] == memberid)
		{
			pParty->uid[i]       = -1;
			pParty->sHp[i]       = 0;
			pParty->bLevel[i]    = 0;
			pParty->sClass[i]    = 0;
			pUser->m_sPartyIndex = -1;
		}
	}

	// AI Server
	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, AG_USER_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_REMOVE, sendIndex);
	SetShort(sendBuffer, pParty->wIndex, sendIndex);
	SetShort(sendBuffer, memberid, sendIndex);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
}

void CUser::PartyDelete()
{
	if (m_sPartyIndex == -1)
		return;

	_PARTY_GROUP* pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
	if (pParty == nullptr)
	{
		m_sPartyIndex = -1;
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		auto pUser = m_pMain->GetUserPtr(pParty->uid[i]);
		if (pUser != nullptr)
			pUser->m_sPartyIndex = -1;
	}

	// 삭제된 인원을 브로드캐스팅..
	int sendIndex = 0;
	char sendBuffer[256] {};
	SetByte(sendBuffer, WIZ_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_DELETE, sendIndex);
	m_pMain->Send_PartyMember(pParty->wIndex, sendBuffer, sendIndex);

	// AI Server
	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, AG_USER_PARTY, sendIndex);
	SetByte(sendBuffer, PARTY_DELETE, sendIndex);
	SetShort(sendBuffer, pParty->wIndex, sendIndex);
	m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);

	std::lock_guard<std::recursive_mutex> lock(g_region_mutex);
	m_pMain->m_PartyMap.DeleteData(pParty->wIndex);
}

void CUser::ExchangeProcess(char* pBuf)
{
	int index = 0, subcommand = 0;
	subcommand = GetByte(pBuf, index);
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

		default:
			spdlog::error(
				"User::ExchangeProcess: Unhandled opcode {:02X} [accountName={} characterName={}]",
				subcommand, m_strAccountID, m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			break;
	}
}

void CUser::ExchangeReq(char* pBuf)
{
	int index = 0, destid = -1, sendIndex = 0;
	std::shared_ptr<CUser> pUser;
	char sendBuffer[256] {};

	destid = GetShort(pBuf, index);

	// 교환 안되게.....
	if (m_bResHpType == USER_DEAD || m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::ExchangeReq: dead user cannot exchange [charId={} socketId={} "
					  "resHpType={} hp={} x={} z={}]",
			m_pUserData->m_id, _socketId, m_bResHpType, m_pUserData->m_sHp,
			static_cast<int32_t>(m_pUserData->m_curx), static_cast<int32_t>(m_pUserData->m_curz));

		goto fail_return;
	}

	pUser = m_pMain->GetUserPtr(destid);
	if (pUser == nullptr)
		goto fail_return;

	if (pUser->m_sExchangeUser != -1)
		goto fail_return;

	if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
		goto fail_return;

	m_sExchangeUser        = destid;
	pUser->m_sExchangeUser = _socketId;

	SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
	SetByte(sendBuffer, EXCHANGE_REQ, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	pUser->Send(sendBuffer, sendIndex);

	return;

fail_return:
	SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
	SetByte(sendBuffer, EXCHANGE_CANCEL, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::ExchangeAgree(char* pBuf)
{
	int index = 0, sendIndex = 0, result = 0;
	std::shared_ptr<CUser> pUser;
	char sendBuffer[256] {};

	result = GetByte(pBuf, index);

	pUser  = m_pMain->GetUserPtr(m_sExchangeUser);
	if (pUser == nullptr)
	{
		m_sExchangeUser = -1;
		return;
	}

	// 거절
	if (result == 0)
	{
		m_sExchangeUser        = -1;
		pUser->m_sExchangeUser = -1;
	}
	else
	{
		InitExchange(true);
		pUser->InitExchange(true);
	}

	SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
	SetByte(sendBuffer, EXCHANGE_AGREE, sendIndex);
	SetShort(sendBuffer, static_cast<int16_t>(result), sendIndex);
	pUser->Send(sendBuffer, sendIndex);
}

void CUser::ExchangeAdd(char* pBuf)
{
	int index = 0, sendIndex = 0, count = 0, itemid = 0, duration = 0;
	_EXCHANGE_ITEM* pItem = nullptr;
	model::Item* pTable   = nullptr;
	uint8_t pos           = 0xff;
	bool bAdd = true, bGold = false;
	char sendBuffer[256] {};

	auto pUser = m_pMain->GetUserPtr(m_sExchangeUser);
	if (pUser == nullptr)
	{
		ExchangeCancel();
		return;
	}

	pos    = GetByte(pBuf, index);
	itemid = GetDWORD(pBuf, index);
	count  = GetDWORD(pBuf, index);

	pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		goto add_fail;

	if (itemid != ITEM_GOLD && pos >= HAVE_MAX)
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
				bAdd                  = false;
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
					pExchangeItem->count     += count;
					m_MirrorItem[pos].sCount -= count;
					bAdd                      = false;
					break;
				}
			}
		}

		if (bAdd)
			m_MirrorItem[pos].sCount -= count;

		duration = m_MirrorItem[pos].sDuration;

		if (m_MirrorItem[pos].sCount <= 0 || pTable->Countable == 0)
		{
			m_MirrorItem[pos].nNum       = 0;
			m_MirrorItem[pos].sDuration  = 0;
			m_MirrorItem[pos].sCount     = 0;
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
		pItem             = new _EXCHANGE_ITEM;
		pItem->itemid     = itemid;
		pItem->duration   = duration;
		pItem->count      = count;
		pItem->nSerialNum = m_MirrorItem[pos].nSerialNum;
		m_ExchangeItemList.push_back(pItem);
	}

	SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
	SetByte(sendBuffer, EXCHANGE_ADD, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	Send(sendBuffer, sendIndex);

	sendIndex = 0;
	SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
	SetByte(sendBuffer, EXCHANGE_OTHERADD, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex);
	SetDWORD(sendBuffer, count, sendIndex);
	SetShort(sendBuffer, duration, sendIndex);
	pUser->Send(sendBuffer, sendIndex);

	return;

add_fail:
	SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
	SetByte(sendBuffer, EXCHANGE_ADD, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::ExchangeDecide()
{
	int sendIndex = 0, getmoney = 0, putmoney = 0;
	bool bSuccess = true;
	char sendBuffer[256] {};

	auto pUser = m_pMain->GetUserPtr(m_sExchangeUser);
	if (pUser == nullptr)
	{
		ExchangeCancel();
		return;
	}

	if (!pUser->m_bExchangeOK)
	{
		m_bExchangeOK = 0x01;
		SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
		SetByte(sendBuffer, EXCHANGE_OTHERDECIDE, sendIndex);
		pUser->Send(sendBuffer, sendIndex);
	}
	// 둘다 승인한 경우
	else
	{
		// 교환 실패
		if (!ExecuteExchange() || !pUser->ExecuteExchange())
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
				ItemLogToAgent(m_pUserData->m_id, pUser->m_pUserData->m_id, ITEM_LOG_EXCHANGE_GET,
					0, ITEM_GOLD, getmoney, 0);
			}

			if (putmoney > 0)
			{
				ItemLogToAgent(m_pUserData->m_id, pUser->m_pUserData->m_id, ITEM_LOG_EXCHANGE_PUT,
					0, ITEM_GOLD, putmoney, 0);
			}

			SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
			SetByte(sendBuffer, EXCHANGE_DONE, sendIndex);
			SetByte(sendBuffer, 0x01, sendIndex);
			SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
			SetShort(sendBuffer, static_cast<int16_t>(pUser->m_ExchangeItemList.size()), sendIndex);

			for (_EXCHANGE_ITEM* pExchangeItem : pUser->m_ExchangeItemList)
			{
				// 새로 들어갈 인벤토리 위치
				SetByte(sendBuffer, pExchangeItem->pos, sendIndex);
				SetDWORD(sendBuffer, pExchangeItem->itemid, sendIndex);
				SetShort(sendBuffer, pExchangeItem->count, sendIndex);
				SetShort(sendBuffer, pExchangeItem->duration, sendIndex);

				ItemLogToAgent(m_pUserData->m_id, pUser->m_pUserData->m_id, ITEM_LOG_EXCHANGE_GET,
					pExchangeItem->nSerialNum, pExchangeItem->itemid, pExchangeItem->count,
					pExchangeItem->duration);
			}
			Send(sendBuffer, sendIndex); // 나한테 보내고...

			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
			SetByte(sendBuffer, EXCHANGE_DONE, sendIndex);
			SetByte(sendBuffer, 1, sendIndex);
			SetDWORD(sendBuffer, pUser->m_pUserData->m_iGold, sendIndex);
			SetShort(sendBuffer, static_cast<int16_t>(m_ExchangeItemList.size()), sendIndex);

			for (_EXCHANGE_ITEM* pExchangeItem : m_ExchangeItemList)
			{
				// 새로 들어갈 인벤토리 위치
				SetByte(sendBuffer, pExchangeItem->pos, sendIndex);
				SetDWORD(sendBuffer, pExchangeItem->itemid, sendIndex);
				SetShort(sendBuffer, pExchangeItem->count, sendIndex);
				SetShort(sendBuffer, pExchangeItem->duration, sendIndex);

				ItemLogToAgent(m_pUserData->m_id, pUser->m_pUserData->m_id, ITEM_LOG_EXCHANGE_PUT,
					pExchangeItem->nSerialNum, pExchangeItem->itemid, pExchangeItem->count,
					pExchangeItem->duration);
			}

			pUser->Send(sendBuffer, sendIndex); // 상대방도 보내준다.

			SendItemWeight();
			pUser->SendItemWeight();
		}
		else
		{
			SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
			SetByte(sendBuffer, EXCHANGE_DONE, sendIndex);
			SetByte(sendBuffer, 0, sendIndex);
			Send(sendBuffer, sendIndex);
			pUser->Send(sendBuffer, sendIndex);
		}

		InitExchange(false);
		pUser->InitExchange(false);
	}
}

void CUser::ExchangeCancel()
{
	bool bFind    = true;
	int sendIndex = 0;
	char sendBuffer[256] {};

	auto pUser = m_pMain->GetUserPtr(m_sExchangeUser);
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

		SetByte(sendBuffer, WIZ_EXCHANGE, sendIndex);
		SetByte(sendBuffer, EXCHANGE_CANCEL, sendIndex);
		pUser->Send(sendBuffer, sendIndex);
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
			m_MirrorItem[i].nNum       = m_pUserData->m_sItemArray[SLOT_MAX + i].nNum;
			m_MirrorItem[i].sDuration  = m_pUserData->m_sItemArray[SLOT_MAX + i].sDuration;
			m_MirrorItem[i].sCount     = m_pUserData->m_sItemArray[SLOT_MAX + i].sCount;
			m_MirrorItem[i].nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum;
		}
	}
	// 교환 종료시 클리어
	else
	{
		m_sExchangeUser = -1;
		m_bExchangeOK   = 0;

		for (int i = 0; i < HAVE_MAX; i++)
		{
			m_MirrorItem[i].nNum       = 0;
			m_MirrorItem[i].sDuration  = 0;
			m_MirrorItem[i].sCount     = 0;
			m_MirrorItem[i].nSerialNum = 0;
		}
	}
}

bool CUser::ExecuteExchange()
{
	model::Item* pTable = nullptr;
	int16_t weight      = 0;
	uint8_t i           = 0;

	auto pUser          = m_pMain->GetUserPtr(m_sExchangeUser);
	if (pUser == nullptr)
		return false;

	auto Iter = pUser->m_ExchangeItemList.begin();
	for (; Iter != pUser->m_ExchangeItemList.end(); ++Iter)
	{
		// 비러머글 크리스마스 이밴트 >.<
		if ((*Iter)->itemid >= ITEM_NO_TRADE)
			return false;

		if ((*Iter)->itemid == ITEM_GOLD)
		{
			// money = (*Iter)->count;
		}
		else
		{
			pTable = m_pMain->m_ItemTableMap.GetData((*Iter)->itemid);
			if (pTable == nullptr)
				continue;

			for (i = 0; i < HAVE_MAX; i++)
			{
				// 증복허용 않되는 아이템!!!
				if (m_MirrorItem[i].nNum == 0 && pTable->Countable == 0)
				{
					m_MirrorItem[i].nNum        = (*Iter)->itemid;
					m_MirrorItem[i].sDuration   = (*Iter)->duration;
					m_MirrorItem[i].sCount      = (*Iter)->count;
					m_MirrorItem[i].nSerialNum  = (*Iter)->nSerialNum;

					// 패킷용 데이터...
					(*Iter)->pos                = i;
					weight                     += pTable->Weight;
					break;
				}
				// 증복허용 아이템!!!
				else if (m_MirrorItem[i].nNum == (*Iter)->itemid && pTable->Countable == 1)
				{
					m_MirrorItem[i].sCount += (*Iter)->count;

					if (m_MirrorItem[i].sCount > MAX_ITEM_COUNT)
						m_MirrorItem[i].sCount = MAX_ITEM_COUNT;

					// 패킷용 데이터...
					(*Iter)->pos  = i;
					weight       += (pTable->Weight * (*Iter)->count);
					break;
				}
			}

			// 중복 허용 아이템인데 기존에 가지고 있지 않은 경우 빈곳에 추가
			if (i == HAVE_MAX && pTable->Countable == 1)
			{
				for (i = 0; i < HAVE_MAX; i++)
				{
					if (m_MirrorItem[i].nNum != 0)
						continue;

					m_MirrorItem[i].nNum       = (*Iter)->itemid;
					m_MirrorItem[i].sDuration  = (*Iter)->duration;
					m_MirrorItem[i].sCount     = (*Iter)->count;

					// 패킷용 데이터...
					(*Iter)->pos               = i;
					weight                    += (pTable->Weight * (*Iter)->count);
					break;
				}
			}

			// 인벤토리 공간 부족...
			if (Iter != pUser->m_ExchangeItemList.end() && i == HAVE_MAX)
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
	int money           = 0;
	model::Item* pTable = nullptr;

	auto pUser          = m_pMain->GetUserPtr(m_sExchangeUser);
	if (pUser == nullptr)
		return 0;

	for (auto Iter = pUser->m_ExchangeItemList.begin(); Iter != pUser->m_ExchangeItemList.end();)
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
		m_pUserData->m_sItemArray[SLOT_MAX + i].nNum       = m_MirrorItem[i].nNum;
		m_pUserData->m_sItemArray[SLOT_MAX + i].sDuration  = m_MirrorItem[i].sDuration;
		m_pUserData->m_sItemArray[SLOT_MAX + i].sCount     = m_MirrorItem[i].sCount;
		m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum = m_MirrorItem[i].nSerialNum;

		pTable = m_pMain->m_ItemTableMap.GetData(m_pUserData->m_sItemArray[SLOT_MAX + i].nNum);
		if (pTable == nullptr)
			continue;

		if (pTable->Countable == 0 && m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum == 0)
			m_pUserData->m_sItemArray[SLOT_MAX + i].nSerialNum = m_pMain->GenerateItemSerial();
	}

	return money;
}

void CUser::SkillPointChange(char* pBuf)
{
	int index = 0, sendIndex = 0;
	uint8_t type = 0;
	char sendBuffer[128] {};

	type = GetByte(pBuf, index);
	if (type > 8)
		return; // goto fail_return;

	if (m_pUserData->m_bstrSkill[0] < 1)
		goto fail_return;

	if ((m_pUserData->m_bstrSkill[type] + 1) > m_pUserData->m_bLevel)
		goto fail_return;

	m_pUserData->m_bstrSkill[0]    -= 1;
	m_pUserData->m_bstrSkill[type] += 1;

	return;

fail_return:
	SetByte(sendBuffer, WIZ_SKILLPT_CHANGE, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bstrSkill[type], sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::UpdateGameWeather(char* pBuf, uint8_t type)
{
	int index = 0, sendIndex = 0, year = 0, month = 0, date = 0;
	char sendBuffer[128] {};

	// is this user administrator?
	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	if (type == WIZ_WEATHER)
	{
		m_pMain->m_nWeather = GetByte(pBuf, index);
		m_pMain->m_nAmount  = GetShort(pBuf, index);

		SetByte(sendBuffer, WIZ_WEATHER, sendIndex);
		SetByte(sendBuffer, (uint8_t) m_pMain->m_nWeather, sendIndex);
		SetShort(sendBuffer, m_pMain->m_nAmount, sendIndex);
		m_pMain->Send_All(sendBuffer, sendIndex);
	}
	else if (type == WIZ_TIME)
	{
		year             = GetShort(pBuf, index);
		month            = GetShort(pBuf, index);
		date             = GetShort(pBuf, index);
		m_pMain->m_nHour = GetShort(pBuf, index);
		m_pMain->m_nMin  = GetShort(pBuf, index);

		SetByte(sendBuffer, WIZ_TIME, sendIndex);
		SetShort(sendBuffer, year, sendIndex);
		SetShort(sendBuffer, month, sendIndex);
		SetShort(sendBuffer, date, sendIndex);
		SetShort(sendBuffer, m_pMain->m_nHour, sendIndex);
		SetShort(sendBuffer, m_pMain->m_nMin, sendIndex);
		m_pMain->Send_All(sendBuffer, sendIndex);
	}
}

void CUser::ClassChange(char* pBuf)
{
	int index = 0, classcode = 0, sendIndex = 0, type = 0, sub_type = 0, money = 0;
	char sendBuffer[128] {};
	bool bSuccess = false;

	type          = GetByte(pBuf, index);

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

		money    = static_cast<int>(pow((m_pUserData->m_bLevel * 2), 3.4));
		money    = (money / 100) * 100;

		if (m_pUserData->m_bLevel < 30)
			money = static_cast<int>(money * 0.4);
#if 0
		else if (m_pUserData->m_bLevel >= 30
			&& m_pUserData->m_bLevel < 60)
			money = static_cast<int>(money * 1);
#endif
		else if (m_pUserData->m_bLevel >= 60 && m_pUserData->m_bLevel <= 90)
			money = static_cast<int>(money * 1.5);

		// 능력치 포인트
		if (sub_type == 1)
		{
			// 할인시점이고 승리국이라면
			if (m_pMain->m_sDiscount == 1 && m_pMain->m_byOldVictory == m_pUserData->m_bNation)
			{
				// old_money = money;
				money = static_cast<int>(money * 0.5);
				//TRACE(_T("^^ ClassChange -  point Discount ,, money=%d->%d\n"), old_money, money);
			}

			if (m_pMain->m_sDiscount == 2)
			{
				// old_money = money;
				money = static_cast<int>(money * 0.5);
			}

			SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
			SetByte(sendBuffer, CHANGE_MONEY_REQ, sendIndex);
			SetDWORD(sendBuffer, money, sendIndex);
			Send(sendBuffer, sendIndex);
		}
		// skill 포인트
		else if (sub_type == 2)
		{
			// 스킬은 한번 더
			money = static_cast<int>(money * 1.5);

			// 할인시점이고 승리국이라면
			if (m_pMain->m_sDiscount == 1 && m_pMain->m_byOldVictory == m_pUserData->m_bNation)
			{
				// old_money = money;
				money = static_cast<int>(money * 0.5);
				//TRACE(_T("^^ ClassChange -  skillpoint Discount ,, money=%d->%d\n"), old_money, money);
			}

			if (m_pMain->m_sDiscount == 2)
			{
				// old_money = money;
				money = static_cast<int>(money * 0.5);
			}

			SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
			SetByte(sendBuffer, CHANGE_MONEY_REQ, sendIndex);
			SetDWORD(sendBuffer, money, sendIndex);
			Send(sendBuffer, sendIndex);
		}

		return;
	}

	classcode = GetByte(pBuf, index);

	switch (m_pUserData->m_sClass)
	{
		case KARUWARRRIOR:
			if (classcode == BERSERKER || classcode == GUARDIAN)
				bSuccess = true;
			break;

		case KARUROGUE:
			if (classcode == HUNTER || classcode == PENETRATOR)
				bSuccess = true;
			break;

		case KARUWIZARD:
			if (classcode == SORSERER || classcode == NECROMANCER)
				bSuccess = true;
			break;

		case KARUPRIEST:
			if (classcode == SHAMAN || classcode == DARKPRIEST)
				bSuccess = true;
			break;

		case ELMORWARRRIOR:
			if (classcode == BLADE || classcode == PROTECTOR)
				bSuccess = true;
			break;

		case ELMOROGUE:
			if (classcode == RANGER || classcode == ASSASSIN)
				bSuccess = true;
			break;

		case ELMOWIZARD:
			if (classcode == MAGE || classcode == ENCHANTER)
				bSuccess = true;
			break;

		case ELMOPRIEST:
			if (classcode == CLERIC || classcode == DRUID)
				bSuccess = true;
			break;

		default:
			break;
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	if (!bSuccess)
	{
		SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
		SetByte(sendBuffer, CLASS_CHANGE_RESULT, sendIndex);
		SetByte(sendBuffer, 0, sendIndex);
		Send(sendBuffer, sendIndex);
	}
	else
	{
		m_pUserData->m_sClass = classcode;

		if (m_sPartyIndex != -1)
		{
			SetByte(sendBuffer, WIZ_PARTY, sendIndex);
			SetByte(sendBuffer, PARTY_CLASSCHANGE, sendIndex);
			SetShort(sendBuffer, _socketId, sendIndex);
			SetShort(sendBuffer, m_pUserData->m_sClass, sendIndex);
			m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
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
	int index = 0, sendIndex = 0, idlen = 0;
	char chatid[MAX_ID_SIZE + 1] {}, sendBuffer[128] {};

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE || idlen < 0)
		return;

	GetString(chatid, pBuf, idlen, index);

	int socketCount = m_pMain->GetUserSocketCount();
	int i           = 0;
	for (; i < socketCount; i++)
	{
		auto pUser = m_pMain->GetUserPtrUnchecked(i);
		if (pUser != nullptr && pUser->GetState() == CONNECTION_STATE_GAMESTART
			&& strnicmp(chatid, pUser->m_pUserData->m_id, MAX_ID_SIZE) == 0)
		{
			m_sPrivateChatUser = i;
			break;
		}
	}

	SetByte(sendBuffer, WIZ_CHAT_TARGET, sendIndex);
	if (i == socketCount)
		SetShort(sendBuffer, 0, sendIndex);
	else
		SetString2(sendBuffer, chatid, sendIndex);
	Send(sendBuffer, sendIndex);
}

// AI server에 User정보를 전부 전송...
void CUser::SendUserInfo(char* temp_send, int& index)
{
	SetShort(temp_send, _socketId, index);
	SetString2(temp_send, m_pUserData->m_id, index);
	SetByte(temp_send, m_pUserData->m_bZone, index);
	SetShort(temp_send, m_iZoneIndex, index);
	SetByte(temp_send, m_pUserData->m_bNation, index);
	SetByte(temp_send, m_pUserData->m_bLevel, index);
	SetShort(temp_send, m_pUserData->m_sHp, index);
	SetShort(temp_send, m_pUserData->m_sMp, index);
	SetShort(temp_send, m_sTotalHit * m_bAttackAmount / 100, index); // 표시
	SetShort(temp_send, m_sTotalAc + m_sACAmount, index);            // 표시
	SetFloat(temp_send, m_fTotalHitRate, index);
	SetFloat(temp_send, m_fTotalEvasionRate, index);
	SetShort(temp_send, m_sPartyIndex, index);
	SetByte(temp_send, m_pUserData->m_bAuthority, index);
}

void CUser::CountConcurrentUser()
{
	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	int usercount = 0, sendIndex = 0;
	char sendBuffer[128] {};

	int socketCount = m_pMain->GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = m_pMain->GetUserPtrUnchecked(i);
		if (pUser != nullptr && pUser->GetState() == CONNECTION_STATE_GAMESTART)
			++usercount;
	}

	SetByte(sendBuffer, WIZ_CONCURRENTUSER, sendIndex);
	SetShort(sendBuffer, usercount, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::LoyaltyDivide(int tid)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	int levelsum = 0, individualvalue = 0;
	int16_t level_difference = 0, loyalty_source = 0, loyalty_target = 0, average_level = 0;
	uint8_t total_member = 0;

	if (m_sPartyIndex < 0)
		return;

	_PARTY_GROUP* pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
	if (pParty == nullptr)
		return;

	auto pTUser = m_pMain->GetUserPtr(tid);

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
			if (pTUser->m_pUserData->m_bNation == NATION_KARUS)
			{
				++m_pMain->m_sKarusDead;
				//TRACE(_T("++ LoyaltyDivide - ka=%d, el=%d\n"), m_pMain->m_sKarusDead, m_pMain->m_sElmoradDead);
			}
			else if (pTUser->m_pUserData->m_bNation == NATION_ELMORAD)
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
			auto pUser = m_pMain->GetUserPtr(pParty->uid[j]);
			if (pUser == nullptr)
				continue;

			//TRACE(_T("LoyaltyDivide 111 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);

			pUser->m_pUserData->m_iLoyalty += individualvalue;

			// Cannot be less than zero.
			if (pUser->m_pUserData->m_iLoyalty < 0)
				pUser->m_pUserData->m_iLoyalty = 0;

			//TRACE(_T("LoyaltyDivide 222 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);

			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, WIZ_LOYALTY_CHANGE, sendIndex); // Send result to source.
			SetDWORD(sendBuffer, pUser->m_pUserData->m_iLoyalty, sendIndex);
			pUser->Send(sendBuffer, sendIndex);
		}

		return;
	}

	if (m_pUserData->m_bZone != m_pUserData->m_bNation && m_pUserData->m_bZone < 3)
		loyalty_source = 2 * loyalty_source;

	// Distribute loyalty amongst party members.
	for (int j = 0; j < 8; j++)
	{
		auto pUser = m_pMain->GetUserPtr(pParty->uid[j]);
		if (pUser == nullptr)
			continue;

		//TRACE(_T("LoyaltyDivide 333 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);
		individualvalue                 = pUser->m_pUserData->m_bLevel * loyalty_source / levelsum;
		pUser->m_pUserData->m_iLoyalty += individualvalue;

		if (pUser->m_pUserData->m_iLoyalty < 0)
			pUser->m_pUserData->m_iLoyalty = 0;

		//TRACE(_T("LoyaltyDivide 444 - user1=%hs, %d\n"), pUser->m_pUserData->m_id, pUser->m_pUserData->m_iLoyalty);

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_LOYALTY_CHANGE, sendIndex); // Send result to source.
		SetDWORD(sendBuffer, pUser->m_pUserData->m_iLoyalty, sendIndex);
		pUser->Send(sendBuffer, sendIndex);
	}

	pTUser->m_pUserData->m_iLoyalty += loyalty_target; // Recalculate target loyalty.

	if (pTUser->m_pUserData->m_iLoyalty < 0)
		pTUser->m_pUserData->m_iLoyalty = 0;

	//TRACE(_T("LoyaltyDivide 555 - user1=%hs, %d\n"), pTUser->m_pUserData->m_id, pTUser->m_pUserData->m_iLoyalty);

	// Send result to target.
	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_LOYALTY_CHANGE, sendIndex);
	SetDWORD(sendBuffer, pTUser->m_pUserData->m_iLoyalty, sendIndex);
	pTUser->Send(sendBuffer, sendIndex);
}

void CUser::Dead()
{
	int sendIndex      = 0;
	CKnights* pKnights = nullptr;
	char sendBuffer[1024] {}, strKnightsName[MAX_ID_SIZE + 1] {};

	SetByte(sendBuffer, WIZ_DEAD, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	m_pMain->Send_Region(sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ);

	m_bResHpType = USER_DEAD;

	// 유저에게는 바로 데드 패킷을 날림... (한 번 더 보냄, 유령을 없애기 위해서)
	Send(sendBuffer, sendIndex);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;

	// 지휘권한이 있는 유저가 죽는다면,, 지휘 권한 박탈
	if (m_pUserData->m_bFame == KNIGHTS_DUTY_CAPTAIN)
	{
		m_pUserData->m_bFame = KNIGHTS_DUTY_CHIEF;
		SetByte(sendBuffer, WIZ_AUTHORITY_CHANGE, sendIndex);
		SetByte(sendBuffer, COMMAND_AUTHORITY, sendIndex);
		SetShort(sendBuffer, GetSocketID(), sendIndex);
		SetByte(sendBuffer, m_pUserData->m_bFame, sendIndex);
		m_pMain->Send_Region(sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ);
		Send(sendBuffer, sendIndex);

		pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);
		if (pKnights != nullptr)
			strcpy_safe(strKnightsName, pKnights->m_strName);
		else
			strcpy_safe(strKnightsName, "*");

		std::string chatstr;

		//TRACE(_T("---> Dead Captain Deprive - %hs\n"), m_pUserData->m_id);
		if (m_pUserData->m_bNation == NATION_KARUS)
			chatstr = fmt::format_db_resource(
				IDS_KARUS_CAPTAIN_DEPRIVE, strKnightsName, m_pUserData->m_id);
		else if (m_pUserData->m_bNation == NATION_ELMORAD)
			chatstr = fmt::format_db_resource(
				IDS_ELMO_CAPTAIN_DEPRIVE, strKnightsName, m_pUserData->m_id);

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;

		chatstr   = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);
		SetByte(sendBuffer, WIZ_CHAT, sendIndex);
		SetByte(sendBuffer, WAR_SYSTEM_CHAT, sendIndex);
		SetByte(sendBuffer, 1, sendIndex);
		SetShort(sendBuffer, -1, sendIndex);
		SetByte(sendBuffer, 0, sendIndex); // sender name length
		SetString2(sendBuffer, chatstr, sendIndex);
		m_pMain->Send_All(sendBuffer, sendIndex, nullptr, m_pUserData->m_bNation);
	}
}

void CUser::ItemWoreOut(int type, int damage)
{
	int worerate = static_cast<int>(sqrt(damage / 10.0));
	if (worerate == 0)
		return;

	if (type == DURABILITY_TYPE_ATTACK)
	{
		if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[RIGHTHAND].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[RIGHTHAND].nNum);
			if (pTable != nullptr
				// 2 == DEFENCE ITEM
				&& pTable->Slot != 2)
			{
				m_pUserData->m_sItemArray[RIGHTHAND].sDuration -= worerate;
				ItemDurationChange(RIGHTHAND, pTable->Durability,
					m_pUserData->m_sItemArray[RIGHTHAND].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[LEFTHAND].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[LEFTHAND].nNum);
			if (pTable != nullptr
				// 2 == DEFENCE ITEM
				&& pTable->Slot != 2)
			{
				m_pUserData->m_sItemArray[LEFTHAND].sDuration -= worerate;
				ItemDurationChange(LEFTHAND, pTable->Durability,
					m_pUserData->m_sItemArray[LEFTHAND].sDuration, worerate);
			}
		}
	}
	else if (type == DURABILITY_TYPE_DEFENCE)
	{
		if (m_pUserData->m_sItemArray[HEAD].nNum != 0
			&& m_pUserData->m_sItemArray[HEAD].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[HEAD].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[HEAD].sDuration -= worerate;
				ItemDurationChange(
					HEAD, pTable->Durability, m_pUserData->m_sItemArray[HEAD].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[BREAST].nNum != 0
			&& m_pUserData->m_sItemArray[BREAST].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[BREAST].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[BREAST].sDuration -= worerate;
				ItemDurationChange(BREAST, pTable->Durability,
					m_pUserData->m_sItemArray[BREAST].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[LEG].nNum != 0
			&& m_pUserData->m_sItemArray[LEG].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[LEG].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[LEG].sDuration -= worerate;
				ItemDurationChange(
					LEG, pTable->Durability, m_pUserData->m_sItemArray[LEG].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[GLOVE].nNum != 0
			&& m_pUserData->m_sItemArray[GLOVE].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[GLOVE].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[GLOVE].sDuration -= worerate;
				ItemDurationChange(GLOVE, pTable->Durability,
					m_pUserData->m_sItemArray[GLOVE].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[FOOT].nNum != 0
			&& m_pUserData->m_sItemArray[FOOT].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[FOOT].nNum);
			if (pTable != nullptr)
			{
				m_pUserData->m_sItemArray[FOOT].sDuration -= worerate;
				ItemDurationChange(
					FOOT, pTable->Durability, m_pUserData->m_sItemArray[FOOT].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[RIGHTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[RIGHTHAND].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[RIGHTHAND].nNum);
			if (pTable != nullptr
				// 방패?
				&& pTable->Slot == 2)
			{
				m_pUserData->m_sItemArray[RIGHTHAND].sDuration -= worerate;
				ItemDurationChange(RIGHTHAND, pTable->Durability,
					m_pUserData->m_sItemArray[RIGHTHAND].sDuration, worerate);
			}
		}

		if (m_pUserData->m_sItemArray[LEFTHAND].nNum != 0
			&& m_pUserData->m_sItemArray[LEFTHAND].sDuration != 0)
		{
			model::Item* pTable = m_pMain->m_ItemTableMap.GetData(
				m_pUserData->m_sItemArray[LEFTHAND].nNum);
			if (pTable
				// 방패?
				&& pTable->Slot == 2)
			{
				m_pUserData->m_sItemArray[LEFTHAND].sDuration -= worerate;
				ItemDurationChange(LEFTHAND, pTable->Durability,
					m_pUserData->m_sItemArray[LEFTHAND].sDuration, worerate);
			}
		}
	}
}

void CUser::ItemDurationChange(int slot, int maxvalue, int curvalue, int amount)
{
	if (maxvalue <= 0)
		return;

	if (slot < 0 || slot > SLOT_MAX)
		return;

	int curpercent = 0, beforepercent = 0, curbasis = 0, beforebasis = 0;
	int sendIndex = 0;
	char sendBuffer[128] {};

	if (m_pUserData->m_sItemArray[slot].sDuration <= 0)
	{
		m_pUserData->m_sItemArray[slot].sDuration = 0;

		SetByte(sendBuffer, WIZ_DURATION, sendIndex);
		SetByte(sendBuffer, slot, sendIndex);
		SetShort(sendBuffer, 0, sendIndex);
		Send(sendBuffer, sendIndex);

		SetSlotItemValue();
		SetUserAbility();

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_ITEM_MOVE, sendIndex); // durability 변경에 따른 수치 재조정...
		SetByte(sendBuffer, 0x01, sendIndex);
		SetShort(sendBuffer, m_sTotalHit, sendIndex);
		SetShort(sendBuffer, m_sTotalAc, sendIndex);
		SetShort(sendBuffer, GetCurrentWeightForClient(), sendIndex);
		SetShort(sendBuffer, m_iMaxHp, sendIndex);
		SetShort(sendBuffer, m_iMaxMp, sendIndex);
		SetShort(sendBuffer, m_sItemStr + m_sStrAmount, sendIndex);
		SetShort(sendBuffer, m_sItemSta + m_sStaAmount, sendIndex);
		SetShort(sendBuffer, m_sItemDex + m_sDexAmount, sendIndex);
		SetShort(sendBuffer, m_sItemIntel + m_sIntelAmount, sendIndex);
		SetShort(sendBuffer, m_sItemCham + m_sChaAmount, sendIndex);
		SetShort(sendBuffer, m_bFireR, sendIndex);
		SetShort(sendBuffer, m_bColdR, sendIndex);
		SetShort(sendBuffer, m_bLightningR, sendIndex);
		SetShort(sendBuffer, m_bMagicR, sendIndex);
		SetShort(sendBuffer, m_bDiseaseR, sendIndex);
		SetShort(sendBuffer, m_bPoisonR, sendIndex);
		Send(sendBuffer, sendIndex);
		return;
	}

	curpercent    = static_cast<int>((curvalue / (double) maxvalue) * 100);
	beforepercent = static_cast<int>(((curvalue + amount) / (double) maxvalue) * 100);

	curbasis      = curpercent / 5;
	beforebasis   = beforepercent / 5;

	if (curbasis != beforebasis)
	{
		SetByte(sendBuffer, WIZ_DURATION, sendIndex);
		SetByte(sendBuffer, slot, sendIndex);
		SetShort(sendBuffer, curvalue, sendIndex);
		Send(sendBuffer, sendIndex);

		if (curpercent >= 65 && curpercent < 70)
			UserLookChange(slot, m_pUserData->m_sItemArray[slot].nNum, curvalue);

		if (curpercent >= 25 && curpercent < 30)
			UserLookChange(slot, m_pUserData->m_sItemArray[slot].nNum, curvalue);
	}
}

void CUser::HPTimeChange(double currentTime)
{
	m_fHPLastTimeNormal = currentTime;

	if (m_bResHpType == USER_DEAD)
		return;

	//char logstr[128] {};
	//wsprintf(logstr, "HPTimeChange ,, nid=%d, name=%hs, hp=%d, type=%d ******", _socketId, m_pUserData->m_id, m_pUserData->m_sHp, m_bResHpType);
	//TimeTrace(logstr);

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE && m_pMain->m_byBattleOpen == SNOW_BATTLE)
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
		{
			HpChange(
				(int) ((m_pUserData->m_bLevel * (1 + m_pUserData->m_bLevel / 60.0) + 1) * 0.2) + 3);
		}

		if (m_iMaxMp != m_pUserData->m_sMp)
		{
			MSpChange(
				(int) ((m_pUserData->m_bLevel * (1 + m_pUserData->m_bLevel / 60.0) + 1) * 0.2) + 3);
		}
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

void CUser::HPTimeChangeType3(double currentTime)
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	// Get the current time for all the last times...
	for (int g = 0; g < MAX_TYPE3_REPEAT; g++)
		m_fHPLastTime[g] = currentTime;

	// Make sure the user is not dead first!!!
	if (m_bResHpType == USER_DEAD)
		return;

	for (int h = 0; h < MAX_TYPE3_REPEAT; h++)
	{
		HpChange(m_bHPAmount[h]); // Reduce HP...

		// Send report to the source...
		auto pUser = m_pMain->GetUserPtr(m_sSourceID[h]);
		if (pUser != nullptr)
			pUser->SendTargetHP(0, _socketId, m_bHPAmount[h]);

		// Check if the target is dead.
		if (m_pUserData->m_sHp == 0)
		{
			m_bResHpType = USER_DEAD; // Officially declare the user DEAD!!!!!

			// If the killer was a NPC
			if (m_sSourceID[h] >= NPC_BAND)
			{
				if (m_pUserData->m_bZone != m_pUserData->m_bNation && m_pUserData->m_bZone < 3)
				{
					ExpChange(-m_iMaxExp / 100);
					//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
				}
				else
				{
					ExpChange(-m_iMaxExp / 20); // Reduce target experience.
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
						pUser->LoyaltyChange(_socketId);
					else
						pUser->LoyaltyDivide(_socketId);

					pUser->GoldChange(_socketId, 0);
				}
			}
			// 기범이의 완벽한 보호 코딩!!!
			InitType3(); // Init Type 3.....
			InitType4(); // Init Type 4.....

			if (m_pMain->IsValidUserId(m_sSourceID[h]))
			{
				m_sWhoKilledMe = m_sSourceID[h]; // Who the hell killed me?
												 //
				if (m_pUserData->m_bZone != m_pUserData->m_bNation && m_pUserData->m_bZone < 3)
				{
					ExpChange(-m_iMaxExp / 100);
					//TRACE(_T("정말로 1%만 깍였다니까요 ㅠ.ㅠ\r\n"));
				}
				//
			}

			break; // Exit the for loop :)
		}
	}

	// Type 3 Cancellation Process.
	for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
	{
		if (m_bHPDuration[i] > 0)
		{
			if (((currentTime - m_fHPStartTime[i]) >= m_bHPDuration[i])
				|| m_bResHpType == USER_DEAD)
			{
				/*	Send Party Packet.....
				if (m_sPartyIndex != -1)
				{
					SetByte(sendBuffer, WIZ_PARTY, sendIndex );
					SetByte(sendBuffer, PARTY_STATUSCHANGE, sendIndex );
					SetShort(sendBuffer, _socketId, sendIndex );
					SetByte(sendBuffer, 1, sendIndex );
					SetByte(sendBuffer, 0, sendIndex);
					m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
					memset(sendBuffer, 0, sizeof(sendBuffer));
					sendIndex = 0;
				}
				//  end of Send Party Packet.....*/

				SetByte(sendBuffer, WIZ_MAGIC_PROCESS, sendIndex);
				SetByte(sendBuffer, MAGIC_TYPE3_END, sendIndex);

				if (m_bHPAmount[i] > 0)
					SetByte(sendBuffer, 100, sendIndex);
				else
					SetByte(sendBuffer, 200, sendIndex);

				Send(sendBuffer, sendIndex);
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sendIndex         = 0;

				m_fHPStartTime[i] = 0.0;
				m_fHPLastTime[i]  = 0.0;
				m_bHPAmount[i]    = 0;
				m_bHPDuration[i]  = 0;
				m_bHPInterval[i]  = 5;
				m_sSourceID[i]    = -1;
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
	if (m_sPartyIndex != -1 && bType3Test)
	{
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_STATUSCHANGE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetByte(sendBuffer, 1, sendIndex);
		SetByte(sendBuffer, 0, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
	}
	//  end of Send Party Packet.....  //
	//
}

void CUser::ItemRepair(char* pBuf)
{
	int index = 0, sendIndex = 0, money = 0, quantity = 0;
	int itemid = 0, pos = 0, slot = -1, durability = 0;
	model::Item* pTable = nullptr;
	char sendBuffer[128] {};

	pos    = GetByte(pBuf, index);
	slot   = GetByte(pBuf, index);
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

	money = static_cast<int>((((pTable->BuyPrice - 10) / 10000.0f) + pow(pTable->BuyPrice, 0.75))
							 * quantity / (double) durability);
	if (money > m_pUserData->m_iGold)
		goto fail_return;

	m_pUserData->m_iGold -= money;
	if (pos == 1)
		m_pUserData->m_sItemArray[slot].sDuration = durability;
	else if (pos == 2)
		m_pUserData->m_sItemArray[SLOT_MAX + slot].sDuration = durability;

	SetByte(sendBuffer, WIZ_ITEM_REPAIR, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_ITEM_REPAIR, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::Type4Duration(double currentTime)
{
	int sendIndex = 0, buff_type = 0;
	char sendBuffer[128] {};

	if (m_sDuration1 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime1 + m_sDuration1))
		{
			m_sDuration1   = 0;
			m_fStartTime1  = 0.0;
			m_sMaxHPAmount = 0;
			buff_type      = 1;
		}
	}

	if (m_sDuration2 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime2 + m_sDuration2))
		{
			m_sDuration2  = 0;
			m_fStartTime2 = 0.0;
			m_sACAmount   = 0;
			buff_type     = 2;
		}
	}

	if (m_sDuration3 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime3 + m_sDuration3))
		{
			m_sDuration3  = 0;
			m_fStartTime3 = 0.0;
			buff_type     = 3;

			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, 3, sendIndex); // You are now normal again!!!
			SetByte(sendBuffer, ABNORMAL_NORMAL, sendIndex);
			StateChange(sendBuffer);
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
		}
	}

	if (m_sDuration4 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime4 + m_sDuration4))
		{
			m_sDuration4    = 0;
			m_fStartTime4   = 0.0;
			m_bAttackAmount = 100;
			buff_type       = 4;
		}
	}

	if (m_sDuration5 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime5 + m_sDuration5))
		{
			m_sDuration5         = 0;
			m_fStartTime5        = 0.0;
			m_bAttackSpeedAmount = 100;
			buff_type            = 5;
		}
	}

	if (m_sDuration6 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime6 + m_sDuration6))
		{
			m_sDuration6   = 0;
			m_fStartTime6  = 0.0;
			m_bSpeedAmount = 100;
			buff_type      = 6;
		}
	}

	if (m_sDuration7 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime7 + m_sDuration7))
		{
			m_sDuration7   = 0;
			m_fStartTime7  = 0.0;
			m_sStrAmount   = 0;
			m_sStaAmount   = 0;
			m_sDexAmount   = 0;
			m_sIntelAmount = 0;
			m_sChaAmount   = 0;
			buff_type      = 7;
		}
	}

	if (m_sDuration8 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime8 + m_sDuration8))
		{
			m_sDuration8        = 0;
			m_fStartTime8       = 0.0;
			m_bFireRAmount      = 0;
			m_bColdRAmount      = 0;
			m_bLightningRAmount = 0;
			m_bMagicRAmount     = 0;
			m_bDiseaseRAmount   = 0;
			m_bPoisonRAmount    = 0;
			buff_type           = 8;
		}
	}

	if (m_sDuration9 != 0 && buff_type == 0)
	{
		if (currentTime > (m_fStartTime9 + m_sDuration9))
		{
			m_sDuration9       = 0;
			m_fStartTime9      = 0.0;
			m_bHitRateAmount   = 100;
			m_sAvoidRateAmount = 100;
			buff_type          = 9;
		}
	}

	if (buff_type != 0)
	{
		m_bType4Buff[buff_type - 1] = 0;

		SetSlotItemValue();
		SetUserAbility();
		Send2AI_UserUpdateInfo(); // AI Server에 바끤 데이타 전송....

		/*	Send Party Packet.....
		if (m_sPartyIndex != -1)
		{
			SetByte(sendBuffer, WIZ_PARTY, sendIndex);
			SetByte(sendBuffer, PARTY_STATUSCHANGE, sendIndex);
			SetShort(sendBuffer, _socketId, sendIndex);
//			if (buff_type != 5 && buff_type != 6)
//				SetByte(sendBuffer, 3, sendIndex);
//			else
				SetByte(sendBuffer, 2, sendIndex);
			SetByte(sendBuffer, 0, sendIndex);
			m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
		}
		//  end of Send Party Packet.....  */

		SetByte(sendBuffer, WIZ_MAGIC_PROCESS, sendIndex);
		SetByte(sendBuffer, MAGIC_TYPE4_END, sendIndex);
		SetByte(sendBuffer, buff_type, sendIndex);
		Send(sendBuffer, sendIndex);
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
	if (m_sPartyIndex != -1 && bType4Test)
	{
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_PARTY, sendIndex);
		SetByte(sendBuffer, PARTY_STATUSCHANGE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetByte(sendBuffer, 2, sendIndex);
		SetByte(sendBuffer, 0, sendIndex);
		m_pMain->Send_PartyMember(m_sPartyIndex, sendBuffer, sendIndex);
	}
	//  end of Send Party Packet.....  //
	//
}

// Returns:
// 0: Requested item not available.
// 1: Amount is greater than current item count.
// 2: Success. Current item count updated or deleted.
uint8_t CUser::ItemCountChange(int itemid, int type, int amount)
{
	int sendIndex = 0, result = 0, slot = -1;
	char sendBuffer[128] {};

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

		if (pTable->RequiredDexterity > (m_pUserData->m_bDex + m_sItemDex + m_sDexAmount)
			&& pTable->RequiredDexterity != 0)
			return result;

		if (pTable->RequiredStrength > (m_pUserData->m_bStr + m_sItemStr + m_sStrAmount)
			&& pTable->RequiredStrength != 0)
			return result;

		if (pTable->RequiredStamina > (m_pUserData->m_bSta + m_sItemSta + m_sStaAmount)
			&& pTable->RequiredStamina != 0)
			return result;

		if (pTable->RequiredIntelligence > (m_pUserData->m_bIntel + m_sItemIntel + m_sIntelAmount)
			&& pTable->RequiredIntelligence != 0)
			return result;

		if (pTable->RequiredCharisma > (m_pUserData->m_bCha + m_sItemCham + m_sChaAmount)
			&& pTable->RequiredCharisma != 0)
			return result;

		// This checks if the user actually has that item.
		if (pTable->Countable == 0)
		{
			result = 2;
			return result;
		}

		slot                                 = i;
		m_pUserData->m_sItemArray[i].sCount -= amount;

		// You have just ran out of items.
		if (m_pUserData->m_sItemArray[i].sCount == 0)
		{
			m_pUserData->m_sItemArray[i].nNum = 0;
			result                            = 2;
			break;
		}
		// Insufficient number of items.
		else if (m_pUserData->m_sItemArray[i].sCount < 0)
		{
			m_pUserData->m_sItemArray[i].sCount += amount;
			result                               = 1;
			break;
		}

		// No error detected....
		result = 2;
	}

	// Something happened :(
	if (result < 2)
		return result;

	SendItemWeight();

	SetByte(sendBuffer, WIZ_ITEM_COUNT_CHANGE, sendIndex);
	SetShort(sendBuffer, 1, sendIndex);      // The number of for-loops
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, slot - type * SLOT_MAX, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex); // The ID of item.
	SetDWORD(sendBuffer, m_pUserData->m_sItemArray[slot].sCount, sendIndex);

	Send(sendBuffer, sendIndex);

	return result; // Success :)
}

void CUser::SendAllKnightsID()
{
	int sendIndex = 0, count = 0, buff_index = 0;
	char sendBuffer[4096] {}, temp_buff[4096] {};

	for (const auto& [_, pKnights] : m_pMain->m_KnightsMap)
	{
		if (pKnights == nullptr)
			continue;

		//if (pKnights->bFlag != KNIGHTS_TYPE)
		//	continue;

		SetShort(temp_buff, pKnights->m_sIndex, buff_index);
		SetString2(temp_buff, pKnights->m_strName, buff_index);
		++count;
	}

	SetByte(sendBuffer, WIZ_KNIGHTS_LIST, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex); // All List
	SetShort(sendBuffer, count, sendIndex);
	SetString(sendBuffer, temp_buff, buff_index, sendIndex);
	SendCompressingPacket(sendBuffer, sendIndex);
	//Send( sendBuffer, sendIndex );
}

void CUser::ItemRemove(char* pBuf)
{
	int index = 0, sendIndex = 0, slot = 0, pos = 0, itemid = 0, count = 0, durability = 0;
	int64_t serial = 0;
	char sendBuffer[128] {};

	slot   = GetByte(pBuf, index);
	pos    = GetByte(pBuf, index);
	itemid = GetDWORD(pBuf, index);

	if (slot == 1)
	{
		if (pos < 0 || pos >= SLOT_MAX)
			goto fail_return;

		if (m_pUserData->m_sItemArray[pos].nNum != itemid)
			goto fail_return;

		count                                     = m_pUserData->m_sItemArray[pos].sCount;
		durability                                = m_pUserData->m_sItemArray[pos].sDuration;
		serial                                    = m_pUserData->m_sItemArray[pos].nSerialNum;

		m_pUserData->m_sItemArray[pos].nNum       = 0;
		m_pUserData->m_sItemArray[pos].sCount     = 0;
		m_pUserData->m_sItemArray[pos].sDuration  = 0;
		m_pUserData->m_sItemArray[pos].nSerialNum = 0;
	}
	else
	{
		if (pos < 0 || pos >= HAVE_MAX)
			goto fail_return;

		if (m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum != itemid)
			goto fail_return;

		count      = m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount;
		durability = m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration;
		serial     = m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum;

		m_pUserData->m_sItemArray[SLOT_MAX + pos].nNum       = 0;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount     = 0;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].sDuration  = 0;
		m_pUserData->m_sItemArray[SLOT_MAX + pos].nSerialNum = 0;
	}

	SendItemWeight();

	SetByte(sendBuffer, WIZ_ITEM_REMOVE, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	Send(sendBuffer, sendIndex);

	ItemLogToAgent(
		m_pUserData->m_id, "DESTROY", ITEM_LOG_DESTROY, serial, itemid, count, durability);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_ITEM_REMOVE, sendIndex);
	SetByte(sendBuffer, 0, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::OperatorCommand(char* pBuf)
{
	int index = 0, idlen = 0, command = 0;
	char userid[MAX_ID_SIZE + 1] {};

	// Is this user's authority operator?
	if (m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
		return;

	command = GetByte(pBuf, index);
	idlen   = GetShort(pBuf, index);

	if (idlen < 0 || idlen > MAX_ID_SIZE)
		return;

	GetString(userid, pBuf, idlen, index);

	auto pUser = m_pMain->GetUserPtr(userid, NameType::Character);
	if (pUser == nullptr)
		return;

	switch (command)
	{
		case OPERATOR_ARREST:
			ZoneChange(pUser->m_pUserData->m_bZone, pUser->m_pUserData->m_curx,
				pUser->m_pUserData->m_curz);
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

		default:
			spdlog::error(
				"User::OperatorCommand: Unhandled opcode {:02X} [accountName={} characterName={}]",
				command, m_strAccountID, m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			break;
	}
}

void CUser::SpeedHackTime(char* pBuf)
{
	uint8_t b_first   = 0x00;
	int index         = 0;
	double serverTime = 0.0, clientTime = 0.0, clientGap = 0.0, serverGap = 0.0;

	b_first    = GetByte(pBuf, index);
	clientTime = GetFloat(pBuf, index);

	if (b_first)
	{
		m_fSpeedHackClientTime = clientTime;
		m_fSpeedHackServerTime = TimeGet();
	}
	else
	{
		serverTime = TimeGet();

		serverGap  = serverTime - m_fSpeedHackServerTime;
		clientGap  = clientTime - m_fSpeedHackClientTime;

		if ((clientGap - serverGap) > 10.0)
		{
			spdlog::debug(
				"User::SpeedHackTime: speed hack check performed on charId={}", m_pUserData->m_id);
			Close();
		}
		else if ((clientGap - serverGap) < 0.0)
		{
			m_fSpeedHackClientTime = clientTime;
			m_fSpeedHackServerTime = TimeGet();
		}
	}
}

// server의 상태를 체크..
void CUser::ServerStatusCheck()
{
	int sendIndex = 0;
	char sendBuffer[256] {};
	SetByte(sendBuffer, WIZ_SERVER_CHECK, sendIndex);
	SetShort(sendBuffer, m_pMain->m_sErrorSocketCount, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::Type3AreaDuration(double currentTime)
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	CMagicProcess magic_process;

	model::MagicType3* pType = m_pMain->m_MagicType3TableMap.GetData(
		m_iAreaMagicID); // Get magic skill table type 3.
	if (pType == nullptr)
		return;

	// Did one second pass?
	if (m_fAreaLastTime != 0.0 && (currentTime - m_fAreaLastTime) > m_bAreaInterval)
	{
		m_fAreaLastTime = currentTime;
		if (m_bResHpType == USER_DEAD)
			return;

		// Actual damage procedure.
		int socketCount = m_pMain->GetUserSocketCount();
		for (int i = 0; i < socketCount; i++)
		{
			// Region check.
			if (!magic_process.UserRegionCheck(_socketId, i, m_iAreaMagicID, pType->Radius))
				continue;

			auto pTUser = m_pMain->GetUserPtrUnchecked(i);
			if (pTUser == nullptr)
				continue;

			SetByte(sendBuffer, WIZ_MAGIC_PROCESS, sendIndex); // Set packet.
			SetByte(sendBuffer, MAGIC_EFFECTING, sendIndex);
			SetDWORD(sendBuffer, m_iAreaMagicID, sendIndex);
			SetShort(sendBuffer, _socketId, sendIndex);
			SetShort(sendBuffer, i, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			m_pMain->Send_Region(
				sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
		}

		// Did area duration end?
		if (((currentTime - m_fAreaStartTime) >= pType->Duration) || m_bResHpType == USER_DEAD)
		{
			m_bAreaInterval  = 5;
			m_fAreaStartTime = 0.0;
			m_fAreaLastTime  = 0.0;
			m_iAreaMagicID   = 0;
		}
	}

	SetByte(sendBuffer, WIZ_MAGIC_PROCESS, sendIndex); // Set packet.
	SetByte(sendBuffer, MAGIC_EFFECTING, sendIndex);
	SetDWORD(sendBuffer, m_iAreaMagicID, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetShort(sendBuffer, 0, sendIndex);
	SetShort(sendBuffer, 0, sendIndex);
	SetShort(sendBuffer, 0, sendIndex);

	// Send packet to region.
	m_pMain->Send_Region(
		sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
}

void CUser::WarehouseProcess(char* pBuf)
{
	int index = 0, sendIndex = 0, itemid = 0, srcpos = -1, destpos = -1, page = -1,
		reference_pos = -1, count = 0;
	model::Item* pTable = nullptr;
	char sendBuffer[2048] {};

	uint8_t command = GetByte(pBuf, index);

	// 창고 안되게...
	if (m_bResHpType == USER_DEAD || m_pUserData->m_sHp == 0)
	{
		spdlog::error("User::WarehouseProcess: dead user cannot use warehouse [charId={} "
					  "socketId={} resHpType={} hp={} x={} z={}]",
			m_pUserData->m_id, _socketId, m_bResHpType, m_pUserData->m_sHp,
			static_cast<int32_t>(m_pUserData->m_curx), static_cast<int32_t>(m_pUserData->m_curz));
		return;
	}

	if (m_sExchangeUser != -1)
		goto fail_return;

	if (command == WAREHOUSE_OPEN)
	{
		SetByte(sendBuffer, WIZ_WAREHOUSE, sendIndex);
		SetByte(sendBuffer, WAREHOUSE_OPEN, sendIndex);
		SetByte(sendBuffer, 1, sendIndex); /* success */
		SetDWORD(sendBuffer, m_pUserData->m_iBank, sendIndex);

		for (int i = 0; i < WAREHOUSE_MAX; i++)
		{
			SetDWORD(sendBuffer, m_pUserData->m_sWarehouseArray[i].nNum, sendIndex);
			SetShort(sendBuffer, m_pUserData->m_sWarehouseArray[i].sDuration, sendIndex);
			SetShort(sendBuffer, m_pUserData->m_sWarehouseArray[i].sCount, sendIndex);
		}

		SendCompressingPacket(sendBuffer, sendIndex);
		return;
	}

	itemid  = GetDWORD(pBuf, index);
	page    = GetByte(pBuf, index);
	srcpos  = GetByte(pBuf, index);
	destpos = GetByte(pBuf, index);
	pTable  = m_pMain->m_ItemTableMap.GetData(itemid);

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
			m_pUserData->m_sWarehouseArray[reference_pos + destpos]
				.sDuration = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos]
				.nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

			if (pTable->Countable == 0
				&& m_pUserData->m_sWarehouseArray[reference_pos + destpos].nSerialNum == 0)
				m_pUserData->m_sWarehouseArray[reference_pos + destpos]
					.nSerialNum = m_pMain->GenerateItemSerial();

			if (pTable->Countable != 0)
				m_pUserData->m_sWarehouseArray[reference_pos + destpos].sCount += count;
			else
				m_pUserData->m_sWarehouseArray[reference_pos + destpos]
					.sCount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;

			if (!pTable->Countable)
			{
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum       = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration  = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount     = 0;
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = 0;
			}
			else
			{
				m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount -= count;

				if (m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount <= 0)
				{
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum       = 0;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration  = 0;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount     = 0;
					m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum = 0;
				}
			}

			SendItemWeight();
			ItemLogToAgent(m_pUserData->m_Accountid, m_pUserData->m_id, ITEM_LOG_WAREHOUSE_PUT, 0,
				itemid, count, m_pUserData->m_sWarehouseArray[reference_pos + destpos].sDuration);
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

			if (m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum && pTable->Countable == 0)
				goto fail_return;

			if (m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount < count)
				goto fail_return;

			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum = itemid;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos]
				.sDuration = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos]
				.nSerialNum = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum;

			if (pTable->Countable != 0)
			{
				m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount += count;
			}
			else
			{
				if (m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum == 0)
					m_pUserData->m_sItemArray[SLOT_MAX + destpos]
						.nSerialNum = m_pMain->GenerateItemSerial();

				m_pUserData->m_sItemArray[SLOT_MAX + destpos]
					.sCount = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount;
			}

			if (pTable->Countable == 0)
			{
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum       = 0;
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration  = 0;
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount     = 0;
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum = 0;
			}
			else
			{
				m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount -= count;

				if (m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount <= 0)
				{
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum       = 0;
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration  = 0;
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount     = 0;
					m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum = 0;
				}
			}

			SendItemWeight();
			ItemLogToAgent(m_pUserData->m_id, m_pUserData->m_Accountid, ITEM_LOG_WAREHOUSE_GET, 0,
				itemid, count, m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration);
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
			m_pUserData->m_sWarehouseArray[reference_pos + destpos]
				.sDuration = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos]
				.sCount = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount;
			m_pUserData->m_sWarehouseArray[reference_pos + destpos]
				.nSerialNum = m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum;

			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nNum       = 0;
			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sDuration  = 0;
			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].sCount     = 0;
			m_pUserData->m_sWarehouseArray[reference_pos + srcpos].nSerialNum = 0;
			break;

		case WAREHOUSE_INVENMOVE:
		{
			if (itemid != m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nNum)
				goto fail_return;

			int16_t duration  = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sDuration;
			int16_t itemcount = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].sCount;
			int64_t serial    = m_pUserData->m_sItemArray[SLOT_MAX + srcpos].nSerialNum;

			m_pUserData->m_sItemArray[SLOT_MAX + srcpos]
				.nNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum;
			m_pUserData->m_sItemArray[SLOT_MAX + srcpos]
				.sDuration = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration;
			m_pUserData->m_sItemArray[SLOT_MAX + srcpos]
				.sCount = m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount;
			m_pUserData->m_sItemArray[SLOT_MAX + srcpos]
				.nSerialNum = m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum;

			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nNum       = itemid;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].sDuration  = duration;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].sCount     = itemcount;
			m_pUserData->m_sItemArray[SLOT_MAX + destpos].nSerialNum = serial;
		}
		break;

		default:
			spdlog::error(
				"User::WarehouseProcess: Unhandled opcode {:02X} [accountName={} characterName={}]",
				command, m_strAccountID, m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			return;
	}

	m_pUserData->m_bWarehouse = 1;

	SetByte(sendBuffer, WIZ_WAREHOUSE, sendIndex);
	SetByte(sendBuffer, command, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_WAREHOUSE, sendIndex);
	SetByte(sendBuffer, command, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::InitType4()
{
	m_bAttackSpeedAmount = 100; // this is for the duration spells Type 4
	m_bSpeedAmount       = 100;
	m_sACAmount          = 0;
	m_bAttackAmount      = 100;
	m_sMaxHPAmount       = 0;
	m_bHitRateAmount     = 100;
	m_sAvoidRateAmount   = 100;
	m_sStrAmount         = 0;
	m_sStaAmount         = 0;
	m_sDexAmount         = 0;
	m_sIntelAmount       = 0;
	m_sChaAmount         = 0;
	m_bFireRAmount       = 0;
	m_bColdRAmount       = 0;
	m_bLightningRAmount  = 0;
	m_bMagicRAmount      = 0;
	m_bDiseaseRAmount    = 0;
	m_bPoisonRAmount     = 0;
	// 비러머글 수능
	m_bAbnormalType      = 1;
	//
	m_sDuration1         = 0;
	m_fStartTime1        = 0.0; // Used for Type 4 Durational Spells.
	m_sDuration2         = 0;
	m_fStartTime2        = 0.0;
	m_sDuration3         = 0;
	m_fStartTime3        = 0.0;
	m_sDuration4         = 0;
	m_fStartTime4        = 0.0;
	m_sDuration5         = 0;
	m_fStartTime5        = 0.0;
	m_sDuration6         = 0;
	m_fStartTime6        = 0.0;
	m_sDuration7         = 0;
	m_fStartTime7        = 0.0;
	m_sDuration8         = 0;
	m_fStartTime8        = 0.0;
	m_sDuration9         = 0;
	m_fStartTime9        = 0.0;

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

	int index = 0, chatlen = 0;
	char chatMsg[1024] {};

	chatlen = GetShort(pBuf, index);
	if (chatlen > 512 || chatlen <= 0)
		return;

	GetString(chatMsg, pBuf, chatlen, index);

	//	TRACE( " Short : %d   String : %hs  \n ", chatlen, chatstr);
	if (strlen(m_pUserData->m_id) == 0)
		return;

	spdlog::info("User::ReportBug: [charId={} chatMsg={}]", m_pUserData->m_id, chatMsg);
}

void CUser::Home()
{
	int16_t x = 0, z = 0;
	if (!GetStartPosition(&x, &z, m_pUserData->m_bZone))
		return;

	Warp(static_cast<float>(x), static_cast<float>(z));
}

bool CUser::GetStartPosition(int16_t* x, int16_t* z, int zoneId) const
{
	model::StartPosition* startPosition = m_pMain->m_StartPositionTableMap.GetData(zoneId);
	if (startPosition == nullptr)
		return false;

	if (m_pUserData->m_bNation == NATION_KARUS)
	{
		*x = startPosition->KarusX + myrand(0, startPosition->RangeX);
		*z = startPosition->KarusZ + myrand(0, startPosition->RangeZ);
		return true;
	}
	else if (m_pUserData->m_bNation == NATION_ELMORAD)
	{
		*x = startPosition->ElmoX + myrand(0, startPosition->RangeX);
		*z = startPosition->ElmoZ + myrand(0, startPosition->RangeZ);
		return true;
	}

	return false;
}

std::shared_ptr<CUser> CUser::GetItemRoutingUser(int itemid, int16_t /*itemcount*/)
{
	if (m_sPartyIndex == -1)
		return nullptr;

	_PARTY_GROUP* pParty = nullptr;
	int count            = 0;

	pParty               = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
	if (pParty == nullptr)
		return nullptr;

	if (pParty->bItemRouting > 7)
		return nullptr;

	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return nullptr;

	while (count < 8)
	{
		int select_user = pParty->uid[pParty->bItemRouting];
		auto pUser      = m_pMain->GetUserPtr(select_user);
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
	int index = 0, sendIndex = 0;
	int16_t usercount = 0, idlen = 0;
	char sendBuffer[256] {}, userid[MAX_ID_SIZE + 1] {};

	usercount = GetShort(pBuf, index); // Get usercount packet.
	if (usercount >= 30 || usercount < 0)
		return;

	SetByte(sendBuffer, WIZ_FRIEND_PROCESS, sendIndex);
	SetShort(sendBuffer, usercount, sendIndex);

	for (int k = 0; k < usercount; k++)
	{
		idlen = GetShort(pBuf, index);
		if (idlen > MAX_ID_SIZE)
		{
			SetString2(sendBuffer, userid, sendIndex);
			SetShort(sendBuffer, -1, sendIndex);
			SetByte(sendBuffer, 0, sendIndex);
			continue;
		}

		GetString(userid, pBuf, idlen, index);

		auto pUser = m_pMain->GetUserPtr(userid, NameType::Character);

		SetShort(sendBuffer, idlen, sendIndex);
		SetString(sendBuffer, userid, idlen, sendIndex);

		// No such user
		if (pUser == nullptr)
		{
			SetShort(sendBuffer, -1, sendIndex);
			SetByte(sendBuffer, 0, sendIndex);
		}
		else
		{
			SetShort(sendBuffer, pUser->_socketId, sendIndex);
			if (pUser->m_sPartyIndex >= 0)
				SetByte(sendBuffer, 3, sendIndex);
			else
				SetByte(sendBuffer, 1, sendIndex);
		}
	}

	Send(sendBuffer, sendIndex);
}

void CUser::ClassChangeReq()
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
	SetByte(sendBuffer, CLASS_CHANGE_RESULT, sendIndex);

	if (m_pUserData->m_bLevel < 10)
		SetByte(sendBuffer, 2, sendIndex);
	else if ((m_pUserData->m_sClass % 100) > 4)
		SetByte(sendBuffer, 3, sendIndex);
	else
		SetByte(sendBuffer, 1, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::AllSkillPointChange()
{
	// 돈을 먼저 깍고.. 만약,, 돈이 부족하면.. 에러...
	int sendIndex = 0, skill_point = 0, money = 0, i = 0, j = 0, temp_value = 0;
	uint8_t type = 0; // 0:돈이 부족, 1:성공, 2:초기화할 스킬이 없을때..
	char sendBuffer[128] {};

	temp_value = static_cast<int>(pow((m_pUserData->m_bLevel * 2), 3.4));
	temp_value = (temp_value / 100) * 100;

	if (m_pUserData->m_bLevel < 30)
		temp_value = static_cast<int>(temp_value * 0.4);
#if 0
	else if (m_pUserData->m_bLevel >= 30
		&& m_pUserData->m_bLevel < 60)
		temp_value = temp_value * 1;
#endif
	else if (m_pUserData->m_bLevel >= 60 && m_pUserData->m_bLevel <= 90)
		temp_value = static_cast<int>(temp_value * 1.5);

	// 스킬은 한번 더
	temp_value = static_cast<int>(temp_value * 1.5);

	// 할인시점이고 승리국이라면
	if (m_pMain->m_sDiscount == 1 && m_pMain->m_byOldVictory == m_pUserData->m_bNation)
	{
		// old_money = temp_value;
		temp_value = static_cast<int>(temp_value * 0.5);
		//TRACE(_T("^^ AllSkillPointChange - Discount ,, money=%d->%d\n"), old_money, temp_value);
	}

	if (m_pMain->m_sDiscount == 2)
	{
		// old_money = temp_value;
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
	type                 = 1;

	SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
	SetByte(sendBuffer, ALL_SKILLPT_CHANGE, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	SetByte(sendBuffer, m_pUserData->m_bstrSkill[0], sendIndex);
	Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
	SetByte(sendBuffer, ALL_SKILLPT_CHANGE, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetDWORD(sendBuffer, temp_value, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::AllPointChange()
{
	// 돈을 먼저 깍고.. 만약,, 돈이 부족하면.. 에러...
	int sendIndex = 0, money = 0, temp_money = 0;
	uint8_t type = 0;
	char sendBuffer[128] {};
	int i = 0;

	if (m_pUserData->m_bLevel > 80)
		goto fail_return;

	temp_money = static_cast<int>(pow((m_pUserData->m_bLevel * 2), 3.4));
	temp_money = (temp_money / 100) * 100;
	if (m_pUserData->m_bLevel < 30)
		temp_money = static_cast<int>(temp_money * 0.4);
#if 0
	else if (m_pUserData->m_bLevel >= 30
		&& m_pUserData->m_bLevel < 60)
		temp_money = static_cast<int>(temp_money * 1);
#endif
	else if (m_pUserData->m_bLevel >= 60 && m_pUserData->m_bLevel <= 90)
		temp_money = static_cast<int>(temp_money * 1.5);

	// 할인시점이고 승리국이라면
	if (m_pMain->m_sDiscount == 1 && m_pMain->m_byOldVictory == m_pUserData->m_bNation)
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
		case KARUS_MIDDLE:
			if (m_pUserData->m_bStr == 65 && m_pUserData->m_bSta == 65 && m_pUserData->m_bDex == 60
				&& m_pUserData->m_bIntel == 50 && m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr   = 65;
			m_pUserData->m_bSta   = 65;
			m_pUserData->m_bDex   = 60;
			m_pUserData->m_bIntel = 50;
			m_pUserData->m_bCha   = 50;
			break;

		case KARUS_SMALL:
			if (m_pUserData->m_bStr == 50 && m_pUserData->m_bSta == 50 && m_pUserData->m_bDex == 70
				&& m_pUserData->m_bIntel == 70 && m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr   = 50;
			m_pUserData->m_bSta   = 50;
			m_pUserData->m_bDex   = 70;
			m_pUserData->m_bIntel = 70;
			m_pUserData->m_bCha   = 50;
			break;

		case KARUS_WOMAN:
			if (m_pUserData->m_bStr == 50 && m_pUserData->m_bSta == 60 && m_pUserData->m_bDex == 60
				&& m_pUserData->m_bIntel == 70 && m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr   = 50;
			m_pUserData->m_bSta   = 60;
			m_pUserData->m_bDex   = 60;
			m_pUserData->m_bIntel = 70;
			m_pUserData->m_bCha   = 50;
			break;

		case BABARIAN:
			if (m_pUserData->m_bStr == 65 && m_pUserData->m_bSta == 65 && m_pUserData->m_bDex == 60
				&& m_pUserData->m_bIntel == 50 && m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr   = 65;
			m_pUserData->m_bSta   = 65;
			m_pUserData->m_bDex   = 60;
			m_pUserData->m_bIntel = 50;
			m_pUserData->m_bCha   = 50;
			break;

		case ELMORAD_MAN:
			if (m_pUserData->m_bStr == 60 && m_pUserData->m_bSta == 60 && m_pUserData->m_bDex == 70
				&& m_pUserData->m_bIntel == 50 && m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr   = 60;
			m_pUserData->m_bSta   = 60;
			m_pUserData->m_bDex   = 70;
			m_pUserData->m_bIntel = 50;
			m_pUserData->m_bCha   = 50;
			break;

		case ELMORAD_WOMAN:
			if (m_pUserData->m_bStr == 50 && m_pUserData->m_bSta == 50 && m_pUserData->m_bDex == 70
				&& m_pUserData->m_bIntel == 70 && m_pUserData->m_bCha == 50)
			{
				type = 2;
				goto fail_return;
			}

			m_pUserData->m_bStr   = 50;
			m_pUserData->m_bSta   = 50;
			m_pUserData->m_bDex   = 70;
			m_pUserData->m_bIntel = 70;
			m_pUserData->m_bCha   = 50;
			break;

		default:
			spdlog::error(
				"User::AllPointChange: Unhandled race {} [accountName={} characterName={}]",
				m_pUserData->m_bRace, m_strAccountID, m_pUserData->m_id);
			type = 2;
			goto fail_return;
	}

	m_pUserData->m_bPoints = (m_pUserData->m_bLevel - 1) * 3 + 10;
	m_pUserData->m_iGold   = money;

	SetUserAbility();
	Send2AI_UserUpdateInfo();

	type = 1;
	SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
	SetByte(sendBuffer, ALL_POINT_CHANGE, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_bStr, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_bSta, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_bDex, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_bIntel, sendIndex);
	SetShort(sendBuffer, m_pUserData->m_bCha, sendIndex);
	SetShort(sendBuffer, m_iMaxHp, sendIndex);
	SetShort(sendBuffer, m_iMaxMp, sendIndex);
	SetShort(sendBuffer, m_sTotalHit, sendIndex);
	SetShort(sendBuffer, GetMaxWeightForClient(), sendIndex);
	SetShort(sendBuffer, m_pUserData->m_bPoints, sendIndex);
	Send(sendBuffer, sendIndex);

fail_return:
	SetByte(sendBuffer, WIZ_CLASS_CHANGE, sendIndex);
	SetByte(sendBuffer, ALL_POINT_CHANGE, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetDWORD(sendBuffer, temp_money, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::GoldChange(int tid, int gold)
{
	// Money only changes in Frontier zone and Battle zone!!!
	if (m_pUserData->m_bZone < 3)
		return;

	if (m_pUserData->m_bZone == ZONE_SNOW_BATTLE)
		return;

	int s_temp_gold = 0, t_temp_gold = 0, sendIndex = 0;
	uint8_t s_type = 0, t_type = 0; // 1 -> Get gold    2 -> Lose gold

	char sendBuffer[256] {};

	// Users ONLY!!!
	auto pTUser = m_pMain->GetUserPtr(tid);
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
			s_type                        = GOLD_CHANGE_GAIN;
			t_type                        = GOLD_CHANGE_LOSE;

			s_temp_gold                   = (pTUser->m_pUserData->m_iGold * 4) / 10;
			t_temp_gold                   = pTUser->m_pUserData->m_iGold / 2;

			m_pUserData->m_iGold         += s_temp_gold;
			pTUser->m_pUserData->m_iGold -= t_temp_gold;
		}
		// When the source is in a party.
		else
		{
			_PARTY_GROUP* pParty = m_pMain->m_PartyMap.GetData(m_sPartyIndex);
			if (pParty == nullptr)
				return;

			// s_type                     = GOLD_CHANGE_GAIN;
			t_type                        = GOLD_CHANGE_LOSE;

			s_temp_gold                   = (pTUser->m_pUserData->m_iGold * 4) / 10;
			t_temp_gold                   = pTUser->m_pUserData->m_iGold / 2;

			pTUser->m_pUserData->m_iGold -= t_temp_gold;

			SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex); // First the victim...
			SetByte(sendBuffer, t_type, sendIndex);
			SetDWORD(sendBuffer, t_temp_gold, sendIndex);
			SetDWORD(sendBuffer, pTUser->m_pUserData->m_iGold, sendIndex);
			pTUser->Send(sendBuffer, sendIndex);

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
				auto pUser = m_pMain->GetUserPtr(pParty->uid[i]);
				if (pUser == nullptr)
					continue;

				money = static_cast<int>(
					count * (float) (pUser->m_pUserData->m_bLevel / (float) levelsum));
				pUser->m_pUserData->m_iGold += money;

				// Now the party members...
				sendIndex                    = 0;
				memset(sendBuffer, 0, sizeof(sendBuffer));
				SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
				SetByte(sendBuffer, GOLD_CHANGE_GAIN, sendIndex);
				SetDWORD(sendBuffer, money, sendIndex);
				SetDWORD(sendBuffer, pUser->m_pUserData->m_iGold, sendIndex);
				pUser->Send(sendBuffer, sendIndex);
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
			s_type                        = GOLD_CHANGE_GAIN;
			t_type                        = GOLD_CHANGE_LOSE;

			s_temp_gold                   = gold;
			t_temp_gold                   = gold;

			m_pUserData->m_iGold         += s_temp_gold;
			pTUser->m_pUserData->m_iGold -= t_temp_gold;
		}
		// Source loses money.
		else
		{
			s_type                        = GOLD_CHANGE_LOSE;
			t_type                        = GOLD_CHANGE_GAIN;

			s_temp_gold                   = gold;
			t_temp_gold                   = gold;

			m_pUserData->m_iGold         -= s_temp_gold;
			pTUser->m_pUserData->m_iGold += t_temp_gold;
		}
	}

	SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
	SetByte(sendBuffer, s_type, sendIndex);
	SetDWORD(sendBuffer, s_temp_gold, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	Send(sendBuffer, sendIndex);

	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));

	// Now the target
	SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
	SetByte(sendBuffer, t_type, sendIndex);
	SetDWORD(sendBuffer, t_temp_gold, sendIndex);
	SetDWORD(sendBuffer, pTUser->m_pUserData->m_iGold, sendIndex);
	pTUser->Send(sendBuffer, sendIndex);
}

void CUser::SelectWarpList(char* pBuf)
{
	int index = 0, sendIndex = 0, warpid = 0;
	_WARP_INFO* pWarp       = nullptr;
	_ZONE_SERVERINFO* pInfo = nullptr;
	C3DMap *pCurrentMap = nullptr, *pTargetMap = nullptr;
	char sendBuffer[128] {};

	// 비러머글 순간이동 >.<
	uint8_t type = 2;
	//
	warpid       = GetShort(pBuf, index);

	pCurrentMap  = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pCurrentMap == nullptr)
		return;

	pWarp = pCurrentMap->m_WarpArray.GetData(warpid);
	if (pWarp == nullptr)
		return;

	// We cannot use warp gates when invading.
	if (m_pUserData->m_bNation != pWarp->sZone && pWarp->sZone <= ZONE_ELMORAD)
		return;

	// We cannot use warp gates belonging to another nation.
	if (pWarp->sNation != 0 && pWarp->sNation != m_pUserData->m_bNation)
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
	SetByte(sendBuffer, WIZ_WARP_LIST, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, 1, sendIndex);
	Send(sendBuffer, sendIndex);
*/

	if (m_pUserData->m_bZone == pWarp->sZone)
	{
		m_bZoneChangeSameZone = true;

		SetByte(sendBuffer, WIZ_WARP_LIST, sendIndex);
		SetByte(sendBuffer, type, sendIndex);
		SetByte(sendBuffer, 1, sendIndex);
		Send(sendBuffer, sendIndex);
	}
	//
	ZoneChange(pWarp->sZone, pWarp->fX + rx, pWarp->fZ + rz);

	/*	SetByte(sendBuffer, WIZ_VIRTUAL_SERVER, sendIndex);
	SetShort(sendBuffer, strlen(pInfo->strServerIP), sendIndex);
	SetString(sendBuffer, pInfo->strServerIP, strlen(pInfo->strServerIP), sendIndex);
	SetShort(sendBuffer, pInfo->sPort, sendIndex);
	Send(sendBuffer, sendIndex);*/
}

void CUser::ZoneConCurrentUsers(char* pBuf)
{
	int index = 0, sendIndex = 0, zone = 0, usercount = 0, nation = 0;
	char sendBuffer[128] {};

	zone            = GetShort(pBuf, index);
	nation          = GetByte(pBuf, index);

	int socketCount = m_pMain->GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pUser = m_pMain->GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bZone == zone && pUser->m_pUserData->m_bNation == nation)
			usercount++;
	}

	SetByte(sendBuffer, WIZ_ZONE_CONCURRENT, sendIndex);
	SetShort(sendBuffer, usercount, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::ServerChangeOk(char* pBuf)
{
	int index = 0, warpid = 0;
	_WARP_INFO* pWarp = nullptr;
	C3DMap* pMap      = nullptr;
	float rx = 0.0f, rz = 0.0f;
	/* 비러머글 순간이동 >.<
	int sendIndex = 0;
	uint8_t type = 2 ;
	char sendBuffer[128] {};
*/
	warpid = GetShort(pBuf, index);

	pMap   = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return;

	pWarp = pMap->m_WarpArray.GetData(warpid);
	if (pWarp == nullptr)
		return;

	rx = (float) myrand(0, (int) pWarp->fR * 2);
	if (rx < pWarp->fR)
		rx = -rx;
	rz = (float) myrand(0, (int) pWarp->fR * 2);
	if (rz < pWarp->fR)
		rz = -rz;

	/* 비러머글 순간이동 >.<
	SetByte(sendBuffer, WIZ_WARP_LIST, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, 1, sendIndex);
	Send(sendBuffer, sendIndex);
*/
	ZoneChange(pWarp->sZone, pWarp->fX + rx, pWarp->fZ + rz);
}

bool CUser::GetWarpList(int warp_group)
{
	int sendIndex  = 0; // 헤더와 카운트를 나중에 패킹...
	int temp_index = 0, count = 0;
	// 비러머글 마을 이름 표시 >.<
	uint8_t type = 1; // 1이면 일반, 2이면 워프 성공했는지 않했는지 ^^;
	char buff[8192] {}, sendBuffer[8192] {};

	C3DMap* pCurrentMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pCurrentMap == nullptr)
		return false;

	for (const auto& [_, pWarp] : pCurrentMap->m_WarpArray)
	{
		if (pWarp == nullptr)
			continue;

		if ((pWarp->sWarpID / 10) != warp_group)
			continue;

		SetShort(buff, pWarp->sWarpID, sendIndex);

		SetString2(buff, pWarp->strWarpName, sendIndex);
		SetString2(buff, pWarp->strAnnounce, sendIndex);
		SetShort(buff, pWarp->sZone, sendIndex);

		C3DMap* pTargetMap = m_pMain->GetMapByID(pWarp->sZone);
		if (pTargetMap != nullptr)
			SetShort(buff, pTargetMap->m_sMaxUser, sendIndex);
		else
			SetShort(buff, 0, sendIndex);

		SetDWORD(buff, pWarp->dwPay, sendIndex);
		SetShort(buff, (int16_t) (pWarp->fX * 10), sendIndex);
		SetShort(buff, (int16_t) (pWarp->fZ * 10), sendIndex);
		SetShort(buff, (int16_t) (pWarp->fY * 10), sendIndex);
		count++;
	}

	SetByte(sendBuffer, WIZ_WARP_LIST, temp_index);
	// 비러머글 마을 이름 표시 >.<
	SetByte(sendBuffer, type, temp_index);
	//
	SetShort(sendBuffer, count, temp_index);
	SetString(sendBuffer, buff, sendIndex, temp_index);
	Send(sendBuffer, temp_index);

	return true;
}

void CUser::InitType3()
{
	// This is for the duration spells Type 3.
	for (int i = 0; i < MAX_TYPE3_REPEAT; i++)
	{
		m_fHPStartTime[i] = 0.0;
		m_fHPLastTime[i]  = 0.0;
		m_bHPAmount[i]    = 0;
		m_bHPDuration[i]  = 0;
		m_bHPInterval[i]  = 5;
		m_sSourceID[i]    = -1;
	}

	m_bType3Flag = false;
}

bool CUser::BindObjectEvent(int16_t objectindex, int16_t /*nid*/)
{
	int sendIndex = 0, result = 0;
	char sendBuffer[128] {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	if (pEvent->sBelong != 0 && pEvent->sBelong != m_pUserData->m_bNation)
	{
		result = 0;
	}
	else
	{
		m_pUserData->m_sBind = pEvent->sIndex;
		result               = 1;
	}

	SetByte(sendBuffer, WIZ_OBJECT_EVENT, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(pEvent->sType), sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	Send(sendBuffer, sendIndex);

	return true;
}

bool CUser::GateObjectEvent(int16_t objectindex, int16_t nid)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return false;

	int sendIndex = 0, result = 0;
	char sendBuffer[128] {};

	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	CNpc* pNpc = m_pMain->m_NpcMap.GetData(nid);
	if (pNpc == nullptr)
		return false;

	if (pNpc->m_tNpcType == NPC_GATE || pNpc->m_tNpcType == NPC_PHOENIX_GATE
		|| pNpc->m_tNpcType == NPC_SPECIAL_GATE)
	{
		// NOTE: This can have other states; they should ideally be defined.
		pNpc->m_byGateOpen = pNpc->m_byGateOpen == 0 ? true : false;
		result             = 1;

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, AG_NPC_GATE_OPEN, sendIndex);
		SetShort(sendBuffer, nid, sendIndex);
		SetByte(sendBuffer, pNpc->m_byGateOpen, sendIndex);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
	}
	else
	{
		result = 0;
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_OBJECT_EVENT, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(pEvent->sType), sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, nid, sendIndex);
	SetByte(sendBuffer, pNpc->m_byGateOpen, sendIndex);
	m_pMain->Send_Region(
		sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	return true;
}

bool CUser::GateLeverObjectEvent(int16_t objectindex, int16_t nid)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return false;

	int sendIndex = 0, result = 0;
	char sendBuffer[128] {};

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
		if (pGateNpc->m_tNpcType == NPC_GATE || pGateNpc->m_tNpcType == NPC_PHOENIX_GATE
			|| pGateNpc->m_tNpcType == NPC_SPECIAL_GATE)
		{
			if (pNpc->m_byGroup != m_pUserData->m_bNation && pNpc->m_byGroup != 0)
			{
				if (pNpc->m_byGateOpen == 0)
					return false;
			}

			// NOTE: This can have other states; ideally they should be defined.
			pNpc->m_byGateOpen = pNpc->m_byGateOpen == 0 ? true : false;
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, AG_NPC_GATE_OPEN, sendIndex);
			SetShort(sendBuffer, nid, sendIndex);
			SetByte(sendBuffer, pNpc->m_byGateOpen, sendIndex);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);

			pGateNpc->m_byGateOpen = !pGateNpc->m_byGateOpen;
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, AG_NPC_GATE_OPEN, sendIndex);
			SetShort(sendBuffer, pGateNpc->m_sNid, sendIndex);
			SetByte(sendBuffer, pGateNpc->m_byGateOpen, sendIndex);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);

			result = 1;
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, WIZ_OBJECT_EVENT, sendIndex);
			SetByte(sendBuffer, static_cast<uint8_t>(pGateEvent->sType), sendIndex);
			SetByte(sendBuffer, result, sendIndex);
			SetShort(sendBuffer, pGateNpc->m_sNid, sendIndex);
			SetByte(sendBuffer, pGateNpc->m_byGateOpen, sendIndex);
			m_pMain->Send_Region(
				sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
		}
		else
		{
			result = 0;
		}
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_OBJECT_EVENT, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(pEvent->sType), sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, nid, sendIndex);
	SetByte(sendBuffer, pNpc->m_byGateOpen, sendIndex);
	m_pMain->Send_Region(
		sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	return true;
}

bool CUser::FlagObjectEvent(int16_t objectindex, int16_t nid)
{
	// 포인터 참조하면 안됨
	if (!m_pMain->m_bPointCheckFlag)
		return false;

	int sendIndex = 0, result = 0;
	char sendBuffer[128] {};

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
		if (pFlagNpc->m_tNpcType == NPC_GATE || pFlagNpc->m_tNpcType == NPC_PHOENIX_GATE
			|| pFlagNpc->m_tNpcType == NPC_SPECIAL_GATE)
		{
			if (m_pMain->m_bVictory > 0)
				return false;

			if (pNpc->m_byGateOpen == 0)
				return false;

			// if (pNpc->m_byGroup != 0
			//	&& pFlagNpc->m_byGroup != 0)
			//	goto fail_return;

			result             = 1;

			// pNpc->m_byGroup = m_pUserData->m_bNation;
			// pNpc->m_byGateOpen = !pNpc->m_byGateOpen;
			pNpc->m_byGateOpen = 0; // LEVER !!!

			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, AG_NPC_GATE_OPEN, sendIndex);
			SetShort(sendBuffer, nid, sendIndex);
			SetByte(sendBuffer, pNpc->m_byGateOpen, sendIndex);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex              = 0;

			// pFlagNpc->m_byGroup = m_pUserData->m_bNation;
			// pFlagNpc->m_byGateOpen = !pFlagNpc->m_byGateOpen;	// FLAG !!!
			pFlagNpc->m_byGateOpen = 0;

			SetByte(sendBuffer, AG_NPC_GATE_OPEN, sendIndex); // (Send to AI....)
			SetShort(sendBuffer, pFlagNpc->m_sNid, sendIndex);
			SetByte(sendBuffer, pFlagNpc->m_byGateOpen, sendIndex);
			m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;

			SetByte(sendBuffer, WIZ_OBJECT_EVENT, sendIndex); // (Send to Region...)
			SetByte(sendBuffer, static_cast<uint8_t>(pFlagEvent->sType), sendIndex);
			SetByte(sendBuffer, result, sendIndex);
			SetShort(sendBuffer, pFlagNpc->m_sNid, sendIndex);
			SetByte(sendBuffer, pFlagNpc->m_byGateOpen, sendIndex);
			m_pMain->Send_Region(
				sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

			// ADD FLAG SCORE !!!
			if (m_pUserData->m_bNation == NATION_KARUS)
				++m_pMain->m_bKarusFlag;
			else if (m_pUserData->m_bNation == NATION_ELMORAD)
				++m_pMain->m_bElmoradFlag;

			// Did one of the teams win?
			m_pMain->BattleZoneVictoryCheck();
		}
		else
		{
			result = 0;
		}
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_OBJECT_EVENT, sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(pEvent->sType), sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, nid, sendIndex);
	SetByte(sendBuffer, pNpc->m_byGateOpen, sendIndex);
	m_pMain->Send_Region(
		sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);

	return true;
}

bool CUser::WarpListObjectEvent(int16_t objectindex, int16_t /*nid*/)
{
	C3DMap* pMap = m_pMain->GetMapByIndex(m_iZoneIndex);
	if (pMap == nullptr)
		return false;

	_OBJECT_EVENT* pEvent = pMap->GetObjectEvent(objectindex);
	if (pEvent == nullptr)
		return false;

	// We cannot use warp gates belonging to another nation.
	if (pEvent->sBelong != 0 && pEvent->sBelong != m_pUserData->m_bNation)
		return false;

	// We cannot use warp gates when invading.
	if (m_pUserData->m_bNation != m_pUserData->m_bZone && m_pUserData->m_bZone <= ZONE_ELMORAD)
		return false;

	if (!GetWarpList(pEvent->sControlNpcID))
		return false;

	return true;
}

void CUser::ObjectEvent(char* pBuf)
{
	int index = 0, objectindex = 0, sendIndex = 0, nid = 0;
	uint8_t objectType    = 0;
	C3DMap* pMap          = nullptr;
	_OBJECT_EVENT* pEvent = nullptr;
	char sendBuffer[128] {};

	objectindex = GetShort(pBuf, index);
	nid         = GetShort(pBuf, index);

	pMap        = m_pMain->GetMapByIndex(m_iZoneIndex);
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

		default:
			spdlog::error("User::ObjectEvent: Unhandled object type {} [objectIndex={} npcId={} "
						  "accountName={} characterName={}]",
				pEvent->sType, objectindex, nid, m_strAccountID, m_pUserData->m_id);
#ifndef _DEBUG
			Close();
#endif
			goto fail_return;
	}
	return;

fail_return:
	SetByte(sendBuffer, WIZ_OBJECT_EVENT, sendIndex);
	SetByte(sendBuffer, objectType, sendIndex);
	SetByte(sendBuffer, 0, sendIndex);
	Send(sendBuffer, sendIndex);
}

#if 0 // outdated
void CUser::Friend(char* pBuf)
{
	int index = 0;
	uint8_t subcommand = GetByte(pBuf, index);

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
	int index = 0, destid = -1, sendIndex = 0;

	CUser* pUser = nullptr;
	char sendBuffer[256] {};

	destid = GetShort(pBuf, index);

	pUser = m_pMain->GetUserPtr(destid);
	if (pUser == nullptr)
		goto fail_return;

	if (pUser->m_sFriendUser != -1)
		goto fail_return;

	if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
		goto fail_return;

	m_sFriendUser = destid;
	pUser->m_sFriendUser = _socketId;

	SetByte(sendBuffer, WIZ_FRIEND_REPORT, sendIndex);
	SetByte(sendBuffer, FRIEND_REQUEST, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	pUser->Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_FRIEND_REPORT, sendIndex);
	SetByte(sendBuffer, FRIEND_CANCEL, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::FriendAccept(char* pBuf)
{
	int index = 0, destid = -1, sendIndex = 0;
	CUser* pUser = nullptr;
	char sendBuffer[256] {};

	uint8_t result = GetByte(pBuf, index);

	pUser = m_pMain->GetUserPtr(m_sFriendUser);
	if (pUser == nullptr)
	{
		m_sFriendUser = -1;
		return;
	}

	m_sFriendUser = -1;
	pUser->m_sFriendUser = -1;

	SetByte(sendBuffer, WIZ_FRIEND_REPORT, sendIndex);
	SetByte(sendBuffer, FRIEND_ACCEPT, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	pUser->Send(sendBuffer, sendIndex);
}
#endif

void CUser::Corpse()
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	SetByte(sendBuffer, WIZ_CORPSE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	m_pMain->Send_Region(
		sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, nullptr, false);
}

void CUser::PartyBBS(char* pBuf)
{
	int index          = 0;
	uint8_t subcommand = GetByte(pBuf, index);
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

		default:
			spdlog::error("User::PartyBBS: Unhandled opcode {:02X} [characterName={}]", subcommand,
				m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			break;
	}
}

void CUser::PartyBBSRegister(char* /*pBuf*/)
{
	int sendIndex = 0;
	char sendBuffer[256] {};
	int i = 0, counter = 0, socketCount = 0;

	// You are already in a party!
	if (m_sPartyIndex != -1)
		goto fail_return;

	// You are already on the BBS!
	if (m_bNeedParty == 2)
		goto fail_return;

	// Success! Now you officially need a party!!!
	m_bNeedParty = 2;

	// Send new 'Need Party Status' to region!!!
	SetByte(sendBuffer, 2, sendIndex);
	SetByte(sendBuffer, m_bNeedParty, sendIndex);
	StateChange(sendBuffer);

	// Now, let's find out which page the user is on.
	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));

	socketCount = m_pMain->GetUserSocketCount();
	for (i = 0; i < socketCount; i++)
	{
		auto pUser = m_pMain->GetUserPtrUnchecked(i);
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
			continue;

		if (pUser->m_bNeedParty == 1)
			continue;

		if (!((pUser->m_pUserData->m_bLevel <= (int) (m_pUserData->m_bLevel * 1.5)
				  && pUser->m_pUserData->m_bLevel >= (int) (m_pUserData->m_bLevel * 1.5))
				|| (pUser->m_pUserData->m_bLevel <= (m_pUserData->m_bLevel + 8)
					&& pUser->m_pUserData->m_bLevel >= ((int) (m_pUserData->m_bLevel) - 8))))
			continue;

		if (pUser->_socketId == _socketId)
			break;

		++counter;
	}

	SetShort(sendBuffer, counter / MAX_BBS_PAGE, sendIndex);
	PartyBBSNeeded(sendBuffer, PARTY_BBS_REGISTER);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_PARTY_BBS, sendIndex);
	SetByte(sendBuffer, PARTY_BBS_REGISTER, sendIndex);
	SetByte(sendBuffer, 0, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::PartyBBSDelete(char* /*pBuf*/)
{
	int sendIndex = 0; // Basic Initializations.
	char sendBuffer[256] {};

	// You don't need anymore
	if (m_bNeedParty == 1)
		goto fail_return;

	// Success! You no longer need a party !!!
	m_bNeedParty = 1;

	// Send new 'Need Party Status' to region!!!
	SetByte(sendBuffer, 2, sendIndex);
	SetByte(sendBuffer, m_bNeedParty, sendIndex);
	StateChange(sendBuffer);

	// Now, let's find out which page the user is on.
	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetShort(sendBuffer, 0, sendIndex);
	PartyBBSNeeded(sendBuffer, PARTY_BBS_DELETE);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_PARTY_BBS, sendIndex);
	SetByte(sendBuffer, PARTY_BBS_DELETE, sendIndex);
	SetByte(sendBuffer, 0, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::PartyBBSNeeded(char* pBuf, uint8_t type)
{
	int index = 0, sendIndex = 0, i = 0, j = 0, socketCount = m_pMain->GetUserSocketCount();
	int16_t page_index = 0, start_counter = 0, BBS_Counter = 0;
	uint8_t result = 0, valid_counter = 0;
	char sendBuffer[256] {};

	page_index    = GetShort(pBuf, index);
	start_counter = page_index * MAX_BBS_PAGE;

	if (start_counter < 0)
		goto fail_return;

	if (start_counter >= socketCount)
		goto fail_return;

	result = 1;

	SetByte(sendBuffer, WIZ_PARTY_BBS, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, result, sendIndex);

	for (i = 0; i < socketCount; i++)
	{
		auto pUser = m_pMain->GetUserPtrUnchecked(i);

		// Protection codes.
		if (pUser == nullptr)
			continue;

		if (pUser->m_pUserData->m_bNation != m_pUserData->m_bNation)
			continue;

		if (pUser->m_bNeedParty == 1)
			continue;

		if (!((pUser->m_pUserData->m_bLevel <= (int) (m_pUserData->m_bLevel * 1.5)
				  && pUser->m_pUserData->m_bLevel >= (int) (m_pUserData->m_bLevel * 1.5))
				|| (pUser->m_pUserData->m_bLevel <= (m_pUserData->m_bLevel + 8)
					&& pUser->m_pUserData->m_bLevel >= ((int) (m_pUserData->m_bLevel) - 8))))
			continue;

		BBS_Counter++;

		// Range check codes.
		if (i < start_counter)
			continue;

		if (valid_counter >= MAX_BBS_PAGE)
			continue;

		// Create packet.
		SetString2(sendBuffer, pUser->m_pUserData->m_id, sendIndex);
		SetByte(sendBuffer, pUser->m_pUserData->m_bLevel, sendIndex);
		SetShort(sendBuffer, pUser->m_pUserData->m_sClass, sendIndex);

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
			SetShort(sendBuffer, 0, sendIndex);
			SetByte(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
		}
	}

	SetShort(sendBuffer, page_index, sendIndex);
	SetShort(sendBuffer, BBS_Counter, sendIndex);
	Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_PARTY_BBS, sendIndex);
	SetByte(sendBuffer, PARTY_BBS_NEEDED, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::MarketBBS(char* pBuf)
{
	int index          = 0;
	uint8_t subcommand = GetByte(pBuf, index);

	MarketBBSBuyPostFilter(); // Get rid of empty slots.
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

		default:
			spdlog::error("User::MarketBBS: Unhandled opcode {:02X} [characterName={}]", subcommand,
				m_pUserData->m_id);

#ifndef _DEBUG
			Close();
#endif
			break;
	}
}

void CUser::MarketBBSRegister(char* pBuf)
{
	int index = 0, sendIndex = 0, i = 0, page_index = 0;
	int16_t title_len = 0, message_len = 0;
	uint8_t result = 0, sub_result = 1, buysell_index = 0;
	char sendBuffer[256] {};

	buysell_index = GetByte(pBuf, index); // Buy or sell?

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
				m_pMain->m_sBuyID[i] = _socketId;

				title_len            = GetShort(pBuf, index);
				GetString(m_pMain->m_strBuyTitle[i], pBuf, title_len, index);

				message_len = GetShort(pBuf, index);
				GetString(m_pMain->m_strBuyMessage[i], pBuf, message_len, index);

				m_pMain->m_iBuyPrice[i]     = GetDWORD(pBuf, index);
				m_pMain->m_fBuyStartTime[i] = TimeGet();

				result                      = 1;
				break;
			}
		}
		// Sell
		else if (buysell_index == MARKET_BBS_SELL)
		{
			if (m_pMain->m_sSellID[i] == -1)
			{
				m_pMain->m_sSellID[i] = _socketId;

				title_len             = GetShort(pBuf, index);
				GetString(m_pMain->m_strSellTitle[i], pBuf, title_len, index);

				message_len = GetShort(pBuf, index);
				GetString(m_pMain->m_strSellMessage[i], pBuf, message_len, index);

				m_pMain->m_iSellPrice[i]     = GetDWORD(pBuf, index);
				m_pMain->m_fSellStartTime[i] = TimeGet();

				result                       = 1;
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

	SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
	SetByte(sendBuffer, GOLD_CHANGE_LOSE, sendIndex);

	if (buysell_index == MARKET_BBS_BUY)
	{
		m_pUserData->m_iGold -= BUY_POST_PRICE;
		SetDWORD(sendBuffer, BUY_POST_PRICE, sendIndex);
	}
	else if (buysell_index == MARKET_BBS_SELL)
	{
		m_pUserData->m_iGold -= SELL_POST_PRICE;
		SetDWORD(sendBuffer, SELL_POST_PRICE, sendIndex);
	}

	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	Send(sendBuffer, sendIndex);

	page_index = i / MAX_BBS_PAGE;
	sendIndex  = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, buysell_index, sendIndex);
	SetShort(sendBuffer, page_index, sendIndex);
	MarketBBSReport(sendBuffer, MARKET_BBS_REGISTER);
	return;

fail_return:
	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, WIZ_MARKET_BBS, sendIndex);
	SetByte(sendBuffer, MARKET_BBS_REGISTER, sendIndex);
	SetByte(sendBuffer, buysell_index, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetByte(sendBuffer, sub_result, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::MarketBBSDelete(char* pBuf)
{
	int index = 0, sendIndex = 0;
	int16_t delete_id  = 0;
	uint8_t sub_result = 1, buysell_index = 0;
	char sendBuffer[256] {};

	buysell_index = GetByte(pBuf, index);  // Buy or sell?
	delete_id     = GetShort(pBuf, index); // Which message should I delete?

	if (delete_id < 0 || delete_id >= MAX_BBS_POST)
		goto fail_return;

	// Buy
	if (buysell_index == MARKET_BBS_BUY)
	{
		if (m_pMain->m_sBuyID[delete_id] != _socketId
			&& m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
			goto fail_return;

		m_pMain->MarketBBSBuyDelete(delete_id);
	}
	// Sell
	else if (buysell_index == MARKET_BBS_SELL)
	{
		if (m_pMain->m_sSellID[delete_id] != _socketId
			&& m_pUserData->m_bAuthority != AUTHORITY_MANAGER)
			goto fail_return;

		m_pMain->MarketBBSSellDelete(delete_id);
	}
	// Error
	else
	{
		goto fail_return;
	}

	SetShort(sendBuffer, buysell_index, sendIndex);
	SetShort(sendBuffer, 0, sendIndex);
	MarketBBSReport(sendBuffer, MARKET_BBS_DELETE);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_MARKET_BBS, sendIndex);
	SetByte(sendBuffer, MARKET_BBS_DELETE, sendIndex);
	SetByte(sendBuffer, buysell_index, sendIndex);
	SetByte(sendBuffer, 0, sendIndex);
	SetByte(sendBuffer, sub_result, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::MarketBBSReport(char* pBuf, uint8_t type)
{
	int index = 0, sendIndex = 0, i = 0, j = 0;
	int16_t page_index = 0, start_counter = 0, valid_counter = 0, BBS_Counter = 0, title_length = 0,
			message_length = 0;
	uint8_t result = 0, sub_result = 1, buysell_index = 0;
	char sendBuffer[8192] {};

	buysell_index = GetByte(pBuf, index);  // Buy or sell?
	page_index    = GetShort(pBuf, index); // Which message should I delete?

	start_counter = page_index * MAX_BBS_PAGE;

	if (type == MARKET_BBS_OPEN)
	{
		start_counter = 0;
		page_index    = 0;
	}

	if (start_counter < 0)
		goto fail_return;

	if (start_counter > MAX_BBS_POST)
		goto fail_return;

	result = 1;

	SetByte(sendBuffer, WIZ_MARKET_BBS, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetByte(sendBuffer, buysell_index, sendIndex);
	SetByte(sendBuffer, result, sendIndex);

	for (i = 0; i < MAX_BBS_POST; i++)
	{
		if (buysell_index == MARKET_BBS_BUY)
		{
			if (m_pMain->m_sBuyID[i] == -1)
				continue;

			auto pUser = m_pMain->GetUserPtr(m_pMain->m_sBuyID[i]);

			// Delete info!!!
			if (pUser == nullptr)
			{
				m_pMain->MarketBBSBuyDelete(i);
				continue;
			}

			// Increment number of total messages.
			++BBS_Counter;

			// Range check codes.
			if (i < start_counter)
				continue;

			if (valid_counter >= MAX_BBS_PAGE)
				continue;

			SetShort(sendBuffer, m_pMain->m_sBuyID[i], sendIndex);
			SetString2(sendBuffer, pUser->m_pUserData->m_id, sendIndex);

			title_length = static_cast<int16_t>(strlen(m_pMain->m_strBuyTitle[i]));
			if (title_length > MAX_BBS_TITLE)
				title_length = MAX_BBS_TITLE;

			SetString2(sendBuffer, m_pMain->m_strBuyTitle[i], title_length, sendIndex);

			message_length = static_cast<int16_t>(strlen(m_pMain->m_strBuyMessage[i]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(sendBuffer, m_pMain->m_strBuyMessage[i], message_length, sendIndex);

			SetDWORD(sendBuffer, m_pMain->m_iBuyPrice[i], sendIndex);
			SetShort(sendBuffer, i, sendIndex);

			++valid_counter;
		}
		else if (buysell_index == MARKET_BBS_SELL)
		{
			if (m_pMain->m_sSellID[i] == -1)
				continue;

			// Delete info!!!
			auto pUser = m_pMain->GetUserPtr(m_pMain->m_sSellID[i]);
			if (pUser == nullptr)
			{
				m_pMain->MarketBBSSellDelete(i);
				continue;
			}

			++BBS_Counter;

			// Range check codes.
			if (i < start_counter)
				continue;

			if (valid_counter >= MAX_BBS_PAGE)
				continue;

			SetShort(sendBuffer, m_pMain->m_sSellID[i], sendIndex);
			SetString2(sendBuffer, pUser->m_pUserData->m_id, sendIndex);

			title_length = static_cast<int16_t>(strlen(m_pMain->m_strSellTitle[i]));
			if (title_length > MAX_BBS_TITLE)
				title_length = MAX_BBS_TITLE;

			SetString2(sendBuffer, m_pMain->m_strSellTitle[i], title_length, sendIndex);

			message_length = static_cast<int16_t>(strlen(m_pMain->m_strSellMessage[i]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(sendBuffer, m_pMain->m_strSellMessage[i], message_length, sendIndex);

			SetDWORD(sendBuffer, m_pMain->m_iSellPrice[i], sendIndex);
			SetShort(sendBuffer, i, sendIndex);

			++valid_counter; // Increment number of messages on the requested page
		}
	}

	if (valid_counter == 0 && page_index > 0)
		goto fail_return1;

	// You still need to fill up slots.
	if (valid_counter < MAX_BBS_PAGE)
	{
		for (j = valid_counter; j < MAX_BBS_PAGE; j++)
		{
			SetShort(sendBuffer, -1, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, 0, sendIndex);
			SetDWORD(sendBuffer, 0, sendIndex);
			SetShort(sendBuffer, -1, sendIndex);

			++valid_counter;
		}
	}

	SetShort(sendBuffer, page_index, sendIndex);
	SetShort(sendBuffer, BBS_Counter, sendIndex);
	Send(sendBuffer, sendIndex);
	return;

fail_return:
	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetByte(sendBuffer, WIZ_MARKET_BBS, sendIndex);
	SetByte(sendBuffer, MARKET_BBS_REPORT, sendIndex);
	SetByte(sendBuffer, buysell_index, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetByte(sendBuffer, sub_result, sendIndex);
	Send(sendBuffer, sendIndex);
	return;

fail_return1:
	sendIndex = 0;
	memset(sendBuffer, 0, sizeof(sendBuffer));
	SetShort(sendBuffer, buysell_index, sendIndex);
	SetShort(sendBuffer, page_index - 1, sendIndex);
	MarketBBSReport(sendBuffer, type);
}

void CUser::MarketBBSRemotePurchase(char* pBuf)
{
	int sendIndex = 0, index = 0;
	int16_t message_index = -1;
	uint8_t result = 0, sub_result = 1, buysell_index = 0;
	char sendBuffer[256] {};
	std::shared_ptr<CUser> pUser;

	buysell_index = GetByte(pBuf, index);  // Buy or sell?
	message_index = GetShort(pBuf, index); // Which message should I retrieve?

	if (buysell_index != MARKET_BBS_BUY && buysell_index != MARKET_BBS_SELL)
		goto fail_return;

	if (buysell_index == MARKET_BBS_BUY)
	{
		if (m_pMain->m_sBuyID[message_index] == -1)
		{
			sub_result = 3;
			goto fail_return;
		}

		pUser = m_pMain->GetUserPtr(m_pMain->m_sBuyID[message_index]);

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

		pUser = m_pMain->GetUserPtr(m_pMain->m_sSellID[message_index]);

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

		SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
		SetByte(sendBuffer, GOLD_CHANGE_LOSE, sendIndex);
		SetDWORD(sendBuffer, REMOTE_PURCHASE_PRICE, sendIndex);
		SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
		Send(sendBuffer, sendIndex);

		result = 1;
	}
	// User does not have gold.
	else
	{
		sub_result = 2;
	}

fail_return:
	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_MARKET_BBS, sendIndex);
	SetByte(sendBuffer, MARKET_BBS_REMOTE_PURCHASE, sendIndex);
	SetByte(sendBuffer, buysell_index, sendIndex);
	SetByte(sendBuffer, result, sendIndex);

	// Only on errors!!!
	if (result == 0)
		SetByte(sendBuffer, sub_result, sendIndex);

	Send(sendBuffer, sendIndex);
}

void CUser::MarketBBSTimeCheck()
{
	int sendIndex      = 0;
	double currentTime = TimeGet();
	char sendBuffer[256] {};

	for (int i = 0; i < MAX_BBS_POST; i++)
	{
		// BUY!!!
		if (m_pMain->m_sBuyID[i] != -1)
		{
			auto pUser = m_pMain->GetUserPtr(m_pMain->m_sBuyID[i]);
			if (pUser == nullptr)
			{
				m_pMain->MarketBBSBuyDelete(i);
				continue;
			}

			if (m_pMain->m_fBuyStartTime[i] + BBS_CHECK_TIME < currentTime)
			{
				if (pUser->m_pUserData->m_iGold >= BUY_POST_PRICE)
				{
					pUser->m_pUserData->m_iGold -= BUY_POST_PRICE;
					m_pMain->m_fBuyStartTime[i]  = TimeGet();

					// Now the target
					memset(sendBuffer, 0, sizeof(sendBuffer));
					sendIndex = 0;
					SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
					SetByte(sendBuffer, GOLD_CHANGE_LOSE, sendIndex);
					SetDWORD(sendBuffer, BUY_POST_PRICE, sendIndex);
					SetDWORD(sendBuffer, pUser->m_pUserData->m_iGold, sendIndex);
					pUser->Send(sendBuffer, sendIndex);
				}
				else
				{
					m_pMain->MarketBBSBuyDelete(i);
				}
			}
		}

		// SELL!!!
		if (m_pMain->m_sSellID[i] != -1)
		{
			auto pUser = m_pMain->GetUserPtr(m_pMain->m_sSellID[i]);
			if (pUser == nullptr)
			{
				m_pMain->MarketBBSSellDelete(i);
				continue;
			}

			if (m_pMain->m_fSellStartTime[i] + BBS_CHECK_TIME < currentTime)
			{
				if (pUser->m_pUserData->m_iGold >= SELL_POST_PRICE)
				{
					pUser->m_pUserData->m_iGold  -= SELL_POST_PRICE;
					m_pMain->m_fSellStartTime[i]  = TimeGet();

					// Now the target
					memset(sendBuffer, 0, sizeof(sendBuffer));
					sendIndex = 0;
					SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
					SetByte(sendBuffer, GOLD_CHANGE_LOSE, sendIndex);
					SetDWORD(sendBuffer, SELL_POST_PRICE, sendIndex);
					SetDWORD(sendBuffer, pUser->m_pUserData->m_iGold, sendIndex);
					pUser->Send(sendBuffer, sendIndex);
				}
				else
				{
					m_pMain->MarketBBSSellDelete(i);
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
		if (m_pMain->m_sBuyID[i] == _socketId)
			m_pMain->MarketBBSBuyDelete(i);

		// SELL!!
		if (m_pMain->m_sSellID[i] == _socketId)
			m_pMain->MarketBBSSellDelete(i);
	}
}

void CUser::MarketBBSMessage(char* pBuf)
{
	int index = 0, sendIndex = 0;
	int16_t message_index = 0, message_length = 0;
	uint8_t result = 0, sub_result = 1, buysell_index = 0;
	char sendBuffer[256] {};

	buysell_index = GetByte(pBuf, index);  // Buy or sell?
	message_index = GetShort(pBuf, index); // Which message should I retrieve?

	if (buysell_index != MARKET_BBS_BUY && buysell_index != MARKET_BBS_SELL)
		goto fail_return;

	SetByte(sendBuffer, WIZ_MARKET_BBS, sendIndex);
	SetByte(sendBuffer, MARKET_BBS_MESSAGE, sendIndex);
	SetByte(sendBuffer, result, sendIndex);

	switch (buysell_index)
	{
		case MARKET_BBS_BUY:
			if (m_pMain->m_sBuyID[message_index] == -1)
				goto fail_return;

			message_length = static_cast<int16_t>(strlen(m_pMain->m_strBuyMessage[message_index]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(
				sendBuffer, m_pMain->m_strBuyMessage[message_index], message_length, sendIndex);
			break;

		case MARKET_BBS_SELL:
			if (m_pMain->m_sSellID[message_index] == -1)
				goto fail_return;

			message_length = static_cast<int16_t>(strlen(m_pMain->m_strSellMessage[message_index]));
			if (message_length > MAX_BBS_MESSAGE)
				message_length = MAX_BBS_MESSAGE;

			SetString2(
				sendBuffer, m_pMain->m_strSellMessage[message_index], message_length, sendIndex);
			break;

		default:
			goto fail_return;
	}

	Send(sendBuffer, sendIndex);
	return;

fail_return:
	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_MARKET_BBS, sendIndex);
	SetByte(sendBuffer, MARKET_BBS_MESSAGE, sendIndex);
	SetByte(sendBuffer, result, sendIndex);
	SetByte(sendBuffer, sub_result, sendIndex);
	Send(sendBuffer, sendIndex);
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
				strcpy_safe(m_pMain->m_strBuyTitle[i - empty_counter], m_pMain->m_strBuyTitle[i]);
				strcpy_safe(
					m_pMain->m_strBuyMessage[i - empty_counter], m_pMain->m_strBuyMessage[i]);
				m_pMain->m_iBuyPrice[i - empty_counter]     = m_pMain->m_iBuyPrice[i];
				m_pMain->m_fBuyStartTime[i - empty_counter] = m_pMain->m_fBuyStartTime[i];

				m_pMain->MarketBBSBuyDelete(i);
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
				strcpy_safe(m_pMain->m_strSellTitle[i - empty_counter], m_pMain->m_strSellTitle[i]);
				strcpy_safe(
					m_pMain->m_strSellMessage[i - empty_counter], m_pMain->m_strSellMessage[i]);
				m_pMain->m_iSellPrice[i - empty_counter]     = m_pMain->m_iSellPrice[i];
				m_pMain->m_fSellStartTime[i - empty_counter] = m_pMain->m_fSellStartTime[i];

				m_pMain->MarketBBSSellDelete(i);
			}
		}
	}
}

void CUser::BlinkTimeCheck(double currentTime)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	if (BLINK_TIME < (currentTime - m_fBlinkStartTime))
	{
		m_fBlinkStartTime = 0.0;

		m_bAbnormalType   = ABNORMAL_NORMAL;

		if (m_bRegeneType == REGENE_MAGIC)
			HpChange(m_iMaxHp / 2); // Refill HP.
		else
			HpChange(m_iMaxHp);

		m_bRegeneType = REGENE_NORMAL;

		SetByte(sendBuffer, 3, sendIndex);
		SetByte(sendBuffer, m_bAbnormalType, sendIndex);
		StateChange(sendBuffer);

		//TRACE(_T("?? BlinkTimeCheck : name=%hs(%d), type=%d ??\n"), m_pUserData->m_id, _socketId, m_bAbnormalType);
		//
		// AI_server로 regene정보 전송...
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, AG_USER_REGENE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetShort(sendBuffer, m_pUserData->m_sHp, sendIndex);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
		//
		//
		// To AI Server....
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, AG_USER_INOUT, sendIndex);
		SetByte(sendBuffer, USER_REGENE, sendIndex);
		SetShort(sendBuffer, _socketId, sendIndex);
		SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
		SetFloat(sendBuffer, m_pUserData->m_curx, sendIndex);
		SetFloat(sendBuffer, m_pUserData->m_curz, sendIndex);
		m_pMain->Send_AIServer(m_pUserData->m_bZone, sendBuffer, sendIndex);
		//
	}
}

void CUser::SetLogInInfoToDB(uint8_t bInit)
{
	int sendIndex = 0, retvalue = 0;
	_ZONE_SERVERINFO* pInfo = nullptr;
	char sendBuffer[256] {};

	pInfo = m_pMain->m_ServerArray.GetData(m_pMain->m_nServerNo);
	if (pInfo == nullptr)
	{
		spdlog::error(
			"User::SetLogInInfoToDB: invalid serverId={}, closing user", m_pMain->m_nServerNo);
		Close();
		return;
	}

	SetByte(sendBuffer, DB_LOGIN_INFO, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
	SetString2(sendBuffer, pInfo->strServerIP, sendIndex);
	SetShort(sendBuffer, pInfo->sPort, sendIndex);
	SetString2(sendBuffer, GetRemoteIP(), sendIndex);
	SetByte(sendBuffer, bInit, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
		spdlog::error("User::SetLogInInfoToDB: send error: {}", retvalue);
}

void CUser::KickOut(char* pBuf)
{
	int idlen = 0, index = 0, sendIndex = 0;
	char accountid[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE || idlen <= 0)
		return;

	GetString(accountid, pBuf, idlen, index);

	auto pUser = m_pMain->GetUserPtr(accountid, NameType::Account);
	if (pUser != nullptr)
	{
		pUser->UserDataSaveToAgent();
		pUser->Close();
	}
	else
	{
		SetByte(sendBuffer, WIZ_KICKOUT, sendIndex);
		SetString2(sendBuffer, accountid, idlen, sendIndex);
		m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
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

	int index              = 0;
	CNpc* pNpc             = nullptr;
	int nid                = 0;
	EVENT* pEvent          = nullptr;
	EVENT_DATA* pEventData = nullptr;
	int eventid            = -1;

	nid                    = GetShort(pBuf, index);
	pNpc                   = m_pMain->m_NpcMap.GetData(nid);

	// Better to check than not to check ;)
	if (pNpc == nullptr)
		return;

	m_sEventNid = nid; // 이건 나중에 내가 추가한 거였슴....

	// 이거 일단 주석 처리 절대 빼지마!!
	// if (pNpc->m_byEvent < 0)
	//	return;

	pEvent      = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);
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

		default:
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
	RunEvent(pEventData);
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
				if (CheckSkillPoint(
						pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1], pLE->m_LogicElseInt[2]))
					bExact = true;
				break;

			case LOGIC_CHECK_SKILL_TOTAL:
				if (CheckSkillTotal(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_STAT_TOTAL:
				if (CheckStatTotal(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
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
				if (CheckClass(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1],
						pLE->m_LogicElseInt[2], pLE->m_LogicElseInt[3], pLE->m_LogicElseInt[4],
						pLE->m_LogicElseInt[5]))
					bExact = true;
				break;

			case LOGIC_CHECK_NOCLASS:
				if (!CheckClass(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1],
						pLE->m_LogicElseInt[2], pLE->m_LogicElseInt[3], pLE->m_LogicElseInt[4],
						pLE->m_LogicElseInt[5]))
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
				if (CheckItemCount(
						pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1], pLE->m_LogicElseInt[2]))
					bExact = true;
				break;

			case LOGIC_CHECK_NOAH:
				if (m_pUserData->m_iGold >= pLE->m_LogicElseInt[0]
					&& m_pUserData->m_iGold <= pLE->m_LogicElseInt[1])
					bExact = true;
				break;

			case LOGIC_CHECK_NATION:
				if (m_pUserData->m_bNation == pLE->m_LogicElseInt[0])
					bExact = true;
				break;

			case LOGIC_CHECK_LOYALTY:
				if (m_pUserData->m_iLoyalty >= pLE->m_LogicElseInt[0]
					&& m_pUserData->m_iLoyalty <= pLE->m_LogicElseInt[1])
					bExact = true;
				break;

			case LOGIC_CHECK_CHIEF:
				if (m_pUserData->m_bFame == KNIGHTS_DUTY_CHIEF)
					bExact = true;
				break;

			case LOGIC_CHECK_NO_CHIEF:
				if (m_pUserData->m_bFame != KNIGHTS_DUTY_CHIEF)
					bExact = true;
				break;

			case LOGIC_CHECK_CLAN_GRADE:
				if (CheckClanGrade(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_MIDDLE_STATUE_CAPTURE:
				if (CheckMiddleStatueCapture())
				{
					// NOTE: officially this returns true, ending check processing immediately
					bExact = true;
				}
				break;

			case LOGIC_CHECK_MIDDLE_STATUE_NOCAPTURE:
				if (!CheckMiddleStatueCapture())
				{
					// NOTE: officially this returns true, ending check processing immediately
					bExact = true;
				}
				break;

			case LOGIC_CHECK_EMPTY_SLOT:
				if (GetNumberOfEmptySlots() >= pLE->m_LogicElseInt[0])
				{
					// NOTE: officially this returns true, ending check processing immediately
					bExact = true;
				}
				break;

			case LOGIC_CHECK_MONSTER_CHALLENGE_TIME:
				if (m_pMain->_monsterChallengeActiveType == pLE->m_LogicElseInt[0]
					&& m_pMain->_monsterChallengeState != 0)
				{
					// NOTE: officially this returns true, ending check processing immediately
					bExact = true;
				}
				break;

			case LOGIC_CHECK_EXIST_EVENT:
				if (CheckExistEvent(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_NOEXIST_EVENT:
				if (!CheckExistEvent(pLE->m_LogicElseInt[0], pLE->m_LogicElseInt[1]))
					bExact = true;
				break;

			case LOGIC_CHECK_ITEMCHANGE_NUM:
				if (m_byLastExchangeNum == pLE->m_LogicElseInt[0])
					bExact = true;
				break;

			case LOGIC_CHECK_KNIGHT:
				if (CheckKnight())
					bExact = true;
				break;

			case LOGIC_CHECK_PROMOTION_ELIGIBLE:
				if (CheckPromotionEligible())
					bExact = true;
				break;

			case LOGIC_CHECK_NO_CASTLE:
				if (m_pUserData->m_bKnights != m_pMain->m_KnightsSiegeWar._masterKnights
					|| m_pMain->m_KnightsSiegeWar._masterKnights == 0
					|| m_pUserData->m_bFame != KNIGHTS_DUTY_CHIEF)
				{
					// NOTE: officially this returns true, ending check processing immediately
					bExact = true;
				}
				break;

			case LOGIC_CHECK_CASTLE:
				if (m_pUserData->m_bKnights == m_pMain->m_KnightsSiegeWar._masterKnights
					&& m_pMain->m_KnightsSiegeWar._masterKnights > 0
					&& m_pUserData->m_bFame == KNIGHTS_DUTY_CHIEF)
				{
					// NOTE: officially this returns true, ending check processing immediately
					bExact = true;
				}
				break;

			case LOGIC_CHECK_MONSTER_CHALLENGE_USERCOUNT:
				if (m_pMain->_monsterChallengePlayerCount > pLE->m_LogicElseInt[0])
				{
					// NOTE: officially this returns true, ending check processing immediately
					bExact = true;
				}
				break;

			case LOGIC_CHECK_BEEF_ROAST_KARUS_VICTORY:
				if (m_pMain->_beefRoastVictoryType == BEEF_ROAST_VICTORY_KARUS)
					bExact = true;
				break;

			case LOGIC_CHECK_BEEF_ROAST_ELMORAD_VICTORY:
				if (m_pMain->_beefRoastVictoryType == BEEF_ROAST_VICTORY_ELMORAD)
					bExact = true;
				break;

			case LOGIC_CHECK_BEEF_ROAST_NO_VICTORY:
				if (m_pMain->_beefRoastVictoryType != BEEF_ROAST_VICTORY_KARUS
					&& m_pMain->_beefRoastVictoryType != BEEF_ROAST_VICTORY_ELMORAD)
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
				EVENT* pEvent = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);
				if (pEvent == nullptr)
					break;

				EVENT_DATA* pEventData = pEvent->m_arEvent.GetData(pExec->m_ExecInt[0]);
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

			// 비러머글 복권 >.<
			case EXEC_OPEN_EDITBOX:
				OpenEditBox(pExec->m_ExecInt[1], pExec->m_ExecInt[2]);
				break;

			case EXEC_GIVE_NOAH:
				GoldGain(pExec->m_ExecInt[0]);
				break;

			case EXEC_LOG_COUPON_ITEM:
				LogCoupon(pExec->m_ExecInt[0], pExec->m_ExecInt[1]);
				break;

			// 비러머글 엑셀 >.<
			case EXEC_SAVE_COM_EVENT:
				SaveComEvent(pExec->m_ExecInt[0]);
				break;

			case EXEC_ROB_NOAH:
				GoldLose(pExec->m_ExecInt[0]);
				break;

			case EXEC_ZONE_CHANGE:
				ZoneChange(pExec->m_ExecInt[0], static_cast<float>(pExec->m_ExecInt[1]),
					static_cast<float>(pExec->m_ExecInt[2]));
				break;

			case EXEC_RETURN:
				return false;

			default:
				spdlog::warn("User::RunEvent: unhandled opcode. opcode={:02X} zoneId={}",
					pExec->m_Exec, m_pUserData->m_bZone);
				break;
		}
	}

	return true;
}

void CUser::TestPacket()
{
	/* do nothing */
}

void CUser::ItemLogToAgent(const char* srcid, const char* tarid, int type, int64_t serial,
	int itemid, int count, int durability)
{
	int sendIndex = 0, retvalue = 0;
	char sendBuffer[1024] {};

	SetByte(sendBuffer, WIZ_ITEM_LOG, sendIndex);
	SetString2(sendBuffer, srcid, sendIndex);
	SetString2(sendBuffer, tarid, sendIndex);
	SetByte(sendBuffer, type, sendIndex);
	SetInt64(sendBuffer, serial, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex);
	SetShort(sendBuffer, count, sendIndex);
	SetShort(sendBuffer, durability, sendIndex);

	retvalue = m_pMain->m_ItemLoggerSendQ.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
		spdlog::error("User::ItemLogToAgent: item send error: {}", retvalue);
}

void CUser::SendItemWeight()
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	SetSlotItemValue();
	SetByte(sendBuffer, WIZ_WEIGHT_CHANGE, sendIndex);
	SetShort(sendBuffer, GetCurrentWeightForClient(), sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::GoldGain(int gold)
{
	int sendIndex      = 0;
	int64_t iTotalGold = 0;
	char sendBuffer[256] {};

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

	SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
	SetByte(sendBuffer, GOLD_CHANGE_GAIN, sendIndex);
	SetDWORD(sendBuffer, gold, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	Send(sendBuffer, sendIndex);
}

bool CUser::GoldLose(int gold)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

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

	m_pUserData->m_iGold -= gold; // Subtract gold.

	SetByte(sendBuffer, WIZ_GOLD_CHANGE, sendIndex);
	SetByte(sendBuffer, GOLD_CHANGE_LOSE, sendIndex);
	SetDWORD(sendBuffer, gold, sendIndex);
	SetDWORD(sendBuffer, m_pUserData->m_iGold, sendIndex);
	Send(sendBuffer, sendIndex);

	return true;
}

bool CUser::CheckSkillPoint(uint8_t skillnum, uint8_t min, uint8_t max) const
{
	if (skillnum < 5 || skillnum > 8)
		return false;

	if (m_pUserData->m_bstrSkill[skillnum] < min || m_pUserData->m_bstrSkill[skillnum] > max)
		return false;

	return true;
}

bool CUser::CheckSkillTotal(uint8_t min, uint8_t max) const
{
	uint8_t skillTotal = m_pUserData->m_bstrSkill[SKILLPT_TYPE_ORDER]
						 + m_pUserData->m_bstrSkill[SKILLPT_TYPE_PRO_1]
						 + m_pUserData->m_bstrSkill[SKILLPT_TYPE_PRO_2]
						 + m_pUserData->m_bstrSkill[SKILLPT_TYPE_PRO_3]
						 + m_pUserData->m_bstrSkill[SKILLPT_TYPE_PRO_4];

	if (skillTotal < min || skillTotal > max)
		return false;

	return true;
}

bool CUser::CheckStatTotal(uint8_t min, uint8_t max) const
{
	uint8_t statTotal = m_pUserData->m_bDex + m_pUserData->m_bIntel + m_pUserData->m_bCha
						+ m_pUserData->m_bPoints + m_pUserData->m_bStr + m_pUserData->m_bSta;

	if (statTotal < min || statTotal > max)
		return false;

	return true;
}

bool CUser::CheckWeight(int itemid, int16_t count) const
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

bool CUser::CheckExistItem(int itemid, int16_t count) const
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

bool CUser::RobItem(int itemid, int16_t count)
{
	int sendIndex = 0;
	char sendBuffer[256] {};

	// This checks if such an item exists.
	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(itemid);
	if (pTable == nullptr)
		return false;

	int i = SLOT_MAX;
	for (; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (m_pUserData->m_sItemArray[i].nNum != itemid)
			continue;

		// Remove item from inventory (Non-countable items)
		if (pTable->Countable == 0)
		{
			m_pUserData->m_sItemArray[i].nNum      = 0;
			m_pUserData->m_sItemArray[i].sCount    = 0;
			m_pUserData->m_sItemArray[i].sDuration = 0;
			break;
		}

		// Remove the number of items from the inventory (Countable Items)
		if (m_pUserData->m_sItemArray[i].sCount < count)
			return false;

		m_pUserData->m_sItemArray[i].sCount -= count;

		if (m_pUserData->m_sItemArray[i].sCount == 0)
		{
			m_pUserData->m_sItemArray[i].nNum      = 0;
			m_pUserData->m_sItemArray[i].sCount    = 0;
			m_pUserData->m_sItemArray[i].sDuration = 0;
		}

		break;
	}

	// No match
	if (i < 0 || i >= SLOT_MAX + HAVE_MAX)
		return false;

	SendItemWeight();                        // Change weight first :)
	SetByte(sendBuffer, WIZ_ITEM_COUNT_CHANGE, sendIndex);
	SetShort(sendBuffer, 1, sendIndex);      // The number of for-loops
	SetByte(sendBuffer, 1, sendIndex);
	SetByte(sendBuffer, i - SLOT_MAX, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex); // The ID of item.
	SetDWORD(sendBuffer, m_pUserData->m_sItemArray[i].sCount, sendIndex);
	Send(sendBuffer, sendIndex);
	return true;
}

bool CUser::GiveItem(int itemid, int16_t count)
{
	int pos = 255, sendIndex = 0;
	char sendBuffer[128] {};

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

	SendItemWeight();                        // Change weight first :)
	SetByte(sendBuffer, WIZ_ITEM_COUNT_CHANGE, sendIndex);
	SetShort(sendBuffer, 1, sendIndex);      // The number of for-loops
	SetByte(sendBuffer, 1, sendIndex);
	SetByte(sendBuffer, pos, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex); // The ID of item.
	SetDWORD(sendBuffer, m_pUserData->m_sItemArray[SLOT_MAX + pos].sCount, sendIndex);
	Send(sendBuffer, sendIndex);
	return true;
}

bool CUser::CheckClass(int16_t class1, int16_t class2, int16_t class3, int16_t class4,
	int16_t class5, int16_t class6) const
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

bool CUser::CheckPromotionEligible()
{
	CNpc* npc = m_pMain->m_NpcMap.GetData(m_sEventNid);
	if (npc == nullptr)
		return false;

	if (CheckClass(
			CLASS_KA_GUARDIAN, CLASS_KA_PENETRATOR, CLASS_KA_NECROMANCER, CLASS_KA_DARKPRIEST)
		|| CheckClass(CLASS_EL_PROTECTOR, CLASS_EL_ASSASIN, CLASS_EL_ENCHANTER, CLASS_EL_DRUID))
	{
		// Here we return that the user is already mastered
		switch (m_pUserData->m_sClass)
		{
			case CLASS_EL_PROTECTOR:
			case CLASS_KA_GUARDIAN:
				SendSay(-1, -1, 6006);
				break;

			case CLASS_EL_ASSASIN:
			case CLASS_KA_PENETRATOR:
				SendSay(-1, -1, 7006);
				break;

			case CLASS_EL_ENCHANTER:
			case CLASS_KA_NECROMANCER:
				SendSay(-1, -1, 8006);
				break;

			case CLASS_EL_DRUID:
			case CLASS_KA_DARKPRIEST:
				SendSay(-1, -1, 9006);
				break;

			default:
				break;
		}

		return false;
	}

	constexpr int MASTER_LVL = 60;

	// The following SendSay() calls tell the user that they're at the wrong NPC or they've not reached level 60.
	switch (npc->m_tNpcType)
	{
		case NPC_MASTER_WARRIOR:
			if ((m_pUserData->m_sClass != CLASS_KA_BERSERKER
					&& m_pUserData->m_sClass != CLASS_EL_BLADE)
				|| m_pUserData->m_bLevel < MASTER_LVL)
			{
				SendSay(-1, -1, 6001);
				return false;
			}
			return true;

		case NPC_MASTER_ROGUE:
			if ((m_pUserData->m_sClass != CLASS_KA_HUNTER
					&& m_pUserData->m_sClass != CLASS_EL_RANGER)
				|| m_pUserData->m_bLevel < MASTER_LVL)
			{
				SendSay(-1, -1, 7001);
				return false;
			}
			return true;

		case NPC_MASTER_MAGE:
			if ((m_pUserData->m_sClass != CLASS_KA_SORCERER
					&& m_pUserData->m_sClass != CLASS_EL_MAGE)
				|| m_pUserData->m_bLevel < MASTER_LVL)
			{
				SendSay(-1, -1, 8001);
				return false;
			}
			return true;

		case NPC_MASTER_PRIEST:
			if ((m_pUserData->m_sClass != CLASS_KA_SHAMAN
					&& m_pUserData->m_sClass != CLASS_EL_CLERIC)
				|| m_pUserData->m_bLevel < MASTER_LVL)
			{
				SendSay(-1, -1, 9001);
				return false;
			}
			return true;

		default:
			spdlog::error("User::CheckPromotionEligible: Unhandled NPC type {} [npcId={} "
						  "npcName={} accountName={} characterName={}]",
				npc->m_tNpcType, npc->m_sNid, npc->m_strName, m_strAccountID, m_pUserData->m_id);
			break;
	}

	return false;
}

// Receive menu reply from client.
void CUser::RecvSelectMsg(char* pBuf)
{
	int index = 0, menuEntryId = 0, selectedMenuIndex = 0;
	EVENT* pEvent          = nullptr;
	EVENT_DATA* pEventData = nullptr;

	selectedMenuIndex      = GetByte(pBuf, index);

	// 비러머글 퀘스트 >.<
	if (selectedMenuIndex < 0 || selectedMenuIndex >= MAX_MESSAGE_EVENT)
	{
		ResetSelectMsg();
		return;
	}

	// Get the event number that needs to be processed next.
	menuEntryId = m_iSelMsgEvent[selectedMenuIndex];

	pEvent      = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone);
	if (pEvent == nullptr)
	{
		spdlog::error("User::RecvSelectMsg: Missing event script [zoneId={} characterName={}]",
			m_pUserData->m_bZone, m_pUserData->m_id);
		ResetSelectMsg();
		return;
	}

	pEventData = pEvent->m_arEvent.GetData(menuEntryId);
	if (pEventData == nullptr)
	{
		spdlog::error(
			"User::RecvSelectMsg: Missing menu entry [zoneId={} menuEntryId={} characterName={}]",
			m_pUserData->m_bZone, menuEntryId, m_pUserData->m_id);
		ResetSelectMsg();
		return;
	}

	if (!CheckEventLogic(pEventData) || !RunEvent(pEventData))
		ResetSelectMsg();
}

void CUser::ResetSelectMsg()
{
	for (int i = 0; i < MAX_MESSAGE_EVENT; i++)
		m_iSelMsgEvent[i] = -1;
}

void CUser::SendNpcSay(const EXEC* pExec)
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	if (pExec == nullptr)
		return;

	SetByte(sendBuffer, WIZ_NPC_SAY, sendIndex);

	// It will be TEN for now!!!
	for (int i = 0; i < 10; i++)
		SetDWORD(sendBuffer, pExec->m_ExecInt[i], sendIndex);

	Send(sendBuffer, sendIndex);
}

void CUser::SendSay(int16_t eventIdUp, int16_t eventIdOk, int16_t message1, int16_t message2,
	int16_t message3, int16_t message4, int16_t message5, int16_t message6, int16_t message7,
	int16_t message8)
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	SetByte(sendBuffer, WIZ_NPC_SAY, sendIndex);
	SetDWORD(sendBuffer, eventIdUp, sendIndex);
	SetDWORD(sendBuffer, eventIdOk, sendIndex);
	SetDWORD(sendBuffer, message1, sendIndex);
	SetDWORD(sendBuffer, message2, sendIndex);
	SetDWORD(sendBuffer, message3, sendIndex);
	SetDWORD(sendBuffer, message4, sendIndex);
	SetDWORD(sendBuffer, message5, sendIndex);
	SetDWORD(sendBuffer, message6, sendIndex);
	SetDWORD(sendBuffer, message7, sendIndex);
	SetDWORD(sendBuffer, message8, sendIndex);

	Send(sendBuffer, sendIndex);
}

void CUser::SelectMsg(const EXEC* pExec)
{
	int sendIndex = 0;
	char sendBuffer[128] {};

	if (pExec == nullptr)
		return;

	SetByte(sendBuffer, WIZ_SELECT_MSG, sendIndex);
	SetShort(sendBuffer, m_sEventNid, sendIndex);
	//	SetByte(sendBuffer, (uint8_t) pExec->m_ExecInt[0], sendIndex);		// NPC 직업
	SetDWORD(sendBuffer, pExec->m_ExecInt[1], sendIndex); // 지문 번호

	// 비러머글 퀘스트 >.<
	for (int i = 0, chat = 2; i < MAX_MESSAGE_EVENT; i++, chat += 2)
		SetDWORD(sendBuffer, pExec->m_ExecInt[chat], sendIndex);

	Send(sendBuffer, sendIndex);

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

bool CUser::JobGroupCheck(int16_t jobgroupid) const
{
	// Check job groups
	if (jobgroupid < 100)
	{
		switch (jobgroupid)
		{
			case JOB_GROUP_WARRIOR:
				if (m_pUserData->m_sClass == CLASS_KA_WARRIOR
					|| m_pUserData->m_sClass == CLASS_KA_BERSERKER
					|| m_pUserData->m_sClass == CLASS_KA_GUARDIAN
					|| m_pUserData->m_sClass == CLASS_EL_WARRIOR
					|| m_pUserData->m_sClass == CLASS_EL_BLADE
					|| m_pUserData->m_sClass == CLASS_EL_PROTECTOR)
					return true;
				break;

			case JOB_GROUP_ROGUE:
				if (m_pUserData->m_sClass == CLASS_KA_ROGUE
					|| m_pUserData->m_sClass == CLASS_KA_HUNTER
					|| m_pUserData->m_sClass == CLASS_KA_PENETRATOR
					|| m_pUserData->m_sClass == CLASS_EL_ROGUE
					|| m_pUserData->m_sClass == CLASS_EL_RANGER
					|| m_pUserData->m_sClass == CLASS_EL_ASSASIN)
					return true;
				break;

			case JOB_GROUP_MAGE:
				if (m_pUserData->m_sClass == CLASS_KA_WIZARD
					|| m_pUserData->m_sClass == CLASS_KA_SORCERER
					|| m_pUserData->m_sClass == CLASS_KA_NECROMANCER
					|| m_pUserData->m_sClass == CLASS_EL_WIZARD
					|| m_pUserData->m_sClass == CLASS_EL_MAGE
					|| m_pUserData->m_sClass == CLASS_EL_ENCHANTER)
					return true;
				break;

			case JOB_GROUP_CLERIC:
				if (m_pUserData->m_sClass == CLASS_KA_PRIEST
					|| m_pUserData->m_sClass == CLASS_KA_SHAMAN
					|| m_pUserData->m_sClass == CLASS_KA_DARKPRIEST
					|| m_pUserData->m_sClass == CLASS_EL_PRIEST
					|| m_pUserData->m_sClass == CLASS_EL_CLERIC
					|| m_pUserData->m_sClass == CLASS_EL_DRUID)
					return true;
				break;

			case JOB_GROUP_ATTACK_WARRIOR:
				if (m_pUserData->m_sClass == CLASS_KA_BERSERKER
					|| m_pUserData->m_sClass == CLASS_EL_BLADE)
					return true;
				break;

			case JOB_GROUP_DEFENSE_WARRIOR:
				if (m_pUserData->m_sClass == CLASS_KA_GUARDIAN
					|| m_pUserData->m_sClass == CLASS_EL_PROTECTOR)
					return true;
				break;

			case JOB_GROUP_ARCHERER:
				if (m_pUserData->m_sClass == CLASS_KA_HUNTER
					|| m_pUserData->m_sClass == CLASS_EL_RANGER)
					return true;
				break;

			case JOB_GROUP_ASSASSIN:
				if (m_pUserData->m_sClass == CLASS_KA_PENETRATOR
					|| m_pUserData->m_sClass == CLASS_EL_ASSASIN)
					return true;
				break;

			case JOB_GROUP_ATTACK_MAGE:
				if (m_pUserData->m_sClass == CLASS_KA_SORCERER
					|| m_pUserData->m_sClass == CLASS_EL_MAGE)
					return true;
				break;

			case JOB_GROUP_PET_MAGE:
				if (m_pUserData->m_sClass == CLASS_KA_NECROMANCER
					|| m_pUserData->m_sClass == CLASS_EL_ENCHANTER)
					return true;
				break;

			case JOB_GROUP_HEAL_CLERIC:
				if (m_pUserData->m_sClass == CLASS_KA_SHAMAN
					|| m_pUserData->m_sClass == CLASS_EL_CLERIC)
					return true;
				break;

			case JOB_GROUP_CURSE_CLERIC:
				if (m_pUserData->m_sClass == CLASS_KA_DARKPRIEST
					|| m_pUserData->m_sClass == CLASS_EL_DRUID)
					return true;
				break;

			default:
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
	double currentTime = TimeGet();

	// Time interval has passed :)
	if (ZONE_TRAP_INTERVAL < (currentTime - m_fLastTrapAreaTime))
	{
		// Reduce target health point.
		HpChange(-ZONE_TRAP_DAMAGE);

		// Check if the target is dead.
		if (m_pUserData->m_sHp == 0)
		{
			// Target status is officially dead now.
			m_bResHpType = USER_DEAD;

			InitType3();         // Init Type 3.....
			InitType4();         // Init Type 4.....

			m_sWhoKilledMe = -1; // Who the hell killed me?
		}
	}

	// Update Last Trap Area time :)
	m_fLastTrapAreaTime = currentTime;
}

void CUser::KickOutZoneUser(bool home)
{
	int zoneindex = m_pMain->GetZoneIndex(m_pUserData->m_bNation);
	if (zoneindex < 0)
		return;

	C3DMap* pMap = m_pMain->GetMapByIndex(zoneindex);
	if (pMap == nullptr)
		return;

	if (home)
	{
		int regene_event = 0;

		// ZoneChange(pMap->m_nZoneNumber, pMap->m_fInitX, pMap->m_fInitZ);

		// 비러머글 버퍼
		int random       = myrand(0, 9000);
		if (random < 3000)
			regene_event = 0;
		else if (random < 6000)
			regene_event = 1;
		else if (random < 9001)
			regene_event = 2;

		_REGENE_EVENT* pRegene = pMap->GetRegeneEvent(regene_event);
		if (pRegene == nullptr)
		{
			spdlog::error("User::KickOutZoneUser: no regene event found [charId={} "
						  "regeneEventId={} zoneIndex={}]",
				m_pUserData->m_id, regene_event, zoneindex);
			KickOutZoneUser();
			return;
		}

		//TRACE(_T("KickOutZoneUser - user=%hs, regene_event=%d\n"), m_pUserData->m_id, regene_event);

		int delta_x = myrand(0, static_cast<int>(pRegene->fRegeneAreaX));
		int delta_z = myrand(0, static_cast<int>(pRegene->fRegeneAreaZ));

		float x     = pRegene->fRegenePosX + delta_x;
		float y     = pRegene->fRegenePosZ + delta_z;

		ZoneChange(pMap->m_nZoneNumber, x, y);
	}
	else
	{
		// Move user to native zone.
		if (m_pUserData->m_bNation == NATION_KARUS)
			ZoneChange(pMap->m_nZoneNumber, 1335, 83);
		else
			ZoneChange(pMap->m_nZoneNumber, 445, 1950);
	}
}

void CUser::EventMoneyItemGet(int /*itemid*/, int /*count*/)
{
}

void CUser::NativeZoneReturn()
{
	// Send user back home in case it was the battlezone.
	model::Home* pHomeInfo = m_pMain->m_HomeTableMap.GetData(m_pUserData->m_bNation);
	if (pHomeInfo == nullptr)
		return;

	m_pUserData->m_bZone = m_pUserData->m_bNation;

	if (m_pUserData->m_bNation == NATION_KARUS)
	{
		m_pUserData->m_curx = static_cast<float>(
			pHomeInfo->KarusZoneX + myrand(0, pHomeInfo->KarusZoneLX));
		m_pUserData->m_curz = static_cast<float>(
			pHomeInfo->KarusZoneZ + myrand(0, pHomeInfo->KarusZoneLZ));
	}
	else
	{
		m_pUserData->m_curx = static_cast<float>(
			pHomeInfo->ElmoZoneX + myrand(0, pHomeInfo->ElmoZoneLX));
		m_pUserData->m_curz = static_cast<float>(
			pHomeInfo->ElmoZoneZ + myrand(0, pHomeInfo->ElmoZoneLZ));
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
	int sendIndex = 0, retvalue = 0;
	char sendBuffer[256] {};

	SetByte(sendBuffer, DB_COUPON_EVENT, sendIndex);
	SetByte(sendBuffer, CHECK_COUPON_EVENT, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);
	SetDWORD(sendBuffer, event, sendIndex);
	//	비러머글 대사문 >.<
	SetDWORD(sendBuffer, message, sendIndex);
	//
	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
	{
		spdlog::error("User::OpenEditBox: coupon send error: {}", retvalue);
		return;
	}
}

bool CUser::CheckRandom(int16_t percent) const
{
	if (percent < 0 || percent > 1000)
		return false;

	int rand = myrand(0, 1000);
	return (percent > rand);
}

void CUser::RecvEditBox(char* pBuf)
{
	EVENT* pEvent          = nullptr;
	EVENT_DATA* pEventData = nullptr;

	int index = 0, selevent = -1;
	int16_t coupon_length = 0;

	coupon_length         = GetShort(pBuf, index); // Get length of coupon number.
	if (coupon_length < 0 || coupon_length > MAX_COUPON_ID_LENGTH)
		return;

	GetString(m_strCouponId, pBuf, coupon_length, index); // Get actual coupon number.

	selevent = m_iEditBoxEvent;

	pEvent   = m_pMain->m_EventMap.GetData(m_pUserData->m_bZone); // 여기서 부터 중요함 --;
	if (pEvent == nullptr)
	{
		ResetEditBox();
		return;
	}

	pEventData = pEvent->m_arEvent.GetData(selevent);
	if (pEventData == nullptr || !CheckEventLogic(pEventData))
	{
		ResetEditBox();
		return;
	}

	if (!RunEvent(pEventData))
		ResetEditBox();
}

void CUser::ResetEditBox()
{
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
	int sendIndex = 0, retvalue = 0;
	char sendBuffer[256] {};

	SetByte(sendBuffer, DB_COUPON_EVENT, sendIndex);
	SetByte(sendBuffer, UPDATE_COUPON_EVENT, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	SetString2(sendBuffer, m_strAccountID, sendIndex);
	SetString2(sendBuffer, m_pUserData->m_id, sendIndex);
	SetString2(sendBuffer, m_strCouponId, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex);
	SetDWORD(sendBuffer, count, sendIndex);

	retvalue = m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	if (retvalue >= SMQ_FULL)
		spdlog::error("User::LogCoupon: send error: {}", retvalue);
}

void CUser::CouponEvent(const char* pBuf)
{
	int index = 0, nEventNum = 0, nResult = 0, nMessageNum = 0;
	nResult     = GetByte(pBuf, index);
	nEventNum   = GetDWORD(pBuf, index);
	// 비러머글 대사 >.<
	nMessageNum = GetDWORD(pBuf, index);
	//

	if (nResult == 0)
		return;

	// 알아서 사용
	int sendIndex = 0;
	char sendBuffer[128] {};
	m_iEditBoxEvent = nEventNum; // What will the next event be when an answer is given?
	SetByte(sendBuffer, WIZ_EDIT_BOX, sendIndex);
	// 비러머글 대사 >.<
	SetDWORD(sendBuffer, nMessageNum, sendIndex);
	//
	Send(sendBuffer, sendIndex);
}

bool CUser::CheckItemCount(int itemid, int16_t min, int16_t max) const
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
		if (m_pUserData->m_sItemArray[i].sCount < min || m_pUserData->m_sItemArray[i].sCount > max)
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
	if (min == 0 || max == 0)
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

bool CUser::CheckKnight() const
{
	CKnights* pKnights = m_pMain->GetKnightsPtr(m_pUserData->m_bKnights);
	if (pKnights == nullptr)
		return false;

	return (pKnights->m_byFlag == KNIGHTS_TYPE);
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

void CUser::RecvDeleteChar(const char* pBuf)
{
	int nResult = 0, nLen = 0, index = 0, sendIndex = 0, char_index = 0, knightsId = 0;
	char charId[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};

	nResult    = GetByte(pBuf, index);
	char_index = GetByte(pBuf, index);
	knightsId  = GetShort(pBuf, index);
	nLen       = GetShort(pBuf, index);
	GetString(charId, pBuf, nLen, index);

	if (nResult == 1 && knightsId != 0)
	{
		m_pMain->m_KnightsManager.RemoveKnightsUser(knightsId, charId);
		spdlog::debug("User::RecvDeleteChar: removed [charId={} knightsId={}]", charId, knightsId);

		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, UDP_KNIGHTS_PROCESS, sendIndex);
		SetByte(sendBuffer, KNIGHTS_WITHDRAW, sendIndex);
		SetShort(sendBuffer, knightsId, sendIndex);
		SetShort(sendBuffer, nLen, sendIndex);
		SetString(sendBuffer, charId, nLen, sendIndex);

		if (m_pMain->m_nServerGroup == 0)
			m_pMain->Send_UDP_All(sendBuffer, sendIndex);
		else
			m_pMain->Send_UDP_All(sendBuffer, sendIndex, 1);
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_DEL_CHAR, sendIndex);
	SetByte(sendBuffer, nResult, sendIndex); // 성공시 국가 정보
	SetByte(sendBuffer, char_index, sendIndex);

	Send(sendBuffer, sendIndex);
}

void CUser::GetUserInfo(char* buff, int& buff_index)
{
	CKnights* pKnights = nullptr;

	SetString1(buff, m_pUserData->m_id, buff_index);
	SetByte(buff, m_pUserData->m_bNation, buff_index);
	SetShort(buff, m_pUserData->m_bKnights, buff_index);
	SetByte(buff, m_pUserData->m_bFame, buff_index);

	if (m_pUserData->m_bKnights != 0)
		pKnights = m_pMain->m_KnightsMap.GetData(m_pUserData->m_bKnights);

	if (pKnights != nullptr)
	{
		SetShort(buff, pKnights->m_sAllianceKnights, buff_index);
		SetString1(buff, pKnights->m_strName, buff_index);
		SetByte(buff, pKnights->m_byGrade, buff_index); // knights grade
		SetByte(buff, pKnights->m_byRanking, buff_index);
		SetShort(buff, pKnights->m_sMarkVersion, buff_index);
		SetShort(buff, pKnights->m_sCape, buff_index);
	}
	else
	{
		SetShort(buff, 0, buff_index);  // m_sAllianceKnights
		SetByte(buff, 0, buff_index);   // m_strName
		SetByte(buff, 0, buff_index);   // m_byGrade
		SetByte(buff, 0, buff_index);   // m_byRanking
		SetShort(buff, 0, buff_index);  // m_sMarkVerison
		SetShort(buff, -1, buff_index); // m_sCape
	}

	SetByte(buff, m_pUserData->m_bLevel, buff_index);
	SetByte(buff, m_pUserData->m_bRace, buff_index);
	SetShort(buff, m_pUserData->m_sClass, buff_index);
	SetShort(buff, (uint16_t) m_pUserData->m_curx * 10, buff_index);
	SetShort(buff, (uint16_t) m_pUserData->m_curz * 10, buff_index);
	SetShort(buff, (int16_t) m_pUserData->m_cury * 10, buff_index);
	SetByte(buff, m_pUserData->m_bFace, buff_index);
	SetByte(buff, m_pUserData->m_bHairColor, buff_index);
	SetByte(buff, m_bResHpType, buff_index);
	// 비러머글 수능...
	SetDWORD(buff, m_bAbnormalType, buff_index);
	//
	SetByte(buff, m_bNeedParty, buff_index);
	SetByte(buff, m_pUserData->m_bAuthority, buff_index);

	SetByte(buff, static_cast<uint8_t>(m_bIsPartyLeader), buff_index);
	SetByte(buff, m_byInvisibilityState, buff_index);
	SetShort(buff, m_sDirection, buff_index);
	SetByte(buff, static_cast<uint8_t>(m_bIsChicken), buff_index);
	SetByte(buff, m_pUserData->m_bRank, buff_index);
	SetByte(buff, m_byKnightsRank, buff_index);
	SetByte(buff, m_byPersonalRank, buff_index);

	for (const int slot : { BREAST, LEG, HEAD, GLOVE, FOOT, SHOULDER, RIGHTHAND, LEFTHAND })
	{
		SetDWORD(buff, m_pUserData->m_sItemArray[slot].nNum, buff_index);
		SetShort(buff, m_pUserData->m_sItemArray[slot].sDuration, buff_index);
		SetByte(buff, m_pUserData->m_sItemArray[slot].byFlag, buff_index);
	}
}

void CUser::GameStart(char* pBuf)
{
	int index = 0, sendIndex = 0;
	char sendBuffer[512] {};

	int opcode = GetByte(pBuf, index);

	// Started loading
	if (opcode == 1)
	{
		SendMyInfo(0);
		m_pMain->UserInOutForMe(this);
		m_pMain->NpcInOutForMe(this);
		SendNotice();
		SendTimeStatus();

		spdlog::debug(
			"User::GameStart: loading [charId={} socketId={}]", m_pUserData->m_id, _socketId);

		SetByte(sendBuffer, WIZ_GAMESTART, sendIndex);
		Send(sendBuffer, sendIndex);
	}
	// Finished loading
	else if (opcode == 2)
	{
		// NOTE: This behaviour is flipped as compared to official to give it a more meaningful name.
		// bool bRecastSavedMagic = true;

		_state = CONNECTION_STATE_GAMESTART;

		spdlog::debug(
			"User::GameStart: in game [charId={} socketId={}]", m_pUserData->m_id, _socketId);

		UserInOut(USER_REGENE);

		if (m_pUserData->m_bCity == 0 && m_pUserData->m_sHp <= 0)
			m_pUserData->m_bCity = 0xff;

		if (m_pUserData->m_bCity == 0 || m_pUserData->m_bCity == 0xFF)
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

		if (m_iLostExp > 0 || m_pUserData->m_bCity == 0xff)
		{
			HpChange(-m_iMaxHp);

			// NOTE: This behaviour is flipped as compared to official to give it a more meaningful name.
			// bRecastSavedMagic = false;
		}

		SendMyInfo(2);

		// TODO:
		// BlinkStart();

		SetUserAbility();

		// If there is a permanent chat available!!!
		if (m_pMain->m_bPermanentChatMode)
		{
			SetByte(sendBuffer, WIZ_CHAT, sendIndex);
			SetByte(sendBuffer, PERMANENT_CHAT, sendIndex);
			SetByte(sendBuffer, NATION_KARUS, sendIndex);
			SetShort(sendBuffer, -1, sendIndex); // sid
			SetByte(sendBuffer, 0, sendIndex);   // sender name length
			SetString2(sendBuffer, m_pMain->m_strPermanentChat, sendIndex);
			Send(sendBuffer, sendIndex);
		}

#if 0 // TODO
		if (bRecastSavedMagic)
			ItemMallMagicRecast(false);
#endif
	}
}

void CUser::RecvZoneChange(char* pBuf)
{
	int index      = 0;
	uint8_t opcode = GetByte(pBuf, index);
	if (opcode == ZONE_CHANGE_LOADING)
	{
		m_pMain->UserInOutForMe(this);
		m_pMain->NpcInOutForMe(this);

		char sendBuffer[128];
		int sendIndex = 0;
		SetByte(sendBuffer, WIZ_ZONE_CHANGE, sendIndex);
		SetByte(sendBuffer, ZONE_CHANGE_LOADED, sendIndex);
		Send(sendBuffer, sendIndex);
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

	bool bCanTradeWithOtherNation = false, bCanTalkToOtherNation = false;
	uint8_t byZoneAbilityType = ZONE_ABILITY_NEUTRAL;
	int16_t sTariff           = TARIFF_BASE;
	int sendIndex             = 0;
	char sendBuffer[128] {};

	SetByte(sendBuffer, WIZ_ZONEABILITY, sendIndex);
	SetByte(sendBuffer, ZONE_ABILITY_UPDATE, sendIndex);

	if (zone == ZONE_MORADON || ((zone / 10) == 5 && zone != ZONE_CAITHAROS_ARENA))
	{
		bCanTradeWithOtherNation = true;
		byZoneAbilityType        = ZONE_ABILITY_NEUTRAL;
		bCanTalkToOtherNation    = true;
	}
	else
	{
		switch (zone)
		{
			case ZONE_ARENA:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType        = ZONE_ABILITY_NEUTRAL;
				bCanTalkToOtherNation    = true;
				break;

			case ZONE_CAITHAROS_ARENA:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType        = ZONE_ABILITY_CAITHAROS_ARENA;
				bCanTalkToOtherNation    = true;
				break;

			case ZONE_DESPERATION_ABYSS:
			case ZONE_HELL_ABYSS:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType        = ZONE_ABILITY_PVP_NEUTRAL_NPCS;
				bCanTalkToOtherNation    = true;
				break;

			case ZONE_FRONTIER:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType        = ZONE_ABILITY_PVP;
				bCanTalkToOtherNation    = false;
				sTariff                  = TARIFF_BASE + 10;
				break;

			case ZONE_DELOS:
				bCanTradeWithOtherNation = true;
				byZoneAbilityType        = ZONE_ABILITY_SIEGE_DISABLED;
				bCanTalkToOtherNation    = true;
				break;

			default:
				bCanTradeWithOtherNation = false;
				byZoneAbilityType        = ZONE_ABILITY_PVP;
				bCanTalkToOtherNation    = false;
				break;
		}
	}

	SetByte(sendBuffer, bCanTradeWithOtherNation, sendIndex);
	SetByte(sendBuffer, byZoneAbilityType, sendIndex);
	SetByte(sendBuffer, bCanTalkToOtherNation, sendIndex);
	SetShort(sendBuffer, sTariff, sendIndex);
	Send(sendBuffer, sendIndex);
}

bool CUser::CheckMiddleStatueCapture() const
{
	switch (m_pUserData->m_bNation)
	{
		case NATION_KARUS:
			return (m_pMain->_elmoradInvasionMonumentLastCapturedNation[INVASION_MONUMENT_DODA]
					== m_pUserData->m_bNation);

		case NATION_ELMORAD:
			return (m_pMain->_karusInvasionMonumentLastCapturedNation[INVASION_MONUMENT_DODA]
					== m_pUserData->m_bNation);

		default:
			return false;
	}
}

} // namespace Ebenezer
