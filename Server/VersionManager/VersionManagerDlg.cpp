// VersionManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VersionManager.h"
#include "VersionManagerDlg.h"
#include "IOCPSocket2.h"
#include "SettingDlg.h"
#include "User.h"

#include <shared/Ini.h>

#include <db-library/ConnectionManager.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NOTE: Explicitly handled under DEBUG_NEW override
#include <db-library/RecordSetLoader.h>

import VersionManagerBinder;

CIOCPort CVersionManagerDlg::IocPort;

constexpr int DB_POOL_CHECK = 100;

/////////////////////////////////////////////////////////////////////////////
// CVersionManagerDlg dialog

CVersionManagerDlg::CVersionManagerDlg(CWnd* parent)
	: CDialog(IDD, parent),
	DbProcess(this),
	_logger(logger::VersionManager)
{
	//{{AFX_DATA_INIT(CVersionManagerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	memset(_ftpUrl, 0, sizeof(_ftpUrl));
	memset(_ftpPath, 0, sizeof(_ftpPath));
	_lastVersion = 0;

	_icon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	db::ConnectionManager::DefaultConnectionTimeout = DB_PROCESS_TIMEOUT;
	db::ConnectionManager::Create();
}

CVersionManagerDlg::~CVersionManagerDlg()
{
	db::ConnectionManager::Destroy();
}

