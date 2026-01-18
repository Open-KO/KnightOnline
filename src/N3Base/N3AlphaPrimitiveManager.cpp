// N3AlphaPrimitiveManager.cpp: implementation of the CN3AlphaPrimitiveManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3AlphaPrimitiveManager.h"
#include "N3Base.h"

CN3AlphaPrimitiveManager::CN3AlphaPrimitiveManager()
{
	m_nToDrawCount = 0;
}

CN3AlphaPrimitiveManager::~CN3AlphaPrimitiveManager()
{
}

void CN3AlphaPrimitiveManager::Render()
{
	__ASSERT(CN3Base::s_lpD3DDev, "3d device pointer is NULL!!!");
	if (m_nToDrawCount <= 0)
		return;

	static __AlphaPrimitive* pBuffs[MAX_ALPHAPRIMITIVE_BUFFER] {};
	for (int i = 0; i < m_nToDrawCount; i++)
		pBuffs[i] = &m_Buffers[i];
	qsort(&pBuffs, m_nToDrawCount, sizeof(__AlphaPrimitive*),
		SortByCameraDistance); // 버퍼에 쌓인 프리미티브대로 정렬하고..

	struct __RenderState
	{
		DWORD dwAlpha, dwFog, dwCull, dwLgt, dwZWrite, dwAO, dwAA1, dwAA2, dwCO, dwCA1, dwCA2,
			dwPointSampling;
		DWORD dwAddressU, dwAddressV;
		DWORD dwSrcBlend, dwDestBlend;
		DWORD dwZEnable;
	};
	__RenderState RS_old;     // 이전 render state (나중에 되돌려놓기 위해)
	__RenderState RS_current; // 현재 render state (현재 어떤 상태인가 판단하기 위해)

	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &RS_old.dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &RS_old.dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &RS_old.dwCull);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &RS_old.dwLgt);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZWRITEENABLE, &RS_old.dwZWrite);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &RS_old.dwZEnable);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &RS_old.dwSrcBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &RS_old.dwDestBlend);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &RS_old.dwAO);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &RS_old.dwAA1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG2, &RS_old.dwAA2);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &RS_old.dwCO);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &RS_old.dwCA1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG2, &RS_old.dwCA2);
	CN3Base::s_lpD3DDev->GetSamplerState(0, D3DSAMP_MIPFILTER, &RS_old.dwPointSampling);
	CN3Base::s_lpD3DDev->GetSamplerState(0, D3DSAMP_ADDRESSU, &RS_old.dwAddressU);
	CN3Base::s_lpD3DDev->GetSamplerState(0, D3DSAMP_ADDRESSV, &RS_old.dwAddressV);

	RS_current = RS_old;

	if (TRUE != RS_current.dwAlpha)
	{
		RS_current.dwAlpha = TRUE;
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	}
	static __Material mtl;
	mtl.Init();
	CN3Base::s_lpD3DDev->SetMaterial(&mtl);

	for (int i = 0; i < m_nToDrawCount; i++)
	{
		if (pBuffs[i]->nRenderFlags & RF_NOTUSEFOG)
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
		else
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, RS_old.dwFog);
		if (pBuffs[i]->nRenderFlags & RF_DOUBLESIDED)
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		else
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, RS_old.dwCull);
		if (pBuffs[i]->nRenderFlags & RF_NOTUSELIGHT)
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
		else
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, RS_old.dwLgt);
		if (pBuffs[i]->nRenderFlags & RF_NOTZWRITE)
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		else
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, RS_old.dwZWrite);
		if (pBuffs[i]->nRenderFlags & RF_NOTZBUFFER)
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		else
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, RS_old.dwZEnable);
		if (pBuffs[i]->nRenderFlags & RF_DIFFUSEALPHA)
		{
			if (pBuffs[i]->lpTex)
			{
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
			}
			else
			{
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			}
		}
		else
		{
			if (pBuffs[i]->lpTex)
			{
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, RS_old.dwAO);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, RS_old.dwAA1);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, RS_old.dwAA2);
			}
			else
			{
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, RS_old.dwAO);
				CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, RS_old.dwAA1);
			}
		}
		if (pBuffs[i]->nRenderFlags & RF_POINTSAMPLING)
			CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		else
			CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, RS_old.dwPointSampling);
		if (pBuffs[i]->nRenderFlags & RF_UV_CLAMP)
		{
			CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		}
		else
		{
			CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, RS_old.dwAddressU);
			CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, RS_old.dwAddressV);
		}

		if (pBuffs[i]->dwBlendSrc != RS_current.dwSrcBlend)
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, pBuffs[i]->dwBlendSrc);
		else
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, RS_old.dwSrcBlend);
		if (pBuffs[i]->dwBlendDest != RS_current.dwDestBlend)
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, pBuffs[i]->dwBlendDest);
		else
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, RS_old.dwDestBlend);

		CN3Base::s_lpD3DDev->SetFVF(pBuffs[i]->dwFVF);
		CN3Base::s_lpD3DDev->SetTexture(0, pBuffs[i]->lpTex);
		CN3Base::s_lpD3DDev->SetTransform(
			D3DTS_WORLD, pBuffs[i]->MtxWorld.toD3D()); // 월드 행렬 적용

		if (pBuffs[i]->lpTex)
		{
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		}
		else
		{
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		}

		if (pBuffs[i]->pwIndices && pBuffs[i]->nPrimitiveCount > 0) // Index 가 있으면..
		{
			if (pBuffs[i]->bUseVB)
			{
				CN3Base::s_lpD3DDev->SetStreamSource(0,
					(LPDIRECT3DVERTEXBUFFER9) pBuffs[i]->pVertices, 0, pBuffs[i]->dwPrimitiveSize);
				CN3Base::s_lpD3DDev->SetIndices((LPDIRECT3DINDEXBUFFER9) pBuffs[i]->pwIndices);
				CN3Base::s_lpD3DDev->DrawIndexedPrimitive(pBuffs[i]->ePrimitiveType, 0, 0,
					pBuffs[i]->nVertexCount, 0, pBuffs[i]->nPrimitiveCount);
			}
			else
			{
				CN3Base::s_lpD3DDev->DrawIndexedPrimitiveUP(pBuffs[i]->ePrimitiveType, 0,
					pBuffs[i]->nVertexCount, pBuffs[i]->nPrimitiveCount, pBuffs[i]->pwIndices,
					D3DFMT_INDEX16, pBuffs[i]->pVertices, pBuffs[i]->dwPrimitiveSize);
			}
		}
		else if (pBuffs[i]->nPrimitiveCount > 0)
		{
			if (pBuffs[i]->bUseVB)
			{
				CN3Base::s_lpD3DDev->SetStreamSource(0,
					(LPDIRECT3DVERTEXBUFFER9) pBuffs[i]->pVertices, 0, pBuffs[i]->dwPrimitiveSize);
				CN3Base::s_lpD3DDev->DrawPrimitive(
					pBuffs[i]->ePrimitiveType, 0, pBuffs[i]->nPrimitiveCount);
			}
			else
			{
				CN3Base::s_lpD3DDev->DrawPrimitiveUP(pBuffs[i]->ePrimitiveType,
					pBuffs[i]->nPrimitiveCount, pBuffs[i]->pVertices, pBuffs[i]->dwPrimitiveSize);
			}
		}

