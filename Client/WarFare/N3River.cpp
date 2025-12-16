// N3River.cpp: implementation of the CN3River class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "N3River.h"

#include <N3Base/N3Texture.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define WAVE_TOP		0.02f
//#define WAVE_STEP		0.0005f
#define WAVE_STEP		0.001f

CN3River::CN3River()
{
	m_fTexIndex = 0.0f;
	m_pRiverInfo = nullptr;
	m_iRiverCount = 0;
}

CN3River::~CN3River()
{
	if (m_pRiverInfo)
		delete[] m_pRiverInfo, m_pRiverInfo = nullptr;
}

bool CN3River::Load(HANDLE hFile)
{
	DWORD dwNum;
	uint16_t wIndex[18] = {4,0,1,4,1,5,5,1,2,5,2,6,6,2,3,6,3,7};

	ReadFile(hFile, &m_iRiverCount, sizeof(m_iRiverCount), &dwNum, nullptr);
	if (m_iRiverCount == 0)	return true;
	
	m_pRiverInfo = new _RIVER_INFO[m_iRiverCount];

	_RIVER_INFO	*pInfo = nullptr;
	for (int i=0;i<m_iRiverCount;i++)
	{
		pInfo = m_pRiverInfo+i;
		ReadFile(hFile, &pInfo->iVC, sizeof(int), &dwNum, nullptr);
		__ASSERT(pInfo->iVC, "CN3River : nVertexCount is zero!!");
		__ASSERT(pInfo->iVC%4==0, "RiverVertex is a multiple of 4");

		pInfo->pVertices = new __VertexRiver[pInfo->iVC];
		ReadFile(hFile, pInfo->pVertices, pInfo->iVC*sizeof(__VertexRiver), &dwNum, nullptr);
		ReadFile(hFile, &pInfo->iIC, sizeof(int), &dwNum, nullptr);
		__ASSERT(pInfo->iIC%18==0, "River-Vertex-Index is a multiple of 18");

		int iTexNameLength = 0;
		ReadFile(hFile, &iTexNameLength, sizeof(int), &dwNum, nullptr);
		if (iTexNameLength > 0)
		{
			char szTexture[50];
			ReadFile(hFile, szTexture, iTexNameLength, &dwNum, nullptr);			// texture name
			szTexture[iTexNameLength] = '\0';

			std::string szTextureFName = fmt::format("misc\\river\\{}", szTexture);

			pInfo->m_pTexWave = s_MngTex.Get(szTextureFName);
			__ASSERT(pInfo->m_pTexWave, "CN3River::texture load failed");
		}


		pInfo->pwIndex = new uint16_t[pInfo->iIC];		
		for (int l=0;l<pInfo->iIC/18;l++)
		{
			for (int j=0;j<18;j++)
			{
				pInfo->pwIndex[l*18+j] = wIndex[j] + l*4;
			}
		}

		// 
		pInfo->pDiff = new _RIVER_DIFF[pInfo->iVC];
		float fAdd = 0.0f;
		float fMul = 0.002f;
		for (int l=0;l<pInfo->iVC;l++)
		{
			pInfo->pDiff[l].fDiff = fAdd;
			if (l%2==0)
				pInfo->pDiff[l].fWeight = 1.0f;
			else
				pInfo->pDiff[l].fWeight = -1.0f;
			if (l%4==0)
			{
				fAdd += fMul;
				if (fAdd > WAVE_TOP)
					fMul = -0.002f;
				else if (fAdd < - WAVE_TOP)
					fMul = 0.002f;
			}
		}

		int j,k;
		__VertexRiver* ptVtx = pInfo->pVertices;
		float StX,EnX,StZ,EnZ;
		StX = ptVtx[0].x,EnX = ptVtx[4].x;
		StZ = ptVtx[0].z,EnZ = ptVtx[pInfo->iVC/4].z;
		for (j=0; j<pInfo->iVC/4; j++)
		{
			for (k=0; k<4; k++)
			{
				if(StX>ptVtx->x) StX = ptVtx->x;
				if(EnX<ptVtx->x) EnX = ptVtx->x;
				if(StZ>ptVtx->z) StZ = ptVtx->z;
				if(EnZ<ptVtx->z) EnZ = ptVtx->z;
				ptVtx++;
			}
		}

		pInfo->m_vCenterPo.Set(((EnX-StX)/2.0f) + StX
							  ,pInfo->pVertices[0].y
							  ,((EnZ-StZ)/2.0f) + StZ);

		if(EnX-StX > EnZ-StZ)
			pInfo->m_fRadius = (float)(EnX-StX)*2.0f;
		else pInfo->m_fRadius = (float)(EnZ-StZ)*2.0f;

	}	

	std::string szFileName;
	for (int i = 0; i < MAX_RIVER_TEX; i++)
	{
		szFileName = fmt::format("misc\\river\\caust{:02}.dxt", i);
		m_pTexRiver[i] = s_MngTex.Get(szFileName);
		__ASSERT(m_pTexRiver[i], "CN3River::texture load failed");
	}

	return true;
}

