// RegenUser.cpp: implementation of the CRegenUser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "RegenUser.h"
#include "DlgRegenUser.h"
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

CRegenUser::CRegenUser()
{
	m_pRefMapMng = nullptr;				// 지형 참조 포인터..
	m_bActive = false;

	m_vrListRegion.clear();
	m_vrCurrRegion;

	m_vCurrStart.Set(0,0,0);
	m_vCurrEnd = m_vCurrStart;

	m_pDlg = new CDlgRegenUser;
	m_pDlg->Create(IDD_MAKE_REGEN_USER);
	m_pDlg->ShowWindow(FALSE);
	m_pDlg->m_pRefRegenUser = this;

	m_vrSelRegion = nullptr;
}

CRegenUser::~CRegenUser()
{
	ClearList();

	if(m_pDlg) 
	{
		m_pDlg->DestroyWindow();
		delete m_pDlg;
		m_pDlg = nullptr;
	}
}

void CRegenUser::Render()
{
	// set transform
	HRESULT hr;

	D3DXMATRIX mtx;
	D3DXMatrixIdentity(&mtx);
	hr = s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtx); // 월드 행렬 적용..
	
	// set texture
	hr = s_lpD3DDev->SetTexture(0, nullptr);
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

	__VertexXyzColor v[4];

	//이미 만들어진 영역..
	std::list<VERTEXRECT*>::iterator it, ite;
	ite = m_vrListRegion.end();
	for(it=m_vrListRegion.begin(); it!=ite; it++)
	{
		VERTEXRECT* pVR = (*it);
		v[0] = pVR->m_vLB;
		v[0].color = 0xff0000ff;
		v[1] = pVR->m_vLT;
		v[1].color = 0xff0000ff;
		v[2] = pVR->m_vRT;
		v[2].color = 0xff0000ff;
		v[3] = pVR->m_vRB;
		v[3].color = 0xff0000ff;

		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(__VertexXyzColor));
	}

	//지금 만들고 있는 영역..
	v[0] = m_vrCurrRegion.m_vLB;
	v[0].color = 0xffff0000;
	v[1] = m_vrCurrRegion.m_vLT;
	v[1].color = 0xffff0000;
	v[2] = m_vrCurrRegion.m_vRT;
	v[2].color = 0xffff0000;
	v[3] = m_vrCurrRegion.m_vRB;
	v[3].color = 0xffff0000;
	hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(__VertexXyzColor));

	//
	if(m_vrSelRegion)
	{
		v[0] = m_vrSelRegion->m_vLB;
		v[0].color = 0xff00ff00;
		v[1] = m_vrSelRegion->m_vLT;
		v[1].color = 0xff00ff00;
		v[2] = m_vrSelRegion->m_vRT;
		v[2].color = 0xff00ff00;
		v[3] = m_vrSelRegion->m_vRB;
		v[3].color = 0xff00ff00;
		hr = s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(__VertexXyzColor));
	}
		
	// restore
	hr = s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	hr = s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
}

void CRegenUser::SetActive(bool active)
{
	m_vrSelRegion = nullptr;
	if(m_bActive==active) return;
	m_bActive = active;

	if(active) m_pDlg->ShowWindow(TRUE);
	else m_pDlg->ShowWindow(FALSE);
}

BOOL CRegenUser::MouseMsgFilter(LPMSG pMsg)
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
			if(!pRefTerrain->Pick(point.x, point.y, &vec, nullptr)) break;

			m_vCurrEnd = vec;
			SetCurrRECT(m_vCurrStart, m_vCurrEnd);
		}
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;
			if(!pRefTerrain->Pick(point.x, point.y, &vec, nullptr)) break;

			m_vCurrEnd = m_vCurrStart = vec;
			SetCurrRECT(m_vCurrStart, m_vCurrEnd);
		}
		return TRUE;

	case WM_MOUSEMOVE:
		{
			DWORD_PTR nFlags = pMsg->wParam;
			POINT point = {short(LOWORD(pMsg->lParam)), short(HIWORD(pMsg->lParam))};

			__Vector3 vec;

			if(nFlags & MK_LBUTTON)
			{
				if(!pRefTerrain->Pick(point.x, point.y, &vec, nullptr)) break;

				m_vCurrEnd = vec;
				SetCurrRECT(m_vCurrStart, m_vCurrEnd);
			}
		}
		return TRUE;

	case WM_RBUTTONUP:
		{
			if(m_vCurrEnd == m_vCurrStart) break;

			VERTEXRECT* pVR = new VERTEXRECT;
			(*pVR) = m_vrCurrRegion;
			m_vrListRegion.push_back(pVR);
			m_pDlg->PutRegion(pVR);

			m_vCurrEnd.Set(0,0,0);
			m_vCurrStart.Set(0,0,0);
			m_vrCurrRegion.m_vLT.Set(0,0,0);
			m_vrCurrRegion.m_vRB.Set(0,0,0);
			m_vrCurrRegion.m_vRT.Set(0,0,0);
		}
		return TRUE;
	}
	return TRUE;
}

