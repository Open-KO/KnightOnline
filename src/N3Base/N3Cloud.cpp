// N3Cloud.cpp: implementation of the CN3Cloud class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3Cloud.h"
#include "N3Texture.h"

CN3Cloud::CN3Cloud()
{
	for (int i = 0; i < NUM_CLOUD; i++)
		m_pTextures[i] = nullptr;

	m_Color1.ChangeColor(0xffffffff);
	m_Color2.ChangeColor(0xffffffff);
	m_Alpha.ChangeColor(0xffffffff);

	m_eCloud1       = CLOUD_WISPS;
	m_eCloud2       = CLOUD_PUFFS;
	m_eCloud3       = CLOUD_NONE;

	m_fCloudTexTime = 0.0f;
	m_eBackupCloud  = CLOUD_NONE;
	m_fBackupTime   = 0.0f;

	memset(&m_pVertices, 0, sizeof(m_pVertices));
}

CN3Cloud::~CN3Cloud()
{
	for (int i = 0; i < NUM_CLOUD; ++i)
		s_MngTex.Delete(&m_pTextures[i]);
}

void CN3Cloud::Release()
{
	CN3Base::Release();

	for (int i = 0; i < NUM_CLOUD; ++i)
	{
		s_MngTex.Delete(&m_pTextures[i]);
		m_szTextures[i].clear();
	}

	m_Color1.ChangeColor(0xffffffff);
	m_Color2.ChangeColor(0xffffffff);
	m_Alpha.ChangeColor(0xffffffff);
	m_eCloud1       = CLOUD_WISPS;
	m_eCloud2       = CLOUD_PUFFS;
	m_eCloud3       = CLOUD_NONE;

	m_fCloudTexTime = 0.0f;
	m_eBackupCloud  = CLOUD_NONE;
	m_fBackupTime   = 0.0f;
}

void CN3Cloud::Tick()
{
	static float fCloudLayer  = 0.0f;
	fCloudLayer              += s_fSecPerFrm;

	// 구름층 움직이기
	if (fCloudLayer >= 0.05)
	{
		//		float du = 0.001*fCloudLayer;
		//		float dv = 0.003*fCloudLayer;
		//		float du2 = 0.003*fCloudLayer;
		//		float dv2 = 0.005*fCloudLayer;
		float du  = 0.005f * fCloudLayer;
		float dv  = 0.015f * fCloudLayer;
		float du2 = 0.015f * fCloudLayer;
		float dv2 = 0.025f * fCloudLayer;

		for (int i = 0; i < NUM_CLOUD_VERTEX; ++i)
		{
			m_pVertices[i].tu  += du;
			m_pVertices[i].tv  += dv;
			m_pVertices[i].tu2 += du2;
			m_pVertices[i].tv2 += dv2;
		}

		fCloudLayer = 0.0f;

		if (m_pVertices[0].tu > 10.0f)
		{
			for (int i = 0; i < NUM_CLOUD_VERTEX; ++i)
				m_pVertices[i].tu -= 10.0f;
		}

		if (m_pVertices[0].tv > 10.0f)
		{
			for (int i = 0; i < NUM_CLOUD_VERTEX; ++i)
				m_pVertices[i].tv -= 10.0f;
		}
	}

	// 색 변화하기
	m_Color1.Tick();
	m_Color2.Tick();

	// 구름 교체
	if (CLOUD_NONE != m_eCloud3)
	{
		m_Alpha.Tick();
		m_fCloudTexTime -= s_fSecPerFrm;
		if (m_fCloudTexTime < 0.0f) // 구름 교체가 끝났으면
		{
			m_fCloudTexTime = 0.0f;
			m_eCloud2       = m_eCloud3;
			m_eCloud3       = CLOUD_NONE;
			m_Alpha.ChangeColor(0xffffffff);

			if (CLOUD_NONE != m_eBackupCloud) // 구름 바꿔야 할 것이 있으면
			{
				e_CLOUDTEX eTmp = m_eCloud1;
				m_eCloud1       = m_eCloud2;
				m_eCloud2       = eTmp;

				m_eCloud3       = m_eBackupCloud;
				m_fCloudTexTime = m_fBackupTime;
				m_eBackupCloud  = CLOUD_NONE;
				m_fBackupTime   = 0.0f;
				m_Alpha.ChangeColor(0x00ffffff, m_fCloudTexTime);

				// uv 좌표도 바꾸기
				for (int i = 0; i < NUM_CLOUD_VERTEX; ++i)
				{
					float fTempUV      = m_pVertices[i].tu;
					m_pVertices[i].tu  = m_pVertices[i].tu2;
					m_pVertices[i].tu2 = fTempUV;
					fTempUV            = m_pVertices[i].tv;
					m_pVertices[i].tv  = m_pVertices[i].tv2;
					m_pVertices[i].tv2 = fTempUV;
				}
			}
		}
	}
}

