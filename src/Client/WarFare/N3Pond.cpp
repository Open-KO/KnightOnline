// N3Pond.cpp: implementation of the CN3Pond class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "N3Pond.h"

#include <N3Base/N3Texture.h>

constexpr float ATISQRT = 4.94974747f;

CN3Pond::~CN3Pond()
{
	if (m_iMaxVtxNum > 0)
	{
		delete[] m_pfMaxVtx;
		m_pfMaxVtx   = nullptr;
		m_iMaxVtxNum = 0;
	}

	for (int i = 0; i < MAX_POND_TEX; i++)
		s_MngTex.Delete(&m_pTexPond[i]);
}

void CN3Pond::Release()
{
	m_PondMeshes.clear();

	if (m_iMaxVtxNum > 0)
	{
		delete[] m_pfMaxVtx;
		m_pfMaxVtx   = nullptr;
		m_iMaxVtxNum = 0;
	}

	for (int i = 0; i < MAX_POND_TEX; i++)
		s_MngTex.Delete(&m_pTexPond[i]);

	m_fTexIndex = 0.0f;
}

bool CN3Pond::Load(File& file, int iGtdVersion)
{
	constexpr int MAX_SUPPORTED_POND_MESH_COUNT = 1024;
	constexpr int MAX_SUPPORTED_TEX_NAME_LENGTH = 50;

	Release();

	int iPondMeshNum = 0;

	file.Read(&iPondMeshNum, sizeof(int));
	if (iPondMeshNum <= 0)
		return true;

	if (iPondMeshNum > MAX_SUPPORTED_POND_MESH_COUNT)
		throw std::runtime_error("CN3Pond: invalid pond mesh count");

	m_PondMeshes.resize(iPondMeshNum);

	for (CPondMesh& mesh : m_PondMeshes)
	{
		int iVC = 0;
		file.Read(&iVC, sizeof(int)); // 점 갯수
		mesh.m_iVC        = iVC;      ///
		mesh.m_bTick2Rand = FALSE;    ///
		if (iVC <= 0)
		{
			mesh.m_pVertices = nullptr;
			continue;
		}

		int iWidthVertex = 0;
		file.Read(&iWidthVertex, sizeof(int));   // 한 라인당 점 갯수
		mesh.m_iWidthVtx   = iWidthVertex;       ///
		mesh.m_iHeightVtx  = iVC / iWidthVertex; ///

		int iTexNameLength = 0;
		file.Read(&iTexNameLength, sizeof(int));

		if (iTexNameLength < 0 || iTexNameLength > MAX_SUPPORTED_TEX_NAME_LENGTH)
			throw std::runtime_error("CN3Pond: invalid pond mesh texture name length");

		if (iTexNameLength > 0)
		{
			char szTexture[MAX_SUPPORTED_TEX_NAME_LENGTH + 1] {};
			file.Read(szTexture, iTexNameLength); // texture name
			szTexture[iTexNameLength]  = '\0';

			std::string szTextureFName = fmt::format("misc\\river\\{}", szTexture);

			mesh.m_pTexWave            = s_MngTex.Get(szTextureFName);
			__ASSERT(mesh.m_pTexWave, "CN3Pond::texture load failed");
		}

		// XyxT2 -> XyzColorT2 Converting.
		mesh.m_pVertices = new __VertexPond[iVC]; ///
		file.Read(mesh.m_pVertices, iVC * sizeof(__VertexPond));

		float fWaveVariance = 0.2f;
		if (iGtdVersion >= 2)
			file.Read(&fWaveVariance, sizeof(float));

		// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		mesh.m_pVertices[0].y            += fWaveVariance;           //	수치가 높으면 물결이 크게 요동친다
		mesh.m_pVertices[iWidthVertex].y += fWaveVariance;           //	수치가 높으면 물결이 크게 요동친다
		mesh.m_pfMaxHeight = mesh.m_pVertices[0].y += fWaveVariance; //	물결의 최대치
		// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

		mesh.m_pfVelocityArray                      = new float[iVC] {}; ///

		int iIC                                     = 0;
		file.Read(&iIC, sizeof(int));                                    // IndexBuffer Count.
		mesh.m_iIC     = iIC;                                            ///
		mesh.m_wpIndex = new uint16_t[iVC * 6];                          ///

		int iWidth = iWidthVertex, iHeight = iVC / iWidthVertex;
		int x = 0, y = iWidth;
		uint16_t* indexPtr = mesh.m_wpIndex; //	삼각형을 부를 위치 설정
		iWidth--;

		__VertexPond* ptVtx = mesh.m_pVertices;
		float StX = 0.0f, EnX = 0.0f, StZ = 0.0f, EnZ = 0.0f;
		StX = ptVtx[0].x, EnX = ptVtx[iWidth].x;
		StZ = ptVtx[0].z, EnZ = ptVtx[iHeight].z;
		for (int j = 0; j < iHeight; j++)
		{
			for (int k = 0; k < iWidth; k++)
			{
				//	삼각형을 부를 위치 설정
				indexPtr[0]  = x;
				indexPtr[1]  = x + 1;
				indexPtr[2]  = y;
				indexPtr[3]  = y;
				indexPtr[4]  = x + 1;
				indexPtr[5]  = y + 1;

				indexPtr    += 6;
				x++;
				y++;

				//	연못의 최소최대 위치 구함
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
			x++;
			y++;
		}

		float fmin = 0.0f, fmax = 0.0f, fmaxcal = 0.0f, fmincal = 0.0f;
		if (mesh.m_pfMaxHeight > 0.0f)
		{
			fmax    = mesh.m_pfMaxHeight * 0.04f;
			fmin    = -fmax;
			fmaxcal = fmax * ATISQRT;
			fmincal = -fmaxcal;
		}
		else if (mesh.m_pfMaxHeight < 0.0f)
		{
			fmin    = mesh.m_pfMaxHeight * 0.04f;
			fmax    = -fmin;
			fmincal = fmin * ATISQRT;
			fmaxcal = -fmincal;
		}
		else
		{
			fmax    = 0.04f;
			fmin    = -fmax;
			fmaxcal = fmax * ATISQRT;
			fmincal = -fmaxcal;
		}

		mesh.m_fmin    = fmin;
		mesh.m_fmax    = fmax;
		mesh.m_fmaxcal = fmaxcal;
		mesh.m_fmincal = fmincal;

		mesh.m_vCenterPo.Set(((EnX - StX) / 2.0f) + StX, mesh.m_pVertices[1].y, ((EnZ - StZ) / 2.0f) + StZ);
		if (EnX - StX > EnZ - StZ)
			mesh.m_fRadius = EnX - StX;
		else
			mesh.m_fRadius = EnZ - StZ;

		mesh.m_bTick2Rand = TRUE; ///

		if (m_iMaxVtxNum < iVC)
			m_iMaxVtxNum = iVC;   //	가장큰 계산범위 구함
	}

	m_pfMaxVtx    = new float[m_iMaxVtxNum];
	m_iMaxVtxNum *= sizeof(float);

	std::string szFileName;
	for (int i = 0; i < MAX_POND_TEX; i++)
	{
		szFileName    = fmt::format("misc\\river\\caust{:02}.dxt", i);
		m_pTexPond[i] = CN3Base::s_MngTex.Get(szFileName);
		__ASSERT(m_pTexPond[i], "CN3Pond::texture load failed");
	}

	return true;
}

void CN3Pond::Tick()
{
	if (m_PondMeshes.empty())
		return;

	float frame  = 0.0f;

	m_fTexIndex += s_fSecPerFrm * 15.0f;
	if (m_fTexIndex >= MAX_POND_TEX)
		m_fTexIndex -= MAX_POND_TEX;

	// 프레임이 임계값보다 작으면 버린다..
	if (CN3Base::s_fFrmPerSec < 0.1f)
		return;

	// Desire Frame Rate보다 Frame이 잘 나오는 경우..
	if (30.0f <= CN3Base::s_fFrmPerSec)
	{
		static float ftemp  = 0.0f;
		frame               = (30.0f / CN3Base::s_fFrmPerSec) * 1.2f;
		ftemp              += frame;
		if (ftemp > 1.0f)
		{
			UpdateWaterPositions();
			ftemp -= 1.0f;
		}
	}
	// Desire Frame보다 Frame이 잘 안나오는 경우..
	else
	{
		static float ftemp  = 0.0f;
		frame               = (30.0f / CN3Base::s_fFrmPerSec) * 1.2f;
		ftemp              += frame;
		int i               = (int) ftemp;
		float j             = ftemp - (float) i;

		for (int k = 0; k < i; k++)
			UpdateWaterPositions();

		ftemp = j;
	}
}

void CN3Pond::Render()
{
	if (m_PondMeshes.empty())
		return;

	int iTex = (int) m_fTexIndex;
	__ASSERT(iTex < MAX_POND_TEX, "Pond Texture index overflow..");
	if (iTex >= MAX_POND_TEX || nullptr == m_pTexPond[iTex])
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

	// texture state 세팅 (alpha)
	s_lpD3DDev->SetTexture(0, m_pTexPond[iTex]->Get());
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

	for (const CPondMesh& mesh : m_PondMeshes)
	{
		if (!mesh.m_bTick2Rand)
			continue;

		if (mesh.m_pTexWave != nullptr)
			s_lpD3DDev->SetTexture(1, mesh.m_pTexWave->Get());
		else
			s_lpD3DDev->SetTexture(1, nullptr);

		s_lpD3DDev->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST, 0, mesh.m_iVC, mesh.m_iIC, mesh.m_wpIndex, D3DFMT_INDEX16, mesh.m_pVertices, sizeof(__VertexPond));
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

void CN3Pond::UpdateWaterPositions()
{
	//	기초 데이타
	int x = 0, y = 0, n = 0, m = 0;
	float d            = 0.0f;
	__VertexPond *pVtx = nullptr, *ptmpVtx = nullptr, *ptmpVtxSub = nullptr, *ptmpVtxPlus = nullptr;
	float *pForceArray = nullptr, *ptmpForceArray = nullptr, *ptmpFArrSub = nullptr, *ptmpFArrPlus = nullptr;

	//	계산 변수
	float max = 0.0f, min = 0.0f, mincal = 0.0f, maxcal = 0.0f;

	for (CPondMesh& mesh : m_PondMeshes)
	{
		//	이번에 쓰이지 않을 경우 넘어감
		if (CN3Base::s_CameraData.IsOutOfFrustum(mesh.m_vCenterPo, mesh.m_fRadius))
		{
			mesh.m_bTick2Rand = FALSE;
			continue;
		}

		mesh.m_bTick2Rand = TRUE;

		//	기초데이타 작성
		m                 = mesh.m_iWidthVtx;
		n                 = mesh.m_iHeightVtx;
		max               = mesh.m_fmax;
		min               = mesh.m_fmin;
		maxcal            = mesh.m_fmaxcal;
		mincal            = mesh.m_fmincal;

		memset(m_pfMaxVtx, 0, m_iMaxVtxNum);

		pVtx        = mesh.m_pVertices;
		pForceArray = m_pfMaxVtx;

		//	계산
		for (x = 1; x < n - 1; x++)
		{
			ptmpFArrSub     = pForceArray;
			pForceArray    += m;
			ptmpForceArray  = pForceArray;
			ptmpFArrPlus    = ptmpForceArray + m;

			ptmpVtxSub      = pVtx;
			pVtx           += m;
			ptmpVtx         = pVtx;
			ptmpVtxPlus     = ptmpVtx + m;

			for (y = 1; y < m - 1; y++)
			{
				//   Kernel looks like this:
				//
				//    1/Root2 |    1    | 1/Root2
				//   ---------+---------+---------
				//       1    |    0    |    1
				//   ---------+---------+---------
				//    1/Root2 |    1    | 1/Root2

				ptmpForceArray++, ptmpFArrPlus++, ptmpFArrSub++;
				ptmpVtx++, ptmpVtxPlus++, ptmpVtxSub++;

				d = ptmpVtx->y - (ptmpVtx - 1)->y;
				if (d < min)
					d = min;
				if (d > max)
					d = max;
				*ptmpForceArray       -= d;
				*(ptmpForceArray - 1) += d;

				d                      = ptmpVtx->y - ptmpVtxSub->y;
				if (d < min)
					d = min;
				if (d > max)
					d = max;
				*ptmpForceArray -= d;
				*ptmpFArrSub    += d;

				d                = ptmpVtx->y - (ptmpVtx + 1)->y;
				if (d < min)
					d = min;
				if (d > max)
					d = max;
				*ptmpForceArray       -= d;
				*(ptmpForceArray + 1) += d;

				d                      = ptmpVtx->y - ptmpVtxPlus->y;
				if (d < min)
					d = min;
				if (d > max)
					d = max;
				*ptmpForceArray -= d;
				*ptmpFArrPlus   += d;

				d                = (ptmpVtx->y - (ptmpVtxPlus + 1)->y) * ATISQRT;
				if (d < mincal)
					d = mincal;
				if (d > maxcal)
					d = maxcal;
				*ptmpForceArray     -= d;
				*(ptmpFArrPlus + 1) += d;

				d                    = (ptmpVtx->y - (ptmpVtxSub - 1)->y) * ATISQRT;
				if (d < mincal)
					d = mincal;
				if (d > maxcal)
					d = maxcal;
				*ptmpForceArray    -= d;
				*(ptmpFArrSub - 1) += d;

				d                   = (ptmpVtx->y - (ptmpVtxPlus - 1)->y) * ATISQRT;
				if (d < mincal)
					d = mincal;
				if (d > maxcal)
					d = maxcal;
				*ptmpForceArray     -= d;
				*(ptmpFArrPlus - 1) += d;

				d                    = (ptmpVtx->y - (ptmpVtxSub + 1)->y) * ATISQRT;
				if (d < mincal)
					d = mincal;
				if (d > maxcal)
					d = maxcal;
				*ptmpForceArray    -= d;
				*(ptmpFArrSub + 1) += d;
			}
		}

		ptmpForceArray = mesh.m_pfVelocityArray; //	같은형이라 빌려씀
		pForceArray    = m_pfMaxVtx;
		pVtx           = mesh.m_pVertices;
		for (x = 0; x < mesh.m_iVC; x++)
		{
			//			*ptmpForceArray += *pForceArray*0.02f;
			(*ptmpForceArray) += (*pForceArray) * 0.001f;

			pVtx->y           += (*ptmpForceArray);
			if (pVtx->y > mesh.m_pfMaxHeight)
				pVtx->y = mesh.m_pfMaxHeight;

			pForceArray++, pVtx++, ptmpForceArray++;
		}
	}
}
