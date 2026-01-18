#if !defined(AFX_DLGEDITFXG_H__2F7E0A2C_C6F1_4F4F_B297_3A2E35168739__INCLUDED_)
#define AFX_DLGEDITFXG_H__2F7E0A2C_C6F1_4F4F_B297_3A2E35168739__INCLUDED_

#pragma once

// DlgEditFxg.h : header file
//

class CMainFrame;

const int MAX_FXB = 4;

/////////////////////////////////////////////////////////////////////////////
// CDlgEditFxg dialog

class CDlgEditFxg : public CDialog
{
	// Construction
public:
	CDlgEditFxg(CWnd* pParent = nullptr); // standard constructor

	CMainFrame* m_pRefFrm;
	CComboBox* m_pJoint[MAX_FXB];
	BOOL* m_pLoop[MAX_FXB];
	CString* m_pFXBName[MAX_FXB];

	//functions...
	bool LoadScript(const char* szPath);
	void SaveGameData();

	// Dialog Data
	//{{AFX_DATA(CDlgEditFxg)
	enum
	{
		IDD = IDD_DLG_GROUP
	};
	CComboBox m_CBJoint_3;
	CComboBox m_CBJoint_2;
	CComboBox m_CBJoint_1;
	CComboBox m_CBJoint_0;
	BOOL m_bLoop_0;
	BOOL m_bLoop_1;
	BOOL m_bLoop_2;
	BOOL m_bLoop_3;
	CString m_strFileName;
	CString m_strFXBName_0;
	CString m_strFXBName_1;
	CString m_strFXBName_2;
	CString m_strFXBName_3;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEditFxg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation

protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgEditFxg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnBtnSaveAs();
	afx_msg void OnBtnSave();
	afx_msg void OnBtnLoadFxb0();
	afx_msg void OnBtnLoadFxb1();
	afx_msg void OnBtnLoadFxb2();
	afx_msg void OnBtnLoadFxb3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITFXG_H__2F7E0A2C_C6F1_4F4F_B297_3A2E35168739__INCLUDED_)
