// WallMgr.cpp: implementation of the CWallMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "WallMgr.h"
#include "Wall.h"
#include "MapMng.h"
#include "MainFrm.h"
#include "DlgMakeWall.h"
#include "LyTerrain.h"
#include <N3Base/N3ShapeMgr.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWallMgr::CWallMgr()
{
	m_BaseCube[0].Set(0, 1, 0);	// 앞쪽 LT
	m_BaseCube[1].Set(1, 1, 0);	// 앞쪽 RT
	m_BaseCube[2].Set(0, 0, 0); // 앞쪽 LB
	m_BaseCube[3].Set(1, 0, 0); // 앞쪽 RB
	m_BaseCube[4].Set(0, 1, 1); // 뒤쪽 LT
	m_BaseCube[5].Set(1, 1, 1); // 뒤쪽 RT
	m_BaseCube[6].Set(0, 0, 1); // 뒤쪽 LB
	m_BaseCube[7].Set(1, 0, 1);	// 뒤쪽 RB
	
	m_pRefMapMng = NULL;	// 지형 참조 포인터..
	m_pWalls.clear();		// 벽들...

	m_pDlg = new CDlgMakeWall;
	m_pDlg->Create(IDD_MAKE_WALL);
	m_pDlg->ShowWindow(FALSE);
	m_pDlg->m_pRefWallMgr = this;

	m_bActive = false; // 이기능이 활성화 되어 있는지...1:활성화, 0:비활성화..
	m_pCurrWall = NULL;
}

