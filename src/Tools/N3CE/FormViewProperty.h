#if !defined(AFX_FORMVIEWPROPERTY_H__BF74B26C_54D1_42EF_8535_7BBECE9535E8__INCLUDED_)
#define AFX_FORMVIEWPROPERTY_H__BF74B26C_54D1_42EF_8535_7BBECE9535E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FormViewProperty.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFormViewProperty form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "../Common Control/PropertyList.h"
#include "Resource.h"

class CN3Base;
class CN3CEDoc;

class CFormViewProperty : public CFormView
{
public:
	HTREEITEM m_hTI_Joint;
	HTREEITEM m_hTI_Parts;
	HTREEITEM m_hTI_Plugs;
	HTREEITEM m_hTI_FXPlug;

	CImageList m_ImgList;	// 0:Position, 1:Rotation, 2:Scale 3:Camera, 4:Light, 5:Texture, 6:Shape, 7:Shape_Part
							// 8:Character, 9:Joint, 10:Character_Part, 11:Character_Plug, 12:Progressive_Mesh, 13:IndexedMesh, 14:Terrain, 15:Normal_Mesh

public:
	void UpdateWindowPos();
	void UpdateInfo();

protected:
	CFormViewProperty();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFormViewProperty)

// Form Data
public:
	//{{AFX_DATA(CFormViewProperty)
	enum { IDD = IDD_CHR_PROPERTY };
	CTreeCtrl		m_TreeChr;
	CPropertyList	m_LPMtl;
	CPropertyList	m_LPPlug;
	CPropertyList	m_LPFXPlugPart;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void SelectJointNode(HTREEITEM hItemParent, CN3Joint* pJoint);
	void UpdateJointItem(HTREEITEM hItem, CN3Joint* pJoint, int& iNumber);
	CN3CEDoc* GetDocument();
	int		GetSelectedNode();
	void	SelectNode(int nNodeIndex);
	CN3Base* GetSelectedObject();
	void UpdateAllInfo();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormViewProperty)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTreeChrEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeChrDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeChrRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditChrJointSet();
	afx_msg void OnEditChrPartAdd();
	afx_msg void OnEditChrPartDelete();
	afx_msg void OnEditChrPlugAdd();
	afx_msg void OnEditChrPlugDelete();
	afx_msg void OnEditChrPartSet();
	afx_msg void OnEditChrPlugSet();
	afx_msg void OnSelchangedTreeChr(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileSaveSelected();
	afx_msg void OnEditChrPlugRotationReset();
	afx_msg void OnEditChrPlugAddcloak();
	afx_msg void OnEditSetVisible();
	afx_msg void OnUpdateEditSetVisible(CCmdUI* pCmdUI);
	afx_msg void OnEditChrPlugImportPMesh();
	afx_msg void OnEditFxplugSet();
	afx_msg void OnEditFxplugDelete();
	afx_msg void OnEditFxplugPartAdd();
	afx_msg void OnEditFxplugPartDelete();
	afx_msg void OnEditFxplugStart();
	afx_msg void OnEditFxplugStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FormViewAnimation.cpp
inline CN3CEDoc* CFormViewProperty::GetDocument()
   { return (CN3CEDoc*)m_pDocument; }
#endif
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMVIEWPROPERTY_H__BF74B26C_54D1_42EF_8535_7BBECE9535E8__INCLUDED_)
