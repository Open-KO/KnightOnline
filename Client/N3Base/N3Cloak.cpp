// N3Cloak.cpp: implementation of the CN3Cloak class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3Cloak.h"
#include "N3Texture.h"
#include "N3PMeshInstance.h"
#include "N3Chr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CN3Cloak::CN3Cloak()
{
//	m_pPMesh = nullptr;
	m_pTex = nullptr;
	m_pParticle = nullptr;
	m_nLOD = -1;
	m_pPMesh = nullptr;
	m_pIndex = nullptr;
	m_pVertex = nullptr;
	m_fOffsetRecoveryTime = 0.0f;
	m_fPrevYaw = 0.0f;
	m_eAnchorPattern = AMP_NONE;
	m_fAnchorPreserveTime = 0.0f;
}

CN3Cloak::~CN3Cloak()
{
	delete[] m_pParticle;
	delete[] m_pIndex;
	delete[] m_pVertex;
}

void CN3Cloak::Release()
{
	delete[] m_pParticle;
	m_pParticle = nullptr;

	delete[] m_pIndex;
	m_pIndex = nullptr;

	delete[] m_pVertex;
	m_pVertex = nullptr;
}

void CN3Cloak::Init(CN3CPlug_Cloak *pPlugCloak)
{
	m_pTex = pPlugCloak->Tex();
	m_pPMesh = pPlugCloak->PMesh();
	__ASSERT(m_pPMesh && m_pTex, "IN CN3Cloak, PMesh or m_pTex is null");

	SetLOD(0);

	//
	m_GravityForce.x = 0.0f;
	m_GravityForce.z = 0.0005f;
	//m_GravityForce.y = -0.0025f;
	m_GravityForce.y = -0.0005f;

	m_Force.x = m_Force.y = m_Force.z = 0.0f;
}

void CN3Cloak::Tick(int nLOD, float fYaw, e_CloakMove eCloakMove)
{
	//SetLOD(nLOD);	

	//
//	static float fForceDelay = 0.0f;
//	static float fForceDelayLimit = 4.0f;
//	fForceDelay += s_fSecPerFrm;
//	if (fForceDelay > fForceDelayLimit)
//	{
		//float x = (rand()%30)*0.0001f;
		//float z = (rand()%60)*0.0001f;
		//SetForce(dif.x, dif.y, dif.z);
//		m_Force.x = dif.x;
//		m_Force.y = dif.y;
//		m_Force.z = dif.z;
//		fForceDelay = 0.0f;
//		fForceDelayLimit = 2.0f + rand()%10;
//	}
	m_fAnchorPreserveTime -= s_fSecPerFrm;
	if (m_eAnchorPattern != AMP_NONE)
	{
		if (m_fAnchorPreserveTime < 0.0f)		
		{
			RestoreAnchorLine();
		}
	}

	TickByPlayerMotion(eCloakMove);
	TickYaw(fYaw);
	

	UpdateLocalForce();
	ApplyForce();

	m_Force.x = m_Force.y = m_Force.z = 0.0f;
}

void CN3Cloak::Render(__Matrix44 &mtx)
{
	if (m_nLOD < 0)	return;

	s_lpD3DDev->SetTransform( D3DTS_WORLD, &mtx);
	DWORD dwCull, dwLight;
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLight);
	s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCull);
	
	// 
	//s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, FALSE);
	if (dwCull != D3DCULL_NONE)
		s_lpD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);

	s_lpD3DDev->SetTexture(0, m_pTex->Get());
	s_lpD3DDev->SetTexture(1, nullptr);
	
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
//	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
//	s_lpD3DDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
//	s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);


	s_lpD3DDev->SetFVF(FVF_VNT1);
	s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_nVertexCount, m_nIndexCount/3, m_pIndex, D3DFMT_INDEX16 ,m_pVertex, sizeof(__VertexT1));

	//
	__VertexXyzColor	Vtx[2];
