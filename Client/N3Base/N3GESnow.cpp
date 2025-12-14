// N3GESnow.cpp: implementation of the CN3GESnow class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3GESnow.h"
#include "N3Texture.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3GESnow::CN3GESnow()
{
	m_pTex = nullptr;
	m_pSnowParticle = nullptr;
	CN3GESnow::Release();
}

CN3GESnow::~CN3GESnow()
{
	CN3GESnow::Release();
}

void CN3GESnow::Release()
{
	m_fWidth = m_fHeight = 0.0f;
	m_vVelocity.Set(0,0,0);
	s_MngTex.Delete(&m_pTex);
	if (m_pSnowParticle) {delete [] m_pSnowParticle; m_pSnowParticle = nullptr;}
	CN3GlobalEffect::Release();
}

void CN3GESnow::Tick()
{
	if (m_bActive == FALSE || m_iVC <= 0 || m_pVB == nullptr) return;
	CN3GlobalEffect::Tick();

	int iCount = m_iVC/3;
	int iActiveCount = iCount;
	if(m_iFadeMode > 0) // 차차 많아지게 한다..
	{
		if(m_fFadeTime > 0 && m_fFadeTimeCur < m_fFadeTime) // Fade시간땜에 건너뛰고 찍을 양 결정..
		{
			iActiveCount = (int)(iCount * m_fFadeTimeCur / m_fFadeTime);
			if(iActiveCount > iCount) iActiveCount = iCount;
			else if(iActiveCount < 0) iActiveCount = 0;
		}
	}
	else if(m_iFadeMode < 0)
	{
		if(m_fFadeTime > 0 && m_fFadeTimeCur < m_fFadeTime) // Fade시간땜에 건너뛰고 찍을 양 결정..
		{
			iActiveCount = (int)(iCount * (1.0f - m_fFadeTimeCur / m_fFadeTime));
			if(iActiveCount > iCount) iActiveCount = iCount;
			else if(iActiveCount < 0) iActiveCount = 0;
		}
	}
	if(iActiveCount <= 0) return;

	int i;
	__VertexXyzT1* pVertices;
	HRESULT hr = m_pVB->Lock(0, 0, (void**)&pVertices, D3DLOCK_NOSYSLOCK);
	if (FAILED(hr))
		return;

	__Vector3	vN = m_vVelocity;	vN.Normalize();
	__Vector3	vAdd = m_vVelocity*s_fSecPerFrm;
	float		fAddRadian = __PI*s_fSecPerFrm*0.1f;

	const float fHalfWidth = m_fWidth/2.0f;
	const float fHalfHeight = m_fHeight/2.0f;
	const float fCurY = s_CameraData.vEye.y;

	static const float sqrt3 = sqrtf(3.0f);

	for (i=0; i<iActiveCount; ++i)
	{
		// 위치 결정하기
		__VertexXyzT1* pV1 = pVertices+i*3+0;
		__VertexXyzT1* pV2 = pVertices+i*3+1;
		__VertexXyzT1* pV3 = pVertices+i*3+2;
//		__Vector3 vOffset2 = __Vector3(pV2->v) - __Vector3(pV1->v);
//		__Vector3 vOffset3 = __Vector3(pV3->v) - __Vector3(pV1->v);

		__SnowParticle* pParticle = m_pSnowParticle + i;
		pParticle->vPos += vAdd;
//		pV1->x += vAdd.x;	pV1->y += vAdd.y;	pV1->z += vAdd.z;

		float fDiff = pParticle->vPos.y - (fCurY-fHalfHeight);
		if (fDiff < 0)	// 높이 범위를 벗어났을 경우
		{
			pParticle->vPos.y -= (((int)(fDiff/m_fHeight)-1)*m_fHeight);
			pParticle->vPos.x = m_fWidth*(rand()%10000-5000)/10000.f;
			pParticle->vPos.z = m_fWidth*(rand()%10000-5000)/10000.f;

			pParticle->fRadius = (rand()%10000)/10000.f;
			pParticle->fRadian = 2*__PI*((rand()%10000)/10000.f);

			float		fRadian = __PI*((rand()%10000)/10000.f);
			pParticle->vOffset1.Set(0, sqrt3*m_fSnowSize/3.f, 0);
			pParticle->vOffset2.Set(cosf(fRadian)*m_fSnowSize/2, -sqrt3*m_fSnowSize/6.f, sinf(fRadian)*m_fSnowSize/2);
			pParticle->vOffset3.Set(-cosf(fRadian)*m_fSnowSize/2, -sqrt3*m_fSnowSize/6.f, -sinf(fRadian)*m_fSnowSize/2);
		}
		else
		{
			fDiff = pParticle->vPos.y - (fCurY+fHalfHeight);
			if (fDiff > 0)	// 높이 범위를 반대로 벗어났을경우
				pParticle->vPos.y -= ((int)(fDiff/m_fHeight)+1)*m_fHeight;
			// x 너비 범위를 벗어났을 경우
			fDiff = pParticle->vPos.x - fHalfWidth;
			if (fDiff > 0) pParticle->vPos.x -= ((int)(fDiff/m_fWidth)+1)*m_fWidth;
			fDiff = pParticle->vPos.x + fHalfWidth;
			if (fDiff < 0) pParticle->vPos.x -= ((int)(fDiff/m_fWidth)-1)*m_fWidth;
			// z 너비 범위를 벗어났을 경우
			fDiff = pParticle->vPos.z - fHalfWidth;
			if (fDiff >  0) pParticle->vPos.z -= ((int)(fDiff/m_fWidth)+1)*m_fWidth;
			fDiff = pParticle->vPos.z + fHalfWidth;
			if (fDiff < 0) pParticle->vPos.z -= ((int)(fDiff/m_fWidth)-1)*m_fWidth;
		}
		// 중심축을 주위로 회전한 위치 계산
		pParticle->fRadian += fAddRadian;
		__Vector3 vPos;	vPos.Set(cosf(pParticle->fRadian), 0, sinf(pParticle->fRadian));
		vPos += pParticle->vPos;
		
		// 버텍스 버퍼의 점 다시 세팅하기
		pV1->x = vPos.x + pParticle->vOffset1.x;	pV1->y = vPos.y + pParticle->vOffset1.y;	pV1->z = vPos.z + pParticle->vOffset1.z;
		pV2->x = vPos.x + pParticle->vOffset2.x;	pV2->y = vPos.y + pParticle->vOffset2.y;	pV2->z = vPos.z + pParticle->vOffset2.z;
		pV3->x = vPos.x + pParticle->vOffset3.x;	pV3->y = vPos.y + pParticle->vOffset3.y;	pV3->z = vPos.z + pParticle->vOffset3.z;
	}
	m_pVB->Unlock();
}

