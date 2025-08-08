// N3FXPartBillBoard.cpp: implementation of the CN3FXPartBillBoard class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3FXPartBillBoard.h"
#include "N3FXBundle.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3FXPartBillBoard::CN3FXPartBillBoard()
{
	m_iVersion = SUPPORTED_PART_VERSION;

	m_iNum = 1;
	m_fSizeX = m_fSizeY = 1.0f;

	m_iTexIdx = 0;
	m_dwCurrColor = 0xffffffff;
	
	m_pVB = NULL;
	m_fRadius = 0.0f;

	m_bTexLoop = false;

	m_bRoateOnlyY = false;

	m_fScaleVelX = 0;
	m_fScaleVelY = 0;
	m_fScaleAccelX = 0;
	m_fScaleAccelY = 0;

	m_mtxRot.Identity();

	CreateVB();
}

CN3FXPartBillBoard::~CN3FXPartBillBoard()
{
	if(m_pVB)
	{
		delete[] m_pVB;
		m_pVB = NULL;
	}
}


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////


//
//
//
#ifdef _N3TOOL
bool CN3FXPartBillBoard::ParseScript(char* szCommand, char* szBuff0, char* szBuff1, char* szBuff2, char* szBuff3)
{
	if(CN3FXPartBase::ParseScript(szCommand, szBuff0, szBuff1, szBuff2, szBuff3)) return true;

	//	보드 갯수.
	if(lstrcmpi(szCommand, "<billboard_count>")==0)
	{
		m_iNum = atoi(szBuff0);
		if(m_iNum>0) CreateVB();
		return true;
	}

	//	보드 크기.
	if(lstrcmpi(szCommand, "<billboard_size>")==0)
	{
		m_fSizeX = atof(szBuff0);
		m_fSizeY = atof(szBuff1);

		m_fCurrSizeX = m_fSizeX;
		m_fCurrSizeY = m_fSizeY;
		return true;
	}
	if(lstrcmpi(szCommand, "<tex_loop>")==0)
	{
		if(lstrcmpi(szBuff0,"true")==0) m_bTexLoop = true;
		else if(lstrcmpi(szBuff0,"false")==0) m_bTexLoop = false;

		return true;
	}
	if(lstrcmpi(szCommand, "<distance>")==0)
	{
		m_fRadius = atof(szBuff0);
		return true;
	}

	if(lstrcmpi(szCommand, "<Rotate_only_y>")==0)
	{
		if(lstrcmpi(szBuff0,"true")==0) m_bRoateOnlyY = true;
		else if(lstrcmpi(szBuff0,"false")==0) m_bRoateOnlyY = false;
		return true;
	}

	if(lstrcmpi(szCommand, "<billboard_scale_velocity>")==0)
	{
		m_fScaleVelX = atof(szBuff0);
		m_fScaleVelY = atof(szBuff1);

		m_fCurrScaleVelX = m_fScaleVelX;
		m_fCurrScaleVelY = m_fScaleVelY;
		return true;
	}

	if(lstrcmpi(szCommand, "<billboard_scale_acceleration>")==0)
	{
		m_fScaleAccelX = atof(szBuff0);
		m_fScaleAccelY = atof(szBuff1);
		return true;
	}

	if(lstrcmpi(szCommand, "<billboard_Rotation>")==0)
	{
		m_fRotBillBoardX = atof(szBuff0);
		m_fRotBillBoardY = atof(szBuff1);
		m_fRotBillBoardZ = atof(szBuff2);
		m_mtxRot.Identity();
		m_mtxRot.Rotation(D3DXToRadian(m_fRotBillBoardX),
						  D3DXToRadian(m_fRotBillBoardY),
						  D3DXToRadian(m_fRotBillBoardZ));
		//m_mtxRot.PosSet(0,0,0);
		return true;
	}

	return false;
}
#endif // end of _N3TOOL


