// N3UITrackBar.cpp: implementation of the CN3UITrackBar class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UITrackBar.h"
#include "N3UIImage.h"

CN3UITrackBar::CN3UITrackBar()
{
	m_eType          = UI_TYPE_TRACKBAR;

	m_pBkGndImageRef = nullptr;
	m_pThumbImageRef = nullptr;
	m_iMaxPos        = 10;
	m_iMinPos        = 0;
	m_iCurPos        = 0;
	m_iPageSize      = 2;
}

CN3UITrackBar::~CN3UITrackBar()
{
}

void CN3UITrackBar::Release()
{
	CN3UIBase::Release();
	m_pBkGndImageRef = nullptr;
	m_pThumbImageRef = nullptr;
	m_iMaxPos        = 10;
	m_iMinPos        = 0;
	m_iCurPos        = 0;
	m_iPageSize      = 2;
}

bool CN3UITrackBar::Load(File& file)
{
	if (false == CN3UIBase::Load(file))
		return false;

	// ImageRef 설정하기
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (UI_TYPE_IMAGE != pChild->UIType())
			continue; // image만 골라내기
		int iImageType = (int) (pChild->GetReserved());
		if (IMAGETYPE_BKGND == iImageType)
		{
			m_pBkGndImageRef = (CN3UIImage*) pChild;
		}
		else if (IMAGETYPE_THUMB == iImageType)
		{
			m_pThumbImageRef = (CN3UIImage*) pChild;
		}
	}
	return true;
}

void CN3UITrackBar::SetRegion(const RECT& Rect)
{
	CN3UIBase::SetRegion(Rect);
	if (m_pBkGndImageRef)
		m_pBkGndImageRef->SetRegion(m_rcRegion); // 배경이미지는 같은 영역으로
	RECT rcThumb     = m_pThumbImageRef->GetRegion();

	int iThumbWidth  = rcThumb.right - rcThumb.left;
	int iThumbHeight = rcThumb.bottom - rcThumb.top;
	int iBkWidth     = Rect.right - Rect.left;
	int iBkHeight    = Rect.bottom - Rect.top;
	if (iBkWidth <= 0 || iBkHeight <= 0)
		return;

	if (iThumbWidth <= 0 && iThumbHeight <= 0)
	{     // thumb 이미지는 설정되어 있지 않다면 임으로 적당하게 설정
		RECT rc;
		if (UISTYLE_TRACKBAR_VERTICAL == m_dwStyle)
		{ // 세로
			rc.left   = Rect.left;
			rc.top    = Rect.top + (int) (iBkHeight * 0.3f);
			rc.right  = Rect.right;
			rc.bottom = rc.top + (int) (iBkHeight * 0.3f);
		}
		else
		{ // 가로
			rc.left   = Rect.left + (int) (iBkWidth * 0.3f);
			rc.top    = Rect.top;
			rc.right  = rc.left + (int) (iBkWidth * 0.3f);
			rc.bottom = Rect.bottom;
		}
		m_pThumbImageRef->SetRegion(rc);
	}
	UpdateThumbPos();
}

