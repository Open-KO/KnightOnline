#include "pch.h"
#include "KnightsManager.h"
#include "User.h"
#include "GameDefine.h"
#include "EbenezerApp.h"
#include "db_resources.h"

#include <shared/DateTime.h>
#include <shared/packets.h>
#include <shared/StringUtils.h>

#include <spdlog/spdlog.h>

namespace Ebenezer
{

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

	int index       = 0;
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
			WithdrawKnights(pUser);
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
			AllKnightsMember(pUser);
			break;

		case KNIGHTS_CURRENT_REQ:
			CurrentKnightsMember(pUser, pBuf + index);
			break;

		case KNIGHTS_STASH:
			break;

		case KNIGHTS_JOIN_REQ:
			JoinKnightsReq(pUser, pBuf + index);
			break;

		default:
			spdlog::error(
				"KnightsManager::PacketProcess: Unhandled opcode {:02X} [characterName={}]",
				command, pUser->m_pUserData->m_id);
			break;
	}
}

void CKnightsManager::CreateKnights(CUser* pUser, char* pBuf)
{
	int index = 0, sendIndex = 0, idlen = 0, knightindex = 0, ret_value = 3;
	// int week;
	char idname[MAX_ID_SIZE + 1] {}, sendBuffer[256] {};
	// DateTime time = DateTime::GetNow();

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
	/*week = time.GetDayOfWeek();
	if (week != 1
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

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, DB_KNIGHTS_CREATE, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetByte(sendBuffer, CLAN_TYPE, sendIndex);
	SetShort(sendBuffer, knightindex, sendIndex);
	SetByte(sendBuffer, pUser->m_pUserData->m_bNation, sendIndex);
	SetShort(sendBuffer, idlen, sendIndex);
	SetString(sendBuffer, idname, idlen, sendIndex);
	SetString2(sendBuffer, pUser->m_pUserData->m_id, sendIndex);
	m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CREATE, sendIndex);
	SetByte(sendBuffer, ret_value, sendIndex);
	//TRACE(_T("## CreateKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);

	pUser->Send(sendBuffer, sendIndex);
}

bool CKnightsManager::IsAvailableName(const char* strname) const
{
	for (const auto& [_, pKnights] : m_pMain->m_KnightsMap)
	{
		if (strnicmp(pKnights->m_strName, strname, MAX_ID_SIZE) == 0)
			return false;
	}

	return true;
}

int CKnightsManager::GetKnightsIndex(int nation)
{
	//TRACE(_T("GetKnightsIndex = nation=%d\n"), nation);
	int knightindex = 0;
	// sungyong tw~
	//if (m_pMain->m_nServerNo == SERVER_ZONE_ELMORAD) knightindex = 15000;
	if (nation == NATION_ELMORAD)
		knightindex = 15000;
	// ~sungyong tw

	for (const auto& [_, pKnights] : m_pMain->m_KnightsMap)
	{
		if (knightindex < pKnights->m_sIndex)
		{
			if (nation == NATION_KARUS)
			{
				// sungyong,, 카루스와 전쟁존의 합침으로 인해서,,,
				if (pKnights->m_sIndex >= 15000)
					continue;
			}

			knightindex = pKnights->m_sIndex;
		}
	}

	knightindex++;
	if (nation == NATION_KARUS)
	{
		if (knightindex >= 15000 || knightindex < 0)
			return -1;
	}
	else if (nation == NATION_ELMORAD)
	{
		if (knightindex < 15000 || knightindex > 30000)
			return -1;
	}

	// 확인 사살..
	if (m_pMain->m_KnightsMap.GetData(knightindex))
		return -1;

	return knightindex;
}

void CKnightsManager::JoinKnights(CUser* pUser, char* pBuf)
{
	int knightsindex = 0, index = 0, sendIndex = 0, ret_value = 0, member_id = 0;
	CKnights* pKnights = nullptr;
	std::shared_ptr<CUser> pTUser;
	char sendBuffer[128] {};

	if (pUser == nullptr)
		return;

	// 전쟁존에서는 기사단 처리가 안됨
	if (pUser->m_pUserData->m_bZone > 2)
	{
		ret_value = 12;
		goto fail_return;
	}

	if (pUser->m_pUserData->m_bFame != KNIGHTS_DUTY_CHIEF
		&& pUser->m_pUserData->m_bFame != KNIGHTS_DUTY_VICECHIEF)
	{
		ret_value = 6;
		goto fail_return;
	}

	knightsindex = pUser->m_pUserData->m_bKnights;
	pKnights     = m_pMain->m_KnightsMap.GetData(knightsindex);
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

	member_id = GetShort(pBuf, index);

	pTUser    = m_pMain->GetUserPtr(member_id);
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

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_JOIN_REQ, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetShort(sendBuffer, knightsindex, sendIndex);
	SetString2(sendBuffer, pKnights->m_strName, sendIndex);
	pTUser->Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_JOIN, sendIndex);
	SetByte(sendBuffer, ret_value, sendIndex);
	//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::JoinKnightsReq(CUser* pUser, char* pBuf)
{
	int knightsindex = 0, index = 0, sendIndex = 0, ret_value = 0, flag = 0, sid = -1;
	CKnights* pKnights = nullptr;
	std::shared_ptr<CUser> pTUser;
	char sendBuffer[128] {};

	if (pUser == nullptr)
		return;

	flag   = GetByte(pBuf, index);
	sid    = GetShort(pBuf, index);

	pTUser = m_pMain->GetUserPtr(sid);
	if (pTUser == nullptr)
	{
		ret_value = 2;
		SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
		SetByte(sendBuffer, KNIGHTS_JOIN, sendIndex);
		SetByte(sendBuffer, ret_value, sendIndex);
		//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
		pUser->Send(sendBuffer, sendIndex);
		return;
	}

	// 거절
	if (flag == 0)
	{
		ret_value = 11;
		SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
		SetByte(sendBuffer, KNIGHTS_JOIN, sendIndex);
		SetByte(sendBuffer, ret_value, sendIndex);
		//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pTUser->GetSocketID(), pTUser->m_pUserData->m_id, ret_value);
		pTUser->Send(sendBuffer, sendIndex);
		return;
	}

	knightsindex = GetShort(pBuf, index);
	pKnights     = m_pMain->m_KnightsMap.GetData(knightsindex);
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

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, DB_KNIGHTS_JOIN, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetShort(sendBuffer, knightsindex, sendIndex);
	m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_JOIN, sendIndex);
	SetByte(sendBuffer, ret_value, sendIndex);
	//TRACE(_T("## JoinKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::WithdrawKnights(CUser* pUser)
{
	int sendIndex = 0, ret_value = 0;
	char sendBuffer[128] {};

	if (pUser == nullptr)
		return;

	// 기사단에 가입되어 있지 않습니다
	if (pUser->m_pUserData->m_bKnights < 1 || pUser->m_pUserData->m_bKnights > 30000)
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
	if (pUser->m_pUserData->m_bFame == KNIGHTS_DUTY_CHIEF)
	{
		// 전쟁존에서는 클랜을 파괴할 수 없다
		if (pUser->m_pUserData->m_bZone > 2)
		{
			ret_value = 12;
			goto fail_return;
		}

		SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
		SetByte(sendBuffer, DB_KNIGHTS_DESTROY, sendIndex);
		SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
		SetShort(sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex);
		m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
		return;
	}

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, DB_KNIGHTS_WITHDRAW, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetShort(sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex);
	m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_WITHDRAW, sendIndex);
	SetByte(sendBuffer, ret_value, sendIndex);
	//TRACE(_T("## WithDrawKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::DestroyKnights(CUser* pUser)
{
	int sendIndex = 0, ret_value = 0;
	char sendBuffer[128] {};

	if (pUser == nullptr)
		return;

	if (pUser->m_pUserData->m_bFame != KNIGHTS_DUTY_CHIEF)
		goto fail_return;

	if (pUser->m_pUserData->m_bZone > 2)
	{
		ret_value = 12;
		goto fail_return;
	}

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, DB_KNIGHTS_DESTROY, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetShort(sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex);
	m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_DESTROY, sendIndex);
	SetByte(sendBuffer, ret_value, sendIndex);
	//TRACE(_T("## DestoryKnights Fail - nid=%d, name=%hs, error_code=%d ##\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::ModifyKnightsMember(CUser* pUser, char* pBuf, uint8_t command)
{
	std::shared_ptr<CUser> pTUser;
	int index = 0, sendIndex = 0, idlen = 0, ret_value = 0, remove_flag = 0;
	char sendBuffer[128] {}, userid[MAX_ID_SIZE + 1] {};

	if (pUser == nullptr)
		return;

	idlen = GetShort(pBuf, index);

	// 잘못된 아이디
	if (idlen > MAX_ID_SIZE || idlen <= 0)
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
	if (strnicmp(userid, pUser->m_pUserData->m_id, MAX_ID_SIZE) == 0)
	{
		ret_value = 9;
		goto fail_return;
	}

	// 기사단, 멤버가입 및 멤버거절, 장교 이상이 할 수 있습니다
	if (command == KNIGHTS_ADMIT || command == KNIGHTS_REJECT)
	{
		if (pUser->m_pUserData->m_bFame < KNIGHTS_DUTY_OFFICER)
			goto fail_return;
	}
	// 징계, 부기사단장 이상이 할 수 있습니다
	else if (command == KNIGHTS_PUNISH)
	{
		if (pUser->m_pUserData->m_bFame < KNIGHTS_DUTY_VICECHIEF)
			goto fail_return;
	}
	// 기사단장 만이 할 수 있습니다
	else
	{
		if (pUser->m_pUserData->m_bFame != KNIGHTS_DUTY_CHIEF)
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
			SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
			SetByte(sendBuffer, command + 0x10, sendIndex);
			SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
			SetShort(sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex);
			SetShort(sendBuffer, idlen, sendIndex);
			SetString(sendBuffer, userid, idlen, sendIndex);
			SetByte(sendBuffer, remove_flag, sendIndex);
			m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
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
		if (pTUser->m_pUserData->m_bFame == KNIGHTS_DUTY_VICECHIEF)
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

	remove_flag = 1; // 게임상에 있는 유저 추방시 (1)
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, command + 0x10, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetShort(sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex);
	SetShort(sendBuffer, idlen, sendIndex);
	SetString(sendBuffer, userid, idlen, sendIndex);
	SetByte(sendBuffer, remove_flag, sendIndex);
	m_pMain->m_LoggerSendQueue.PutData(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, command, sendIndex);
	SetByte(sendBuffer, ret_value, sendIndex);
	//TRACE(_T("## ModifyKnights Fail - command=%d, nid=%d, name=%hs, error_code=%d ##\n"), command, pUser->GetSocketID(), pUser->m_pUserData->m_id, ret_value);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::AllKnightsList(CUser* pUser, char* pBuf)
{
	int sendIndex = 0, buff_index = 0, count = 0, page = 0, index = 0, start = 0;
	char sendBuffer[4096] {}, temp_buff[4096] {};

	if (pUser == nullptr)
		return;

	page  = GetShort(pBuf, index);
	start = page * 10; // page : 0 ~

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
		SetString2(temp_buff, pKnights->m_strName, buff_index);
		SetShort(temp_buff, pKnights->m_sMembers, buff_index);
		SetString2(temp_buff, pKnights->m_strChief, buff_index);
		SetDWORD(temp_buff, pKnights->m_nPoints, buff_index);

		count++;
		if (count >= start + 10)
			break;
	}

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_ALLLIST_REQ, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetShort(sendBuffer, page, sendIndex);
	SetShort(sendBuffer, count - start, sendIndex);
	SetString(sendBuffer, temp_buff, buff_index, sendIndex);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::AllKnightsMember(CUser* pUser)
{
	CKnights* pKnights = nullptr;
	int sendIndex = 0, ret_value = 0, temp_index = 0, count = 0, onlineCount = 0, pktsize = 0;
	char sendBuffer[4096] {}, temp_buff[4096] {};

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
		SetByte( sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex );
		SetByte( sendBuffer, DB_KNIGHTS_MEMBER_REQ, sendIndex );
		SetShort( sendBuffer, pUser->GetSocketID(), sendIndex );
		SetShort( sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex );
		//SetShort( sendBuffer, page, sendIndex );
		m_pMain->m_LoggerSendQueue.PutData( sendBuffer, sendIndex );
		return;
	}*/

	onlineCount = m_pMain->GetKnightsAllMembers(
		pUser->m_pUserData->m_bKnights, temp_buff, temp_index, 0);

	// 직접.. 게임서버에서 유저정보를 참조해서 불러오는 방식 (단장이 아닌 모든 사람)
	if (pUser->m_pUserData->m_bFame == KNIGHTS_DUTY_CHIEF)
		count = m_pMain->GetKnightsAllMembers(
			pUser->m_pUserData->m_bKnights, temp_buff, temp_index, 1);
	else
		count = onlineCount;

	pktsize = temp_index + 4;
	if (count > MAX_CLAN)
		return;

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_MEMBER_REQ, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetShort(sendBuffer, pktsize, sendIndex);
	SetShort(sendBuffer, onlineCount, sendIndex);
	SetShort(sendBuffer, pKnights->m_sMembers, sendIndex);
	SetShort(sendBuffer, count, sendIndex);
	SetString(sendBuffer, temp_buff, temp_index, sendIndex);
	pUser->Send(sendBuffer, sendIndex);

	/*	SetByte( sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex );
	SetByte( sendBuffer, DB_KNIGHTS_MEMBER_REQ, sendIndex );
	SetShort( sendBuffer, pUser->GetSocketID(), sendIndex );
	SetShort( sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex );
	//SetShort( sendBuffer, page, sendIndex );
	m_pMain->m_LoggerSendQueue.PutData( sendBuffer, sendIndex );	*/
	return;

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_MEMBER_REQ, sendIndex);
	SetByte(sendBuffer, ret_value, sendIndex);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::CurrentKnightsMember(CUser* pUser, char* pBuf)
{
	int index = 0, sendIndex = 0, buff_index = 0, count = 0, i = 0, page = 0, start = 0,
		socketCount = 0;
	char sendBuffer[128] {}, temp_buff[4096] {};
	CKnights* pKnights   = nullptr;
	std::string errormsg = fmt::format_db_resource(IDP_KNIGHT_NOT_REGISTERED);

	if (pUser == nullptr)
		return;

	if (pUser->m_pUserData->m_bKnights <= 0)
		goto fail_return;

	pKnights = m_pMain->m_KnightsMap.GetData(pUser->m_pUserData->m_bKnights);
	if (pKnights == nullptr)
		goto fail_return;

	page        = GetShort(pBuf, index);
	start       = page * 10; // page : 0 ~
	socketCount = m_pMain->GetUserSocketCount();

	for (i = 0; i < socketCount; i++)
	{
		auto pTUser = m_pMain->GetUserPtrUnchecked(i);
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bKnights != pUser->m_pUserData->m_bKnights)
			continue;

		if (count < start)
		{
			count++;
			continue;
		}

		SetString2(temp_buff, pUser->m_pUserData->m_id, buff_index);
		SetByte(temp_buff, pUser->m_pUserData->m_bFame, buff_index);
		SetByte(temp_buff, pUser->m_pUserData->m_bLevel, buff_index);
		SetShort(temp_buff, pUser->m_pUserData->m_sClass, buff_index);

		count++;
		if (count >= start + 10)
			break;
	}

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CURRENT_REQ, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetString2(sendBuffer, pKnights->m_strChief, sendIndex);
	SetShort(sendBuffer, page, sendIndex);
	SetShort(sendBuffer, count - start, sendIndex);
	SetString(sendBuffer, temp_buff, buff_index, sendIndex);
	pUser->Send(sendBuffer, sendIndex);
	return;

