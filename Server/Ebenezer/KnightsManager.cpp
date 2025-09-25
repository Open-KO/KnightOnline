// KnightsManager.cpp: implementation of the CKnightsManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KnightsManager.h"
#include "User.h"
#include "GameDefine.h"
#include "EbenezerDlg.h"
#include "db_resources.h"

#include <shared/packets.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKnightsManager::CKnightsManager()
{
	m_pMain = nullptr;
}

CKnightsManager::~CKnightsManager()
{
}

void CKnightsManager::PacketProcess(CUser* pUser, char* pBuf)
{
	if (pUser == nullptr)
		return;

	int index = 0;

	uint8_t command = GetByte(pBuf, index);
	switch (command)
	{
		case KNIGHTS_CREATE:
			CreateKnights(pUser, pBuf + index);
			break;

		case KNIGHTS_JOIN:
			JoinKnights(pUser, pBuf + index);
			break;

		case KNIGHTS_WITHDRAW:
			WithdrawKnights(pUser, pBuf + index);
			break;

		case KNIGHTS_REMOVE:
		case KNIGHTS_ADMIT:
		case KNIGHTS_REJECT:
		case KNIGHTS_CHIEF:
		case KNIGHTS_VICECHIEF:
		case KNIGHTS_OFFICER:
		case KNIGHTS_PUNISH:
			ModifyKnightsMember(pUser, pBuf + index, command);
			break;

		case KNIGHTS_DESTROY:
			DestroyKnights(pUser);
			break;

		case KNIGHTS_ALLLIST_REQ:
			AllKnightsList(pUser, pBuf + index);
			break;

		case KNIGHTS_MEMBER_REQ:
			AllKnightsMember(pUser, pBuf + index);
			break;

		case KNIGHTS_CURRENT_REQ:
			CurrentKnightsMember(pUser, pBuf + index);
			break;

		case KNIGHTS_STASH:
			break;

		case KNIGHTS_JOIN_REQ:
			JoinKnightsReq(pUser, pBuf + index);
			break;
	}
}

void CKnightsManager::CreateKnights(CUser* pUser, char* pBuf)
{
	int index = 0, send_index = 0, idlen = 0, knightindex = 0, ret_value = 3, week = 0;
	char idname[MAX_ID_SIZE + 1] = {};
	CTime time = CTime::GetCurrentTime();

	char send_buff[256] = {};

	if (pUser == nullptr)
		return;

	idlen = GetShort(pBuf, index);
	if (idlen > MAX_ID_SIZE || idlen < 0)
		goto fail_return;

	GetString(idname, pBuf, idlen, index);

	if (!IsAvailableName(idname))
		goto fail_return;

	// 기사단에 가입되어 있습니다
	if (pUser->m_pUserData->m_bKnights != 0)
	{
		ret_value = 5;
		goto fail_return;
	}

	// 기사단은 서버 1군에서만 만들 수 있도록...
	if (m_pMain->m_nServerGroup == 2)
	{
		ret_value = 8;
		goto fail_return;
	}

	// 요일 체크
	week = time.GetDayOfWeek();
	/*if (week != 1
		&& week != 6
		&& week != 7)
		ret_value = 7;
		goto fail_return;
	}*/

	// 20 Level 이상이 필요
	if (pUser->m_pUserData->m_bLevel < 20)
	{
		ret_value = 2;
		goto fail_return;
	}

	// 국가기여도 800 이상이 필요
	/*if (pUser->m_pUserData->m_iLoyalty < 800)
		goto fail_return;

	// 지휘스킬 10, 예절스킬 5 이상이 필요
	if (pUser->m_pUserData->m_bstrSkill[SKILLPT_TYPE_ORDER] < 10
		|| pUser->m_pUserData->m_bstrSkill[SKILLPT_TYPE_MANNER] < 5)
		goto fail_return;

	// 매력 120 이상이 필요
	if (pUser->m_pUserData->m_bCha < 120)
		goto fail_return;
	}*/

	// 5000000노아 이상이 필요
	if (pUser->m_pUserData->m_iGold < 500000)
	{
		ret_value = 4;
		goto fail_return;
	}

	knightindex = GetKnightsIndex(pUser->m_pUserData->m_bNation);

	// 기사단 창설에 실패
	if (knightindex == -1)
	{
		ret_value = 6;
		goto fail_return;
	}

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_CREATE + 0x10, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetByte(send_buff, CLAN_TYPE, send_index);
	SetShort(send_buff, knightindex, send_index);
	SetByte(send_buff, pUser->m_pUserData->m_bNation, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, idname, idlen, send_index);
	SetShort(send_buff, strlen(pUser->m_pUserData->m_id), send_index);
	SetString(send_buff, pUser->m_pUserData->m_id, strlen(pUser->m_pUserData->m_id), send_index);
	m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_CREATE, send_index);
	SetByte(send_buff, ret_value, send_index);
	//TRACE(_T("## CreateKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);

	pUser->Send(send_buff, send_index);
}

bool CKnightsManager::IsAvailableName(const char* strname) const
{
	for (const auto& [_, pKnights] : m_pMain->m_KnightsMap)
	{
		if (_strnicmp(pKnights->m_strName, strname, MAX_ID_SIZE) == 0)
			return false;
	}

	return true;
}

int CKnightsManager::GetKnightsIndex(int nation)
{
	//TRACE(_T("GetKnightsIndex = nation=%d\n"), nation);
	int knightindex = 0;
	// sungyong tw~
	//if (m_pMain->m_nServerNo == ELMORAD) knightindex = 15000;
	if (nation == ELMORAD)
		knightindex = 15000;
	// ~sungyong tw

	for (const auto& [_, pKnights] : m_pMain->m_KnightsMap)
	{
		if (knightindex < pKnights->m_sIndex)
		{
			if (nation == KARUS)
			{
				// sungyong,, 카루스와 전쟁존의 합침으로 인해서,,,
				if (pKnights->m_sIndex >= 15000)
					continue;
			}

			knightindex = pKnights->m_sIndex;
		}
	}

	knightindex++;
	if (nation == KARUS)
	{
		if (knightindex >= 15000
			|| knightindex < 0)
			return -1;
	}
	else if (nation == ELMORAD)
	{
		if (knightindex < 15000
			|| knightindex > 30000)
			return -1;
	}

	// 확인 사살..
	if (m_pMain->m_KnightsMap.GetData(knightindex))
		return -1;

	return knightindex;
}

