// N3GlobalEffect.h: interface for the CN3GlobalEffect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3GLOBALEFFECT_H__01468E41_4EE1_4893_8886_E57EE2491007__INCLUDED_)
#define AFX_N3GLOBALEFFECT_H__01468E41_4EE1_4893_8886_E57EE2491007__INCLUDED_

#pragma once

#include "N3Transform.h"

enum e_GlobalEffectType : uint8_t
{
	GETYPE_RAIN = 1,
	GETYPE_SNOW = 2
};

class CN3GlobalEffect : public CN3Transform
{
public:
	CN3GlobalEffect();
	~CN3GlobalEffect() override;

	// Attributes
public:
	void SetActive(bool bActive)
	{
		m_bActive = bActive;
	}

protected:
	int m_iVC;
	int m_iIC;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9 m_pIB;
	uint32_t m_dwEffectType;
	bool m_bActive;
	float m_fFadeTime;    // 이시간 동안 차차 목표한 양만큼 파티클의 수가 늘어난다..
	float m_fFadeTimeCur; // 지난시간..
	int m_iFadeMode;      // 1 - FadeIn 0... -1 FadeOut

						  // Operations
public:
	virtual bool NeedDelete()
	{
		return (m_iFadeMode < 0 && m_fFadeTimeCur >= m_fFadeTime);
	}

	virtual void FadeSet(float fTimeToFade, bool bFadeIn);
	void Release() override;

	// NOLINTNEXTLINE(clang-diagnostic-overloaded-virtual)
	virtual void Tick();

	virtual void Render(__Vector3& vPos);
};

#endif // !defined(AFX_N3GLOBALEFFECT_H__01468E41_4EE1_4893_8886_E57EE2491007__INCLUDED_)
