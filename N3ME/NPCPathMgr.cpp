// NPCPathMgr.cpp: implementation of the CNPCPathMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "NPCPathMgr.h"
#include "NPCPath.h"
#include "DlgMakeNPCPath.h"
#include "LyTerrainDef.h"
#include "LyTerrain.h"
#include "MapMng.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNPCPathMgr::CNPCPathMgr()
{
	m_pPaths.clear();
	m_bActive = false;
	m_pRefMapMng = NULL;

	m_pDlgMakePath = new CDlgMakeNPCPath;
	m_pDlgMakePath->Create(IDD_MAKE_NPCPATH);
	m_pDlgMakePath->ShowWindow(FALSE);
	m_pDlgMakePath->m_pRefNPCPathMgr = this;

	m_pCurrPath = NULL;
	m_pppRefEvent = NULL;
	m_bRenderMovableRegion = false;

	m_BaseCube[0].Set(0, 1, 0);	// 앞쪽 LT
	m_BaseCube[1].Set(1, 1, 0);	// 앞쪽 RT
	m_BaseCube[2].Set(0, 0, 0); // 앞쪽 LB
	m_BaseCube[3].Set(1, 0, 0); // 앞쪽 RB
	m_BaseCube[4].Set(0, 1, 1); // 뒤쪽 LT
	m_BaseCube[5].Set(1, 1, 1); // 뒤쪽 RT
	m_BaseCube[6].Set(0, 0, 1); // 뒤쪽 LB
	m_BaseCube[7].Set(1, 0, 1);	// 뒤쪽 RB 

	m_LTStartVertex.Set(0,0,0);
	m_RBStartVertex.Set(0,0,0);

	m_LTActVertex.Set(0,0,0);
	m_RBActVertex.Set(0,0,0);
}


CNPCPathMgr::CNPCPathMgr(CMapMng* pMapMng)
{
	m_pPaths.clear();
	m_bActive = false;
	m_pRefMapMng = pMapMng;

	m_pDlgMakePath = new CDlgMakeNPCPath;
	m_pDlgMakePath->Create(IDD_MAKE_NPCPATH, m_pRefMapMng->m_pMainFrm);
	m_pDlgMakePath->ShowWindow(FALSE);
	m_pDlgMakePath->m_pRefNPCPathMgr = this;

	m_pCurrPath = NULL;
	m_pppRefEvent = NULL;
	m_bRenderMovableRegion = false;

	m_BaseCube[0].Set(0, 1, 0);	// 앞쪽 LT
	m_BaseCube[1].Set(1, 1, 0);	// 앞쪽 RT
	m_BaseCube[2].Set(0, 0, 0); // 앞쪽 LB
	m_BaseCube[3].Set(1, 0, 0); // 앞쪽 RB
	m_BaseCube[4].Set(0, 1, 1); // 뒤쪽 LT
	m_BaseCube[5].Set(1, 1, 1); // 뒤쪽 RT
	m_BaseCube[6].Set(0, 0, 1); // 뒤쪽 LB
	m_BaseCube[7].Set(1, 0, 1);	// 뒤쪽 RB 

	m_LTStartVertex.Set(0,0,0);
	m_RBStartVertex.Set(0,0,0);

	m_LTActVertex.Set(0,0,0);
	m_RBActVertex.Set(0,0,0);
}


CNPCPathMgr::~CNPCPathMgr()
{
	std::list<CNPCPath*>::iterator it;
	for(it = m_pPaths.begin(); it != m_pPaths.end(); it++)
	{
		delete (*it);
		(*it) = NULL;
	}
	m_pPaths.clear();

	if(m_pCurrPath)
	{
		delete m_pCurrPath;
		m_pCurrPath = NULL;
	}

	if(m_pDlgMakePath) 
	{
		m_pDlgMakePath->DestroyWindow();
		delete m_pDlgMakePath;
		m_pDlgMakePath = NULL;
	}

	m_pppRefEvent = NULL;
	m_bRenderMovableRegion = false;
}


