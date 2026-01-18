// UIKnightsOperation.cpp: implementation of the CUIKnightsOperation class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIKnightsOperation.h"
#include "GameProcMain.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIList.h>
#include <N3Base/N3UIScrollBar.h>

CUIKnightsOperation::CUIKnightsOperation()
{
	m_iPageCur          = 0;

	m_pBtn_Up           = nullptr;
	m_pBtn_Down         = nullptr;
	m_pBtn_Close        = nullptr;

	m_pList_Knights     = nullptr;
	m_pBtn_Join         = nullptr;
	m_pBtn_Create       = nullptr;
	m_pBtn_Destroy      = nullptr;
	m_pBtn_Withdraw     = nullptr;
	m_pEdit_KnightsName = nullptr;
}

CUIKnightsOperation::~CUIKnightsOperation()
{
}

void CUIKnightsOperation::Release()
{
	CN3UIBase::Release();

	m_iPageCur = 0;
	m_KnightsListExt.clear();

	m_pBtn_Up           = nullptr;
	m_pBtn_Down         = nullptr;
	m_pBtn_Close        = nullptr;

	m_pList_Knights     = nullptr;
	m_pBtn_Join         = nullptr;
	m_pBtn_Create       = nullptr;
	m_pBtn_Destroy      = nullptr;
	m_pBtn_Withdraw     = nullptr;
	m_pEdit_KnightsName = nullptr;
}

