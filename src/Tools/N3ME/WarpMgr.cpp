// WarpMgr.cpp: implementation of the CRegenUser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "n3me.h"
#include "WarpMgr.h"
#include "DlgEditWarp.h"
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

CWarpMgr::CWarpMgr()
{
	m_pRefMapMng = nullptr;				// 지형 참조 포인터..
	m_bActive = false;

	m_ListWarpInfo.clear();
	m_iVersion = 1;

	m_pDlg = new CDlgEditWarp;
	m_pDlg->Create(IDD_EDIT_WARP);
	m_pDlg->ShowWindow(FALSE);
	m_pDlg->m_pRefWarpMgr = this;
}

CWarpMgr::~CWarpMgr()
{
	ClearList();

	if(m_pDlg) 
	{
		m_pDlg->DestroyWindow();
		delete m_pDlg;
		m_pDlg = nullptr;
	}
}

void CWarpMgr::SetActive(bool active)
{
	if(m_bActive==active) return;
	m_bActive = active;

	if(active) m_pDlg->ShowWindow(TRUE);
	else m_pDlg->ShowWindow(FALSE);
}

void CWarpMgr::ClearList()
{
	std::list<WARPINFO*>::iterator it, ite;

	ite = m_ListWarpInfo.end();
	for(it=m_ListWarpInfo.begin(); it!=ite; it++)
	{
		WARPINFO* pWI = (*it);
		if(pWI) delete pWI;
	}
	m_ListWarpInfo.clear();
}

bool CWarpMgr::Load(File& file)
{
	ClearList();

	file.Read(&m_iVersion, sizeof(int));

	if (m_iVersion == 1)
	{
		int cnt;
		file.Read(&cnt, sizeof(int));
		for (int i = 0; i < cnt; i++)
		{
			WARPINFO* pWI = new WARPINFO;
			file.Read(pWI, sizeof(WARPINFO));
			m_ListWarpInfo.push_back(pWI);
		}
	}

	m_pDlg->ResetAll();
	return true;
}

bool CWarpMgr::Save(File& file)
{
	file.Write(&m_iVersion, sizeof(int));

	int cnt = static_cast<int>(m_ListWarpInfo.size());
	file.Write(&cnt, sizeof(int));

	for (WARPINFO* pWI : m_ListWarpInfo)
		file.Write(pWI, sizeof(WARPINFO));
	return true;
}

void CWarpMgr::SaveServerData(File& file)
{
	int cnt = static_cast<int>(m_ListWarpInfo.size());
	file.Write(&cnt, sizeof(int));

	for (WARPINFO* pWI : m_ListWarpInfo)
		file.Write(pWI, sizeof(WARPINFO));
}

WARPINFO* CWarpMgr::GetInfoByName(char* pName)
{
	std::list<WARPINFO*>::iterator it, ite;

	ite = m_ListWarpInfo.end();
	for(it=m_ListWarpInfo.begin(); it!=ite; it++)
	{
		WARPINFO* pWI = (*it);
		if(strcmp(pName, pWI->szName)==0) return pWI;
	}
	return nullptr;
}

void CWarpMgr::DelInfo(WARPINFO* pInfo)
{
	std::list<WARPINFO*>::iterator it, ite;

	ite = m_ListWarpInfo.end();
	for(it=m_ListWarpInfo.begin(); it!=ite; it++)
	{
		WARPINFO* pWI = (*it);
		if(pInfo==pWI) 
		{
			m_ListWarpInfo.erase(it);
			it = ite;
		}
	}
}