void CN3River::Render()
{
	if (m_iRiverCount <= 0) return;
	int iTex = (int)m_fTexIndex;
	__ASSERT(iTex < MAX_RIVER_TEX, "River Texture index overflow..");
	if(iTex >= MAX_RIVER_TEX || nullptr == m_pTexRiver[iTex]) return;

	// Backup
	__Matrix44 matWorld, matOld;	matWorld.Identity();
	DWORD dwAlphaEnable, dwSrcBlend, dwDestBlend;
	s_lpD3DDev->GetTransform(D3DTS_WORLD, matOld.toD3D());
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	DWORD dwColor_0, dwColorArg1_0, dwColorArg2_0, dwMipFilter_0;
	DWORD dwColor_1, dwColorArg1_1, dwColorArg2_1, dwMipFilter_1;
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
	
	s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    s_lpD3DDev->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    s_lpD3DDev->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );

	s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);	
	s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    s_lpD3DDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);	
    s_lpD3DDev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
    s_lpD3DDev->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	s_lpD3DDev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2);
	_RIVER_INFO		*pInfo = nullptr;
	for (int i=0;i<m_iRiverCount;i++)
	{
		pInfo = m_pRiverInfo + i;
		if(pInfo->m_bTick2Rand==TRUE)
		{
			if(pInfo->m_pTexWave) s_lpD3DDev->SetTexture(1, pInfo->m_pTexWave->Get());
			else s_lpD3DDev->SetTexture(1, nullptr);

			s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, pInfo->iVC, pInfo->iIC/3, pInfo->pwIndex, D3DFMT_INDEX16, pInfo->pVertices, sizeof(__VertexRiver));
		}
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
	if (m_iRiverCount == 0)return;

	_RIVER_INFO	*pInfo=nullptr;
	for (int i=0;i<m_iRiverCount;i++)
	{
		pInfo = m_pRiverInfo+i;
		if(CN3Base::s_CameraData.IsOutOfFrustum(pInfo->m_vCenterPo,pInfo->m_fRadius)==TRUE)
		{
			pInfo->m_bTick2Rand = FALSE;
			continue;
		}
		else pInfo->m_bTick2Rand = TRUE;

		for (int j=0;j<pInfo->iVC;j++)
		{
			(pInfo->pVertices+j)->v += 0.01f*s_fSecPerFrm;
			(pInfo->pVertices+j)->v2 += 0.01f*s_fSecPerFrm;
		}
	}
	

	m_fTexIndex += s_fSecPerFrm*15.0f;
	if (m_fTexIndex >= 32.0f)
	{
		m_fTexIndex -= 32.0f;
	}

	static float fWave = 0.0f;
	fWave += s_fSecPerFrm;
	if (fWave > 0.1f)
	{
		fWave = 0.0f;
		UpdateWaterPositions();
	}
}

void CN3River::UpdateWaterPositions()
{
	if(m_iRiverCount == 0)return;

	_RIVER_INFO		*pInfo = nullptr;
	_RIVER_DIFF		*pDiff = nullptr;
	__VertexRiver	*pVertex;
	int tmp;

	for (int i=0;i<m_iRiverCount;i++)
	{
		pInfo = m_pRiverInfo + i;
		__ASSERT(pInfo, "pInfo is null");
		pDiff = pInfo->pDiff;
		
		pVertex = pInfo->pVertices;
		for (int j=0;j<pInfo->iVC; j++)
		{
			// berserk
			// For optimizing.
			tmp = j%4;
			if (tmp==0 || tmp==3)	
			{
				pDiff++;
				continue;
			}

			pDiff->fDiff += WAVE_STEP*pDiff->fWeight;
			if (pDiff->fDiff > WAVE_TOP)
				pDiff->fWeight = -1.0f;
			else if (pDiff->fDiff < -WAVE_TOP)
				pDiff->fWeight = 1.0f;

			pVertex[j].y += pDiff->fDiff;
			pDiff++;
		}
	}
}
