// KscViewerView.h : interface of the CKscViewerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_KSCVIEWERVIEW_H__AC830AF2_D01D_48DD_A29E_036847EB5E61__INCLUDED_)
#define AFX_KSCVIEWERVIEW_H__AC830AF2_D01D_48DD_A29E_036847EB5E61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CKscViewerView : public CView
{
protected: // create from serialization only
	CKscViewerView();
	DECLARE_DYNCREATE(CKscViewerView)

// Attributes
public:
	CKscViewerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKscViewerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL DrawImage();
	virtual ~CKscViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CKscViewerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in KscViewerView.cpp
inline CKscViewerDoc* CKscViewerView::GetDocument()
   { return (CKscViewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KSCVIEWERVIEW_H__AC830AF2_D01D_48DD_A29E_036847EB5E61__INCLUDED_)