uint32_t CN3UITrackBar::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible)
		return dwRet;
	if (false == IsIn(ptCur.x, ptCur.y)) // 영역 밖이면
	{
		SetState(UI_STATE_COMMON_NONE);
		return dwRet;
	}
	dwRet |= UI_MOUSEPROC_INREGION; // 이번좌표가 영역 안이다.

	if (UI_STATE_TRACKBAR_THUMBDRAG == m_eState)
	{
		if (dwFlags & UI_MOUSE_LBCLICKED)   // 왼쪽버튼 떼는 순간
		{
			SetState(UI_STATE_COMMON_NONE); // drag 해제
			dwRet |= UI_MOUSEPROC_DONESOMETHING;
			return dwRet;
		}
		else if (dwFlags & UI_MOUSE_LBDOWN)
		{
			// thumb을 움직인다.
			if (UISTYLE_TRACKBAR_VERTICAL == m_dwStyle)
				UpDownThumbPos(ptCur.y - ptOld.y);
			else
				UpDownThumbPos(ptCur.x - ptOld.x);
			// 부모에게 메세지를 보내자
			if (m_pParent)
				m_pParent->ReceiveMessage(this, UIMSG_TRACKBAR_POS);
			dwRet |= UI_MOUSEPROC_DONESOMETHING;
			return dwRet;
		}
	}
	else
	{
		if (dwFlags & UI_MOUSE_LBCLICK)                   // 왼쪽버튼 눌르는 순간
		{
			if (m_pThumbImageRef->IsIn(ptCur.x, ptCur.y)) // thumb을 눌렀다.
			{
				SetState(UI_STATE_TRACKBAR_THUMBDRAG);
				dwRet |= UI_MOUSEPROC_DONESOMETHING;
				return dwRet;
			}
			else // thumb위부분 또는 아래 부분(좌 또는 우 여백)을 눌렀따.
			{
				RECT rcThumb = m_pThumbImageRef->GetRegion();
				if (UISTYLE_TRACKBAR_VERTICAL == m_dwStyle)
				{
					if (ptCur.y <= rcThumb.top)
						SetCurrentPos(m_iCurPos - m_iPageSize); // 윗부분 클릭
					else
						SetCurrentPos(m_iCurPos + m_iPageSize); // 아래 부분 클릭
				}
				else
				{
					if (ptCur.x <= rcThumb.left)
						SetCurrentPos(m_iCurPos - m_iPageSize); // 왼쪽 부분 클릭
					else
						SetCurrentPos(m_iCurPos + m_iPageSize); // 오른쪽 부분 클릭
				}
				if (m_pParent)
					m_pParent->ReceiveMessage(this, UIMSG_TRACKBAR_POS);
				dwRet |= UI_MOUSEPROC_DONESOMETHING;
				return dwRet;
			}
		}
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

//void CN3UITrackBar::Render()
//{
//	if(!m_bVisible) return;
//	if (m_pBkGndImageRef) m_pBkGndImageRef->Render();
//	if (m_pThumbImageRef) m_pThumbImageRef->Render();
//}

void CN3UITrackBar::SetRange(int iMin, int iMax)
{
	if (m_iMaxPos == iMax && m_iMinPos == iMin)
		return;
	m_iMaxPos = iMax;
	m_iMinPos = iMin;
	if (m_iCurPos > m_iMaxPos)
		m_iCurPos = m_iMaxPos;
	if (m_iCurPos < m_iMinPos)
		m_iCurPos = m_iMinPos;
	UpdateThumbPos();
}

void CN3UITrackBar::SetCurrentPos(int iPos)
{
	if (iPos == m_iCurPos)
		return;
	m_iCurPos = iPos;
	if (m_iCurPos > m_iMaxPos)
		m_iCurPos = m_iMaxPos;
	if (m_iCurPos < m_iMinPos)
		m_iCurPos = m_iMinPos;
	UpdateThumbPos();
}

// Pos수치로 Thumb의 위치를 조정
void CN3UITrackBar::UpdateThumbPos()
{
	if (nullptr == m_pThumbImageRef)
		return;
	float fDiff = (float) m_iMaxPos - (float) m_iMinPos;
	if (0.0f == fDiff)
		return;
	float fPercentage = (float) m_iCurPos / fDiff;
	RECT rcThumb      = m_pThumbImageRef->GetRegion();

	if (UISTYLE_TRACKBAR_VERTICAL == m_dwStyle)
	{
		int iDY = (int) (fPercentage
						 * ((m_rcRegion.bottom - m_rcRegion.top) - (rcThumb.bottom - rcThumb.top)));
		m_pThumbImageRef->SetPos(m_rcRegion.left, m_rcRegion.top + iDY);
	}
	else
	{
		int iDX = (int) (fPercentage
						 * ((m_rcRegion.right - m_rcRegion.left) - (rcThumb.right - rcThumb.left)));
		m_pThumbImageRef->SetPos(m_rcRegion.left + iDX, m_rcRegion.top);
	}
}

// thumb을 pixel단위로 위치 조정하고 thumb의 위치를 바탕으로 pos 수치를 계산하여 넣음
void CN3UITrackBar::UpDownThumbPos(int iDiff)
{
	if (nullptr == m_pThumbImageRef)
		return;
	RECT rcThumb = m_pThumbImageRef->GetRegion();

	if (UISTYLE_TRACKBAR_VERTICAL == m_dwStyle) // 아래 움직일 대
	{
		int iRegionHeight = m_rcRegion.bottom - m_rcRegion.top;
		int iThumbHeight  = rcThumb.bottom - rcThumb.top;
		if (0 == iRegionHeight || 0 == iThumbHeight)
			return;

		// 옮긴후 thumb의 위치 percentage 구하기
		float fPercentage = ((rcThumb.top - m_rcRegion.top) + iDiff)
							/ (((float) (iRegionHeight)) - iThumbHeight);

		if (fPercentage > 1.0f) // 너무 아래로 내렸다.
		{
			m_pThumbImageRef->SetPos(rcThumb.left, rcThumb.bottom - iThumbHeight);
			m_iCurPos =
				m_iMaxPos; // SetCurrentPos함수를 호출하면 thumb위치를 다시 계산하기 때문에 직접 바꾸어줌.
		}
		else if (fPercentage < 0.0f) // 너무 위로 올렸다.
		{
			m_pThumbImageRef->SetPos(rcThumb.left, rcThumb.top);
			m_iCurPos =
				m_iMinPos; // SetCurrentPos함수를 호출하면 thumb위치를 다시 계산하기 때문에 직접 바꾸어줌.
		}
		else
		{
			m_pThumbImageRef->SetPos(rcThumb.left, rcThumb.top + iDiff);
			m_iCurPos = static_cast<int>(
				m_iMinPos
				+ (m_iMaxPos - m_iMinPos)
					  * fPercentage); // SetCurrentPos함수를 호출하면 thumb위치를 다시 계산하기 때문에 직접 바꾸어줌.
		}
	}
	else                              // 좌우로 움직일 때
	{
		int iRegionWidth = m_rcRegion.right - m_rcRegion.left;
		int iThumbWidth  = rcThumb.right - rcThumb.left;
		if (0 == iRegionWidth || 0 == iThumbWidth)
			return;

		// 옮긴후 thumb의 위치 percentage 구하기
		float fPercentage = ((rcThumb.left - m_rcRegion.left) + iDiff)
							/ (((float) (iRegionWidth)) - iThumbWidth);

		if (fPercentage > 1.0f) // 너무 오른쪽으로 밀었다.
		{
			m_pThumbImageRef->SetPos(rcThumb.right - iThumbWidth, rcThumb.top);
			m_iCurPos =
				m_iMaxPos; // SetCurrentPos함수를 호출하면 thumb위치를 다시 계산하기 때문에 직접 바꾸어줌.
		}
		else if (fPercentage < 0.0f) // 너무 왼쪽으로 밀었다
		{
			m_pThumbImageRef->SetPos(rcThumb.left, rcThumb.top);
			m_iCurPos =
				m_iMinPos; // SetCurrentPos함수를 호출하면 thumb위치를 다시 계산하기 때문에 직접 바꾸어줌.
		}
		else
		{
			m_pThumbImageRef->SetPos(rcThumb.left + iDiff, rcThumb.top);
			m_iCurPos = static_cast<int>(
				m_iMinPos
				+ (m_iMaxPos - m_iMinPos)
					  * fPercentage); // SetCurrentPos함수를 호출하면 thumb위치를 다시 계산하기 때문에 직접 바꾸어줌.
		}
	}
}

#ifdef _N3TOOL
CN3UITrackBar& CN3UITrackBar::operator=(const CN3UITrackBar& other)
{
	if (this == &other)
		return *this;

	CN3UIBase::operator=(other);

	m_iMaxPos   = other.m_iMaxPos;   // 최대
	m_iMinPos   = other.m_iMinPos;   // 최소
	m_iCurPos   = other.m_iCurPos;   // 현재 값
	m_iPageSize = other.m_iPageSize; // page단위 이동할때 이동값

	// ImageRef 설정하기
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (UI_TYPE_IMAGE != pChild->UIType())
			continue; // image만 골라내기

		int iImageType = (int) (pChild->GetReserved());
		if (IMAGETYPE_BKGND == iImageType)
		{
			m_pBkGndImageRef = (CN3UIImage*) pChild;
		}
		else if (IMAGETYPE_THUMB == iImageType)
		{
			m_pThumbImageRef = (CN3UIImage*) pChild;
		}
	}

	return *this;
}

void CN3UITrackBar::CreateImages()
{
	__ASSERT(nullptr == m_pBkGndImageRef && nullptr == m_pThumbImageRef,
		"이미지가 이미 할당되어 있어여");
	m_pBkGndImageRef = new CN3UIImage();
	m_pBkGndImageRef->Init(this);
	m_pBkGndImageRef->SetReserved(IMAGETYPE_BKGND);

	m_pThumbImageRef = new CN3UIImage();
	m_pThumbImageRef->Init(this);
	m_pThumbImageRef->SetReserved(IMAGETYPE_THUMB);

	SetRegion(m_rcRegion); // 영역 다시 설정
}

void CN3UITrackBar::DeleteBkImage()
{
	delete m_pBkGndImageRef;
	m_pBkGndImageRef = nullptr;
}

#endif