/*
	__VertexT1 *pTemp = m_pVertex;
	Vtx[0].Set(pTemp->x, pTemp->y, pTemp->z, 0xffffffff);
	Vtx[1].Set(1.0f, 1.0f, 1.0f, 0xffffffff);
	s_lpD3DDev->SetVertexShader(FVF_CV);
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, Vtx, sizeof(__VertexXyzColor));
	pTemp++;
	Vtx[0].Set(pTemp->x, pTemp->y, pTemp->z, 0xffff0000);
	Vtx[1].Set(1.0f, 1.0f, 1.0f, 0xffffffff);
	s_lpD3DDev->SetVertexShader(FVF_CV);
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, Vtx, sizeof(__VertexXyzColor));
*/

	for (int i = 0 ; i< m_nGridH;i++)
	{
		for (int j=0;j<m_nGridW-1;j++)
		{
			// Horizontal
			int nIndex = i*m_nGridW+j;
			Vtx[0].Set(m_pParticle[nIndex].x, m_pParticle[nIndex].y, m_pParticle[nIndex].z, 0xffffffff);
			nIndex++;
			Vtx[1].Set(m_pParticle[nIndex].x, m_pParticle[nIndex].y, m_pParticle[nIndex].z, 0xffffffff);
			s_lpD3DDev->SetFVF(FVF_CV);
			s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, Vtx, sizeof(__VertexXyzColor));
		}
	}
	for (int i = 0 ; i< m_nGridH-1;i++)
	{
		for (int j=0;j<m_nGridW;j++)
		{
			// Vertical
			int nIndex = i*m_nGridW+j;
			Vtx[0].Set(m_pParticle[nIndex].x, m_pParticle[nIndex].y, m_pParticle[nIndex].z, 0xffffffff);
			nIndex+=m_nGridW;
			Vtx[1].Set(m_pParticle[nIndex].x, m_pParticle[nIndex].y, m_pParticle[nIndex].z, 0xffffffff);
			s_lpD3DDev->SetFVF(FVF_CV);
			s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, Vtx, sizeof(__VertexXyzColor));
		}
	}

	// restore renderstate.
	if (dwCull != D3DCULL_NONE)
		s_lpD3DDev->SetRenderState( D3DRS_CULLMODE, dwCull);
}


#define SPRING_TOLERANCE	0.000001f
#define SPRING_COEFFICIENT		0.3f
void CN3Cloak::UpdateLocalForce()
{
	float length;

	int i, j, index, idx;
	__Vector3 up, down, left, right;
	const float nSub = 0.2f;

	for ( i = 0; i < m_nGridH; i ++ )
	{
		for ( j = 0; j < m_nGridW; j ++ )
		{
			index = i*m_nGridW + j;			
			
			up.x = down.x = left.x = right.x = 0; 
			up.y = down.y = left.y = right.y = 0; 
			up.z = down.z = left.z = right.z = 0; 
			
			if ( j-1 >= 0 )
			{
				idx = i*m_nGridW + (j-1);
				up.x = m_pParticle[idx].x - m_pParticle[index].x; 
				up.y = m_pParticle[idx].y - m_pParticle[index].y; 
				up.z = m_pParticle[idx].z - m_pParticle[index].z;
				
				length = sqrt(up.x * up.x + up.y * up.y + up.z * up.z);

				if (length > SPRING_TOLERANCE)
				{
					length = 1.0f-nSub/length;

					up.x = (length ) * up.x;
					up.y = (length ) * up.y;
					up.z = (length ) * up.z;
				}
			}
			if ( j+1 < m_nGridW)
			{
				idx = i * m_nGridW + j+1;
				down.x = m_pParticle[idx].x - m_pParticle[index].x; 
				down.y = m_pParticle[idx].y - m_pParticle[index].y; 
				down.z = m_pParticle[idx].z - m_pParticle[index].z; 

				length = sqrt(down.x * down.x + down.y * down.y + down.z * down.z);
				if (length > SPRING_TOLERANCE)
				{				
					length = 1.0f-nSub/length;
					down.x = (length ) * down.x ;
					down.y = (length ) * down.y ;
					down.z = (length ) * down.z ;
				}
			}
			if ( i-1 >= 0 )
			{
				idx = (i-1) * m_nGridW + j;
				left.x = m_pParticle[idx].x - m_pParticle[index].x; 
				left.y = m_pParticle[idx].y - m_pParticle[index].y; 
				left.z = m_pParticle[idx].z - m_pParticle[index].z; 

				length = sqrt(left.x * left.x + left.y * left.y + left.z * left.z);
				if (length > SPRING_TOLERANCE)
				{
					length = 1.0f-nSub/length;					
					left.x = (length ) * left.x ;
					left.y = (length ) * left.y ;
					left.z = (length ) * left.z ;
				}
			}
			if ( i+1 < m_nGridH )
			{
				idx = (i+1) * m_nGridW + j;
				right.x = m_pParticle[idx].x - m_pParticle[index].x; 
				right.y = m_pParticle[idx].y - m_pParticle[index].y; 
				right.z = m_pParticle[idx].z - m_pParticle[index].z; 

				length = sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
				if (length > SPRING_TOLERANCE)
				{
					length = 1.0f-nSub/length;
					right.x = (length ) * right.x ;
					right.y = (length ) * right.y ;
					right.z = (length ) * right.z ;
				}
			}
			
			m_pParticle[index].LocalForce.x = (up.x + down.x + left.x + right.x)*SPRING_COEFFICIENT;
			m_pParticle[index].LocalForce.y = (up.y + down.y + left.y + right.y)*SPRING_COEFFICIENT;
			m_pParticle[index].LocalForce.z = (up.z + down.z + left.z + right.z)*SPRING_COEFFICIENT;
		}
	}
}