fail_return:
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CURRENT_REQ, sendIndex);
	SetByte(sendBuffer, 0x00, sendIndex);
	SetString2(sendBuffer, errormsg, sendIndex);
	pUser->Send(sendBuffer, sendIndex);
}

void CKnightsManager::ReceiveKnightsProcess(CUser* pUser, const char* pBuf, uint8_t command)
{
	if (pUser == nullptr)
		return;

	int index = 0, sendIndex = 0, pktsize = 0, count = 0;
	uint8_t result = 0;
	char sendBuffer[2048] {};
	std::string errormsg;

	errormsg = fmt::format_db_resource(IDP_KNIGHT_DB_FAIL);
	result   = GetByte(pBuf, index);

	//TRACE(_T("ReceiveKnightsProcess - command=%d, result=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, result, pUser->GetSocketID(), pUser->m_pUserData->m_id, pUser->m_pUserData->m_bKnights, pUser->m_pUserData->m_bFame);

	if (result > 0)
	{
		SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
		SetByte(sendBuffer, command - 0x10, sendIndex);
		//SetByte( sendBuffer, 0x00, sendIndex );
		SetByte(sendBuffer, result, sendIndex);
		SetString2(sendBuffer, errormsg, sendIndex);
		pUser->Send(sendBuffer, sendIndex);
		return;
	}

	switch (command)
	{
		case DB_KNIGHTS_CREATE:
			RecvCreateKnights(pUser, pBuf + index);
			break;

		case DB_KNIGHTS_JOIN:
		case DB_KNIGHTS_WITHDRAW:
			RecvJoinKnights(pUser, pBuf + index, command);
			break;

		case DB_KNIGHTS_REMOVE:
		case DB_KNIGHTS_ADMIT:
		case DB_KNIGHTS_REJECT:
		case DB_KNIGHTS_CHIEF:
		case DB_KNIGHTS_VICECHIEF:
		case DB_KNIGHTS_OFFICER:
		case DB_KNIGHTS_PUNISH:
			RecvModifyFame(pUser, pBuf + index, command);
			break;

		case DB_KNIGHTS_DESTROY:
			RecvDestroyKnights(pUser, pBuf + index);
			break;

		case DB_KNIGHTS_MEMBER_REQ:
		{
			CKnights* pKnights = m_pMain->m_KnightsMap.GetData(pUser->m_pUserData->m_bKnights);
			if (pKnights == nullptr)
				break;

			pktsize = GetShort(pBuf, index);
			count   = GetShort(pBuf, index);

			if (count > MAX_CLAN)
				break;

			SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
			SetByte(sendBuffer, KNIGHTS_MEMBER_REQ, sendIndex);
			SetByte(sendBuffer, 0x01, sendIndex);
			SetShort(sendBuffer, pktsize, sendIndex);
			SetShort(sendBuffer, count, sendIndex);
			//SetShort( sendBuffer, strlen(pKnights->strChief), sendIndex );
			//SetString( sendBuffer, pKnights->strChief, strlen( pKnights->strChief), sendIndex );
			//SetString( sendBuffer, pBuf+index, pktsize, sendIndex );
			SetString(sendBuffer, pBuf + index, pktsize, sendIndex);
			pUser->Send(sendBuffer, sendIndex);
		}
		break;

		case DB_KNIGHTS_STASH:
			break;

		case DB_KNIGHTS_LIST_REQ:
			RecvKnightsList(pBuf + index);
			break;

		default:
			spdlog::error(
				"KnightsManager::ReceivePacketProcess: Unhandled opcode {:02X} [characterName={}]",
				command, pUser->m_pUserData->m_id);
			break;
	}
}

