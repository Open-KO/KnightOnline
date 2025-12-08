#include "pch.h"
#include "UdpSocket.h"
#include "Define.h"
#include "EbenezerApp.h"
#include "Knights.h"
#include "User.h"
#include "db_resources.h"

#include <shared/packets.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

CUdpSocket::CUdpSocket(EbenezerApp* main)
	: _recvUdpThread(this),
	_socket(_io)	
{
	_main = main;
}

CUdpSocket::~CUdpSocket()
{
	_recvUdpThread.shutdown();
}

bool CUdpSocket::CreateSocket()
{
	asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), _UDP_PORT);
	asio::error_code ec;

	_socket.open(endpoint.protocol(), ec);
	if (ec)
	{
		spdlog::error("UdpSocket::CreateSocket: open() failed: {}", ec.message());
		return false;
	}

	// Set the buffer sizes
	_socket.set_option(asio::socket_base::receive_buffer_size(UDP_SOCKET_BUFFER_SIZE), ec);
	if (ec)
	{
		spdlog::error("UdpSocket::CreateSocket: failed to set receive buffer size: {}", ec.message());
		return false;
	}

	_socket.set_option(asio::socket_base::send_buffer_size(UDP_SOCKET_BUFFER_SIZE), ec);
	if (ec)
	{
		spdlog::error("UdpSocket::CreateSocket: failed to set send buffer size: {}", ec.message());
		return false;
	}

	_socket.bind(endpoint, ec);
	if (ec)
	{
		spdlog::error("UdpSocket::CreateSocket: failed to bind local address: {}", ec.message());
		_socket.close();
		return false;
	}

	_recvUdpThread.start();

	spdlog::debug("UdpSocket::CreateSocket: success");
	return true;
}

void CUdpSocket::AsyncReceive()
{
	_socket.async_receive_from(
		asio::buffer(_recvBuff),
		_sender,
		[this](asio::error_code ec, size_t transferredBytes)
		{
			if (!ec)
			{
				PacketProcess(static_cast<int>(transferredBytes));
			}
			else
			{
				spdlog::error("UdpSocket::AsyncReceive: error [err={} ip={}]",
					ec.message(), _sender.address().to_string());
			}

			AsyncReceive();
		});
}

int CUdpSocket::SendUDPPacket(char* strAddress, char* pBuf, int len)
{
	int s_size = 0, index = 0;
	uint8_t pTBuf[2048] = {};

	if (len > static_cast<int>(sizeof(pTBuf))
		|| len <= 0)
		return 0;

	pTBuf[index++] = (uint8_t) PACKET_START1;
	pTBuf[index++] = (uint8_t) PACKET_START2;
	memcpy(pTBuf + index, &len, 2);
	index += 2;
	memcpy(pTBuf + index, pBuf, len);
	index += len;
	pTBuf[index++] = (uint8_t) PACKET_END1;
	pTBuf[index++] = (uint8_t) PACKET_END2;

	asio::error_code ec;

	asio::ip::address ip = asio::ip::make_address(strAddress, ec);
	if (ec)
	{
		spdlog::error("UdpSocket::SendUDPPacket: invalid ip [err={} ip={}]",
			ec.message(), ip.to_string());
		return 0;
	}

	asio::ip::udp::endpoint endpoint(ip, _UDP_PORT);

	s_size = static_cast<int>(_socket.send_to(
		asio::buffer(pBuf, index),
		endpoint,
		0, // message flags
		ec));

	if (ec)
	{
		spdlog::error("UdpSocket::SendUDPPacket: send_to() failed [err={} ip={}]",
			ec.message(), ip.to_string());
		return 0;
	}

	return s_size;
}

bool CUdpSocket::PacketProcess(int len)
{
	uint8_t*	pTmp;
	bool		foundCore;
	MYSHORT		slen;
	int			length;

	if (len <= 0)
		return false;

	pTmp = new uint8_t[len + 1];

	memcpy(pTmp, _recvBuff, len);

	foundCore = false;

	int	sPos = 0, ePos = 0;

	for (int i = 0; i < len && !foundCore; i++)
	{
		if (i + 2 >= len) break;

		if (pTmp[i] == PACKET_START1
			&& pTmp[i + 1] == PACKET_START2)
		{
			sPos = i + 2;

			slen.b[0] = pTmp[sPos];
			slen.b[1] = pTmp[sPos + 1];

			length = slen.w;

			if (length <= 0)
				goto cancelRoutine;

			if (length > len)
				goto cancelRoutine;

			ePos = sPos + length + 2;

			if ((ePos + 2) > len)
				goto cancelRoutine;

			if (pTmp[ePos] != PACKET_END1
				|| pTmp[ePos + 1] != PACKET_END2)
				goto cancelRoutine;

			Parsing((char*) (pTmp + sPos + 2), length);
			foundCore = true;
			break;
		}
	}

cancelRoutine:
	delete[] pTmp;
	return foundCore;
}

