// N3River.cpp: implementation of the CN3River class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "N3River.h"

#include <N3Base/N3Texture.h>

constexpr float WAVE_TOP  = 0.02f;
constexpr float WAVE_STEP = 0.001f;

CN3River::~CN3River()
{
	for (int i = 0; i < MAX_RIVER_TEX; i++)
		s_MngTex.Delete(&m_pTexRiver[i]);
}

void CN3River::Release()
{
	m_Rivers.clear();

	for (int i = 0; i < MAX_RIVER_TEX; i++)
		s_MngTex.Delete(&m_pTexRiver[i]);
}

bool CN3River::Load(File& file)
{
	constexpr int MAX_SUPPORTED_RIVER_COUNT     = 1024;
	constexpr int MAX_SUPPORTED_TEX_NAME_LENGTH = 50;
	constexpr uint16_t wIndex[18]               = { 4, 0, 1, 4, 1, 5, 5, 1, 2, 5, 2, 6, 6, 2, 3, 6, 3, 7 };

	Release();

	int iRiverCount = 0;
	file.Read(&iRiverCount, sizeof(int));
	if (iRiverCount <= 0)
		return true;

	if (iRiverCount > MAX_SUPPORTED_RIVER_COUNT)
		throw std::runtime_error("invalid river count");

	m_Rivers.resize(iRiverCount);

	for (_RIVER_INFO& river : m_Rivers)
	{
		file.Read(&river.iVC, sizeof(int));
		if (river.iVC == 0 || (river.iVC % 4) != 0)
			throw std::runtime_error("invalid river mesh vertex count");

		river.pVertices = new __VertexRiver[river.iVC];
		file.Read(river.pVertices, river.iVC * sizeof(__VertexRiver));
		file.Read(&river.iIC, sizeof(int));
		__ASSERT(river.iIC % 18 == 0, "River-Vertex-Index is a multiple of 18");

		if ((river.iIC % 18) != 0)
			throw std::runtime_error("invalid river mesh index count");

		int iTexNameLength = 0;
		file.Read(&iTexNameLength, sizeof(int));

		if (iTexNameLength < 0 || iTexNameLength > MAX_SUPPORTED_TEX_NAME_LENGTH)
			throw std::runtime_error("invalid river mesh texture name length");

		if (iTexNameLength > 0)
		{
			char szTexture[MAX_SUPPORTED_TEX_NAME_LENGTH + 1] {};
			file.Read(szTexture, iTexNameLength); // texture name
			szTexture[iTexNameLength]  = '\0';

			std::string szTextureFName = fmt::format("misc\\river\\{}", szTexture);

			river.m_pTexWave           = s_MngTex.Get(szTextureFName);
			__ASSERT(river.m_pTexWave, "CN3River::texture load failed");
		}

		river.pwIndex = new uint16_t[river.iIC];
		for (int l = 0; l < river.iIC / 18; l++)
		{
			for (int j = 0; j < 18; j++)
				river.pwIndex[l * 18 + j] = wIndex[j] + l * 4;
		}

		//
		river.pDiff = new _RIVER_DIFF[river.iVC];
		float fAdd  = 0.0f;
		float fMul  = 0.002f;
		for (int l = 0; l < river.iVC; l++)
		{
			river.pDiff[l].fDiff = fAdd;
			if (l % 2 == 0)
				river.pDiff[l].fWeight = 1.0f;
			else
				river.pDiff[l].fWeight = -1.0f;

			if (l % 4 == 0)
			{
				fAdd += fMul;
				if (fAdd > WAVE_TOP)
					fMul = -0.002f;
				else if (fAdd < -WAVE_TOP)
					fMul = 0.002f;
			}
		}

		// Below code expects at least 5 vertices.
		__ASSERT(river.iVC >= 5, "Requires at least 5 vertices per river mesh");
		if (river.iVC < 5)
			return false;

		__VertexRiver* ptVtx = river.pVertices;
		float StX = 0.0f, EnX = 0.0f, StZ = 0.0f, EnZ = 0.0f;
		StX = ptVtx[0].x, EnX = ptVtx[4].x;
		StZ = ptVtx[0].z, EnZ = ptVtx[river.iVC / 4].z;
		for (int j = 0; j < river.iVC / 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				if (StX > ptVtx->x)
					StX = ptVtx->x;

				if (EnX < ptVtx->x)
					EnX = ptVtx->x;

				if (StZ > ptVtx->z)
					StZ = ptVtx->z;

				if (EnZ < ptVtx->z)
					EnZ = ptVtx->z;

				ptVtx++;
			}
		}

		river.m_vCenterPo.Set(((EnX - StX) / 2.0f) + StX, river.pVertices[0].y, ((EnZ - StZ) / 2.0f) + StZ);

		if (EnX - StX > EnZ - StZ)
			river.m_fRadius = (float) (EnX - StX) * 2.0f;
		else
			river.m_fRadius = (float) (EnZ - StZ) * 2.0f;
	}

	std::string szFileName;
	for (int i = 0; i < MAX_RIVER_TEX; i++)
	{
		szFileName     = fmt::format("misc\\river\\caust{:02}.dxt", i);
		m_pTexRiver[i] = s_MngTex.Get(szFileName);
		__ASSERT(m_pTexRiver[i], "CN3River::texture load failed");
	}

	return true;
}

