// VersionManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VersionManager.h"
#include "VersionManagerDlg.h"
#include "IOCPSocket2.h"
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
	: CDialog(CVersionManagerDlg::IDD, pParent),
	m_DBProcess(this)
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
		m_Iocport.m_SockArrayInActive[i] = new CUser(this);

	if (!m_Iocport.Listen(_LISTEN_PORT))
	{
		AfxMessageBox(_T("FAIL TO CREATE LISTEN STATE"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!GetInfoFromIni())
	{
		AfxMessageBox(_T("Ini File Info Error!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	CString strConnection;
	strConnection.Format(_T("ODBC;DSN=%s;UID=%s;PWD=%s"), m_ODBCName, m_ODBCLogin, m_ODBCPwd);

	if (!m_DBProcess.InitDatabase(strConnection))
	{
		AfxMessageBox(_T("Database Connection Fail!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!m_DBProcess.LoadVersionList())
	{
		AfxMessageBox(_T("Load Version List Fail!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	m_OutputList.AddString(strConnection);
	CString version;
	version.Format(_T("Latest Version : %d"), m_nLastVersion);
	m_OutputList.AddString(version);

	::ResumeThread(m_Iocport.m_hAcceptThread);

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CVersionManagerDlg::GetInfoFromIni()
{
	std::filesystem::path iniPath(GetProgPath().GetString());
	iniPath /= L"Version.ini";

	CIni ini(iniPath);

	ini.GetString("DOWNLOAD", "URL", "127.0.0.1", m_strFtpUrl, _countof(m_strFtpUrl));
	ini.GetString("DOWNLOAD", "PATH", "/", m_strFilePath, _countof(m_strFilePath));

	ini.GetString(_T("ODBC"), _T("DSN"), _T("KN_online"), m_ODBCName, _countof(m_ODBCName));
	ini.GetString(_T("ODBC"), _T("UID"), _T("knight"), m_ODBCLogin, _countof(m_ODBCLogin));
	ini.GetString(_T("ODBC"), _T("PWD"), _T("knight"), m_ODBCPwd, _countof(m_ODBCPwd));
	ini.GetString(_T("ODBC"), _T("TABLE"), _T("VERSION"), m_TableName, _countof(m_TableName));
	ini.GetString(_T("CONFIGURATION"), _T("DEFAULT_PATH"), _T(""), m_strDefaultPath, _countof(m_strDefaultPath));

	m_nServerCount = ini.GetInt("SERVER_LIST", "COUNT", 1);

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

	char key[20] = {};
	m_ServerList.reserve(20);

	for (int i = 0; i < m_nServerCount; i++)
	{
		_SERVER_INFO* pInfo = new _SERVER_INFO;

		snprintf(key, sizeof(key), "SERVER_%02d", i);
		ini.GetString("SERVER_LIST", key, "127.0.0.1", pInfo->strServerIP, _countof(pInfo->strServerIP));

		snprintf(key, sizeof(key), "NAME_%02d", i);
		ini.GetString("SERVER_LIST", key, "TEST|Server 1", pInfo->strServerName, _countof(pInfo->strServerName));

		snprintf(key, sizeof(key), "ID_%02d", i);
		pInfo->sServerID = static_cast<short>(ini.GetInt("SERVER_LIST", key, 1));

		snprintf(key, sizeof(key), "USER_LIMIT_%02d", i);
		pInfo->sUserLimit = static_cast<short>(ini.GetInt("SERVER_LIST", key, MAX_USER));

		m_ServerList.push_back(pInfo);
	}

	// Read news from INI (max 3 blocks)
	std::stringstream ss;
	std::string title, message;

	m_News.Size = 0;
	for (int i = 0; i < 3; i++)
	{
		snprintf(key, sizeof(key), "TITLE_%02d", i);
		title = ini.GetString("NEWS", key, "");
		if (title.empty())
			continue;

		snprintf(key, sizeof(key), "MESSAGE_%02d", i);
		message = ini.GetString("NEWS", key, "");
		if (message.empty())
			continue;

#define BOX_START			'#' << uint8_t(0) << '\n'
#define LINE_ENDING			uint8_t(0) << '\n'
#define BOX_END				BOX_START << LINE_ENDING

		ss << title << BOX_START;
		ss << message << LINE_ENDING << BOX_END;

#undef BOX_START
#undef LINE_ENDING
#undef BOX_END
	}

	const std::string newsContent = ss.str();
	if (!newsContent.empty())
	{
		if (newsContent.size() > sizeof(m_News.Content))
		{
			AfxMessageBox(_T("News too long"));
			return FALSE;
		}

		memcpy(&m_News.Content, newsContent.c_str(), newsContent.size());
		m_News.Size = static_cast<short>(newsContent.size());
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
	CSettingDlg	setdlg(m_nLastVersion, this);
	
	_tcscpy(setdlg.m_strDefaultPath, m_strDefaultPath);
	if (setdlg.DoModal() != IDOK)
		return;

	_tcscpy(m_strDefaultPath, setdlg.m_strDefaultPath);

	std::filesystem::path iniPath(GetProgPath().GetString());
	iniPath /= L"Version.ini";

	CIni ini(iniPath);
	ini.SetString(_T("CONFIGURATION"), _T("DEFAULT_PATH"), m_strDefaultPath);
	ini.Save();
}
