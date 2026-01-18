// N3Sun.cpp: implementation of the CN3Sun class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3Sun.h"
#include "N3Texture.h"

CN3Sun::CN3Sun()
{
	for (int i = 0; i < NUM_SUNPART; ++i)
	{
		m_Parts[i] = {};
		m_Parts[i].Color.ChangeColor(0xffffffff);
	}
	m_fCurRadian = 0.0f;
}

CN3Sun::~CN3Sun()
{
	for (int i = 0; i < NUM_SUNPART; i++)
		s_MngTex.Delete(&m_Parts[i].pTex);
}

void CN3Sun::Release()
{
	CN3Base::Release();

	for (int i = 0; i < NUM_SUNPART; i++)
	{
		s_MngTex.Delete(&m_Parts[i].pTex);
		m_Parts[i] = {};
		m_Parts[i].Color.ChangeColor(0xffffffff);
	}

	m_fCurRadian = 0.0f;
}

void CN3Sun::Render(__Matrix44& matView, __Matrix44& matProj)
{
	// 해가 그려지는 화면 좌표 계산하기 (2D로 그리는 이유는 왜곡되어서 보이는것을 막기 위해)
	__Matrix44 matWorld;
	matWorld.RotationZ(m_fCurRadian);
	__Matrix44 matFinal;
	matFinal  = matWorld * matView;
	matFinal *= matProj;

	__Vector3 vSun;
	vSun.Set(5, 0, 0);
	__Vector4 vOut;
	vOut.Transform(vSun, matFinal);

	int Width  = s_CameraData.vp.Width;
	int Height = s_CameraData.vp.Height;
	int X      = s_CameraData.vp.X;
	int Y      = s_CameraData.vp.Y;

	float rhw  = 1.0f / vOut.w;
	vSun.z     = vOut.z * rhw;
	if (vSun.z < 0.0f || vSun.z > 1.0f)
		return;

	// Mapping Screen Coordinate.
	vSun.x           = (float) X + int((vOut.x * rhw + 1) * Width * 0.5f);
	vSun.y           = (float) Y + int((-vOut.y * rhw + 1) * Height * 0.5f);

	// back up render state
	DWORD dwSrcBlend = 0, dwDestBlend = 0;
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);

	// set render state
	if (D3DBLEND_ONE != dwSrcBlend)
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);

	if (D3DBLEND_ONE != dwDestBlend)
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	s_lpD3DDev->SetFVF(FVF_TRANSFORMED);

	RECT rcSun[NUM_SUNPART];
	RECT rcScreen;
	SetRect(&rcScreen, X, Y, X + Width, Y + Width);
	for (int i = 0; i < NUM_SUNPART; ++i)
	{
		__SunPart* pSP = &m_Parts[i];

		float iWTmp    = (Width * pSP->Delta.GetCurDelta()) / 2;
		SetRect(&rcSun[i], (int) (vSun.x - iWTmp), (int) (vSun.y - iWTmp), (int) (vSun.x + iWTmp),
			(int) (vSun.y + iWTmp));

		// clipping with screen.
		if (rcSun[i].right < rcScreen.left || rcSun[i].bottom < rcScreen.top
			|| rcSun[i].left > rcScreen.right || rcSun[i].top > rcScreen.bottom)
			continue; // 화면 밖에 그려진다.

		// 2D로 그리기
		pSP->pVertices[0].x = (float) rcSun[i].left;
		pSP->pVertices[0].y = (float) rcSun[i].top;
		pSP->pVertices[1].x = (float) rcSun[i].right;
		pSP->pVertices[1].y = (float) rcSun[i].top;
		pSP->pVertices[2].x = (float) rcSun[i].right;
		pSP->pVertices[2].y = (float) rcSun[i].bottom;
		pSP->pVertices[3].x = (float) rcSun[i].left;
		pSP->pVertices[3].y = (float) rcSun[i].bottom;

		if (pSP->pTex)
			s_lpD3DDev->SetTexture(0, pSP->pTex->Get());
		else
			s_lpD3DDev->SetTexture(0, nullptr);
		s_lpD3DDev->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN, 2, pSP->pVertices, sizeof(__VertexTransformed));
	}

	// restore render state
	if (D3DBLEND_ONE != dwSrcBlend)
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	if (D3DBLEND_ONE != dwDestBlend)
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
}

void CN3Sun::Tick()
{
	// 해의 색, 크기 변화 계산
	for (int i = 0; i < NUM_SUNPART; i++)
	{
		m_Parts[i].Color.Tick();
		m_Parts[i].Delta.Tick();
		__VertexTransformed* pV = m_Parts[i].pVertices;
		pV[0].color = pV[1].color = pV[2].color = pV[3].color = m_Parts[i].Color.GetCurColor();
	}
}

void CN3Sun::Init(const std::string* pszFNs)
{
	Release();

	const float fZ       = 0.9f;
	const float rhw      = 1.0f;
	const D3DCOLOR color = 0xffffffff;
	for (int i = 0; i < NUM_SUNPART; ++i)
	{
		m_Parts[i].pTex = s_MngTex.Get(pszFNs[i]); // load texture

		m_Parts[i].pVertices[0].Set(0, 0, fZ, rhw, color, 0.0f, 0.0f);
		m_Parts[i].pVertices[1].Set(0, 0, fZ, rhw, color, 1.0f, 0.0f);
		m_Parts[i].pVertices[2].Set(0, 0, fZ, rhw, color, 1.0f, 1.0f);
		m_Parts[i].pVertices[3].Set(0, 0, fZ, rhw, color, 0.0f, 1.0f);
	}

	m_Parts[SUNPART_SUN].Delta.ChangeDelta(0.1f); // ViewPort 에서 상대적인 크기
	m_Parts[SUNPART_GLOW].Delta.ChangeDelta(0.25f);
	m_Parts[SUNPART_FLARE].Delta.ChangeDelta(0.13f);
}