void CKnightsManager::RecvCreateKnights(CUser* pUser, const char* pBuf)
{
	int index = 0, sendIndex = 0, namelen = 0, idlen = 0, knightsindex = 0, nation = 0,
		community = 0, money = 0;
	CKnights* pKnights = nullptr;
	char sendBuffer[128] {}, knightsname[MAX_ID_SIZE + 1] {}, chiefname[MAX_ID_SIZE + 1] {};

	if (pUser == nullptr)
		return;

	community    = GetByte(pBuf, index);
	knightsindex = GetShort(pBuf, index);
	nation       = GetByte(pBuf, index);
	namelen      = GetShort(pBuf, index);
	GetString(knightsname, pBuf, namelen, index);
	idlen = GetShort(pBuf, index);
	GetString(chiefname, pBuf, idlen, index);

	pKnights = new CKnights();
	pKnights->InitializeValue();

	pKnights->m_sIndex   = knightsindex;
	pKnights->m_byFlag   = community;
	pKnights->m_byNation = nation;
	strcpy_safe(pKnights->m_strName, knightsname);
	strcpy_safe(pKnights->m_strChief, chiefname);
	memset(pKnights->m_strViceChief_1, 0, sizeof(pKnights->m_strViceChief_1));
	memset(pKnights->m_strViceChief_2, 0, sizeof(pKnights->m_strViceChief_2));
	memset(pKnights->m_strViceChief_3, 0, sizeof(pKnights->m_strViceChief_3));
	pKnights->m_sMembers           = 1;
	pKnights->m_nMoney             = 0;
	pKnights->m_nPoints            = 0;
	pKnights->m_byGrade            = 5;
	pKnights->m_byRanking          = 0;

	pUser->m_pUserData->m_bKnights = knightsindex;
	pUser->m_pUserData->m_bFame    = KNIGHTS_DUTY_CHIEF;
	money                          = pUser->m_pUserData->m_iGold - 500000;
	pUser->m_pUserData->m_iGold    = money;

	for (int i = 0; i < MAX_CLAN; i++)
	{
		pKnights->m_arKnightsUser[i].byUsed = 0;
		strcpy_safe(pKnights->m_arKnightsUser[i].strUserName, "");
	}

	m_pMain->m_KnightsMap.PutData(pKnights->m_sIndex, pKnights);

	// 클랜정보에 추가
	AddKnightsUser(knightsindex, chiefname);

	//TRACE(_T("RecvCreateKnights - nid=%d, name=%hs, index=%d, fame=%d, money=%d\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, knightsindex, pUser->m_pUserData->m_bFame, money);

	//if( pKnights->bFlag == KNIGHTS_TYPE )	{
	/*	memset( sendBuffer, 0x00, 128 ); sendIndex = 0;
		SetByte( sendBuffer, WIZ_KNIGHTS_LIST, sendIndex );
		SetByte( sendBuffer, 0x02, sendIndex );					// Insert Knights From List
		SetShort( sendBuffer, knightsindex, sendIndex );
		SetShort( sendBuffer, namelen, sendIndex );
		SetString( sendBuffer, knightsname, namelen, sendIndex );
		m_pMain->Send_All( sendBuffer, sendIndex, pUser );	*/
	//}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CREATE, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetShort(sendBuffer, knightsindex, sendIndex);
	SetShort(sendBuffer, namelen, sendIndex);
	SetString(sendBuffer, knightsname, namelen, sendIndex);
	SetByte(sendBuffer, 5, sendIndex); // knights grade
	SetByte(sendBuffer, 0, sendIndex);
	SetDWORD(sendBuffer, money, sendIndex);
	m_pMain->Send_Region(sendBuffer, sendIndex, pUser->m_pUserData->m_bZone, pUser->m_RegionX,
		pUser->m_RegionZ, nullptr, false);
	//pUser->Send( sendBuffer, sendIndex );

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, UDP_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CREATE, sendIndex);
	SetByte(sendBuffer, community, sendIndex);
	SetShort(sendBuffer, knightsindex, sendIndex);
	SetByte(sendBuffer, nation, sendIndex);
	SetShort(sendBuffer, namelen, sendIndex);
	SetString(sendBuffer, knightsname, namelen, sendIndex);
	SetShort(sendBuffer, idlen, sendIndex);
	SetString(sendBuffer, chiefname, idlen, sendIndex);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(sendBuffer, sendIndex);
	else
		m_pMain->Send_UDP_All(sendBuffer, sendIndex, 1);
}

