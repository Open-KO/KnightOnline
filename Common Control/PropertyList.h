#if !defined(AFX_PROPERTYLIST_H__74205380_1B56_11D4_BC48_00105AA2186F__INCLUDED_)
#define AFX_PROPERTYLIST_H__74205380_1B56_11D4_BC48_00105AA2186F__INCLUDED_

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyList.h : header file
//

#define PIT_COMBO	0  //PIT = property item type
#define PIT_EDIT	1
#define PIT_COLOR	2
#define PIT_FONT	3
#define PIT_FILE	4
#define PIT_FILE_MULTI	5 // FileName1'\n'FileName2'\n'....FileName'\n' 이렇게 문자열이 된다..
#define PIT_CHECK	6
#define PIT_BUTTON	7

#define IDC_PROPCMBBOX   712
#define IDC_PROPEDITBOX  713
#define IDC_PROP_BUTTON  714

/////////////////////////////////////////////////////////////////////////////
//CPropertyList Items
class CPropertyItem
{
// Attributes
public:
	CString			m_propName;
	CString			m_curValue; // Color 값의 Alpha 값으로도 쓰인다..
	int				m_nItemType;
	CStringArray	m_szCBItemsOrFilter;
	COLORREF		m_crColor; // ColorReference | Combo Box Index 로 쓰인다..

public:
	void D3DColorSet(D3DCOLOR color);
	D3DCOLOR D3DColorGet();
	void D3DColorValueSet(D3DCOLORVALUE color);
	D3DCOLORVALUE D3DColorValueGet();
	CPropertyItem(const char* szPropName, const char* szVal, int nItemType, const char* szComboItemsOrFilter, COLORREF crColor = 0);
	
	D3DVECTOR VectorGet();
	void VectorSet(const D3DVECTOR& v);
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyList window

class CPropertyList : public CListBox
{
// Construction
public:
	CPropertyList();

// Attributes
public:

// Operations
public:
	int AddItem(CString txt);
	int AddPropItem(const char* szPropName, const char* szVal, int nItemType, const char* szComboItemsOrFilter, COLORREF crColor = 0);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyList)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	void DeleteItem( LPDELETEITEMSTRUCT lpDeleteItemStruct );
	void SetDividerWidth(int x);
	CPropertyItem* GetPropItem(const char* szPropName);
	CPropertyItem* GetPropItem(int iIndex) { return ((CPropertyItem*)(GetItemDataPtr(iIndex))); }
	virtual ~CPropertyList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelchange();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnKillfocusCmbBox();
	afx_msg void OnSelchangeCmbBox();
	afx_msg void OnKillfocusEditBox();
	afx_msg void OnChangeEditBox();
	afx_msg void OnButton();

	DECLARE_MESSAGE_MAP()

	void InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo);
	void DisplayPushButton(CRect region);
//	void DisplayCheckButton(CRect region);

	CComboBox m_ComboBox;
	CEdit m_EditBox;
	CButton m_ButtonPush;
//	CButton m_ButtonCheck;
	CFont m_SSerif8Font;

	
	int m_curSel,m_prevSel;
	int m_nDivider;
	int m_nDivTop;
	int m_nDivBtm;
	int m_nOldDivX;
	int m_nLastBox;
	BOOL m_bTracking;
	BOOL m_bDivIsSet;
	HCURSOR m_hCursorArrow;
	HCURSOR m_hCursorSize;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYLIST_H__74205380_1B56_11D4_BC48_00105AA2186F__INCLUDED_)
