// ItemManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ItemManagerDlg.h"
#include "ItemManager.h"

#include <shared/Ini.h>
#include <spdlog/spdlog.h>

#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NOTE: Explicitly handled under DEBUG_NEW override
#include "ItemManagerReadQueueThread.h"

/////////////////////////////////////////////////////////////////////////////
// CItemManagerDlg dialog

CItemManagerDlg::CItemManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CItemManagerDlg::IDD, pParent),
	_logger()
{
	//{{AFX_DATA_INIT(CItemManagerDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	_readQueueThread = std::make_unique<ItemManagerReadQueueThread>(this);
}

CItemManagerDlg::~CItemManagerDlg()
{
}

void CItemManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CItemManagerDlg)
	DDX_Control(pDX, IDC_OUT_LIST, m_strOutList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CItemManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CItemManagerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_EXIT_BTN, OnExitBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CItemManagerDlg message handlers

BOOL CItemManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//----------------------------------------------------------------------
	//	Logfile initialize
	//----------------------------------------------------------------------
	CString exePath(GetProgPath());
	std::string exePathUtf8(CT2A(exePath, CP_UTF8));

	std::filesystem::path iniPath(exePath.GetString());
	iniPath /= L"ItemManager.ini";

	CIni ini(iniPath);

	// configure logger
	_logger.Setup(ini, exePathUtf8);

	if (!m_LoggerRecvQueue.Open(SMQ_ITEMLOGGER))
	{
		AfxMessageBox(_T("Shared memory queue not yet available. Run Ebenezer first."));
		AfxPostQuitMessage(0);
		return FALSE;
	}

	_readQueueThread->start();

	CTime cur = CTime::GetCurrentTime();
	CString strLog;
	strLog.Format(_T("%04d/%02d/%02d %02d:%02d ItemManager started"),
		cur.GetYear(), cur.GetMonth(), cur.GetDay(),
		cur.GetHour(), cur.GetMinute());
	m_strOutList.AddString(strLog);

	spdlog::info("ItemManager started");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void ItemManagerLogger::SetupExtraLoggers(CIni& ini,
	std::shared_ptr<spdlog::details::thread_pool> threadPool,
	const std::string& baseDir)
{
	SetupExtraLogger(ini, threadPool, baseDir, logger::ItemManagerItem, ini::ITEM_LOG_FILE);
	SetupExtraLogger(ini, threadPool, baseDir, logger::ItemManagerExp, ini::EXP_LOG_FILE);
}

void CItemManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CItemManagerDlg::OnPaint()
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
HCURSOR CItemManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CItemManagerDlg::ItemLogWrite(const char* pBuf)
{
	int index = 0, srclen = 0, tarlen = 0, type = 0, putitem = 0, putcount = 0, putdure = 0;
	int64_t putserial = 0;
	char srcid[MAX_ID_SIZE + 1] = {},
		tarid[MAX_ID_SIZE + 1] = {};

	srclen = GetShort(pBuf, index);
	if (srclen <= 0
		|| srclen > MAX_ID_SIZE)
	{
		TRACE(_T("### ItemLogWrite Fail : srclen = %d ###\n"), srclen);
		return;
	}

	GetString(srcid, pBuf, srclen, index);

	tarlen = GetShort(pBuf, index);
	if (tarlen <= 0
		|| tarlen > MAX_ID_SIZE)
	{
		TRACE(_T("### ItemLogWrite Fail : tarlen = %d ###\n"), tarlen);
		return;
	}

	GetString(tarid, pBuf, tarlen, index);

	type = GetByte(pBuf, index);
	putserial = GetInt64(pBuf, index);
	putitem = GetDWORD(pBuf, index);
	putcount = GetShort(pBuf, index);
	putdure = GetShort(pBuf, index);

	spdlog::get(logger::ItemManagerItem)->info("{}, {}, {}, {}, {}, {}, {}",
		srcid, tarid, type, putserial, putitem, putcount, putdure);
}

void CItemManagerDlg::ExpLogWrite(const char* pBuf)
{
	int index = 0, aclen = 0, charlen = 0, type = 0, level = 0, exp = 0, loyalty = 0, money = 0;
	char acname[MAX_ID_SIZE + 1] = {},
		charid[MAX_ID_SIZE + 1] = {};

	aclen = GetShort(pBuf, index);
	if (aclen <= 0
		|| aclen > MAX_ID_SIZE)
	{
		TRACE(_T("### ExpLogWrite Fail : tarlen = %d ###\n"), aclen);
		return;
	}

	GetString(acname, pBuf, aclen, index);
	charlen = GetShort(pBuf, index);
	if (charlen <= 0
		|| charlen > MAX_ID_SIZE)
	{
		TRACE(_T("### ExpLogWrite Fail : tarlen = %d ###\n"), charlen);
		return;
	}

	GetString(charid, pBuf, charlen, index);
	type = GetByte(pBuf, index);
	level = GetByte(pBuf, index);
	exp = GetDWORD(pBuf, index);
	loyalty = GetDWORD(pBuf, index);
	money = GetDWORD(pBuf, index);

	spdlog::get(logger::ItemManagerExp)->info("{}, {}, {}, {}, {}, {}, {}",
		acname, charid, type, level, exp, loyalty, money);
}

BOOL CItemManagerDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	if (_readQueueThread != nullptr)
		_readQueueThread->shutdown();

	return CDialog::DestroyWindow();
}

void CItemManagerDlg::OnExitBtn()
{
	// TODO: Add your control notification handler code here
	if (AfxMessageBox(_T("Are you sure you wish to exit?"), MB_YESNO) == IDYES)
		PostQuitMessage(0);
}