void CKnightsManager::JoinKnights(CUser* pUser, char* pBuf)
{
	int knightsindex = 0, index = 0, send_index = 0, ret_value = 0, member_id = 0, community = 0;
	char send_buff[128] = {};
	CUser* pTUser = nullptr;
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

	// 전쟁존에서는 기사단 처리가 안됨
	if (pUser->m_pUserData->m_bZone > 2)
	{
		ret_value = 12;
		goto fail_return;
	}

	if (pUser->m_pUserData->m_bFame != CHIEF
		&& pUser->m_pUserData->m_bFame != VICECHIEF)
	{
		ret_value = 6;
		goto fail_return;
	}

	knightsindex = pUser->m_pUserData->m_bKnights;
	pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);
	if (pKnights == nullptr)
	{
		ret_value = 7;
		goto fail_return;
	}

	// 인원 체크
/*	if (pKnights->sMembers >= 24)
	{
		ret_value = 8;
		goto fail_return;
	}*/

	//community = GetByte(pBuf, index);
	member_id = GetShort(pBuf, index);
	if (member_id < 0
		|| member_id >= MAX_USER)
	{
		ret_value = 2;
		goto fail_return;
	}

	pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[member_id];
	if (pTUser == nullptr)
	{
		ret_value = 2;
		goto fail_return;
	}

	if (pTUser->m_bResHpType == USER_DEAD)
	{
		ret_value = 3;
		goto fail_return;
	}

	if (pTUser->m_pUserData->m_bNation != pUser->m_pUserData->m_bNation)
	{
		ret_value = 4;
		goto fail_return;
	}

	// 기사단에 가입되어 있습니다
	if (pTUser->m_pUserData->m_bKnights > 0)
	{
		ret_value = 5;
		goto fail_return;
	}

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_JOIN_REQ, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetShort(send_buff, knightsindex, send_index);
	SetShort(send_buff, strlen(pKnights->m_strName), send_index);
	SetString(send_buff, pKnights->m_strName, strlen(pKnights->m_strName), send_index);
	pTUser->Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_JOIN, send_index);
	SetByte(send_buff, ret_value, send_index);
	//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::JoinKnightsReq(CUser* pUser, char* pBuf)
{
	int knightsindex = 0, index = 0, send_index = 0, ret_value = 0, member_id = 0, community = 0, flag = 0, sid = -1;
	char send_buff[128] = {};
	CUser* pTUser = nullptr;
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

	flag = GetByte(pBuf, index);
	sid = GetShort(pBuf, index);
	if (sid < 0
		|| sid >= MAX_USER)
	{
		ret_value = 2;
		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_JOIN, send_index);
		SetByte(send_buff, ret_value, send_index);
		//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pTUser->GetSocketID(), pTUser->m_pUserData->m_id, ret_value);
		pTUser->Send(send_buff, send_index);
		return;
	}

	pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[sid];
	if (pTUser == nullptr)
	{
		ret_value = 2;
		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_JOIN, send_index);
		SetByte(send_buff, ret_value, send_index);
		//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
		pUser->Send(send_buff, send_index);
		return;
	}

	// 거절
	if (flag == 0)
	{
		ret_value = 11;
		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_JOIN, send_index);
		SetByte(send_buff, ret_value, send_index);
		//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pTUser->GetSocketID(), pTUser->m_pUserData->m_id, ret_value);
		pTUser->Send(send_buff, send_index);
		return;
	}

	knightsindex = GetShort(pBuf, index);
	pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);
	if (pKnights == nullptr)
	{
		ret_value = 7;
		goto fail_return;
	}

	// 인원 체크
	/*if (pKnights->sMembers >= 24)
	{
		ret_value = 8;
		goto fail_return;
	}*/

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_JOIN + 0x10, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetShort(send_buff, knightsindex, send_index);
	m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_JOIN, send_index);
	SetByte(send_buff, ret_value, send_index);
	//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::WithdrawKnights(CUser* pUser, char* pBuf)
{
	int index = 0, send_index = 0, ret_value = 0;
	char send_buff[128] = {};
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

	// 기사단에 가입되어 있지 않습니다
	if (pUser->m_pUserData->m_bKnights < 1
		|| pUser->m_pUserData->m_bKnights > 30000)
	{
		ret_value = 10;
		goto fail_return;
	}

	// 전쟁존에서는 기사단 처리가 안됨
	if (pUser->m_pUserData->m_bZone > 2)
	{
		ret_value = 12;
		goto fail_return;
	}

	// 단장이 탈퇴할 경우에는 클랜 파괴
	if (pUser->m_pUserData->m_bFame == CHIEF)
	{
		// 전쟁존에서는 클랜을 파괴할 수 없다
		if (pUser->m_pUserData->m_bZone > 2)
		{
			ret_value = 12;
			goto fail_return;
		}

		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_DESTROY + 0x10, send_index);
		SetShort(send_buff, pUser->GetSocketID(), send_index);
		SetShort(send_buff, pUser->m_pUserData->m_bKnights, send_index);
		m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
		return;
	}

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_WITHDRAW + 0x10, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetShort(send_buff, pUser->m_pUserData->m_bKnights, send_index);
	m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_WITHDRAW, send_index);
	SetByte(send_buff, ret_value, send_index);
	//TRACE(_T("## WithDrawKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::DestroyKnights(CUser* pUser)
{
	int index = 0, send_index = 0, ret_value = 0;
	char send_buff[128] = {};

	if (pUser == nullptr)
		return;

	if (pUser->m_pUserData->m_bFame != CHIEF)
		goto fail_return;

	if (pUser->m_pUserData->m_bZone > 2)
	{
		ret_value = 12;
		goto fail_return;
	}

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_DESTROY + 0x10, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetShort(send_buff, pUser->m_pUserData->m_bKnights, send_index);
	m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_DESTROY, send_index);
	SetByte(send_buff, ret_value, send_index);
	//TRACE(_T("## DestoryKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::ModifyKnightsMember(CUser* pUser, char* pBuf, uint8_t command)
{
	int index = 0, send_index = 0, idlen = 0, ret_value = 0, vicechief = 0, remove_flag = 0;
	char send_buff[128] = {};
	char userid[MAX_ID_SIZE + 1] = {};
	CUser* pTUser = nullptr;

	if (pUser == nullptr)
		return;

	idlen = GetShort(pBuf, index);

	// 잘못된 아이디
	if (idlen > MAX_ID_SIZE
		|| idlen <= 0)
	{
		ret_value = 2;
		goto fail_return;
	}

	GetString(userid, pBuf, idlen, index);

	// 전쟁존에서는 기사단 처리가 안됨
	if (pUser->m_pUserData->m_bZone > 2)
	{
		ret_value = 12;
		goto fail_return;
	}

	// 자신은 할 수 없습니다
	if (_strnicmp(userid, pUser->m_pUserData->m_id, MAX_ID_SIZE) == 0)
	{
		ret_value = 9;
		goto fail_return;
	}

	// 기사단, 멤버가입 및 멤버거절, 장교 이상이 할 수 있습니다
	if (command == KNIGHTS_ADMIT
		|| command == KNIGHTS_REJECT)
	{
		if (pUser->m_pUserData->m_bFame < OFFICER)
			goto fail_return;
	}
	// 징계, 부기사단장 이상이 할 수 있습니다
	else if (command == KNIGHTS_PUNISH)
	{
		if (pUser->m_pUserData->m_bFame < VICECHIEF)
			goto fail_return;
	}
	// 기사단장 만이 할 수 있습니다
	else
	{
		if (pUser->m_pUserData->m_bFame != CHIEF)
		{
			ret_value = 6;
			goto fail_return;
		}
	}

	pTUser = m_pMain->GetUserPtr(userid, NameType::Character);
	if (pTUser == nullptr)
	{
		// 게임상에 없는 유저 추방시 (100)
		if (command == KNIGHTS_REMOVE)
		{
			remove_flag = 0;
			SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
			SetByte(send_buff, command + 0x10, send_index);
			SetShort(send_buff, pUser->GetSocketID(), send_index);
			SetShort(send_buff, pUser->m_pUserData->m_bKnights, send_index);
			SetShort(send_buff, idlen, send_index);
			SetString(send_buff, userid, idlen, send_index);
			SetByte(send_buff, remove_flag, send_index);
			m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
			return;
		}
		else
		{
			ret_value = 2;
			goto fail_return;
		}
	}

	if (pUser->m_pUserData->m_bNation != pTUser->m_pUserData->m_bNation)
	{
		ret_value = 4;
		goto fail_return;
	}

	if (pUser->m_pUserData->m_bKnights != pTUser->m_pUserData->m_bKnights)
	{
		ret_value = 5;
		goto fail_return;
	}

	// 부단장이 3명이 됐는지를 판단 (클랜인 경우이다,,)
	// 부단장 임명
	if (command == KNIGHTS_VICECHIEF)
	{
		// 이미 부단장인 경우
		if (pTUser->m_pUserData->m_bFame == VICECHIEF)
		{
			ret_value = 8;
			goto fail_return;
		}

		CKnights* pKnights = m_pMain->m_KnightsMap.GetData(pUser->m_pUserData->m_bKnights);
		if (pKnights == nullptr)
		{
			ret_value = 7;
			goto fail_return;
		}

	/*	if( !strcmp( pKnights->strViceChief_1, "") )	vicechief = 1;
		else if( !strcmp( pKnights->strViceChief_2, "") )	vicechief = 2;
		else if( !strcmp( pKnights->strViceChief_3, "") )	vicechief = 3;
		else {
			ret_value = 8;
			goto fail_return;
		}	*/
	}

	remove_flag = 1;	// 게임상에 있는 유저 추방시 (1)
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, command + 0x10, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetShort(send_buff, pUser->m_pUserData->m_bKnights, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, userid, idlen, send_index);
	SetByte(send_buff, remove_flag, send_index);
	m_pMain->m_LoggerSendQueue.PutData(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, command, send_index);
	SetByte(send_buff, ret_value, send_index);
	//TRACE(_T("## ModifyKnights Fail - command=%d, nid=%d, name=%hs, error_code=%d ##\n"), command, pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::AllKnightsList(CUser* pUser, char* pBuf)
{
	int send_index = 0, buff_index = 0, count = 0, page = 0, index = 0, start = 0;
	char send_buff[4096] = {};
	char temp_buff[4096] = {};

	if (pUser == nullptr)
		return;

	page = GetShort(pBuf, index);
	start = page * 10;			// page : 0 ~

	for (const auto& [_, pKnights] : m_pMain->m_KnightsMap)
	{
		if (pKnights == nullptr)
			continue;

		// 기사단 리스트만 받자
		if (pKnights->m_byFlag != KNIGHTS_TYPE)
			continue;

		if (pKnights->m_byNation != pUser->m_pUserData->m_bNation)
			continue;

		if (count < start)
		{
			count++;
			continue;
		}

		SetShort(temp_buff, pKnights->m_sIndex, buff_index);
		SetShort(temp_buff, strlen(pKnights->m_strName), buff_index);
		SetString(temp_buff, pKnights->m_strName, strlen(pKnights->m_strName), buff_index);
		SetShort(temp_buff, pKnights->m_sMembers, buff_index);
		SetShort(temp_buff, strlen(pKnights->m_strChief), buff_index);
		SetString(temp_buff, pKnights->m_strChief, strlen(pKnights->m_strChief), buff_index);
		SetDWORD(temp_buff, pKnights->m_nPoints, buff_index);

		count++;
		if (count >= start + 10)
			break;
	}

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_ALLLIST_REQ, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetShort(send_buff, page, send_index);
	SetShort(send_buff, count - start, send_index);
	SetString(send_buff, temp_buff, buff_index, send_index);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::AllKnightsMember(CUser* pUser, char* pBuf)
{
	int index = 0, send_index = 0, page = 0, ret_value = 0, temp_index = 0, count = 0, onlineCount = 0, pktsize = 0;
	char send_buff[4096] = {};
	char temp_buff[4096] = {};
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

	// 기사단에 가입되어 있지 않습니다
	if (pUser->m_pUserData->m_bKnights <= 0)
	{
		ret_value = 2;
		goto fail_return;
	}

	pKnights = m_pMain->m_KnightsMap.GetData(pUser->m_pUserData->m_bKnights);
	if (pKnights == nullptr)
	{
		ret_value = 7;
		goto fail_return;
	}

	// 단장
/*	if (pUser->m_pUserData->m_bFame == CHIEF)
	{
		SetByte( send_buff, WIZ_KNIGHTS_PROCESS, send_index );
		SetByte( send_buff, KNIGHTS_MEMBER_REQ+0x10, send_index );
		SetShort( send_buff, pUser->GetSocketID(), send_index );
		SetShort( send_buff, pUser->m_pUserData->m_bKnights, send_index );
		//SetShort( send_buff, page, send_index );
		m_pMain->m_LoggerSendQueue.PutData( send_buff, send_index );
		return;
	}*/

	onlineCount = m_pMain->GetKnightsAllMembers(pUser->m_pUserData->m_bKnights, temp_buff, temp_index, 0);

	// 직접.. 게임서버에서 유저정보를 참조해서 불러오는 방식 (단장이 아닌 모든 사람)
	if (pUser->m_pUserData->m_bFame == CHIEF)
		count = m_pMain->GetKnightsAllMembers(pUser->m_pUserData->m_bKnights, temp_buff, temp_index, 1);
	else
		count = onlineCount;

	pktsize = temp_index + 4;
	if (count > MAX_CLAN)
		return;

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_MEMBER_REQ, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetShort(send_buff, pktsize, send_index);
	SetShort(send_buff, onlineCount, send_index);
	SetShort(send_buff, pKnights->m_sMembers, send_index);
	SetShort(send_buff, count, send_index);
	SetString(send_buff, temp_buff, temp_index, send_index);
	pUser->Send(send_buff, send_index);

/*	SetByte( send_buff, WIZ_KNIGHTS_PROCESS, send_index );
	SetByte( send_buff, KNIGHTS_MEMBER_REQ+0x10, send_index );
	SetShort( send_buff, pUser->GetSocketID(), send_index );
	SetShort( send_buff, pUser->m_pUserData->m_bKnights, send_index );
	//SetShort( send_buff, page, send_index );
	m_pMain->m_LoggerSendQueue.PutData( send_buff, send_index );	*/
	return;

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_MEMBER_REQ, send_index);
	SetByte(send_buff, ret_value, send_index);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::CurrentKnightsMember(CUser* pUser, char* pBuf)
{
	int index = 0, send_index = 0, buff_index = 0, count = 0, i = 0, page = 0, start = 0;
	char send_buff[128] = {};
	char temp_buff[4096] = {};
	CUser* pTUser = nullptr;
	CKnights* pKnights = nullptr;
	std::string errormsg = fmt::format_db_resource(IDP_KNIGHT_NOT_REGISTERED);

	if (pUser == nullptr)
		return;

	if (pUser->m_pUserData->m_bKnights <= 0)
		goto fail_return;

	pKnights = m_pMain->m_KnightsMap.GetData(pUser->m_pUserData->m_bKnights);
	if (pKnights == nullptr)
		goto fail_return;

	page = GetShort(pBuf, index);
	start = page * 10;			// page : 0 ~

	for (i = 0; i < MAX_USER; i++)
	{
		pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bKnights != pUser->m_pUserData->m_bKnights)
			continue;

		if (count < start)
		{
			count++;
			continue;
		}

		SetShort(temp_buff, strlen(pUser->m_pUserData->m_id), buff_index);
		SetString(temp_buff, pUser->m_pUserData->m_id, strlen(pUser->m_pUserData->m_id), buff_index);
		SetByte(temp_buff, pUser->m_pUserData->m_bFame, buff_index);
		SetByte(temp_buff, pUser->m_pUserData->m_bLevel, buff_index);
		SetShort(temp_buff, pUser->m_pUserData->m_sClass, buff_index);

		count++;
		if (count >= start + 10)
			break;
	}

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_CURRENT_REQ, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetShort(send_buff, strlen(pKnights->m_strChief), send_index);
	SetString(send_buff, pKnights->m_strChief, strlen(pKnights->m_strChief), send_index);
	SetShort(send_buff, page, send_index);
	SetShort(send_buff, count - start, send_index);
	SetString(send_buff, temp_buff, buff_index, send_index);
	pUser->Send(send_buff, send_index);
	return;

fail_return:
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_CURRENT_REQ, send_index);
	SetByte(send_buff, 0x00, send_index);
	SetString2(send_buff, errormsg, send_index);
	pUser->Send(send_buff, send_index);
}

