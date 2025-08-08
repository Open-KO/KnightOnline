#include "stdafx.h"
#include "TblEditor.h"
#include "TblEditorDoc.h"
#include "TblEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CTblEditorApp
BEGIN_MESSAGE_MAP(CTblEditorApp, CWinApp)
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CTblEditorApp construction
CTblEditorApp::CTblEditorApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_hAccelTable = nullptr;

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	m_pShellManager = new CShellManager();
}

CTblEditorApp::~CTblEditorApp()
{
	delete m_pShellManager;

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif
}

// The one and only CTblEditorApp object
CTblEditorApp theApp;

static BOOL NEAR PASCAL SetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue) 
{ 
	if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ, lpszValue, lstrlen(lpszValue)) != ERROR_SUCCESS)
	{
		TRACE1("Warning: registration database update failed for key '%Fs'\n", lpszKey);
		return FALSE;
	}

	return TRUE;
} 

// CTblEditorApp initialization
BOOL CTblEditorApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("OpenKO TBL Editor"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	m_hAccelTable = LoadAccelerators(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDR_ACCELERATOR));

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CTblEditorDoc),
		RUNTIME_CLASS(CFrameWnd),       // main SDI frame window
		RUNTIME_CLASS(CTblEditorView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	EnableShellOpen();
	RegisterShellFileTypes();

	CString strFileTypeName;

	pDocTemplate->GetDocString(strFileTypeName, CDocTemplate::regFileTypeId);
	SetRegKey(_T(".tbl"), strFileTypeName);

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

// App command to run the dialog
void CTblEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CTblEditorApp::ProcessMessageFilter(
	int code,
	LPMSG lpMsg) 
{
	if (code >= 0
		&& m_pMainWnd != nullptr
		&& m_hAccelTable != nullptr)
	{
		if (TranslateAccelerator(m_pMainWnd->m_hWnd, m_hAccelTable, lpMsg)) 
			return(TRUE);
	}

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}