//
//
//
void CN3FXPartBillBoard::CreateVB()
{
	if(m_pVB) delete[] m_pVB;
	m_pVB = new	__VertexXyzColorT1[m_iNum*4];

	m_vUnit[0].Set(-0.5f, 0.5f, 0.0f);
	m_vUnit[1].Set(0.5f, 0.5f, 0.0f);
	m_vUnit[2].Set(0.5f, -0.5f, 0.0f);
	m_vUnit[3].Set(-0.5f, -0.5f, 0.0f);
	
	for(int i=0;i<m_iNum;i++)
	{
		int idx = i*4;
		m_pVB[idx].Set(-0.5f, 0.5f, 0.0f, m_dwCurrColor, 0.0f, 0.0f);
		m_pVB[idx+1].Set(0.5f, 0.5f, 0.0f, m_dwCurrColor, 1.0f, 0.0f);
		m_pVB[idx+2].Set(0.5f, -0.5f, 0.0f, m_dwCurrColor, 1.0f, 1.0f);
		m_pVB[idx+3].Set(-0.5f, -0.5f, 0.0f, m_dwCurrColor, 0.0f, 1.0f);
	}
}


//
//	init...
//
void CN3FXPartBillBoard::Init()
{
	CN3FXPartBase::Init();

	m_iTexIdx = 0;
	m_dwCurrColor = 0xffffffff;

	m_vCurrPos = m_vPos;
	m_vCurrVelocity = m_vVelocity;

	m_fCurrScaleVelX = m_fScaleVelX;
	m_fCurrScaleVelY = m_fScaleVelY;

	m_fCurrSizeX = m_fSizeX;
	m_fCurrSizeY = m_fSizeY;
}


