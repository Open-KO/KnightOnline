// UIEView.h : interface of the CUIEView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIEVIEW_H__02B4B855_D902_4583_916F_743B2A486BF4__INCLUDED_)
#define AFX_UIEVIEW_H__02B4B855_D902_4583_916F_743B2A486BF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CUIEView : public CView
{
protected: // create from serialization only
	CUIEView();
	DECLARE_DYNCREATE(CUIEView)

// Attributes
public:
	enum	eUIE_MODE {UIEMODE_PREVIEW=0, UIEMODE_EDIT, NUM_UIEMODE};
	enum	eRECTTYPE {RT_NONE = -1, RT_REGION=0, RT_MOVE, RT_CLICK, NUM_RECTTYPE};
	CUIEDoc* GetDocument();
	eUIE_MODE	GetMode() const {return m_eMode;}
	BOOL		ResizeSelectedRect(int dx, int dy);
	BOOL		MoveSelectedRect(int dx, int dy);
	D3DCOLOR	m_BkgndColor;

protected:
//	int			m_iZoom;
	HACCEL		m_hAccelTable;
	eUIE_MODE	m_eMode;					// 현재 Mode
	eRECTTYPE	m_eSelectedRectType;		// 수정하는 사각형 종류 (영역, 움직이는 영역, 클릭 영역)
	CPoint		m_ptOldMouse;				// 이전 마우스 좌표
	CPoint		m_ptOldLBPos;				// 이전에 왼쪽 버튼으로 찍은 마우스..

	COLORREF	m_RegionRectColor;			// 영역 사각형을 표시하는 선 색
	COLORREF	m_MoveRectColor;			// 움직이는 영역 사각형을 표시하는 선 색
	COLORREF	m_ClickRectColor;			// click 사각형을 표시하는 선 색

	enum	eDRAGTYPE {DRAGTYPE_NONE=0, DRAGTYPE_MOVE, DRAGTYPE_LEFT, DRAGTYPE_RIGHT,
					DRAGTYPE_TOP, DRAGTYPE_BOTTOM, DRAGTYPE_LEFTTOP, DRAGTYPE_RIGHTTOP,
					DRAGTYPE_LEFTBOTTOM, DRAGTYPE_RIGHTBOTTOM};
	eDRAGTYPE		m_eDragType;			// Drag 상태
	CRect			m_rcSelectedRect;		// 선택된 사각형 영역

	BOOL		m_bViewGrid; // 그리드 보기..

// Operations
public:
	CN3UIBase*	Pick(const POINT& point, CN3UIBase* pUI);
	void		SetMode(eUIE_MODE eMode);					// 모드를 변경하는 함수(Edit, Preview)
	void		SelectRectType(eRECTTYPE eRectType);		// 수정하는 사각형 종류(영역, 움직이는 영역, 클릭 영역)를 정하는 함수
	void		ChangeBkgndColor();							// 배경색을 바꿈
protected:
	void	RenderPreview();							// Preview 화면을 렌더하는 함수
	void	RenderEditview();							// Editview 화면을 렌더하는 함수
	eDRAGTYPE CheckDragType(CRect rcSel, CPoint point);	// 마우스 위치에 따라 드래그 상태를 구별하는 함수
	void	UpdateStatusBarText();						// Status Bar text를 현재 상태에 받게 갱신하는 함수
	void	UpdateUIInfo_SelectedRect();				// selected rect정보를 토대로 UI 정보를 갱신하기
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUIEView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUIEView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CUIEView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnEditModeMoveAndResize();
	afx_msg void OnEditModeSelect();
	afx_msg void OnViewGrid();
	afx_msg void OnUpdateViewGrid(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in UIEView.cpp
inline CUIEDoc* CUIEView::GetDocument()
   { return (CUIEDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIEVIEW_H__02B4B855_D902_4583_916F_743B2A486BF4__INCLUDED_)
