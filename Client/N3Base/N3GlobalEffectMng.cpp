// N3GlobalEffectMng.cpp: implementation of the CN3GlobalEffectMng class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3GlobalEffectMng.h"
#include "N3GERain.h"
#include "N3GESnow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3GlobalEffectMng::CN3GlobalEffectMng()
{
	m_pGERain = NULL;
	m_pGESnow = NULL;
	CN3GlobalEffectMng::Release();
}

CN3GlobalEffectMng::~CN3GlobalEffectMng()
{
	CN3GlobalEffectMng::Release();
}

void CN3GlobalEffectMng::Release()
{
	CN3Base::Release();
	m_fCellSize = 0.0f;
	m_CurCellPos.x = m_CurCellPos.y = -1;

	if (m_pGERain) {delete m_pGERain; m_pGERain = NULL;}
	if (m_pGESnow) {delete m_pGESnow; m_pGESnow = NULL;}
}

void CN3GlobalEffectMng::Tick()
{
	if (m_fCellSize<=0.0f) return;

	POINT NewCellPos;	
	NewCellPos.x = int(s_CameraData.vEye.x/m_fCellSize);
	NewCellPos.y = int(s_CameraData.vEye.z/m_fCellSize);
	if (NewCellPos.x != m_CurCellPos.x || NewCellPos.y != m_CurCellPos.y)
	{
		m_CurCellPos = NewCellPos;
		int i, j;
		for(i=0; i<3; ++i)
			for(j=0; j<3; ++j)
				m_vPos[j*3+i].Set( (m_CurCellPos.x+i-0.5f)*m_fCellSize, 0, (m_CurCellPos.y+j-0.5f)*m_fCellSize);
	}

	if (m_pGERain)
	{
		m_pGERain->Tick();
		if(m_pGERain->NeedDelete()) { delete m_pGERain; m_pGERain = NULL; }
	}
	if (m_pGESnow)
	{
		m_pGESnow->Tick();
		if(m_pGESnow->NeedDelete()) { delete m_pGESnow; m_pGESnow = NULL; }
	}

}

void CN3GlobalEffectMng::Render()
{
	int i, j;
	for (i=0; i<3; ++i)
	{
		for(j=0; j<3; ++j)
		{
			if (m_pGERain)
			{
				m_pGERain->Render(m_vPos[j*3+i]);
			}
			if (m_pGESnow)
			{
				m_pGESnow->Render(m_vPos[j*3+i]);
			}
		}
	}
}
/*
void CN3GlobalEffectMng::SetWeather(int iWeather)
{
	const float fHeight = 20.0f;
	BOOL	bRainy;
	float fDensity;
	__Vector3 vVelocity;
	float fRainLength;
	float fSnowSize;

	switch(iWeather)
	{
	case GEW_CLEAR:
		if (m_pGERain) m_pGERain->FadeSet(3.0f, false);
		if (m_pGESnow) m_pGESnow->FadeSet(3.0f, false);
		return;
		break;
	case GEW_DRIZZLE_RAIN:
		bRainy = TRUE;
		fDensity = 0.03f;
		vVelocity.Set(0.3f, -7.0f, 0);
		fRainLength = 0.1f;
		break;
	case GEW_RAINY:
		bRainy = TRUE;
		fDensity = 0.08f;
		vVelocity.Set(0, -10.0f, 1);
		fRainLength= 0.2f;
		break;
	case GEW_HEAVY_RAIN:
		bRainy = TRUE;
		fDensity = 0.12f;
		vVelocity.Set(0.3f, -10.0f, -1.5f);
		fRainLength= 0.25f;
		break;
	case GEW_SNOW1:
		bRainy = FALSE;
		fDensity = 0.08f;
		vVelocity.Set(0.0f, -0.7f, 0.0f);
		fSnowSize = 0.05f;
		break;
	case GEW_SNOW2:
		bRainy = FALSE;
		fDensity = 0.12f;
		vVelocity.Set(0.5f, -1.0f, 0.0f);
		fSnowSize = 0.08f;
		break;
	case GEW_HEAVY_SNOW:
		bRainy = FALSE;
		fDensity = 0.2f;
		vVelocity.Set(-9.0f, -3.0f, 0.0f);
		fSnowSize = 0.1f;
		break;
	default:
		return;
	}
	
	if (bRainy)
	{
		if (m_pGERain == NULL) m_pGERain = new CN3GERain;

		m_fCellSize = 20.0f;
		m_pGERain->Create(fDensity, m_fCellSize, fHeight, fRainLength, vVelocity);	// 비
		m_pGERain->SetActive(TRUE);
		if (m_pGESnow) m_pGESnow->FadeSet(3.0f, false);
	}
	else
	{
		if (m_pGESnow == NULL) m_pGESnow = new CN3GESnow;

		m_fCellSize = 10.0f;
		m_pGESnow->Create(fDensity, m_fCellSize, fHeight, fSnowSize, vVelocity);	// 눈
		m_pGESnow->SetActive(TRUE);
		if (m_pGERain) m_pGERain->FadeSet(3.0f, false);
	}
}
*/
void CN3GlobalEffectMng::WeatherSetClean()
{
	if (m_pGESnow) m_pGESnow->FadeSet(3.0f, false);
	if (m_pGERain) m_pGERain->FadeSet(3.0f, false);
}

void CN3GlobalEffectMng::WeatherSetRainy(int iPercent)
{
	if (m_pGESnow) m_pGESnow->FadeSet(3.0f, false);
	if (m_pGERain == NULL) m_pGERain = new CN3GERain;

	float fHeight = 20.0f;
	float fPercent = iPercent / 100.0f;
	float fDensity = fPercent * 0.1f;
	__Vector3 vVelocity(3.0f * ((50-rand()%100) / 50.0f), -(10.0f + 8.0f * fPercent), 0);
	float fRainLength = 0.4f + 0.6f * fPercent;

	m_fCellSize = 20.0f;
	m_pGERain->Create(fDensity, m_fCellSize, fHeight, fRainLength, vVelocity, 10.0f);	// 비
	m_pGERain->SetActive(TRUE);
}

void CN3GlobalEffectMng::WeatherSetSnow(int iPercent)
{
	if (m_pGERain) m_pGERain->FadeSet(3.0f, false);
	if (m_pGESnow == NULL) m_pGESnow = new CN3GESnow;

	float fHeight = 20.0f;
	float fPercent = iPercent / 100.0f;
	float fDensity = fPercent * 0.1f;
	float fHorz = (3.0f * fPercent) + (3.0f * ((50-rand()%100) / 50.0f));
	__Vector3 vVelocity(fHorz, -(2.0f + 2.0f * fPercent), 0);
	float fSnowSize = 0.1f + 0.1f * fPercent;

	m_fCellSize = 20.0f;
	m_pGESnow->Create(fDensity, m_fCellSize, fHeight, fSnowSize, vVelocity, 10.0f);	// 비
	m_pGESnow->SetActive(TRUE);
}