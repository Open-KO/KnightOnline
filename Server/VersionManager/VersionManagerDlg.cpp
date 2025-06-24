// VersionManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VersionManager.h"
#include "VersionManagerDlg.h"
#include "IOCPSocket2.h"
#include "VersionSet.h"
#include "SettingDlg.h"
#include "User.h"

#include <shared/Ini.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CIOCPort CVersionManagerDlg::m_Iocport;

/////////////////////////////////////////////////////////////////////////////
// CVersionManagerDlg dialog

CVersionManagerDlg::CVersionManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CVersionManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVersionManagerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	memset(m_strFtpUrl, 0, sizeof(m_strFtpUrl));
	memset(m_strFilePath, 0, sizeof(m_strFilePath));
	m_nLastVersion = 0;
	memset(m_ODBCName, 0, sizeof(m_ODBCName));
	memset(m_ODBCLogin, 0, sizeof(m_ODBCLogin));
	memset(m_ODBCPwd, 0, sizeof(m_ODBCPwd));
	memset(m_TableName, 0, sizeof(m_TableName));

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVersionManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVersionManagerDlg)
	DDX_Control(pDX, IDC_LIST1, m_OutputList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVersionManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CVersionManagerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SETTING, OnVersionSetting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVersionManagerDlg message handlers

BOOL CVersionManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	m_Iocport.Init(MAX_USER, CLIENT_SOCKSIZE, 1);

	for (int i = 0; i < MAX_USER; i++)
		m_Iocport.m_SockArrayInActive[i] = new CUser;

	if (!m_Iocport.Listen(_LISTEN_PORT))
	{
		AfxMessageBox("FAIL TO CREATE LISTEN STATE");
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!GetInfoFromIni())
	{
		AfxMessageBox("Ini File Info Error!!");
		AfxPostQuitMessage(0);
		return FALSE;
	}

	char strconnection[256] = {};
	sprintf(strconnection, "ODBC;DSN=%s;UID=%s;PWD=%s", m_ODBCName, m_ODBCLogin, m_ODBCPwd);

	if (!m_DBProcess.InitDatabase(strconnection))
	{
		AfxMessageBox("Database Connection Fail!!");
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!m_DBProcess.LoadVersionList())
	{
		AfxMessageBox("Load Version List Fail!!");
		AfxPostQuitMessage(0);
		return FALSE;
	}

	m_OutputList.AddString(strconnection);
	CString version;
	version.Format("Latest Version : %d", m_nLastVersion);
	m_OutputList.AddString(version);

	::ResumeThread(m_Iocport.m_hAcceptThread);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CVersionManagerDlg::GetInfoFromIni()
{
	CString inipath;
	inipath.Format(_T("%s\\Version.ini"), GetProgPath());

	CIni ini(inipath.GetString());

	ini.GetString("DOWNLOAD", "URL", "", m_strFtpUrl, _countof(m_strFtpUrl));
	ini.GetString("DOWNLOAD", "PATH", "", m_strFilePath, _countof(m_strFilePath));

	ini.GetString(_T("ODBC"), _T("DSN"), _T(""), m_ODBCName, _countof(m_ODBCName));
	ini.GetString(_T("ODBC"), _T("UID"), _T(""), m_ODBCLogin, _countof(m_ODBCLogin));
	ini.GetString(_T("ODBC"), _T("PWD"), _T(""), m_ODBCPwd, _countof(m_ODBCPwd));
	ini.GetString(_T("ODBC"), _T("TABLE"), _T(""), m_TableName, _countof(m_TableName));
	ini.GetString(_T("CONFIGURATION"), _T("DEFAULT_PATH"), _T(""), m_strDefaultPath, _countof(m_strDefaultPath));

	m_nServerCount = ini.GetInt("SERVER_LIST", "COUNT", 0);

	if (strlen(m_strFtpUrl) == 0
		|| strlen(m_strFilePath) == 0)
		return FALSE;

	if (_tcslen(m_ODBCName) == 0
		|| _tcslen(m_ODBCLogin) == 0
		|| _tcslen(m_ODBCPwd) == 0
		|| _tcslen(m_TableName) == 0)
		return FALSE;

	if (m_nServerCount <= 0)
		return FALSE;

	char ipkey[20] = {},
		namekey[20] = {};

	m_ServerList.reserve(20);

	for (int i = 0; i < m_nServerCount; i++)
	{
		_SERVER_INFO* pInfo = new _SERVER_INFO;
		sprintf(ipkey, "SERVER_%02d", i);
		sprintf(namekey, "NAME_%02d", i);
		ini.GetString("SERVER_LIST", ipkey, "", pInfo->strServerIP, _countof(pInfo->strServerIP));
		ini.GetString("SERVER_LIST", namekey, "", pInfo->strServerName, _countof(pInfo->strServerName));
		m_ServerList.push_back(pInfo);
	}

	return TRUE;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVersionManagerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVersionManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CVersionManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN
		|| pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CVersionManagerDlg::DestroyWindow()
{
	if (!m_VersionList.IsEmpty())
		m_VersionList.DeleteAllData();

	for (_SERVER_INFO* pInfo : m_ServerList)
		delete pInfo;
	m_ServerList.clear();

	return CDialog::DestroyWindow();
}

void CVersionManagerDlg::OnVersionSetting() 
{
	CString errorstr, inipath;
	inipath.Format(_T("%s\\Version.ini"), GetProgPath());

	CSettingDlg	setdlg(m_nLastVersion, this);
	
	_tcscpy(setdlg.m_strDefaultPath, m_strDefaultPath);
	if( setdlg.DoModal() == IDOK ) {
		strcpy( m_strDefaultPath, setdlg.m_strDefaultPath );
		WritePrivateProfileString("CONFIGURATION", "DEFAULT_PATH", m_strDefaultPath, inipath);
	}
}