//
//
//
bool CN3FXPartBillBoard::Load(HANDLE hFile)
{
	if(!CN3FXPartBase::Load(hFile)) return false;

	DWORD dwRWC = 0;
	ReadFile(hFile, &m_iNum, sizeof(int), &dwRWC, NULL);
	ReadFile(hFile, &m_fSizeX, sizeof(float), &dwRWC, NULL);
	ReadFile(hFile, &m_fSizeY, sizeof(float), &dwRWC, NULL);

	ReadFile(hFile, &m_bTexLoop, sizeof(bool), &dwRWC, NULL);
	ReadFile(hFile, &m_fRadius, sizeof(float), &dwRWC, NULL);

	if(m_iVersion>=3) ReadFile(hFile, &m_bRoateOnlyY, sizeof(bool), &dwRWC, NULL);	

	if(m_iVersion>=4)
	{
		ReadFile(hFile, &m_fScaleVelX, sizeof(float), &dwRWC, NULL);
		ReadFile(hFile, &m_fScaleVelY, sizeof(float), &dwRWC, NULL);
		ReadFile(hFile, &m_fScaleAccelX, sizeof(float), &dwRWC, NULL);
		ReadFile(hFile, &m_fScaleAccelY, sizeof(float), &dwRWC, NULL);
	}

	if(m_iVersion>=5) ReadFile(hFile, &m_mtxRot, sizeof(m_mtxRot), &dwRWC, NULL);

	// NOTE: This should ideally just be an assertion, but we'll continue to allow it to run
	// and otherwise be broken for now.
#if defined(_DEBUG)
	if (m_iVersion > SUPPORTED_PART_VERSION)
	{
		TRACE(
			"!!! WARNING: CN3FXPartBillBoard::Load(%s) encountered version %d (base version %d). Needs support!",
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
bool CN3FXPartBillBoard::Save(HANDLE hFile)
{
	if(!CN3FXPartBase::Save(hFile)) return false;

	DWORD dwRWC = 0;
	WriteFile(hFile, &m_iNum, sizeof(int), &dwRWC, NULL);
	WriteFile(hFile, &m_fSizeX, sizeof(float), &dwRWC, NULL);
	WriteFile(hFile, &m_fSizeY, sizeof(float), &dwRWC, NULL);

	WriteFile(hFile, &m_bTexLoop, sizeof(bool), &dwRWC, NULL);
	WriteFile(hFile, &m_fRadius, sizeof(float), &dwRWC, NULL);

	if(m_iVersion>=3) WriteFile(hFile, &m_bRoateOnlyY, sizeof(bool), &dwRWC, NULL);
	
	WriteFile(hFile, &m_fScaleVelX, sizeof(float), &dwRWC, NULL);
	WriteFile(hFile, &m_fScaleVelY, sizeof(float), &dwRWC, NULL);
	WriteFile(hFile, &m_fScaleAccelX, sizeof(float), &dwRWC, NULL);
	WriteFile(hFile, &m_fScaleAccelY, sizeof(float), &dwRWC, NULL);

	if(m_iVersion>=5) WriteFile(hFile, &m_mtxRot, sizeof(m_mtxRot), &dwRWC, NULL);

	return true;
}


//
//
//
void CN3FXPartBillBoard::Start()
{
	CN3FXPartBase::Start();

}


//
//
//
void CN3FXPartBillBoard::Stop()
{
	CN3FXPartBase::Stop();
}


//
//
//
bool CN3FXPartBillBoard::Tick()
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

	//위치에 관한 틱틱...m_vCurrPos
	m_vCurrVelocity += m_vAcceleration*CN3Base::s_fSecPerFrm;
	m_vCurrPos += m_vCurrVelocity*CN3Base::s_fSecPerFrm;

	m_fCurrScaleVelX += m_fScaleAccelX * CN3Base::s_fSecPerFrm;
	m_fCurrScaleVelY += m_fScaleAccelY * CN3Base::s_fSecPerFrm;

	m_fCurrSizeX += m_fCurrScaleVelX * CN3Base::s_fSecPerFrm;
	m_fCurrSizeY += m_fCurrScaleVelY * CN3Base::s_fSecPerFrm;

	//	m_vUnit[0].Set(-0.5f, 0.5f, 0.0f);
	//m_vUnit[1].Set(0.5f, 0.5f, 0.0f);
	//m_vUnit[2].Set(0.5f, -0.5f, 0.0f);
	//m_vUnit[3].Set(-0.5f, -0.5f, 0.0f);

	m_vUnit[0].x = (-0.5f) * m_fCurrSizeX;
	m_vUnit[1].x = (0.5f) * m_fCurrSizeX;
	m_vUnit[2].x = (0.5f) * m_fCurrSizeX;
	m_vUnit[3].x = (-0.5f) * m_fCurrSizeX;

	m_vUnit[0].y = (0.5f) * m_fCurrSizeY;
	m_vUnit[1].y = (0.5f) * m_fCurrSizeY;
	m_vUnit[2].y = (-0.5f) * m_fCurrSizeY;
	m_vUnit[3].y = (-0.5f) * m_fCurrSizeY;
	
	m_vUnit[0].z = 0.0f;
	m_vUnit[1].z = 0.0f;
	m_vUnit[2].z = 0.0f;
	m_vUnit[3].z = 0.0f;

	m_vUnit[0] *= m_mtxRot;
	m_vUnit[1] *= m_mtxRot;
	m_vUnit[2] *= m_mtxRot;
	m_vUnit[3] *= m_mtxRot;

	return true;
}


//
//
//
bool CN3FXPartBillBoard::IsDead()
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
void CN3FXPartBillBoard::Render()
{
	if(m_iTexIdx >= m_iNumTex) return;

	//회전...
	__Matrix44 mtxRotZ;
	mtxRotZ.Identity();
	mtxRotZ.RotationZ(m_fCurrLife*m_vRotVelocity.x);

	if(!m_bRoateOnlyY)
	{
		//위치지정 & 나를 바라보게 셋팅..
		__Matrix44 mtxVI;
		mtxVI = s_CameraData.mtxViewInverse;
		__Vector3 vpp;
		vpp = mtxVI.Pos();
		mtxVI.PosSet(0,0,0);
	
		__Vector3 AbsoluteCurrPos = Rotate2AbsolutePos(m_vCurrPos);

		__Vector3 vRadiusPos = s_CameraData.vEye - (AbsoluteCurrPos + m_pRefBundle->m_vPos);
		if(vRadiusPos.Magnitude() <= m_fRadius)
		{
			vRadiusPos += (s_CameraData.vAt-s_CameraData.vEye) * (s_CameraData.fNP+0.1f);
		}
		else
		{
			vRadiusPos.Normalize();
			vRadiusPos *= m_fRadius;
		}
		
		__Vector3 vCalPos = AbsoluteCurrPos + m_pRefBundle->m_vPos + vRadiusPos;

		if(m_pRefBundle->m_bDependScale)
		{
			__Vector3 vUnit[4];
			for(int i=0;i<4;i++)
			{
				vUnit[i].x = m_vUnit[i].x * m_pRefBundle->m_fTargetScale;
				vUnit[i].y = m_vUnit[i].y * m_pRefBundle->m_fTargetScale;
				vUnit[i].z = 0.0f;
				vUnit[i] *= mtxRotZ;
			}

			for(int i=0;i<m_iNum;i++)
			{
				int idx = i*4;
				
				m_pVB[idx] = (vUnit[0] * mtxVI) + vCalPos;
				m_pVB[idx+1] = (vUnit[1] * mtxVI) + vCalPos;
				m_pVB[idx+2] = (vUnit[2] * mtxVI) + vCalPos;
				m_pVB[idx+3] = (vUnit[3] * mtxVI) + vCalPos;

				if(m_bOnGround)
				{
					float GroundHeight = GetGroundHeight(vCalPos.x, vCalPos.z);
					float NewY = GroundHeight - (vCalPos.y - vRadiusPos.y) + vUnit[0].y;


					m_pVB[idx].y = m_pVB[idx].y + NewY;
					m_pVB[idx+1].y = m_pVB[idx+1].y + NewY;
					m_pVB[idx+2].y = m_pVB[idx+2].y + NewY;
					m_pVB[idx+3].y = m_pVB[idx+3].y + NewY;
				}

				m_pVB[idx].color = m_dwCurrColor;
				m_pVB[idx+1].color = m_dwCurrColor;
				m_pVB[idx+2].color = m_dwCurrColor;
				m_pVB[idx+3].color = m_dwCurrColor;
			}
		}
		else
		{
			for(int i=0;i<m_iNum;i++)
			{
				int idx = i*4;

				m_pVB[idx] = (m_vUnit[0] * mtxRotZ * mtxVI) + vCalPos;
				m_pVB[idx+1] = (m_vUnit[1] * mtxRotZ * mtxVI) + vCalPos;
				m_pVB[idx+2] = (m_vUnit[2] * mtxRotZ * mtxVI) + vCalPos;
				m_pVB[idx+3] = (m_vUnit[3] * mtxRotZ * mtxVI) + vCalPos;

				if(m_bOnGround)
				{
					float GroundHeight = GetGroundHeight(vCalPos.x, vCalPos.z);
					float NewY = GroundHeight - (vCalPos.y - vRadiusPos.y) + m_vUnit[0].y;

					m_pVB[idx].y = m_pVB[idx].y + NewY;
					m_pVB[idx+1].y = m_pVB[idx+1].y + NewY;
					m_pVB[idx+2].y = m_pVB[idx+2].y + NewY;
					m_pVB[idx+3].y = m_pVB[idx+3].y + NewY;
				}

				m_pVB[idx].color = m_dwCurrColor;
				m_pVB[idx+1].color = m_dwCurrColor;
				m_pVB[idx+2].color = m_dwCurrColor;
				m_pVB[idx+3].color = m_dwCurrColor;
			}
		}
	}
	else
	{
		//위치지정 & 나를 바라보게 셋팅..
		__Vector3 AbsoluteCurrPos = Rotate2AbsolutePos(m_vCurrPos);
		__Vector3 vRadiusPos = s_CameraData.vEye - (AbsoluteCurrPos + m_pRefBundle->m_vPos);

		float ang;
		if(vRadiusPos.z==0.0f && vRadiusPos.x >= 0) ang =  __PI*(-0.5f);
		else if(vRadiusPos.z==0.0f && vRadiusPos.x < 0) ang =  __PI*(0.5f);
		else ang = -atan2(vRadiusPos.x, -vRadiusPos.z);

		__Matrix44 mtxRot;
		mtxRot.Identity();
		mtxRot.RotationY(ang);
		
		if(vRadiusPos.Magnitude() <= m_fRadius)
		{
			vRadiusPos += (s_CameraData.vAt-s_CameraData.vEye) * (s_CameraData.fNP+0.1f);
			vRadiusPos.y = 0;
		}
		else
		{
			vRadiusPos.Normalize();
			vRadiusPos *= m_fRadius;
			vRadiusPos.y = 0;
		}

		__Vector3 vCalPos = AbsoluteCurrPos + m_pRefBundle->m_vPos + vRadiusPos;

		if(m_pRefBundle->m_bDependScale)
		{
			__Vector3 vUnit[4];
			for(int i=0;i<4;i++)
			{
				//vUnit[i].x = m_vUnit[i].x * m_pRefBundle->m_vTargetScale.x;
				//vUnit[i].y = m_vUnit[i].y * m_pRefBundle->m_vTargetScale.y;
				vUnit[i].x = m_vUnit[i].x * m_pRefBundle->m_fTargetScale;
				vUnit[i].y = m_vUnit[i].y * m_pRefBundle->m_fTargetScale;
				vUnit[i].z = 0.0f;
				vUnit[i] *= mtxRotZ;
			}

			for(int i=0;i<m_iNum;i++)
			{
				int idx = i*4;

				m_pVB[idx] = (vUnit[0] * mtxRotZ * mtxRot) + vCalPos;
				m_pVB[idx+1] = (vUnit[1] * mtxRotZ * mtxRot) + vCalPos;
				m_pVB[idx+2] = (vUnit[2] * mtxRotZ * mtxRot) + vCalPos;
				m_pVB[idx+3] = (vUnit[3] * mtxRotZ * mtxRot) + vCalPos;

				if(m_bOnGround)
				{
					float GroundHeight = GetGroundHeight(vCalPos.x, vCalPos.z);
				
					m_pVB[idx].y = m_pVB[idx].y - m_pVB[idx+3].y + GroundHeight;
					m_pVB[idx+1].y = m_pVB[idx+1].y - m_pVB[idx+2].y + GroundHeight;					
					m_pVB[idx+2].y = GroundHeight;
					m_pVB[idx+3].y = GroundHeight;					
				}
				
				m_pVB[idx].color = m_dwCurrColor;
				m_pVB[idx+1].color = m_dwCurrColor;
				m_pVB[idx+2].color = m_dwCurrColor;
				m_pVB[idx+3].color = m_dwCurrColor;
			}
		}
		else
		{
			for(int i=0;i<m_iNum;i++)
			{
				int idx = i*4;

				m_pVB[idx] = (m_vUnit[0] * mtxRotZ * mtxRot) + vCalPos;
				m_pVB[idx+1] = (m_vUnit[1] * mtxRotZ * mtxRot) + vCalPos;
				m_pVB[idx+2] = (m_vUnit[2] * mtxRotZ * mtxRot) + vCalPos;
				m_pVB[idx+3] = (m_vUnit[3] * mtxRotZ * mtxRot) + vCalPos;

				if(m_bOnGround)
				{
					float GroundHeight = GetGroundHeight(vCalPos.x, vCalPos.z);

					m_pVB[idx].y = m_pVB[idx].y - m_pVB[idx+3].y + GroundHeight;
					m_pVB[idx+1].y = m_pVB[idx+1].y - m_pVB[idx+2].y + GroundHeight;					
					m_pVB[idx+2].y = GroundHeight;
					m_pVB[idx+3].y = GroundHeight;					
				}

				
				m_pVB[idx].color = m_dwCurrColor;
				m_pVB[idx+1].color = m_dwCurrColor;
				m_pVB[idx+2].color = m_dwCurrColor;
				m_pVB[idx+3].color = m_dwCurrColor;
			}
		}
	}

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
			pAP->fCameraDistance	= CameraDist();

			if(m_ppRefTex[m_iTexIdx]) 
				pAP->lpTex = m_ppRefTex[m_iTexIdx]->Get();
			else pAP->lpTex = NULL;

			__Matrix44 mtxWorld;
			mtxWorld.Identity();
			pAP->MtxWorld = mtxWorld;
			pAP->nRenderFlags = m_dwRenderFlag;
			pAP->ePrimitiveType		= D3DPT_TRIANGLEFAN;
			pAP->nPrimitiveCount	= m_iNum*2;
			pAP->nVertexCount		= m_iNum*4;
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

		for(int i=0;i<m_iNum;i++)
			HRESULT hr = CN3Base::s_lpD3DDev->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, &(m_pVB[i*4]), sizeof(__VertexXyzColorT1));

		if(m_bAlpha != dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
		if(m_dwZEnable != dwZBufferEnable) s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZBufferEnable);
		if(m_dwZWrite != dwZWriteEnable) s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable);
		if(m_dwLight != dwLight) s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLight);
		if(m_dwDoubleSide != dwCullMode) s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCullMode);
	}
}