void CKnightsManager::RecvJoinKnights(CUser* pUser, const char* pBuf, uint8_t command)
{
	int sendIndex = 0, knightsindex = 0, index = 0, idlen = 0;
	char sendBuffer[128] {};
	std::string finalstr;
	CKnights* pKnights = nullptr;

	if (pUser == nullptr)
		return;

	knightsindex = GetShort(pBuf, index);
	pKnights     = m_pMain->m_KnightsMap.GetData(knightsindex);

	if (command == DB_KNIGHTS_JOIN)
	{
		pUser->m_pUserData->m_bKnights = knightsindex;
		pUser->m_pUserData->m_bFame    = KNIGHTS_DUTY_TRAINEE;
		finalstr = fmt::format_db_resource(IDS_KNIGHTS_JOIN, pUser->m_pUserData->m_id);

		// 클랜정보에 추가
		AddKnightsUser(knightsindex, pUser->m_pUserData->m_id);

		//TRACE(_T("RecvJoinKnights - 가입, nid=%d, name=%hs, index=%d, fame=%d\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, pUser->m_pUserData->m_bKnights, pUser->m_pUserData->m_bFame);
	}
	// 탈퇴..
	else
	{
		pUser->m_pUserData->m_bKnights = 0;
		pUser->m_pUserData->m_bFame    = 0;

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

	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, command - 0x10, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	SetShort(sendBuffer, pUser->GetSocketID(), sendIndex);
	SetShort(sendBuffer, pUser->m_pUserData->m_bKnights, sendIndex);
	SetByte(sendBuffer, pUser->m_pUserData->m_bFame, sendIndex);

	if (pKnights != nullptr)
	{
		SetString2(sendBuffer, pKnights->m_strName, sendIndex);
		SetByte(sendBuffer, pKnights->m_byGrade, sendIndex);   // knights grade
		SetByte(sendBuffer, pKnights->m_byRanking, sendIndex); // knights grade
	}

	m_pMain->Send_Region(sendBuffer, sendIndex, pUser->m_pUserData->m_bZone, pUser->m_RegionX,
		pUser->m_RegionZ, nullptr, false);
	//pUser->Send( sendBuffer, sendIndex );

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_CHAT, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CHAT, sendIndex);
	SetByte(sendBuffer, 1, sendIndex);
	SetShort(sendBuffer, -1, sendIndex);
	SetByte(sendBuffer, 0, sendIndex); // sender name length
	SetString2(sendBuffer, finalstr, sendIndex);
	m_pMain->Send_KnightsMember(knightsindex, sendBuffer, sendIndex);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, UDP_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, command - 0x10, sendIndex);
	SetShort(sendBuffer, knightsindex, sendIndex);
	SetShort(sendBuffer, idlen, sendIndex);
	SetString2(sendBuffer, pUser->m_pUserData->m_id, sendIndex);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(sendBuffer, sendIndex);
	else
		m_pMain->Send_UDP_All(sendBuffer, sendIndex, 1);
}

