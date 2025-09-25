// UdpSocket.cpp: implementation of the CUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UdpSocket.h"
#include "Define.h"
#include "EbenezerDlg.h"
#include "Knights.h"
#include "User.h"
#include "db_resources.h"

#include <shared/packets.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

DWORD WINAPI RecvUDPThread(LPVOID lp)
{
	CUdpSocket* pUdp = (CUdpSocket*) lp;
	int ret = 0, addrlen = sizeof(pUdp->m_ReplyAddress);

	while (1)
	{
		ret = recvfrom(pUdp->m_hUDPSocket, pUdp->m_pRecvBuf, 1024, 0, (SOCKADDR*) &pUdp->m_ReplyAddress, &addrlen);
		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			getpeername(pUdp->m_hUDPSocket, (SOCKADDR*) &pUdp->m_ReplyAddress, &addrlen);
			spdlog::error("UdpSocket::RecvUDPThread: winsock error [err={} ip={}]",
				err, inet_ntoa(pUdp->m_ReplyAddress.sin_addr));

			// 재전송 루틴...

			Sleep(10);
			continue;
		}

		if (ret)
		{
			if (!pUdp->PacketProcess(ret))
			{
				// broken packet...
			}
		}

		Sleep(10);
	}

	return 1;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUdpSocket::CUdpSocket(CEbenezerDlg* pMain)
{
	m_hUDPSocket = INVALID_SOCKET;
	memset(m_pRecvBuf, 0, sizeof(m_pRecvBuf));
	m_pMain = pMain;

	memset(&m_SocketAddress, 0, sizeof(m_SocketAddress));
	memset(&m_ReplyAddress, 0, sizeof(m_ReplyAddress));

	m_hUdpThread = nullptr;
}

CUdpSocket::~CUdpSocket()
{
}

bool CUdpSocket::CreateSocket()
{
	m_hUDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_hUDPSocket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		spdlog::error("UdpSocket::CreateSocket: winsock error [socketErr={}]",
			err);
		return false;
	}

	int sock_buf_size = 32768;
	setsockopt(m_hUDPSocket, SOL_SOCKET, SO_RCVBUF, (char*) &sock_buf_size, sizeof(sock_buf_size));
	setsockopt(m_hUDPSocket, SOL_SOCKET, SO_SNDBUF, (char*) &sock_buf_size, sizeof(sock_buf_size));

	int optlen = sizeof(sock_buf_size);
	int r = getsockopt(m_hUDPSocket, SOL_SOCKET, SO_RCVBUF, (char*) &sock_buf_size, &optlen);
	if (r == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		spdlog::error("UdpSocket::CreateSocket: failed to set buffer size [socketErr={}]",
			err);
		return false;
	}

	memset(&m_SocketAddress, 0, sizeof(m_SocketAddress));
	m_SocketAddress.sin_family = AF_INET;
	m_SocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_SocketAddress.sin_port = htons(_UDP_PORT);

	if (bind(m_hUDPSocket, (LPSOCKADDR) &m_SocketAddress, sizeof(m_SocketAddress)) == SOCKET_ERROR)
	{
		spdlog::error("UdpSocket::CreateSocket: failed to bind local address");
		closesocket(m_hUDPSocket);
		return false;
	}

	DWORD id;
	m_hUdpThread = ::CreateThread(nullptr, 0, RecvUDPThread, this, 0, &id);
	::SetThreadPriority(m_hUdpThread, THREAD_PRIORITY_ABOVE_NORMAL);

	spdlog::debug("UdpSocket::CreateSocket: success");
	return true;
}

int CUdpSocket::SendUDPPacket(char* strAddress, char* pBuf, int len)
{
	int s_size = 0, index = 0;

	uint8_t pTBuf[2048] = {};

	if (len > sizeof(pTBuf)
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

	m_SocketAddress.sin_addr.s_addr = inet_addr(strAddress);

	s_size = sendto(m_hUDPSocket, (char*) pTBuf, index, 0, (SOCKADDR*) &m_SocketAddress, sizeof(m_SocketAddress));

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

	memcpy(pTmp, m_pRecvBuf, len);

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

	delete[] pTmp;

	return foundCore;

cancelRoutine:
	delete[] pTmp;
	return foundCore;
}

void CUdpSocket::Parsing(char* pBuf, int len)
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
	
	m_pMain->AddOutputMessage(chatstr);
}

