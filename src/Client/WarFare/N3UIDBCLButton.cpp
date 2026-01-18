// N3UIDBCLButton.cpp: implementation of the CN3UIDBCLButton class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3UIDBCLButton.h"

CN3UIDBCLButton::CN3UIDBCLButton()
{
}

CN3UIDBCLButton::~CN3UIDBCLButton()
{
}

uint32_t CN3UIDBCLButton::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;

	RECT rect      = GetRegion();
	if (!::PtInRect(&rect, ptCur)) // 영역 밖이면
	{
		dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
		return dwRet;
	}

	if (dwFlags & UI_MOUSE_LBDBLCLK)
	{
		m_pParent->ReceiveMessage(this, UIMSG_ICON_DBLCLK); // 부모에게 버튼 클릭 통지..
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}