void CKnightsManager::RecvModifyFame(CUser* pUser, const char* pBuf, uint8_t command)
{
	int index = 0, sendIndex = 0;
	std::string finalstr;
	char sendBuffer[128] {}, userid[MAX_ID_SIZE + 1] {};

	if (pUser == nullptr)
		return;

	int knightsindex = GetShort(pBuf, index);
	int idlen        = GetShort(pBuf, index);
	GetString(userid, pBuf, idlen, index);
	/*int vicechief =*/GetByte(pBuf, index);

	auto pTUser = m_pMain->GetUserPtr(userid, NameType::Character);

	switch (command)
	{
		case DB_KNIGHTS_REMOVE:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bKnights = 0;
				pTUser->m_pUserData->m_bFame    = 0;
				finalstr = fmt::format_db_resource(IDS_KNIGHTS_REMOVE, pTUser->m_pUserData->m_id);

				RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			else
			{
				RemoveKnightsUser(knightsindex, userid);
			}
			break;

		case DB_KNIGHTS_ADMIT:
			if (pTUser != nullptr)
				pTUser->m_pUserData->m_bFame = KNIGHTS_DUTY_KNIGHT;
			break;

		case DB_KNIGHTS_REJECT:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bKnights = 0;
				pTUser->m_pUserData->m_bFame    = 0;

				RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
			}
			break;

		case DB_KNIGHTS_CHIEF:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = KNIGHTS_DUTY_CHIEF;
				ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
				finalstr = fmt::format_db_resource(IDS_KNIGHTS_CHIEF, pTUser->m_pUserData->m_id);
			}
			break;

		case DB_KNIGHTS_VICECHIEF:
			if (pTUser != nullptr)
			{
				pTUser->m_pUserData->m_bFame = KNIGHTS_DUTY_VICECHIEF;
				ModifyKnightsUser(knightsindex, pTUser->m_pUserData->m_id);
				finalstr = fmt::format_db_resource(
					IDS_KNIGHTS_VICECHIEF, pTUser->m_pUserData->m_id);
			}
			break;

		case DB_KNIGHTS_OFFICER:
			if (pTUser != nullptr)
				pTUser->m_pUserData->m_bFame = KNIGHTS_DUTY_OFFICER;
			break;

		default:
			spdlog::error("KnightsManager::RecvModify: Unhandled opcode {:02X} [characterName={}]",
				command, pUser->m_pUserData->m_id);
			break;
	}

	/*	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, command - 0x10, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	pUser->Send( sendBuffer, sendIndex);
*/
	//TRACE(_T("RecvModifyFame - command=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, pTUser->GetSocketID(), pTUser->m_pUserData->m_id, knightsindex, pTUser->m_pUserData->m_bFame);

	if (pTUser != nullptr)
	{
		//TRACE(_T("RecvModifyFame - command=%d, nid=%d, name=%hs, index=%d, fame=%d\n"), command, pTUser->GetSocketID(), pTUser->m_pUserData->m_id, knightsindex, pTUser->m_pUserData->m_bFame);
		memset(sendBuffer, 0, sizeof(sendBuffer));
		sendIndex = 0;
		SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
		SetByte(sendBuffer, KNIGHTS_MODIFY_FAME, sendIndex);
		SetByte(sendBuffer, 0x01, sendIndex);

		if (command == DB_KNIGHTS_REMOVE)
		{
			SetShort(sendBuffer, pTUser->GetSocketID(), sendIndex);
			SetShort(sendBuffer, pTUser->m_pUserData->m_bKnights, sendIndex);
			SetByte(sendBuffer, pTUser->m_pUserData->m_bFame, sendIndex);
			m_pMain->Send_Region(sendBuffer, sendIndex, pTUser->m_pUserData->m_bZone,
				pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
		}
		else
		{
			SetShort(sendBuffer, pTUser->GetSocketID(), sendIndex);
			SetShort(sendBuffer, pTUser->m_pUserData->m_bKnights, sendIndex);
			SetByte(sendBuffer, pTUser->m_pUserData->m_bFame, sendIndex);
			pTUser->Send(sendBuffer, sendIndex);
		}

		if (command == DB_KNIGHTS_REMOVE)
		{
			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, WIZ_CHAT, sendIndex);
			SetByte(sendBuffer, KNIGHTS_CHAT, sendIndex);
			SetByte(sendBuffer, 1, sendIndex);
			SetShort(sendBuffer, -1, sendIndex);
			SetByte(sendBuffer, 0, sendIndex); // sender name length
			SetString2(sendBuffer, finalstr, sendIndex);
			pTUser->Send(sendBuffer, sendIndex);
		}
	}

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_CHAT, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CHAT, sendIndex);
	SetByte(sendBuffer, 1, sendIndex);
	SetShort(sendBuffer, -1, sendIndex);
	SetByte(sendBuffer, 0, sendIndex); // sender name length
	SetString2(sendBuffer, finalstr, sendIndex);
	m_pMain->Send_KnightsMember(knightsindex, sendBuffer, sendIndex);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, UDP_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, command - 0x10, sendIndex);
	SetShort(sendBuffer, knightsindex, sendIndex);
	SetString2(sendBuffer, userid, sendIndex);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(sendBuffer, sendIndex);
	else
		m_pMain->Send_UDP_All(sendBuffer, sendIndex, 1);
}

