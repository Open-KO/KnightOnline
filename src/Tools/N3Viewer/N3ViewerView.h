// N3ViewerView.h : interface of the CN3ViewerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3VIEWERVIEW_H__DA98F929_87BD_4D61_8DB7_5DE373A09938__INCLUDED_)
#define AFX_N3VIEWERVIEW_H__DA98F929_87BD_4D61_8DB7_5DE373A09938__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CN3ViewerDoc;
class CN3TransformCollision;

const int MAX_SELECT = 256;

class CN3ViewerView : public CView
{
public:
	bool m_bViewGrid;
	bool m_bViewAxis;
	bool m_bViewWireFrame;
	bool m_bViewObjectAxis;
	bool m_bViewCollisionMesh;
	bool m_bViewClimbMesh;
	bool m_bViewSelectedMeshWireFrame;

	struct __Sort
	{
		CN3TransformCollision*	pObj;
		float		fCamDist;
		__Vector3	vMin, vMax;
	};
	CN3Base*		m_pSelObjs[MAX_SELECT];
	D3DCOLOR		m_crBkg; // 안개가 꺼졌을때 배경색

protected: // create from serialization only
	CN3ViewerView();
	DECLARE_DYNCREATE(CN3ViewerView)

// Attributes
public:
	CN3ViewerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3ViewerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetFocusToSelectedObject();
	CN3Base* Pick(POINT point, int* pnPart);
	static int SortByCameraDistance(const void *pArg1, const void *pArg2);
	virtual ~CN3ViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3ViewerView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewAxis();
	afx_msg void OnUpdateViewAxis(CCmdUI* pCmdUI);
	afx_msg void OnViewGrid();
	afx_msg void OnUpdateViewGrid(CCmdUI* pCmdUI);
	afx_msg void OnViewObjectAxis();
	afx_msg void OnUpdateViewObjectAxis(CCmdUI* pCmdUI);
	afx_msg void OnViewSolid();
	afx_msg void OnUpdateViewSolid(CCmdUI* pCmdUI);
	afx_msg void OnViewWireframe();
	afx_msg void OnUpdateViewWireframe(CCmdUI* pCmdUI);
	afx_msg void OnViewCollisionMesh();
	afx_msg void OnUpdateViewCollisionMesh(CCmdUI* pCmdUI);
	afx_msg void OnViewClimbMesh();
	afx_msg void OnUpdateViewClimbMesh(CCmdUI* pCmdUI);
	afx_msg void OnViewWireframeSelectedMesh();
	afx_msg void OnUpdateViewWireframeSelectedMesh(CCmdUI* pCmdUI);
	afx_msg void OnViewFocus();
	afx_msg void OnSetBackgroundColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in N3ViewerView.cpp
inline CN3ViewerDoc* CN3ViewerView::GetDocument()
   { return (CN3ViewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3VIEWERVIEW_H__DA98F929_87BD_4D61_8DB7_5DE373A09938__INCLUDED_)
