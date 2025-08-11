// Grass.cpp: implementation of the CGrassMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameProcMain.h"
#include "N3WorldManager.h"
#include "N3Terrain.h"
#include "GameEng.h"
#include "GrassMng.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

enum {GRASS_LEFT=0,GRASS_TOP,GRASS_RIGHT,GRASS_BOTTOM};

CGrassMng::CGrassMng()
{
	Release();
}

CGrassMng::~CGrassMng()
{
	CLogWriter::Write("CGrassMng::~CGrassMng -> 소멸자"); // TmpLog1122
	Release();
	CLogWriter::Write("CGrassMng::~CGrassMng -> 소멸자 finish"); // TmpLog1122
}


void CGrassMng::Init(__Vector3 CamPo)
{	
	for(int i=0;i<GRASS_TILENUM;++i)
		for(int j=0;j<GRASS_TILENUM;++j)
		{
			m_pGrasses[i][j].Init(CamPo,BOARD_Y);
			m_pGrasses[i][j].SetBrightInit(12,15);
		}
}


void CGrassMng::Tick(CGameProcedure* pProc)
{
	if(m_bChkZoneChange==false) return;

	__Vector3 vCamPos = pProc->s_pEng->s_CameraData.vEye;	//	현재 카메라의 위치

	ChkTileRange(vCamPos.x,vCamPos.z);

	BOOL IsCamOut;
	CGrassBoard* pGrassBoard = &m_pGrasses[0][0];
	for(int i=0;i<GRASS_TILENUM;++i)
		for(int j=0;j<GRASS_TILENUM;++j,++pGrassBoard)
		{
			pGrassBoard->ScaleSet(1.5f, 1.5f, 1.5f);

			//	카메라 밖이면 랜더를 하지 않는다.
			vCamPos = pGrassBoard->GetPo();
			IsCamOut = pProc->s_pEng->s_CameraData.IsOutOfFrustum(vCamPos,3);
			if(IsCamOut==TRUE)
			{
				pGrassBoard->m_bCamOut = TRUE;
				continue;
			}

			pGrassBoard->m_bCamOut = FALSE;
			pGrassBoard->Tick(ACT_WORLD->GetTerrainRef());
		}

/*
	CGrassBoard*   pGrass = NULL; 
	__Vector3	  vPivot;		

	uint8_t GrassIndex=0;
	int mRand,nRand;
	float p,n;

	it_GrassBoard it = m_Grasses.begin();
	int iSize = m_Grasses.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pGrass = *it;

		float XResult = vCamPos.x - m_pCount[i].x;
		float ZResult = vCamPos.z - m_pCount[i].z;

		if(XResult >= 18 || XResult < -18 || ZResult >= 18 || ZResult < -18)
		{
			do{
				mRand = rand();	p = (mRand%180)/10.000;
				nRand = rand();	n = (nRand%180)/10.000;
			}while(p<15 && n<15);

			if(XResult > 0 && ZResult > 0)
			{
				m_pCount[i].x = (float)((m_pCount[i].x + XResult) + p);
				m_pCount[i].z = (float)((m_pCount[i].z + ZResult) + n);
			}

			else if(XResult > 0 && ZResult < 0)
			{
				m_pCount[i].x = (float)((m_pCount[i].x + XResult) + p);
				m_pCount[i].z = (float)((m_pCount[i].z + ZResult) - n);
			}

			else if(XResult < 0 && ZResult > 0)
			{
				m_pCount[i].x = (float)((m_pCount[i].x + XResult) - p);
				m_pCount[i].z = (float)((m_pCount[i].z + ZResult) + n);
			}

			else if(XResult < 0 && ZResult < 0)
			{
				m_pCount[i].x = (float)((m_pCount[i].x + XResult) - p);
				m_pCount[i].z = (float)((m_pCount[i].z + ZResult) - n);
			}

			XResult = 0;
			ZResult = 0;
			
			if(m_pCount[i].y == -FLT_MAX)                                              
				return;
		}

		// 지형정보(풀이 위치한 타일의 지형정보를 얻는다)
		uint8_t cGroundInfo = s_pTerrain->GetGrassAttr(((uint16_t)m_pCount[i].x)/TILE_SIZE, ((uint16_t)m_pCount[i].z)/TILE_SIZE);	//	현재 타일의 정보
		if(cGroundInfo==0)	//	풀을 뿌리는 셀(영역,지역) 아님
		{
			pGrass->m_bCamOut = TRUE;
			continue;
		}

		GrassIndex |= cGroundInfo;

		m_pCount[i].y = s_pTerrain->GetHeight(m_pCount[i].x, m_pCount[i].z);	//	높이 설정

    	pGrass->ScaleSet(1.5f, 1.5f, 1.5f);
		vPivot.Set(m_pCount[i].x,m_pCount[i].y,m_pCount[i].z);

		BOOL IsCamOut;
		//	카메라 밖이면 랜더를 하지 않는다.
		IsCamOut = pProc->s_pEng->s_CameraData.IsOutOfFrustum(vPivot,1);
		if(IsCamOut==TRUE)
		{
			pGrass->m_bCamOut = TRUE;
			continue;
		}

		//	거리를 계산하여 부드럽게 보이게 한다.
		__Vector3 leng = vCamPos - m_pCount[i];
		float lengs = leng.Magnitude();
		IsCamOut = pGrass->SetBrightLevel(lengs);	//	거리간 알파, 랜더하게끔 자동 세팅
		if(IsCamOut == TRUE) 	//	거리가 멀면 랜더를 안한다.
		{
			pGrass->m_bCamOut = TRUE;
			continue;
		}

		pGrass->PosSet(vPivot);		//	위치 입력
		pGrass->Tick(-1);			//	카메라의 위치에 맞게 변환
		if(pGrass->ThisTexIsHave(cGroundInfo) == false)
		{
			int nGrassNum;
			uint8_t uCGrassMngIndex;
			FindGrassIndex(cGroundInfo,nGrassNum,uCGrassMngIndex);
			pGrass->TexSelectNum(nGrassNum,uCGrassMngIndex);
		}
		pGrass->m_bCamOut = FALSE;	//	화면에 뿌려줌(랜더)
	}

	//	만약 다른 풀의 종류에 변동이 생겼을 경우 모두 지우고 다시 풀로 채운다
	if(m_ucDrawIndex!=GrassIndex)
	{
		m_ucDrawIndex = GrassIndex;
		ChkThisZoneUseGrass(GRASS_MAKEUSE);
	}
*/
}

