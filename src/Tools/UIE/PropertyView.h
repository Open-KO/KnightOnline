#if !defined(AFX_PROPERTYVIEW_H__5DBF88F9_D535_4292_A230_CB192C574AD5__INCLUDED_)
#define AFX_PROPERTYVIEW_H__5DBF88F9_D535_4292_A230_CB192C574AD5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropertyView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include "PropertyList.h"

class CUIEDoc;
class CN3UIBase;
class CN3UIImage;
class CN3UIString;
class CN3UIButton;
class CN3UIProgress;
class CN3UITrackBar;
class CN3UIScrollBar;
class CN3UIArea;
class CPropertyView : public CFormView
{
protected:
	CUIEDoc* GetDocument();
	CPropertyView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPropertyView)

// Form Data
public:
	//{{AFX_DATA(CPropertyView)
	enum { IDD = IDD_PROPERTY };
	CPropertyList	m_UIStatic;
	CPropertyList	m_UIIconSlot;
	CPropertyList	m_UIArea;
	CPropertyList	m_UIEdit;
	CPropertyList	m_UITrackBar;
	CPropertyList	m_UIScrollBar;
	CPropertyList	m_UIProgress;
	CPropertyList	m_UIString;
	CPropertyList	m_UIImage;
	CPropertyList	m_UIButton;
	CPropertyList	m_UIBase;
	CPropertyList	m_UIList;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void UpdateInfo();		// 모든 정보 갱신
	void UpdateUIBaseInfo();			// Base정보 갱신
	void UpdateUIImageInfo();			// Image정보 갱신
	void UpdateUIStringInfo();			// String정보 갱신
	void UpdateUIListInfo();			// List정보 갱신
	void UpdateUIButtonInfo();			// Button정보 갱신
	void UpdateUIEditInfo();			// Edit정보 갱신
	void UpdateUIProgressInfo();		// Progress정보 갱신
	void UpdateUITrackBarInfo();		// Trackbar정보 갱신
	void UpdateUIScrollBarInfo();		// ScrollBar정보 갱신
	void UpdateUIAreaInfo();			// Area정보 갱신
	void UpdateUIIconSlotInfo();		// IconSlot 정보 갱신(리펜트 전용)
	void UpdateUIStaticInfo();			// Static 정보 갱신
protected:
	void Resize();			// 화면 구성요소 재배치
	void SetPropertyListPos(CPropertyList* pList, CPoint& ptWndPos);	// 구성요소 특정위치에 배치하는 함수
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPropertyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CPropertyView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG  // debug version in UIEView.cpp
inline CUIEDoc* CPropertyView::GetDocument()
   { return (CUIEDoc*)m_pDocument; }
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYVIEW_H__5DBF88F9_D535_4292_A230_CB192C574AD5__INCLUDED_)
