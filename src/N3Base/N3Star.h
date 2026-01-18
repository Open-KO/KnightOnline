// N3Star.h: interface for the CN3Star class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3STAR_H__09250C74_48BB_4F81_9903_45EAE7605CB6__INCLUDED_)
#define AFX_N3STAR_H__09250C74_48BB_4F81_9903_45EAE7605CB6__INCLUDED_

#pragma once

#include "N3Base.h"

inline constexpr int MAX_STAR = 200;

class CN3Star : public CN3Base
{
public:
	CN3Star();
	~CN3Star() override;

	// Attributes
public:
	int GetNumStar() const
	{
		return m_iCurNumStar;
	}

protected:
	int m_iCurNumStar                  = 0;     // 현재 떠있는 별의 수
	int m_iNextNumStar                 = 0;     // 앞으로 보일 별 숫자
	float m_fAddTime                   = 0.0f;  // 별 하나가 추가되거나 없어지는 시간 간격
	BOOL m_bIncrease                   = FALSE; // 별이 증가할 것인가 감소할 것인가
	__VertexXyzColor m_Stars[MAX_STAR] = {};
	// Operations
public:
	void Init();
	void Release() override;
	virtual void Tick();
	virtual void Render();

	void SetStar(int iNum, float fSec);
};

#endif // !defined(AFX_N3STAR_H__09250C74_48BB_4F81_9903_45EAE7605CB6__INCLUDED_)
