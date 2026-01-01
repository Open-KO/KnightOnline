// RiverMng2.h: interface for the CRiverMng2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RIVERMNG2_H__D967C973_4844_48A6_B534_E10472968A10__INCLUDED_)
#define AFX_RIVERMNG2_H__D967C973_4844_48A6_B534_E10472968A10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\N3BASE\N3BaseFileAccess.h"
#define MAX_RIVER_TEX	32
#define	VER_RIVER		1

class CRiverMng2 : public CN3BaseFileAccess  
{
public:
	struct __River
	{
		float		fHeight;
		D3DCOLOR	Diffuse;
	};
protected:
	struct __VertexRiver
	{
	public:
		float x,y,z;
		float nx,ny,nz;
		D3DCOLOR	color;
		float u,v, u2,v2;
		void Set(float sx, float sy,float sz, float snx, float sny, float snz, D3DCOLOR scolor, float su, float sv, float su2, float sv2)
		{
			x = sx, y = sy, z = sz;
			nx = snx, ny = sny, nz = sny;
			color = scolor;
			u = su, v = sv;
			u2 = su2, v2 = sv2;
		}
	};

	int		m_nMapSize;
	__River**	m_ppRiver;
	//bool**	m_ppIsRiver;

	int		m_nVer;
	bool	m_bActive;
	class	CLyTerrain*	m_pRefTerrain;
	class	CEventMgr*	m_pRefEventMgr;
	class	CDlg_River*	m_pDlgRiver;
	CRect	m_rtDrag, m_rtDragTransformed;
	bool	m_bDrag,  m_bDragRightBtn;

	void	MakeTileVB(__VertexRiver *pVtx, int x, int z);

	CN3Texture		*m_pTexRiver[MAX_RIVER_TEX];
	CN3Texture		*m_pTexWave;	
	float			m_fTexIndex;
public:
	bool	Load(HANDLE hFile);
	bool	Save(HANDLE hFile);
	void	Release();
	void	Tick();
	void	Render();

	void	SetActive(bool bActive, class CLyTerrain *pTerrain = NULL);
	bool	IsActive(){return m_bActive;}

	BOOL	MouseMsgFilter(LPMSG pMsg);

	__River**	GetRiverData(){return m_ppRiver;}

public:	
	CRiverMng2();
	virtual ~CRiverMng2();
};

#endif // !defined(AFX_RIVERMNG2_H__D967C973_4844_48A6_B534_E10472968A10__INCLUDED_)
