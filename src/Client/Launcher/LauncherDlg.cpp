// LauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Launcher.h"
#include "LauncherDlg.h"
#include <wininet.h>

#include "PacketDef.h"
#include "APISocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLauncherDlg dialog

CLauncherDlg::CLauncherDlg(CWnd* pParent /*=nullptr*/) : CDialog(CLauncherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLauncherDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	m_pSocket = nullptr;

	memset(m_strServiceName, 0, sizeof(m_strServiceName));
	m_nCurVersion    = 0;
	m_nServerVersion = 0;
	m_nGetFileNum    = 0;

	m_hRegistryKey   = nullptr;
	memset(m_nVersionNum, 0, sizeof(m_nVersionNum));
	m_hFtpConnection = nullptr;
	m_hInetSession   = nullptr;
}

void CLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLauncherDlg)
	DDX_Control(pDX, IDC_STATUS, m_Status);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLauncherDlg, CDialog)
//{{AFX_MSG_MAP(CLauncherDlg)
ON_WM_QUERYDRAGICON()
ON_WM_DESTROY()
ON_WM_SETFOCUS()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLauncherDlg message handlers

BOOL CLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString szInfo;
	(void) szInfo.LoadString(IDS_INFO_VERSION_CHECK);
	m_Status.SetWindowText(szInfo);        // 화면에 표시..

	m_progress.SetColor(RGB(64, 255, 64)); // 프로그래스 색을 정한다.

	m_pSocket = new CAPISocket();

	CString szProduct, szKey = "SOFTWARE\\";
	(void) szProduct.LoadString(IDS_PRODUCT);
	szKey          += szProduct;

	m_hRegistryKey  = nullptr;
	long lStatus    = RegOpenKey(HKEY_CURRENT_USER, szKey, &m_hRegistryKey);
	if (ERROR_SUCCESS != lStatus)
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_REGISTRY_OPEN);
		MessageBox(szErr);
		exit(-1);
	}

	DWORD dwType  = REG_SZ;
	DWORD dwBytes = 0;
	TCHAR szBuff[256] {};

	dwType  = REG_DWORD;
	dwBytes = 4;
	lStatus = RegQueryValueEx(
		m_hRegistryKey, _T("VERSION"), nullptr, &dwType, (BYTE*) (&m_nCurVersion), &dwBytes);
	if (ERROR_SUCCESS != lStatus)
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_REGISTRY_READ_VERSION);
		MessageBox(szErr);
		exit(-1);
	}

	for (int j = 0; j < MAX_DOWNLOAD_FILE; j++)
		m_nVersionNum[j] = m_nCurVersion;

	dwType  = REG_SZ;
	dwBytes = 256;
	lStatus = RegQueryValueEx(
		m_hRegistryKey, _T("PATH"), nullptr, &dwType, (BYTE*) szBuff, &dwBytes); // 인스톨 경로
	if (ERROR_SUCCESS != lStatus)
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_REGISTRY_READ_PATH);
		MessageBox(szErr);
		exit(-1);
	}
	m_szInstalledPath = szBuff;

	dwType            = REG_SZ;
	dwBytes           = 256;
	lStatus           = RegQueryValueEx(
        m_hRegistryKey, _T("EXE"), nullptr, &dwType, (BYTE*) szBuff, &dwBytes); // 실행파일 이름
	if (ERROR_SUCCESS != lStatus)
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_REGISTRY_READ_EXE);
		MessageBox(szErr);
		exit(-1);
	}
	m_szExeName = szBuff;

	dwType      = REG_SZ;
	dwBytes     = 256;
	lStatus     = RegQueryValueEx(m_hRegistryKey, _T("SERVICE"), nullptr, &dwType,
			(BYTE*) m_strServiceName, &dwBytes); // 서비스 이름..
	if (ERROR_SUCCESS != lStatus)
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_REGISTRY_READ_SERVICE);
		MessageBox(szErr);
		exit(-1);
	}

	// 소켓 접속..
	char szIniPath[_MAX_PATH] {};
	::GetCurrentDirectoryA(_MAX_PATH, szIniPath);
	lstrcatA(szIniPath, "\\Server.Ini");
	int iServerCount = GetPrivateProfileIntA("Server", "Count", 0, szIniPath);

	std::vector<std::string> ips;
	ips.reserve(iServerCount);
	for (int i = 0; i < iServerCount; i++)
	{
		char szKey[32] {}, szIP[128] {};

		snprintf(szKey, sizeof(szKey), "IP%d", i);
		GetPrivateProfileStringA("Server", szKey, "", szIP, sizeof(szIP), szIniPath);
		ips.push_back(szIP);
	}

	if (iServerCount > 0)
	{
		int iServer = rand() % iServerCount;
		while (!m_pSocket->Connect(m_hWnd, ips[iServer].c_str(), CONNECT_PORT))
		{
			int iErrCode = GetLastError();
			CString szFmt;
			(void) szFmt.LoadString(IDS_FMT_FAILED_CONNECT_LOGIN_SERVER);

			CString szErr;
			szErr.Format(szFmt, iErrCode);
			if (MessageBox(szErr, _T(""), MB_YESNO) == IDNO)
			{
				AfxPostQuitMessage(0);
				return FALSE;
			}
			else
				continue;
		}
	}
	else
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_INVALID_SERVER_COUNT);
		this->MessageBox(szInfo); // 끝낸다.
		PostQuitMessage(0);
	}

	TCHAR titlebar[256] {};
	_stprintf(titlebar, _T("%s AUTO UPGRADE LAUNCHER"), m_strServiceName);
	SetWindowText(titlebar);

	this->PacketSend_VersionReq();

	return TRUE; // return TRUE  unless you set the focus to a control
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
BOOL CLauncherDlg::DestroyWindow()
{
	if (m_pSocket)
	{
		m_pSocket->Disconnect();
		delete m_pSocket;
		m_pSocket = nullptr;
	}

	return CDialog::DestroyWindow();
}