void CN3Cloak::ApplyForce()
{
	const float energy_loss = 0.99f;
	__Particle *pParticle = m_pParticle;
	__VertexT1	*pVtx;
	for (int i=m_nGridW;i<m_nGridH*m_nGridW;i++)
	{
		pParticle = m_pParticle+i;
		pVtx = m_pVertex+i+m_nGridW*CLOAK_SKIP_LINE;	// 'm_nGridW*CLOAK_SKIP_LINE' means non-movable points..
		{
			// Old.

			pParticle->vx += (m_GravityForce.x + m_Force.x + pParticle->LocalForce.x) ;
			pParticle->vy += (m_GravityForce.y + m_Force.y + pParticle->LocalForce.y) ;
			pParticle->vz += (m_GravityForce.z + m_Force.z + pParticle->LocalForce.z) ;

			pParticle->x += pParticle->vx ;
			pParticle->y += pParticle->vy ;
			pParticle->z += pParticle->vz ;

			pVtx->x += pParticle->vx;
			pVtx->y += pParticle->vy;
			pVtx->z += pParticle->vz;

			// loss process
			pParticle->vx *= energy_loss;
			pParticle->vy *= energy_loss;;
			pParticle->vz *= energy_loss;			
		}
	}
}

void CN3Cloak::SetLOD(int nLevel)
{
	if (nLevel == m_nLOD)	return;

	if (m_pIndex)
		delete[] m_pIndex, m_pIndex = nullptr;

	memset(m_vOffset, 0, sizeof(__Vector3)*CLOAK_MAX_WIDTH);

	switch(nLevel)
	{
	case 0:
		{
			m_nGridW = 7;
			m_nGridH = 5;

			int nVertexCount = m_pPMesh->GetMaxNumVertices();
			int nIndexCount = m_pPMesh->GetMaxNumIndices();
			m_pVertex = new __VertexT1[nVertexCount];
			memcpy(m_pVertex, m_pPMesh->GetVertices(), sizeof(__VertexT1)*nVertexCount);
			m_pIndex = new uint16_t[nIndexCount];
			memcpy(m_pIndex, m_pPMesh->GetIndices(), sizeof(uint16_t)*nIndexCount);
			m_nVertexCount = nVertexCount;
			m_nIndexCount = nIndexCount;
			//
		}
		break;
	case 1:
		{
/*
			m_nGridW = 5;
			m_nGridH = 4;

			int nVertexCount = 25;
			int nIndexCount = 96;
			m_pVertex = new __VertexT1[nVertexCount];
			m_pIndex = new uint16_t[nIndexCount];
			__VertexT1 *pVtx0 = m_pPMesh->GetVertices();
			memcpy(m_pVertex, pVtx0, sizeof(__VertexT1)*5);
			memcpy(m_pVertex+5, pVtx0+6, sizeof(__VertexT1)*5);
			memcpy(m_pVertex+10, pVtx0+12, sizeof(__VertexT1)*5);
			memcpy(m_pVertex+15, pVtx0+18, sizeof(__VertexT1)*5);
			memcpy(m_pVertex+20, pVtx0+24, sizeof(__VertexT1)*5);			

			uint16_t *pIndex = m_pIndex;
			int x,y;
			for (y=0;y<4;y++)
			{
				for(x=0;x<4;x++)
				{
					*(pIndex)   = x   + y*5;
					*(pIndex+1) = x+1 + y*5;
					*(pIndex+2) = x   + (y+1)*5;

					*(pIndex+3) = *(pIndex+2);
					*(pIndex+4) = *(pIndex+1);
					*(pIndex+5) = x+1 + (y+1)*5;
					pIndex+=6;
				}
			}
			m_nVertexCount = nVertexCount;
			m_nIndexCount = nIndexCount;
*/
		}
		break;
	case 2:
		break;
	case 3:
		break;
	}	

	if (m_pParticle)
		delete[] m_pParticle;
	
	m_pParticle = new __Particle[m_nGridW*m_nGridH];
	__Particle *pParticle = m_pParticle;
	int i,j;
	for (i=0;i<m_nGridH;i++)
	{
		for (j=0;j<m_nGridW;j++)
		{
			pParticle->Set(0.5f, 2.0f-j*.2f, 2.0f-i*.2f, 0.0f, 0.0f, 0.0f, 0.0f);
			pParticle++;
		}
	}

	m_nLOD = nLevel;
	//TRACE ("CN3Cloak Set LOD lvl %d\n", nLevel);
}