void CUdpSocket::Parsing(char* pBuf, int /*len*/)
{
	uint8_t command;
	int index = 0;

	command = GetByte(pBuf, index);
	switch (command)
	{
		case STS_CHAT:
			ServerChat(pBuf + index);
			break;

		case UDP_BATTLE_EVENT_PACKET:
			RecvBattleEvent(pBuf + index);
			break;

		case UDP_KNIGHTS_PROCESS:
			ReceiveKnightsProcess(pBuf + index);
			break;

		case UDP_BATTLEZONE_CURRENT_USERS:
			RecvBattleZoneCurrentUsers(pBuf + index);
			break;
	}
}

void CUdpSocket::ServerChat(char* pBuf)
{
	int index = 0, chatlen = 0;
	char chatstr[1024] = {};

	chatlen = GetShort(pBuf, index);
	if (chatlen > 512
		|| chatlen <= 0)
		return;

	GetString(chatstr, pBuf, chatlen, index);

	spdlog::info("UdpSocket::ServerChat: {}", chatstr);
}

void CUdpSocket::RecvBattleEvent(char* pBuf)
{
	int index = 0, send_index = 0;
	int nType = 0, nResult = 0, nLen = 0, nKillKarus = 0, nElmoKill = 0;
	char strMaxUserName[MAX_ID_SIZE + 1] = {},
		send_buff[256] = {};

	nType = GetByte(pBuf, index);
	nResult = GetByte(pBuf, index);

	if (nType == BATTLE_EVENT_OPEN)
	{
	}
	else if (nType == BATTLE_MAP_EVENT_RESULT)
	{
		if (_main->m_byBattleOpen == NO_BATTLE)
		{
			spdlog::error("UdpSocket::RecvBattleEvent: No active battle [battleOpen={} type={}]",
				_main->m_byBattleOpen, nType);
			return;
		}

		if (nResult == KARUS)
		{
			//TRACE(_T("--> UDP RecvBattleEvent : 카루스 땅으로 넘어갈 수 있어\n"));
			_main->m_byKarusOpenFlag = 1;		// 카루스 땅으로 넘어갈 수 있어
		}
		else if (nResult == ELMORAD)
		{
			//TRACE(_T("--> UDP  RecvBattleEvent : 엘모 땅으로 넘어갈 수 있어\n"));
			_main->m_byElmoradOpenFlag = 1;	// 엘모 땅으로 넘어갈 수 있어
		}
	}
	else if (nType == BATTLE_EVENT_RESULT)
	{
		if (_main->m_byBattleOpen == NO_BATTLE)
		{
			spdlog::error("UdpSocket::RecvBattleEvent: No active battle [battleOpen={} type={}]",
				_main->m_byBattleOpen, nType);
			return;
		}
		if (nResult == KARUS)
		{
			//TRACE(_T("-->  UDP RecvBattleEvent : 카루스가 승리하였습니다.\n"));
		}
		else if (nResult == ELMORAD)
		{
			//TRACE(_T("-->  UDP RecvBattleEvent : 엘모라드가 승리하였습니다.\n"));
		}

		_main->m_bVictory = nResult;
		_main->m_byOldVictory = nResult;
		_main->m_byKarusOpenFlag = 0;		// 카루스 땅으로 넘어갈 수 없도록
		_main->m_byElmoradOpenFlag = 0;	// 엘모 땅으로 넘어갈 수 없도록
		_main->m_byBanishFlag = 1;
	}
	else if (nType == BATTLE_EVENT_MAX_USER)
	{
		nLen = GetByte(pBuf, index);

		if (nLen > 0
			&& nLen < MAX_ID_SIZE + 1)
		{
			GetString(strMaxUserName, pBuf, nLen, index);

			std::string chatstr;

			//TRACE(_T("-->  UDP RecvBattleEvent : 적국의 대장을 죽인 유저이름은? %hs, len=%d\n"), strMaxUserName, nResult);
			if (nResult == 1)
				chatstr = fmt::format_db_resource(IDS_KILL_CAPTAIN, strMaxUserName);
			else if (nResult == 2)
				chatstr = fmt::format_db_resource(IDS_KILL_GATEKEEPER, strMaxUserName);
			else if (nResult == 3)
				chatstr = fmt::format_db_resource(IDS_KILL_KARUS_GUARD1, strMaxUserName);
			else if (nResult == 4)
				chatstr = fmt::format_db_resource(IDS_KILL_KARUS_GUARD2, strMaxUserName);
			else if (nResult == 5)
				chatstr = fmt::format_db_resource(IDS_KILL_ELMO_GUARD1, strMaxUserName);
			else if (nResult == 6)
				chatstr = fmt::format_db_resource(IDS_KILL_ELMO_GUARD2, strMaxUserName);

			chatstr = fmt::format_db_resource(IDP_ANNOUNCEMENT, chatstr);
			SetByte(send_buff, WIZ_CHAT, send_index);
			SetByte(send_buff, WAR_SYSTEM_CHAT, send_index);
			SetByte(send_buff, 1, send_index);
			SetShort(send_buff, -1, send_index);
			SetByte(send_buff, 0, send_index);			// sender name length
			SetString2(send_buff, chatstr, send_index);
			_main->Send_All(send_buff, send_index);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_CHAT, send_index);
			SetByte(send_buff, PUBLIC_CHAT, send_index);
			SetByte(send_buff, 1, send_index);
			SetShort(send_buff, -1, send_index);
			SetByte(send_buff, 0, send_index);			// sender name length
			SetString2(send_buff, chatstr, send_index);
			_main->Send_All(send_buff, send_index);
		}
	}
	else if (nType == BATTLE_EVENT_KILL_USER)
	{
		if (nResult == 1)
		{
			nKillKarus = GetShort(pBuf, index);
			nElmoKill = GetShort(pBuf, index);
			_main->m_sKarusDead = _main->m_sKarusDead + nKillKarus;
			_main->m_sElmoradDead = _main->m_sElmoradDead + nElmoKill;

			//TRACE(_T("-->  UDP RecvBattleEvent type = 1 : 적국 유저 죽인수 : karus=%d->%d, elmo=%d->%d\n"), nKillKarus, _main->m_sKarusDead, nElmoKill, _main->m_sElmoradDead);

			SetByte(send_buff, UDP_BATTLE_EVENT_PACKET, send_index);
			SetByte(send_buff, BATTLE_EVENT_KILL_USER, send_index);
			SetByte(send_buff, 2, send_index);						// karus의 정보 전송
			SetShort(send_buff, _main->m_sKarusDead, send_index);
			SetShort(send_buff, _main->m_sElmoradDead, send_index);
			_main->Send_UDP_All(send_buff, send_index);
		}
		else if (nResult == 2)
		{
			nKillKarus = GetShort(pBuf, index);
			nElmoKill = GetShort(pBuf, index);

			//TRACE(_T("-->  UDP RecvBattleEvent type = 2 : 적국 유저 죽인수 : karus=%d->%d, elmo=%d->%d\n"), _main->m_sKarusDead, nKillKarus, _main->m_sElmoradDead, nElmoKill);

			_main->m_sKarusDead = nKillKarus;
			_main->m_sElmoradDead = nElmoKill;
		}
	}
}

