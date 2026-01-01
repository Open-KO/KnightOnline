// N3MEView.h : interface of the CN3MEView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3MEVIEW_H__335CC7EE_91A2_41E3_88CA_75FD02DBDBFC__INCLUDED_)
#define AFX_N3MEVIEW_H__335CC7EE_91A2_41E3_88CA_75FD02DBDBFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMapMng;
class CN3MEDoc;
class CN3MEView : public CView
{
public:

protected: // create from serialization only
	CN3MEView();
	DECLARE_DYNCREATE(CN3MEView)

// Attributes
public:
	CPoint		m_CurrMousePos;
	CN3MEDoc*	GetDocument();
	void		SetMapMng(CMapMng* pMapMng) {m_pMapMng = pMapMng;}
protected:
//	LYCAMERADATA	m_LyCamera;
//	CMove			m_Move;
	CMapMng*		m_pMapMng;
	
// Operations
public:
protected:
	void Render();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3MEView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CN3MEView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3MEView)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in N3MEView.cpp
inline CN3MEDoc* CN3MEView::GetDocument()
   { return (CN3MEDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3MEVIEW_H__335CC7EE_91A2_41E3_88CA_75FD02DBDBFC__INCLUDED_)
