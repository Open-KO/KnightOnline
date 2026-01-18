#if !defined(AFX_DLGMAKENPCPATH_H__A6BC8EF9_5309_42E9_8986_CDE2F8565813__INCLUDED_)
#define AFX_DLGMAKENPCPATH_H__A6BC8EF9_5309_42E9_8986_CDE2F8565813__INCLUDED_

#pragma once

// DlgMakeNPCPath.h : header file
//

class CNPCPathMgr;
class CNPCPath;

/////////////////////////////////////////////////////////////////////////////
// CDlgMakeNPCPath dialog

class CDlgMakeNPCPath : public CDialog
{
	// Construction
public:
	void SetNumPoint(int NumPoint);
	CNPCPath* m_pSelPath;
	CNPCPathMgr* m_pRefNPCPathMgr;
	char m_strSelNPCName[80];
	int m_iSelNPCID;
	int m_iSelActType;

	unsigned char m_cSelAttrRegen;
	unsigned char m_cSelAttrGroup;
	unsigned char m_cSelAttrCreate;
	unsigned char m_cSelOption;

	CDlgMakeNPCPath(CWnd* pParent = nullptr); // standard constructor

											  // Dialog Data
	//{{AFX_DATA(CDlgMakeNPCPath)
	enum
	{
		IDD = IDD_MAKE_NPCPATH
	};
	CComboBox m_OptionList;
	CComboBox m_AttrRegenList;
	CComboBox m_AttrGroupList;
	CComboBox m_AttrCreateList;
	CComboBox m_ActTypeList;
	CListBox m_ListPathGroup;
	CComboBox m_NPCList;
	int m_NumNPC;
	int m_iRegenTime;
	CString m_PathSetFileName;
	int m_State;
	CString m_strNPCPathName;
	int m_ViewNonMove;
	float m_fTransX;
	float m_fTransZ;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMakeNPCPath)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgMakeNPCPath)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditNumNpc();
	afx_msg void OnSelchangeComboNpc();
	afx_msg void OnBtnRecord();
	virtual void OnOK();
	afx_msg void OnRdoStartRgn();
	afx_msg void OnRdoMakePath();
	afx_msg void OnChangeEditRegentime();
	afx_msg void OnSelchangeComboActiontype();
	afx_msg void OnBtnLoadPathset();
	afx_msg void OnBtnSavePathset();
	afx_msg void OnBtnSaveServerPathset();
	afx_msg void OnBtnPathDelete();
	afx_msg void OnBtnPathModify();
	afx_msg void OnSelchangeListNpcpath();
	afx_msg void OnChangeEdtNpcpathname();
	afx_msg void OnRdoViewNone();
	afx_msg void OnRdoViewNonmove();
	afx_msg void OnBtnSetcolor();
	afx_msg void OnBtnLoadNpclist();
	afx_msg void OnBtnTrans();
	afx_msg void OnRdoMakeActRgn();
	afx_msg void OnBtnLoadCreate();
	afx_msg void OnBtnLoadGroup();
	afx_msg void OnBtnLoadRegen();
	afx_msg void OnSelchangeComboNpcAttrCreate();
	afx_msg void OnSelchangeComboNpcAttrGroup();
	afx_msg void OnSelchangeComboNpcAttrRegen();
	afx_msg void OnBtnLoadOption();
	afx_msg void OnSelchangeComboNpcOption();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMAKENPCPATH_H__A6BC8EF9_5309_42E9_8986_CDE2F8565813__INCLUDED_)
