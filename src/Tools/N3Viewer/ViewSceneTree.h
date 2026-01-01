#if !defined(AFX_VIEWSCENETREE_H__71D8A066_827D_4575_973A_0AFE570B66EF__INCLUDED_)
#define AFX_VIEWSCENETREE_H__71D8A066_827D_4575_973A_0AFE570B66EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewSceneTree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewSceneTree view
class CN3Base;
class CN3ViewerDoc;

class CViewSceneTree : public CTreeView
{
public:
	CImageList m_ImgList;

protected:
	CViewSceneTree();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewSceneTree)

public:
	CN3ViewerDoc* GetDocument();
	void UpdateTreeItem(HTREEITEM hParent, CN3Base* pBase);
	void SelectObject(HTREEITEM hItem, void* pItemData);
//	void SelectTree(HTREEITEM hParent, int nDepth, int nSeq);
//	void GetSeq(HTREEITEM hParent, HTREEITEM hItem, int& nDepth, int *nSeq);
	
	void ExpandTree(HTREEITEM hItem);
	void UpdateAllInfo();

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewSceneTree)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CViewSceneTree();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewSceneTree)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in N3ViewerView.cpp
inline CN3ViewerDoc* CViewSceneTree::GetDocument()
   { return (CN3ViewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWSCENETREE_H__71D8A066_827D_4575_973A_0AFE570B66EF__INCLUDED_)
