// N3FXPartBottomBoard.cpp: implementation of the CN3FXPartBillBoard class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXPartBottomBoard.h"
#include "N3FXBundle.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3FXPartBottomBoard::CN3FXPartBottomBoard()
{
	m_iVersion = SUPPORTED_PART_VERSION;
	m_fSizeX = m_fSizeZ = 1.0f;

	m_iTexIdx = 0;
	m_dwCurrColor = 0xffffffff;

	m_bTexLoop = false;

	m_pVB = NULL;
	m_vUnit = NULL;

	m_fScaleVelX = 0.0f;
	m_fScaleVelZ = 0.0f;
	m_fScaleAccelX = 0.0f;
	m_fScaleAccelZ = 0.0f;
	CreateVB();

	m_fGap = 0.0f;
}

CN3FXPartBottomBoard::~CN3FXPartBottomBoard()
{
	if(m_pVB)
	{
		delete[] m_pVB;
		m_pVB = NULL;
	}

	if(m_vUnit)
	{
		delete[] m_vUnit;
		m_vUnit = NULL;
	}
}


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////


//
//
//
#ifdef _N3TOOL
bool CN3FXPartBottomBoard::ParseScript(char* szCommand, char* szBuff0, char* szBuff1, char* szBuff2, char* szBuff3)
{
	if(CN3FXPartBase::ParseScript(szCommand, szBuff0, szBuff1, szBuff2, szBuff3)) return true;

	//	보드 크기.
	if(lstrcmpi(szCommand, "<ground_size>")==0)
	{
		m_fCurrSizeX = m_fSizeX = atof(szBuff0);
		m_fCurrSizeZ = m_fSizeZ = atof(szBuff1);
/*
		for(int i=0;i<NUM_VERTEX_BOTTOM;i++)
		{
			m_vUnit[i].x *= m_fSizeX;
			m_vUnit[i].z *= m_fSizeZ;
		}
*/
		return true;
	}
	if(lstrcmpi(szCommand, "<tex_loop>")==0)
	{
		if(lstrcmpi(szBuff0,"true")==0) m_bTexLoop = true;
		else if(lstrcmpi(szBuff0,"false")==0) m_bTexLoop = false;
		return true;
	}
	if(lstrcmpi(szCommand, "<ground_scale_velocity>")==0)
	{
		m_fScaleVelX = atof(szBuff0);
		m_fScaleVelZ = atof(szBuff1);
		return true;
	}
	if(lstrcmpi(szCommand, "<ground_scale_acceleration>")==0)
	{
		m_fScaleAccelX = atof(szBuff0);
		m_fScaleAccelZ = atof(szBuff1);
		return true;
	}
	if(lstrcmpi(szCommand, "<ground_gap>")==0)
	{
		m_fGap = atof(szBuff0);
		return true;
	}
	return false;
}
#endif // end of _N3TOOL


//
//
//
void CN3FXPartBottomBoard::CreateVB()
{
	if(m_vUnit)
	{
		delete[] m_vUnit;
		m_vUnit = NULL;
	}
	m_vUnit = new __Vector3[NUM_VERTEX_BOTTOM];

	m_vUnit[0].Set(0.0f, 0.1f, 0.0f);
	m_vUnit[9].Set(-0.5f, 0.1f, -0.5f);
	m_vUnit[8].Set(0.0f, 0.1f, -0.5f);
	m_vUnit[7].Set(0.5f, 0.1f, -0.5f);
	m_vUnit[6].Set(0.5f, 0.1f, 0.0f);
	m_vUnit[5].Set(0.5f, 0.1f, 0.5f);
	m_vUnit[4].Set(0.0f, 0.1f, 0.5f);
	m_vUnit[3].Set(-0.5f, 0.1f, 0.5f);
	m_vUnit[2].Set(-0.5f, 0.1f, 0.0f);
	m_vUnit[1] = m_vUnit[9];
	
	if(m_pVB)
	{
		delete[] m_pVB;
		m_pVB = NULL;
	}
	m_pVB = new	__VertexXyzColorT1[NUM_VERTEX_BOTTOM];
	
	m_pVB[0].Set(-1.0f, 0.1f, -1.0f, m_dwCurrColor, 0.5f, 0.5f);
	m_pVB[9].Set(-1.5f, 0.1f, -1.5f, m_dwCurrColor, 0.0f, 1.0f);
	m_pVB[8].Set(-1.0f, 0.1f, -1.5f, m_dwCurrColor, 0.5f, 1.0f);
	m_pVB[7].Set(-1.5f, 0.1f, -1.5f, m_dwCurrColor, 1.0f, 1.0f);
	m_pVB[6].Set(-1.5f, 0.1f, -1.0f, m_dwCurrColor, 1.0f, 0.5f);
	m_pVB[5].Set(-1.5f, 0.1f, -1.5f, m_dwCurrColor, 1.0f, 0.0f);
	m_pVB[4].Set(-1.0f, 0.1f, -1.5f, m_dwCurrColor, 0.5f, 0.0f);
	m_pVB[3].Set(-1.5f, 0.1f, -1.5f, m_dwCurrColor, 0.0f, 0.0f);
	m_pVB[2].Set(-1.5f, 0.1f, -1.0f, m_dwCurrColor, 0.0f, 0.5f);
	m_pVB[1] = m_pVB[9];

	for(int i=0;i<NUM_VERTEX_BOTTOM;i++)
	{
		m_vUnit[i].y = GetGroundHeight(m_vUnit[i].x, m_vUnit[i].z);
		m_pVB[i].y = GetGroundHeight(m_pVB[i].x, m_pVB[i].z);
	}
}


