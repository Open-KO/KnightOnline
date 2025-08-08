// Bird.cpp: implementation of the CBird class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Bird.h"
#include <N3Base/N3Shape.h>
#include <N3Base/N3SndObj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// TESTING !@#$%@#$@#$

CBird::CBird()
{
	m_pSnd = NULL;
	m_pShape = NULL;

	Release();	
}

CBird::~CBird()
{
	Release();
}

void CBird::Release()
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd);
	if (m_pShape) {delete m_pShape; m_pShape = NULL;}
	m_vPivot.Set(0,0,0);
	m_fRadius = 0.0f;
	m_fRadian = 0.0f;

	m_fFactor1 = m_fFactor2 = 0;
	m_fFactorSpeed1 = m_fFactorSpeed2 = 0;

	m_fRadiusY = m_fFactorY1 = m_fFactorY2 = 0;
	m_fFactorYSpeed1 = m_fFactorYSpeed2 = 0;
}

void CBird::Tick()
{
	if (m_pShape == NULL) return;

	m_fRadian += (m_fRadianSpeed*s_fSecPerFrm);

	if (m_fRadian > 2*D3DX_PI) m_fRadian -= (2*D3DX_PI);
	
	m_fFactor1 += (m_fFactorSpeed1*s_fSecPerFrm);
	if (m_fFactor1 > 2*D3DX_PI) m_fFactor1 -= (2*D3DX_PI);
	
	m_fFactor2 += (m_fFactorSpeed2*s_fSecPerFrm);
	if (m_fFactor2 > 2*D3DX_PI) m_fFactor2 -= (2*D3DX_PI);

	m_fFactorY1 += (m_fFactorYSpeed1*s_fSecPerFrm);
	if (m_fFactorY1 > 2*D3DX_PI) m_fFactorY1 -= (2*D3DX_PI);
	
	m_fFactorY2 += (m_fFactorYSpeed2*s_fSecPerFrm);
	if (m_fFactorY2 > 2*D3DX_PI) m_fFactorY2 -= (2*D3DX_PI);

	// 위치계산
	__Vector3 vPos;
	 vPos.Set(	sinf(m_fFactor1)*m_fRadius*cosf(m_fRadian),
				m_fRadiusY*(sinf(m_fFactorY1)+cosf(m_fFactorY2))/2.0f,
				sinf(m_fFactor2)*m_fRadius*sinf(m_fRadian));

	 // 각도 계산
	 // 미분식을 이용하여 기울기 구하기
	float x = (m_fFactorSpeed1*cosf(m_fFactor1)*cosf(m_fRadian) - 
		 m_fRadianSpeed*sinf(m_fFactor1)*sinf(m_fRadian));
	float z = (m_fFactorSpeed2*cosf(m_fFactor2)*sinf(m_fRadian) + 
		 m_fRadianSpeed*sinf(m_fFactor2)*cosf(m_fRadian));
	float fRotY = atan2f(z, x)-(D3DX_PI/2);

	D3DXVECTOR3 vAxis(0, 1, 0);	 
	D3DXQUATERNION qt;
	D3DXQuaternionRotationAxis(&qt, &vAxis, -fRotY);

	__Vector3 vNewPos = m_vPivot+vPos;
	 
	m_pShape->RotSet(qt);
	m_pShape->PosSet(vNewPos);
	m_pShape->Tick(-1);

	m_fSndInterval += s_fSecPerFrm;

	if(m_pSnd && m_fSndInterval > 30.0f)
	{
		m_fSndInterval = (float)(-(rand()%20));
		m_pSnd->Play(&vNewPos);
	}
}

void CBird::Render()
{
	if (m_pShape == NULL) return;
	m_pShape->Render();
}

int CBird::LoadBird(const std::string& szFN)
{
	Release();
	FILE* stream = fopen(szFN.c_str(), "r"); //text파일로 만든다 
	if(NULL == stream)
	{
#if _DEBUG
		std::string szErr = fmt::format("failed to open file - {}", szFN);
		__ASSERT(stream, szErr.c_str());
#endif
		return false;
	}

	char szRrcName[_MAX_PATH];
	float	fSpeed = 0.0f;
	int result = fscanf(stream, "ResourceName = %s\n", szRrcName);			__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");
	result = fscanf(stream, "Pivot = %f %f %f\n", &(m_vPivot.x), &(m_vPivot.y), &(m_vPivot.z));	__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");
	result = fscanf(stream, "Radius = %f\n", &m_fRadius);					__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");
	result = fscanf(stream, "RadiusY = %f\n", &m_fRadiusY);					__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");
	result = fscanf(stream, "Speed = %f\n", &fSpeed);						__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	fclose(stream);

	__ASSERT(m_pShape == NULL, "Bird memory leak 가능성");
	m_pShape = new CN3Shape;
	m_pShape->LoadFromFile(szRrcName);

	m_fRadianSpeed = fSpeed/m_fRadius;

	m_fFactorSpeed1 = 0.1f + ((rand()%2000)/10000.0f);
	m_fFactorSpeed2 = 0.02f + ((rand()%4000)/100000.0f);

	m_fFactorYSpeed1 = 0.35f + ((rand()%3000)/10000.0f);
	m_fFactorYSpeed2 = 0.18f + ((rand()%3500)/10000.0f);

	if(NULL == m_pSnd) m_pSnd = CN3Base::s_SndMgr.CreateObj(1000);
	m_fSndInterval = 0.0f;

	return true;
}
