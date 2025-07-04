// N3GlobalEffectMng.h: interface for the CN3GlobalEffectMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3GLOBALEFFECTMNG_H__E95C3268_DD68_45AE_AA24_DC64B11A1B08__INCLUDED_)
#define AFX_N3GLOBALEFFECTMNG_H__E95C3268_DD68_45AE_AA24_DC64B11A1B08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3Base.h"

enum {	GEW_CLEAR=0,			// 맑음
		GEW_DRIZZLE_RAIN=1,		// 가랑비
		GEW_RAINY=2,			// 적당량의 비
		GEW_HEAVY_RAIN=3,		// 폭우
		GEW_SNOW1=11,			// 포근한 눈
		GEW_SNOW2=12,			// 적당한 눈
		GEW_HEAVY_SNOW=13		// 폭설
};

class CN3GERain;
class CN3GESnow;
class CN3GlobalEffectMng : public CN3Base  
{
public:
	CN3GlobalEffectMng();
	virtual ~CN3GlobalEffectMng();

// Attributes
public:
protected:
	float			m_fCellSize;	// 내부 셀 크기
	POINT			m_CurCellPos;	// 현재 내부셀 좌표
	__Vector3		m_vPos[9];		// 주변 셀 9칸의 중심좌표들

	// 효과들..
	CN3GERain*		m_pGERain;		// 비
	CN3GESnow*		m_pGESnow;		// 눈

// Operations
public:
	virtual void	Release();
	void			Tick();
	void			Render();
//	void			SetWeather(int iWeather);	// 날씨 지정하기
	void			WeatherSetRainy(int iPercent);
	void			WeatherSetSnow(int iPercent);
	void			WeatherSetClean();

protected:

};

#endif // !defined(AFX_N3GLOBALEFFECTMNG_H__E95C3268_DD68_45AE_AA24_DC64B11A1B08__INCLUDED_)