CString CLauncherDlg::GetProgPath()
{
	TCHAR Buf[256], Path[256];
	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	::GetModuleFileName(AfxGetApp()->m_hInstance, Buf, 256);
	_tsplitpath(Buf, drive, dir, fname, ext);
	_tcscpy(Path, drive);
	_tcscat(Path, dir);
	return Path;
}

void CLauncherDlg::PacketSend_VersionReq()
{
	int iOffset = 0;
	BYTE byBuffs[128] {};

	m_pSocket->MP_AddByte(byBuffs, iOffset, VERSION_REQ);
	m_pSocket->MP_AddShort(byBuffs, iOffset, m_nCurVersion);

	m_pSocket->Send(byBuffs, iOffset);
}

void CLauncherDlg::PacketSend_DownloadInfo()
{
	int iOffset = 0;
	BYTE byBuffs[128] {};

	m_pSocket->MP_AddByte(byBuffs, iOffset, DOWNLOAD_INFO_REQ);
	m_pSocket->MP_AddShort(byBuffs, iOffset, (short) m_nCurVersion);

	m_pSocket->Send(byBuffs, iOffset);
}

void CLauncherDlg::PacketProcess(BYTE* pBuf, int size)
{
	if (nullptr == pBuf)
		return;

	BYTE command;
	int iIndex = 0;
	command    = m_pSocket->Parse_GetByte(pBuf, iIndex);
	switch (command)
	{
		case VERSION_REQ:
			this->PacketReceive_Version(pBuf, iIndex);
			break;
		case DOWNLOAD_INFO_REQ:
			this->PacketReceive_DownloadInfo(pBuf, iIndex);
			break;
	}
}

