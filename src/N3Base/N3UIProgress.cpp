// CN3UIProgress.cpp: implementation of the CN3UIProgress class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIImage.h"
#include "N3UIProgress.h"

CN3UIProgress::CN3UIProgress()
{
	m_eType        = UI_TYPE_PROGRESS;

	m_pBkGndImgRef = nullptr;
	m_pFrGndImgRef = nullptr;

	memset(&m_frcFrGndImgUV, 0, sizeof(m_frcFrGndImgUV));

	m_iMinValue          = 0;
	m_iMaxValue          = 0;
	m_iValueToReach      = 0;
	m_fCurValue          = 0;
	m_fChangeSpeedPerSec = 0;
	m_fTimeToDelay       = 0; // 지연시간..
	m_iStepValue         = 0;
}

CN3UIProgress::~CN3UIProgress()
{
}

void CN3UIProgress::Release()
{
	CN3UIBase::Release();

	m_pBkGndImgRef = nullptr;
	m_pFrGndImgRef = nullptr;
	ZeroMemory(&m_frcFrGndImgUV, sizeof(m_frcFrGndImgUV));
	m_iMaxValue = m_iMinValue = 0;
	m_fCurValue               = 0;
	m_fChangeSpeedPerSec      = 0;
}

void CN3UIProgress::Render()
{
	if (!m_bVisible)
		return;

	CN3UIBase::Render();

	if (m_pBkGndImgRef)
		m_pBkGndImgRef->Render();
	if (m_pFrGndImgRef)
		m_pFrGndImgRef->Render();
}

void CN3UIProgress::SetCurValue(int iValue, float fTimeToDelay, float fChangeSpeedPerSec)
{
	if (iValue < m_iMinValue)
		iValue = m_iMinValue;
	if (iValue > m_iMaxValue)
		iValue = m_iMaxValue;
	if (fTimeToDelay < 0.0f)
		fTimeToDelay = 0.0f;
	if (fChangeSpeedPerSec < 0.0f)
		fChangeSpeedPerSec = 0.0f;

	if (m_iValueToReach == iValue)
		return;

	m_iValueToReach      = iValue;
	m_fTimeToDelay       = fTimeToDelay;                    // 지연시간..
	m_fChangeSpeedPerSec = fChangeSpeedPerSec;

	if (0.0f == fTimeToDelay && 0.0f == fChangeSpeedPerSec) // 지연 시간이 없으면 바로 세팅..
	{
		m_fCurValue = (float) iValue;
		UpdateFrGndImage();
	}
}

void CN3UIProgress::SetRegion(const RECT& Rect)
{
	CN3UIBase::SetRegion(Rect);
	if (m_pBkGndImgRef)
		m_pBkGndImgRef->SetRegion(Rect);
	UpdateFrGndImage();
}

void CN3UIProgress::SetStyle(uint32_t dwStyle)
{
	CN3UIBase::SetStyle(dwStyle);
	UpdateFrGndImage();
}