float CN3FXPartBillBoard::CameraDist()
{
	__Vector3 vA = m_pVB[1] - m_pVB[0];
	__Vector3 vB = m_pVB[3] - m_pVB[0];
	__Vector3 vN;
	vN.Cross(vA, vB);
	vN.Normalize();

	float D = -(vN.x*m_pVB[0].x + vN.y*m_pVB[0].y + vN.z*m_pVB[0].z);

	return (vN.x*s_CameraData.vEye.x + vN.y*s_CameraData.vEye.y + vN.z*s_CameraData.vEye.z + D);
}

__Vector3 CN3FXPartBillBoard::Rotate2AbsolutePos(__Vector3 vRelativePos)
{
	__Vector3 vAbsolutePos(0,0,1);
	__Vector3 vAxisZ(0,0,1);
	__Vector3 vDirAxis;

	__Quaternion qtRot;

	vDirAxis.Cross(vAxisZ, m_pRefBundle->m_vDir);
	int tmp;
	tmp = (int)(vDirAxis.x*10000.0f);
	vDirAxis.x = (float)(tmp)/10000.0f;
	tmp = (int)(vDirAxis.y*10000.0f);
	vDirAxis.y = (float)(tmp)/10000.0f;
	tmp = (int)(vDirAxis.z*10000.0f);
	vDirAxis.z = (float)(tmp)/10000.0f;

	if(vDirAxis.x==0.0f && vDirAxis.y==0.0f && vDirAxis.z==0.0f) vDirAxis.Set(0,1,0);

	float fDirAng = acos(vAxisZ.Dot(m_pRefBundle->m_vDir));

	qtRot.RotationAxis(vDirAxis, fDirAng);

	__Matrix44 RotMtx = qtRot;

	vAbsolutePos = vRelativePos * RotMtx;

	return vAbsolutePos;
}

void CN3FXPartBillBoard::Duplicate(CN3FXPartBillBoard* pSrc)
{
	if(!pSrc) return;

	CN3FXPartBase::Duplicate(pSrc);

	m_iNum = pSrc->m_iNum;
	m_fSizeX = pSrc->m_fSizeX;
	m_fSizeY = pSrc->m_fSizeY;

	m_fCurrSizeX = m_fSizeX;
	m_fCurrSizeY = m_fSizeY;

	m_bTexLoop = pSrc->m_bTexLoop;
	m_fRadius = pSrc->m_fRadius;
	m_bRoateOnlyY = pSrc->m_bRoateOnlyY;

	m_fScaleVelX = pSrc->m_fScaleVelX;
	m_fScaleVelY = pSrc->m_fScaleVelY;
	m_fScaleAccelX = pSrc->m_fScaleAccelX;
	m_fScaleAccelY = pSrc->m_fScaleAccelY;

	m_fCurrScaleVelX = m_fScaleVelX;
	m_fCurrScaleVelY = m_fScaleVelY;
	m_mtxRot = pSrc->m_mtxRot;
	
	CreateVB();
	Init();

	return;
}
