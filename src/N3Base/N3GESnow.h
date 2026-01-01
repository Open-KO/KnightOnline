// N3GESnow.h: interface for the CN3GESnow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3GESNOW_H__F7097F94_15E3_42B8_95DB_9EEC3CFD2E9C__INCLUDED_)
#define AFX_N3GESNOW_H__F7097F94_15E3_42B8_95DB_9EEC3CFD2E9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3GlobalEffect.h"

class CN3GESnow : public CN3GlobalEffect  
{
// Structures
protected:
	struct __SnowParticle
	{
		__Vector3	vPos;							// 회전 반경의 줌심
		__Vector3	vOffset1, vOffset2, vOffset3;	// 점 1 2 3이 파티클 중점에서 떨어진 정도
		float		fRadius;						// 회전 반경
		float		fRadian;						// 현재 회전된 각도
		//float		fRVel							// 회전각속도 (우선 일정하다고 가정)
		// __Vector3 vVelocity						// 떨어지는 속도(우선 모두 같다고 가정)
	};

// Attributes
public:
	void SetVelocity(__Vector3& v) {m_vVelocity = v;}
protected:
	float		m_fWidth;		// 눈이 떨어지는 공간의 너비
	float		m_fHeight;		// 눈이 떨어지는 공간의 높이
	float		m_fSnowSize;	// 눈 하나의 크기
	__Vector3	m_vVelocity;	// 전체적으로 떨어지는 방향과 속도
	CN3Texture*	m_pTex;			// 눈 텍스쳐
	__SnowParticle*	m_pSnowParticle;	// 눈송이 정보

// Operations
public:
	void Release();
	void Tick();
	void Render(__Vector3& vPos);
	void Create(float fDensity, float fWidth, float fHeight, float fSnowSize, const __Vector3& vVelocity, float fTimeToFade = 3.0f);

	CN3GESnow();
	virtual ~CN3GESnow();

};

#endif // !defined(AFX_N3GESNOW_H__F7097F94_15E3_42B8_95DB_9EEC3CFD2E9C__INCLUDED_)
