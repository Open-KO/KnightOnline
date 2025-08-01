// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "versionmanager.h"
#include "SettingDlg.h"
#include "DlgBrowsePath.h"
#include "VersionManagerDlg.h"

#include <string>

import VersionManagerModel;
namespace model = versionmanager_model;

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

/// \brief standard constructor called by VersionManagerDlg::OnVersionSetting().
/// Constructs using _main->LastVersion
CSettingDlg::CSettingDlg(int lastVersion, CWnd* parent /*=NULL*/)
	: CDialog(IDD, parent)
{
	//{{AFX_DATA_INIT(CSettingDlg)
	_lastVersion = lastVersion;
	_isRepackage = FALSE;
	_isAddAllFiles = FALSE;
	//}}AFX_DATA_INIT

	memset(_defaultPath, 0, sizeof(_defaultPath));

	_main = (CVersionManagerDlg*) parent;
}

/// \brief performs MFC data exchange
/// \see https://learn.microsoft.com/en-us/cpp/mfc/dialog-data-exchange?view=msvc-170
void CSettingDlg::DoDataExchange(CDataExchange* data)
{
	CDialog::DoDataExchange(data);
	//{{AFX_DATA_MAP(CSettingDlg)
	DDX_Control(data, IDC_PATH_EDIT, _basePathInput);
	DDX_Control(data, IDC_FILE_LIST, _fileListTextArea);
	DDX_Control(data, IDC_PROGRESS, _progressBar);
	DDX_Text(data, IDC_VERSION_EDIT, _lastVersion);
	DDX_Check(data, IDC_CHECK, _isRepackage);
	DDX_Check(data, IDC_CHECK2, _isAddAllFiles);
	//}}AFX_DATA_MAP
}

/// \brief event mapping
BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingDlg)
	ON_BN_CLICKED(IDC_ADDFILE, OnAddFile)
	ON_BN_CLICKED(IDC_DELETEFILE, OnDeleteFile)
	ON_BN_CLICKED(IDC_COMPRESS, OnCompress)
	ON_BN_CLICKED(IDC_PATH_BROWSE, OnPathBrowse)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_DBCSTEST, OnDbcstest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg message handlers

/// \brief initializes the version settings dialogue box
BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_basePathInput.SetWindowText(_defaultPath);

	_progressBar.SetRange(0, 100);
	_progressBar.SetPos(0);

	OnRefresh();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

/// \brief closes the settings dialog window when the "OK" button is clicked
void CSettingDlg::OnOK()
{
	CDialog::OnOK();
}

/// \brief triggered when the Add button is clicked.  Opens a file picker and processes the selection
void CSettingDlg::OnAddFile()
{
	constexpr DWORD FilenameBufferSize = 512000;

	CFileDialog dlg(TRUE);
	PathType fileName, filePath, defaultPath;
	TCHAR tempstr1[_MAX_PATH] = {};
	std::vector<TCHAR> szFileName(FilenameBufferSize);
	int strSize = 0;

	UpdateData(TRUE);

	if (_isAddAllFiles)
	{
		if (AfxMessageBox(_T("All files of Base Path will be inserted."), MB_OKCANCEL) == IDCANCEL)
			return;

		BeginWaitCursor();
		FolderRecurse(_defaultPath);
		EndWaitCursor();
		return;
	}

	::SetCurrentDirectory(_defaultPath);

	dlg.m_ofn.lpstrInitialDir = _defaultPath;
	dlg.m_ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	dlg.m_ofn.lpstrFile = &szFileName[0];
	dlg.m_ofn.nMaxFile = FilenameBufferSize;

	if (dlg.DoModal() != IDOK)
		return;

	POSITION pos = dlg.GetStartPosition();
	while (pos != nullptr)
	{
		_tcscpy(tempstr1, dlg.GetNextPathName(pos));
		filePath = _tcslwr(tempstr1);
		defaultPath = _tcslwr(_defaultPath);
		strSize = defaultPath.size();
		fileName = filePath.substr(strSize);

		InsertProcess(&fileName[0]);
	}
}