//
//	init...
//
void CN3FXPartBottomBoard::Init()
{
	CN3FXPartBase::Init();

	m_iTexIdx = 0;
	m_dwCurrColor = 0xffffffff;
	m_vCurrPos = m_vPos;
	m_vCurrVelocity = m_vVelocity;

	m_fCurrScaleVelX = m_fScaleVelX;
	m_fCurrScaleVelZ = m_fScaleVelZ;

	m_fCurrSizeX = m_fSizeX;
	m_fCurrSizeZ = m_fSizeZ;
}


//
//
//
bool CN3FXPartBottomBoard::Load(HANDLE hFile)
{
	if(!CN3FXPartBase::Load(hFile)) return false;

	DWORD dwRWC = 0;

	ReadFile(hFile, &m_fSizeX, sizeof(float), &dwRWC, NULL);
	ReadFile(hFile, &m_fSizeZ, sizeof(float), &dwRWC, NULL);

	ReadFile(hFile, &m_fScaleVelX, sizeof(float), &dwRWC, NULL);
	ReadFile(hFile, &m_fScaleVelZ, sizeof(float), &dwRWC, NULL);

	ReadFile(hFile, &m_bTexLoop, sizeof(bool), &dwRWC, NULL);

	if(m_iVersion>=1)
	{
		ReadFile(hFile, &m_fGap, sizeof(float), &dwRWC, NULL);
	}

	// NOTE: This should ideally just be an assertion, but we'll continue to allow it to run
	// and otherwise be broken for now.
#if defined(_DEBUG)
	if (m_iVersion > SUPPORTED_PART_VERSION)
	{
		TRACE(
			"!!! WARNING: CN3FXPartBottomBoard::Load(%s) encountered version %d (base version %d). Needs support!",
			m_pRefBundle != nullptr ? m_pRefBundle->FileName().c_str() : "<unknown>",
			m_iVersion,
			m_iBaseVersion);
	}
#endif

	CreateVB();
	Init();

	return true;
}


//
//
//
bool CN3FXPartBottomBoard::Save(HANDLE hFile)
{
	if(!CN3FXPartBase::Save(hFile)) return false;

	DWORD dwRWC = 0;

	WriteFile(hFile, &m_fSizeX, sizeof(float), &dwRWC, NULL);
	WriteFile(hFile, &m_fSizeZ, sizeof(float), &dwRWC, NULL);

	WriteFile(hFile, &m_fScaleVelX, sizeof(float), &dwRWC, NULL);
	WriteFile(hFile, &m_fScaleVelZ, sizeof(float), &dwRWC, NULL);

	WriteFile(hFile, &m_bTexLoop, sizeof(bool), &dwRWC, NULL);	
	
	//version 1....
	WriteFile(hFile, &m_fGap, sizeof(float), &dwRWC, NULL);	
	return true;
}


//
//
//
void CN3FXPartBottomBoard::Start()
{
	CN3FXPartBase::Start();
}


//
//
//
void CN3FXPartBottomBoard::Stop()
{
	CN3FXPartBase::Stop();
}