void CRegenUser::SaveServerData(HANDLE hFile)
{
	int NumRegion = static_cast<int>(m_vrListRegion.size());

	DWORD dwNum;
	WriteFile(hFile, &NumRegion, sizeof(int), &dwNum, nullptr);

	std::list<VERTEXRECT*>::iterator it, ite;

	ite = m_vrListRegion.end();
	for(it=m_vrListRegion.begin(); it!=ite; it++)
	{
		VERTEXRECT* pVR = (*it);

		WriteFile(hFile, &(pVR->m_vLB.x), sizeof(float), &dwNum, nullptr);
		WriteFile(hFile, &(pVR->m_vLB.y), sizeof(float), &dwNum, nullptr);
		WriteFile(hFile, &(pVR->m_vLB.z), sizeof(float), &dwNum, nullptr);

		float Height, Width;
		Height = pVR->m_vLT.z - pVR->m_vLB.z;
		Width = pVR->m_vRB.x - pVR->m_vLB.x;

		WriteFile(hFile, &Height, sizeof(float), &dwNum, nullptr);
		WriteFile(hFile, &Width, sizeof(float), &dwNum, nullptr);
	}	
}

void CRegenUser::SetCurrRECT(__Vector3 vS, __Vector3 vE)
{
	if(!m_pRefMapMng) return;

	CLyTerrain* pRefTerrain = m_pRefMapMng->GetTerrain();
	if(!pRefTerrain) return;

	if(vS.x < vE.x)
	{
		m_vrCurrRegion.m_vLB.x = vS.x;
		m_vrCurrRegion.m_vLT.x = vS.x;
		m_vrCurrRegion.m_vRB.x = vE.x;
		m_vrCurrRegion.m_vRT.x = vE.x;
	}
	else 
	{
		m_vrCurrRegion.m_vLB.x = vE.x;
		m_vrCurrRegion.m_vLT.x = vE.x;
		m_vrCurrRegion.m_vRB.x = vS.x;
		m_vrCurrRegion.m_vRT.x = vS.x;
	}

	if(vS.z < vE.z)
	{
		m_vrCurrRegion.m_vLB.z = vS.z;
		m_vrCurrRegion.m_vLT.z = vE.z;
		m_vrCurrRegion.m_vRB.z = vS.z;
		m_vrCurrRegion.m_vRT.z = vE.z;
	}
	else 
	{
		m_vrCurrRegion.m_vLB.z = vE.z;
		m_vrCurrRegion.m_vLT.z = vS.z;
		m_vrCurrRegion.m_vRB.z = vE.z;
		m_vrCurrRegion.m_vRT.z = vS.z;
	}

	m_vrCurrRegion.m_vLB.y = pRefTerrain->GetHeight(m_vrCurrRegion.m_vLB.x, m_vrCurrRegion.m_vLB.z);
	m_vrCurrRegion.m_vLT.y = pRefTerrain->GetHeight(m_vrCurrRegion.m_vLT.x, m_vrCurrRegion.m_vLT.z);
	m_vrCurrRegion.m_vRB.y = pRefTerrain->GetHeight(m_vrCurrRegion.m_vRB.x, m_vrCurrRegion.m_vRB.z);
	m_vrCurrRegion.m_vRT.y = pRefTerrain->GetHeight(m_vrCurrRegion.m_vRT.x, m_vrCurrRegion.m_vRT.z);	
}

void CRegenUser::DeleteSel()
{
	if(!m_vrSelRegion) return;

	std::list<VERTEXRECT*>::iterator it, ite;
	ite = m_vrListRegion.end();
	for(it=m_vrListRegion.begin(); it!=ite; it++)
	{
		VERTEXRECT* pVR = (*it);
		if(pVR==m_vrSelRegion)
		{
			delete pVR;
			m_vrListRegion.erase(it);
			break;
		}
	}
	m_vrSelRegion = nullptr;
}

