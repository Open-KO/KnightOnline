#if !defined(AFX_FORMVIEWTOOL_H__17EA3D2C_64E7_4C80_95CA_F01045F7BDF7__INCLUDED_)
#define AFX_FORMVIEWTOOL_H__17EA3D2C_64E7_4C80_95CA_F01045F7BDF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FormViewTool.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFormViewTool form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CFormViewTool : public CFormView
{
protected:
	CFormViewTool();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFormViewTool)

// Form Data
public:
	//{{AFX_DATA(CFormViewTool)
	enum { IDD = IDD_TOOL };
	CListBox	m_ListSound1;
	CListBox	m_ListSound0;
	CComboBox	m_CBLOD;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void UpdateAllInfo();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormViewTool)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFormViewTool();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CFormViewTool)
	afx_msg void OnBMakeSmooth();
	afx_msg void OnSelchangeCbLod();
	afx_msg void OnBAddTestSound0();
	afx_msg void OnBDeleteTestSound0();
	afx_msg void OnBAddTestSound1();
	afx_msg void OnBDeleteTestSound1();
	afx_msg void OnSelchangeListSound0();
	afx_msg void OnSelchangeListSound1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBApplyOffsetFrame();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMVIEWTOOL_H__17EA3D2C_64E7_4C80_95CA_F01045F7BDF7__INCLUDED_)