void CGrassMng::FindGrassIndex(uint8_t GrassIndex,int& nFineGrass,uint8_t& ucFineIndex)
{
	int GrassInputCount;
	int InputGrass[8];
	uint8_t InputGrassOrg[8];
	nFineGrass=0;

	FindGrassIndex(GrassIndex,InputGrass,InputGrassOrg,GrassInputCount);

	if(GrassInputCount>0) //	만약 풀이 2개 이상이면 랜덤으로 구한다
	{
		nFineGrass = (int)(rand()%GrassInputCount);
	}

	ucFineIndex = InputGrassOrg[nFineGrass];
	nFineGrass = InputGrass[nFineGrass];
}



void CGrassMng::Render()
{
	if(m_bChkZoneChange==false) return;

	// backup
	DWORD dwLighting;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

	DWORD dwAlphaBlend, dwTextureFactor;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_TEXTUREFACTOR, &dwTextureFactor);
	
	DWORD dwAlphaOP, dwAlphaArg1,dwAlphaArg2;
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg2);

	// render state 세팅
	CN3Base::s_lpD3DDev->SetRenderState( D3DRS_LIGHTING,	TRUE );  
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);

	// texture state 세팅 (alpha)
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	for(int i=0;i<GRASS_TILENUM;++i)
		for(int j=0;j<GRASS_TILENUM;++j)
			m_pGrasses[i][j].Render(m_txTexture);

	// restore
	CN3Base::s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, dwLighting );  
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAlphaOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg2);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_TEXTUREFACTOR, dwTextureFactor);

}



void CGrassMng::Release()
{
	m_bChkZoneChange = false;

	ZeroMemory(m_fChkRange, sizeof(m_fChkRange));

	for (int i = 0; i < m_iFileMaxNum; i++)
		CGameBase::s_MngTex.Delete(&m_txTexture[i]);

	m_iFileMaxNum = 0;
	m_usDrawIndex = 0;
}

int CGrassMng::SetFile(int iTexIndex,uint8_t ucTexOrgIndex,__Vector3 CamPo)
{
/*	if(iTexIndex<0) return 2;

	int iGrassNum = m_Grasses.size();
	if(iGrassNum >= GRASS_MAX) return 1;

	float row = 18.0f, col = 18.0f;

	CGrassBoard* pGrass = new CGrassBoard;
	pGrass->LoadFromFile(iTexIndex,ucTexOrgIndex,CamPo);
	pGrass->SetBrightInit(12,15);
	m_Grasses.push_back(pGrass);
	
	m_pCount[iGrassNum].x  = CamPo.x+RandomGenf(row, -(row));
	m_pCount[iGrassNum].z  = CamPo.z+RandomGenf(col, -(col));
*/
	return 0;
}


