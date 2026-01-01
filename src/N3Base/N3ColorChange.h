// N3ColorChange.h: interface for the CN3ColorChange class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3COLORCHANGE_H__6A4576D3_1880_40A4_AED9_39B306508D0C__INCLUDED_)
#define AFX_N3COLORCHANGE_H__6A4576D3_1880_40A4_AED9_39B306508D0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "N3Base.h"

class CN3ColorChange  : public CN3Base
{
public:
	CN3ColorChange();
	virtual ~CN3ColorChange();

public:
	D3DCOLOR	GetCurColor() const {return m_CurColor;}
	D3DCOLOR	GetNextColor() const {return m_NextColor;}
	void		ChangeColor(D3DCOLOR color, float fSec = 0.0f);
	void		Tick();
	void		SetPercentage(float fPercentage);
protected:
	D3DCOLOR	m_CurColor;	// 현재 색.
	D3DCOLOR	m_PrevColor;	// 변화하기 전 색.
	D3DCOLOR	m_NextColor;	// 최종적으로 변화할 색.
	float		m_fPercentage;	// 현재 변화된 정도(0.0 ~ 1.0)
	float		m_fRate;		// 초당 변화율
	float		m_fTempSec;		// 임시 시간 저장 변수
};

class CN3DeltaChange  : public CN3Base
{
public:
	CN3DeltaChange();
	virtual ~CN3DeltaChange();

public:
	float		GetCurDelta() const {return m_fCurDelta;}
	float		GetNextDelta() const {return m_fNextDelta;}
	void		ChangeDelta(float fDelta, float fSec = 0.0f);
	void		Tick();
	void		SetPercentage(float fPercentage);
protected:
	float		m_fCurDelta;	// 현재 색.
	float		m_fPrevDelta;	// 변화하기 전 색.
	float		m_fNextDelta;	// 최종적으로 변화할 색.

	float		m_fPercentage;	// 현재 변화된 정도(0.0 ~ 1.0)
	float		m_fRate;		// 초당 변화율
	float		m_fTempSec;		// 임시 시간 저장 변수
};

#endif // !defined(AFX_N3COLORCHANGE_H__6A4576D3_1880_40A4_AED9_39B306508D0C__INCLUDED_)