bool CUIKnightsOperation::Load(File& file)
{
	if (false == CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Up, GetChildByID<CN3UIButton>("btn_up"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Down, GetChildByID<CN3UIButton>("btn_down"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Close, GetChildByID<CN3UIButton>("btn_close"));

	N3_VERIFY_UI_COMPONENT(m_pList_Knights, GetChildByID<CN3UIList>("List_Knights"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Join, GetChildByID<CN3UIButton>("Btn_Join"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Create, GetChildByID<CN3UIButton>("Btn_Create"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Destroy, GetChildByID<CN3UIButton>("Btn_Destroy"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Withdraw, GetChildByID<CN3UIButton>("Btn_Withdraw"));
	N3_VERIFY_UI_COMPONENT(m_pEdit_KnightsName, GetChildByID<CN3UIEdit>("Edit_KnightsName"));

	return true;
}

bool CUIKnightsOperation::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		POINT pt;
		::GetCursorPos(&pt);

		if (pSender == m_pBtn_Up)
		{
			m_iPageCur--;
			if (m_iPageCur < 0)
			{
				m_iPageCur = 0;
			}
			else
			{
				this->MsgSend_KnightsList(m_iPageCur);
			}
		}
		else if (pSender == m_pBtn_Close)
		{
			this->Close();
		}
		else if (pSender == m_pBtn_Create)
		{
			this->MsgSend_KnightsCreate();
		}
		else if (pSender == m_pBtn_Join) // 가입
		{
			this->MsgSend_KnightsJoin();
		}
		else if (pSender == m_pBtn_Destroy)                                                 // 탈퇴
		{
			std::string szMsg = fmt::format_text_resource(IDS_KNIGHTS_DESTROY_CONFIRM);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_KNIGHTS_DESTROY);  // 기사단 해체 물어보기..
		}
		else if (pSender == m_pBtn_Withdraw)                                                // 탈퇴
		{
			std::string szMsg = fmt::format_text_resource(IDS_KNIGHTS_WITHDRAW_CONFIRM);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_KNIGHTS_WITHDRAW); // 기사단 탈퇴 물어보기..
		}
	}

	return false;
}

void CUIKnightsOperation::KnightsInfoDelete(int iID)
{
	it_KIB it = m_KnightsMapBase.find(iID);
	if (it != m_KnightsMapBase.end())
		m_KnightsMapBase.erase(it);
}

void CUIKnightsOperation::KnightsInfoInsert(int iID, const std::string& szName)
{
	__KnightsInfoBase KIB;
	KIB.iID    = iID;
	KIB.szName = szName;

	m_KnightsMapBase.insert(val_KIB(iID, KIB));
}

__KnightsInfoBase* CUIKnightsOperation::KnightsInfoFind(int iID)
{
	it_KIB it = m_KnightsMapBase.find(iID);
	if (it != m_KnightsMapBase.end())
		return &(it->second);
	return nullptr;
}

void CUIKnightsOperation::KnightsListAdd(int iID, const std::string& szName, const std::string& szChiefName, int iMemberCount, int iPoint)
{
	__KnightsInfoExt KIE;
	KIE.iID          = iID;
	KIE.szName       = szName;
	KIE.szChiefName  = szChiefName;
	KIE.iMemberCount = iMemberCount;
	KIE.iPoint       = iPoint;

	m_KnightsListExt.push_back(KIE);
}

void CUIKnightsOperation::KnightsListUpdate()
{
	if (nullptr == m_pList_Knights)
		return;

	m_pList_Knights->ResetContent();

	std::string szBuff;
	for (const __KnightsInfoExt& KIE : m_KnightsListExt)
	{
		szBuff = fmt::format("{:16} {:12} {:4} {:8}", KIE.szName, KIE.szChiefName, KIE.iMemberCount, KIE.iPoint);
		m_pList_Knights->AddString(szBuff);
	}
}

void CUIKnightsOperation::KnightsListClear()
{
	m_KnightsListExt.clear();
	if (m_pList_Knights)
		m_pList_Knights->ResetContent();
}

void CUIKnightsOperation::ChangeUIByDuty(e_KnightsDuty eDuty)
{
	if (eDuty == KNIGHTS_DUTY_CHIEF) // 기사단장이면 기사단 해체도 가능하다..
	{
		if (m_pBtn_Destroy)
			m_pBtn_Destroy->SetState(UI_STATE_BUTTON_NORMAL);
		if (m_pBtn_Withdraw)
			m_pBtn_Withdraw->SetState(UI_STATE_BUTTON_DISABLE); // 기사 단장이 탈퇴한다????!!!!
		if (m_pBtn_Join)
			m_pBtn_Join->SetState(UI_STATE_BUTTON_DISABLE);
	}
	else
	{
		if (m_pBtn_Destroy)
			m_pBtn_Destroy->SetState(UI_STATE_BUTTON_DISABLE);
		if (m_pBtn_Withdraw)
			m_pBtn_Withdraw->SetState(UI_STATE_BUTTON_NORMAL); // 기사 단장이 탈퇴한다????!!!!
		if (m_pBtn_Join)
			m_pBtn_Join->SetState(UI_STATE_BUTTON_NORMAL);
	}
}

void CUIKnightsOperation::Open(e_KnightsDuty eDuty)
{
	m_iPageCur = 0;
	this->KnightsListClear();    // 기사단 정보 클리어
	this->SetPosCenter();        // 가운데로 맞추고..
	this->SetVisible(true);

	this->ChangeUIByDuty(eDuty); // 권한에 따라 UI 변경..
}

void CUIKnightsOperation::Close()
{
	this->KnightsListClear();             // 기사단 정보 클리어
	this->SetVisible(false);
	if (m_pEdit_KnightsName)
		m_pEdit_KnightsName->KillFocus(); // 이래야 다른곳에 문제가 안생긴다..
}

bool CUIKnightsOperation::MsgRecv_KnightsList(Packet& pkt)
{
	std::string szName, szChiefName;
	int iID = 0, iNameLength = 0, iMemberCount = 0, iPoint = 0, iKC = 0;

	m_iPageCur = pkt.read<int16_t>();
	iKC        = pkt.read<int16_t>();
	for (int i = 0; i < iKC; i++)
	{
		iID         = pkt.read<int16_t>();
		iNameLength = pkt.read<int16_t>();
		pkt.readString(szName, iNameLength);
		iMemberCount = pkt.read<int16_t>();
		iNameLength  = pkt.read<int16_t>();
		pkt.readString(szChiefName, iNameLength);
		iPoint = pkt.read<uint32_t>();

		KnightsListAdd(iID, szName, szChiefName, iMemberCount, iPoint); // UI 에 추가..
	}

	KnightsListUpdate();                                                // List 에 다 넣었으면 UI Update!!
	EnableKnightsUIs(true);                                             // Disable 된 버튼들 Enable 시킨다.

	return true;
}

void CUIKnightsOperation::MsgSend_KnightsCreate()
{
	if (nullptr == m_pEdit_KnightsName)
		return;

	std::string szKnightsName = m_pEdit_KnightsName->GetString();
	if (szKnightsName.empty()) // 이름이 없으면 에러..
	{
		std::string szMsg = fmt::format_text_resource(IDS_ERR_KNIGHTS_CREATE_FAILED_NAME_EMPTY);
		CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff00ff);
		return;
	}

	int iOffset = 0;
	uint8_t byBuff[128];

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_KNIGHTS_CREATE); // 생성 Send - s1(Name Length) str1 | Recv - b1(1:성공 0:실패)
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) szKnightsName.size());
	CAPISocket::MP_AddString(byBuff, iOffset, szKnightsName);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIKnightsOperation::MsgSend_KnightsDestroy()
{
	int iOffset = 0;
	uint8_t byBuff[8];

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_KNIGHTS_DESTROY); // 생성 Send - s1(Name Length) str1 | Recv - b1(1:성공 0:실패)

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIKnightsOperation::MsgSend_KnightsJoin()
{
	if (nullptr == m_pList_Knights)
		return;
	int iCurSel = m_pList_Knights->GetCurSel();
	if (iCurSel < 0 || iCurSel >= static_cast<int>(m_KnightsListExt.size()))
		return;

	auto it = m_KnightsListExt.begin();
	std::advance(it, iCurSel);

	__KnightsInfoExt KIE = (*it);

	int iOffset          = 0;
	uint8_t byBuff[32];

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_KNIGHTS_JOIN);
	CAPISocket::MP_AddShort(byBuff, iOffset, KIE.iID);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIKnightsOperation::MsgSend_KnightsWithdraw()
{
	uint8_t byBuff[32];
	int iOffset = 0;

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_KNIGHTS_WITHDRAW);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIKnightsOperation::MsgSend_KnightsList(int iPage)
{
	int iOffset = 0;
	uint8_t byBuff[32];

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_KNIGHTS_GRADE_CHANGE_ALL);
	CAPISocket::MP_AddShort(byBuff, iOffset, iPage);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	// 페이지를 넘길때는 버튼들을 막아 놓는다.
	this->EnableKnightsUIs(false);
}

void CUIKnightsOperation::EnableKnightsUIs(bool bEnable)
{
	eUI_STATE eState = UI_STATE_BUTTON_NORMAL;
	if (bEnable)
		eState = UI_STATE_BUTTON_NORMAL;
	else
		eState = UI_STATE_BUTTON_DISABLE;

	if (m_pBtn_Up)
		m_pBtn_Up->SetState(eState);
	if (m_pBtn_Down)
		m_pBtn_Down->SetState(eState);
	if (m_pBtn_Close)
		m_pBtn_Close->SetState(eState);

	if (m_pList_Knights)
		m_pList_Knights->SetState(eState);
	if (m_pBtn_Join)
		m_pBtn_Join->SetState(eState);
	if (m_pBtn_Create)
		m_pBtn_Create->SetState(eState);
	if (m_pBtn_Withdraw)
		m_pBtn_Withdraw->SetState(eState);
	if (m_pEdit_KnightsName)
		m_pEdit_KnightsName->SetState(eState);
}
