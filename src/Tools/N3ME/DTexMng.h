// DTexMng.h: interface for the CDTexMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DTEXMNG_H__30DCE41A_64F9_4876_AA67_5E344993325D__INCLUDED_)
#define AFX_DTEXMNG_H__30DCE41A_64F9_4876_AA67_5E344993325D__INCLUDED_

#pragma once

#include "DTexGroup.h"
#include <list>

class CDTex;
class CMainFrame;

typedef std::list<CDTex*>::iterator it_DTex;

class CDTexMng : public CN3Base
{
protected:
	int m_NextID;

public:
	std::list<CDTex*> m_pDTex;
	CMainFrame* m_pMainFrm;
	//int					m_NumDTex;	//dtex를 담고 있는 마지막 배열 + 1...^^
	//CDTex*			m_pDTex[MAX_TILETEXTURE];

public:
	int GetMaxID() const
	{
		return m_NextID - 1;
	}

	int GetNumDTex() const
	{
		return static_cast<int>(m_pDTex.size());
	}

	bool IsInDTex(CString FileName);
	CDTex* GetDTexByID(int id);
	CDTex* GetDTexByName(CString FileName);
	bool AddDTex(CString FileName);
	void DelDTexByID(int id);
	void Init(CMainFrame* pFrm = nullptr);
	void Release();

	void SaveToFile(CString RealFileName);
	void LoadFromFile(CString RealFileName);
	void SaveGameTile();

	CDTexMng();
	virtual ~CDTexMng();
};

#endif // !defined(AFX_DTEXMNG_H__30DCE41A_64F9_4876_AA67_5E344993325D__INCLUDED_)