//
//
//
bool CN3FXPartBottomBoard::Tick()
{
	if(!CN3FXPartBase::Tick()) return false;

	if(m_bTexLoop)	m_iTexIdx = (int)(m_fCurrLife * m_fTexFPS) % m_iNumTex;
	else m_iTexIdx = (int)(m_fCurrLife * m_fTexFPS);

	if(m_fCurrLife<=m_fFadeIn)
	{
		uint32_t Alpha = (uint32_t)(255.0f * m_fCurrLife / m_fFadeIn);
		m_dwCurrColor = (Alpha<<24) + 0x00ffffff;		
	}
	else m_dwCurrColor = 0xffffffff;

	if(m_dwState==FX_PART_STATE_DYING)
	{
		float TotalLife = m_fFadeIn + m_fLife + m_fFadeOut;
		if(m_fCurrLife >= TotalLife)
		{
			m_dwCurrColor = 0x00ffffff;
		}
		else
		{
			uint32_t Alpha = (uint32_t)(255.0f * (TotalLife - m_fCurrLife) / m_fFadeOut);
			m_dwCurrColor = (Alpha<<24) + 0x00ffffff;
		}
	}

	//위치이동..
	m_vCurrVelocity += m_vAcceleration*CN3Base::s_fSecPerFrm;
	m_vCurrPos += m_vCurrVelocity*CN3Base::s_fSecPerFrm;

	//회전...
	__Matrix44 mtxRot;
	mtxRot.Identity();
	mtxRot.RotationY(m_fCurrLife*m_vRotVelocity.y);

	//스케일변환..
	m_fCurrScaleVelX += m_fScaleAccelX*CN3Base::s_fSecPerFrm;
	m_fCurrScaleVelZ += m_fScaleAccelZ*CN3Base::s_fSecPerFrm;
	m_fCurrSizeX += m_fCurrScaleVelX*CN3Base::s_fSecPerFrm;
	m_fCurrSizeZ += m_fCurrScaleVelZ*CN3Base::s_fSecPerFrm;

	if(m_fCurrSizeX < 0.0f) m_fCurrSizeX = 0.0f;
	if(m_fCurrSizeZ < 0.0f) m_fCurrSizeZ = 0.0f;

	__Vector3 vTmp;
	if(m_pRefBundle->m_bDependScale)
	{
		for(int i=0;i<NUM_VERTEX_BOTTOM;i++)
		{
			vTmp = m_pVB[i];
			m_pVB[i].x = m_vUnit[i].x * m_fCurrSizeX * m_pRefBundle->m_fTargetScale;
			m_pVB[i].z = m_vUnit[i].z * m_fCurrSizeZ * m_pRefBundle->m_fTargetScale;
			m_pVB[i] *= mtxRot;
			m_pVB[i].x += m_pRefBundle->m_vPos.x + m_vCurrPos.x;
			m_pVB[i].z += m_pRefBundle->m_vPos.z + m_vCurrPos.z;

			if(vTmp.x != m_pVB[i].x || vTmp.z != m_pVB[i].z) m_pVB[i].y = GetGroundHeight(m_pVB[i].x, m_pVB[i].z) + m_fGap;
			else m_pVB[i].y = vTmp.y;

			m_pVB[i].color = m_dwCurrColor;
		}
	}
	else
	{
		for(int i=0;i<NUM_VERTEX_BOTTOM;i++)
		{
			vTmp = m_pVB[i];
			m_pVB[i].x = m_vUnit[i].x * m_fCurrSizeX;
			m_pVB[i].z = m_vUnit[i].z * m_fCurrSizeZ;
			m_pVB[i] *= mtxRot;
			m_pVB[i].x += m_pRefBundle->m_vPos.x + m_vCurrPos.x;
			m_pVB[i].z += m_pRefBundle->m_vPos.z + m_vCurrPos.z;
			
			if(vTmp.x != m_pVB[i].x || vTmp.z != m_pVB[i].z) m_pVB[i].y = GetGroundHeight(m_pVB[i].x, m_pVB[i].z) + m_fGap;
			else m_pVB[i].y = vTmp.y;

			m_pVB[i].color = m_dwCurrColor;
		}
	}
	return true;
}


//
//
//
bool CN3FXPartBottomBoard::IsDead()
{
	float TotalLife = m_fFadeIn + m_fLife + m_fFadeOut;
	if(m_fCurrLife >= TotalLife) return true;
	return false;
}