//
//	FileName은 경로명 하나도 안들어간 순수한 파일이름과 확장자..
//
void CNPCPathMgr::LoadFromFile(const char* FileName)
{
	if(m_pCurrPath)
	{
		delete m_pCurrPath;
		m_pCurrPath = NULL;
	}
	m_pCurrPath = new CNPCPath;

	std::list<CNPCPath*>::iterator itPath;
	for(itPath = m_pPaths.begin(); itPath != m_pPaths.end(); itPath++)
	{
		delete (*itPath);
	}
	
	char szNPCPathFileName[_MAX_PATH];
	wsprintf(szNPCPathFileName, "%snpcpath\\%s.npi", s_szPath.c_str(), FileName);
	
	DWORD dwRWC;
	HANDLE hFile = CreateFile(szNPCPathFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	int NumPath;
	ReadFile(hFile, &NumPath, sizeof(int), &dwRWC, NULL);

	m_pPaths.clear();
	for(int i=0;i<NumPath;i++)
	{
		CNPCPath* pPath = new CNPCPath;
		pPath->m_pRefTerrain = m_pRefMapMng->GetTerrain();
		pPath->Load(hFile);
		m_pPaths.push_back(pPath);
	}
	CloseHandle(hFile);
}

void CNPCPathMgr::SaveToFile(const char* FileName)
{
	char szOldPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szOldPath);	
	SetCurrentDirectory(s_szPath.c_str());

	CreateDirectory("npcpath", NULL); // 경로 만들고..
	char szNPCPathFileName[_MAX_PATH];
	wsprintf(szNPCPathFileName, "%snpcpath\\%s.npi", s_szPath.c_str(), FileName);

	DWORD dwRWC;
	HANDLE hFile = CreateFile(szNPCPathFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	int NumPath = m_pPaths.size();
	WriteFile(hFile, &NumPath, sizeof(int), &dwRWC, NULL);

	std::list<CNPCPath*>::iterator itPath;
	CNPCPath* pPath;
	for(itPath = m_pPaths.begin(); itPath != m_pPaths.end(); itPath++)
	{
		pPath = (*itPath);
		pPath->m_iZoneID = m_pRefMapMng->m_iZoneID;
		pPath->Save(hFile);
	}
	CloseHandle(hFile);
	SetCurrentDirectory(szOldPath);
}

void CNPCPathMgr::MakeServerDataFile(const char* FullFileName)
{
	// text 파일 버전...
	FILE* stream = fopen(FullFileName, "w");
	if(!stream)	return;

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
	if(!pFrm) return;

	std::list<CNPCPath*>::iterator itPath;
	std::list<__Vector3>::iterator itVertex;
	CNPCPath* pPath;
	fprintf(stream, "ZoneID");
	fprintf(stream, "\tNpcID");
	fprintf(stream, "\tActType");
	fprintf(stream, "\tAttrRegen");
	fprintf(stream, "\tAttrGroup");
	fprintf(stream, "\tAttrCreate");
	fprintf(stream, "\tLeftX");
	fprintf(stream, "\tTopZ");
	fprintf(stream, "\tRightX");
	fprintf(stream, "\tBottomZ");
	fprintf(stream, "\tLimitLeftX");
	fprintf(stream, "\tLimitTopZ");
	fprintf(stream, "\tLimitRightX");
	fprintf(stream, "\tLimitBottomZ");
	fprintf(stream, "\tNumNPC");
	fprintf(stream, "\tRegTime");
	fprintf(stream, "\tDotCnt");
	fprintf(stream, "\n");

	for(itPath = m_pPaths.begin(); itPath != m_pPaths.end(); itPath++)
	{
		pPath = (*itPath);
		if(!pPath->CheckValid()) continue;

		RECT rt;
		SetRect(&rt, (int)pPath->m_LTStartVertex.x, (int)pPath->m_LTStartVertex.z,
					(int)pPath->m_RBStartVertex.x, (int)pPath->m_RBStartVertex.z );

		RECT LimitRC;
		SetRect(&LimitRC, (int)pPath->m_LTActVertex.x, (int)pPath->m_LTActVertex.z,
					(int)pPath->m_RBActVertex.x, (int)pPath->m_RBActVertex.z );

		int DotCount = pPath->m_Path.size();
		pPath->m_iZoneID = m_pRefMapMng->m_iZoneID;

		fprintf(stream, "%04d", pPath->m_iZoneID);
		fprintf(stream, "\t%03d", pPath->m_iNPCID);
		fprintf(stream, "\t%02d", pPath->m_iActType);
		fprintf(stream, "\t%02d", (int)pPath->m_cAttr_Regen);
		fprintf(stream, "\t%02d", (int)pPath->m_cAttr_Group);
		fprintf(stream, "\t%02d", (int)pPath->m_cAttr_Create);
		fprintf(stream, "\t%02d", (int)pPath->m_cAttr_Option);
		fprintf(stream, "\t%04d", rt.left);
		fprintf(stream, "\t%04d", rt.top);
		fprintf(stream, "\t%04d", rt.right);
		fprintf(stream, "\t%04d", rt.bottom);
		fprintf(stream, "\t%04d", LimitRC.left);
		fprintf(stream, "\t%04d", LimitRC.top);
		fprintf(stream, "\t%04d", LimitRC.right);
		fprintf(stream, "\t%04d", LimitRC.bottom);
		fprintf(stream, "\t%03d", pPath->m_iNumNPC);
		fprintf(stream, "\t%04d", pPath->m_iRegenTime);
		fprintf(stream, "\t%02d\t", DotCount);

		__Vector3 Vertex;
		int x,z;
		for(itVertex = pPath->m_Path.begin(); itVertex != pPath->m_Path.end(); itVertex++)
		{
			Vertex = (*itVertex);

			x = (int)Vertex.x;
			z = (int)Vertex.z;
			fprintf(stream, "%04d%04d", x, z);
		}
		fprintf(stream, "\n");
	}
	fclose(stream);	
}

void CNPCPathMgr::SetActive(bool active)
{
	if(m_bActive==active) return;
	m_bActive = active;

	if(active)
	{
		m_pDlgMakePath->ShowWindow(TRUE);

		CNPCPath* pPath = new CNPCPath;
		if(m_pRefMapMng) pPath->m_pRefTerrain = m_pRefMapMng->GetTerrain();
		if(m_pCurrPath)
		{
			delete m_pCurrPath;
			m_pCurrPath = NULL;
		}
		m_pCurrPath = pPath;
	}
	else
	{
		m_pDlgMakePath->ShowWindow(FALSE);

		if(m_pCurrPath)
		{
			delete m_pCurrPath;
			m_pCurrPath = NULL;
		}
	}
}

BOOL CNPCPathMgr::MouseMsgFilter(LPMSG pMsg)
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

			if(m_pDlgMakePath->m_State==0)	// 시작 RECT 그림.
			{
				m_pCurrPath->m_LTStartVertex = m_LTStartVertex;
				m_pCurrPath->m_RBStartVertex = m_RBStartVertex;
				SetLTRB(&(m_pCurrPath->m_LTStartVertex), &(m_pCurrPath->m_RBStartVertex));
				//MakeStartRectVB(m_LTStartVertex, m_RBStartVertex, 0xff00ffff);
			}

			if(m_pDlgMakePath->m_State==1)	// 길 만듦
			{
				__Vector3 PrevPos;
				if(false==m_pCurrPath->GetPath(m_pCurrPath->GetSize()-1, &PrevPos))	// 
				{
					if(m_pCurrPath->GetSize() < MAX_NPCPATH) m_pCurrPath->AddPos(vec);
					m_pDlgMakePath->SetNumPoint(m_pCurrPath->GetSize());
				}
				else
				{
					__Vector3 tmpVec = vec - PrevPos;
					tmpVec.Normalize();
					tmpVec *= MAX_LEN_PATH;

					float dist = DISTANCE2D(PrevPos,vec);
					while(dist > MAX_LEN_PATH)
					{
						PrevPos += tmpVec;
						if(m_pCurrPath->GetSize() < MAX_NPCPATH) m_pCurrPath->AddPos(PrevPos);
						m_pDlgMakePath->SetNumPoint(m_pCurrPath->GetSize());

						dist = DISTANCE2D(PrevPos,vec);
					}
					if(m_pCurrPath->GetSize() < MAX_NPCPATH) m_pCurrPath->AddPos(vec);
					m_pDlgMakePath->SetNumPoint(m_pCurrPath->GetSize());
				}
				
			}

			if(m_pDlgMakePath->m_State==2)	// 시작 RECT 그림.
			{
				m_pCurrPath->m_LTActVertex = m_LTActVertex;
				m_pCurrPath->m_RBActVertex = m_RBActVertex;
				SetLTRB(&(m_pCurrPath->m_LTActVertex), &(m_pCurrPath->m_RBActVertex));
				//MakeStartRectVB(m_LTStartVertex, m_RBStartVertex, 0xff00ffff);
			}			
		}
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
			if(!pRefTerrain->Pick(point.x, point.y, &vec, NULL)) break;

			if(m_pDlgMakePath->m_State==0)
			{
				m_LTStartVertex = m_RBStartVertex = vec;
			}
			if(m_pDlgMakePath->m_State==2)
			{
				m_LTActVertex = m_RBActVertex = vec;
			}
		}
		return TRUE;

	case WM_MOUSEMOVE:
		{
			DWORD nFlags = pMsg->wParam;
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
//			if(nFlags & MK_SHIFT)
//			{
//				if(!pRefTerrain->Pick(point.x, point.y, &vec, NULL)) break;
//				
//				CStatusBar* pBar = m_pRefMapMng->GetStatusBar();
//				CString str; str.Format("X:%f Y:%f Z:%f", vec.x, vec.y, vec.z);
//				if (pBar) pBar->SetPaneText(0, str);
//				return TRUE;
//			}

			if(m_pDlgMakePath->m_State==0)
			{
				if(nFlags & MK_LBUTTON)
				{
					if(!pRefTerrain->Pick(point.x, point.y, &vec, NULL)) break;

					m_RBStartVertex = vec;

					__Vector3 lt, rb;
					m_pCurrPath->m_LTStartVertex = m_LTStartVertex;
					m_pCurrPath->m_RBStartVertex = m_RBStartVertex;
					SetLTRB(&(m_pCurrPath->m_LTStartVertex), &(m_pCurrPath->m_RBStartVertex));
					//MakeStartRectVB(lt, rb, 0xff00ffff);
				}
			}
			if(m_pDlgMakePath->m_State==2)
			{
				if(nFlags & MK_LBUTTON)
				{
					if(!pRefTerrain->Pick(point.x, point.y, &vec, NULL)) break;

					m_RBActVertex = vec;

					__Vector3 lt, rb;
					m_pCurrPath->m_LTActVertex = m_LTActVertex;
					m_pCurrPath->m_RBActVertex = m_RBActVertex;
					SetLTRB(&(m_pCurrPath->m_LTActVertex), &(m_pCurrPath->m_RBActVertex));
					//MakeStartRectVB(lt, rb, 0xff00ffff);
				}
			}
		}
		return TRUE;

	case WM_RBUTTONUP:
		if(m_pDlgMakePath->m_State==1)
		{
			m_pCurrPath->DelPrevPos();
			m_pDlgMakePath->SetNumPoint(m_pCurrPath->GetSize());
		}
		return TRUE;
	}
	return TRUE;
}

