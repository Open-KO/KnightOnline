// N3FXEView.h : interface of the CN3FXEView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3FXEVIEW_H__DE5A5E73_150A_458B_9C8A_002C189405A5__INCLUDED_)
#define AFX_N3FXEVIEW_H__DE5A5E73_150A_458B_9C8A_002C189405A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CN3FXBundle;
class CN3FXEDoc;

class CN3FXEView : public CView
{
protected: // create from serialization only
	CN3FXEView();
	DECLARE_DYNCREATE(CN3FXEView)

// Attributes
public:
	CN3FXEDoc* GetDocument();
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3FXEView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~CN3FXEView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3FXEView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in N3FXEView.cpp
inline CN3FXEDoc* CN3FXEView::GetDocument()
   { return (CN3FXEDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3FXEVIEW_H__DE5A5E73_150A_458B_9C8A_002C189405A5__INCLUDED_)
