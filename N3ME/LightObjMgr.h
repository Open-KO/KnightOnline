// LightObjMgr.h: interface for the CLightObjMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__LIGHTOBJMGR_H__)
#define __LIGHTOBJMGR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3BaseFileAccess.h>
#include <N3Base/N3Light.h>
#include <list>
#include "VtxPosDummy.h"

class CDlgLight;
class CMapMng;

typedef struct __LightObj
{
	char		szName[80];
	CN3Light*	pRefLight;

	__LightObj()
	{
		ZeroMemory(szName, 80);		
	}	
} LIGHTOBJ, *LPLIGHTOBJ;

const int IDX_CURR_LIGHT = 3;
const int IDX_SEL_LIGHT = 4;
const int IDX_STANDBY_LIGHT = 5;

class CLightObjMgr : public CN3BaseFileAccess
{
public:
	__Vector3				m_BaseCube[8];
	__VertexXyzColor		m_CubeVB[36];
	CDlgLight*				m_pDlg;

	CMapMng*				m_pRefMapMng;		// 지형 참조 포인터..
	bool					m_bActive;

	std::list<LIGHTOBJ*>	m_ListObj;
	LIGHTOBJ*				m_pCurrLO;

	int						m_iVersion;
	
	CVtxPosDummy			m_VtxPosDummy;
	__VertexXyzT1			m_vCurrLOPos;

protected:
	void MakeCube(__Vector3 cv, D3DCOLOR color);

public:
	/*
	void DelInfo(WARPINFO* pInfo);
	WARPINFO* GetInfoByName(char* pName);
	void AddInfo(WARPINFO* pInfo) { m_ListWarpInfo.push_back(pInfo); }
	void AddWall2Coll(CN3ShapeMgr* pShapeMgr);
	void SetFocus(CWall* pWall);
	void SetCurrWall(CWall* pWall);
	void DelWall(CWall* pWall);
	int	 GetSize() { return m_pWalls.size(); }
	CWall*	GetpWall(int idx);
		
	void UpdateWall();
	*/
	bool MakeGameFile(char* szFN);
	void ChangeSelLights();
	void RefreshCurrLights(float fRange, float fAtten, D3DCOLORVALUE crLgt);
	void DeleteLO(LPLIGHTOBJ pLO);
	void DownLoad();
	void UpLoad(const char* pName, float fRange, float fAtten, D3DCOLORVALUE crLgt);
	
	bool IsActive() { return m_bActive; }
	void SetActive(bool active);
	BOOL MouseMsgFilter(LPMSG pMsg);
	void Render();
	void ClearList();
	
	bool Load(HANDLE hFile);
	bool Save(HANDLE hFile);
		
	CLightObjMgr();
	virtual ~CLightObjMgr();
};

#endif // __LIGHTOBJMGR_H__