void CN3River::Render()
{
	if (m_Rivers.empty())
		return;

	int iTex = (int) m_fTexIndex;
	__ASSERT(iTex < MAX_RIVER_TEX, "River Texture index overflow..");
	if (iTex >= MAX_RIVER_TEX || nullptr == m_pTexRiver[iTex])
		return;

	// Backup
	__Matrix44 matWorld, matOld;
	matWorld.Identity();

	DWORD dwAlphaEnable = 0, dwSrcBlend = 0, dwDestBlend = 0;
	DWORD dwColor_0 = 0, dwColorArg1_0 = 0, dwColorArg2_0 = 0, dwMipFilter_0 = 0;
	DWORD dwColor_1 = 0, dwColorArg1_1 = 0, dwColorArg2_1 = 0, dwMipFilter_1 = 0;

	s_lpD3DDev->GetTransform(D3DTS_WORLD, matOld.toD3D());
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);

	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwColor_0);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorArg1_0);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG2, &dwColorArg2_0);
	s_lpD3DDev->GetSamplerState(0, D3DSAMP_MIPFILTER, &dwMipFilter_0);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLOROP, &dwColor_1);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG1, &dwColorArg1_1);
	s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG2, &dwColorArg2_1);
	s_lpD3DDev->GetSamplerState(1, D3DSAMP_MIPFILTER, &dwMipFilter_1);

	// Set
	s_lpD3DDev->SetTransform(D3DTS_WORLD, matWorld.toD3D());

	s_lpD3DDev->SetTexture(0, m_pTexRiver[iTex]->Get());
	s_lpD3DDev->SetTexture(2, nullptr);

	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	s_lpD3DDev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	s_lpD3DDev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2);
	for (const _RIVER_INFO& river : m_Rivers)
	{
		if (!river.m_bTick2Rand)
			continue;

		if (river.m_pTexWave != nullptr)
			s_lpD3DDev->SetTexture(1, river.m_pTexWave->Get());
		else
			s_lpD3DDev->SetTexture(1, nullptr);

		s_lpD3DDev->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST, 0, river.iVC, river.iIC / 3, river.pwIndex, D3DFMT_INDEX16, river.pVertices, sizeof(__VertexRiver));
	}

	// restore
	s_lpD3DDev->SetTransform(D3DTS_WORLD, matOld.toD3D());
	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwColor_0);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwColorArg1_0);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, dwColorArg2_0);
	s_lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, dwMipFilter_0);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, dwColor_1);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, dwColorArg1_1);
	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, dwColorArg2_1);
	s_lpD3DDev->SetSamplerState(1, D3DSAMP_MIPFILTER, dwMipFilter_1);
}

void CN3River::Tick()
{
	if (m_Rivers.empty())
		return;

	const float vDelta = 0.01f * s_fSecPerFrm;
	for (_RIVER_INFO& river : m_Rivers)
	{
		if (CN3Base::s_CameraData.IsOutOfFrustum(river.m_vCenterPo, river.m_fRadius))
		{
			river.m_bTick2Rand = FALSE;
			continue;
		}

		river.m_bTick2Rand = TRUE;

		for (int j = 0; j < river.iVC; j++)
		{
			river.pVertices[j].v  += vDelta;
			river.pVertices[j].v2 += vDelta;
		}
	}

	m_fTexIndex += s_fSecPerFrm * 15.0f;
	if (m_fTexIndex >= MAX_RIVER_TEX)
		m_fTexIndex -= MAX_RIVER_TEX;

	static float fWave  = 0.0f;
	fWave              += s_fSecPerFrm;
	if (fWave > 0.1f)
	{
		fWave = 0.0f;
		UpdateWaterPositions();
	}
}

void CN3River::UpdateWaterPositions()
{
	if (m_Rivers.empty())
		return;

	int tmp = 0;
	for (_RIVER_INFO& river : m_Rivers)
	{
		_RIVER_DIFF* pDiff     = river.pDiff;

		__VertexRiver* pVertex = river.pVertices;
		for (int j = 0; j < river.iVC; j++)
		{
			// berserk
			// For optimizing.
			tmp = j % 4;
			if (tmp == 0 || tmp == 3)
			{
				pDiff++;
				continue;
			}

			pDiff->fDiff += WAVE_STEP * pDiff->fWeight;
			if (pDiff->fDiff > WAVE_TOP)
				pDiff->fWeight = -1.0f;
			else if (pDiff->fDiff < -WAVE_TOP)
				pDiff->fWeight = 1.0f;

			pVertex[j].y += pDiff->fDiff;
			pDiff++;
		}
	}
}
