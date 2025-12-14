// N3Board2DGrass.cpp: implementation of the CGrassBoard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GrassBoard.h"
#include "N3Terrain.h"
#include <N3Base/N3Texture.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrassBoard::CGrassBoard()
{
	m_dwType |= OBJ_BOARD;
	CGrassBoard::Release();
}

CGrassBoard::~CGrassBoard()
{
	this->Release();
}

void CGrassBoard::Release()
{
//	memset(m_vRects, 0, sizeof(m_vRects));

	m_nTexIndex = -1;
	m_ucTexIndex = 0;
	m_ucTexNum = 0;

	m_vCenterPo.Zero();

	m_dwBoardType = BOARD_Y;

	m_fBrightmin = 1.0f;
	m_fBrightmax = 1.0f;
	m_bCamOut = FALSE;

	CN3Transform::Release();
}

void CGrassBoard::ReCalcMatrix()
{
	Grass_Info* pGrass;
	for (int i = 0; i < m_ucTexNum; ++i)
	{
		pGrass = &m_sGrassInfo[i];

		pGrass->mtxWorld.Scale(m_vScale);
		if (m_qRot.w != 0)
			pGrass->mtxWorld *= m_qRot;
		pGrass->mtxWorld.PosSet(m_vPos);
	}
}

void CGrassBoard::Tick(CN3Terrain* pTerrain)
{
	// 회전 시킨다..
	static __Vector3 vBakCam;
	if(vBakCam == s_CameraData.vEye) return;	//	카메라 움직이지 않으면 계산 필요없음 
	vBakCam = s_CameraData.vEye;

	uint32_t dwAlpha;
	__Vector3 vDir;
	Grass_Info* pGrass;
	for(int i=0;i<m_ucTexNum;++i)
	{
		pGrass = &m_sGrassInfo[i];

		if(pGrass->vPos.y==0.0f)	//	높이값 
			pGrass->vPos.y = pTerrain->GetHeight(pGrass->vPos.x,pGrass->vPos.z);

		//	카메라와의 방향백터
		vDir = s_CameraData.vEye - pGrass->vPos;

		//	카메라 방향으로 방향을 튼다
		if(vDir.x > 0.0f) pGrass->mtxWorld.RotationY(-atanf(vDir.z/vDir.x) - (__PI * 0.5f));
		else pGrass->mtxWorld.RotationY(-atanf(vDir.z/vDir.x) + (__PI * 0.5f));
		pGrass->mtxWorld.PosSet(m_sGrassInfo[i].vPos);

		//	카메라와의 거리에 따라 알파먹임
		dwAlpha = SetBrightLevel(vDir.Magnitude());
		if(dwAlpha!=0x00000000) pGrass->dwAlpColor = dwAlpha;
	}
}

void CGrassBoard::Render(CN3Texture** ppTex)
{
	if(m_bCamOut==TRUE) return;	//	카메라 범위 벋어나 찍지 않음

	DWORD dwColorop, dwColorA1, dwColorA2;

	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP,   &dwColorop);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorA1);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG2, &dwColorA2);

	static Grass_Info* pGrass;
	for(int i=0;i<m_ucTexNum;++i)
	{
		pGrass = &m_sGrassInfo[i];
		if(ppTex[pGrass->iTexIndex]==nullptr) continue;

		s_lpD3DDev->SetTransform(D3DTS_WORLD, &pGrass->mtxWorld);

		s_lpD3DDev->SetTexture(0,ppTex[pGrass->iTexIndex]->Get());
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		s_lpD3DDev->SetFVF(FVF_XYZCOLORT1);

		static __VertexXyzColorT1 vRects[4];
		vRects[0].Set((-1.0f/2.0f), 1.0f, 0,  pGrass->dwAlpColor, 0.0f, 0.0f);
		vRects[1].Set( (1.0f/2.0f), 1.0f, 0,  pGrass->dwAlpColor, 1.0f, 0.0f);
		vRects[2].Set( (1.0f/2.0f),  0	, 0,  pGrass->dwAlpColor, 1.0f, 1.0f);
		vRects[3].Set((-1.0f/2.0f),  0	, 0,  pGrass->dwAlpColor, 0.0f, 1.0f);

		s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vRects, sizeof(__VertexXyzColorT1));
	}

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP,   dwColorop);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwColorA1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, dwColorA2);

}

