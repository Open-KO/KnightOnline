#pragma once

#include "Define.h"

#include <shared-server/SharedMemoryQueue.h>

/////////////////////////////////////////////////////////////////////////////
// CItemManagerDlg dialog

class CItemManagerDlg : public CDialog
{
// Construction
public:
	SharedMemoryQueue m_LoggerRecvQueue;

	HANDLE	m_hReadQueueThread;

	CFile m_ItemLogFile;		// log file
	CFile m_ExpLogFile;			// log file
	int	m_nItemLogFileDay;		// 
	int	m_nExpLogFileDay;		// 

private:
	void WriteItemLogFile(char* pData);
	void WriteExpLogFile(char* pData);

public:
	CItemManagerDlg(CWnd* pParent = nullptr);	// standard constructor

	void ItemLogWrite(char* pBuf);
	void ExpLogWrite(char* pBuf);

// Dialog Data
	//{{AFX_DATA(CItemManagerDlg)
	enum { IDD = IDD_ITEMMANAGER_DIALOG };
	CListBox	m_strOutList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CItemManagerDlg)
public:
	virtual BOOL DestroyWindow();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CItemManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnExitBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
