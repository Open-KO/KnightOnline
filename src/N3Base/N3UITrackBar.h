// N3UITrackBar.h: interface for the CN3UITrackBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UITRACKBAR_H__EB496B74_468B_4D7B_89A9_D3A5A1A3E538__INCLUDED_)
#define AFX_N3UITRACKBAR_H__EB496B74_468B_4D7B_89A9_D3A5A1A3E538__INCLUDED_

#pragma once

#include "N3UIBase.h"

class CN3UITrackBar : public CN3UIBase
{
public:
	CN3UITrackBar();
	~CN3UITrackBar() override;

	// Attributes
public:
	enum eIMAGE_TYPE : uint8_t
	{
		IMAGETYPE_BKGND = 0,
		IMAGETYPE_THUMB,
		NUM_IMAGETYPE
	};

protected:
	CN3UIImage* m_pBkGndImageRef; // 배경 이미지 reference (메모리 할당은 children list로 관리)
	CN3UIImage* m_pThumbImageRef; // 가운데 드레그 하여 옮길 수 있는 이미지 reference

	int m_iMaxPos;                // 최대
	int m_iMinPos;                // 최소
	int m_iCurPos;                // 현재 값
	int m_iPageSize;              // page단위 이동할때 이동값
								  // Operations
public:
	void Release() override;
	bool Load(File& file) override;
	void SetRegion(const RECT& Rect) override;
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;

	void SetRange(int iMin, int iMax);
	void SetCurrentPos(int iPos);

	void SetRangeMax(int iMax)
	{
		SetRange(m_iMinPos, iMax);
	}

	void SetRangeMin(int iMin)
	{
		SetRange(iMin, m_iMaxPos);
	}

	int GetPos() const
	{
		return m_iCurPos;
	}

	void SetPageSize(int iSize)
	{
		m_iPageSize = iSize;
	}

	int GetPageSize() const
	{
		return m_iPageSize;
	}

	int GetMaxPos() const
	{
		return m_iMaxPos;
	}

	int GetMinPos() const
	{
		return m_iMinPos;
	}

protected:
	void UpdateThumbPos(); // m_iCurPos를 계산하여 Thumb위치 다시 계산하여 바꾸기
	void UpDownThumbPos(
		int iDiff);        // Thumb위치를 아래 위로 iDiff pixel만큼 움직인 후 m_iCurPos 갱신하기

#ifdef _N3TOOL
	// tool에서 쓰이는 함수
public:
	CN3UITrackBar& operator=(const CN3UITrackBar& other);
	void CreateImages();  // 이미지 생성
	void DeleteBkImage(); // 배경이미지 삭제

	CN3UIImage* GetBkGndImgRef() const
	{
		return m_pBkGndImageRef;
	}

	CN3UIImage* GetThumbImgRef() const
	{
		return m_pThumbImageRef;
	}
#endif
};

#endif // !defined(AFX_N3UITRACKBAR_H__EB496B74_468B_4D7B_89A9_D3A5A1A3E538__INCLUDED_)
