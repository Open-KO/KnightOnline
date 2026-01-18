// N3UIArea.cpp: implementation of the CN3UIArea class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIArea.h"
#include "N3UIEdit.h"

CN3UIArea::CN3UIArea()
{
	m_eType     = UI_TYPE_AREA;
	m_eAreaType = UI_AREA_TYPE_NONE;
}

CN3UIArea::~CN3UIArea()
{
}

void CN3UIArea::Release()
{
	CN3UIBase::Release();
}

void CN3UIArea::SetRegion(const RECT& Rect)
{
	CN3UIBase::SetRegion(Rect);
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		(*itor)->SetRegion(Rect);
	}
}

bool CN3UIArea::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

#ifndef _REPENT
	// 추가사항이 있으면 이곳에 추가하기
	int iAreaType = 0;
	file.Read(&iAreaType, sizeof(int)); // click 영역
	m_eAreaType = (eUI_AREA_TYPE) iAreaType;
#endif
	return true;
}

uint32_t CN3UIArea::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible)
		return dwRet;

#ifndef _REPENT
#ifdef _N3GAME
	if (s_bWaitFromServer)
		return dwRet;
#endif
#endif

	// 특정 이벤트에 대해 메시지 전송..
	if (IsIn(ptCur.x, ptCur.y) && (dwFlags & UI_MOUSE_LBCLICK))
	{
		m_pParent->ReceiveMessage(this, UIMSG_BUTTON_CLICK); // 부모에게 버튼 클릭 통지..
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
	}

	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

#ifndef _REPENT
#ifdef _N3GAME
bool CN3UIArea::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (s_bWaitFromServer)
		return false;

	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}
#endif
#endif

#ifdef _N3TOOL
bool CN3UIArea::Save(File& file)
{
	if (false == CN3UIBase::Save(file))
		return false;
#ifndef _REPENT
	int iAreaType = (int) m_eAreaType;
	file.Write(&iAreaType, sizeof(int)); // click 영역
#endif
	return true;
}

CN3UIArea& CN3UIArea::operator=(const CN3UIArea& other)
{
	if (this == &other)
		return *this;

	CN3UIBase::operator=(other);
#ifndef _REPENT
	m_eAreaType = other.m_eAreaType;
#endif

	return *this;
}
#endif