#ifdef _DEBUG
		CN3Base::s_RenderInfo.nAlpha_Polygon += pBuffs[i]->nPrimitiveCount
												/ 3; // Rendering Information Update...
#endif
	}

	m_nToDrawCount = 0; // 다 그렸다...

	// restore
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, RS_old.dwAlpha);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, RS_old.dwFog);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, RS_old.dwCull);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, RS_old.dwLgt);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, RS_old.dwZWrite);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, RS_old.dwZEnable);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, RS_old.dwSrcBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, RS_old.dwDestBlend);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, RS_old.dwAO);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, RS_old.dwAA1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, RS_old.dwAA2);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, RS_old.dwCO);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, RS_old.dwCA1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, RS_old.dwCA2);
	CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, RS_old.dwPointSampling);
	CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, RS_old.dwAddressU);
	CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, RS_old.dwAddressV);

	/*	
	__ASSERT(CN3Base::s_lpD3DDev, "3d device pointer is NULL!!!");
	if(m_nToDrawCount <= 0) return;
	
	static __AlphaPrimitive* pBuffs[MAX_ALPHAPRIMITIVE_BUFFER];
	for(int i = 0; i < m_nToDrawCount; i++) pBuffs[i] = &(m_Buffers[i]);
	qsort(pBuffs, m_nToDrawCount, sizeof(__AlphaPrimitive*), SortByCameraDistance); // 버퍼에 쌓인 프리미티브대로 정렬하고..

	struct __RenderState
	{
		uint32_t dwAlpha, dwFog, dwCull, dwLgt, dwZWrite, dwAO, dwAA1, dwAA2, dwCO, dwCA1, dwCA2, dwPointSampling;
		uint32_t dwSrcBlend, dwDestBlend;
	};
	__RenderState RS_old;		// 이전 render state (나중에 되돌려놓기 위해)
	__RenderState RS_current;	// 현재 render state (현재 어떤 상태인가 판단하기 위해)

	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &RS_old.dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &RS_old.dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &RS_old.dwCull);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &RS_old.dwLgt);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZWRITEENABLE, &RS_old.dwZWrite);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND,  &RS_old.dwSrcBlend );
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &RS_old.dwDestBlend);
	CN3Base::s_lpD3DDev->GetTextureStageState( 0, D3DTSS_ALPHAOP,   &RS_old.dwAO);
	CN3Base::s_lpD3DDev->GetTextureStageState( 0, D3DTSS_ALPHAARG1, &RS_old.dwAA1);
	CN3Base::s_lpD3DDev->GetTextureStageState( 0, D3DTSS_ALPHAARG2, &RS_old.dwAA2);
	CN3Base::s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLOROP,   &RS_old.dwCO);
	CN3Base::s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG1, &RS_old.dwCA1);
	CN3Base::s_lpD3DDev->GetTextureStageState( 0, D3DTSS_COLORARG2, &RS_old.dwCA2);
	CN3Base::s_lpD3DDev->GetTextureStageState( 0, D3DTSS_MIPFILTER, &RS_old.dwPointSampling);

	RS_current = RS_old;

	if(TRUE != RS_current.dwAlpha) {RS_current.dwAlpha = TRUE; CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);}
	static __Material mtl; mtl.Init();
	CN3Base::s_lpD3DDev->SetMaterial(&mtl);

	for(i = 0; i < m_nToDrawCount; i++)
	{
		if(pBuffs[i]->nRenderFlags & RF_NOTUSEFOG)
		{
			if (FALSE != RS_current.dwFog) { RS_current.dwFog = FALSE;	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);}
		} // Fog 무시..
		else
		{
			if (TRUE != RS_current.dwFog) {	RS_current.dwFog = TRUE;	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, TRUE);	}
		}

		if(pBuffs[i]->nRenderFlags & RF_DOUBLESIDED)
		{
			if (D3DCULL_NONE != RS_current.dwCull) { RS_current.dwCull = D3DCULL_NONE;	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);}
		} // Render Flags - 
		else
		{
			if (D3DCULL_CCW != RS_current.dwCull) {	RS_current.dwCull = D3DCULL_CCW; CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);}
		}

		if(pBuffs[i]->nRenderFlags & RF_NOTUSELIGHT)
		{
			if (FALSE != RS_current.dwLgt) { RS_current.dwLgt = FALSE;	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);	}
		} // Render Flags - 
		else
		{
			if (TRUE != RS_current.dwLgt) {	RS_current.dwLgt = TRUE;	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, TRUE);}
		}

		if(pBuffs[i]->nRenderFlags & RF_NOTZWRITE)
		{
			if (FALSE != RS_current.dwZWrite) {	RS_current.dwZWrite = FALSE; CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);}
		}
		else
		{
			if (TRUE != RS_current.dwZWrite) {	RS_current.dwZWrite = TRUE;	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);}
		}

		if(pBuffs[i]->nRenderFlags & RF_DIFFUSEALPHA)
		{
			if(pBuffs[i]->lpTex)
			{
				if (D3DTOP_MODULATE != RS_current.dwAO)	{ RS_current.dwAO = D3DTOP_MODULATE; CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );	}
				if (D3DTA_DIFFUSE != RS_current.dwAA1)	{ RS_current.dwAA1 = D3DTA_DIFFUSE; CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);}
				if (D3DTA_TEXTURE != RS_current.dwAA2)	{ RS_current.dwAA2 = D3DTA_TEXTURE;	CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);}
			}
			else
			{
				if (D3DTOP_SELECTARG1 != RS_current.dwAO) {	RS_current.dwAO = D3DTOP_SELECTARG1; CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );}
				if (D3DTA_DIFFUSE != RS_current.dwAA1) { RS_current.dwAA1 = D3DTA_DIFFUSE;	CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);}
			}
		}
		if(pBuffs[i]->nRenderFlags & RF_POINTSAMPLING)
		{
			if ( D3DTEXF_POINT != RS_current.dwPointSampling) { RS_current.dwPointSampling = D3DTEXF_POINT;	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT); }// Render Flags -
		}
		else
		{
			if ( D3DTEXF_NONE != RS_current.dwPointSampling) { RS_current.dwPointSampling = D3DTEXF_NONE; CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE); }// Render Flags - 
		}
		
		if (pBuffs[i]->dwBlendSrc != RS_current.dwSrcBlend)	{ RS_current.dwSrcBlend = pBuffs[i]->dwBlendSrc; CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND,   pBuffs[i]->dwBlendSrc);}
		if (pBuffs[i]->dwBlendDest != RS_current.dwDestBlend) {	RS_current.dwDestBlend = pBuffs[i]->dwBlendDest; CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND,  pBuffs[i]->dwBlendDest);}

		CN3Base::s_lpD3DDev->SetFVF(pBuffs[i]->dwFVF);
		CN3Base::s_lpD3DDev->SetTexture(0, pBuffs[i]->lpTex);
		CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, &(pBuffs[i]->MtxWorld)); // 월드 행렬 적용

		if(pBuffs[i]->lpTex)
		{
			if (D3DTOP_MODULATE != RS_current.dwCO)	{ RS_current.dwCO = D3DTOP_MODULATE; CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);	}
			if (D3DTA_DIFFUSE != RS_current.dwCA1) { RS_current.dwCA1 = D3DTA_DIFFUSE; CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);}
			if (D3DTA_TEXTURE != RS_current.dwCA2) { RS_current.dwCA2 = D3DTA_TEXTURE; CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);}
		}
		else
		{
			if (D3DTOP_SELECTARG1 != RS_current.dwCO) {	RS_current.dwCO = D3DTOP_SELECTARG1; CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);}
			if (D3DTA_DIFFUSE != RS_current.dwCA1) { RS_current.dwCA1 = D3DTA_DIFFUSE;	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);}
		}
	
		if(pBuffs[i]->pwIndices && pBuffs[i]->nPrimitiveCount > 0) // Index 가 있으면..
		{
			if (pBuffs[i]->bUseVB)
			{
				CN3Base::s_lpD3DDev->SetStreamSource( 0, (LPDIRECT3DVERTEXBUFFER8)pBuffs[i]->pVertices, pBuffs[i]->dwPrimitiveSize );
				CN3Base::s_lpD3DDev->SetIndices((LPDIRECT3DINDEXBUFFER8)pBuffs[i]->pwIndices, 0);
				CN3Base::s_lpD3DDev->DrawIndexedPrimitive(	pBuffs[i]->ePrimitiveType,
												0,
												pBuffs[i]->nVertexCount,
												0,
												pBuffs[i]->nPrimitiveCount );
			}
			else
			{
				CN3Base::s_lpD3DDev->DrawIndexedPrimitiveUP(	pBuffs[i]->ePrimitiveType, 
												0,
												pBuffs[i]->nVertexCount,
												pBuffs[i]->nPrimitiveCount,
												pBuffs[i]->pwIndices,
												D3DFMT_INDEX16,
												pBuffs[i]->pVertices,
												pBuffs[i]->dwPrimitiveSize );
			}
		}
		else if(pBuffs[i]->nPrimitiveCount > 0)
		{
			if (pBuffs[i]->bUseVB)
			{
				CN3Base::s_lpD3DDev->SetStreamSource( 0, (LPDIRECT3DVERTEXBUFFER8)pBuffs[i]->pVertices, pBuffs[i]->dwPrimitiveSize );
				CN3Base::s_lpD3DDev->DrawPrimitive(	pBuffs[i]->ePrimitiveType,
										0,
										pBuffs[i]->nPrimitiveCount );
			}
			else
			{
				CN3Base::s_lpD3DDev->DrawPrimitiveUP(	pBuffs[i]->ePrimitiveType, 
										pBuffs[i]->nPrimitiveCount,
										pBuffs[i]->pVertices,
										pBuffs[i]->dwPrimitiveSize );
			}
		}

#ifdef _DEBUG
		CN3Base::s_RenderInfo.nAlpha_Polygon += pBuffs[i]->nPrimitiveCount / 3; // Rendering Information Update...
#endif
	}
		
	m_nToDrawCount = 0; // 다 그렸다...

	// restore
	if(RS_old.dwAlpha != RS_current.dwAlpha) CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, RS_old.dwAlpha);
	if(RS_old.dwFog != RS_current.dwFog) CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, RS_old.dwFog);
	if(RS_old.dwCull != RS_current.dwCull) CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, RS_old.dwCull);
	if(RS_old.dwLgt != RS_current.dwLgt) CN3Base::s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, RS_old.dwLgt);
	if(RS_old.dwZWrite != RS_current.dwZWrite) CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, RS_old.dwZWrite);
	if(RS_old.dwSrcBlend != RS_current.dwSrcBlend) CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND,  RS_old.dwSrcBlend );
	if(RS_old.dwDestBlend != RS_current.dwDestBlend) CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, RS_old.dwDestBlend);
	if(RS_old.dwAO != RS_current.dwAO) CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   RS_old.dwAO);
	if(RS_old.dwAA1 != RS_current.dwAA1) CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, RS_old.dwAA1);
	if(RS_old.dwAA2 != RS_current.dwAA2) CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, RS_old.dwAA2);
	if(RS_old.dwCO != RS_current.dwCO) CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   RS_old.dwCO);
	if(RS_old.dwCA1 != RS_current.dwCA1) CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, RS_old.dwCA1);
	if(RS_old.dwCA2 != RS_current.dwCA2) CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, RS_old.dwCA2);
	if(RS_old.dwPointSampling != RS_current.dwPointSampling) CN3Base::s_lpD3DDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, RS_old.dwPointSampling);

*/
}

