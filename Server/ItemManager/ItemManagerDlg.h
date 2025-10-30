#pragma once

#include "Define.h"
#include "resource.h"

#include <shared-server/logger.h>
#include <shared-server/SharedMemoryQueue.h>

class ItemManagerLogger : public logger::Logger
{
public:
	ItemManagerLogger()
		: Logger(logger::ItemManager)
	{
	}

	void SetupExtraLoggers(CIni& ini,
		std::shared_ptr<spdlog::details::thread_pool> threadPool,
		const std::string& baseDir) override;
};

/////////////////////////////////////////////////////////////////////////////
// CItemManagerDlg dialog

class ReadQueueThread;
class CItemManagerDlg : public CDialog
{
public:
	SharedMemoryQueue m_LoggerRecvQueue;
	std::unique_ptr<ReadQueueThread> _readQueueThread;

public:
	CItemManagerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CItemManagerDlg() override;

	void ItemLogWrite(const char* pBuf);
	void ExpLogWrite(const char* pBuf);

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
	ItemManagerLogger _logger;

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