void CN3Cloak::ApplyOffset(__Vector3& vDif)
{	
/*
	if (m_fOffsetRecoveryTime == 0.0f)
	{
		static int Weight[] = {-3, -2, -1, 1, 2, 3};

		for (int i=0;i<m_nGridW;i++)
		{
			m_pParticle[i].x += vDif.x*Weight[i];
			m_pParticle[i].y += vDif.y*Weight[i];
			m_pParticle[i].z += vDif.z*Weight[i];
			m_vOffset[i].x = vDif.x*Weight[i];
			m_vOffset[i].y = vDif.y*Weight[i];
			m_vOffset[i].z = vDif.z*Weight[i];
		}
		m_fOffsetRecoveryTime = 1.4f;
	}
	else
	{	// offset 이 적용되어 있는 상태.
		m_fOffsetRecoveryTime -= s_fSecPerFrm;
		if (m_fOffsetRecoveryTime < 0.0f)
		{	// Recovery process
			for (int i=0;i<m_nGridW;i++)
			{
				m_pParticle[i].x -= m_vOffset[i].x;
				m_pParticle[i].y -= m_vOffset[i].y;
				m_pParticle[i].z -= m_vOffset[i].z;
				m_vOffset[i].x = 0.0f;
				m_vOffset[i].y = 0.0f;
				m_vOffset[i].z = 0.0f;
			}
			m_fOffsetRecoveryTime = 0.0f;				
		}
	}
*/	
}

void CN3Cloak::TickYaw(float fYaw)
{
	// 회전이 있었다.
	if (fYaw != m_fPrevYaw)
	{
		if (fYaw - m_fPrevYaw > 0.0f)
		{
			if (m_eAnchorPattern == AMP_NONE && m_fAnchorPreserveTime < 0.0f)
				MoveAnchorLine(AMP_YAWCCW, 2.0f);
		}
		else
		{
			if (m_eAnchorPattern == AMP_NONE && m_fAnchorPreserveTime < 0.0f)
				MoveAnchorLine(AMP_YAWCW, 2.0f);				
		}		
	}
	m_fPrevYaw = fYaw;
}