void CKnightsManager::ReceiveKnightsProcess(CUser* pUser, char* pBuf, uint8_t command)
{
	int index = 0, send_index = 0, pktsize = 0, count = 0;
	uint8_t result;
	char send_buff[2048] = {};
	CUser* pTUser = nullptr;
	std::string errormsg = fmt::format_db_resource(IDP_KNIGHT_DB_FAIL);

	result = GetByte(pBuf, index);

	//TRACE(_T("ReceiveKnightsProcess - command=%d, result=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, result, pUser->GetSocketID(), pUser->m_pUserData->m_id, pUser->m_pUserData->m_bKnights, pUser->m_pUserData->m_bFame);

	if (result > 0)
	{
		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, command - 0x10, send_index);
		//SetByte( send_buff, 0x00, send_index );
		SetByte(send_buff, result, send_index);
		SetString2(send_buff, errormsg, send_index);
		pUser->Send(send_buff, send_index);
		return;
	}

	switch (command)
	{
		case KNIGHTS_CREATE + 0x10:
			RecvCreateKnights(pUser, pBuf + index);
			break;

		case KNIGHTS_JOIN + 0x10:
		case KNIGHTS_WITHDRAW + 0x10:
			RecvJoinKnights(pUser, pBuf + index, command);
			break;

		case KNIGHTS_REMOVE + 0x10:
		case KNIGHTS_ADMIT + 0x10:
		case KNIGHTS_REJECT + 0x10:
		case KNIGHTS_CHIEF + 0x10:
		case KNIGHTS_VICECHIEF + 0x10:
		case KNIGHTS_OFFICER + 0x10:
		case KNIGHTS_PUNISH + 0x10:
			RecvModifyFame(pUser, pBuf + index, command);
			break;

		case KNIGHTS_DESTROY + 0x10:
			RecvDestroyKnights(pUser, pBuf + index);
			break;

		case KNIGHTS_MEMBER_REQ + 0x10:
		{
			CKnights* pKnights = m_pMain->m_KnightsMap.GetData(pUser->m_pUserData->m_bKnights);
			if (pKnights == nullptr)
				break;

			pktsize = GetShort(pBuf, index);
			count = GetShort(pBuf, index);

			if (count > 24)
				break;

			SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
			SetByte(send_buff, KNIGHTS_MEMBER_REQ, send_index);
			SetByte(send_buff, 0x01, send_index);
			SetShort(send_buff, pktsize, send_index);
			SetShort(send_buff, count, send_index);
			//SetShort( send_buff, strlen(pKnights->strChief), send_index );
			//SetString( send_buff, pKnights->strChief, strlen( pKnights->strChief), send_index );
			//SetString( send_buff, pBuf+index, pktsize, send_index );
			SetString(send_buff, pBuf + index, pktsize, send_index);
			pUser->Send(send_buff, send_index);
		}
		break;

		case KNIGHTS_STASH + 0x10:
			break;

		case KNIGHTS_LIST_REQ + 0x10:
			RecvKnightsList(pBuf + index);
			break;
	}
}

