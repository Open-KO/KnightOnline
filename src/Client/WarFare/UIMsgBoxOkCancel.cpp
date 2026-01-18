// UIMsgBoxOkCancel.cpp: implementation of the CUIMsgBoxOkCancel class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIMsgBoxOkCancel.h"
#include "LocalInput.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>

CUIMsgBoxOkCancel::CUIMsgBoxOkCancel()
{
	m_pBtn_OK     = nullptr;
	m_pBtn_Cancel = nullptr;
	m_pText_Msg   = nullptr;
}

CUIMsgBoxOkCancel::~CUIMsgBoxOkCancel()
{
}

void CUIMsgBoxOkCancel::Release()
{
	CN3UIBase::Release();

	m_pBtn_OK     = nullptr;
	m_pBtn_Cancel = nullptr;
	m_pText_Msg   = nullptr;
}

bool CUIMsgBoxOkCancel::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_OK, GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("btn_cancel"));
	N3_VERIFY_UI_COMPONENT(m_pText_Msg, GetChildByID<CN3UIString>("text_msg"));

	return true;
}

void CUIMsgBoxOkCancel::SetText(const std::string& szMsg)
{
	if (m_pText_Msg != nullptr)
		m_pText_Msg->SetString(szMsg);
}

bool CUIMsgBoxOkCancel::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_OK)
		{
			if (m_pParentUI != nullptr)
				m_pParentUI->CallBackProc(m_iChildID, CALLBACK_OK);

			SetVisible(false);
			return true;
		}

		if (pSender == m_pBtn_Cancel)
		{
			if (m_pParentUI != nullptr)
				m_pParentUI->CallBackProc(m_iChildID, CALLBACK_CANCEL);

			SetVisible(false);
			return true;
		}
	}

	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}

bool CUIMsgBoxOkCancel::OnKeyPress(int iKey)
{
	if (iKey == DIK_ESCAPE)
	{
		ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