void CN3Cloak::TickByPlayerMotion(e_CloakMove eCurMove)
{
	static float	fTriggerTick = 0.0f;
	static bool		bForceApply = false;
	fTriggerTick += s_fSecPerFrm;

	switch(eCurMove)
	{
	case CLOAK_MOVE_STOP:
		m_GravityForce.y = -0.0015f;
		break;
	case CLOAK_MOVE_WALK:
		m_Force.z = 0.0005f;
		m_GravityForce.y = -0.0025f;
		if (m_eAnchorPattern == AMP_NONE && m_fAnchorPreserveTime < 0.0f)
			MoveAnchorLine(AMP_MOVEXZ, 2.0f);
		//m_GravityForce.y = (rand()%2+1)*-0.0015f;
		//TRACE("Apply force %f\n", m_Force.z);
		break;
	case CLOAK_MOVE_RUN:
		m_Force.z = 0.0009f;
		m_GravityForce.y = -0.0025f;
		if (m_eAnchorPattern == AMP_NONE && m_fAnchorPreserveTime < 0.0f)
			MoveAnchorLine(AMP_MOVEXZ2, 2.0f);
		//m_GravityForce.y = (rand()%2+1)*-0.0015f;
		//TRACE("Apply force %f\n", m_Force.z);
		break;
	case CLOAK_MOVE_WALK_BACKWARD:
		break;
	default:
		break;
	}
}

void CN3Cloak::MoveAnchorLine(e_Cloak_AnchorMovePattern eType, float fPreserveTime)
{
	if (m_eAnchorPattern != AMP_NONE)	return;

	static float Weight[CLOAK_MAX_WIDTH] = {-0.03f, -0.02f, -0.01f, 0.0f, 0.01f, 0.02f, 0.03f};

	switch(eType)
	{
	case AMP_MOVEXZ:
		{
			float x_Offset = (rand()%2-1)*0.01f;
			float z_Offset = 0.03f;

			for(int i=0;i<m_nGridW;i++)
			{
				m_pParticle[i].x += x_Offset;
				m_pParticle[i].z += z_Offset;
				m_vOffset[i].x = x_Offset;
				m_vOffset[i].z = z_Offset;
			}
			//TRACE ("AMP_MOVEXZ Applyed \n");
		}
		break;
	case AMP_MOVEXZ2:
		{
			float x_Offset = (rand()%2-1)*0.02f;
			float z_Offset = 0.04f;

			for(int i=0;i<m_nGridW;i++)
			{
				m_pParticle[i].x += x_Offset;
				m_pParticle[i].z += z_Offset;
				m_vOffset[i].x = x_Offset;
				m_vOffset[i].z = z_Offset;
//				m_pParticle[i].y += x_Offset;
//				m_vOffset[i].y = x_Offset;
			}
			//TRACE ("AMP_MOVEXZ2 Applyed \n");
		}
		break;
	case AMP_YAWCCW:
		{
			for (int i=0;i<m_nGridW;i++)
			{
				m_pParticle[i].z += Weight[i];
				m_vOffset[i].z = Weight[i];
			}
			//TRACE ("AMP_YAWCCW Applyed \n");
		}
		break;
	case AMP_YAWCW:
		{
			for (int i=0;i<m_nGridW;i++)
			{
				m_pParticle[i].z += Weight[i];
				m_vOffset[i].z = Weight[i];
			}
			//TRACE ("AMP_YAWCW Applyed \n");
		}	
		break;
	default:
		break;
	}

	m_fAnchorPreserveTime = fPreserveTime;
	m_eAnchorPattern = eType;
}

void CN3Cloak::RestoreAnchorLine()
{
	__ASSERT(m_eAnchorPattern != AMP_NONE, "call berserk");
	if (m_eAnchorPattern == AMP_NONE)	return;

	__Particle *pParticle = m_pParticle;
	for (int i=0;i<m_nGridW;i++)
	{	
		pParticle->x -= m_vOffset[i].x;
		pParticle->y -= m_vOffset[i].y;
		pParticle->z -= m_vOffset[i].z;
		m_vOffset[i].x = m_vOffset[i].y = m_vOffset[i].z = 0.0f;
		pParticle++;
	}

	m_eAnchorPattern = AMP_NONE;
	m_fAnchorPreserveTime = 1.0f;
	//TRACE ("Anchor Line restored \n");
}