void CN3GESnow::Render(__Vector3& vPos)
{
	if (m_bActive == FALSE || m_iVC <= 0 || m_pVB == nullptr) return;
	
	CN3GlobalEffect::Render(vPos);

	int iCount = m_iVC / 3;
	int iActiveCount = iCount;
	if(m_iFadeMode > 0) // 차차 많아지게 한다..
	{
		if(m_fFadeTime > 0 && m_fFadeTimeCur < m_fFadeTime) // Fade시간땜에 건너뛰고 찍을 양 결정..
		{
			iActiveCount = (int)(iCount * m_fFadeTimeCur / m_fFadeTime);
			if(iActiveCount > iCount) iActiveCount = iCount;
			else if(iActiveCount < 0) iActiveCount = 0;
		}
	}
	else if(m_iFadeMode < 0)
	{
		if(m_fFadeTime > 0 && m_fFadeTimeCur < m_fFadeTime) // Fade시간땜에 건너뛰고 찍을 양 결정..
		{
			iActiveCount = (int)(iCount * (1.0f - m_fFadeTimeCur / m_fFadeTime));
			if(iActiveCount > iCount) iActiveCount = iCount;
			else if(iActiveCount < 0) iActiveCount = 0;
		}
	}
	if(iActiveCount <= 0) return;

	// 이전 render state 저장
	DWORD dwAlphaBlend, dwSrcAlpha, dwDestAlpha, dwCullMode, dwLight;
	s_lpD3DDev->GetRenderState( D3DRS_ALPHABLENDENABLE, &dwAlphaBlend );
	s_lpD3DDev->GetRenderState( D3DRS_SRCBLEND, &dwSrcAlpha );
	s_lpD3DDev->GetRenderState( D3DRS_DESTBLEND, &dwDestAlpha );
	s_lpD3DDev->GetRenderState( D3DRS_CULLMODE, &dwCullMode );
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);

	DWORD dwAddressU, dwAddressV;
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_ADDRESSU, &dwAddressU);
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_ADDRESSV, &dwAddressV);

	// set render state
	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA);
	s_lpD3DDev->SetRenderState(D3DRS_CULLMODE,   D3DCULL_NONE);
	s_lpD3DDev->SetRenderState(D3DRS_LIGHTING,	 FALSE);

	// set transform
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &m_Matrix);

	// set texture 
	__ASSERT(m_pTex, "Texture pointer is NULL!");
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0xffff0000);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	s_lpD3DDev->SetTexture(0, m_pTex->Get());

	// render
	s_lpD3DDev->SetFVF(FVF_XYZT1);
	s_lpD3DDev->SetStreamSource(0, m_pVB, 0, sizeof(__VertexXyzT1)); // 버텍스 버퍼 지정
	s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, iActiveCount);

	// restore
	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND,   dwSrcAlpha);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND,  dwDestAlpha);
	s_lpD3DDev->SetRenderState( D3DRS_CULLMODE, dwCullMode );
	s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, dwAddressU);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, dwAddressV);
}