void CLauncherDlg::PacketReceive_DownloadInfo(const BYTE* pBuf, int& iIndex)
{
	int iLen = 0;

	iLen     = m_pSocket->Parse_GetShort(pBuf, iIndex);
	m_pSocket->Parse_GetString(pBuf, iIndex, m_szFtpUrl, iLen);
	iLen = m_pSocket->Parse_GetShort(pBuf, iIndex);
	m_pSocket->Parse_GetString(pBuf, iIndex, m_szFtpPath, iLen);
	m_nGetFileNum = m_pSocket->Parse_GetShort(pBuf, iIndex);

	if (m_nGetFileNum < 1 || m_nGetFileNum >= 32)
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_INVALID_DOWNLOAD_FILE_COUNT);
		MessageBox(szErr);
		AfxPostQuitMessage(0);
	}

	char szBuf[MAX_PATH] {};
	std::string szVersion;
	int nVersion = 0;
	for (int i = 0; i < m_nGetFileNum; i++)
	{
		iLen = m_pSocket->Parse_GetShort(pBuf, iIndex);
		m_pSocket->Parse_GetString(pBuf, iIndex, m_szGetFileNames[i], iLen);

		int components = sscanf(m_szGetFileNames[i].c_str(), "patch%s", szBuf);
		if (components < 1)
			continue;

		szVersion = szBuf;
		szVersion.resize(4);
		nVersion = atoi(szVersion.c_str());

		if (m_nVersionNum[i] < nVersion)
			m_nVersionNum[i] = nVersion;
	}

	DownloadProcess();
}

void CLauncherDlg::PacketReceive_Version(const BYTE* pBuf, int& iIndex)
{
	m_nServerVersion = m_pSocket->Parse_GetShort(pBuf, iIndex);
	if (m_nCurVersion == m_nServerVersion)     // 버전이 일치하면..
	{
		this->StartGame();                     // 게임 실행..
	}
	else if (m_nCurVersion < m_nServerVersion) // 버전이 낮으면..
	{
		PacketSend_DownloadInfo();             // 다운로드 요청..
	}
	else
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_INVALID_VERSION);
		MessageBox(szErr);
		PostQuitMessage(-1);
	}
}

void CLauncherDlg::StartGame()
{
	CString szCmd = GetCommandLine(); // 커맨드 라인을 가져오고..
	TCHAR szApp[_MAX_PATH] {};
	GetModuleFileName(nullptr, szApp, _MAX_PATH);
	int iML = lstrlen(szApp);

	CString szParam;
	if (iML >= 0)
	{
		int ii = szCmd.Find(szApp);
		if (ii >= 0 && szCmd.GetLength() > ii + iML + 2)
			szParam = szCmd.Mid(ii + iML + 2);
	}

	CString szExeFN = m_szInstalledPath + _T("\\") + m_szExeName; // 실행 파일 이름 만들고..
	::ShellExecute(
		nullptr, _T("open"), szExeFN, szParam, m_szInstalledPath, SW_SHOWNORMAL); // 게임 실행..

	PostQuitMessage(0);
}