void CKnightsManager::RecvCreateKnights(CUser* pUser, char* pBuf)
{
	int index = 0, send_index = 0, namelen = 0, idlen = 0, knightsindex = 0, nation = 0, community = 0, money = 0;
	char send_buff[128] = {};
	char knightsname[MAX_ID_SIZE + 1] = {};
	char chiefname[MAX_ID_SIZE + 1] = {};
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

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
	memset(pKnights->m_strViceChief_1, 0, sizeof(pKnights->m_strViceChief_1));
	memset(pKnights->m_strViceChief_2, 0, sizeof(pKnights->m_strViceChief_2));
	memset(pKnights->m_strViceChief_3, 0, sizeof(pKnights->m_strViceChief_3));
	pKnights->m_sMembers = 1;
	pKnights->m_nMoney = 0;
	pKnights->m_nPoints = 0;
	pKnights->m_byGrade = 5;
	pKnights->m_byRanking = 0;

	pUser->m_pUserData->m_bKnights = knightsindex;
	pUser->m_pUserData->m_bFame = CHIEF;
	money = pUser->m_pUserData->m_iGold - 500000;
	pUser->m_pUserData->m_iGold = money;

	for (int i = 0; i < MAX_CLAN; i++)
	{
		pKnights->m_arKnightsUser[i].byUsed = 0;
		strcpy(pKnights->m_arKnightsUser[i].strUserName, "");
	}

	m_pMain->m_KnightsMap.PutData(pKnights->m_sIndex, pKnights);

	// 클랜정보에 추가
	AddKnightsUser(knightsindex, chiefname);

	//TRACE(_T("RecvCreateKnights - nid=%d, name=%hs, index=%d, fame=%d, money=%d\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, knightsindex, pUser->m_pUserData->m_bFame, money);

	//if( pKnights->bFlag == KNIGHTS_TYPE )	{
/*	memset( send_buff, 0x00, 128 ); send_index = 0;
		SetByte( send_buff, WIZ_KNIGHTS_LIST, send_index );
		SetByte( send_buff, 0x02, send_index );					// Insert Knights From List
		SetShort( send_buff, knightsindex, send_index );
		SetShort( send_buff, namelen, send_index );
		SetString( send_buff, knightsname, namelen, send_index );
		m_pMain->Send_All( send_buff, send_index, pUser );	*/
	//}

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_CREATE, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetShort(send_buff, knightsindex, send_index);
	SetShort(send_buff, namelen, send_index);
	SetString(send_buff, knightsname, namelen, send_index);
	SetByte(send_buff, 5, send_index);  // knights grade
	SetByte(send_buff, 0, send_index);
	SetDWORD(send_buff, money, send_index);
	m_pMain->Send_Region(send_buff, send_index, pUser->m_pUserData->m_bZone, pUser->m_RegionX, pUser->m_RegionZ, nullptr, false);
	//pUser->Send( send_buff, send_index );

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, UDP_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_CREATE, send_index);
	SetByte(send_buff, community, send_index);
	SetShort(send_buff, knightsindex, send_index);
	SetByte(send_buff, nation, send_index);
	SetShort(send_buff, namelen, send_index);
	SetString(send_buff, knightsname, namelen, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, chiefname, idlen, send_index);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(send_buff, send_index);
	else
		m_pMain->Send_UDP_All(send_buff, send_index, 1);
}