CWallMgr::~CWallMgr()
{
	std::list<CWall*>::iterator it;
	for(it = m_pWalls.begin(); it != m_pWalls.end(); it++)
	{
		delete (*it);
		(*it) = NULL;
	}
	m_pWalls.clear();

	if(m_pCurrWall)
	{
		delete m_pCurrWall;
		m_pCurrWall = NULL;
	}

	if(m_pDlg) 
	{
		m_pDlg->DestroyWindow();
		delete m_pDlg;
		m_pDlg = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////

void CWallMgr::MakeLine(__Vector3 sv, __Vector3 ev, D3DCOLOR color)
{
	m_LineVB[0].Set(sv.x, sv.y, sv.z, color);
	m_LineVB[1].Set(ev.x, ev.y, ev.z, color);
}

void CWallMgr::MakeBoard(__Vector3 sv, __Vector3 ev, D3DCOLOR color)
{
	m_BoardVB[0].Set(sv.x, sv.y + 100.0f, sv.z, color);
	m_BoardVB[1].Set(ev.x, ev.y + 100.0f, ev.z, color);
	m_BoardVB[2].Set(ev.x, ev.y - 100.0f, ev.z, color);
	m_BoardVB[3].Set(sv.x, sv.y - 100.0f, sv.z, color);
}

void CWallMgr::MakeCube(__Vector3 cv, D3DCOLOR color)
{
	__Vector3 tmp;
	cv.x -= 0.5f;
	cv.y -= 0.5f;
	cv.z -= 0.5f;

	//front lt...
	tmp = cv + m_BaseCube[0];
	m_CubeVB[0].Set(tmp.x, tmp.y, tmp.z, color);			
	tmp = cv + m_BaseCube[1];
	m_CubeVB[1].Set(tmp.x, tmp.y, tmp.z, color);			
	tmp = cv + m_BaseCube[2];
	m_CubeVB[2].Set(tmp.x, tmp.y, tmp.z, color);
	
	//front rb...
	m_CubeVB[3] = m_CubeVB[2];
	m_CubeVB[4] = m_CubeVB[1];
	tmp = cv + m_BaseCube[3];
	m_CubeVB[5].Set(tmp.x, tmp.y, tmp.z, color);

	//right lt..
	m_CubeVB[6] = m_CubeVB[1];
	tmp = cv + m_BaseCube[5];
	m_CubeVB[7].Set(tmp.x, tmp.y, tmp.z, color);
	m_CubeVB[8] = m_CubeVB[5];

	//right rb..
	m_CubeVB[9] = m_CubeVB[8]; 
	m_CubeVB[10] = m_CubeVB[7];
	tmp = cv + m_BaseCube[7];
	m_CubeVB[11].Set(tmp.x, tmp.y, tmp.z, color);

	//back lt..
	m_CubeVB[12] = m_CubeVB[7];
	tmp = cv + m_BaseCube[4];
	m_CubeVB[13].Set(tmp.x, tmp.y, tmp.z, color);
	m_CubeVB[14] = m_CubeVB[11];

	//back rb..
	m_CubeVB[15] = m_CubeVB[14];
	m_CubeVB[16] = m_CubeVB[13];
	tmp = cv + m_BaseCube[6];
	m_CubeVB[17].Set(tmp.x, tmp.y, tmp.z, color);

	//left lt..
	m_CubeVB[18] = m_CubeVB[13];
	m_CubeVB[19] = m_CubeVB[0];
	m_CubeVB[20] = m_CubeVB[17];			

	//left rb..
	m_CubeVB[21] = m_CubeVB[20];
	m_CubeVB[22] = m_CubeVB[19];
	m_CubeVB[23] = m_CubeVB[2];

	//top lt..
	m_CubeVB[24] = m_CubeVB[13];
	m_CubeVB[25] = m_CubeVB[12];
	m_CubeVB[26] = m_CubeVB[0];			

	//top rb..
	m_CubeVB[27] = m_CubeVB[26];
	m_CubeVB[28] = m_CubeVB[25];
	m_CubeVB[29] = m_CubeVB[1];

	//bottom lt..
	m_CubeVB[30] = m_CubeVB[2];
	m_CubeVB[31] = m_CubeVB[5];
	m_CubeVB[32] = m_CubeVB[17];			

	//botom rb..
	m_CubeVB[33] = m_CubeVB[32];
	m_CubeVB[34] = m_CubeVB[31];
	m_CubeVB[35] = m_CubeVB[14];
}

///////////////////////////////////////////////////////////////////////////

void CWallMgr::SetActive(bool active)
{
	if(m_bActive==active) return;
	m_bActive = active;

	if(active)
	{
		m_pDlg->ShowWindow(TRUE);

		CWall* pWall = new CWall;
		if(m_pCurrWall)
		{
			delete m_pCurrWall;
			m_pCurrWall = NULL;
		}
		m_pCurrWall = pWall;
	}
	else
	{
		m_pDlg->ShowWindow(FALSE);

		if(m_pCurrWall)
		{
			delete m_pCurrWall;
			m_pCurrWall = NULL;
		}
	}
}

BOOL CWallMgr::MouseMsgFilter(LPMSG pMsg)
{
	if(!m_pRefMapMng) return FALSE;
	CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();
	if(!m_bActive || !pRefTerrain) return FALSE;

	switch(pMsg->message)
	{
	case WM_LBUTTONUP:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
			if(!pRefTerrain->Pick(point.x, point.y, &vec, NULL)) break;

			m_pCurrWall->AddVertex(vec);			
		}
		return TRUE;

	case WM_RBUTTONUP:
		{
			m_pCurrWall->DelPrevVertex();
		}
		return TRUE;
	}
	return TRUE;
}

void CWallMgr::Render()
{
	// set transform
	HRESULT hr;

	D3DXMATRIX mtx;
	D3DXMatrixIdentity(&mtx);
		
	hr = s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtx); // 월드 행렬 적용..
	
	// set texture
	hr = s_lpD3DDev->SetTexture(0, NULL);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

	// backup state
	DWORD dwZEnable, dwLighting, dwCullMode;
	hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);
	hr = s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCullMode);

	// set state
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);


	hr = s_lpD3DDev->SetFVF(FVF_XYZCOLOR);

	//이미 만들어진 길 그리기...
	std::list<CWall*>::iterator itWall;
	std::list<__Vector3>::iterator itVertex;

	CWall* pWall;
	__Vector3 PrevVertex, Vertex;

	for(itWall = m_pWalls.begin(); itWall != m_pWalls.end(); itWall++)
	{
		pWall = (*itWall);
		if(!pWall) continue;
		
		for(itVertex = pWall->m_Wall.begin(); itVertex != pWall->m_Wall.end(); itVertex++)
		{
			Vertex = (*itVertex);
			Vertex.y += .5f;
									
			if(itVertex==pWall->m_Wall.begin()) PrevVertex = Vertex;
			
			MakeLine(PrevVertex, Vertex, 0xff0000ff);
			MakeCube(Vertex, 0xff0000ff);
			MakeBoard(PrevVertex, Vertex, 0xff0000ff);
						
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_LineVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_CubeVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_BoardVB, sizeof(__VertexXyzColor));
			
			PrevVertex = Vertex;
		}
	}

	//다이얼로그 창에서 선택된 길 그리기..
	CWall* pSelWall = m_pDlg->m_pSelWall;
	if(pSelWall)
	{
		for(itVertex = pSelWall->m_Wall.begin(); itVertex != pSelWall->m_Wall.end(); itVertex++)
		{
			Vertex = (*itVertex);
			Vertex.y += .5f;

			if(itVertex==pSelWall->m_Wall.begin()) PrevVertex = Vertex;

			MakeLine(PrevVertex, Vertex, 0xff00ff00);
			MakeCube(Vertex, 0xff00ff00);
			MakeBoard(PrevVertex, Vertex, 0xff00ff00);
			
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_LineVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_CubeVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_BoardVB, sizeof(__VertexXyzColor));
			
			PrevVertex = Vertex;
		}
	}

	//만들고 있는 길 & 영역 그리기..
	if(m_pCurrWall)
	{
		for(itVertex = m_pCurrWall->m_Wall.begin(); itVertex != m_pCurrWall->m_Wall.end(); itVertex++)
		{
			Vertex = (*itVertex);
			Vertex.y += .5f;

			if(itVertex==m_pCurrWall->m_Wall.begin()) PrevVertex = Vertex;

			MakeLine(PrevVertex, Vertex, 0xffff0000);
			MakeCube(Vertex, 0xffff0000);
			MakeBoard(PrevVertex, Vertex, 0xffff0000);
			
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_LineVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_CubeVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_BoardVB, sizeof(__VertexXyzColor));
			
			PrevVertex = Vertex;
		}
	}

	// restore
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
	hr = s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCullMode);
}