void CLauncherDlg::DownloadProcess()
{
	if (!FTP_Open())
	{
		PostQuitMessage(0);
		return;
	}

	m_progress.SetRange(0, 100);
	m_progress.SetPos(0);

	std::string szFullPath;
	CString szLocalFName;

	bool bExtractSuccess = true;

	for (int i = 0; i < m_nGetFileNum; i++)
	{
		szFullPath            = m_szFtpPath + "/" + m_szGetFileNames[i];
		BOOL bDownloadSuccess = GetDownloadFile(szFullPath, m_szGetFileNames[i]);
		while (!bDownloadSuccess)
		{
			CString szErr;
			(void) szErr.LoadString(IDS_ERR_DOWNLOAD_PATCH_FILE_AND_RETRY); // 다시 시도할까여??
			int iID = MessageBox(szErr, _T("Patch error"), MB_YESNO);
			if (IDYES == iID)
				bDownloadSuccess = GetDownloadFile(szFullPath, m_szGetFileNames[i]);
			else
			{
				AfxPostQuitMessage(-1);
				break;
			}
		}

		if (bDownloadSuccess)
		{
			szLocalFName.Format(
				_T("%s\\%hs"), m_szInstalledPath.GetString(), m_szGetFileNames[i].c_str());

			CString szInfo;
			(void) szInfo.LoadString(IDS_INFO_EXTRACTING);
			m_Status.SetWindowText(szInfo);

			if (false == ArchiveClose())
			{
				bExtractSuccess = false;
				break;
			}
			if (false == ArchiveOpen(szLocalFName))
			{
				bExtractSuccess = false;
				break;
			}
			if (false == ArchiveExtract(m_szInstalledPath))
			{
				bExtractSuccess = false;
				break;
			}
			if (false == ArchiveClose())
			{
				bExtractSuccess = false;
				break;
			}

			CFile file;
			if (file.Open(szLocalFName, CFile::modeRead | CFile::shareDenyNone, nullptr))
			{
				file.Close();
				file.Remove(szLocalFName);
				if (m_hRegistryKey) // 압축 풀기와 쓰기, 압축 파일 삭제에 성공하면 버전을 쓰고..
				{
					RegSetValueEx(m_hRegistryKey, _T("VERSION"), 0, REG_DWORD,
						((BYTE*) (&m_nVersionNum[i])), 4);
				}
			}
			else
			{
				bExtractSuccess = false;
				break;
			}
		}
		else
		{
			FTP_Close();

			CString szErr, szMsg;
			(void) szErr.LoadString(IDS_ERR_DOWNLOAD_PATCH_FILE);
			szMsg.Format(_T("%hs"), m_szGetFileNames[i].c_str());
			MessageBox(szMsg, szErr);

			PostQuitMessage(0);

			bExtractSuccess = false;
			break;
		}
	}

	FTP_Close();

	//	CString inipath, version;
	//	inipath.Format( "%s\\server.ini", GetProgPath() );
	//	itoa( m_nServerVersion, (char*)(LPCTSTR)version, 10 );
	//	WritePrivateProfileString("VERSION","CURRENT",version, inipath);

	if (true == bExtractSuccess
		&& m_hRegistryKey) // 압축 풀기와 쓰기, 압축 파일 삭제에 성공하면 버전을 쓰고..
	{
		long lStatus = RegSetValueEx(
			m_hRegistryKey, _T("VERSION"), 0, REG_DWORD, ((BYTE*) (&m_nServerVersion)), 4);
		this->StartGame(); // 게임 실행..
	}
	else
	{
		CString szErr;
		(void) szErr.LoadString(IDS_ERR_PATCH);
		MessageBox(szErr);
		PostQuitMessage(-1);
	}
}