void CKnightsManager::RecvJoinKnights(CUser* pUser, char* pBuf, uint8_t command)
{
	int send_index = 0, knightsindex = 0, index = 0, idlen = 0;
	char send_buff[128] = {};
	std::string finalstr;
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

	knightsindex = GetShort(pBuf, index);
	pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);

	if (command == KNIGHTS_JOIN + 0x10)
	{
		pUser->m_pUserData->m_bKnights = knightsindex;
		pUser->m_pUserData->m_bFame = TRAINEE;
		finalstr = fmt::format_db_resource(IDS_KNIGHTS_JOIN, pUser->m_pUserData->m_id);

		// 클랜정보에 추가
		AddKnightsUser(knightsindex, pUser->m_pUserData->m_id);

		//TRACE(_T("RecvJoinKnights - 가입, nid=%d, name=%hs, index=%d, fame=%d\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, pUser->m_pUserData->m_bKnights, pUser->m_pUserData->m_bFame);
	}
	// 탈퇴..
	else
	{
		pUser->m_pUserData->m_bKnights = 0;
		pUser->m_pUserData->m_bFame = 0;

		// 클랜정보에 추가
		RemoveKnightsUser(knightsindex, pUser->m_pUserData->m_id);

		/*if (pKnights != nullptr)
		{
			if (strcmp(pKnights->strViceChief_1, pUser->m_pUserData->m_id) == 0)
				memset(pKnights->strViceChief_1, 0, sizeof(pKnights->strViceChief_1));
			else if (strcmp( pKnights->strViceChief_2, pUser->m_pUserData->m_id) == 0)
				memset(pKnights->strViceChief_2, 0, sizeof(pKnights->strViceChief_2));
			else if (strcmp( pKnights->strViceChief_3, pUser->m_pUserData->m_id) == 0)
				memset(pKnights->strViceChief_3, 0, sizeof(pKnights->strViceChief_3));
		}*/

		finalstr = fmt::format_db_resource(IDS_KNIGHTS_WITHDRAW, pUser->m_pUserData->m_id);
		//TRACE(_T("RecvJoinKnights - 탈퇴, nid=%d, name=%hs, index=%d, fame=%d\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, pUser->m_pUserData->m_bKnights, pUser->m_pUserData->m_bFame);
	}

	//TRACE(_T("RecvJoinKnights - command=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, pUser->GetSocketID(), pUser->m_pUserData->m_id, pUser->m_pUserData->m_bKnights, pUser->m_pUserData->m_bFame);

	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, command - 0x10, send_index);
	SetByte(send_buff, 0x01, send_index);
	SetShort(send_buff, pUser->GetSocketID(), send_index);
	SetShort(send_buff, pUser->m_pUserData->m_bKnights, send_index);
	SetByte(send_buff, pUser->m_pUserData->m_bFame, send_index);

	if (pKnights != nullptr)
	{
		SetString2(send_buff, pKnights->m_strName, send_index);
		SetByte(send_buff, pKnights->m_byGrade, send_index);  // knights grade
		SetByte(send_buff, pKnights->m_byRanking, send_index);  // knights grade
	}

	m_pMain->Send_Region(send_buff, send_index, pUser->m_pUserData->m_bZone, pUser->m_RegionX, pUser->m_RegionZ, nullptr, false);
	//pUser->Send( send_buff, send_index );

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, KNIGHTS_CHAT, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, finalstr, send_index);
	m_pMain->Send_KnightsMember(knightsindex, send_buff, send_index);

	idlen = strlen(pUser->m_pUserData->m_id);
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, UDP_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, command - 0x10, send_index);
	SetShort(send_buff, knightsindex, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, pUser->m_pUserData->m_id, idlen, send_index);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(send_buff, send_index);
	else
		m_pMain->Send_UDP_All(send_buff, send_index, 1);
}

