// N3Moon.h: interface for the CN3Moon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3MOON_H__A89DCBCF_C959_4D36_92C1_E138448717D9__INCLUDED_)
#define AFX_N3MOON_H__A89DCBCF_C959_4D36_92C1_E138448717D9__INCLUDED_

#pragma once

#include "N3Base.h"
#include <string>

class CN3Texture;
class CN3Moon : public CN3Base
{
#ifdef _N3TOOL
	friend class CN3SkyMng;
#endif

public:
	CN3Moon();
	~CN3Moon() override;

	// Attributes
public:
	int GetMoonPhaseIndex() const
	{
		return m_iMoonPhaseIndex;
	}

protected:
	__VertexTransformed m_pVertices[4];
	CN3Texture* m_pTexture;
	float m_fCurRadian;    // 현재 달의 회전위치
	float m_fMoonRatio;    // 달의 pixel싸이즈
	int m_iMoonPhaseIndex; // 달의 모양 인덱스 0~23;

						   // Operations
public:
	void Init(const std::string& szTexFN);
	void SetMoonPhase(int iIndex); // 달의 모양 설정

	// 현재 각도설정
	void SetCurAngle(float fAngle)
	{
		m_fCurRadian = DegreesToRadians(fAngle);
	}

	void Render(__Matrix44& matView, __Matrix44& matProj);
	void Release() override;
};

#endif // !defined(AFX_N3MOON_H__A89DCBCF_C959_4D36_92C1_E138448717D9__INCLUDED_)
