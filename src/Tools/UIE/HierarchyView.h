// HierarchyView.h : interface of the CHierarchyView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_HIERARCHYVIEW_H__99E40D58_CC41_4A2B_9CCC_CA157570E22F__INCLUDED_)
#define AFX_HIERARCHYVIEW_H__99E40D58_CC41_4A2B_9CCC_CA157570E22F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUIEDoc;
class CN3UIBase;
class CHierarchyView : public CTreeView
{
protected: // create from serialization only
	CHierarchyView();
	DECLARE_DYNCREATE(CHierarchyView)

// Attributes
public:
	CUIEDoc* GetDocument();

// Operations
public:
protected:
	void UpdateAllInfo();										// 모든 tree item 정보 갱신하기
	void UpdateTreeItem(HTREEITEM hParent, CN3UIBase *pUIBase);	// tree item 정보 갱신하기
	void SelectObject(HTREEITEM hItem,  CN3UIBase* pUIBase);	// UPBase 포인터로 tree item 선택하기

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHierarchyView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHierarchyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CHierarchyView)
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in HierarchyView.cpp
inline CUIEDoc* CHierarchyView::GetDocument()
   { return (CUIEDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HIERARCHYVIEW_H__99E40D58_CC41_4A2B_9CCC_CA157570E22F__INCLUDED_)