void CKnightsManager::RecvModifyFame(CUser* pUser, char* pBuf, uint8_t command)
{
	int index = 0, send_index = 0, knightsindex = 0, idlen = 0, vicechief = 0;
	char send_buff[128] = {};
	std::string finalstr;
	char userid[MAX_ID_SIZE + 1] = {};
	CUser* pTUser = nullptr;
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

	knightsindex = GetShort(pBuf, index);
	idlen = GetShort(pBuf, index);
	GetString(userid, pBuf, idlen, index);
	vicechief = GetByte(pBuf, index);

	pTUser = m_pMain->GetUserPtr(userid, NameType::Character);
	pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);

	switch (command)
	{
		case KNIGHTS_REMOVE + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bKnights = 0;
				pTUser->m_pUserData->m_bFame = 0;
				finalstr = fmt::format_db_resource(IDS_KNIGHTS_REMOVE, pTUser->m_pUserData->m_id);

				RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			else
			{
				RemoveKnightsUser(knightsindex, userid);
			}
			break;

		case KNIGHTS_ADMIT + 0x10:
			if (pTUser != nullptr)
				pTUser->m_pUserData->m_bFame = KNIGHT;
			break;

		case KNIGHTS_REJECT + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bKnights = 0;
				pTUser->m_pUserData->m_bFame = 0;

				RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			break;

		case KNIGHTS_CHIEF + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = CHIEF;
				ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
				finalstr = fmt::format_db_resource(IDS_KNIGHTS_CHIEF, pTUser->m_pUserData->m_id);
			}
			break;

		case KNIGHTS_VICECHIEF + 0x10:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = VICECHIEF;
				ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
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