inline float CGrassMng::RandomGenf(float max, float min)
{
	if ( max == min ) return max;
	if ( min > max ) { float b = min; min = max; max = b; }
	int k = rand()%(int)((max*100-min*100));	

	return (float)((float)(k*0.01f)+min);
}

void CGrassMng::FindGrassIndex(const uint8_t uCGrassMngOrder,int* pnInputGrass,uint8_t* GrassIndex,int& nGrassTotNum)
{
	nGrassTotNum=0;
	if(uCGrassMngOrder & 0x01)  { pnInputGrass[nGrassTotNum] = 0 , GrassIndex[nGrassTotNum] = 0x01; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x02)  { pnInputGrass[nGrassTotNum] = 1 , GrassIndex[nGrassTotNum] = 0x02; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x04)  { pnInputGrass[nGrassTotNum] = 2 , GrassIndex[nGrassTotNum] = 0x04; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x08)  { pnInputGrass[nGrassTotNum] = 3 , GrassIndex[nGrassTotNum] = 0x08; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x10)  { pnInputGrass[nGrassTotNum] = 4 , GrassIndex[nGrassTotNum] = 0x10; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x20)  { pnInputGrass[nGrassTotNum] = 5 , GrassIndex[nGrassTotNum] = 0x20; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x40)  { pnInputGrass[nGrassTotNum] = 6 , GrassIndex[nGrassTotNum] = 0x40; nGrassTotNum++; }
	if(uCGrassMngOrder & 0x80)  { pnInputGrass[nGrassTotNum] = 7 , GrassIndex[nGrassTotNum] = 0x80; nGrassTotNum++; }
}

void CGrassMng::ChkThisZoneUseGrass(int nGrassUseOrder)
{
	if(m_usDrawIndex==0) return;
/*
	int GrassInputCount;
	int InputGrass[8];
	uint8_t InputGrassOrg[8];

	FindGrassIndex(m_usDrawIndex,InputGrass,InputGrassOrg,GrassInputCount);

	if(nGrassUseOrder==GRASS_MAKENEW)
	{
		int iReturnMsg=0;
		while(!iReturnMsg)
		{
			for(int i=0;i<GrassInputCount && !iReturnMsg;i++)
			{
				iReturnMsg = SetFile(InputGrass[i],InputGrassOrg[i],m_vCamPo);
			}
		}
	}
	else if(nGrassUseOrder==GRASS_MAKEUSE)
	{
		CGrassBoard*   pGrass = NULL; 
		it_GrassBoard it = m_Grasses.begin();
		int iSize = m_Grasses.size();
		int nGrassCount=0,i;
		
		if(iSize==0) return;

//		for(int nGrassCount=0;nGrassCount<iSize;nGrassCount++)
		while(nGrassCount<iSize)
		{
			for(i=0;i<GrassInputCount && nGrassCount<iSize;i++,nGrassCount++,it++)
			{
				pGrass = *it;
				pGrass->TexSelectNum(InputGrass[i],InputGrassOrg[i]);
			}
		}
	}
*/
}

bool CGrassMng::ChangeZone()
{
	Release();

	//	이번 존에서 쓰일 그림의 최대 갯수
	if (!ACT_WORLD->GetTerrainRef()) return false;
	m_iFileMaxNum = ACT_WORLD->GetTerrainRef()->m_iNumGrass;

	if(m_iFileMaxNum==0) 
	{
		m_usDrawIndex=0;
		return false;
	}

	//	그림의 수많큰 이름을 옮기고 나머지 지움
	for(int i=0;i<m_iFileMaxNum;i++)
	{
#ifdef _DEBUG
		if (ACT_WORLD->GetTerrainRef())
			continue;
		strcpy(m_strFileName[i], ACT_WORLD->GetTerrainRef()->m_pGrassTextureName[i]);
#endif
		m_txTexture[i] = CGameBase::s_MngTex.Get(ACT_WORLD->GetTerrainRef()->m_pGrassTextureName[i]);
	}

	for(int i=0;i<8;i++)
	{
#ifdef _DEBUG
		m_strFileName[i][0] = '\0';
#endif
		m_txTexture[i] = NULL;
	}


	//	지금 옮긴 지형에서 쓰일 풀의 인덱스를 가짐
	__Vector3 vCamPos = CGameBase::s_CameraData.vEye;
	this->ChkTileRange(vCamPos.x,vCamPos.z);

	//	Tick,Render를 허락
	m_bChkZoneChange=true;

	return true;
}

inline bool CGrassMng::IsInRect(float fPoX,float fPoY,float* fRange)
{
	return (fPoX>fRange[GRASS_LEFT] && fPoX<fRange[GRASS_RIGHT] 
		 && fPoY>fRange[GRASS_TOP] && fPoY<fRange[GRASS_BOTTOM]);
}