/// \brief triggered when the delete button is clicked. Attempts to delete the
/// file selected in the _fileListTextArea
void CSettingDlg::OnDeleteFile()
{
	CString delFileName, compressName, errMsg;
	std::string			fileName;
	std::vector<int>	selList;
	model::Version* delVersion = nullptr;

	int selCount = _fileListTextArea.GetSelCount();
	if (selCount == 0)
	{
		AfxMessageBox(_T("File Not Selected."));
		return;
	}

	BeginWaitCursor();

	selList.reserve(selCount);
	for (int i = 0; i < _fileListTextArea.GetCount(); i++)
	{
		if (_fileListTextArea.GetSel(i))
		{
			selList.push_back(i);
		}
	}

	for (int i = 0; i < selCount; i++)
	{
		_fileListTextArea.GetText(selList[i], delFileName);
		fileName = CT2A(delFileName);

		int delVersionId = -1;
		for (const auto& version : _main->VersionList)
		{
			if (version.second->FileName == fileName)
			{
				delVersionId = version.second->Number;
				break;
			}
		}

		delVersion = _main->VersionList.GetData(delVersionId);
		if (delVersion == nullptr)
			continue;

		if (!_main->DbProcess.DeleteVersion(delVersionId))
		{
			errMsg.Format(_T("%hs DB Delete Fail"), fileName.c_str());
			AfxMessageBox(errMsg);
			return;
		}

		// Restore
		if (delVersion->HistoryVersion > 0)
		{
			delVersion->Number = delVersion->HistoryVersion;
			delVersion->HistoryVersion = 0;

			compressName.Format(_T("patch%.4d.zip"), delVersion->Number);
			if (!_main->DbProcess.InsertVersion(delVersion->Number, fileName.c_str(), CT2A(compressName), 0))
			{
				_main->VersionList.DeleteData(delVersionId);
				errMsg.Format(_T("%hs DB Insert Fail"), fileName.c_str());
				AfxMessageBox(errMsg);
				return;
			}
		}
		else
		{
			_main->VersionList.DeleteData(delVersionId);
		}

		Sleep(10);
	}

	EndWaitCursor();

	OnRefresh();
}

/// \brief triggered when the Compress button is clicked. Attempts to
/// compress the selected entry, or all entries if CompressCheckbox is
/// checked
/// \see CompressCheckbox
void CSettingDlg::OnCompress()
{
	CString			pathName, fileName, errMsg, compressName, compressPath;
	DWORD size;
	CFile file;
	std::string		fileNameA;

	UpdateData(TRUE);

	int count = _fileListTextArea.GetCount();
	if (count == 0)
		return;

	BeginWaitCursor();

	compressName.Format(_T("patch%.4d.zip"), _lastVersion);
	compressPath.Format(
		_T("%s\\%s"),
		_defaultPath,
		compressName.GetString());

	_repackingVersionList.clear();

	_zipArchive.Open(compressPath, CZipArchive::create);

	SetDlgItemText(IDC_STATUS, _T("Compressing.."));

	for (int i = 0; i < count; i++)
	{
		_fileListTextArea.GetText(i, fileName);

		pathName = _defaultPath;
		pathName += fileName;
		if (!file.Open(pathName, CFile::modeRead))
		{
			errMsg.Format(_T("%s File Open Fail"), fileName.GetString());
			AfxMessageBox(errMsg);
			continue;
		}

		size = static_cast<DWORD>(file.GetLength());
		file.Close();

		if (!_zipArchive.AddNewFile(pathName, _defaultPath, -1, size))
		{
			errMsg.Format(_T("%s File Compress Fail"), fileName.GetString());
			AfxMessageBox(errMsg);
			continue;
		}

		_progressBar.SetPos(i * 100 / count);

		fileNameA = CT2A(fileName);

		int versionKey = -1;
		for (const auto& version : _main->VersionList)
		{
			if (version.second->FileName == fileNameA)
			{
				versionKey = version.second->Number;
				break;
			}
		}

		model::Version* pInfo = _main->VersionList.GetData(versionKey);
		if (pInfo != nullptr)
			_repackingVersionList.insert(pInfo->HistoryVersion);
	}

	SetDlgItemText(IDC_STATUS, _T("Compressed"));

	_zipArchive.Close();

	// If unchecked, try repacking all items in history
	if (!_isRepackage)
	{
		if (!_repackingVersionList.empty())
			RepackingHistory();
	}

	_progressBar.SetPos(100);

	EndWaitCursor();
}

