// N3CEView.h : interface of the CN3CEView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3CEVIEW_H__EA63D3E6_6DBA_4E70_B8E2_9526FEB7AE05__INCLUDED_)
#define AFX_N3CEVIEW_H__EA63D3E6_6DBA_4E70_B8E2_9526FEB7AE05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CN3Transform;
class CPosDummy;
class CN3Joint;
class CN3CEDoc;

#include <deque>

typedef std::deque<int>::iterator it_AniIndex;
enum e_CursorMode { eCM_Nothing, eCM_PickJoint, eCM_PlugPosition, eCM_PlugScale, eCM_PlugRotation,
					eCM_PlugFXPosition0,
					eCM_PlugFXPosition1,
					eCM_PlugFXPosition2,
					eCM_PlugFXPosition3,
					eCM_PlugFXPosition4,
					eCM_PlugFXPosition5,
					eCM_PlugFXPosition6,
					eCM_PlugFXPosition7 };

class CN3CEView : public CView
{
public:
	bool m_bRenderOptionXRay;
	bool m_bRenderOptionJoint;
	bool m_bPlayingNow;
	float m_fTickPrev;
	CString m_szFPS;

	std::deque<int> m_DequeAnimation;

	CPoint m_ptPrev;
	CN3Joint* m_pJointSelected;

	e_CursorMode m_eCursorMode;

	CN3Transform*	m_pFXPosTransform;
	CPosDummy*		m_pPosDummy;

protected: // create from serialization only
	CN3CEView();
	DECLARE_DYNCREATE(CN3CEView)

// Attributes
public:
	CN3CEDoc*	GetDocument();

// Operations
public:
	void InitFXPos();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3CEView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	void UpdateAllInfo();
	void SetCameraToDefault();
	virtual ~CN3CEView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3CEView)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnViewXray();
	afx_msg void OnUpdateViewXray(CCmdUI* pCmdUI);
	afx_msg void OnViewJoint();
	afx_msg void OnUpdateViewJoint(CCmdUI* pCmdUI);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in N3CEView.cpp
inline CN3CEDoc* CN3CEView::GetDocument()
   { return (CN3CEDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3CEVIEW_H__EA63D3E6_6DBA_4E70_B8E2_9526FEB7AE05__INCLUDED_)