void CNPCPathMgr::UpdatePath()
{
	m_pCurrPath->m_iNPCID = m_pDlgMakePath->m_iSelNPCID;
	m_pCurrPath->m_iNumNPC = m_pDlgMakePath->m_NumNPC;
	m_pCurrPath->m_iRegenTime = m_pDlgMakePath->m_iRegenTime;
	m_pCurrPath->m_iActType = m_pDlgMakePath->m_iSelActType;
	m_pCurrPath->m_iZoneID = m_pRefMapMng->m_iZoneID;
	m_pCurrPath->m_cAttr_Create = m_pDlgMakePath->m_cSelAttrCreate;
	m_pCurrPath->m_cAttr_Group = m_pDlgMakePath->m_cSelAttrGroup;
	m_pCurrPath->m_cAttr_Regen = m_pDlgMakePath->m_cSelAttrRegen;
	m_pCurrPath->m_cAttr_Option = m_pDlgMakePath->m_cSelOption;

	strcpy(m_pCurrPath->m_strNPCName, m_pDlgMakePath->m_strSelNPCName);

//	m_pCurrPath->m_LTStartVertex = m_LTStartVertex;
//	m_pCurrPath->m_RBStartVertex = m_RBStartVertex;
	
	m_pPaths.push_back(m_pCurrPath);

	CNPCPath* pPath = new CNPCPath;
	if(m_pRefMapMng) pPath->m_pRefTerrain = m_pRefMapMng->GetTerrain();
	m_pCurrPath = pPath;
	m_pDlgMakePath->SetNumPoint(m_pCurrPath->GetSize());
}