/// \brief opens a folder picker dialog box
void CSettingDlg::OnPathBrowse()
{
	CDlgBrowsePath pathdlg;
	if (pathdlg.DoModal() != IDOK)
		return;

	_tcscpy_s(_defaultPath, pathdlg.m_szPath);
	_basePathInput.SetWindowText(_defaultPath);
}

/// \brief reloads the contents of _fileListTextArea. Triggered by
/// the Refresh button and when focus leaves VersionInput
/// \see _fileListTextArea
/// \see VersionInput
void CSettingDlg::OnRefresh()
{
	RefreshFileListTextArea();
	UpdateData(TRUE);
}

/// \brief destroys class resources when the window is being closed
BOOL CSettingDlg::DestroyWindow()
{
	_repackingVersionList.clear();
	return CDialog::DestroyWindow();
}

/// \brief repackages the versions in the RepackingVersionList
/// \see RepackingVersionList
void CSettingDlg::RepackingHistory()
{
	CString errMsg;

	SetDlgItemText(IDC_STATUS, _T("Repacking..."));

	for (const int version : _repackingVersionList)
	{
		if (!Repacking(version))
		{
			errMsg.Format(_T("%d Repacking Fail"), version);
			AfxMessageBox(errMsg);
		}
	}

	SetDlgItemText(IDC_STATUS, _T("Repacked"));
}

/// \brief attempts to repackage for the provided version number 
bool CSettingDlg::Repacking(int version)
{
	CString			filename, errmsg, compname, compfullpath;
	CFile file;

	compname.Format(_T("patch%.4d.zip"), version);
	compfullpath.Format(_T("%s\\%s"), _defaultPath, compname.GetString());

	_zipArchive.Open(compfullpath, CZipArchive::create);

	model::Version* pInfo = _main->VersionList.GetData(version);
	if (pInfo == nullptr)
	{
		errmsg.Format(_T("%d is not a valid version number"), version);
		AfxMessageBox(errmsg);
		_zipArchive.Close(true);
		return false;
	}

	filename.Format(_T("%s%hs"), _defaultPath, pInfo->FileName.c_str());
	if (!file.Open(filename, CFile::modeRead))
	{
		errmsg.Format(_T("%s File Open Fail"), filename.GetString());
		AfxMessageBox(errmsg);
		_zipArchive.Close(true);
		return false;
	}

	DWORD dwsize = static_cast<DWORD>(file.GetLength());
	file.Close();

	if (!_zipArchive.AddNewFile(filename, _defaultPath, -1, dwsize))
	{
		errmsg.Format(_T("%s File Compress Fail"), filename.GetString());
		AfxMessageBox(errmsg);
		_zipArchive.Close(true);
		return false;
	}

	_zipArchive.Close();

	return true;
}