/*	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, command - 0x10, send_index);
	SetByte(send_buff, 0x01, send_index);
	pUser->Send( send_buff, send_index);
*/
	//TRACE(_T("RecvModifyFame - command=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, pTUser->GetSocketID(), pTUser->m_pUserData->m_id, knightsindex, pTUser->m_pUserData->m_bFame);

	if (pTUser != nullptr)
	{
		//TRACE(_T("RecvModifyFame - command=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, pTUser->GetSocketID(), pTUser->m_pUserData->m_id, knightsindex, pTUser->m_pUserData->m_bFame);
		memset(send_buff, 0, sizeof(send_buff));
		send_index = 0;
		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_MODIFY_FAME, send_index);
		SetByte(send_buff, 0x01, send_index);

		if (command == KNIGHTS_REMOVE + 0x10)
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

		if (command == KNIGHTS_REMOVE + 0x10)
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

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, KNIGHTS_CHAT, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, finalstr, send_index);
	m_pMain->Send_KnightsMember(knightsindex, send_buff, send_index);

	idlen = strlen(userid);
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, UDP_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, command - 0x10, send_index);
	SetShort(send_buff, knightsindex, send_index);
	SetShort(send_buff, idlen, send_index);
	SetString(send_buff, userid, idlen, send_index);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(send_buff, send_index);
	else
		m_pMain->Send_UDP_All(send_buff, send_index, 1);
}

void CKnightsManager::RecvDestroyKnights(CUser* pUser, char* pBuf)
{
	int send_index = 0, knightsindex = 0, index = 0, flag = 0;
	char send_buff[128] = {};
	std::string finalstr;
	CKnights* pKnights = nullptr;
	CUser* pTUser = nullptr;

	if (pUser == nullptr)
		return;

	knightsindex = GetShort(pBuf, index);

	pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);
	if (pKnights == nullptr)
	{
		//TRACE(_T("### RecvDestoryKnights  Fail == index = %d ###\n"), knightsindex);
		return;
	}

	flag = pKnights->m_byFlag;

	// 클랜이나 기사단이 파괴된 메시지를 보내고 유저 데이타를 초기화
	if (flag == CLAN_TYPE)
		finalstr = fmt::format_db_resource(IDS_CLAN_DESTORY, pKnights->m_strName);
	else if (flag == KNIGHTS_TYPE)
		finalstr = fmt::format_db_resource(IDS_CLAN_DESTORY, pKnights->m_strName);
	
	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_CHAT, send_index);
	SetByte(send_buff, KNIGHTS_CHAT, send_index);
	SetByte(send_buff, 1, send_index);
	SetShort(send_buff, -1, send_index);
	SetByte(send_buff, 0, send_index);			// sender name length
	SetString2(send_buff, finalstr, send_index);
	m_pMain->Send_KnightsMember(knightsindex, send_buff, send_index);

	for (int i = 0; i < MAX_USER; i++)
	{
		pTUser = (CUser*) m_pMain->m_Iocport.m_SockArray[i];
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bKnights == knightsindex)
		{
			pTUser->m_pUserData->m_bKnights = 0;
			pTUser->m_pUserData->m_bFame = 0;

			RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);

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

	m_pMain->m_KnightsMap.DeleteData(knightsindex);
	//TRACE(_T("RecvDestoryKnights - nid=%d, name=%hs, index=%d, fame=%d\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, knightsindex, pUser->m_pUserData->m_bFame);

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_DESTROY, send_index);
	SetByte(send_buff, 0x01, send_index);
	pUser->Send(send_buff, send_index);

	memset(send_buff, 0, sizeof(send_buff));
	send_index = 0;
	SetByte(send_buff, UDP_KNIGHTS_PROCESS, send_index);
	SetByte(send_buff, KNIGHTS_DESTROY, send_index);
	SetShort(send_buff, knightsindex, send_index);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(send_buff, send_index);
	else
		m_pMain->Send_UDP_All(send_buff, send_index, 1);

	//if( flag == KNIGHTS_TYPE )	{
/*	memset( send_buff, 0x00, 128 ); send_index = 0;
		SetByte( send_buff, WIZ_KNIGHTS_LIST, send_index );
		SetByte( send_buff, 0x03, send_index );					// Knights Remove From List
		SetShort( send_buff, knightsindex, send_index );
		m_pMain->Send_All( send_buff, send_index, pUser );	*/
	//}
}

void CKnightsManager::RecvKnightsList(char* pBuf)
{
	CKnights* pKnights = nullptr;

	int nation = 0, members = 0, index = 0, iLength = 0, knightsindex = 0, points = 0, ranking = 0;
	char knightsname[MAX_ID_SIZE + 1] = {};

	knightsindex = GetShort(pBuf, index);
	nation = GetByte(pBuf, index);
	iLength = GetShort(pBuf, index);
	GetString(knightsname, pBuf, iLength, index);
	members = GetShort(pBuf, index);
	points = GetDWORD(pBuf, index); // knights grade
	ranking = GetByte(pBuf, index);

	if (m_pMain->m_nServerNo == BATTLE)
	{
		pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);
		if (pKnights != nullptr)
		{
			pKnights->m_sIndex = knightsindex;
			pKnights->m_byNation = nation;
			strcpy(pKnights->m_strName, knightsname);
			pKnights->m_sMembers = members;
			pKnights->m_nPoints = points;
			pKnights->m_byGrade = m_pMain->GetKnightsGrade(points);
			pKnights->m_byRanking = ranking;
		}
		else
		{
			pKnights = new CKnights();
			pKnights->m_sIndex = knightsindex;
			pKnights->m_byNation = nation;
			strcpy(pKnights->m_strName, knightsname);
			pKnights->m_sMembers = members;
			strcpy(pKnights->m_strChief, "");
			strcpy(pKnights->m_strViceChief_1, "");
			strcpy(pKnights->m_strViceChief_2, "");
			strcpy(pKnights->m_strViceChief_3, "");
			pKnights->m_nMoney = 0;
			pKnights->m_sDomination = 0;
			pKnights->m_nPoints = points;
			pKnights->m_byGrade = m_pMain->GetKnightsGrade(points);
			pKnights->m_byRanking = ranking;

			if (!m_pMain->m_KnightsMap.PutData(pKnights->m_sIndex, pKnights))
			{
				spdlog::error("KnightsManager::RecvKnightsList: KnightsMap put failed [knightsId={}]",
					pKnights->m_sIndex);
				delete pKnights;
				pKnights = nullptr;
			}
		}
	}
}

