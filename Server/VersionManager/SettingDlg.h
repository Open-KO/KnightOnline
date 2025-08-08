#if !defined(AFX_SETTINGDLG_H__14FB04F9_C15B_4AAF_81A1_544508F3F9ED__INCLUDED_)
#define AFX_SETTINGDLG_H__14FB04F9_C15B_4AAF_81A1_544508F3F9ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingDlg.h : header file
//

#include <set>
#include <string>
#include <unordered_map>

#include <ZipArchive/ZipArchive.h>

typedef std::set <int>	HistoryList;

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg dialog
class CVersionManagerDlg;
class CSettingDlg : public CDialog
{
// Construction
public:
	friend class CVersionManagerDlg;
	
	/// \brief determines if a string contains DBCS characters.
	/// \details a DBCS character is a lead byte for the system default Windows ANSI code page (CP_ACP).
	/// A lead byte is the first byte of a two-byte character in a double-byte character set (DBCS) for the code page.
	/// \see https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-isdbcsleadbyte
	bool IsDBCSString(const char* string);

	/// \brief attempts to find files recursively and insert them
	/// \param folderName
	/// \param isTestDBCS if true will check file names for DBCS, and will not insert files found
	/// \see IsDBCSString()
	void FolderRecurse(const TCHAR* folderName, bool isTestDBCS = false);

	/// \brief Attempts to insert a file as a patch
	/// \details Checks if file name is used in the VERSION table.  If found,
	/// attempts to overwrite that record
	bool InsertProcess(const TCHAR* fileName);

	/// \brief attempts to repackage for the provided version number 
	bool Repacking(int version);
	
	/// \brief repackages the versions in the RepackingVersionList
	/// \see RepackingVersionList
	void RepackingHistory();

	/// \brief standard constructor called by VersionManagerDlg::OnVersionSetting().
	/// Constructs using Main->LastVersion
	CSettingDlg(int lastVersion, CWnd* parent = nullptr);   // standard constructor

	// Dialog Data
	//{{AFX_DATA(CSettingDlg)

	/// \brief Dialog identifier
	enum { IDD = IDD_SETTING };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingDlg)

	/// \brief destroys class resources when the window is being closed
	BOOL DestroyWindow() override;

protected:
	/// \brief performs MFC data exchange
	/// \see https://learn.microsoft.com/en-us/cpp/mfc/dialog-data-exchange?view=msvc-170
	void DoDataExchange(CDataExchange* data) override;    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

	/// \brief clears the contents of FileListTextArea and reloads the default content
	void RefreshFileListTextArea();

	// Generated message map functions
	//{{AFX_MSG(CSettingDlg)

	/// \brief triggered when the Add button is clicked.  Opens a file picker and processes the selection
	afx_msg void OnAddFile();

	/// \brief triggered when the delete button is clicked. Attempts to delete the
	/// file selected in the FileListTextArea
	afx_msg void OnDeleteFile();

	/// \brief triggered when the Compress button is clicked. Attempts to
	/// compress the selected entry, or all entries if CompressCheckbox is
	/// checked
	/// \see CompressCheckbox
	afx_msg void OnCompress();

	/// \brief closes the settings dialog window when the "OK" button is clicked
	virtual void OnOK();
	
	/// \brief initializes the version settings dialogue box
	BOOL OnInitDialog() override;

	/// \brief opens a folder picker dialog box
	afx_msg void OnPathBrowse();

	/// \brief reloads the contents of FileListTextArea. Triggered by
	/// the Refresh button and when focus leaves VersionInput
	/// \see FileListTextArea
	/// \see VersionInput
	afx_msg void OnRefresh();

	/// \brief triggered when the DBCS Test button is clicked. Recursively checks
	/// all the file names in the Base Directory for DBCS characters
	/// \see IsDBCSString()
	afx_msg void OnDbcstest();
	
	//}}AFX_MSG

	// event mapping macro
	DECLARE_MESSAGE_MAP()

protected:
	/// \brief Base Path text input box in the configuration section
	CEdit				_basePathInput;

	/// \brief File List text area box in the File Compress section
	CListBox			_fileListTextArea;

	/// \brief Progress Bar that appears below the text area
	CProgressCtrl		_progressBar;

	/// \brief binds to the version input box in the configuration section
	int					_lastVersion;

	/// \brief "CurrentVer." checkbox in the File Compress section
	/// \details Causes RepackingVersionList to be built if empty
	/// \todo update corresponding label to something more useful
	BOOL				_isRepackage;

	/// \brief "All File Add" checkbox in the File Compress section
	/// \details causes the Add button to add all files in the BasePath
	/// directory to the database as patches
	BOOL				_isAddAllFiles;

	//}}AFX_DATA

	/// \brief Default path copied from _main->_defaultPath on SettingDlg create
	TCHAR				_defaultPath[_MAX_PATH];

	/// \brief reference back to the main instance of VersionManagerDlg
	CVersionManagerDlg* _main;

	/// \brief zip file management class
	CZipArchive			_zipArchive;

	/// \brief versions marked as packaged
	HistoryList			_repackingVersionList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGDLG_H__14FB04F9_C15B_4AAF_81A1_544508F3F9ED__INCLUDED_)