void CUdpSocket::ReceiveKnightsProcess(char* pBuf)
{
	int index = 0, command = 0;

	command = GetByte(pBuf, index);
	//TRACE(_T("UDP - ReceiveKnightsProcess - command=%d\n"), command);

	switch (command)
	{
		case KNIGHTS_CREATE:
			RecvCreateKnights(pBuf + index);
			break;

		case KNIGHTS_JOIN:
		case KNIGHTS_WITHDRAW:
			RecvJoinKnights(pBuf + index, command);
			break;

		case KNIGHTS_REMOVE:
		case KNIGHTS_ADMIT:
		case KNIGHTS_REJECT:
		case KNIGHTS_CHIEF:
		case KNIGHTS_VICECHIEF:
		case KNIGHTS_OFFICER:
		case KNIGHTS_PUNISH:
			RecvModifyFame(pBuf + index, command);
			break;

		case KNIGHTS_DESTROY:
			RecvDestroyKnights(pBuf + index);
			break;
	}
}

void CUdpSocket::RecvCreateKnights(char* pBuf)
{
	int index = 0, namelen = 0, idlen = 0, knightsindex = 0, nation = 0, community = 0;
	char knightsname[MAX_ID_SIZE + 1] = {},
		chiefname[MAX_ID_SIZE + 1] = {};
	CKnights* pKnights = nullptr;

	community = GetByte(pBuf, index);
	knightsindex = GetShort(pBuf, index);
	nation = GetByte(pBuf, index);
	namelen = GetShort(pBuf, index);
	GetString(knightsname, pBuf, namelen, index);
	idlen = GetShort(pBuf, index);
	GetString(chiefname, pBuf, idlen, index);

	pKnights = new CKnights();
	pKnights->InitializeValue();

	pKnights->m_sIndex = knightsindex;
	pKnights->m_byFlag = community;
	pKnights->m_byNation = nation;
	strcpy_safe(pKnights->m_strName, knightsname);
	strcpy_safe(pKnights->m_strChief, chiefname);
	pKnights->m_sMembers = 1;
	pKnights->m_nMoney = 0;
	pKnights->m_nPoints = 0;
	pKnights->m_byGrade = 5;
	pKnights->m_byRanking = 0;

	_main->m_KnightsMap.PutData(pKnights->m_sIndex, pKnights);

	// 클랜정보에 추가
	_main->m_KnightsManager.AddKnightsUser(knightsindex, chiefname);

	//TRACE(_T("UDP - RecvCreateKnights - knname=%hs, name=%hs, index=%d\n"), knightsname, chiefname, knightsindex);
}

