// ItemManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ItemManager.h"
#include "ItemManagerDlg.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD WINAPI ReadQueueThread(LPVOID lp)
{
	CItemManagerDlg* pMain = (CItemManagerDlg*) lp;
	int recvlen = 0, index = 0;
	BYTE command;
	char recv_buff[1024] = {};
	CString string;

	while (true)
	{
		if (pMain->m_LoggerRecvQueue.GetFrontMode() != R)
		{
			index = 0;
			recvlen = pMain->m_LoggerRecvQueue.GetData(recv_buff);
			if (recvlen > MAX_PKTSIZE)
			{
				Sleep(1);
				continue;
			}

			command = GetByte(recv_buff, index);
			switch (command)
			{
				case WIZ_ITEM_LOG:
					pMain->ItemLogWrite(recv_buff + index);
					break;

				case WIZ_DATASAVE:
					pMain->ExpLogWrite(recv_buff + index);
					break;
			}

			recvlen = 0;
			memset(recv_buff, 0, sizeof(recv_buff));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CItemManagerDlg dialog

CItemManagerDlg::CItemManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CItemManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CItemManagerDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	memset(m_strGameDSN, 0, sizeof(m_strGameDSN));
	memset(m_strGameUID, 0, sizeof(m_strGameUID));
	memset(m_strGamePWD, 0, sizeof(m_strGamePWD));
	m_nItemLogFileDay = 0;
	m_nServerNo = 0; m_nZoneNo = 0;
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
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//----------------------------------------------------------------------
	//	Logfile initialize
	//----------------------------------------------------------------------
	CTime time = CTime::GetCurrentTime();
	TCHAR strLogFile[50] = {};
	wsprintf(strLogFile, _T("ItemLog-%d-%d-%d.txt"), time.GetYear(), time.GetMonth(), time.GetDay());
	m_ItemLogFile.Open(strLogFile, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone);
	m_ItemLogFile.SeekToEnd();

	memset(strLogFile, 0, sizeof(strLogFile));
	wsprintf(strLogFile, _T("ExpLog-%d-%d-%d.txt"), time.GetYear(), time.GetMonth(), time.GetDay());
	m_ExpLogFile.Open(strLogFile, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone);
	m_ExpLogFile.SeekToEnd();

	m_nItemLogFileDay = time.GetDay();
	m_nExpLogFileDay = time.GetDay();

	// Dispatcher 의 Send Queue
	if (!m_LoggerRecvQueue.InitializeMMF(MAX_PKTSIZE, MAX_COUNT, _T(SMQ_ITEMLOGGER), false))
	{
		AfxMessageBox(_T("Shared memory queue not yet available. Run Ebenezer first."));
		AfxPostQuitMessage(0);
		return FALSE;
	}
/*
	CString inipath;
	inipath.Format(_T("%s\\ItemDB.ini"), GetProgPath());

	GetPrivateProfileString( "ODBC", "GAME_DSN", "", m_strGameDSN, 24, inipath );
	GetPrivateProfileString( "ODBC", "GAME_UID", "", m_strGameUID, 24, inipath );
	GetPrivateProfileString( "ODBC", "GAME_PWD", "", m_strGamePWD, 24, inipath );

	m_nServerNo = GetPrivateProfileInt("ZONE_INFO", "GROUP_INFO", 1, inipath);
	m_nZoneNo = GetPrivateProfileInt("ZONE_INFO", "ZONE_INFO", 1, inipath);

	if( !m_DBAgent.DatabaseInit() ) {
		AfxPostQuitMessage(0);
		return FALSE;
	}	*/

	DWORD id;
	m_hReadQueueThread = ::CreateThread(nullptr, 0, ReadQueueThread, this, 0, &id);

	CTime cur = CTime::GetCurrentTime();
	CString starttime;
	starttime.Format(_T("ItemManager Start : %d-%d day %d:%d time\r\n"), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute());
	m_ItemLogFile.Write(starttime, starttime.GetLength());

	return TRUE;  // return TRUE  unless you set the focus to a control
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

void CItemManagerDlg::ItemLogWrite(char* pBuf)
{
	int index = 0, srclen = 0, tarlen = 0, type = 0, putitem = 0, putcount = 0, putdure = 0;
	int getitem = 0, getcount = 0, getdure = 0;
	int64_t putserial = 0, getserial = 0;
	char srcid[MAX_ID_SIZE + 1] = {},
		tarid[MAX_ID_SIZE + 1] = {},
		strLog[100] = {};

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
//	getserial = GetInt64( pBuf, index );
//	getitem = GetDWORD( pBuf, index );
//	getcount = GetShort( pBuf, index );
//	getdure = GetShort( pBuf, index );

	/// 
	//sprintf(strLog, "%d, %s, %d, %s, %d, %20d, %d, %d, %d, %20d, %d, %d, %d", srclen, srcid, tarlen, tarid, type, putserial, putitem, putcount, putdure, getserial, getitem, getcount, getdure);
	sprintf(strLog, "%s, %s, %d, %I64d, %d, %d, %d", srcid, tarid, type, putserial, putitem, putcount, putdure);
	WriteItemLogFile(strLog);
}

void CItemManagerDlg::WriteItemLogFile(char* pData)
{
	CTime cur = CTime::GetCurrentTime();
	char strLog[512] = {};
	int nDay = cur.GetDay();

	if (m_nItemLogFileDay != nDay)
	{
		if (m_ItemLogFile.m_hFile != CFile::hFileNull)
			m_ItemLogFile.Close();

		CString filename;
		filename.Format(_T("ItemLog-%d-%d-%d.txt"), cur.GetYear(), cur.GetMonth(), cur.GetDay());

		if (m_ItemLogFile.Open(filename, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone))
			m_ItemLogFile.SeekToEnd();

		m_nItemLogFileDay = nDay;
	}

	sprintf(strLog, "%d-%d-%d %d:%d, %s\r\n", cur.GetYear(), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute(), pData);
	int nLen = strlen(strLog);
	if (nLen >= 512)
	{
		TRACE(_T("### WriteLogFile Fail : length = %d ###\n"), nLen);
		return;
	}

	m_ItemLogFile.Write(strLog, nLen);
}

void CItemManagerDlg::WriteExpLogFile(char* pData)
{
	CTime cur = CTime::GetCurrentTime();
	char strLog[512] = {};
	int nDay = cur.GetDay();

	if (m_nExpLogFileDay != nDay)
	{
		if (m_ExpLogFile.m_hFile != CFile::hFileNull)
			m_ExpLogFile.Close();

		CString filename;
		filename.Format(_T("Exp-%d-%d-%d.txt"), cur.GetYear(), cur.GetMonth(), cur.GetDay());

		if (m_ExpLogFile.Open(filename, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone))
			m_ExpLogFile.SeekToEnd();

		m_nExpLogFileDay = nDay;
	}

	sprintf(strLog, "%d-%d-%d %d:%d, %s\r\n", cur.GetYear(), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute(), pData);
	int nLen = strlen(strLog);
	if (nLen >= 512)
	{
		TRACE(_T("### WriteLogFile Fail : length = %d ###\n"), nLen);
		return;
	}

	m_ExpLogFile.Write(strLog, nLen);
}

BOOL CItemManagerDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_hReadQueueThread != nullptr)
		::TerminateThread(m_hReadQueueThread, 0);

	if (m_ItemLogFile.m_hFile != CFile::hFileNull)
		m_ItemLogFile.Close();

	if (m_ExpLogFile.m_hFile != CFile::hFileNull)
		m_ExpLogFile.Close();

	return CDialog::DestroyWindow();
}

void CItemManagerDlg::ExpLogWrite(char* pBuf)
{
	int index = 0, aclen = 0, charlen = 0, type = 0, level = 0, exp = 0, loyalty = 0, money = 0;
	char acname[MAX_ID_SIZE + 1] = {},
		charid[MAX_ID_SIZE + 1] = {},
		strLog[100] = {};

	aclen = GetShort(pBuf, index);
	if (aclen <= 0
		|| aclen > MAX_ID_SIZE)
	{
		TRACE(_T("### ItemLogWrite Fail : tarlen = %d ###\n"), aclen);
		return;
	}

	GetString(acname, pBuf, aclen, index);
	charlen = GetShort(pBuf, index);
	if (charlen <= 0
		|| charlen > MAX_ID_SIZE)
	{
		TRACE(_T("### ItemLogWrite Fail : tarlen = %d ###\n"), charlen);
		return;
	}

	GetString(charid, pBuf, charlen, index);
	type = GetByte(pBuf, index);
	level = GetByte(pBuf, index);
	exp = GetDWORD(pBuf, index);
	loyalty = GetDWORD(pBuf, index);
	money = GetDWORD(pBuf, index);

	sprintf(strLog, "%s, %s, %d, %d, %d, %d, %d", acname, charid, type, level, exp, loyalty, money);
	WriteExpLogFile(strLog);
}

void CItemManagerDlg::OnExitBtn()
{
	// TODO: Add your control notification handler code here
	if (AfxMessageBox(_T("진짜 끝낼까요?"), MB_YESNO) == IDYES)
		CDialog::OnOK();
}