BOOL CLauncherDlg::FTP_Open()
{
	m_hInetSession = InternetOpen(
		_T("3DOnline"), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (!m_hInetSession)
	{
		CString szErr, szMsg;
		(void) szErr.LoadString(IDS_ERR_INIT_INTERNET);
		szMsg.Format(_T("%hs"), m_szFtpUrl.c_str());
		MessageBox(szMsg, szErr);
		return FALSE;
	}

	m_hFtpConnection = InternetConnectA(m_hInetSession, m_szFtpUrl.c_str(),
		INTERNET_DEFAULT_FTP_PORT, "anonymous", "download", INTERNET_SERVICE_FTP,
		INTERNET_FLAG_PASSIVE, 0);

	if (!m_hFtpConnection)
	{
		CString szErr, szMsg;
		(void) szErr.LoadString(IDS_ERR_CONNECT_FTP);
		szMsg.Format(_T("%hs"), m_szFtpUrl.c_str());
		MessageBox(szMsg, szErr);
		return FALSE;
	}

	return TRUE;
}

void CLauncherDlg::FTP_Close()
{
	if (m_hFtpConnection)
	{
		InternetCloseHandle(m_hFtpConnection);
		m_hFtpConnection = nullptr;
	}

	if (m_hInetSession)
	{
		InternetCloseHandle(m_hInetSession);
		m_hInetSession = nullptr;
	}
}

BOOL CLauncherDlg::GetDownloadFile(const std::string& szFtpUrl, const std::string& szFileName)
{
	if (!m_hFtpConnection)
		return FALSE;

	DWORD dwFileSize = IsFtpExistFile(szFtpUrl, szFileName);

	// open the file
	HINTERNET hFile;

	//CInternetFile *pFile;
	hFile = FtpOpenFileA(
		m_hFtpConnection, szFtpUrl.c_str(), GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 0);

	if (!hFile)
		return FALSE;

	// read & save the file...
	CString szLocalFName;
	szLocalFName.Format(_T("%s\\%hs"), m_szInstalledPath.GetString(), szFileName.c_str());

	FILE* fp = _tfopen(szLocalFName, _T("wb"));
	if (fp == nullptr)
	{
		MessageBox(_T("Can`t open local file"));

		InternetCloseHandle(hFile);
		return TRUE;
	}

	DWORD dwReadSize = 0, dwTemp = 0;
	char Buffer[1024];
	BOOL bRes;
	MSG pMsg;
	BOOL bPeekMessage;

	uint64_t dwLastTime = ::GetTickCount64();

	while (dwReadSize < dwFileSize)
	{
		bRes = InternetReadFile(hFile, Buffer, 1024, &dwTemp);
		fwrite(Buffer, 1, dwTemp, fp);
		dwReadSize += dwTemp;

		if ((dwFileSize / 1024) == 0)
			m_progress.SetPos((dwReadSize / 1024) * 100 / 1);
		else
			m_progress.SetPos((dwReadSize / 1024) * 100 / (dwFileSize / 1024));

		CString szInfo;
		szInfo.Format(_T("%hs Downloading..."), szFileName.c_str());
		m_Status.SetWindowText(szInfo);

		bPeekMessage = ::PeekMessage(&pMsg, nullptr, 0, 0, PM_REMOVE);
		if (bPeekMessage)
		{
			TranslateMessage(&pMsg);
			DispatchMessage(&pMsg);
		}

		//		if ( m_fCancel || !bRes )
		//			break;

		//		Sleep(100);
	}
	fclose(fp);

	//if (dwReadSize != dwFileSize)
	if (dwReadSize < dwFileSize)
	{
		InternetCloseHandle(hFile);

		/*		if ( m_fCancel )
		{
			MessageBox("File Size Error");
			//// added by manseek for DOWNLOAD
			CString sLink;
			sLink.Format("http://download.mgame.com/download/%s.exe", (LPCTSTR)m_strGameID );
			HINSTANCE t = ShellExecute(GetSafeHwnd(), "open", sLink, nullptr,  nullptr, SW_SHOWNORMAL);
			//// 2001.5.23

			return FALSE;
		};
*/
		MessageBox(_T("File Size Error"));

		return FALSE;
	}

	// close the file
	InternetCloseHandle(hFile);

	return TRUE;
}

BOOL CLauncherDlg::IsFtpExistFile(const std::string& szFtpUrl, const std::string& szFileName)
{
	CString strNameU, strNameL, strName;

	strNameU = szFtpUrl.c_str();
	strNameU.MakeUpper();

	strNameL = szFtpUrl.c_str();
	strNameL.MakeLower();

	CString strSub1, strSub2;

	CString strTemp;
	strTemp = szFileName.c_str();

	strSub1 = strNameU.Left(strNameU.GetLength() - strTemp.GetLength() + 1);
	strSub2 = strNameU.Right(strTemp.GetLength() - 1);

	strSub2.MakeLower();

	strName = strSub1 + strSub2;

	if (!m_hFtpConnection)
		return 0;

	// find the file...
	WIN32_FIND_DATA FindFileData;
	HINTERNET hFind;

	hFind = FtpFindFirstFile(m_hFtpConnection, strName, &FindFileData, 0, 0);
	if (!hFind)
	{
		hFind = FtpFindFirstFile(m_hFtpConnection, strNameU, &FindFileData, 0, 0);

		if (!hFind)
		{
			hFind = FtpFindFirstFile(m_hFtpConnection, strNameL, &FindFileData, 0, 0);

			if (!hFind)
				return 0;
		}
	}

	InternetCloseHandle(hFind);

	return FindFileData.nFileSizeLow;
}

bool CLauncherDlg::ArchiveClose()
{
	int berr = false;

	//// ArchiveClose
	try
	{
		m_zip.Close(false);
	}
	catch (CException* e)
	{
		e->Delete();
		berr = true;
	}
	catch (...) // thrown in the STL version
	{
		berr = true;
	}
	if (berr)
	{
		MessageBox(_T("Archive close failed"), _T(""), MB_ICONSTOP);
		return false;
	}

	return true;
	///
}

bool CLauncherDlg::ArchiveOpen(CString OpenFileName)
{
	int berr = 0;

	//// ArchiveOpen
	do
	{
		try
		{
			m_zip.Open(OpenFileName, CZipArchive::open, 1);
			berr = 0;
		}
		catch (CZipException* e)
		{
			if (e->m_iCause == CZipException::cdirNotFound)
				berr = -1;
			else
				berr = 1;
			e->Delete();
		}
		catch (CException* e)
		{
			e->Delete();
			berr = 1;
		}
		// thrown in the STL version
		catch (const CZipException& e)
		{
			if (e.m_iCause == CZipException::cdirNotFound)
				berr = -1;
			else
				berr = 1;
		}
		catch (...)
		{
			berr = 1;
		}
		if (berr == -1)
		{
			if (MessageBox(
					_T("The central directory was not found. If you're opening a multi-disk ")
					_T("archive, make sure you have inserted the last disk. Retry?"),
					_T(""), MB_ICONSTOP | MB_YESNO)
				== IDNO)
				berr = 1;
			else
				m_zip.Close(true);
		}

		if (berr == 1)
		{
			MessageBox(_T("Archive open failed"), _T(""), MB_ICONSTOP);
			return false;
		}
	}
	while (berr == -1);
	////

	return true;
}

bool CLauncherDlg::ArchiveExtract(CString ExtractFolder)
{
	int nZipCount = m_zip.GetNoEntries();

	bool bErr     = false;
	try
	{
		for (int i = 0; i < nZipCount; i++)
		{
			m_zip.ExtractFile((WORD) i, ExtractFolder, true, nullptr); //, Callback, (void*) &p);
		}
	}
	catch (CException* e)
	{
		e->Delete();
		bErr = true;
	}
	catch (...) // thrown in the STL version
	{
		bErr = true;
	}
	if (bErr)
	{
		m_zip.CloseFile(nullptr, true);
		MessageBox(_T("Extract failed"), _T(""), MB_ICONSTOP);
		return false;
	}

	return true;
}

LRESULT CLauncherDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_SOCKETMSG:
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
				case FD_CONNECT:
					TRACE("Socket connected..\n");
					break;
					//			case FD_ACCEPT:
					//				break;
				case FD_CLOSE:
					TRACE("Socket closed..\n");
					break;
				case FD_READ:
				{
					m_pSocket->Receive();

					while (m_pSocket->m_qRecvPkt.size() > 0) // 패킷 리스트에 패킷이 있냐????
					{
						int iOffset         = 0;
						DataPack* pDataPack = m_pSocket->m_qRecvPkt
												  .front(); // 큐의 첫번째 것을 복사..
						this->PacketProcess(
							pDataPack->m_pData, iOffset);   // 패킷을 처리할 상황이 아니다.
						delete pDataPack;
						m_pSocket->m_qRecvPkt.pop();        // 패킷을 큐에서 꺼냄..
					}
				}
				break;
				default:
					ASSERT(0);
					break;
			}
		}
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