void CVersionManagerDlg::DoDataExchange(CDataExchange* data)
{
	CDialog::DoDataExchange(data);
	//{{AFX_DATA_MAP(CVersionManagerDlg)
	DDX_Control(data, IDC_LIST1, _outputList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVersionManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CVersionManagerDlg)
	ON_WM_TIMER()
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
	SetIcon(_icon, TRUE);			// Set big icon
	SetIcon(_icon, FALSE);		// Set small icon
	
	IocPort.Init(MAX_USER, CLIENT_SOCKSIZE, 1);

	for (int i = 0; i < MAX_USER; i++)
		IocPort.m_SockArrayInActive[i] = new CUser(this);

	if (!GetInfoFromIni())
	{
		AfxMessageBox(_T("Ini File Info Error!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	// print the ODBC connection string
	// TODO: modelUtil::DbType::ACCOUNT;  Currently all models are assigned to GAME
	AddOutputMessage(
		db::ConnectionManager::GetOdbcConnectionString(modelUtil::DbType::GAME));

	if (!DbProcess.InitDatabase())
	{
		AfxMessageBox(_T("Database Connection Fail!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!LoadVersionList())
	{
		AfxMessageBox(_T("Load Version List Fail!!"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	if (!IocPort.Listen(_LISTEN_PORT))
	{
		AfxMessageBox(_T("FAIL TO CREATE LISTEN STATE"));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	::ResumeThread(IocPort.m_hAcceptThread);

	AddOutputMessage(fmt::format("Listening on 0.0.0.0:{}",
		_LISTEN_PORT));

	SetTimer(DB_POOL_CHECK, 60000, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
}

bool CVersionManagerDlg::GetInfoFromIni()
{
	CString exePath = GetProgPath();
	std::string exePathUtf8(CT2A(exePath, CP_UTF8));

	std::filesystem::path iniPath(exePath.GetString());
	iniPath /= L"Version.ini";

	CIni ini(iniPath);

	// ftp config
	ini.GetString(ini::DOWNLOAD, ini::URL, "127.0.0.1", _ftpUrl, _countof(_ftpUrl));
	ini.GetString(ini::DOWNLOAD, ini::PATH, "/", _ftpPath, _countof(_ftpPath));

	// configure logger
	_logger.Setup(ini, exePathUtf8);
	
	// TODO: KN_online should be Knight_Account
	std::string datasourceName = ini.GetString(ini::ODBC, ini::DSN, "KN_online");
	std::string datasourceUser = ini.GetString(ini::ODBC, ini::UID, "knight");
	std::string datasourcePass = ini.GetString(ini::ODBC, ini::PWD, "knight");

	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::ACCOUNT,
		datasourceName, datasourceUser, datasourcePass);

	// TODO: Remove this - currently all models are assigned to GAME
	db::ConnectionManager::SetDatasourceConfig(
		modelUtil::DbType::GAME,
		datasourceName, datasourceUser, datasourcePass);

	_defaultPath = ini.GetString(ini::CONFIGURATION, ini::DEFAULT_PATH, "");
	int serverCount = ini.GetInt(ini::SERVER_LIST, ini::COUNT, 1);

	if (strlen(_ftpUrl) == 0
		|| strlen(_ftpPath) == 0)
		return false;

	if (datasourceName.length() == 0
		// TODO: Should we not validate UID/Pass length?  Would that allow Windows Auth?
		|| datasourceUser.length() == 0
		|| datasourcePass.length() == 0)
		return false;

	if (serverCount <= 0)
		return false;

	char key[20] = {};
	ServerList.reserve(serverCount);

	for (int i = 0; i < serverCount; i++)
	{
		_SERVER_INFO* pInfo = new _SERVER_INFO;

		snprintf(key, sizeof(key), "SERVER_%02d", i);
		ini.GetString(ini::SERVER_LIST, key, "127.0.0.1", pInfo->strServerIP, _countof(pInfo->strServerIP));

		snprintf(key, sizeof(key), "NAME_%02d", i);
		ini.GetString(ini::SERVER_LIST, key, "TEST|Server 1", pInfo->strServerName, _countof(pInfo->strServerName));

		snprintf(key, sizeof(key), "ID_%02d", i);
		pInfo->sServerID = static_cast<short>(ini.GetInt(ini::SERVER_LIST, key, 1));

		snprintf(key, sizeof(key), "USER_LIMIT_%02d", i);
		pInfo->sUserLimit = static_cast<short>(ini.GetInt(ini::SERVER_LIST, key, MAX_USER));

		ServerList.push_back(pInfo);
	}

	// Read news from INI (max 3 blocks)
	std::stringstream ss;
	std::string title, message;

	News.Size = 0;
	for (int i = 0; i < MAX_NEWS_COUNT; i++)
	{
		snprintf(key, sizeof(key), "TITLE_%02d", i);
		title = ini.GetString("NEWS", key, "");
		if (title.empty())
			continue;

		snprintf(key, sizeof(key), "MESSAGE_%02d", i);
		message = ini.GetString("NEWS", key, "");
		if (message.empty())
			continue;

		ss << title;
		ss.write(NEWS_MESSAGE_START, sizeof(NEWS_MESSAGE_START));
		ss << message;
		ss.write(NEWS_MESSAGE_END, sizeof(NEWS_MESSAGE_END));
	}

	const std::string newsContent = ss.str();
	if (!newsContent.empty())
	{
		if (newsContent.size() > sizeof(News.Content))
		{
			AfxMessageBox(_T("News too long"));
			return false;
		}

		memcpy(&News.Content, newsContent.c_str(), newsContent.size());
		News.Size = static_cast<short>(newsContent.size());
	}

	// Trigger a save to flush defaults to file.
	ini.Save();

	spdlog::info("Version Manager initialized");

	return true;
}

bool CVersionManagerDlg::LoadVersionList()
{
	VersionInfoList versionList;
	if (!DbProcess.LoadVersionList(&versionList))
		return false;

	int lastVersion = 0;

	for (const auto& [_, pInfo] : versionList)
	{
		if (lastVersion < pInfo->Number)
			lastVersion = pInfo->Number;
	}

	SetLastVersion(lastVersion);

	VersionList.Swap(versionList);
	return true;
}

void CVersionManagerDlg::OnTimer(UINT EventId)
{
	if (EventId == DB_POOL_CHECK)
		db::ConnectionManager::ExpireUnusedPoolConnections();

	CDialog::OnTimer(EventId);
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
		dc.DrawIcon(x, y, _icon);
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
	return (HCURSOR) _icon;
}

BOOL CVersionManagerDlg::PreTranslateMessage(MSG* msg)
{
	if (msg->message == WM_KEYDOWN)
	{
		if (msg->wParam == VK_RETURN
			|| msg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialog::PreTranslateMessage(msg);
}

BOOL CVersionManagerDlg::DestroyWindow()
{
	KillTimer(DB_POOL_CHECK);

	if (!VersionList.IsEmpty())
		VersionList.DeleteAllData();

	for (_SERVER_INFO* pInfo : ServerList)
		delete pInfo;
	ServerList.clear();

	return CDialog::DestroyWindow();
}

void CVersionManagerDlg::OnVersionSetting() 
{
	CSettingDlg	setdlg(_lastVersion, this);

	CString conv = _defaultPath.c_str();
	_tcscpy_s(setdlg._defaultPath, conv);
	if (setdlg.DoModal() != IDOK)
		return;

	std::filesystem::path iniPath(GetProgPath().GetString());
	iniPath /= L"Version.ini";

	CIni ini(iniPath);
	_defaultPath = ini.GetString(ini::CONFIGURATION, ini::DEFAULT_PATH, "");
	ini.Save();
}

void CVersionManagerDlg::ReportTableLoadError(const recordset_loader::Error& err, const char* source)
{
	std::string error = fmt::format("VersionManagerDlg::ReportTableLoadError: {} failed: {}",
		source, err.Message);
	std::wstring werror = LocalToWide(error);
	AfxMessageBox(werror.c_str());
	spdlog::error(error);
}

// \brief updates the last/latest version and resets the output list
void CVersionManagerDlg::SetLastVersion(int lastVersion)
{
	if (lastVersion != _lastVersion)
	{
		AddOutputMessage(std::format(L"Latest Version: {}",
			lastVersion));
	}

	_lastVersion = lastVersion;
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CVersionManagerDlg::AddOutputMessage(const std::string& msg)
{
	std::wstring wMsg = LocalToWide(msg);
	AddOutputMessage(wMsg);
}

/// \brief adds a message to the application's output box and updates scrollbar position
/// \see _outputList
void CVersionManagerDlg::AddOutputMessage(const std::wstring& msg)
{
	_outputList.AddString(msg.data());
	
	// Set the focus to the last item and ensure it is visible
	int lastIndex = _outputList.GetCount()-1;
	_outputList.SetTopIndex(lastIndex);
}