void CKnightsManager::RecvDestroyKnights(CUser* pUser, const char* pBuf)
{
	CKnights* pKnights = nullptr;
	int sendIndex = 0, knightsindex = 0, index = 0, flag = 0;
	char sendBuffer[128] {};
	std::string finalstr;

	if (pUser == nullptr)
		return;

	knightsindex = GetShort(pBuf, index);

	pKnights     = m_pMain->m_KnightsMap.GetData(knightsindex);
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
		finalstr = fmt::format_db_resource(IDS_KNIGHTS_DESTROY, pKnights->m_strName);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_CHAT, sendIndex);
	SetByte(sendBuffer, KNIGHTS_CHAT, sendIndex);
	SetByte(sendBuffer, 1, sendIndex);
	SetShort(sendBuffer, -1, sendIndex);
	SetByte(sendBuffer, 0, sendIndex); // sender name length
	SetString2(sendBuffer, finalstr, sendIndex);
	m_pMain->Send_KnightsMember(knightsindex, sendBuffer, sendIndex);

	int socketCount = m_pMain->GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pTUser = m_pMain->GetUserPtrUnchecked(i);
		if (pTUser == nullptr)
			continue;

		if (pTUser->m_pUserData->m_bKnights == knightsindex)
		{
			pTUser->m_pUserData->m_bKnights = 0;
			pTUser->m_pUserData->m_bFame    = 0;

			RemoveKnightsUser(knightsindex, pTUser->m_pUserData->m_id);

			memset(sendBuffer, 0, sizeof(sendBuffer));
			sendIndex = 0;
			SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
			SetByte(sendBuffer, KNIGHTS_MODIFY_FAME, sendIndex);
			SetByte(sendBuffer, 0x01, sendIndex);
			SetShort(sendBuffer, pTUser->GetSocketID(), sendIndex);
			SetShort(sendBuffer, pTUser->m_pUserData->m_bKnights, sendIndex);
			SetByte(sendBuffer, pTUser->m_pUserData->m_bFame, sendIndex);
			m_pMain->Send_Region(sendBuffer, sendIndex, pTUser->m_pUserData->m_bZone,
				pTUser->m_RegionX, pTUser->m_RegionZ, nullptr, false);
			//pTUser->Send( sendBuffer, sendIndex );
		}
	}

	m_pMain->m_KnightsMap.DeleteData(knightsindex);
	//TRACE(_T("RecvDestoryKnights - nid=%d, name=%hs, index=%d, fame=%d\n"), pUser->GetSocketID(), pUser->m_pUserData->m_id, knightsindex, pUser->m_pUserData->m_bFame);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_DESTROY, sendIndex);
	SetByte(sendBuffer, 0x01, sendIndex);
	pUser->Send(sendBuffer, sendIndex);

	memset(sendBuffer, 0, sizeof(sendBuffer));
	sendIndex = 0;
	SetByte(sendBuffer, UDP_KNIGHTS_PROCESS, sendIndex);
	SetByte(sendBuffer, KNIGHTS_DESTROY, sendIndex);
	SetShort(sendBuffer, knightsindex, sendIndex);
	if (m_pMain->m_nServerGroup == 0)
		m_pMain->Send_UDP_All(sendBuffer, sendIndex);
	else
		m_pMain->Send_UDP_All(sendBuffer, sendIndex, 1);

	//if( flag == KNIGHTS_TYPE )	{
	/*	memset( sendBuffer, 0x00, 128 ); sendIndex = 0;
		SetByte( sendBuffer, WIZ_KNIGHTS_LIST, sendIndex );
		SetByte( sendBuffer, 0x03, sendIndex );					// Knights Remove From List
		SetShort( sendBuffer, knightsindex, sendIndex );
		m_pMain->Send_All( sendBuffer, sendIndex, pUser );	*/
	//}
}

