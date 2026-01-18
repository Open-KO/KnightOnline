// N3UIScrollBar.cpp: implementation of the CN3UIScrollBar class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIScrollBar.h"
#include "N3UIButton.h"

CN3UIScrollBar::CN3UIScrollBar()
{
	m_eType        = UI_TYPE_SCROLLBAR;
	m_pTrackBarRef = nullptr;
	memset(&m_pBtnRef, 0, sizeof(CN3UIButton*) * NUM_BTN_TYPE);
	m_iLineSize = 1;
}

CN3UIScrollBar::~CN3UIScrollBar()
{
}

void CN3UIScrollBar::Release()
{
	CN3UIBase::Release();
	m_pTrackBarRef = nullptr;
	memset(&m_pBtnRef, 0, sizeof(CN3UIButton*) * NUM_BTN_TYPE);
	m_iLineSize = 1;
}

bool CN3UIScrollBar::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;
	__ASSERT(nullptr == m_pTrackBarRef, "scrollbar가 초기화되어 있지 않아여");

	// m_pTrackBarRef, m_pBtnRef  설정하기
	for (CN3UIBase* pChild : m_Children)
	{
		if (UI_TYPE_TRACKBAR == pChild->UIType())
		{
			m_pTrackBarRef = (CN3UITrackBar*) pChild;
		}
		else if (UI_TYPE_BUTTON == pChild->UIType())
		{
			int iBtnType = pChild->GetReserved();
			if (iBtnType < 0 || iBtnType >= NUM_BTN_TYPE)
				continue;

			m_pBtnRef[iBtnType] = (CN3UIButton*) pChild;
		}
	}
	return true;
}

void CN3UIScrollBar::SetRegion(const RECT& Rect)
{
	CN3UIBase::SetRegion(Rect);
	// 우선 임시로 스크롤 영역 크기와 같게 배치
	//	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	//	{
	//		(*itor)->SetRegion(Rect);
	//	}
}

bool CN3UIScrollBar::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (UIMSG_TRACKBAR_POS == dwMsg)
	{
		if (m_pParent)
			return m_pParent->ReceiveMessage(this, UIMSG_SCROLLBAR_POS);
	}
	else if (UIMSG_BUTTON_CLICK == dwMsg)
	{
		if (m_pTrackBarRef)
		{
			if (BTN_LEFTUP == pSender->GetReserved())
				m_pTrackBarRef->SetCurrentPos(m_pTrackBarRef->GetPos() - m_iLineSize);
			else if (BTN_RIGHTDOWN == pSender->GetReserved())
				m_pTrackBarRef->SetCurrentPos(m_pTrackBarRef->GetPos() + m_iLineSize);

			if (m_pParent)
				return m_pParent->ReceiveMessage(this, UIMSG_SCROLLBAR_POS);
		}
	}
	return true;
}

void CN3UIScrollBar::SetStyle(uint32_t dwStyle)
{
	CN3UIBase::SetStyle(dwStyle);
	if (UISTYLE_SCROLLBAR_HORIZONTAL == dwStyle)
	{
		if (m_pTrackBarRef)
			m_pTrackBarRef->SetStyle(UISTYLE_TRACKBAR_HORIZONTAL);
	}
	else
	{
		if (m_pTrackBarRef)
			m_pTrackBarRef->SetStyle(UISTYLE_TRACKBAR_VERTICAL);
	}
}

#ifdef _N3TOOL
CN3UIScrollBar& CN3UIScrollBar::operator=(const CN3UIScrollBar& other)
{
	if (this == &other)
		return *this;

	CN3UIBase::operator=(other);
	m_iLineSize = other.m_iLineSize; // 버튼을 눌렀을때 trackbar가 움직여지는 크기

	// m_pTrackBarRef, m_pBtnRef  설정하기
	for (CN3UIBase* pChild : m_Children)
	{
		if (UI_TYPE_TRACKBAR == pChild->UIType())
		{
			m_pTrackBarRef = (CN3UITrackBar*) pChild;
		}
		else if (UI_TYPE_BUTTON == pChild->UIType())
		{
			int iBtnType = pChild->GetReserved();
			if (iBtnType < 0 || iBtnType >= NUM_BTN_TYPE)
				continue;

			m_pBtnRef[iBtnType] = (CN3UIButton*) pChild;
		}
	}

	return *this;
}

void CN3UIScrollBar::CreateTrackBarAndBtns()
{
	__ASSERT(nullptr == m_pTrackBarRef, "구성요소가 이미 할당되어 있어요");
	for (int i = 0; i < NUM_BTN_TYPE; ++i)
	{
		m_pBtnRef[i] = new CN3UIButton();
		m_pBtnRef[i]->Init(this);
		m_pBtnRef[i]->SetReserved(i); // 상태 번호(eBTN_TYPE) 할당.
		m_pBtnRef[i]->CreateImages();
	}

	m_pTrackBarRef = new CN3UITrackBar();
	m_pTrackBarRef->Init(this);
	m_pTrackBarRef->CreateImages(); // trackbar의 이미지 생성
}
#endif
