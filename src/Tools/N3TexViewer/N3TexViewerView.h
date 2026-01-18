// N3TexViewerView.h : interface of the CN3TexViewerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TEXVIEWERVIEW_H__097CAE80_F002_4377_B06B_E1537225B22D__INCLUDED_)
#define AFX_N3TEXVIEWERVIEW_H__097CAE80_F002_4377_B06B_E1537225B22D__INCLUDED_

#pragma once

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
	void OnDraw(CDC* pDC) override; // overridden to draw this view
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	void OnInitialUpdate() override;

protected:
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
	void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) override;
	void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	//}}AFX_VIRTUAL

	// Implementation
public:
	~CN3TexViewerView() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
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

#ifndef _DEBUG // debug version in N3TexViewerView.cpp
inline CN3TexViewerDoc* CN3TexViewerView::GetDocument()
{
	return (CN3TexViewerDoc*) m_pDocument;
}
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3TEXVIEWERVIEW_H__097CAE80_F002_4377_B06B_E1537225B22D__INCLUDED_)