void CWallMgr::SetCurrWall(CWall* pWall)
{
	if(m_pCurrWall)
	{
		delete m_pCurrWall;
		m_pCurrWall = NULL;
	}

	std::list<CWall*>::iterator itWall;
	
	m_pCurrWall = pWall;
	
	for(itWall = m_pWalls.begin(); itWall != m_pWalls.end(); itWall++)
	{
		if(pWall == (*itWall))
		{
			m_pWalls.erase(itWall);
			return;
		}
	}
}

CWall* CWallMgr::GetpWall(int idx)
{
	if (idx < 0
		|| idx >= static_cast<int>(m_pWalls.size()))
		return nullptr;

	std::list<CWall*>::iterator itWall;
	itWall = m_pWalls.begin();
	for(int i=0;i<idx;i++)
	{
		itWall++;
	}

	return (*itWall);
}

void CWallMgr::UpdateWall()
{
	m_pWalls.push_back(m_pCurrWall);

	CWall* pWall = new CWall;
	m_pCurrWall = pWall;	
}

void CWallMgr::DelWall(CWall* pWall)
{
	std::list<CWall*>::iterator itWall;
	
	for(itWall = m_pWalls.begin(); itWall != m_pWalls.end(); itWall++)
	{
		if(pWall == (*itWall))
		{
			delete pWall;
			m_pWalls.erase(itWall);
			return;
		}
	}
}


