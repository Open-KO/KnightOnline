// N3UIProgress.h: interface for the CN3UIProgress class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UIPROGRESS_H__09307DB0_2F95_4205_AF1D_0262F1CE0E24__INCLUDED_)
#define AFX_N3UIPROGRESS_H__09307DB0_2F95_4205_AF1D_0262F1CE0E24__INCLUDED_

#pragma once

#include "N3UIBase.h"

class CN3UIImage;
class CN3UIProgress : public CN3UIBase
{
#ifdef _N3TOOL
	friend class CPropertyView; // 툴에서 각 변수들을 접근하기 위해서
#endif

public:
	CN3UIProgress();
	~CN3UIProgress() override;

	// Attributes
public:
	enum eIMAGE_TYPE : uint8_t
	{
		IMAGETYPE_BKGND = 0,
		IMAGETYPE_FRGND,
		NUM_IMAGETYPE
	};

	int GetMax() const
	{
		return m_iMaxValue;
	}

	int GetMin() const
	{
		return m_iMinValue;
	}

	int GetCurValue() const
	{
		return (int) m_fCurValue;
	}

	void SetStepValue(int iValue)
	{
		m_iStepValue = iValue;
	}

	void StepIt()
	{
		SetCurValue((int) m_fCurValue + m_iStepValue);
	}

protected:
	CN3UIImage* m_pBkGndImgRef;
	CN3UIImage* m_pFrGndImgRef;

	__FLOAT_RECT m_frcFrGndImgUV; // m_FrGndImgRef 의 uv좌표

	int m_iMaxValue;              // 최대
	int m_iMinValue;              // 최소
	float m_fCurValue;            // 현재 값 - 부드럽게 점차 값을 올려가려고 float 로 했다..
	float m_fChangeSpeedPerSec;   // 현재값이 변해야 되는 속도.. Unit SpeedPerSec
	int m_iValueToReach;          // 도달해야 될값 - 부드럽게 값이 올라가는 경우에 필요하다..
	float m_fTimeToDelay;         // 지연시간..

	int m_iStepValue;             // 변화값 StepIt()을 통한 변화되는 값
								  //	Operations
public:
	void Tick() override;
	void Render() override;
	void Release() override;
	void SetRegion(const RECT& Rect) override;
	void SetStyle(uint32_t dwStyle) override;
	bool Load(File& file) override;

	void SetCurValue(int iValue, float fTimeToDelay = 0, float fChangeSpeedPerSec = 0);

	// 초당 몇 퍼센트 수치로 올라간다.
	void SetCurValue_Smoothly(int iValue, float fPercentPerSec)
	{
		SetCurValue(iValue, 0, fPercentPerSec * (m_iMaxValue - m_iMinValue));
	}

	void SetRange(int iMin, int iMax)
	{
		m_iMinValue = iMin;
		m_iMaxValue = iMax;
		UpdateFrGndImage();
	}

	// m_pFrGndImgRef로부터 uv좌표를 얻어와서 m_frcFrGndImgUV를 세팅한다.
	void SetFrGndUVFromFrGndImage();

protected:
	void UpdateFrGndImage(); // m_FrGndImgRef 의 영역과 uv좌표를 m_fCurValue에 따라 알맞게 바꾼다.

#ifdef _N3TOOL
	// tool에서 사용하는 함수들
public:
	CN3UIProgress& operator=(const CN3UIProgress& other);
	bool Save(File& file) override;

	CN3UIImage* GetBkGndImgRef() const
	{
		return m_pBkGndImgRef;
	}

	CN3UIImage* GetFrGndImgRef() const
	{
		return m_pFrGndImgRef;
	}

	void CreateImages();     // 이미지 생성
	void DeleteBkGndImage(); // Back ground이미지는 필요 없는 경우가 있다.
#endif
};

#endif // !defined(AFX_N3UIPROGRESS_H__09307DB0_2F95_4205_AF1D_0262F1CE0E24__INCLUDED_)