void CN3Cloud::Render()
{
	__Matrix44 matWorld;
	matWorld.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, matWorld.toD3D());

	static uint16_t CloudIndex[30] = { 0, 1, 4, 1, 2, 5, 2, 3, 6, 3, 0, 7, 5, 4, 1, 6, 5, 2, 7, 6,
		3, 4, 7, 0, 4, 5, 7, 5, 6, 7 };
	//static uint16_t CloudIndex[54] = {0,1,4,1,2,5,2,3,6,3,0,7,5,4,1,6,5,2,7,6,3,4,7,0,
	///							4,5,8,5,6,9,6,7,10,7,4,11,9,8,5,10,9,6,11,10,7,8,11,4,
	//							8,9,11,9,10,11};

	// backup state
	DWORD dwAlphaOp = 0, dwAlphaArg1 = 0, dwAlphaArg2 = 0;
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOp);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg1);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG2, &dwAlphaArg2);
	// set state
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	if (D3DTOP_MODULATE != dwAlphaOp)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	if (D3DTA_TEXTURE != dwAlphaArg1)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	if (D3DTA_DIFFUSE != dwAlphaArg2)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	s_lpD3DDev->SetFVF(FVF_XYZCOLORT2);

	D3DCOLOR color1 = m_Color1.GetCurColor();
	D3DCOLOR color2 = m_Color2.GetCurColor();
	__ASSERT(CLOUD_NONE != m_eCloud1 && CLOUD_NONE != m_eCloud2, "no cloud texture type");

	// render cloud 1
	int i = 0;
	for (; i < 4; ++i)
		m_pVertices[i].color = color1 & 0x00ffffff;
	for (i = 4; i < NUM_CLOUD_VERTEX; ++i)
		m_pVertices[i].color = color1;
	s_lpD3DDev->SetTexture(0, GetTex(m_eCloud1));
	s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 10, CloudIndex, D3DFMT_INDEX16,
		m_pVertices, sizeof(__VertexXyzColorT2));

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);
	if (CLOUD_NONE != m_eCloud3)
	{
		D3DCOLOR Alpha = m_Alpha.GetCurColor();

		// 기존 색 변화의 alpha값이 구름 교체alpha값보다 큰 경우 구름 교체 alpha값으로 대체
		if (Alpha < color2)
			color2 = (Alpha & 0xff000000) | (color2 & 0x00ffffff);

		// render cloud 2
		for (i = 0; i < 4; ++i)
			m_pVertices[i].color = color2 & 0x00ffffff;

		for (i = 4; i < NUM_CLOUD_VERTEX; ++i)
			m_pVertices[i].color = color2;

		s_lpD3DDev->SetTexture(0, GetTex(m_eCloud2));
		s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 10, CloudIndex, D3DFMT_INDEX16,
			m_pVertices, sizeof(__VertexXyzColorT2));
		// render cloud 3
		D3DCOLOR color3 = ((0xff - (color2 >> 24)) << 24)
						  | (color2 & 0x00ffffff); // color2의 alpha값을 0xff에서 뺀 값으로 바꿈
		for (i = 4; i < NUM_CLOUD_VERTEX; ++i)
			m_pVertices[i].color = color3;
		s_lpD3DDev->SetTexture(0, GetTex(m_eCloud3));
		s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 10, CloudIndex, D3DFMT_INDEX16,
			m_pVertices, sizeof(__VertexXyzColorT2));
	}
	else
	{
		// render cloud 2
		for (i = 0; i < 4; ++i)
			m_pVertices[i].color = color2 & 0x00ffffff;
		for (i = 4; i < NUM_CLOUD_VERTEX; ++i)
			m_pVertices[i].color = color2;
		s_lpD3DDev->SetTexture(0, GetTex(m_eCloud2));
		s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 10, CloudIndex, D3DFMT_INDEX16,
			m_pVertices, sizeof(__VertexXyzColorT2));
	}
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

	// restore state
	if (D3DTOP_MODULATE != dwAlphaOp)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	if (D3DTA_TEXTURE != dwAlphaArg1)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	if (D3DTA_DIFFUSE != dwAlphaArg2)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
}