void CUdpSocket::RecvJoinKnights(char* pBuf, uint8_t command)
{
	int send_index = 0, knightsId = 0, index = 0, idlen = 0;
	char charId[MAX_ID_SIZE + 1] = {},
		send_buff[128] = {};
	std::string finalstr;

	knightsId = GetShort(pBuf, index);
	idlen = GetShort(pBuf, index);
	GetString(charId, pBuf, idlen, index);

	if (command == KNIGHTS_JOIN)
	{
		finalstr = fmt::format_db_resource(IDS_KNIGHTS_JOIN, charId);

		// 클랜정보에 추가
		_main->m_KnightsManager.AddKnightsUser(knightsId, charId);
		spdlog::debug("UdpSocket::RecvJoinKnights: charId={} joined knightsId={}",
			charId, knightsId);
		
	}
	// 탈퇴..
	else
	{
		// 클랜정보에 추가
		_main->m_KnightsManager.RemoveKnightsUser(knightsId, charId);

		finalstr = fmt::format_db_resource(IDS_KNIGHTS_WITHDRAW, charId);
		spdlog::debug("UdpSocket::RecvJoinKnights: charId={} left knightsId={}",
			charId, knightsId);
	}

	//TRACE(_T("UDP - RecvJoinKnights - command=%d, name=%hs, index=%d\n"), command, charid, knightsindex);

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, KNIGHTS_CHAT, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, finalstr, send_index);
	_main->Send_KnightsMember(knightsId, send_buff, send_index);
}

void CUdpSocket::RecvModifyFame(char* pBuf, uint8_t command)
{
	int index = 0, send_index = 0, knightsindex = 0, idlen = 0;
	char send_buff[128] = {},
		userid[MAX_ID_SIZE + 1] = {};
	std::string finalstr;
	CUser* pTUser = nullptr;

	knightsindex = GetShort(pBuf, index);
	idlen = GetShort(pBuf, index);
	GetString(userid, pBuf, idlen, index);

	pTUser = _main->GetUserPtr(userid, NameType::Character);

	switch (command)
	{
		case KNIGHTS_REMOVE:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bKnights = 0;
				pTUser->m_pUserData->m_bFame = 0;

				finalstr = fmt::format_db_resource(IDS_KNIGHTS_REMOVE, pTUser->m_pUserData->m_id);
				_main->m_KnightsManager.RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			else
			{
				_main->m_KnightsManager.RemoveKnightsUser(knightsindex, userid);
			}
			break;

		case KNIGHTS_ADMIT:
			if (pTUser != nullptr)
				pTUser->m_pUserData->m_bFame = KNIGHT;
			break;

		case KNIGHTS_REJECT:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bKnights = 0;
				pTUser->m_pUserData->m_bFame = 0;
				_main->m_KnightsManager.RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			break;

		case KNIGHTS_CHIEF + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = CHIEF;
				_main->m_KnightsManager.ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
				finalstr = fmt::format_db_resource(IDS_KNIGHTS_CHIEF, pTUser->m_pUserData->m_id);
			}
			break;

		case KNIGHTS_VICECHIEF + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = VICECHIEF;
				_main->m_KnightsManager.ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
				finalstr = fmt::format_db_resource(IDS_KNIGHTS_VICECHIEF, pTUser->m_pUserData->m_id);
			}
			break;

		case KNIGHTS_OFFICER + 0x10:
			if (pTUser != nullptr)
				pTUser->m_pUserData->m_bFame = OFFICER;
			break;

		case KNIGHTS_PUNISH + 0x10:
			if (pTUser != nullptr)
				pTUser->m_pUserData->m_bFame = PUNISH;
			break;
	}

	if (pTUser != nullptr)
	{
		//TRACE(_T("UDP - RecvModifyFame - command=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, pTUser->GetSocketID(), pTUser->m_pUserData->m_id, knightsindex, pTUser->m_pUserData->m_bFame);
		send_index = 0;
		memset(send_buff, 0, sizeof(send_buff));
		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_MODIFY_FAME, send_index);
		SetByte(send_buff, 0x01, send_index);
		if (command == KNIGHTS_REMOVE)
		{
			SetShort(send_buff, pTUser->GetSocketID(), send_index);
			SetShort(send_buff, pTUser->m_pUserData->m_bKnights, send_index);
			SetByte(send_buff, pTUser->m_pUserData->m_bFame, send_index);
			_main->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
		}
		else
		{
			SetShort(send_buff, pTUser->GetSocketID(), send_index);
			SetShort(send_buff, pTUser->m_pUserData->m_bKnights, send_index);
			SetByte(send_buff, pTUser->m_pUserData->m_bFame, send_index);
			pTUser->Send(send_buff, send_index);
		}

		if (command == KNIGHTS_REMOVE)
		{
			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_CHAT, send_index);
			SetByte(send_buff, KNIGHTS_CHAT, send_index);
			SetByte(send_buff, 1, send_index);
			SetShort(send_buff, -1, send_index);
			SetByte(send_buff, 0, send_index);			// sender name length
			SetString2(send_buff, finalstr, send_index);
			pTUser->Send(send_buff, send_index);
		}
	}

	memset(send_buff, 0x00, 128);		send_index = 0;
	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, KNIGHTS_CHAT, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, finalstr, send_index);
	_main->Send_KnightsMember(knightsindex, send_buff, send_index);
}