void CN3UIProgress::UpdateFrGndImage()
{
	if (nullptr == m_pFrGndImgRef)
		return;
	int iDiff = m_iMaxValue - m_iMinValue;
	if (0 == iDiff)
		return;
	const float fPercentage = ((float) (m_fCurValue - m_iMinValue))
							  / ((float) (m_iMaxValue - m_iMinValue));

	RECT rcRegion;
	__FLOAT_RECT frcUVRect;
	if (m_dwStyle & UISTYLE_PROGRESS_RIGHT2LEFT)
	{
		rcRegion.right  = m_rcRegion.right;
		rcRegion.top    = m_rcRegion.top;
		rcRegion.bottom = m_rcRegion.bottom;
		rcRegion.left   = m_rcRegion.right
						- ((int) ((m_rcRegion.right - m_rcRegion.left) * fPercentage));

		frcUVRect.right  = m_frcFrGndImgUV.right;
		frcUVRect.top    = m_frcFrGndImgUV.top;
		frcUVRect.bottom = m_frcFrGndImgUV.bottom;
		frcUVRect.left   = m_frcFrGndImgUV.right
						 - (m_frcFrGndImgUV.right - m_frcFrGndImgUV.left)
							   * ((rcRegion.right - rcRegion.left)
								   / ((float) (m_rcRegion.right - m_rcRegion.left)));
	}
	else if (m_dwStyle & UISTYLE_PROGRESS_TOP2BOTTOM)
	{
		rcRegion.left   = m_rcRegion.left;
		rcRegion.right  = m_rcRegion.right;
		rcRegion.top    = m_rcRegion.top;
		rcRegion.bottom = m_rcRegion.top
						  + ((int) ((m_rcRegion.bottom - m_rcRegion.top) * fPercentage));

		frcUVRect.left   = m_frcFrGndImgUV.left;
		frcUVRect.right  = m_frcFrGndImgUV.right;
		frcUVRect.top    = m_frcFrGndImgUV.top;
		frcUVRect.bottom = m_frcFrGndImgUV.top
						   + (m_frcFrGndImgUV.bottom - m_frcFrGndImgUV.top)
								 * ((rcRegion.bottom - rcRegion.top)
									 / ((float) (m_rcRegion.bottom - m_rcRegion.top)));
	}
	else if (m_dwStyle & UISTYLE_PROGRESS_BOTTOM2TOP)
	{
		rcRegion.left   = m_rcRegion.left;
		rcRegion.right  = m_rcRegion.right;
		rcRegion.bottom = m_rcRegion.bottom;
		rcRegion.top    = m_rcRegion.bottom
					   - ((int) ((m_rcRegion.bottom - m_rcRegion.top) * fPercentage));

		frcUVRect.left   = m_frcFrGndImgUV.left;
		frcUVRect.right  = m_frcFrGndImgUV.right;
		frcUVRect.bottom = m_frcFrGndImgUV.bottom;
		frcUVRect.top    = m_frcFrGndImgUV.bottom
						- (m_frcFrGndImgUV.bottom - m_frcFrGndImgUV.top)
							  * ((rcRegion.bottom - rcRegion.top)
								  / ((float) (m_rcRegion.bottom - m_rcRegion.top)));
	}
	else
	{
		rcRegion.left   = m_rcRegion.left;
		rcRegion.top    = m_rcRegion.top;
		rcRegion.bottom = m_rcRegion.bottom;
		rcRegion.right  = m_rcRegion.left
						 + ((int) ((m_rcRegion.right - m_rcRegion.left) * fPercentage));

		frcUVRect.left   = m_frcFrGndImgUV.left;
		frcUVRect.top    = m_frcFrGndImgUV.top;
		frcUVRect.bottom = m_frcFrGndImgUV.bottom;
		frcUVRect.right  = m_frcFrGndImgUV.left
						  + (m_frcFrGndImgUV.right - m_frcFrGndImgUV.left)
								* ((rcRegion.right - rcRegion.left)
									/ ((float) (m_rcRegion.right - m_rcRegion.left)));
	}

	m_pFrGndImgRef->SetRegion(rcRegion);
	m_pFrGndImgRef->SetUVRect(frcUVRect.left, frcUVRect.top, frcUVRect.right, frcUVRect.bottom);
}

void CN3UIProgress::SetFrGndUVFromFrGndImage()
// m_pFrGndImgRef로부터 uv좌표를 얻어와서 m_frcFrGndImgUV를 세팅한다.
{
	__ASSERT(m_pFrGndImgRef, "not found foreground image in N3UIProgress");
	if (nullptr == m_pFrGndImgRef)
		return;
	m_frcFrGndImgUV = *(m_pFrGndImgRef->GetUVRect());
	UpdateFrGndImage();
}

bool CN3UIProgress::Load(File& file)
{
	if (false == CN3UIBase::Load(file))
		return false;

	// m_ImageRef 설정하기
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (UI_TYPE_IMAGE != pChild->UIType())
			continue; // image만 골라내기
		int iImageType = (int) (pChild->GetReserved());
		if (iImageType == IMAGETYPE_BKGND)
		{
			m_pBkGndImgRef = (CN3UIImage*) pChild;
		}
		else if (iImageType == IMAGETYPE_FRGND)
		{
			m_pFrGndImgRef = (CN3UIImage*) pChild;
		}
		SetFrGndUVFromFrGndImage();
	}
	return true;
}