CNPCPath* CNPCPathMgr::GetpPath(int idx)
{
	if (idx < 0
		|| idx >= static_cast<int>(m_pPaths.size()))
		return nullptr;

	std::list<CNPCPath*>::iterator itPath;
	itPath = m_pPaths.begin();
	for(int i=0;i<idx;i++)
	{
		itPath++;
	}
	
	return (*itPath);
}

void CNPCPathMgr::Render()
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
	DWORD dwZEnable, dwLighting;
	hr = s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	hr = s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

	// set state
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	hr = s_lpD3DDev->SetFVF(FVF_XYZCOLOR);

	//갈 수 없는 곳 빨간색으로 칠하기..
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	if(m_bRenderMovableRegion)RenderMovableRegion();
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	//이미 만들어진 길 그리기...
	std::list<CNPCPath*>::iterator itPath;
	std::list<__Vector3>::iterator itVertex;
	CNPCPath* pPath;
	__Vector3 PrevVertex, Vertex;
	
	for(itPath = m_pPaths.begin(); itPath != m_pPaths.end(); itPath++)
	{
		pPath = (*itPath);
		__Vector3 RgnLT, RgnRB;
		RgnLT = (*itPath)->m_LTStartVertex;
		RgnRB = (*itPath)->m_RBStartVertex;
		
		MakeRectVB(m_StartRectVB, RgnLT, RgnRB, pPath->m_dwColor);
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, m_StartRectVB, sizeof(__VertexXyzColor));

		RgnLT = (*itPath)->m_LTActVertex;
		RgnRB = (*itPath)->m_RBActVertex;
		MakeRectVB(m_ActRectVB, RgnLT, RgnRB, pPath->m_dwColor);
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, m_ActRectVB, sizeof(__VertexXyzColor));

		for(itVertex = pPath->m_Path.begin(); itVertex != pPath->m_Path.end(); itVertex++)
		{
			Vertex = (*itVertex);
			Vertex.y += 0.5f;
			__Vector3 rgnLT, rgnRB;
						
			if(itVertex==pPath->m_Path.begin()) PrevVertex = Vertex;
			
			MakeLine(PrevVertex, Vertex);
			MakeCube(Vertex, pPath->m_dwColor);
						
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_LineVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_CubeVB, sizeof(__VertexXyzColor));
			
			PrevVertex = Vertex;
		}
	}

	//다이얼로그 창에서 선택된 길 그리기..
	CNPCPath* pSelPath = m_pDlgMakePath->m_pSelPath;
	if(pSelPath)
	{
		MakeRectVB(m_StartRectVB, pSelPath->m_LTStartVertex, pSelPath->m_RBStartVertex, 0xff00ff00);
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, m_StartRectVB, sizeof(__VertexXyzColor));

		MakeRectVB(m_ActRectVB, pSelPath->m_LTActVertex, pSelPath->m_RBActVertex, 0xff00ff00);
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, m_ActRectVB, sizeof(__VertexXyzColor));
		
		for(itVertex = pSelPath->m_Path.begin(); itVertex != pSelPath->m_Path.end(); itVertex++)
		{
			Vertex = (*itVertex);
			Vertex.y += 0.5f;

			if(itVertex==pSelPath->m_Path.begin()) PrevVertex = Vertex;

			MakeLine(PrevVertex, Vertex);
			MakeCube(Vertex, 0xff00ff00);
			
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_LineVB, sizeof(__VertexXyzColor));
			hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_CubeVB, sizeof(__VertexXyzColor));		
			
			PrevVertex = Vertex;
		}
	}

	//만들고 있는 길 & 영역 그리기..
	MakeRectVB(m_StartRectVB, m_pCurrPath->m_LTStartVertex, m_pCurrPath->m_RBStartVertex, 0xffff0000);
	hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, m_StartRectVB, sizeof(__VertexXyzColor));

	MakeRectVB(m_ActRectVB, m_pCurrPath->m_LTActVertex, m_pCurrPath->m_RBActVertex, 0xffff0000);
	hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, m_ActRectVB, sizeof(__VertexXyzColor));

	for(itVertex = m_pCurrPath->m_Path.begin(); itVertex != m_pCurrPath->m_Path.end(); itVertex++)
	{
		Vertex = (*itVertex);
		Vertex.y += 0.5f;

		if(itVertex==m_pCurrPath->m_Path.begin()) PrevVertex = Vertex;

		MakeLine(PrevVertex, Vertex);
		MakeCube(Vertex, 0xffff0000);
		
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, m_LineVB, sizeof(__VertexXyzColor));
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, m_CubeVB, sizeof(__VertexXyzColor));		
		
		PrevVertex = Vertex;
	}

	// restore
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
}

