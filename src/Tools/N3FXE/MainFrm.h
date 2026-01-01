// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__3EC7F770_B6B0_4DE7_8DA0_8ECACAD52DC0__INCLUDED_)
#define AFX_MAINFRM_H__3EC7F770_B6B0_4DE7_8DA0_8ECACAD52DC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include <list>

#include <N3Base/N3EngTool.h>
#include <N3Base/N3Camera.h>
#include <N3Base/N3Light.h>
#include <N3Base/N3Shape.h>

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	CN3EngTool	m_Eng;
	CN3Camera	m_Camera;
	CN3Light	m_Light[2];
	DWORD		m_dwBGColor;

	std::list<class CDlgEditScript*>	m_pEditWndList;
	class CN3FXBundle*				m_pCurrFX;

	std::list<class CDlgEditPartMesh*>		m_pEditPartMesh;
	std::list<class CDlgEditPartBillBoard*> m_pEditPartBillBoard;
	std::list<class CDlgEditPartParticle*>	m_pEditPartParticle;
	std::list<class CDlgEditPartGround*>	m_pEditPartGround;
	
	std::list<class CDlgEditFxg*>			m_pEditGroup;
	
	CN3Shape	m_Chr;
	bool		m_bRenderChr;
	bool		m_bRenderGrid;
	bool		m_bRenderAxis;
	bool		m_bRenderFloor;

	class CGround*	m_pGround;
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void	TickRender();
	bool	IsDuplicated(CString& PathName);
	int		GetPartType(CString& PathName);

	void	DestroyGroup(CDlgEditFxg* pGroup);
	void	DestroyBundle(CDlgEditScript* pBundle);
	void	DestroyPartBillBoard(CDlgEditPartBillBoard* pPartMesh);
	void	DestroyPartParticle(CDlgEditPartParticle* pPartParticle);
	void	DestroyPartMesh(CDlgEditPartMesh* pPartMesh);
	void	DestroyPartGround(CDlgEditPartGround* pPartGround);

	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar      m_wndDlgBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileLoadPart();
	afx_msg void OnFileLoadBundle();
	afx_msg void OnViewModel();
	afx_msg void OnUpdateViewModel(CCmdUI* pCmdUI);
	afx_msg void OnViewGrid();
	afx_msg void OnUpdateViewGrid(CCmdUI* pCmdUI);
	afx_msg void OnViewAxis();
	afx_msg void OnUpdateViewAxis(CCmdUI* pCmdUI);
	afx_msg void OnFileNew();
	afx_msg void OnViewBgcolor();
	afx_msg void OnFileNewBundle();
	afx_msg void OnViewFloor();
	afx_msg void OnUpdateViewFloor(CCmdUI* pCmdUI);
	afx_msg void OnFileOpenFloorTex();
	afx_msg void OnFileNewGroup();
	afx_msg void OnFileLoadGroup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__3EC7F770_B6B0_4DE7_8DA0_8ECACAD52DC0__INCLUDED_)
