#if !defined(AFX_VIEWPROPERTY_H__2DE18EC7_53E6_41E3_97EB_C9122F94E12E__INCLUDED_)
#define AFX_VIEWPROPERTY_H__2DE18EC7_53E6_41E3_97EB_C9122F94E12E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewProperty.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewProperty form view
class CN3ViewerDoc;

#include "../Common Control/PropertyList.h"

class CViewProperty : public CFormView
{
protected:
	CN3ViewerDoc* GetDocument();
	CViewProperty();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewProperty)

// Form Data
public:
	//{{AFX_DATA(CViewProperty)
	enum { IDD = IDD_BASE };
	CPropertyList	m_LPMaterial;
	CPropertyList	m_LPLight;
	CPropertyList	m_LPCamera;
	CComboBox	m_CBShapePart; // Shape Part Number	
	CComboBox	m_CBChrPart; // Character Part Number
	CComboBox	m_CBChrLOD; // Character LOD Number
	CComboBox	m_CBChrPlug; // Chraracter Plug Number
	CPropertyList	m_LPTransform;
	CPropertyList	m_LPShape;
	CPropertyList	m_LPChr;
	CPropertyList	m_LPCPart;
	CPropertyList	m_LPCPlug;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void UpdateInfo();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewProperty)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CViewProperty();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CViewProperty)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeCbPartShape();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in N3ViewerView.cpp
inline CN3ViewerDoc* CViewProperty::GetDocument()
   { return (CN3ViewerDoc*)m_pDocument; }
#endif
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWPROPERTY_H__2DE18EC7_53E6_41E3_97EB_C9122F94E12E__INCLUDED_)
