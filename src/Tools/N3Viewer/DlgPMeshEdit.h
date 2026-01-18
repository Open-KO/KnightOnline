#if !defined(AFX_DLGPMESHEDIT_H__03776521_2EA5_473F_BA1A_FD419AFF1FBA__INCLUDED_)
#define AFX_DLGPMESHEDIT_H__03776521_2EA5_473F_BA1A_FD419AFF1FBA__INCLUDED_

#pragma once

// DlgPMeshEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgPMeshEdit dialog
#include <N3Base/N3Shape.h>

class CDlgPMeshEdit : public CDialog
{
	// Construction
public:
	void LOD_Delete();
	void LOD_Add();
	void SavePMeshes();
	void UpdateAllInfo();
	void UpdateInfo();
	CDlgPMeshEdit(CWnd* pParent = nullptr); // standard constructor

	CN3Shape* m_pShapeRef;

	// Dialog Data
	//{{AFX_DATA(CDlgPMeshEdit)
	enum
	{
		IDD = IDD_PMESH_EDIT
	};
	CListBox m_LBLOD;
	CComboBox m_CBPart;
	CScrollBar m_ScrollBar;
	int m_iNumVertices;
	BOOL m_bPreview;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPMeshEdit)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgPMeshEdit)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnCheckPreview();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonMakePmesh();
	afx_msg void OnButtonSave();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeCbPart();
	afx_msg void OnBMakePmeshAll();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPMESHEDIT_H__03776521_2EA5_473F_BA1A_FD419AFF1FBA__INCLUDED_)
