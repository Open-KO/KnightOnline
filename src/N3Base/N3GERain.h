// N3GERain.h: interface for the CN3GERain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3GERAIN_H__FCA2D0E1_3364_4A9D_870E_5B3FC13CD6DD__INCLUDED_)
#define AFX_N3GERAIN_H__FCA2D0E1_3364_4A9D_870E_5B3FC13CD6DD__INCLUDED_

#pragma once

#include "N3GlobalEffect.h"

class CN3GERain : public CN3GlobalEffect
{
public:
	CN3GERain();
	~CN3GERain() override;

	// Attributes
public:
	void SetRainLength(float fLen)
	{
		m_fRainLength = fLen;
	}

	void SetVelocity(const __Vector3& v)
	{
		m_vVelocity = v;
	}

protected:
	float m_fWidth;
	float m_fHeight;
	float m_fRainLength;
	__Vector3 m_vVelocity;

	// Operations
public:
	void Release() override;
	void Tick() override;
	void Render(__Vector3& vPos) override;

	void Create(float fDensity, float fWidth, float fHeight, float fRainLength,
		const __Vector3& vVelocity, float fTimeToFade = 3.0f);

protected:
};

#endif // !defined(AFX_N3GERAIN_H__FCA2D0E1_3364_4A9D_870E_5B3FC13CD6DD__INCLUDED_)
