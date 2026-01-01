// N3ColorChange.cpp: implementation of the CN3ColorChange class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3ColorChange.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3ColorChange::CN3ColorChange()
{
	m_CurColor = m_PrevColor = m_NextColor = 0xffffffff;
	m_fPercentage = 0.0f;
	m_fRate = 0.0f;
	m_fTempSec = 0.0f;
}

CN3ColorChange::~CN3ColorChange()
{
}

void CN3ColorChange::ChangeColor(D3DCOLOR color, float fSec)
{
	if (color == m_CurColor || 0.0f >= fSec)	// 즉시 변화
	{
		m_CurColor = m_PrevColor = m_NextColor = color;
		m_fRate = m_fPercentage = m_fTempSec = 0.0f;
		return;
	}

	m_PrevColor = m_CurColor;
	m_NextColor = color;
	m_fPercentage = 0.0f;
	m_fRate = 1.0f/fSec;
}

void CN3ColorChange::Tick()
{
	if (0.0f == m_fRate) return;
	m_fTempSec += s_fSecPerFrm;
	if (m_fTempSec>0.1f)
	{
		m_fPercentage += (m_fRate * m_fTempSec);
		m_fTempSec = 0.0f;
	}
	if (m_fPercentage > 1.0f)
	{
		m_PrevColor = m_CurColor = m_NextColor;
		m_fPercentage = m_fRate = 0.0f;
		return;
	}
	int iPrevA = (m_PrevColor & 0xff000000)>>24;
	int iPrevR = (m_PrevColor & 0x00ff0000)>>16;
	int iPrevG = (m_PrevColor & 0x0000ff00)>>8;
	int iPrevB = (m_PrevColor & 0x000000ff);
	int iNextA = (m_NextColor & 0xff000000)>>24;
	int iNextR = (m_NextColor & 0x00ff0000)>>16;
	int iNextG = (m_NextColor & 0x0000ff00)>>8;
	int iNextB = (m_NextColor & 0x000000ff);
	uint32_t dwCurA = ((iPrevA + int((iNextA-iPrevA)*m_fPercentage))&0xff) << 24;
	uint32_t dwCurR = ((iPrevR + int((iNextR-iPrevR)*m_fPercentage))&0xff) << 16;
	uint32_t dwCurG = ((iPrevG + int((iNextG-iPrevG)*m_fPercentage))&0xff) << 8;
	uint32_t dwCurB = ((iPrevB + int((iNextB-iPrevB)*m_fPercentage))&0xff);

	m_CurColor = dwCurA|dwCurR|dwCurG|dwCurB;
}

void CN3ColorChange::SetPercentage(float fPercentage)
{
	if (0.0f>fPercentage || 1.0f<fPercentage) return;
	if (1.0f == fPercentage)
	{	// 1.0f이면 바로 색 바꾸기
		m_PrevColor = m_CurColor = m_NextColor;
		m_fPercentage = m_fRate = 0.0f;
		return;
	}
	m_fPercentage = fPercentage;
	m_fTempSec = 0.0f;
}


















CN3DeltaChange::CN3DeltaChange()
{
	m_fCurDelta = m_fPrevDelta = m_fNextDelta = 0.0f;
	m_fPercentage = 0.0f;
	m_fRate = 0.0f;
	m_fTempSec = 0.0f;
}

CN3DeltaChange::~CN3DeltaChange()
{
}

void CN3DeltaChange::ChangeDelta(float fDelta, float fSec)
{
	if (fDelta == m_fCurDelta || 0.0f >= fSec)	// 즉시 변화
	{
		m_fCurDelta = m_fPrevDelta = m_fNextDelta = fDelta;
		m_fRate = m_fPercentage = m_fTempSec = 0.0f;
		return;
	}

	m_fPrevDelta = m_fCurDelta;
	m_fNextDelta = fDelta;
	m_fPercentage = 0.0f;
	m_fRate = 1.0f/fSec;
}

void CN3DeltaChange::Tick()
{
	if (0.0f == m_fRate) return;
	m_fTempSec += s_fSecPerFrm;
	if (m_fTempSec>0.1f)
	{
		m_fPercentage += (m_fRate * m_fTempSec);
		m_fTempSec = 0.0f;
	}
	if (m_fPercentage > 1.0f)
	{
		m_fPrevDelta = m_fCurDelta = m_fNextDelta;
		m_fPercentage = m_fRate = 0.0f;
		return;
	}
	
	m_fCurDelta = m_fPrevDelta + (m_fNextDelta - m_fPrevDelta) * m_fPercentage;
}

void CN3DeltaChange::SetPercentage(float fPercentage)
{
	if (0.0f>fPercentage || 1.0f<fPercentage) return;
	if (1.0f == fPercentage)
	{	// 1.0f이면 바로 색 바꾸기
		m_fPrevDelta = m_fCurDelta = m_fNextDelta;
		m_fPercentage = m_fRate = 0.0f;
		return;
	}
	m_fPercentage = fPercentage;
	m_fTempSec = 0.0f;
}
