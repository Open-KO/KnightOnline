// UIMsgBoxOkCancel.cpp: implementation of the CUIMsgBoxOkCancel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIMsgBoxOkCancel.h"
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include "GameProcMain.h"
#include "UIManager.h"
#include "UITransactionDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIMsgBoxOkCancel::CUIMsgBoxOkCancel()
{
	m_eCallerWnd = UIWND_UNKNOWN;
	m_eCallerWndDistrict = UIWND_DISTRICT_UNKNOWN;
	m_pBtn_OK = nullptr;
	m_pBtn_Cancel = nullptr;
	m_pText_Msg = nullptr;
	m_bLocked = false;
}

CUIMsgBoxOkCancel::~CUIMsgBoxOkCancel()
{

}

void CUIMsgBoxOkCancel::Release()
{
	CN3UIBase::Release();
}

bool CUIMsgBoxOkCancel::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_OK, (CN3UIButton*) GetChildByID("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, (CN3UIButton*) GetChildByID("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pText_Msg, (CN3UIString*) GetChildByID("text_msg"));

	return true;
}

void CUIMsgBoxOkCancel::SetText(const std::string& szMsg)
{
	m_pText_Msg->SetString(szMsg);
}

void CUIMsgBoxOkCancel::Open(e_UIWND eUW, e_UIWND_DISTRICT eUD)
{
	m_bLocked = true;

	SetVisible(true);
	
	m_eCallerWnd = eUW;
	m_eCallerWndDistrict = eUD;

	RECT rcParent, rcMsgBox;
	int iParentW = 0, iParentH = 0, iParentX = 0, iParentY = 0;

	//this ui region
	rcMsgBox = GetRegion();
	int iMsgW = rcMsgBox.right - rcMsgBox.left;
	int iMsgH = rcMsgBox.bottom - rcMsgBox.top;

	int iX = 0, iY = 0;

	switch (eUW)
	{
		case UIWND_TRANSACTION:
			rcParent = CGameProcedure::s_pProcMain->m_pUITransactionDlg->GetRegion();
			break;
		default:
			rcParent = CGameProcedure::s_pProcMain->m_pUITransactionDlg->GetRegion();
			break;
		//note: this can be used inside other UI files in the future
	}

	iParentW = rcParent.right - rcParent.left;
	iParentH = rcParent.bottom - rcParent.top;
	iParentX = rcParent.left;
	iParentY = rcParent.top;

	if (eUW == UIWND_TRANSACTION)
	{
		iX = iParentX + (iParentW - iMsgW) / 2;
		//Note: this is close to correct position, but better calc. is needed
		iY = iParentY + (iParentH / 2) - iMsgH - 30; 
	}

	SetPos(iX, iY);

}

void CUIMsgBoxOkCancel::Close()
{
	m_bLocked = false;

	SetVisibleWithNoSound(false); 
}

bool CUIMsgBoxOkCancel::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr) return false;
	if (IsVisible() == false) return false;
	if (m_eCallerWnd == UIWND_UNKNOWN) return false;
	if (m_eCallerWndDistrict == UIWND_DISTRICT_UNKNOWN) return false;
	
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Cancel)
		{
			
			switch (m_eCallerWnd)
			{
				case UIWND_TRANSACTION:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_TRADE_NPC:
						case UIWND_DISTRICT_TRADE_MY:
							CGameProcedure::s_pProcMain->m_pUITransactionDlg->MsgBoxCancel();
							break;
					}
					break;
			}

		}
		else if (pSender == m_pBtn_OK)
		{

			switch (m_eCallerWnd)
			{
				case UIWND_TRANSACTION:
					switch (m_eCallerWndDistrict)
					{
						case UIWND_DISTRICT_TRADE_NPC:
						case UIWND_DISTRICT_TRADE_MY:
							CGameProcedure::s_pProcMain->m_pUITransactionDlg->MsgBoxOK();
							break;
					}
					break;
			}

		}
	}
	
}

void CUIMsgBoxOkCancel::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}

void CUIMsgBoxOkCancel::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{
	if (bWork)
	{
		ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
	}

	CN3UIBase::SetVisibleWithNoSound(bVisible, bWork, bReFocus);
}