void CNPCPathMgr::MakeCube(__Vector3 cv, D3DCOLOR color)
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

void CNPCPathMgr::MakeLine(__Vector3 sv, __Vector3 ev)
{
	m_LineVB[0].Set(sv.x, sv.y, sv.z, 0xffffffff);
	m_LineVB[1].Set(ev.x, ev.y, ev.z, 0xffffffff);
}

void CNPCPathMgr::SetLTRB(__Vector3* pLT, __Vector3* pRB)
{
	__Vector3 tmp, vLT, vRB;
	vLT = tmp = (*pLT);
	vRB = (*pRB);
	
	if(vLT.x > vRB.x)
	{
		vLT.x = vRB.x;
		vRB.x = tmp.x;
	}
	if(vLT.y < vRB.y)
	{
		vLT.y = vRB.y;
		vRB.y = tmp.y;
	}
	if(vLT.z < vRB.z)
	{
		vLT.z = vRB.z;
		vRB.z = tmp.z;
	}

	(*pLT) = vLT;
	(*pRB) = vRB;
}

void CNPCPathMgr::MakeRectVB(__VertexXyzColor* pVB, __Vector3 lt, __Vector3 rb, DWORD color)
{
	// 상판...
	pVB[0].Set(lt.x, lt.y, lt.z, color);
	pVB[1].Set(lt.x, lt.y, rb.z, color);

	pVB[2] = pVB[1];
	pVB[3].Set(rb.x, lt.y, rb.z, color);

	pVB[4] = pVB[3];
	pVB[5].Set(rb.x, lt.y, lt.z, color);

	pVB[6] = pVB[5];
	pVB[7] = pVB[0];

	// 다리..
	pVB[8] = pVB[0];
	pVB[9].Set(lt.x, rb.y, lt.z, color);

	pVB[10] = pVB[1];
	pVB[11].Set(lt.x, rb.y, rb.z, color);

	pVB[12] = pVB[3];
	pVB[13].Set(rb.x, rb.y, rb.z, color);

	pVB[14] = pVB[5];
	pVB[15].Set(rb.x, rb.y, lt.z, color);

	// 바닥...
	pVB[16] = pVB[9];
	pVB[17] = pVB[11];

	pVB[18] = pVB[17];
	pVB[19] = pVB[13];

	pVB[20] = pVB[19];
	pVB[21] = pVB[15];

	pVB[22] = pVB[21];
	pVB[23] = pVB[9];
}