/// \brief Attempts to insert a file as a patch
/// \details Checks if file name is used in the VERSION table.  If found,
/// attempts to overwrite that record
bool CSettingDlg::InsertProcess(const TCHAR* fileName)
{
	model::Version* pInfo1 = nullptr, *pInfo2 = nullptr;
	CString compressName, errMsg;
	std::string		_fileName;
	int historyVersion = 0;

	_fileName = CT2A(fileName);

	if (IsDBCSString(_fileName.c_str()))
	{
		errMsg.Format(_T("%s include DBCS character"), fileName);
		AfxMessageBox(errMsg);
		return false;
	}

	compressName.Format(_T("patch%.4d.zip"), _lastVersion);

	pInfo1 = _main->VersionList.GetData(_lastVersion);
	if (pInfo1 != nullptr)
	{
		historyVersion = pInfo1->HistoryVersion;
		if (!_main->DbProcess.DeleteVersion(_lastVersion))
		{
			errMsg.Format(_T("%hs DB Delete Fail"), _fileName.c_str());
			AfxMessageBox(errMsg);
			return false;
		}
		_main->VersionList.DeleteData(_lastVersion);
	}

	pInfo2 = new model::Version();
	pInfo2->Number = _lastVersion;
	pInfo2->FileName = _fileName;
	pInfo2->CompressName = CT2A(compressName);
	pInfo2->HistoryVersion = historyVersion;
	if (!_main->VersionList.PutData(_lastVersion, pInfo2))
	{
		delete pInfo2;
		return false;
	}

	if (!_main->DbProcess.InsertVersion(
		_lastVersion,
		_fileName.c_str(),
		pInfo2->CompressName.c_str(),
		historyVersion))
	{
		_main->VersionList.DeleteData(_lastVersion);

		errMsg.Format(_T("%hs DB Insert Fail"), _fileName.c_str());
		AfxMessageBox(errMsg);
		return false;
	}

	RefreshFileListTextArea();

	return true;
}

/// \brief attempts to find files recursively and insert them
/// \param folderName
/// \param isTestDBCS if true will check file names for DBCS, and will not insert files found
/// /// \see IsDBCSString()
void CSettingDlg::FolderRecurse(const TCHAR* folderName, bool isTestDBCS)
{
	CFileFind ff;
	PathType fileName, fullPath, defaultPath;
	TCHAR tempStr[_MAX_PATH] = {};

	::SetCurrentDirectory(folderName);

	BOOL bFind = ff.FindFile();
	while (bFind)
	{
		bFind = ff.FindNextFile();
		_tcscpy_s(tempStr, ff.GetFilePath());

		if (ff.IsDots())
			continue;

		if (ff.IsDirectory())
		{
			FolderRecurse(tempStr, isTestDBCS);
			continue;
		}

		// Test if the file includes a DBCS character, but don't attempt to insert
		if (isTestDBCS)
		{
			if (IsDBCSString(CT2A(tempStr)))
			{
				CString errmsg;
				errmsg.Format(_T("%s include DBCS character"), tempStr);
				AfxMessageBox(errmsg);
			}

			continue;
		}

		fullPath = _tcslwr(tempStr);
		defaultPath = _tcslwr(_defaultPath);
		fileName = fullPath.substr(defaultPath.size());

		InsertProcess(&fileName[0]);
	}
}

/// \brief determines if a string contains DBCS characters.
/// \details a DBCS character is a lead byte for the system default Windows ANSI code page (CP_ACP).
/// A lead byte is the first byte of a two-byte character in a double-byte character set (DBCS) for the code page.
/// \see https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-isdbcsleadbyte
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

/// \brief triggered when the DBCS Test button is clicked. Recursively checks
/// all the file names in the Base Directory for DBCS characters
/// \see IsDBCSString()
void CSettingDlg::OnDbcstest()
{
	FolderRecurse(_defaultPath, true);
	AfxMessageBox(_T("Test Done.."));
}

/// \brief clears the contents of _fileListTextArea and reloads the default content
void CSettingDlg::RefreshFileListTextArea()
{
	_fileListTextArea.ResetContent();
	_lastVersion = 0;
	for (const auto& [_, pInfo] : _main->VersionList)
	{
		if (pInfo->Number > _lastVersion)
		{
			_lastVersion = pInfo->Number;
		}
		_fileListTextArea.AddString(CA2T(pInfo->FileName.c_str()));
	}

	if (_lastVersion > _main->LastVersion())
	{
		_main->SetLastVersion(_lastVersion);
	}
	SetDlgItemInt(IDC_VERSION_EDIT, _lastVersion);
}