__AlphaPrimitive* CN3AlphaPrimitiveManager::Add()
{
	//	__ASSERT(m_nToDrawCount < MAX_ALPHAPRIMITIVE_BUFFER, "Alpha Primitive Manager : Overflow Buffer.");
	if (m_nToDrawCount >= MAX_ALPHAPRIMITIVE_BUFFER)
		return nullptr;
	m_nToDrawCount++;
	return &(m_Buffers[m_nToDrawCount - 1]);
}

/*
__AlphaPrimitive* CN3AlphaPrimitiveManager::Add(	__Vector3& vCamera, 
													uint32_t dwBlendSrc, 
													uint32_t dwBlendDest, 
													int	nRenderFlags, 
													LPDIRECT3DTEXTURE8 lpTex, 
													uint32_t dwFVF, 
													int nPrimitiveCount, 
													D3DPRIMITIVETYPE ePrimitiveType, 
													uint32_t dwPrimitiveSize, 
													BOOL bUseVB,
													const void* pwIndices, 
													int nVertexCount,
													const void* pVertices,
													const __Matrix44& MtxWorld )
{
	__ASSERT(m_nToDrawCount < MAX_ALPHAPRIMITIVE_BUFFER, "Alpha primnitive buffer is full");
	// 메인렌더링시 반드시 이 클래스의 Render() 를 한번 호출해주어야 버퍼를 비워준다..

	__Vector3 vPos = *((__Vector3*)pVertices);

	m_Buffers[m_nToDrawCount].fCameraDistance	= (vCamera - vPos).Magnitude();
	m_Buffers[m_nToDrawCount].dwBlendSrc		= dwBlendSrc;
	m_Buffers[m_nToDrawCount].dwBlendDest		= dwBlendDest;
	m_Buffers[m_nToDrawCount].nRenderFlags		= nRenderFlags;
	m_Buffers[m_nToDrawCount].lpTex				= lpTex;
	m_Buffers[m_nToDrawCount].dwFVF				= dwFVF;
	m_Buffers[m_nToDrawCount].nPrimitiveCount	= nPrimitiveCount;
	m_Buffers[m_nToDrawCount].ePrimitiveType	= ePrimitiveType;
	m_Buffers[m_nToDrawCount].dwPrimitiveSize	= dwPrimitiveSize;
	m_Buffers[m_nToDrawCount].bUseVB			= bUseVB;
	m_Buffers[m_nToDrawCount].pwIndices			= pwIndices;
	m_Buffers[m_nToDrawCount].nVertexCount		= nVertexCount;
	m_Buffers[m_nToDrawCount].pVertices			= pVertices;
	m_Buffers[m_nToDrawCount].MtxWorld			= MtxWorld;

	m_nToDrawCount++;
	return &(m_Buffers[m_nToDrawCount-1]);
}
*/

int CN3AlphaPrimitiveManager::SortByCameraDistance(const void* pArg1, const void* pArg2)
{
	__AlphaPrimitive* pObj1 = *((__AlphaPrimitive**) pArg1);
	__AlphaPrimitive* pObj2 = *((__AlphaPrimitive**) pArg2);

	if (pObj1->fCameraDistance > pObj2->fCameraDistance)
		return -1; // 거리가 먼것부터 소팅..
	else if (pObj1->fCameraDistance < pObj2->fCameraDistance)
		return 1;
	else
		return 0;
}