void CUdpSocket::RecvBattleEvent(char* pBuf)
{
	int index = 0, send_index = 0, udp_index = 0;
	int nType = 0, nResult = 0, nLen = 0, nKillKarus = 0, nElmoKill = 0;
	char strMaxUserName[MAX_ID_SIZE + 1] = {},
		send_buff[256] = {},
		udp_buff[256] = {};;

	nType = GetByte(pBuf, index);
	nResult = GetByte(pBuf, index);

	if (nType == BATTLE_EVENT_OPEN)
	{
	}
	else if (nType == BATTLE_MAP_EVENT_RESULT)
	{
		if (m_pMain->m_byBattleOpen == NO_BATTLE)
		{
			spdlog::error("UdpSocket::RecvBattleEvent: No active battle [battleOpen={} type={}]",
				m_pMain->m_byBattleOpen, nType);
			return;
		}

		if (nResult == KARUS)
		{
			//TRACE(_T("--> UDP RecvBattleEvent : 카루스 땅으로 넘어갈 수 있어\n"));
			m_pMain->m_byKarusOpenFlag = 1;		// 카루스 땅으로 넘어갈 수 있어
		}
		else if (nResult == ELMORAD)
		{
			//TRACE(_T("--> UDP  RecvBattleEvent : 엘모 땅으로 넘어갈 수 있어\n"));
			m_pMain->m_byElmoradOpenFlag = 1;	// 엘모 땅으로 넘어갈 수 있어
		}
	}
	else if (nType == BATTLE_EVENT_RESULT)
	{
		if (m_pMain->m_byBattleOpen == NO_BATTLE)
		{
			spdlog::error("UdpSocket::RecvBattleEvent: No active battle [battleOpen={} type={}]",
				m_pMain->m_byBattleOpen, nType);
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

		m_pMain->m_bVictory = nResult;
		m_pMain->m_byOldVictory = nResult;
		m_pMain->m_byKarusOpenFlag = 0;		// 카루스 땅으로 넘어갈 수 없도록
		m_pMain->m_byElmoradOpenFlag = 0;	// 엘모 땅으로 넘어갈 수 없도록
		m_pMain->m_byBanishFlag = 1;
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
			m_pMain->Send_All(send_buff, send_index);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_CHAT, send_index);
			SetByte(send_buff, PUBLIC_CHAT, send_index);
			SetByte(send_buff, 1, send_index);
			SetShort(send_buff, -1, send_index);
			SetByte(send_buff, 0, send_index);			// sender name length
			SetString2(send_buff, chatstr, send_index);
			m_pMain->Send_All(send_buff, send_index);
		}
	}
	else if (nType == BATTLE_EVENT_KILL_USER)
	{
		if (nResult == 1)
		{
			nKillKarus = GetShort(pBuf, index);
			nElmoKill = GetShort(pBuf, index);
			m_pMain->m_sKarusDead = m_pMain->m_sKarusDead + nKillKarus;
			m_pMain->m_sElmoradDead = m_pMain->m_sElmoradDead + nElmoKill;

			//TRACE(_T("-->  UDP RecvBattleEvent type = 1 : 적국 유저 죽인수 : karus=%d->%d, elmo=%d->%d\n"), nKillKarus, m_pMain->m_sKarusDead, nElmoKill, m_pMain->m_sElmoradDead);

			SetByte(send_buff, UDP_BATTLE_EVENT_PACKET, send_index);
			SetByte(send_buff, BATTLE_EVENT_KILL_USER, send_index);
			SetByte(send_buff, 2, send_index);						// karus의 정보 전송
			SetShort(send_buff, m_pMain->m_sKarusDead, send_index);
			SetShort(send_buff, m_pMain->m_sElmoradDead, send_index);
			m_pMain->Send_UDP_All(send_buff, send_index);
		}
		else if (nResult == 2)
		{
			nKillKarus = GetShort(pBuf, index);
			nElmoKill = GetShort(pBuf, index);

			//TRACE(_T("-->  UDP RecvBattleEvent type = 2 : 적국 유저 죽인수 : karus=%d->%d, elmo=%d->%d\n"), m_pMain->m_sKarusDead, nKillKarus, m_pMain->m_sElmoradDead, nElmoKill);

			m_pMain->m_sKarusDead = nKillKarus;
			m_pMain->m_sElmoradDead = nElmoKill;
		}
	}
}

void CUdpSocket::ReceiveKnightsProcess(char* pBuf)
{
	int index = 0, command = 0, pktsize = 0, count = 0;

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
	int index = 0, send_index = 0, namelen = 0, idlen = 0, knightsindex = 0, nation = 0, community = 0;
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
	strcpy(pKnights->m_strName, knightsname);
	strcpy(pKnights->m_strChief, chiefname);
	pKnights->m_sMembers = 1;
	pKnights->m_nMoney = 0;
	pKnights->m_nPoints = 0;
	pKnights->m_byGrade = 5;
	pKnights->m_byRanking = 0;

	m_pMain->m_KnightsMap.PutData(pKnights->m_sIndex, pKnights);

	// 클랜정보에 추가
	m_pMain->m_KnightsManager.AddKnightsUser(knightsindex, chiefname);

	//TRACE(_T("UDP - RecvCreateKnights - knname=%hs, name=%hs, index=%d\n"), knightsname, chiefname, knightsindex);
}