void CRegenUser::LoadFromFile(LPCTSTR pFullFileName)
{
	FILE* stream = fopen(pFullFileName, "r");
	if(!stream) return;

	char szLine[512] = "", szCommand[80] = "", szBuf[4][80] = { "", "", "", ""};
	char* pResult = fgets(szLine, 512, stream);
	sscanf(szLine, "%s %s %s %s %s", szCommand, szBuf[0], szBuf[1], szBuf[2], szBuf[3]);

	int NumRC;
	if(lstrcmpi(szCommand, "<NUM_REGION>")==0)
	{
		NumRC = atoi(szBuf[0]);
	}
	else return;

	ClearList();

	for(int i=0;i<NumRC;i++)
	{
		VERTEXRECT* pVR = new VERTEXRECT;

		pResult = fgets(szLine, 512, stream);
		if(pResult == nullptr) break;		
		ZeroMemory(szCommand,80);
		ZeroMemory(szBuf[0],80);
		ZeroMemory(szBuf[1],80);
		ZeroMemory(szBuf[2],80);
		ZeroMemory(szBuf[3],80);
		sscanf(szLine, "%s %s %s %s", szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		pVR->m_vLB.x = atof(szBuf[0]);
		pVR->m_vLB.y = atof(szBuf[1]);
		pVR->m_vLB.z = atof(szBuf[2]);

		pResult = fgets(szLine, 512, stream);
		if(pResult == nullptr) break;
		ZeroMemory(szCommand,80);
		ZeroMemory(szBuf[0],80);
		ZeroMemory(szBuf[1],80);
		ZeroMemory(szBuf[2],80);
		ZeroMemory(szBuf[3],80);
		sscanf(szLine, "%s %s %s %s", szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		pVR->m_vLT.x = atof(szBuf[0]);
		pVR->m_vLT.y = atof(szBuf[1]);
		pVR->m_vLT.z = atof(szBuf[2]);

		pResult = fgets(szLine, 512, stream);
		if(pResult == nullptr) break;
		ZeroMemory(szCommand,80);
		ZeroMemory(szBuf[0],80);
		ZeroMemory(szBuf[1],80);
		ZeroMemory(szBuf[2],80);
		ZeroMemory(szBuf[3],80);
		sscanf(szLine, "%s %s %s %s", szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		pVR->m_vRB.x = atof(szBuf[0]);
		pVR->m_vRB.y = atof(szBuf[1]);
		pVR->m_vRB.z = atof(szBuf[2]);

		pResult = fgets(szLine, 512, stream);
		if(pResult == nullptr) break;
		ZeroMemory(szCommand,80);
		ZeroMemory(szBuf[0],80);
		ZeroMemory(szBuf[1],80);
		ZeroMemory(szBuf[2],80);
		ZeroMemory(szBuf[3],80);
		sscanf(szLine, "%s %s %s %s", szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		pVR->m_vRT.x = atof(szBuf[0]);
		pVR->m_vRT.y = atof(szBuf[1]);
		pVR->m_vRT.z = atof(szBuf[2]);
		
		m_vrListRegion.push_back(pVR);
		m_pDlg->PutRegion(pVR);
	}
	fclose(stream);

	m_vCurrEnd.Set(0,0,0);
	m_vCurrStart = m_vCurrEnd;
	SetCurrRECT(m_vCurrStart, m_vCurrEnd);
	m_vrSelRegion = nullptr;
}

void CRegenUser::SaveToFile(LPCTSTR pFullFileName)
{
	FILE* file = fopen(pFullFileName, "w");
	if(!file) return;

	fprintf(file,"<NUM_REGION> %zu\n", m_vrListRegion.size());

	std::list<VERTEXRECT*>::iterator it, ite;
	ite = m_vrListRegion.end();
	for(it=m_vrListRegion.begin(); it!=ite; it++)
	{
		VERTEXRECT* pVR = (*it);
		fprintf(file,"%4.2f %4.2f %4.2f\n", pVR->m_vLB.x, pVR->m_vLB.y, pVR->m_vLB.z);
		fprintf(file,"%4.2f %4.2f %4.2f\n", pVR->m_vLT.x, pVR->m_vLT.y, pVR->m_vLT.z);
		fprintf(file,"%4.2f %4.2f %4.2f\n", pVR->m_vRB.x, pVR->m_vRB.y, pVR->m_vRB.z);
		fprintf(file,"%4.2f %4.2f %4.2f\n", pVR->m_vRT.x, pVR->m_vRT.y, pVR->m_vRT.z);
	}

	fclose(file);
}

void CRegenUser::ClearList()
{
	std::list<VERTEXRECT*>::iterator it, ite;

	ite = m_vrListRegion.end();
	for(it=m_vrListRegion.begin(); it!=ite; it++)
	{
		VERTEXRECT* pVR = (*it);
		if(pVR) delete pVR;
	}
	m_vrListRegion.clear();
//	m_pDlg->ClearList();
}
