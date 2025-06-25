// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "versionmanager.h"
#include "SettingDlg.h"
#include "DlgBrowsePath.h"
#include "VersionManagerDlg.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(_UNICODE)
using PathType = std::wstring;
#else
using PathType = std::string;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg dialog

CSettingDlg::CSettingDlg(int version, CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingDlg)
	m_nVersion = version;
	m_bCompressOption = FALSE;
	m_bAllFileAdd = FALSE;
	//}}AFX_DATA_INIT

	memset(m_strDefaultPath, 0, sizeof(m_strDefaultPath));

	m_pMain = (CVersionManagerDlg*) pParent;
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingDlg)
	DDX_Control(pDX, IDC_PATH_EDIT, m_PathEdit);
	DDX_Control(pDX, IDC_FILE_LIST, m_FileList);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Text(pDX, IDC_VERSION_EDIT, m_nVersion);
	DDX_Check(pDX, IDC_CHECK, m_bCompressOption);
	DDX_Check(pDX, IDC_CHECK2, m_bAllFileAdd);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingDlg)
	ON_BN_CLICKED(IDC_ADDFILE, OnAddfile)
	ON_BN_CLICKED(IDC_DELETEFILE, OnDeletefile)
	ON_BN_CLICKED(IDC_COMPRESS, OnCompress)
	ON_BN_CLICKED(IDC_PATH_BROWSE, OnPathBrowse)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_EN_KILLFOCUS(IDC_VERSION_EDIT, OnKillfocusVersionEdit)
	ON_BN_CLICKED(IDC_DBCSTEST, OnDbcstest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg message handlers

BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_PathEdit.SetWindowText(m_strDefaultPath);

	m_Progress.SetRange(0, 100);
	m_Progress.SetPos(0);

	OnRefresh();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingDlg::OnOK()
{
	CDialog::OnOK();
}

void CSettingDlg::OnAddfile()
{
	constexpr DWORD FilenameBufferSize = 512000;

	CFileDialog dlg(TRUE);
	PathType addfilename, addfullpath, defaultpath;
	TCHAR tempstr1[_MAX_PATH] = {};
	std::vector<TCHAR> szFileName(FilenameBufferSize);
	int strsize = 0;

	UpdateData(TRUE);

	if (m_bAllFileAdd)
	{
		if (AfxMessageBox(_T("All files of Base Path will be inserted."), MB_OKCANCEL) == IDCANCEL)
			return;

		BeginWaitCursor();
		FolderRecurse(m_strDefaultPath);
		EndWaitCursor();
		return;
	}

	::SetCurrentDirectory(m_strDefaultPath);

	dlg.m_ofn.lpstrInitialDir = m_strDefaultPath;
	dlg.m_ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	dlg.m_ofn.lpstrFile = &szFileName[0];
	dlg.m_ofn.nMaxFile = FilenameBufferSize;

	if (dlg.DoModal() != IDOK)
		return;

	POSITION pos = dlg.GetStartPosition();
	while (pos != nullptr)
	{
		_tcscpy(tempstr1, dlg.GetNextPathName(pos));
		addfullpath = _tcslwr(tempstr1);
		defaultpath = _tcslwr(m_strDefaultPath);
		strsize = defaultpath.size();
		addfilename = addfullpath.substr(strsize);

		if (InsertProcess(&addfilename[0]))
			m_FileList.AddString(&addfilename[0]);
	}
}

void CSettingDlg::OnDeletefile()
{
	int selcount = 0;
	CString delfilename, compname, errmsg;
	std::string			filename;
	std::vector<int>	sellist;
	_VERSION_INFO* pInfo = nullptr;

	selcount = m_FileList.GetSelCount();
	if (selcount == 0)
	{
		AfxMessageBox(_T("File Not Selected."));
		return;
	}

	BeginWaitCursor();

	sellist.reserve(selcount);

	m_FileList.GetSelItems(selcount, &sellist[0]);

	for (int i = 0; i < selcount; i++)
	{
		m_FileList.GetText(sellist[i], delfilename);
		filename = CT2A(delfilename);

		pInfo = m_pMain->m_VersionList.GetData(filename);
		if (pInfo == nullptr)
			continue;

		if (!m_pMain->m_DBProcess.DeleteVersion(filename.c_str()))
		{
			errmsg.Format(_T("%hs DB Delete Fail"), filename.c_str());
			AfxMessageBox(errmsg);
			return;
		}

		// Restore
		if (pInfo->sHistoryVersion > 0)
		{
			pInfo->sVersion = pInfo->sHistoryVersion;
			pInfo->sHistoryVersion = 0;

			compname.Format(_T("patch%.4d.zip"), pInfo->sVersion);
			if (!m_pMain->m_DBProcess.InsertVersion(pInfo->sVersion, filename.c_str(), CT2A(compname), 0))
			{
				m_pMain->m_VersionList.DeleteData(filename);
				errmsg.Format(_T("%hs DB Insert Fail"), filename.c_str());
				AfxMessageBox(errmsg);
				return;
			}
		}
		else
		{
			m_pMain->m_VersionList.DeleteData(filename);
		}

		Sleep(10);
	}

	EndWaitCursor();

	OnRefresh();
}

void CSettingDlg::OnCompress()
{
	CString			fullpathname, filename, errmsg, compname, compfullpath;
	DWORD dwsize;
	CFile file;
	std::string		addfilename;
	_VERSION_INFO* pInfo = nullptr;

	UpdateData(TRUE);

	int count = m_FileList.GetCount();
	if (count == 0)
		return;

	BeginWaitCursor();

	compname.Format(_T("patch%.4d.zip"), m_nVersion);
	compfullpath.Format(
		_T("%s\\%s"),
		m_strDefaultPath,
		compname.GetString());

	m_RepackingVersionList.clear();

	m_ZipArchive.Open(compfullpath, CZipArchive::create);

	SetDlgItemText(IDC_STATUS, _T("Compressing.."));

	for (int i = 0; i < count; i++)
	{
		m_FileList.GetText(i, filename);

		fullpathname = m_strDefaultPath;
		fullpathname += filename;
		if (!file.Open(fullpathname, CFile::modeRead))
		{
			errmsg.Format(_T("%s File Open Fail"), filename.GetString());
			AfxMessageBox(errmsg);
			continue;
		}

		dwsize = static_cast<DWORD>(file.GetLength());
		file.Close();

		if (!m_ZipArchive.AddNewFile(fullpathname, m_strDefaultPath, -1, dwsize))
		{
			errmsg.Format(_T("%s File Compress Fail"), filename.GetString());
			AfxMessageBox(errmsg);
			continue;
		}

		m_Progress.SetPos(i * 100 / count);

		addfilename = CT2A(filename);

		pInfo = m_pMain->m_VersionList.GetData(addfilename);
		if (pInfo != nullptr)
			m_RepackingVersionList.insert(pInfo->sHistoryVersion);
	}

	SetDlgItemText(IDC_STATUS, _T("Compressed"));

	m_ZipArchive.Close();

	// Current Version 만 압축
	if (!m_bCompressOption)
	{
		if (!m_RepackingVersionList.empty())
			RepackingHistory();
	}

	m_Progress.SetPos(100);

	EndWaitCursor();
}

void CSettingDlg::OnPathBrowse()
{
	CDlgBrowsePath pathdlg;
	if (pathdlg.DoModal() != IDOK)
		return;

	_tcscpy(m_strDefaultPath, pathdlg.m_szPath);
	m_PathEdit.SetWindowText(m_strDefaultPath);
}

void CSettingDlg::OnRefresh()
{
	UpdateData(TRUE);

	m_FileList.ResetContent();

	for (const auto& [_, pInfo] : m_pMain->m_VersionList)
	{
		if (pInfo->sVersion == m_nVersion)
			m_FileList.AddString(CA2T(pInfo->strFileName.c_str()));
	}
}

BOOL CSettingDlg::DestroyWindow()
{
	m_RepackingVersionList.clear();
	return CDialog::DestroyWindow();
}

void CSettingDlg::RepackingHistory()
{
	CString errmsg;

	SetDlgItemText(IDC_STATUS, _T("Repacking..."));

	for (const int version : m_RepackingVersionList)
	{
		if (!Repacking(version))
		{
			errmsg.Format(_T("%d Repacking Fail"), version);
			AfxMessageBox(errmsg);
		}
	}

	SetDlgItemText(IDC_STATUS, _T("Repacked"));
}

bool CSettingDlg::Repacking(int version)
{
	CString			filename, errmsg, compname, compfullpath;
	DWORD dwsize;
	CFile file;

	compname.Format(_T("patch%.4d.zip"), version);
	compfullpath.Format(_T("%s\\%s"), m_strDefaultPath, compname.GetString());

	m_ZipArchive.Open(compfullpath, CZipArchive::create);

	for (const auto& [_, pInfo] : m_pMain->m_VersionList)
	{
		if (pInfo->sVersion != version)
			continue;

		filename.Format(_T("%s%hs"), m_strDefaultPath, pInfo->strFileName.c_str());
		if (!file.Open(filename, CFile::modeRead))
		{
			errmsg.Format(_T("%s File Open Fail"), filename.GetString());
			AfxMessageBox(errmsg);
			continue;
		}

		dwsize = static_cast<DWORD>(file.GetLength());
		file.Close();

		if (!m_ZipArchive.AddNewFile(filename, m_strDefaultPath, -1, dwsize))
		{
			errmsg.Format(_T("%s File Compress Fail"), filename.GetString());
			AfxMessageBox(errmsg);
			return false;
		}
	}

	m_ZipArchive.Close();

	return true;
}

bool CSettingDlg::InsertProcess(const TCHAR* filename)
{
	_VERSION_INFO* pInfo1 = nullptr, *pInfo2 = nullptr;
	CString compname, errmsg;
	std::string		addfilename;
	int historyversion = 0;

	addfilename = CT2A(filename);

	if (IsDBCSString(addfilename.c_str()))
	{
		errmsg.Format(_T("%s include DBCS character"), filename);
		AfxMessageBox(errmsg);
		return false;
	}

	compname.Format(_T("patch%.4d.zip"), m_nVersion);

	pInfo1 = m_pMain->m_VersionList.GetData(addfilename);
	if (pInfo1 != nullptr)
	{
		historyversion = pInfo1->sVersion;
		m_pMain->m_VersionList.DeleteData(addfilename);

		if (!m_pMain->m_DBProcess.DeleteVersion(addfilename.c_str()))
		{
			errmsg.Format(_T("%hs DB Delete Fail"), addfilename.c_str());
			AfxMessageBox(errmsg);
			return false;
		}
	}

	pInfo2 = new _VERSION_INFO;
	pInfo2->sVersion = m_nVersion;
	pInfo2->strFileName = addfilename;
	pInfo2->strCompName = CT2A(compname);
	pInfo2->sHistoryVersion = historyversion;
	if (!m_pMain->m_VersionList.PutData(addfilename, pInfo2))
	{
		delete pInfo2;
		return false;
	}

	if (!m_pMain->m_DBProcess.InsertVersion(
		m_nVersion,
		addfilename.c_str(),
		pInfo2->strCompName.c_str(),
		historyversion))
	{
		m_pMain->m_VersionList.DeleteData(addfilename);

		errmsg.Format(_T("%hs DB Insert Fail"), addfilename.c_str());
		AfxMessageBox(errmsg);
		return false;
	}

	return true;
}

void CSettingDlg::OnKillfocusVersionEdit()
{
	OnRefresh();
}

void CSettingDlg::FolderRecurse(const TCHAR* foldername, bool b_test)
{
	CFileFind ff;
	PathType addfilename, addfullpath, defaultpath;
	TCHAR tempstr1[_MAX_PATH] = {};
	int strsize = 0;

	::SetCurrentDirectory(foldername);

	BOOL bFind = ff.FindFile();
	while (bFind)
	{
		bFind = ff.FindNextFile();
		_tcscpy(tempstr1, ff.GetFilePath());

		if (ff.IsDots())
			continue;

		if (ff.IsDirectory())
		{
			FolderRecurse(tempstr1, b_test);
			continue;
		}

		// 단순 검사만 수행...
		if (b_test)
		{
			if (IsDBCSString(CT2A(tempstr1)))
			{
				CString errmsg;
				errmsg.Format(_T("%s include DBCS character"), tempstr1);
				AfxMessageBox(errmsg);
			}

			continue;
		}

		addfullpath = _tcslwr(tempstr1);
		defaultpath = _tcslwr(m_strDefaultPath);
		strsize = defaultpath.size();
		addfilename = addfullpath.substr(strsize);

		if (InsertProcess(&addfilename[0]))
			m_FileList.AddString(&addfilename[0]);
	}
}

bool CSettingDlg::IsDBCSString(const char* string)
{
	int total_count = static_cast<int>(strlen(string));
	for (int i = 0; i < total_count; i++)
	{
		if (IsDBCSLeadByte(string[i++]))
			return true;
	}

	return false;
}

void CSettingDlg::OnDbcstest()
{
	FolderRecurse(m_strDefaultPath, true);
	AfxMessageBox(_T("Test Done.."));
}