void CUdpSocket::RecvJoinKnights(char* pBuf, uint8_t command)
{
	int send_index = 0, knightsId = 0, index = 0, idlen = 0;
	char charId[MAX_ID_SIZE + 1] = {},
		send_buff[128] = {};
	std::string finalstr;
	CKnights* pKnights = nullptr;

	knightsId = GetShort(pBuf, index);
	idlen = GetShort(pBuf, index);
	GetString(charId, pBuf, idlen, index);

	pKnights = m_pMain->m_KnightsMap.GetData(knightsId);

	if (command == KNIGHTS_JOIN)
	{
		finalstr = fmt::format_db_resource(IDS_KNIGHTS_JOIN, charId);

		// 클랜정보에 추가
		m_pMain->m_KnightsManager.AddKnightsUser(knightsId, charId);
		spdlog::debug("UdpSocket::RecvJoinKnights: charId={} joined knightsId={}",
			charId, knightsId);
		
	}
	// 탈퇴..
	else
	{
		// 클랜정보에 추가
		m_pMain->m_KnightsManager.RemoveKnightsUser(knightsId, charId);

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
	m_pMain->Send_KnightsMember(knightsId, send_buff, send_index);
}

void CUdpSocket::RecvModifyFame(char* pBuf, uint8_t command)
{
	int index = 0, send_index = 0, knightsindex = 0, idlen = 0, vicechief = 0;
	char send_buff[128] = {},
		userid[MAX_ID_SIZE + 1] = {};
	std::string finalstr;
	CUser* pTUser = nullptr;
	CKnights* pKnights = nullptr;

	knightsindex = GetShort(pBuf, index);
	idlen = GetShort(pBuf, index);
	GetString(userid, pBuf, idlen, index);

	pTUser = m_pMain->GetUserPtr(userid, NameType::Character);
	pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);

	switch (command)
	{
		case KNIGHTS_REMOVE:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bKnights = 0;
				pTUser->m_pUserData->m_bFame = 0;

				finalstr = fmt::format_db_resource(IDS_KNIGHTS_REMOVE, pTUser->m_pUserData->m_id);
				m_pMain->m_KnightsManager.RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			else
			{
				m_pMain->m_KnightsManager.RemoveKnightsUser(knightsindex, userid);
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
				m_pMain->m_KnightsManager.RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			break;

		case KNIGHTS_CHIEF + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = CHIEF;
				m_pMain->m_KnightsManager.ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
				finalstr = fmt::format_db_resource(IDS_KNIGHTS_CHIEF, pTUser->m_pUserData->m_id);
			}
			break;

		case KNIGHTS_VICECHIEF + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = VICECHIEF;
				m_pMain->m_KnightsManager.ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
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
			m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
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
	m_pMain->Send_KnightsMember(knightsindex, send_buff, send_index);
}

void CUdpSocket::RecvDestroyKnights(char* pBuf)
{
	int send_index = 0, knightsId = 0, index = 0, flag = 0;
	char send_buff[128] = {};
	std::string finalstr;
	CKnights* pKnights = nullptr;
	CUser* pTUser = nullptr;

	knightsId = GetShort(pBuf, index);

	pKnights = m_pMain->m_KnightsMap.GetData(knightsId);
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
	m_pMain->Send_KnightsMember(knightsId, send_buff, send_index);

	for (int i = 0; i < MAX_USER; i++)
	{
		pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bKnights == knightsId)
		{
			pTUser->m_pUserData->m_bKnights = 0;
			pTUser->m_pUserData->m_bFame = 0;

			m_pMain->m_KnightsManager.RemoveKnightsUser(knightsId, pTUser->m_pUserData->m_id);

			memset(send_buff, 0, sizeof(send_buff));
			send_index = 0;
			SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
			SetByte(send_buff, KNIGHTS_MODIFY_FAME, send_index);
			SetByte(send_buff, 0x01, send_index);
			SetShort(send_buff, pTUser->GetSocketID(), send_index);
			SetShort(send_buff, pTUser->m_pUserData->m_bKnights, send_index);
			SetByte(send_buff, pTUser->m_pUserData->m_bFame, send_index);
			m_pMain->Send_Region(send_buff, send_index, pTUser->m_pUserData->m_bZone, pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
			//pTUser->Send( send_buff, send_index );
		}
	}

	m_pMain->m_KnightsMap.DeleteData(knightsId);
	//TRACE(_T("UDP - RecvDestoryKnights - index=%d\n"), knightsindex);
}

void CUdpSocket::RecvBattleZoneCurrentUsers(char* pBuf)
{
	int nKarusMan = 0, nElmoradMan = 0, index = 0;

	nKarusMan = GetShort(pBuf, index);
	nElmoradMan = GetShort(pBuf, index);

	m_pMain->m_sKarusCount = nKarusMan;
	m_pMain->m_sElmoradCount = nElmoradMan;
	//TRACE(_T("UDP - RecvBattleZoneCurrentUsers - karus=%d, elmorad=%d\n"), nKarusMan, nElmoradMan);
}
