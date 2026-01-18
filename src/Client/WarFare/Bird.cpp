// Bird.cpp: implementation of the CBird class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Bird.h"
#include <N3Base/N3Shape.h>
#include <N3Base/N3SndObj.h>

CBird::CBird()
{
	m_pSnd           = nullptr;
	m_pShape         = nullptr;

	m_vPivot         = {};
	m_fRadius        = 0.0f;
	m_fRadian        = 0.0f;

	m_fFactor1       = 0.0f;
	m_fFactor2       = 0.0f;
	m_fFactorSpeed1  = 0.0f;
	m_fFactorSpeed2  = 0.0f;

	m_fRadiusY       = 0.0f;
	m_fFactorY1      = 0.0f;
	m_fFactorY2      = 0.0f;
	m_fFactorYSpeed1 = 0.0f;
	m_fFactorYSpeed2 = 0.0f;

	m_fRadianSpeed   = 0.0f;
	m_fSndInterval   = 0.0f;
}

CBird::~CBird()
{
	CBird::Release();
}

void CBird::Release()
{
	s_SndMgr.ReleaseObj(&m_pSnd);

	delete m_pShape;
	m_pShape         = nullptr;

	m_vPivot         = {};
	m_fRadius        = 0.0f;
	m_fRadian        = 0.0f;

	m_fFactor1       = 0.0f;
	m_fFactor2       = 0.0f;
	m_fFactorSpeed1  = 0.0f;
	m_fFactorSpeed2  = 0.0f;

	m_fRadiusY       = 0.0f;
	m_fFactorY1      = 0.0f;
	m_fFactorY2      = 0.0f;
	m_fFactorYSpeed1 = 0.0f;
	m_fFactorYSpeed2 = 0.0f;

	m_fRadianSpeed   = 0.0f;
	m_fSndInterval   = 0.0f;
}

void CBird::Tick()
{
	if (m_pShape == nullptr)
		return;

	m_fRadian += (m_fRadianSpeed * s_fSecPerFrm);

	if (m_fRadian > 2 * __PI)
		m_fRadian -= (2 * __PI);

	m_fFactor1 += (m_fFactorSpeed1 * s_fSecPerFrm);
	if (m_fFactor1 > 2 * __PI)
		m_fFactor1 -= (2 * __PI);

	m_fFactor2 += (m_fFactorSpeed2 * s_fSecPerFrm);
	if (m_fFactor2 > 2 * __PI)
		m_fFactor2 -= (2 * __PI);

	m_fFactorY1 += (m_fFactorYSpeed1 * s_fSecPerFrm);
	if (m_fFactorY1 > 2 * __PI)
		m_fFactorY1 -= (2 * __PI);

	m_fFactorY2 += (m_fFactorYSpeed2 * s_fSecPerFrm);
	if (m_fFactorY2 > 2 * __PI)
		m_fFactorY2 -= (2 * __PI);

	// 위치계산
	__Vector3 vPos;
	vPos.Set(sinf(m_fFactor1) * m_fRadius * cosf(m_fRadian), m_fRadiusY * (sinf(m_fFactorY1) + cosf(m_fFactorY2)) / 2.0f,
		sinf(m_fFactor2) * m_fRadius * sinf(m_fRadian));

	// 각도 계산
	// 미분식을 이용하여 기울기 구하기
	float x     = (m_fFactorSpeed1 * cosf(m_fFactor1) * cosf(m_fRadian) - m_fRadianSpeed * sinf(m_fFactor1) * sinf(m_fRadian));
	float z     = (m_fFactorSpeed2 * cosf(m_fFactor2) * sinf(m_fRadian) + m_fRadianSpeed * sinf(m_fFactor2) * cosf(m_fRadian));
	float fRotY = atan2f(z, x) - (__PI / 2);

	__Vector3 vAxis(0, 1, 0);
	__Quaternion qt;
	qt.RotationAxis(vAxis, -fRotY);

	__Vector3 vNewPos = m_vPivot + vPos;

	m_pShape->RotSet(qt);
	m_pShape->PosSet(vNewPos);
	m_pShape->Tick(-1);

	m_fSndInterval += s_fSecPerFrm;

	if (m_pSnd && m_fSndInterval > 30.0f)
	{
		m_fSndInterval = (float) (-(rand() % 20));
		m_pSnd->Play(&vNewPos);
	}
}

void CBird::Render()
{
	if (m_pShape != nullptr)
		m_pShape->Render();
}

int CBird::LoadBird(const std::string& szFN)
{
	Release();

	FILE* stream = fopen(szFN.c_str(), "r"); //text파일로 만든다
	if (stream == nullptr)
	{
#if _DEBUG
		std::string szErr = fmt::format("failed to open file - {}", szFN);
		__ASSERT(stream, szErr.c_str());
#endif
		return false;
	}

	char szRrcName[_MAX_PATH + 1] {};
	float fSpeed = 0.0f;

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	int result   = fscanf(stream, "ResourceName = %s\n", szRrcName);
	__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	if (result == EOF)
	{
		fclose(stream);
		return false;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	result = fscanf(stream, "Pivot = %f %f %f\n", &(m_vPivot.x), &(m_vPivot.y), &(m_vPivot.z));
	__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	if (result == EOF)
	{
		fclose(stream);
		return false;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	result = fscanf(stream, "Radius = %f\n", &m_fRadius);
	__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	if (result == EOF)
	{
		fclose(stream);
		return false;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	result = fscanf(stream, "RadiusY = %f\n", &m_fRadiusY);
	__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	if (result == EOF)
	{
		fclose(stream);
		return false;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	result = fscanf(stream, "Speed = %f\n", &fSpeed);
	__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	fclose(stream);

	__ASSERT(m_pShape == nullptr, "Bird memory leak 가능성");
	m_pShape = new CN3Shape;
	m_pShape->LoadFromFile(szRrcName);

	m_fRadianSpeed   = fSpeed / m_fRadius;

	m_fFactorSpeed1  = 0.1f + ((rand() % 2000) / 10000.0f);
	m_fFactorSpeed2  = 0.02f + ((rand() % 4000) / 100000.0f);

	m_fFactorYSpeed1 = 0.35f + ((rand() % 3000) / 10000.0f);
	m_fFactorYSpeed2 = 0.18f + ((rand() % 3500) / 10000.0f);

	if (m_pSnd == nullptr)
		m_pSnd = s_SndMgr.CreateObj(1000);
	m_fSndInterval = 0.0f;

	return true;
}
