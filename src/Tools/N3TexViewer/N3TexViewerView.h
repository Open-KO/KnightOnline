// N3TexViewerView.h : interface of the CN3TexViewerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TEXVIEWERVIEW_H__097CAE80_F002_4377_B06B_E1537225B22D__INCLUDED_)
#define AFX_N3TEXVIEWERVIEW_H__097CAE80_F002_4377_B06B_E1537225B22D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CN3TexViewerView : public CView
{
protected: // create from serialization only
	CN3TexViewerView();
	DECLARE_DYNCREATE(CN3TexViewerView)

// Attributes
public:
	CN3TexViewerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3TexViewerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CN3TexViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3TexViewerView)
	afx_msg void OnViewAlpha();
	afx_msg void OnUpdateViewAlpha(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in N3TexViewerView.cpp
inline CN3TexViewerDoc* CN3TexViewerView::GetDocument()
   { return (CN3TexViewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3TEXVIEWERVIEW_H__097CAE80_F002_4377_B06B_E1537225B22D__INCLUDED_)
