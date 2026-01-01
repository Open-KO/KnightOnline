#if !defined(AFX_DLG_RIVER_H__5084B2FB_3DD4_404E_9B5F_319440CBBBD3__INCLUDED_)
#define AFX_DLG_RIVER_H__5084B2FB_3DD4_404E_9B5F_319440CBBBD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Dlg_River.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlg_River dialog
#include "../Common Control/PropertyList.h"

class CDlg_River : public CDialog
{
// Construction
public:
	CDlg_River(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlg_River)
	enum { IDD = IDD_RIVER };
	CPropertyList	m_List;
	//}}AFX_DATA

	D3DCOLOR	GetColor()
	{
		CPropertyItem *pItem = m_List.GetPropItem("Water Color");
		return pItem->D3DColorGet();
	}
	float		GetHeight()
	{
		CPropertyItem *pItem = m_List.GetPropItem("Height");
		return atof(pItem->m_curValue);
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlg_River)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	float		m_fHeight;

	// Generated message map functions
	//{{AFX_MSG(CDlg_River)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditHeight();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLG_RIVER_H__5084B2FB_3DD4_404E_9B5F_319440CBBBD3__INCLUDED_)
