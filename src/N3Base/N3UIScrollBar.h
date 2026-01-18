// N3UIScrollBar.h: interface for the CN3UIScrollBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UISCROLLBAR_H__76F849B9_9A93_4439_8FB0_A1DB922CBC73__INCLUDED_)
#define AFX_N3UISCROLLBAR_H__76F849B9_9A93_4439_8FB0_A1DB922CBC73__INCLUDED_

#pragma once

#include "N3UIBase.h"
#include "N3UITrackBar.h"

class CN3UIScrollBar : public CN3UIBase
{
public:
	CN3UIScrollBar();
	~CN3UIScrollBar() override;

	// Attributes
public:
	enum eBTN_TYPE : uint8_t
	{
		BTN_LEFTUP = 0,
		BTN_RIGHTDOWN,
		NUM_BTN_TYPE
	};

protected:
	CN3UITrackBar* m_pTrackBarRef;
	CN3UIButton* m_pBtnRef[NUM_BTN_TYPE];

	int m_iLineSize; // 버튼을 눌렀을때 trackbar가 움직여지는 크기

					 // Operations
public:
	void Release() override;
	bool Load(File& file) override;
	void SetRegion(const RECT& Rect) override;
	void SetStyle(uint32_t dwStyle) override;

	// 메시지를 받는다.. 보낸놈, msg
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void SetRange(int iMin, int iMax)
	{
		if (m_pTrackBarRef != nullptr)
			m_pTrackBarRef->SetRange(iMin, iMax);
	}

	void SetRangeMax(int iMax)
	{
		if (m_pTrackBarRef != nullptr)
			m_pTrackBarRef->SetRangeMax(iMax);
	}

	void SetRangeMin(int iMin)
	{
		if (m_pTrackBarRef != nullptr)
			m_pTrackBarRef->SetRangeMin(iMin);
	}

	int GetCurrentPos() const
	{
		if (m_pTrackBarRef != nullptr)
			return m_pTrackBarRef->GetPos();

		return 0;
	}

	void SetCurrentPos(int iPos)
	{
		if (m_pTrackBarRef != nullptr)
			m_pTrackBarRef->SetCurrentPos(iPos);
	}

	void SetPageSize(int iSize)
	{
		if (m_pTrackBarRef != nullptr)
			m_pTrackBarRef->SetPageSize(iSize);
	}

	int GetPageSize() const
	{
		if (m_pTrackBarRef != nullptr)
			return m_pTrackBarRef->GetPageSize();

		return 0;
	}

	void SetLineSize(int iSize)
	{
		m_iLineSize = iSize;
	}

	int GetLineSize() const
	{
		return m_iLineSize;
	}

	int GetMaxPos() const
	{
		if (m_pTrackBarRef != nullptr)
			return m_pTrackBarRef->GetMaxPos();

		return 0;
	}

#ifdef _N3TOOL
	// tool에서 사용하는 함수들
public:
	CN3UIScrollBar& operator=(const CN3UIScrollBar& other);
	void CreateTrackBarAndBtns(); // trackbar와 button 생성

	CN3UITrackBar* GetTrackBarRef() const
	{
		return m_pTrackBarRef;
	}

	CN3UIButton* GetBtnRef(eBTN_TYPE eBtnType) const
	{
		return m_pBtnRef[eBtnType];
	}
#endif
};

#endif // !defined(AFX_N3UISCROLLBAR_H__76F849B9_9A93_4439_8FB0_A1DB922CBC73__INCLUDED_)