#ifdef _N3TOOL
CN3UIProgress& CN3UIProgress::operator=(const CN3UIProgress& other)
{
	if (this == &other)
		return *this;

	CN3UIBase::operator=(other);

	m_frcFrGndImgUV      = other.m_frcFrGndImgUV; // m_FrGndImgRef 의 uv좌표
	m_iMaxValue          = other.m_iMaxValue;     // 최대
	m_iMinValue          = other.m_iMinValue;     // 최소

	// 현재 값 - 부드럽게 점차 값을 올려가려고 float 로 했다..
	m_fCurValue          = other.m_fCurValue;

	// 현재값이 변해야 되는 속도.. Unit SpeedPerSec
	m_fChangeSpeedPerSec = other.m_fChangeSpeedPerSec;

	// 도달해야 될값 - 부드럽게 값이 올라가는 경우에 필요하다..
	m_iValueToReach      = other.m_iValueToReach;
	m_fTimeToDelay       = other.m_fTimeToDelay; // 지연시간..
	m_iStepValue         = other.m_iStepValue;   // 변화값 StepIt()을 통한 변화되는 값

	// m_ImageRef 설정하기
	for (CN3UIBase* pChild : m_Children)
	{
		if (UI_TYPE_IMAGE != pChild->UIType())
			continue; // image만 골라내기

		int iImageType = (int) (pChild->GetReserved());
		if (iImageType == IMAGETYPE_BKGND)
		{
			m_pBkGndImgRef = (CN3UIImage*) pChild;
		}
		else if (iImageType == IMAGETYPE_FRGND)
		{
			m_pFrGndImgRef = (CN3UIImage*) pChild;
		}
	}

	SetFrGndUVFromFrGndImage();

	return *this;
}

bool CN3UIProgress::Save(File& file)
{
	int iCur = (int) m_fCurValue;      // 이전 상태 기억해 놓기
	SetCurValue(m_iMaxValue);          // foreground가 꽉 채운 이미지 만들기
	bool bRet = CN3UIBase::Save(file); // 저장하기

	SetCurValue((int) m_fCurValue);    // 이전 상태로 되돌리기

	return bRet;
}

void CN3UIProgress::CreateImages()
{
	m_pBkGndImgRef = new CN3UIImage();
	m_pBkGndImgRef->Init(this);
	m_pBkGndImgRef->SetRegion(m_rcRegion);
	m_pBkGndImgRef->SetReserved(IMAGETYPE_BKGND); // 이 이미지가 배경이미지임을 지정해줌

	m_pFrGndImgRef = new CN3UIImage();
	m_pFrGndImgRef->Init(this);
	m_pFrGndImgRef->SetRegion(m_rcRegion);
	m_pFrGndImgRef->SetReserved(IMAGETYPE_FRGND); // 이 이미지가 foreground 이미지임을 지정해줌

	// 임시로 넣기
	m_iMaxValue = 100;
	m_iMinValue = 0;
	m_fCurValue = 70;
}

void CN3UIProgress::DeleteBkGndImage() // Back ground이미지는 필요 없는 경우가 있다.
{
	if (m_pBkGndImgRef)
	{
		delete m_pBkGndImgRef;
		m_pBkGndImgRef = nullptr;
	}
}

#endif

void CN3UIProgress::Tick()
{
	CN3UIBase::Tick();

	if (m_fTimeToDelay > 0)
	{
		m_fTimeToDelay -= s_fSecPerFrm; // 시간 지연
		if (m_fTimeToDelay < 0)
			m_fTimeToDelay = 0;
		return;
	}

	if (m_fChangeSpeedPerSec <= 0)
		return;
	if ((int) m_fCurValue == m_iValueToReach)
		return;

	if (m_fCurValue < m_iValueToReach)
	{
		m_fCurValue += m_fChangeSpeedPerSec * s_fSecPerFrm; // 초당 30 퍼센트 올라가게 조정..
		if (m_fCurValue > m_iValueToReach)
			m_fCurValue = (float) m_iValueToReach;
		UpdateFrGndImage();
	}
	else if (m_fCurValue > m_iValueToReach)
	{
		m_fCurValue -= m_fChangeSpeedPerSec * s_fSecPerFrm; // 초당 30 퍼센트 올라가게 조정..
		if (m_fCurValue < m_iValueToReach)
			m_fCurValue = (float) m_iValueToReach;
		UpdateFrGndImage();
	}
}
