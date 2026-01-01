// ServerMesh.cpp: implementation of the CServerMesh class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "ServerMesh.h"
#include "N3Terrain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const float SGRID_SIZE = 64.0f;
static const int SMAP_SIZE  = 4096;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerMesh::CServerMesh()
{

}

CServerMesh::~CServerMesh()
{

}



void CServerMesh::Tick(CN3Terrain* pTerrain, const __Vector3& vPosPlayer)
{
	__Vector3 pos = vPosPlayer;
	float ixposL, ixposR, izposL, izposR, ixpos, izpos;
	float fHeightLB, fHeightRB, fHeightTop, fHeightBottom;	

	ixpos   = ((int)pos.x)/SGRID_SIZE; 
	ixpos *= SGRID_SIZE;
	izpos   = ((int)pos.z)/SGRID_SIZE;
	izpos *= SGRID_SIZE;

	D3DCOLOR color = D3DCOLOR_RGBA(50,25,25,0);

	// 제일 왼쪽..	
	ixposL = ixpos - SGRID_SIZE;
	izposL = izpos - SGRID_SIZE;
	ixposR = ixpos - SGRID_SIZE;
	izposR = izpos + SGRID_SIZE*2;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 0 );	

	// 왼쪽 두번째..

	ixposL = ixpos;
	izposL = izpos - SGRID_SIZE;
	ixposR = ixpos;
	izposR = izpos + SGRID_SIZE*2;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 6 );	

	// 왼쪽 세번째..

	ixposL = ixpos + SGRID_SIZE;
	izposL = izpos - SGRID_SIZE;
	ixposR = ixpos + SGRID_SIZE;
	izposR = izpos + SGRID_SIZE*2;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 12 );	

	// 왼쪽에서 끝..

	ixposL = ixpos + SGRID_SIZE*2;
	izposL = izpos - SGRID_SIZE;
	ixposR = ixpos + SGRID_SIZE*2;
	izposR = izpos + SGRID_SIZE*2;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 18 );	

	// 밑에서 첫번째..

	ixposL = ixpos - SGRID_SIZE;
	izposL = izpos - SGRID_SIZE;
	ixposR = ixpos + SGRID_SIZE*2;
	izposR = izpos - SGRID_SIZE;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 24 );	

	// 밑에서 두번째..

	ixposL = ixpos - SGRID_SIZE;
	izposL = izpos;
	ixposR = ixpos + SGRID_SIZE*2;
	izposR = izpos;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 30 );	
	
	// 밑에서 세번째..

	ixposL = ixpos - SGRID_SIZE;
	izposL = izpos + SGRID_SIZE;
	ixposR = ixpos + SGRID_SIZE*2;
	izposR = izpos + SGRID_SIZE;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 36 );	

	// 밑에서 끝번째..

	ixposL = ixpos - SGRID_SIZE;
	izposL = izpos + SGRID_SIZE*2;
	ixposR = ixpos + SGRID_SIZE*2;
	izposR = izpos + SGRID_SIZE*2;

	fHeightLB	=	pTerrain->GetHeight(ixposL, izposL);
	fHeightRB	=	pTerrain->GetHeight(ixposR, izposR);

	if ( fHeightLB >= fHeightRB ) 
	{
		fHeightTop	=	fHeightLB;
		fHeightBottom = fHeightRB;
	}
	else
	{
		fHeightTop	=	fHeightRB;
		fHeightBottom = fHeightLB;
	}
	fHeightTop		+= 1.0f;

	AutoConcMesh( color, ixposL, ixposR, izposL, izposR, fHeightBottom, fHeightTop, 42);
}


void CServerMesh::AutoConcMesh(D3DCOLOR color, float left, float right, float bottom, float top, float low, float high , int iStart)
{
	// left, right  : x
	// bottom, top  : z
	// low, high	: y

	switch(iStart)
	{
		case 0:
		case 6:
		case 12:
		case 18:
			m_vSMesh[iStart].Set( left, low, bottom);
			m_vSMesh[iStart+1].Set( left, high, bottom);
			m_vSMesh[iStart+2].Set( left, low, top);
			m_vSMesh[iStart+3].Set( left, high, top);
			m_vSMesh[iStart+4].Set( left, low, top);
			m_vSMesh[iStart+5].Set( left, high, bottom);
			break;

		case 24:
		case 30:
		case 36:
		case 42:
			m_vSMesh[iStart].Set( left, low, top);
			m_vSMesh[iStart+1].Set( left, high, top);
			m_vSMesh[iStart+2].Set( right, low, top);
			m_vSMesh[iStart+3].Set( right, high, top);
			m_vSMesh[iStart+4].Set( right, low, top);
			m_vSMesh[iStart+5].Set( left, high, top);
			break;
	}
}



void CServerMesh::Render()
{
    __Matrix44 mtxWorld;
	mtxWorld.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxWorld.toD3D());
	s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	
	DWORD dwFillPrev;
	s_lpD3DDev->GetRenderState(D3DRS_FILLMODE, &dwFillPrev);

	s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTexture(0, nullptr);

	s_lpD3DDev->SetFVF(D3DFVF_XYZ);
	s_lpD3DDev->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 16, &m_vSMesh, sizeof(__Vector3));

	s_lpD3DDev->SetRenderState(D3DRS_FILLMODE, dwFillPrev);
	s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}