//
//	render...
//	일단은 파티클 하나씩 그리고....
//	나중에는 같은 텍스쳐 쓰는 것들끼리 묶어서 그리자...
//
void CN3FXPartBottomBoard::Render()
{
	if(m_iTexIdx >= m_iNumTex) return;

	if(m_bAlpha) // Alpha 사용
	{
		__AlphaPrimitive* pAP = s_AlphaMgr.Add();
		if(pAP)
		{
			pAP->bUseVB				= FALSE;
			pAP->dwBlendDest		= m_dwDestBlend;
			pAP->dwBlendSrc			= m_dwSrcBlend;
			pAP->dwFVF				= FVF_XYZCOLORT1;
			pAP->dwPrimitiveSize	= sizeof(__VertexXyzColorT1);
			pAP->fCameraDistance	= (m_pVB[0] - s_CameraData.vEye).Magnitude();
			
			if(m_ppRefTex[m_iTexIdx]) pAP->lpTex = m_ppRefTex[m_iTexIdx]->Get();
			else pAP->lpTex = NULL;

			__Matrix44 mtxWorld;
			mtxWorld.Identity();
			pAP->MtxWorld = mtxWorld;
			pAP->nRenderFlags = m_dwRenderFlag;
			pAP->ePrimitiveType		= D3DPT_TRIANGLEFAN;
			pAP->nPrimitiveCount	= 8;
			pAP->nVertexCount		= NUM_VERTEX_BOTTOM;
			pAP->pVertices			= &(m_pVB[0]);
			pAP->pwIndices			= NULL;
		}

		return; // 렌더링 안하지롱.
	}
	else 
	{
		CN3Base::s_lpD3DDev->SetFVF(FVF_XYZCOLORT1);

		if(m_ppRefTex[m_iTexIdx]) 
			CN3Base::s_lpD3DDev->SetTexture(0, m_ppRefTex[m_iTexIdx]->Get());
		else CN3Base::s_lpD3DDev->SetTexture(0, NULL);

		s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );		
		s_lpD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		s_lpD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		DWORD dwCullMode, dwZWriteEnable, dwZBufferEnable, dwLight, dwAlpha;
		s_lpD3DDev->GetRenderState( D3DRS_ALPHABLENDENABLE, &dwAlpha);
		s_lpD3DDev->GetRenderState( D3DRS_ZWRITEENABLE, &dwZWriteEnable);
		s_lpD3DDev->GetRenderState( D3DRS_ZENABLE, &dwZBufferEnable );
		s_lpD3DDev->GetRenderState( D3DRS_CULLMODE, &dwCullMode );
		s_lpD3DDev->GetRenderState( D3DRS_LIGHTING, &dwLight );

		if(m_bAlpha != dwAlpha)
		{
			s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, m_bAlpha);
			s_lpD3DDev->SetRenderState( D3DRS_SRCBLEND, m_dwSrcBlend );
			s_lpD3DDev->SetRenderState( D3DRS_DESTBLEND, m_dwDestBlend );
		}
		if(m_dwZEnable != dwZBufferEnable) s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, m_dwZEnable);
		if(m_dwZWrite != dwZWriteEnable) s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, m_dwZWrite);
		if(m_dwLight != dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, m_dwLight);
		if(m_dwDoubleSide != dwCullMode) s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, m_dwDoubleSide);

		HRESULT hr = CN3Base::s_lpD3DDev->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 8, m_pVB, sizeof(__VertexXyzColorT1));

		if(m_bAlpha != dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
		if(m_dwZEnable != dwZBufferEnable) s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZBufferEnable);
		if(m_dwZWrite != dwZWriteEnable) s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable);
		if(m_dwLight != dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
		if(m_dwDoubleSide != dwCullMode) s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCullMode);
	}
}

void CN3FXPartBottomBoard::Duplicate(CN3FXPartBottomBoard* pSrc)
{
	CN3FXPartBase::Duplicate(pSrc);

	m_fSizeX = pSrc->m_fSizeX;
	m_fSizeZ = pSrc->m_fSizeZ;

	m_fScaleVelX = pSrc->m_fScaleVelX;
	m_fScaleVelZ = pSrc->m_fScaleVelZ;

	m_fScaleAccelX = pSrc->m_fScaleAccelX;
	m_fScaleAccelZ = pSrc->m_fScaleAccelZ;

	m_bTexLoop = pSrc->m_bTexLoop;

	m_fGap = pSrc->m_fGap;
	
	CreateVB();
	Init();
	return;
}