void CNPCPathMgr::DelPath(CNPCPath* pPath)
{
	std::list<CNPCPath*>::iterator itPath;
	
	for(itPath = m_pPaths.begin(); itPath != m_pPaths.end(); itPath++)
	{
		if(pPath == (*itPath))
		{
			delete pPath;
			m_pPaths.erase(itPath);
			return;
		}
	}
}

void CNPCPathMgr::SetCurrPath(CNPCPath* pPath)
{
	if(m_pCurrPath)
	{
		delete m_pCurrPath;
		m_pCurrPath = NULL;
	}

	std::list<CNPCPath*>::iterator itPath;

	m_pCurrPath = pPath;
	m_pDlgMakePath->SetNumPoint(m_pCurrPath->GetSize());
	
	for(itPath = m_pPaths.begin(); itPath != m_pPaths.end(); itPath++)
	{
		if(pPath == (*itPath))
		{
			m_pPaths.erase(itPath);
			return;
		}
	}
}

void CNPCPathMgr::RenderMovableRegion()
{
	if(!m_pRefMapMng) return;
	CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();
	
	int HeightMapSize = pRefTerrain->m_iHeightMapSize;

	__VertexXyzColor	TileVB[4];
	D3DCOLOR color = 0xffff0000;

	POINT cam;
	cam.x = s_CameraData.vEye.x / TERRAIN_CELL_SIZE;
	cam.y = s_CameraData.vEye.z / TERRAIN_CELL_SIZE;

	RECT rt;
	if( (cam.x-100) >= 0) rt.left = cam.x-100;
	else rt.left = 0;

	if( (cam.x+100) < (HeightMapSize-1) ) rt.right = cam.x+100;
	else rt.right = HeightMapSize-1;

	if( (cam.y-100) >= 0 ) rt.bottom = cam.y-100;
	else rt.bottom = 0;

	if( (cam.y+100) < (HeightMapSize-1) ) rt.top = cam.y+100;
	else rt.top = HeightMapSize-1;

	for(int x=rt.left; x<rt.right; x++)
	{
		for(int z=rt.bottom; z<rt.top; z++)
		{
			if(m_pppRefEvent[x][z]!=0) continue;

			if((x+z)%2==0)	// 슬래쉬 모양의 타일..
			{
				__Vector3 v;
				v.x = x*TERRAIN_CELL_SIZE;
				v.z = z*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[0].Set(v.x, v.y, v.z, color);

				v.z = (z+1)*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[1].Set(v.x, v.y, v.z, color);

				v.x = (x+1)*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[2].Set(v.x, v.y, v.z, color);

				v.z = z*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[3].Set(v.x, v.y, v.z, color);				
			}
			if((x+z)%2==1)	//백슬레쉬 모양의 타일..
			{
				__Vector3 v;
				v.x = x*TERRAIN_CELL_SIZE;
				v.z = (z+1)*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[0].Set(v.x, v.y, v.z, color);

				v.x = (x+1)*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[1].Set(v.x, v.y, v.z, color);

				v.z = z*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[2].Set(v.x, v.y, v.z, color);

				v.x = x*TERRAIN_CELL_SIZE;
				v.y = pRefTerrain->GetHeight(v.x, v.z) + 0.3f;
				TileVB[3].Set(v.x, v.y, v.z, color);
			}

			s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, TileVB, sizeof(__VertexXyzColor));
		}
	}	
}

void CNPCPathMgr::TransPos(float x, float z)
{
	std::list<CNPCPath*>::iterator itPath;
	CNPCPath* pPath;
	for(itPath = m_pPaths.begin(); itPath != m_pPaths.end(); itPath++)
	{
		pPath = (*itPath);
		pPath->TransPos(x, z);
	}
}