void CGrassBoard::Init(__Vector3 vPos, uint32_t dwBoardType)
{
	m_vPos = vPos;
	m_dwBoardType = dwBoardType;
}

bool CGrassBoard::Load(HANDLE hFile)
{
	return CN3Transform::Load(hFile);
}

#ifdef _N3TOOL
bool CGrassBoard::Save(HANDLE hFile)
{
	return CN3Transform::Save(hFile);
}
#endif // end of #ifdef _N3TOOL

void CGrassBoard::LoadFromFile(int iTexIndex,uint8_t ucTexOrgIndex,__Vector3 vPos)
{
	Release();

//	m_nTexIndex = iTexIndex;
//	m_usTexIndex = ucTexOrgIndex;
//	m_dwBoardType = BOARD_Y;
//	Init(vPos, m_dwBoardType, 0.7f, 0.7f);
//	Init(vPos, m_dwBoardType, 1.0f, 1.0f);
}

uint32_t CGrassBoard::SetBrightLevel(float Level)
{
	static float fLevelbak;	//	카메라와의 거리를 백업하여 같을시 알파계산을 넘김-예전 데이타 간직
	if(fLevelbak == Level) return 0x00000000;	//	카메라와의 거리가 같다면 계산할 필요가 없다
	fLevelbak = Level;

	if(Level<0.0f) return 0x00ffffff;
	if(Level>m_fBrightmin+m_fBrightmax)	return 0x00ffffff;	//	일정거리 이상은 보이지 않음		
	
	uint32_t Color=0x00ffffff;
	if(Level>m_fBrightmin)	//	일정거리내만큼 보임
	{
		float brightper = (Level-m_fBrightmin)/m_fBrightmax;
		uint32_t alphaColor = (uint32_t)(255 - 255*brightper);
		Color = (alphaColor << 24) | 0x00ffffff;
	}
	else Color = 0xffffffff;	//	일정거리안은 완전히 보여줌

	return Color;
}

void CGrassBoard::SetInfo(__Vector3 vBoardPosion,uint16_t usData)
{
	m_vCenterPo.Set(vBoardPosion.x+2.0f , vBoardPosion.y , vBoardPosion.z+2.0f);	//	지도에서의 중간위치기억
	m_ucTexIndex = (uint8_t)((usData & 0xff00)>>8);	//	풀의 인덱스
	m_ucTexNum = (uint8_t)(usData&0x00ff);	//	풀의 갯수

	if(m_ucTexNum>20) m_ucTexNum = 20;

	int GrassInputCount;
	int InputGrass[8];
	FindGrassIndex(m_ucTexIndex,InputGrass,GrassInputCount);

	int iCount=0;
	Grass_Info* pGrass;
	int mRand;
	for(int i=0;i<m_ucTexNum;++i)
	{
		pGrass = &m_sGrassInfo[i];

		mRand = rand();	pGrass->vPos.x = (mRand%40)/10.0f + vBoardPosion.x;

		pGrass->dwAlpColor = 0x00ffffff;
		pGrass->iTexIndex = InputGrass[iCount];
		if(++iCount>=GrassInputCount) iCount=0;

		mRand = rand();	pGrass->vPos.z = (mRand%40)/10.0f+ vBoardPosion.z;	//	랜더값를 좀더 벌리기 위해
		pGrass->vPos.y = 0.0f;
	}
}

void CGrassBoard::FindGrassIndex(const uint8_t uCGrassMngOrder,int* pnInputGrass,int& nGrassTotNum)
{
	nGrassTotNum=0;
	if(uCGrassMngOrder & 0x01)  { pnInputGrass[nGrassTotNum] = 0 ; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x02)  { pnInputGrass[nGrassTotNum] = 1 ; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x04)  { pnInputGrass[nGrassTotNum] = 2 ; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x08)  { pnInputGrass[nGrassTotNum] = 3 ; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x10)  { pnInputGrass[nGrassTotNum] = 4 ; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x20)  { pnInputGrass[nGrassTotNum] = 5 ; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x40)  { pnInputGrass[nGrassTotNum] = 6 ; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x80)  { pnInputGrass[nGrassTotNum] = 7 ; nGrassTotNum++; }
}

