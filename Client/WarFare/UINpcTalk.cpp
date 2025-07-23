// UINpcTalk.cpp: implementation of the CUINpcTalk class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "text_resources.h"
#include "UINpcTalk.h"

#include "GameProcMain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUINpcTalk::CUINpcTalk()
{

}

CUINpcTalk::~CUINpcTalk()
{

}

void CUINpcTalk::Release()
{
	CN3UIBase::Release();
}

bool CUINpcTalk::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	return true;
}

bool CUINpcTalk::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)					
	{
		if(pSender->m_szID == "btn_0")
		{
//			CGameProcedure::s_pProcMain->m_pUITradeList->Open(m_iIDTarget);
			Close();
		}

		if(pSender->m_szID == "btn_1")
		{
//			CGameProcedure::s_pProcMain->m_pUITradeList->Open(m_iIDTarget);
			Close();
		}

		if(pSender->m_szID == "btn_2")
		{
//			CGameProcedure::s_pProcMain->m_pUITradeList->Open(m_iIDTarget);
			Close();
		}

		if(pSender->m_szID == "btn_close")
			Close();
	}

	return true;
}

void CUINpcTalk::Open(int iIDTarget)
{
	SetVisible(true);

	m_iIDTarget = iIDTarget;
}

void CUINpcTalk::Close()
{
	SetVisible(false);
}