void CGrassMng::ChkTileRange(float fCamX,float fCamZ)
{
	if(IsInRect(fCamX,fCamZ,m_fChkRange)==true) return;	//	고칠게 없어

	//	캐릭터의 바로 한칸의 영역
	float fCompRange[4];
	fCompRange[GRASS_LEFT]   = (fCamX/TILE_SIZE)*TILE_SIZE;
	fCompRange[GRASS_RIGHT]  = fCompRange[GRASS_LEFT]+TILE_SIZE;
	fCompRange[GRASS_TOP]    = (fCamZ/TILE_SIZE)*TILE_SIZE;	
	fCompRange[GRASS_BOTTOM] = fCompRange[GRASS_TOP]+TILE_SIZE;

	//	전체 구역
	float fLargeRange[4];
	fLargeRange[GRASS_LEFT]	  = fCompRange[GRASS_LEFT] - 5*TILE_SIZE;
	fLargeRange[GRASS_RIGHT]  = fCompRange[GRASS_RIGHT] + 5*TILE_SIZE;
	fLargeRange[GRASS_TOP]    = fCompRange[GRASS_TOP] - 5*TILE_SIZE;
	fLargeRange[GRASS_BOTTOM] = fCompRange[GRASS_BOTTOM] + 5*TILE_SIZE;


	//	새로운 타일의 영역을 얻어온다
	uint16_t FineTile[GRASS_TILENUM*2];
	int iFineTile=0;
	FineNewTile(FineTile,iFineTile,fCompRange,fLargeRange);	//	바꾸어야할 타일 찾음


	int iTileCount=0;
	__Vector3 vCenter;
	CGrassBoard* pBoard;
	uint16_t usGrassInfo;
	for(int x=0; x<GRASS_TILENUM; ++x)
	{
		for(int z=0; z<GRASS_TILENUM; ++z)
		{
			pBoard = &m_pGrasses[x][z];

			vCenter = pBoard->GetPo();

			//	영역 벗어난 구역에 찾은구역 넣음
			if(IsInRect( vCenter.x,vCenter.z,fLargeRange )==false)
			{
				if (!ACT_WORLD->GetTerrainRef())
					continue;
				usGrassInfo = ACT_WORLD->GetTerrainRef()->GetGrassAttr(
					(FineTile[iTileCount+0]) / (int)TILE_SIZE,
					(FineTile[iTileCount+1]) / (int)TILE_SIZE);	//	현재 타일의 정보

				vCenter.Set(FineTile[iTileCount],0.0f,FineTile[iTileCount+1]);	//	왼쪽위의 좌표
				vCenter.y = ACT_WORLD->GetTerrainRef()->GetHeight(vCenter.x,vCenter.z);

				pBoard->SetInfo(vCenter,usGrassInfo);	//	타일의 정보를 바탕으로 풀 갱신

				iTileCount+=2;
			}
		}
	}

	memcpy(m_fChkRange,fCompRange,sizeof(float)*4);
}

void CGrassMng::FineNewTile(uint16_t* Tile,int& iCount,float* fCompRange,float* fLargeRange)
{
	float sx=-1,sy=-1;
	if(fCompRange[GRASS_LEFT] != m_fChkRange[GRASS_LEFT])
	{
		sy = fLargeRange[GRASS_TOP];

		if(fCompRange[GRASS_LEFT] > m_fChkRange[GRASS_LEFT]) sx = fLargeRange[GRASS_RIGHT] - TILE_SIZE;
		else												 sx = fLargeRange[GRASS_LEFT];

		for(int i=0;i<GRASS_TILENUM;++i)
		{
			Tile[iCount] = (uint16_t)sx , Tile[iCount+1] = (uint16_t)(sy+=TILE_SIZE);	iCount+=2;
		}
	}

	if(fCompRange[GRASS_TOP] != m_fChkRange[GRASS_TOP])
	{
		if(fCompRange[GRASS_TOP] > m_fChkRange[GRASS_TOP]) sy = fLargeRange[GRASS_TOP];
		else											   sy = fLargeRange[GRASS_BOTTOM] - TILE_SIZE;

		int si,ei;
		if(sx == -1)						   si = 0 , ei = GRASS_TILENUM   ; 
		else if(sx == fLargeRange[GRASS_LEFT]) si = 1 , ei = GRASS_TILENUM   ;
		else								   si = 0 , ei = GRASS_TILENUM-1 ;

		sx = fLargeRange[GRASS_LEFT] + TILE_SIZE*si;
		
		for(int i=si;i<ei;++i)
		{
			Tile[iCount] = (uint16_t)(sx+=TILE_SIZE) , Tile[iCount+1] = (uint16_t)sy;	iCount+=2;
		}
	}
}