void CUdpSocket::RecvDestroyKnights(char* pBuf)
{
	int send_index = 0, knightsId = 0, index = 0, flag = 0;
	char send_buff[128] = {};
	std::string finalstr;
	CKnights* pKnights = nullptr;
	CUser* pTUser = nullptr;

	knightsId = GetShort(pBuf, index);

	pKnights = _main->m_KnightsMap.GetData(knightsId);
	if (pKnights == nullptr)
	{
		spdlog::error("UdpSocket::RecvDestroyKnights: knightsId={} not found",
			knightsId);
		return;
	}

	flag = pKnights->m_byFlag;

	// 클랜이나 기사단이 파괴된 메시지를 보내고 유저 데이타를 초기화
	if (flag == CLAN_TYPE)
		finalstr = fmt::format_db_resource(IDS_CLAN_DESTORY, pKnights->m_strName);
	else if (flag == KNIGHTS_TYPE)
		finalstr = fmt::format_db_resource(IDS_KNIGHTS_DESTROY, pKnights->m_strName);

	memset(send_buff, 0x00, 128);		send_index = 0;
	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, KNIGHTS_CHAT, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, finalstr, send_index);
	_main->Send_KnightsMember(knightsId, send_buff, send_index);

	int socketCount = _main->GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		pTUser = _main->GetUserPtrUnchecked(i);
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bKnights == knightsId)
		{
			pTUser->m_pUserData->m_bKnights = 0;
			pTUser->m_pUserData->m_bFame = 0;

			_main->m_KnightsManager.RemoveKnightsUser(knightsId, pTUser->m_pUserData->m_id);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
			SetByte(send_buff, KNIGHTS_MODIFY_FAME, send_index);
			SetByte(send_buff, 0x01, send_index);
			SetShort(send_buff, pTUser->GetSocketID(), send_index);
			SetShort(send_buff, pTUser->m_pUserData->m_bKnights, send_index);
			SetByte(send_buff, pTUser->m_pUserData->m_bFame, send_index);
			_main->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
			//pTUser->Send( send_buff, send_index );
		}
	}

	_main->m_KnightsMap.DeleteData(knightsId);
	//TRACE(_T("UDP - RecvDestoryKnights - index=%d\n"), knightsindex);
}

void CUdpSocket::RecvBattleZoneCurrentUsers(char* pBuf)
{
	int nKarusMan = 0, nElmoradMan = 0, index = 0;

	nKarusMan = GetShort(pBuf, index);
	nElmoradMan = GetShort(pBuf, index);

	_main->m_sKarusCount = nKarusMan;
	_main->m_sElmoradCount = nElmoradMan;
	//TRACE(_T("UDP - RecvBattleZoneCurrentUsers - karus=%d, elmorad=%d\n"), nKarusMan, nElmoradMan);
}
