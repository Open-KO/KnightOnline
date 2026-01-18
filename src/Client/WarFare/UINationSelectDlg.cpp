// UINationSelectDlg.cpp: implementation of the CUINationSelectDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UINationSelectDlg.h"
#include "GameProcNationSelect.h"

CUINationSelectDlg::CUINationSelectDlg()
{
	m_pProcNationSelectRef = nullptr;
	m_pBtnKarus            = nullptr;
	m_pBtnElmorad          = nullptr;
	m_pBtnBack             = nullptr;
}

CUINationSelectDlg::~CUINationSelectDlg()
{
}

void CUINationSelectDlg::Release()
{
	CN3UIBase::Release();

	m_pBtnKarus   = nullptr;
	m_pBtnElmorad = nullptr;
	m_pBtnBack    = nullptr;
}

bool CUINationSelectDlg::Load(File& file)
{
	bool bSuccess = CN3UIBase::Load(file);

	N3_VERIFY_UI_COMPONENT(m_pBtnKarus, GetChildByID("btn_karus_selection"));
	N3_VERIFY_UI_COMPONENT(m_pBtnElmorad, GetChildByID("btn_elmo_selection")); //
	N3_VERIFY_UI_COMPONENT(m_pBtnBack, GetChildByID("btn_back"));              //
	RECT rc = this->GetRegion();
	int iX  = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	int iY  = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	this->SetPos(iX, iY);

	return bSuccess;
}

bool CUINationSelectDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (nullptr == pSender)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtnKarus) // Karus
		{
			if (m_pProcNationSelectRef)
				m_pProcNationSelectRef->MsgSendNationSelect(NATION_KARUS);
		}
		else if (pSender == m_pBtnElmorad) // Elmorad
		{
			if (m_pProcNationSelectRef)
				m_pProcNationSelectRef->MsgSendNationSelect(NATION_ELMORAD);
		}
		else if (pSender == m_pBtnBack)                                                    // Back
		{
			CGameProcedure::ProcActiveSet((CGameProcedure*) CGameProcedure::s_pProcLogIn); // 캐릭터 선택 프로시저로 한다..
		}
	}

	return true;
}