LPDIRECT3DTEXTURE9 CN3Cloud::GetTex(e_CLOUDTEX tex)
{
	if (nullptr == m_pTextures[tex])
	{
		m_pTextures[tex] = s_MngTex.Get(m_szTextures[tex]);
		if (nullptr == m_pTextures[tex])
			return nullptr;
	}

	return m_pTextures[tex]->Get();
}

void CN3Cloud::Init(const std::string* pszFNs)
{
	Release();

	for (int i = 0; i < NUM_CLOUD; i++)
		m_szTextures[i] = pszFNs[i];

	//	3단일때
	/*	const float fSqrt3 = 1.0f/sqrtf(3.0f);
	const float fOffset = 3.0f;
	const float fSmallLength = 5.0f;
	const float fMediumLength = fSmallLength + fOffset;
	const float fBigLenth = fMediumLength + fOffset*fSqrt3;
	const float fBigHeight = 0.0f;
	const float fMediumHeight = fBigHeight + fOffset;
	const float fSmallHeight = fMediumHeight + fOffset*fSqrt3;

	D3DCOLOR BigColor = 0x00ffffff;
	D3DCOLOR MediumColor = 0xffffffff;
	D3DCOLOR SmallColor = 0xffffffff;

	float fTmp1, fTmp2;
	fTmp1 = fBigLenth-fMediumLength;
	fTmp2 = fMediumHeight - fBigHeight;
	const float fBig = sqrtf(fTmp1*fTmp1 + fTmp2*fTmp2);	// 젤큰 사각형과 중간사각형간의 거리
	fTmp1 = fMediumLength - fSmallLength;
	fTmp2 = fSmallHeight - fMediumHeight;
	const float fMedium = sqrtf(fTmp1*fTmp1 + fTmp2*fTmp2);	// 중간 사각형과 작은사각형간의 거리
	const float fSmall = fSmallLength;						// 작은 사각형이 0에서 떨어진 거리
	const float fTotal = fBig + fMedium + fSmall;

	float fTexUVLeft = 0.0f, fTexUVTop = 0.0f, fTexUVRight = 3.0f, fTexUVBottom = 3.0f;	// 텍스쳐 구름으로 표시할 영역 좌표(텍스쳐의 전체가 될수도 있고 일부분이 될 수도 있기 때문에)
//	const float fTexOffsetU_M = (fBig/fTotal)*(fTexUVRight-fTexUVLeft)/2;
//	const float fTexOffsetV_M = (fBig/fTotal)*(fTexUVBottom-fTexUVTop)/2;
//	const float fTexOffsetU_S = ((fBig+fMedium)/fTotal)*(fTexUVRight-fTexUVLeft)/2;
//	const float fTexOffsetV_S = ((fBig+fMedium)/fTotal)*(fTexUVBottom-fTexUVTop)/2;

	const float fTexOffsetU_M = (1.0f - fMediumLength/fBigLenth)*(fTexUVRight-fTexUVLeft)/2;
	const float fTexOffsetV_M = (1.0f - fMediumLength/fBigLenth)*(fTexUVBottom-fTexUVTop)/2;
	const float fTexOffsetU_S = (1.0f - fSmallLength/fBigLenth)*(fTexUVRight-fTexUVLeft)/2;
	const float fTexOffsetV_S = (1.0f - fSmallLength/fBigLenth)*(fTexUVBottom-fTexUVTop)/2;

	// big
	m_pVertices[0].Set(-fBigLenth, fBigHeight,-fBigLenth, BigColor, fTexUVLeft,	fTexUVTop,		fTexUVLeft,		fTexUVTop);
	m_pVertices[1].Set( fBigLenth, fBigHeight,-fBigLenth, BigColor, fTexUVRight,	fTexUVTop,		fTexUVRight,	fTexUVTop);
	m_pVertices[2].Set( fBigLenth, fBigHeight, fBigLenth, BigColor, fTexUVRight,	fTexUVBottom,	fTexUVRight,	fTexUVBottom);
	m_pVertices[3].Set(-fBigLenth, fBigHeight, fBigLenth, BigColor, fTexUVLeft,	fTexUVBottom,	fTexUVLeft,		fTexUVBottom);
	// medium
	m_pVertices[4].Set(-fMediumLength, fMediumHeight,-fMediumLength, MediumColor, fTexUVLeft+fTexOffsetU_M,	fTexUVTop+fTexOffsetV_M,		fTexUVLeft+fTexOffsetU_M,	fTexUVTop+fTexOffsetV_M);
	m_pVertices[5].Set( fMediumLength, fMediumHeight,-fMediumLength, MediumColor, fTexUVRight-fTexOffsetU_M, fTexUVTop+fTexOffsetV_M,		fTexUVRight-fTexOffsetU_M, fTexUVTop+fTexOffsetV_M);
	m_pVertices[6].Set( fMediumLength, fMediumHeight, fMediumLength, MediumColor, fTexUVRight-fTexOffsetU_M, fTexUVBottom-fTexOffsetV_M,	fTexUVRight-fTexOffsetU_M, fTexUVBottom-fTexOffsetV_M);
	m_pVertices[7].Set(-fMediumLength, fMediumHeight, fMediumLength, MediumColor, fTexUVLeft+fTexOffsetU_M,	fTexUVBottom-fTexOffsetV_M,	fTexUVLeft+fTexOffsetU_M,	fTexUVBottom-fTexOffsetV_M);

	// small
	m_pVertices[8].Set( -fSmallLength, fSmallHeight,-fSmallLength, SmallColor, fTexUVLeft+fTexOffsetU_S,	fTexUVTop+fTexOffsetV_S,		fTexUVLeft+fTexOffsetU_S,	fTexUVTop+fTexOffsetV_S);
	m_pVertices[9].Set(  fSmallLength, fSmallHeight,-fSmallLength, SmallColor, fTexUVRight-fTexOffsetU_S, fTexUVTop+fTexOffsetV_S,		fTexUVRight-fTexOffsetU_S, fTexUVTop+fTexOffsetV_S);
	m_pVertices[10].Set( fSmallLength, fSmallHeight, fSmallLength, SmallColor, fTexUVRight-fTexOffsetU_S, fTexUVBottom-fTexOffsetV_S,	fTexUVRight-fTexOffsetU_S, fTexUVBottom-fTexOffsetV_S);
	m_pVertices[11].Set(-fSmallLength, fSmallHeight, fSmallLength, SmallColor, fTexUVLeft+fTexOffsetU_S,	fTexUVBottom-fTexOffsetV_S,	fTexUVLeft+fTexOffsetU_S,	fTexUVBottom-fTexOffsetV_S);
*/
	// 2단일때
	const float fSqrt3       = 1.0f / sqrtf(3.0f);
	float fBigLenth          = 16.0f;          // 구름 절두면체의 아래 큰 사각형 길이
	const float fSmallLength = 8.0f;           // 구름 절두면체의 위 작은 사각형 길이
	const float fBigHeight   = 5.0f;           // 구름 사각절두면체의 아래 높이(지평선 0.0f 기준)
	const float fSmallHeight = fBigHeight
							   + (fBigLenth - fSmallLength)
									 * fSqrt3; // 구름 절두면체의 위 높이(지평선 0.0f 기준)

	fBigLenth           = 24.0f; //위에 코드는 최적화 각도, 이것은 큰 사격형만 다시 늘림

	D3DCOLOR BigColor   = 0x00ffffff;
	D3DCOLOR SmallColor = 0xffffffff;

	// 텍스쳐 구름으로 표시할 영역 좌표(텍스쳐의 전체가 될수도 있고 일부분이 될 수도 있기 때문에)
	float fTexUVLeft = 0.0f, fTexUVTop = 0.0f, fTexUVRight = 4.0f, fTexUVBottom = 4.0f;

	//  const float fTmp1  = fBigLenth - fSmallLength;
	//  const float fTmp2 = fSmallHeight - fBigHeight;
	//	const float fBig = sqrtf(fTmp1*fTmp1 + fTmp2*fTmp2);
	//	const float fSmall = fSmallLength;
	//	const float fTotal = fBig + fSmall;
	//	const float fTexOffsetU = (fSmall/fTotal)*(fTexUVRight-fTexUVLeft)/2;
	//	const float fTexOffsetV = (fSmall/fTotal)*(fTexUVBottom-fTexUVTop)/2;
	const float fTexOffsetU = (1.0f - fSmallLength / fBigLenth) * (fTexUVRight - fTexUVLeft) / 2;
	const float fTexOffsetV = (1.0f - fSmallLength / fBigLenth) * (fTexUVBottom - fTexUVTop) / 2;
	// big
	m_pVertices[0].Set(
		-fBigLenth, fBigHeight, -fBigLenth, BigColor, fTexUVLeft, fTexUVTop, fTexUVLeft, fTexUVTop);
	m_pVertices[1].Set(fBigLenth, fBigHeight, -fBigLenth, BigColor, fTexUVRight, fTexUVTop,
		fTexUVRight, fTexUVTop);
	m_pVertices[2].Set(fBigLenth, fBigHeight, fBigLenth, BigColor, fTexUVRight, fTexUVBottom,
		fTexUVRight, fTexUVBottom);
	m_pVertices[3].Set(-fBigLenth, fBigHeight, fBigLenth, BigColor, fTexUVLeft, fTexUVBottom,
		fTexUVLeft, fTexUVBottom);
	// small
	m_pVertices[4].Set(-fSmallLength, fSmallHeight, -fSmallLength, SmallColor,
		fTexUVLeft + fTexOffsetU, fTexUVTop + fTexOffsetV, fTexUVLeft + fTexOffsetU,
		fTexUVTop + fTexOffsetV);
	m_pVertices[5].Set(fSmallLength, fSmallHeight, -fSmallLength, SmallColor,
		fTexUVRight - fTexOffsetU, fTexUVTop + fTexOffsetV, fTexUVRight - fTexOffsetU,
		fTexUVTop + fTexOffsetV);
	m_pVertices[6].Set(fSmallLength, fSmallHeight, fSmallLength, SmallColor,
		fTexUVRight - fTexOffsetU, fTexUVBottom - fTexOffsetV, fTexUVRight - fTexOffsetU,
		fTexUVBottom - fTexOffsetV);
	m_pVertices[7].Set(-fSmallLength, fSmallHeight, fSmallLength, SmallColor,
		fTexUVLeft + fTexOffsetU, fTexUVBottom - fTexOffsetV, fTexUVLeft + fTexOffsetU,
		fTexUVBottom - fTexOffsetV);
}