void CKnightsManager::RecvKnightsList(const char* pBuf)
{
	CKnights* pKnights = nullptr;
	int nation = 0, members = 0, index = 0, iLength = 0, knightsindex = 0, points = 0, ranking = 0;
	char knightsname[MAX_ID_SIZE + 1] {};

	knightsindex = GetShort(pBuf, index);
	nation       = GetByte(pBuf, index);
	iLength      = GetShort(pBuf, index);
	GetString(knightsname, pBuf, iLength, index);
	members = GetShort(pBuf, index);
	points  = GetDWORD(pBuf, index); // knights grade
	ranking = GetByte(pBuf, index);

	if (m_pMain->m_nServerNo == SERVER_ZONE_BATTLE)
	{
		pKnights = m_pMain->m_KnightsMap.GetData(knightsindex);
		if (pKnights != nullptr)
		{
			pKnights->m_sIndex   = knightsindex;
			pKnights->m_byNation = nation;
			strcpy_safe(pKnights->m_strName, knightsname);
			pKnights->m_sMembers  = members;
			pKnights->m_nPoints   = points;
			pKnights->m_byGrade   = m_pMain->GetKnightsGrade(points);
			pKnights->m_byRanking = ranking;
		}
		else
		{
			pKnights             = new CKnights();
			pKnights->m_sIndex   = knightsindex;
			pKnights->m_byNation = nation;
			strcpy_safe(pKnights->m_strName, knightsname);
			pKnights->m_sMembers = members;
			strcpy_safe(pKnights->m_strChief, "");
			strcpy_safe(pKnights->m_strViceChief_1, "");
			strcpy_safe(pKnights->m_strViceChief_2, "");
			strcpy_safe(pKnights->m_strViceChief_3, "");
			pKnights->m_nMoney      = 0;
			pKnights->m_sDomination = 0;
			pKnights->m_nPoints     = points;
			pKnights->m_byGrade     = m_pMain->GetKnightsGrade(points);
			pKnights->m_byRanking   = ranking;

			if (!m_pMain->m_KnightsMap.PutData(pKnights->m_sIndex, pKnights))
			{
				spdlog::error(
					"KnightsManager::RecvKnightsList: KnightsMap put failed [knightsId={}]",
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
		spdlog::error("KnightsManager::AddKnightsUser: knightsId={} not found", knightsId);
		return false;
	}

	for (int i = 0; i < MAX_CLAN; i++)
	{
		if (pKnights->m_arKnightsUser[i].byUsed != 0)
			continue;

		pKnights->m_arKnightsUser[i].byUsed = 1;
		strcpy_safe(pKnights->m_arKnightsUser[i].strUserName, charId);
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
		spdlog::error("KnightsManager::ModifyKnightsUser: knightsId={} not found", knightsId);
		return false;
	}

	for (int i = 0; i < MAX_CLAN; i++)
	{
		if (pKnights->m_arKnightsUser[i].byUsed == 0)
			continue;

		if (strcmp(pKnights->m_arKnightsUser[i].strUserName, charId) == 0)
		{
			pKnights->m_arKnightsUser[i].byUsed = 1;
			strcpy_safe(pKnights->m_arKnightsUser[i].strUserName, charId);
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
		spdlog::error("KnightsManager::RemoveKnightsUser: knightsId={} not found", knightsId);
		return false;
	}

	for (int i = 0; i < MAX_CLAN; i++)
	{
		if (pKnights->m_arKnightsUser[i].byUsed == 0)
			continue;

		if (strcmp(pKnights->m_arKnightsUser[i].strUserName, charId) == 0)
		{
			pKnights->m_arKnightsUser[i].byUsed = 0;
			strcpy_safe(pKnights->m_arKnightsUser[i].strUserName, "");
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
		spdlog::error("KnightsManager::SetKnightsUser: knightsId={} not found", knightsId);
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

void CKnightsManager::RecvKnightsAllList(const char* pBuf)
{
	CKnights* pKnights = nullptr;
	int index = 0, knightsId = 0, points = 0, count = 0, ranking = 0;
	int sendIndex = 0, temp_index = 0, send_count = 0;
	char sendBuffer[512] {}, temp_buff[512] {};

	count = GetByte(pBuf, index);

	for (int i = 0; i < count; i++)
	{
		knightsId = GetShort(pBuf, index);
		points    = GetDWORD(pBuf, index);
		ranking   = GetByte(pBuf, index);

		pKnights  = m_pMain->m_KnightsMap.GetData(knightsId);
		if (pKnights == nullptr)
		{
			spdlog::error("KnightsManager::RecvKnightsAllList: knightsId={} not found", knightsId);
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
		SetByte(sendBuffer, WIZ_KNIGHTS_PROCESS, sendIndex);
		SetByte(sendBuffer, KNIGHTS_ALLLIST_REQ, sendIndex);
		SetShort(sendBuffer, send_count, sendIndex);
		SetString(sendBuffer, temp_buff, temp_index, sendIndex);
		m_pMain->Send_All(sendBuffer, sendIndex, nullptr);
	}
}

} // namespace Ebenezer