void CN3GESnow::Create(float fDensity, float fWidth, float fHeight, float fSnowSize, const __Vector3& vVelocity, float fTimeToFade)
{
	if(nullptr == s_lpD3DDev) return;

	Release();

	CN3GlobalEffect::FadeSet(fTimeToFade, true);

	m_fWidth = fWidth;	m_fHeight = fHeight;	m_fSnowSize = fSnowSize;
	m_vVelocity = vVelocity;
	float fVolume = m_fWidth*m_fWidth*fHeight;
	__ASSERT(fVolume>0, "Snow volume is less than 0");
	int iSnowCount = (int)(fVolume*fDensity);

	// m_pVB, m_pIB 만들기
	__ASSERT(s_lpD3DDev, "D3D Device pointer is NULL!");
	m_iVC = iSnowCount*3;
	HRESULT hr = s_lpD3DDev->CreateVertexBuffer(m_iVC*sizeof(__VertexXyzT1), D3DUSAGE_DYNAMIC, FVF_XYZT1, D3DPOOL_DEFAULT, &m_pVB, nullptr);
	if (FAILED(hr)) return;
	__VertexXyzT1* pVertices;
	hr = m_pVB->Lock(0, iSnowCount*3*sizeof(__VertexXyzT1), (void**)&pVertices, D3DLOCK_NOSYSLOCK);
	if (FAILED(hr)) return;

	// __SnowParticle 정보 채워 넣기
	m_pSnowParticle = new __SnowParticle[iSnowCount];

	const float sqrt3 = sqrtf(3.0f);
	int i;
	for (i=0; i<iSnowCount; ++i)
	{
		m_pSnowParticle[i].vPos.Set(fWidth*(rand()%10000-5000)/10000.f,
									fHeight*(rand()%10000-5000)/10000.f,
									fWidth*(rand()%10000-5000)/10000.f);
		m_pSnowParticle[i].fRadius = (rand()%10000)/10000.f;
		m_pSnowParticle[i].fRadian = 2*__PI*((rand()%10000)/10000.f);

		float		fRadian = __PI*((rand()%10000)/10000.f);
//		정삼각형(한변의 길이가 fSnowSize)
//		m_pSnowParticle[i].vOffset1.Set(0, sqrt3*fSnowSize/3.f, 0);
//		m_pSnowParticle[i].vOffset2.Set(cosf(fRadian)*fSnowSize/2, -sqrt3*fSnowSize/6.f, sinf(fRadian)*fSnowSize/2);
//		m_pSnowParticle[i].vOffset3.Set(-cosf(fRadian)*fSnowSize/2, -sqrt3*fSnowSize/6.f, -sinf(fRadian)*fSnowSize/2);

//		이등변 삼각형(밑변의 길이 fSnowSize, 높이 fSnowSize)
		m_pSnowParticle[i].vOffset1.Set(0, fSnowSize/2.f, 0);
		m_pSnowParticle[i].vOffset2.Set(cosf(fRadian)*fSnowSize/2.f, -fSnowSize/2.f, sinf(fRadian)*fSnowSize/2.f);
		m_pSnowParticle[i].vOffset3.Set(-cosf(fRadian)*fSnowSize/2.f, -fSnowSize/2.f, -sinf(fRadian)*fSnowSize/2.f);

		// uv좌표 넣기
		__VertexXyzT1* pV1 = pVertices + i*3,	*pV2 = pVertices + i*3+1,	*pV3 = pVertices + i*3+2;
// 정삼각형에 눈 동그라미가 삼각형에 꽉 차게 UV좌표 배치 (geforce2카드에서 border color가 제대로 되지 않아서..)
//		pV1->tu = 0.5f;	pV1->tv = 0.5f - sqrt3/2.f;
//		pV2->tu = 0.5f + sqrt3/2.f;	pV2->tv = 1.0f;
//		pV3->tu = 0.5f - sqrt3/2.f;	pV3->tv = 1.0f;

		// 이등변 삼각형에 UV좌표 넣기
		pV1->tu = 0.5f;	pV1->tv = 0.0f;
		pV2->tu = 1.0f;	pV2->tv = 1.0f;
		pV3->tu = 0.0f;	pV3->tv = 1.0f;

		// 이 방식은 눈텍스쳐 사각형에 삼각형을 넣는 방식(따라서 눈 텍스쳐를 삼각형에 맞게 그려주자)

	}

	m_pVB->Unlock();
	m_pTex = s_MngTex.Get("Misc\\Snow.DXT", TRUE);
}
