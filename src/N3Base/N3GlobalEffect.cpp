// N3GlobalEffect.cpp: implementation of the CN3GlobalEffect class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3GlobalEffect.h"

CN3GlobalEffect::CN3GlobalEffect()
{
	m_dwEffectType = 0;
	m_pVB          = nullptr;
	m_pIB          = nullptr;
	m_bActive      = false;
	m_iVC          = 0;
	m_iIC          = 0;
	m_fFadeTime    = 0.0f; // 이시간 동안 차차 목표한 양만큼 파티클의 수가 늘어난다..
	m_fFadeTimeCur = 0.0f; // 지난시간..
	m_iFadeMode    = 0;
}

CN3GlobalEffect::~CN3GlobalEffect()
{
	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}
	if (m_pIB)
	{
		m_pIB->Release();
		m_pIB = nullptr;
	}
}

void CN3GlobalEffect::Release()
{
	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}
	if (m_pIB)
	{
		m_pIB->Release();
		m_pIB = nullptr;
	}
	m_bActive      = false;
	m_iVC          = 0;
	m_iIC          = 0;
	m_fFadeTime    = 0.0f; // 이시간 동안 차차 목표한 양만큼 파티클의 수가 늘어난다..
	m_fFadeTimeCur = 0.0f; // 지난시간..
	m_iFadeMode    = 0;

	CN3Transform::Release();
}

void CN3GlobalEffect::Tick()
{
	CN3Transform::Tick(-1);

	if (m_iFadeMode && m_fFadeTime > 0) // 시간을 지나게 한다..
	{
		m_fFadeTimeCur += s_fSecPerFrm;
		if (m_fFadeTimeCur > m_fFadeTime)
			m_fFadeTimeCur = m_fFadeTime;
	}
}

void CN3GlobalEffect::Render(__Vector3& vPos)
{
	PosSet(vPos);
}

void CN3GlobalEffect::FadeSet(float fTimeToFade, bool bFadeIn)
{
	m_fFadeTime    = fTimeToFade;
	m_fFadeTimeCur = 0;
	if (bFadeIn)
		m_iFadeMode = 1;
	else
		m_iFadeMode = -1;
}
