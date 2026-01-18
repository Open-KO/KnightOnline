#if !defined(AFX_DLGEDITSCRIPT_H__69B3C205_1BDB_480C_9D2C_EFA8D7324EA0__INCLUDED_)
#define AFX_DLGEDITSCRIPT_H__69B3C205_1BDB_480C_9D2C_EFA8D7324EA0__INCLUDED_

#pragma once

// DlgEditScript.h : header file
//

#include <N3Base/N3FXDef.h>

class CMainFrame;
class CN3FXBundle;
class CDlgEditPart;

/////////////////////////////////////////////////////////////////////////////
// CDlgEditScript dialog
class CDlgEditScript : public CDialog
{
public:
	CMainFrame* m_pRefFrm;
	CN3FXBundle* m_pFXBundle;
	CComboBox* m_pPartName[MAX_FX_PART_TOOL];
	float* m_pPartStartTime[MAX_FX_PART_TOOL];

	// Construction
public:
	bool NewBundle();
	bool LoadBundle(CString& strPathName);
	void SaveGameDataPartnBundle();
	CDlgEditScript(CWnd* pParent = nullptr); // standard constructor
	void RefreshParts();
	void ReloadCombo();

	// Dialog Data
	//{{AFX_DATA(CDlgEditScript)
	enum
	{
		IDD = IDD_DLG_SCRIPTEDITOR
	};
	CComboBox m_CBPartName15;
	CComboBox m_CBPartName14;
	CComboBox m_CBPartName13;
	CComboBox m_CBPartName12;
	CComboBox m_CBPartName11;
	CComboBox m_CBPartName10;
	CComboBox m_CBPartName9;
	CComboBox m_CBPartName8;
	float m_fStartTime0;
	float m_fStartTime1;
	float m_fStartTime2;
	float m_fStartTime3;
	float m_fStartTime4;
	float m_fStartTime5;
	float m_fStartTime6;
	float m_fStartTime7;
	CComboBox m_CBPartName0;
	CComboBox m_CBPartName1;
	CComboBox m_CBPartName2;
	CComboBox m_CBPartName3;
	CComboBox m_CBPartName4;
	CComboBox m_CBPartName5;
	CComboBox m_CBPartName6;
	CComboBox m_CBPartName7;
	float m_fVelocity;
	BOOL m_bDependScale;
	CString m_strPathName;
	float m_fStartTime8;
	float m_fStartTime9;
	float m_fStartTime10;
	float m_fStartTime11;
	float m_fStartTime12;
	float m_fStartTime13;
	float m_fStartTime14;
	float m_fStartTime15;
	BOOL m_bStaticPos;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditScript)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditScript)
	afx_msg void OnBtnPlay();
	afx_msg void OnBtnSave();
	afx_msg void OnBtnStop();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboPart1();
	afx_msg void OnSelchangeComboPart2();
	afx_msg void OnSelchangeComboPart3();
	afx_msg void OnSelchangeComboPart4();
	afx_msg void OnSelchangeComboPart5();
	afx_msg void OnSelchangeComboPart6();
	afx_msg void OnSelchangeComboPart7();
	afx_msg void OnSelchangeComboPart8();
	afx_msg void OnChangeEditStartTime1();
	afx_msg void OnChangeEditStartTime2();
	afx_msg void OnChangeEditStartTime3();
	afx_msg void OnChangeEditStartTime4();
	afx_msg void OnChangeEditStartTime5();
	afx_msg void OnChangeEditStartTime6();
	afx_msg void OnChangeEditStartTime7();
	afx_msg void OnChangeEditStartTime8();
	afx_msg void OnBtnSaveAs();
	afx_msg void OnBtnRefresh();
	afx_msg void OnChangeEditVelocity();
	afx_msg void OnCheckDependScale();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnChangeEditStartTime9();
	afx_msg void OnChangeEditStartTime10();
	afx_msg void OnChangeEditStartTime11();
	afx_msg void OnChangeEditStartTime12();
	afx_msg void OnChangeEditStartTime13();
	afx_msg void OnChangeEditStartTime14();
	afx_msg void OnChangeEditStartTime15();
	afx_msg void OnChangeEditStartTime16();
	afx_msg void OnSelchangeComboPart9();
	afx_msg void OnSelchangeComboPart10();
	afx_msg void OnSelchangeComboPart11();
	afx_msg void OnSelchangeComboPart12();
	afx_msg void OnSelchangeComboPart13();
	afx_msg void OnSelchangeComboPart14();
	afx_msg void OnSelchangeComboPart15();
	afx_msg void OnSelchangeComboPart16();
	afx_msg void OnDropdownComboPart1();
	afx_msg void OnDropdownComboPart2();
	afx_msg void OnDropdownComboPart3();
	afx_msg void OnDropdownComboPart4();
	afx_msg void OnDropdownComboPart5();
	afx_msg void OnDropdownComboPart6();
	afx_msg void OnDropdownComboPart7();
	afx_msg void OnDropdownComboPart8();
	afx_msg void OnDropdownComboPart9();
	afx_msg void OnDropdownComboPart10();
	afx_msg void OnDropdownComboPart11();
	afx_msg void OnDropdownComboPart12();
	afx_msg void OnDropdownComboPart13();
	afx_msg void OnDropdownComboPart14();
	afx_msg void OnDropdownComboPart15();
	afx_msg void OnDropdownComboPart16();
	afx_msg void OnCheckStaticPos();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITSCRIPT_H__69B3C205_1BDB_480C_9D2C_EFA8D7324EA0__INCLUDED_)
