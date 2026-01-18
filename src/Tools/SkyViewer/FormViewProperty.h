#if !defined(AFX_FORMVIEWPROPERTY_H__53610780_A85B_4CBC_BFC9_75E2E766FC43__INCLUDED_)
#define AFX_FORMVIEWPROPERTY_H__53610780_A85B_4CBC_BFC9_75E2E766FC43__INCLUDED_

#pragma once

// FormViewProperty.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "../Common Control/PropertyList.h"

class CFormViewProperty : public CFormView
{
protected:
	BOOL m_bUpdatingNow;
	BOOL m_bPlayingNow;

	// Form Data
public:
	//{{AFX_DATA(CFormViewProperty)
	enum
	{
		IDD = IDD_FORMVIEW_PROPERTY
	};
	CSliderCtrl m_SldTime;
	CListBox m_ListCloudTextures;
	CListBox m_ListSunTextures;
	CListBox m_ListDayChanges;
	CPropertyList m_LPProperty;
	//}}AFX_DATA

public:
	void UpdateTime();
	void UpdateInfo();
	void UpdateAllInfo();

protected:
	CSkyViewerDoc* GetDocument();

	CFormViewProperty(); // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFormViewProperty)

						 // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormViewProperty)
public:
	virtual void OnInitialUpdate();

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

	// Implementation
protected:
	virtual ~CFormViewProperty();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CFormViewProperty)
	afx_msg void OnSelchangeListDayChange();
	afx_msg void OnBAdd();
	afx_msg void OnBInsert();
	afx_msg void OnBDelete();
	afx_msg void OnBMoveUp();
	afx_msg void OnBMoveDown();
	afx_msg void OnChangeEName();
	afx_msg void OnCPlay();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDblclkListCloudTextures();
	afx_msg void OnDblclkListSunTextures();
	afx_msg void OnBBrowseMoonTexture();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBRainy();
	afx_msg void OnBSnows();
	afx_msg void OnBClean();
	afx_msg void OnApplyDate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG // debug version in SkyViewerView.cpp
inline CSkyViewerDoc* CFormViewProperty::GetDocument()
{
	return (CSkyViewerDoc*) m_pDocument;
}
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMVIEWPROPERTY_H__53610780_A85B_4CBC_BFC9_75E2E766FC43__INCLUDED_)