bool CKnightsManager::AddKnightsUser(int knightsId, const char* charId)
{
	CKnights* pKnights = m_pMain->m_KnightsMap.GetData(knightsId);
	if (pKnights == nullptr)
	{
		spdlog::error("KnightsManager::AddKnightsUser: knightsId={} not found",
			knightsId);
		return false;
	}

	for (int i = 0; i < MAX_CLAN; i++)
	{
		if (pKnights->m_arKnightsUser[i].byUsed != 0)
			continue;

		pKnights->m_arKnightsUser[i].byUsed = 1;
		strcpy(pKnights->m_arKnightsUser[i].strUserName, charId);
		//TRACE(_T("+++ AddKnightsUser knightsindex : username=%hs, knightsindex=%d, i=%d \n"), UserName, index, i);
		return true;
	}

	//TRACE(_T("#### AddKnightsUser user full : username=%hs, knightsindex=%d ####\n"), UserName, index);
	return false;
}

bool CKnightsManager::ModifyKnightsUser(int knightsId, const char* charId)
{
	CKnights* pKnights = m_pMain->m_KnightsMap.GetData(knightsId);
	if (pKnights == nullptr)
	{
		spdlog::error("KnightsManager::ModifyKnightsUser: knightsId={} not found",
			knightsId);
		return false;
	}

	for (int i = 0; i < MAX_CLAN; i++)
	{
		if (pKnights->m_arKnightsUser[i].byUsed == 0)
			continue;

		if (strcmp(pKnights->m_arKnightsUser[i].strUserName, charId) == 0)
		{
			pKnights->m_arKnightsUser[i].byUsed = 1;
			strcpy(pKnights->m_arKnightsUser[i].strUserName, charId);
			return true;
		}
	}

	//TRACE(_T("#### ModifyKnightsUser user full : username=%hs, knightsindex=%d ####\n"), UserName, index);
	return false;
}

bool CKnightsManager::RemoveKnightsUser(int knightsId, const char* charId)
{
	CKnights* pKnights = m_pMain->m_KnightsMap.GetData(knightsId);
	if (pKnights == nullptr)
	{
		spdlog::error("KnightsManager::RemoveKnightsUser: knightsId={} not found",
			knightsId);
		return false;
	}

	for (int i = 0; i < MAX_CLAN; i++)
	{
		if (pKnights->m_arKnightsUser[i].byUsed == 0)
			continue;

		if (strcmp(pKnights->m_arKnightsUser[i].strUserName, charId) == 0)
		{
			pKnights->m_arKnightsUser[i].byUsed = 0;
			strcpy(pKnights->m_arKnightsUser[i].strUserName, "");
			//TRACE(_T("---> RemoveKnightsUser knightsindex : username=%hs, knightsindex=%d, i=%d \n"), UserName, index, i);
			return true;
		}
	}

	//TRACE(_T("#### RemoveKnightsUser user full : username=%hs, knightsindex=%d ####\n"), UserName, index);
	return false;
}

void CKnightsManager::SetKnightsUser(int knightsId, const char* charId)
{
	CKnights* pKnights = m_pMain->m_KnightsMap.GetData(knightsId);
	if (pKnights == nullptr)
	{
		spdlog::error("KnightsManager::SetKnightsUser: knightsId={} not found",
			knightsId);
		return;
	}

	//TRACE(_T("--- SetKnightsUser knightsindex : username=%hs, knightsindex=%d \n"), UserName, index);

	for (int i = 0; i < MAX_CLAN; i++)
	{
		if (pKnights->m_arKnightsUser[i].byUsed == 0)
			continue;

		if (strcmp(pKnights->m_arKnightsUser[i].strUserName, charId) == 0)
		{
			//TRACE(_T("### SetKnightsUser knightsindex - name is same : username=%hs, knightsindex=%d \n"), UserName, index);
			return;
		}
	}

	if (!AddKnightsUser(knightsId, charId))
	{
		//TRACE(_T("#### SetKnightsUser user full : username=%hs, knightsindex=%d ####\n"), UserName, index);
	}
}

void CKnightsManager::RecvKnightsAllList(char* pBuf)
{
	int index = 0, knightsId = 0, points = 0, count = 0, grade = 0, ranking = 0;
	int send_index = 0, temp_index = 0, send_count = 0;
	CKnights* pKnights = nullptr;
	char send_buff[512] = {};
	char temp_buff[512] = {};

	count = GetByte(pBuf, index);

	for (int i = 0; i < count; i++)
	{
		knightsId = GetShort(pBuf, index);
		points = GetDWORD(pBuf, index);
		ranking = GetByte(pBuf, index);

		pKnights = m_pMain->m_KnightsMap.GetData(knightsId);
		if (pKnights == nullptr)
		{
			spdlog::error("KnightsManager::RecvKnightsAllList: knightsId={} not found",
				knightsId);
			continue;
		}

		if (pKnights->m_nPoints != points)
		{
			pKnights->m_nPoints = points;
			pKnights->m_byGrade = m_pMain->GetKnightsGrade(points);

			SetShort(temp_buff, pKnights->m_sIndex, temp_index);
			SetByte(temp_buff, pKnights->m_byGrade, temp_index);
			SetByte(temp_buff, pKnights->m_byRanking, temp_index);
			send_count++;
		}
		else if (pKnights->m_byRanking != ranking)
		{
			pKnights->m_byRanking = ranking;

			SetShort(temp_buff, pKnights->m_sIndex, temp_index);
			SetByte(temp_buff, pKnights->m_byGrade, temp_index);
			SetByte(temp_buff, pKnights->m_byRanking, temp_index);
			send_count++;
		}
	}

	if (send_count > 0)
	{
		SetByte(send_buff, WIZ_KNIGHTS_PROCESS, send_index);
		SetByte(send_buff, KNIGHTS_ALLLIST_REQ, send_index);
		SetShort(send_buff, send_count, send_index);
		SetString(send_buff, temp_buff, temp_index, send_index);
		m_pMain->Send_All(send_buff, send_index, nullptr);
	}
}