void CWallMgr::SetFocus(CWall* pWall)
{
	if(!m_pRefMapMng || !pWall) return;

	__Vector3 vec;
	if(pWall->GetVertex(0, &vec))
	{
		m_pRefMapMng->FocusAt(vec);
	}
}

bool CWallMgr::Load(HANDLE hFile)
{
	if(m_pCurrWall)
	{
		delete m_pCurrWall;
		m_pCurrWall = NULL;
	}
	m_pCurrWall = new CWall;

	std::list<CWall*>::iterator itWall;
	for(itWall = m_pWalls.begin(); itWall != m_pWalls.end(); itWall++)
	{
		delete (*itWall);
	}
	
	DWORD dwRWC;
	int NumWall;
	ReadFile(hFile, &NumWall, sizeof(int), &dwRWC, NULL);

	m_pWalls.clear();
	for(int i=0;i<NumWall;i++)
	{
		CWall* pWall = new CWall;
		pWall->Load(hFile);
		m_pWalls.push_back(pWall);
	}

	m_pDlg->ResetAll();

	return true;
}

bool CWallMgr::Save(HANDLE hFile)
{
	DWORD dwRWC;
	int NumWall = m_pWalls.size();
	WriteFile(hFile, &NumWall, sizeof(int), &dwRWC, NULL);

	std::list<CWall*>::iterator itWall;

	CWall* pWall;
	for(itWall = m_pWalls.begin(); itWall != m_pWalls.end(); itWall++)
	{
		pWall = (*itWall);
		pWall->Save(hFile);
	}

	return true;
}

void CWallMgr::AddWall2Coll(CN3ShapeMgr* pShapeMgr)
{
	std::list<CWall*>::iterator itWall;
	std::list<__Vector3>::iterator itVertex;

	CWall* pWall;
	__Vector3 PrevVertex(0,0,0), Vertex(0,0,0);
	__Vector3 v1, v2, v3;

	// Let's embed it as a text file...
	FILE* stream = fopen("c:\\Wall_info.txt", "w");

	fprintf(stream, "Walls = %d\n", m_pWalls.size());
	for(itWall = m_pWalls.begin(); itWall != m_pWalls.end(); itWall++)
	{
		pWall = (*itWall);
		if(!pWall)
		{
			fprintf(stream, "I do not have a wall ... but I try\n");
			continue;
		}

		fprintf(stream, "wall = %d\n", pWall->m_Wall.size());
		for(itVertex = pWall->m_Wall.begin(); itVertex != pWall->m_Wall.end(); itVertex++)
		{
			Vertex = (*itVertex);
												
			if(itVertex==pWall->m_Wall.begin())
			{
				PrevVertex = Vertex;
				continue;
			}

			//v1.Set( PrevVertex.x, PrevVertex.y + 1000.0f, PrevVertex.z);
			//v2.Set( Vertex.x, Vertex.y + 1000.0f, Vertex.z);
			//v3.Set( Vertex.x, Vertex.y - 1000.0f, Vertex.z);

			v1.Set( PrevVertex.x, 5000.0f, PrevVertex.z);
			v2.Set( Vertex.x, 5000.0f, Vertex.z);
			v3.Set( Vertex.x, -5000.0f, Vertex.z);

			if(!pShapeMgr->AddCollisionTriangle(v1, v2, v3)) fprintf(stream, "I did not put in the wall\n");
			if(!pShapeMgr->AddCollisionTriangle(v1, v3, v2)) fprintf(stream, "I did not put in the wall\n");
			
			v1.Set( PrevVertex.x, 5000.0f, PrevVertex.z);
			v2.Set( Vertex.x, -5000.0f, Vertex.z);
			v3.Set( PrevVertex.x, -5000.0f, PrevVertex.z);

			if(!pShapeMgr->AddCollisionTriangle(v1, v2, v3)) fprintf(stream, "I did not put in the wall\n");
			if(!pShapeMgr->AddCollisionTriangle(v1, v3, v2)) fprintf(stream, "I did not put in the wall\n");
						
			PrevVertex = Vertex;
		}
	}
	fclose(stream);
}
