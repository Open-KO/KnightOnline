// SkyViewerView.h : interface of the CSkyViewerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKYVIEWERVIEW_H__538AD5EF_87DB_4CAD_9F27_14CC46A58DC2__INCLUDED_)
#define AFX_SKYVIEWERVIEW_H__538AD5EF_87DB_4CAD_9F27_14CC46A58DC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSkyViewerView : public CView
{
protected: // create from serialization only
	CSkyViewerView();
	DECLARE_DYNCREATE(CSkyViewerView)

// Attributes
public:
	class CSkyViewerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkyViewerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSkyViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSkyViewerView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SkyViewerView.cpp
inline CSkyViewerDoc* CSkyViewerView::GetDocument()
   { return (CSkyViewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKYVIEWERVIEW_H__538AD5EF_87DB_4CAD_9F27_14CC46A58DC2__INCLUDED_)
