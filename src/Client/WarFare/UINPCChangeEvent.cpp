// UINPCChangeEvent.cpp: implementation of the CUINPCChangeEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UINPCChangeEvent.h"
#include "PacketDef.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "APISocket.h"
#include "UIInventory.h"
#include "UIManager.h"
#include "UIPointInitDlg.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>

CUINPCChangeEvent::CUINPCChangeEvent()
{
	m_pBtn_Repoint0   = nullptr;
	m_pBtn_Repoint1   = nullptr;
	m_pBtn_Close      = nullptr;

	m_pDlg            = nullptr;

	m_bSendedAllPoint = false;
}

CUINPCChangeEvent::~CUINPCChangeEvent()
{
	delete m_pDlg;
	m_pDlg = nullptr;
}

void CUINPCChangeEvent::Release()
{
	if (m_pDlg != nullptr)
	{
		m_pDlg->Release();
		delete m_pDlg;
		m_pDlg = nullptr;
	}

	CN3UIBase::Release();
}

bool CUINPCChangeEvent::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	int iW = CN3Base::s_CameraData.vp.Width;
	int iH = CN3Base::s_CameraData.vp.Height;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Repoint0, GetChildByID<CN3UIButton>("Btn_repoint0"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Repoint1, GetChildByID<CN3UIButton>("Btn_repoint1"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Close, GetChildByID<CN3UIButton>("Btn_close"));

	// UIPointInitDlg.. ^^
	e_Nation eNation       = CGameBase::s_pPlayer->m_InfoBase.eNation; // 국가....
	__TABLE_UI_RESRC* pTbl = CGameProcedure::s_pTbl_UI.Find(eNation);

	m_pDlg                 = new CUIPointInitDlg();
	m_pDlg->LoadFromFile(pTbl->szChangeInitBill);

	// 위치 계산 ..
	int iXPos = 0, iYPos = 0;
	iXPos = (iW / 2) - (m_pDlg->GetRegion().right - m_pDlg->GetRegion().left) / 2;
	iYPos = (iH / 2) - (m_pDlg->GetRegion().bottom - m_pDlg->GetRegion().top) / 2;
	m_pDlg->SetPos(iXPos, iYPos);
	m_pDlg->Close();

	return true;
}

bool CUINPCChangeEvent::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Close)
		{
			Close();
		}
		else if (pSender == m_pBtn_Repoint0 && !s_bWaitFromServer)
		{
			CUIInventory* pInv = CGameProcedure::s_pProcMain->m_pUIInventory;
			if (pInv == nullptr)
			{
				Close();
				return true;
			}

			if (!pInv->HasAnyItemInSlot())
			{
				PointChangePriceQuery(true);
			}
			else
			{
				Close();

				std::string szMsg = fmt::format_text_resource(IDS_MSG_HASITEMINSLOT);
				CGameProcedure::s_pProcMain->MsgOutput(szMsg, 0xffff3b3b);
			}
		}
		else if (pSender == m_pBtn_Repoint1 && !s_bWaitFromServer)
		{
			PointChangePriceQuery(false);
		}
	}

	return true;
}

void CUINPCChangeEvent::Open()
{
	SetVisible(true);
}

void CUINPCChangeEvent::Close()
{
	SetVisible(false);
	if (m_pDlg != nullptr && m_pDlg->IsVisible())
		m_pDlg->Close();
}

void CUINPCChangeEvent::PointChangePriceQuery(bool bAllPoint)
{
	uint8_t byBuff[32];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_CLASS_CHANGE);
	CAPISocket::MP_AddByte(byBuff, iOffset, CLASS_RESET_COST_REQ);
	if (bAllPoint)
		CAPISocket::MP_AddByte(byBuff, iOffset, 1);
	else
		CAPISocket::MP_AddByte(byBuff, iOffset, 2);

	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
	m_bSendedAllPoint = bAllPoint;
}

void CUINPCChangeEvent::ReceivePriceFromServer(int iGold)
{
	if (m_pDlg != nullptr)
	{
		m_pDlg->ShowWindow(-1, this);
		m_pDlg->InitDlg(m_bSendedAllPoint, iGold);
	}
}

void CUINPCChangeEvent::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

bool CUINPCChangeEvent::OnKeyPress(int iKey)
{
	if (iKey == DIK_ESCAPE)
	{
		ReceiveMessage(m_pBtn_Close, UIMSG_BUTTON_CLICK);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
