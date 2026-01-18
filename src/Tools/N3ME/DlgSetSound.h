#if !defined(AFX_DLGSETSOUND_H__78CB4442_580D_4CBC_9C3F_3927A48D1218__INCLUDED_)
#define AFX_DLGSETSOUND_H__78CB4442_580D_4CBC_9C3F_3927A48D1218__INCLUDED_

#pragma once

// DlgSetSound.h : header file
//

#include "SoundMgr.h"
class CSoundCell;

/////////////////////////////////////////////////////////////////////////////
// CDlgSetSound dialog

class CDlgSetSound : public CDialog
{
	// Construction
public:
	CDlgSetSound(CWnd* pParent = nullptr); // standard constructor

										   // Dialog Data
	//{{AFX_DATA(CDlgSetSound)
	enum
	{
		IDD = IDD_EDIT_BGM
	};
	CListBox m_ListSoundGroup;
	CListBox m_ListSoundInfo;
	//}}AFX_DATA

public:
	// Variables...
	CSoundMgr* m_pRefSoundMgr;
	CSoundCell* m_pSelSound;

	int m_iIdx_Min;
	int m_iIdx_Max;

	// functions...
	void Clear();
	void AddSoundInfo(CSoundCell* pSound);
	void AddSoundGroup(char* szTitle, LPSOUNDINFO pSoundInfo);
	int MakeIdx();
	bool LoadSoundGroup(File& file);
	bool SaveSoundGroup(File& file);
	LPSOUNDINFO GetSoundGroup(DWORD dwID);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSetSound)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

													 // Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgSetSound)
	afx_msg void OnBtnAddSoundgroup();
	afx_msg void OnBtnDeleteSoundgroup();
	afx_msg void OnBtnInputInfo();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeListSoundgroup();
	afx_msg void OnSelchangeListSoundinfo();
	afx_msg void OnBtnInputInfoDel();
	afx_msg void OnBtnInputInfoEdit();
	afx_msg void OnBtnModifySoundgroup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETSOUND_H__78CB4442_580D_4CBC_9C3F_3927A48D1218__INCLUDED_)