void CN3Cloud::SetCloud(e_CLOUDTEX eCloud1, e_CLOUDTEX eCloud2, float fSec)
{
	if (CLOUD_NONE == eCloud1 || CLOUD_NONE == eCloud2)
		return;

	// 변수 초기화
	m_Alpha.ChangeColor(0xffffffff);
	m_fCloudTexTime = 0.0f;
	m_eCloud3       = CLOUD_NONE;
	m_eBackupCloud  = CLOUD_NONE;
	m_fBackupTime   = 0.0f;

	// 바꿀 구름 판단
	int iSameCount  = 0;
	if (eCloud1 == m_eCloud1 || eCloud1 == m_eCloud2)
		++iSameCount;

	if (eCloud2 == m_eCloud1 || eCloud2 == m_eCloud2)
		++iSameCount;

	// 같은 구름이므로 바꿀 필요가 없다
	if (2 == iSameCount)
		return;

	// 하나만 교체하면 된다.
	if (1 == iSameCount)
	{
		if (m_eCloud1 == eCloud1)
		{
			m_eCloud3 = eCloud2;
		}
		else if (m_eCloud1 == eCloud2)
		{
			m_eCloud3 = eCloud1;
		}
		else if (m_eCloud2 == eCloud1)
		{
			m_eCloud2 = m_eCloud1;
			m_eCloud1 = eCloud1;
			m_eCloud3 = eCloud2;
		}
		else
		{
			m_eCloud2 = m_eCloud1;
			m_eCloud1 = eCloud2;
			m_eCloud3 = eCloud1;
		}

		if (0.0f == fSec)
		{
			m_eCloud2 = m_eCloud3;
			m_eCloud3 = CLOUD_NONE;
		}
		else
		{
			m_Alpha.ChangeColor(0x00ffffff, fSec);
			m_fCloudTexTime = fSec;
		}
		return;
	}

	// 두개 모두 교체
	if (0.0f == fSec)
	{
		m_eCloud1 = eCloud1;
		m_eCloud2 = eCloud2;
		m_eCloud3 = CLOUD_NONE;
		return;
	}
	m_eCloud3       = eCloud1;
	m_eBackupCloud  = eCloud2;
	m_fCloudTexTime = fSec * 0.5f;
	m_fBackupTime   = m_fCloudTexTime;
	m_Alpha.ChangeColor(0x00ffffff, m_fCloudTexTime);
}

