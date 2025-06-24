// DlgBrowsePath.cpp : implementation file
//

#include "stdafx.h"
#include "DlgBrowsePath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
//
// This source is part of CShellTree - Selom Ofori
// 
// Version: 1.02 (any previously unversioned copies are older/inferior
//
// This code is free for all to use. Mutatilate it as much as you want
// See MFCENUM sample from microsoft

/////////////////////////////////////////////////////////////////////////////
// CDlgBrowsePath dialog
//
// This source is part of CShellTree - Selom Ofori
// 
// Version: 1.02 (any previously unversioned copies are older/inferior
//
// This code is free for all to use. Mutatilate it as much as you want
// See MFCENUM sample from microsoft

CDlgBrowsePath::CDlgBrowsePath(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgBrowsePath::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgBrowsePath)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDlgBrowsePath::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgBrowsePath)
	DDX_Control(pDX, IDC_FILELIST, m_CBPath);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgBrowsePath, CDialog)
	//{{AFX_MSG_MAP(CDlgBrowsePath)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_BROWSE_TREE, OnItemexpandingBrowseTree)
	ON_NOTIFY(NM_RCLICK, IDC_BROWSE_TREE, OnRclickBrowseTree)
	ON_NOTIFY(TVN_DELETEITEM, IDC_BROWSE_TREE, OnDeleteitemBrowseTree)
	ON_BN_CLICKED(IDC_SYNCH_FOLDERS, OnSynchFolders)
	ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSE_TREE, OnSelchangedBrowseTree)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBrowsePath message handlers

BOOL CDlgBrowsePath::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 최근에 쓴 폴더를 가져온다...
	char szInitDir[256];
	DWORD dwLength = 256;
	memset(szInitDir, 0, 256);
	CString szKey;
	HKEY hKey;
	DWORD dwKeyType = REG_SZ;
	if(ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, "Recenet used folder", &hKey))
	{
		for(int i = 0; i < 10; i++)
		{
			szKey.Format("Folder%.1d", i);
			LONG success = RegQueryValueEx(hKey, szKey, NULL, &dwKeyType, (BYTE*)szInitDir, &dwLength);
			if(success == ERROR_SUCCESS && lstrlen(szInitDir) != 0)
			{
				m_CBPath.AddString(szInitDir);
			}
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_TreeCtl = (CShellTree*)GetDlgItem(IDC_BROWSE_TREE);
	m_TreeCtl->EnableImages();
	m_TreeCtl->PopulateTree();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDlgBrowsePath::OnPaint() 
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
HCURSOR CDlgBrowsePath::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDlgBrowsePath::OnItemexpandingBrowseTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_TreeCtl->OnFolderExpanding(pNMHDR,pResult);
	*pResult = 0;
}

void CDlgBrowsePath::OnSelchangedBrowseTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	CString szPath;
	if(m_TreeCtl->OnFolderSelected(pNMHDR,pResult,szPath))
	{
		m_CBPath.SetWindowText(szPath);
	}
	*pResult = 0;
}

void CDlgBrowsePath::OnRclickBrowseTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_TreeCtl->GetContextMenu(pNMHDR,pResult);
	*pResult = 0;
}

void CDlgBrowsePath::OnDeleteitemBrowseTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_TreeCtl->OnDeleteShellItem(pNMHDR,pResult);
	*pResult = 0;
}

void CDlgBrowsePath::OnSynchFolders() 
{
	// TODO: Add your control notification handler code here
	CString szFindFolder;
	m_CBPath.GetWindowText(szFindFolder);
	m_TreeCtl->TunnelTree(szFindFolder);
}


void CDlgBrowsePath::OnOK() 
{
	m_CBPath.GetWindowText(m_szPath); // 경로 기억..
	if(m_szPath.GetLength() > 0)
	{
		m_CBPath.InsertString(0, m_szPath); // 경로 넣고...
	}

	// 최근에 쓴 폴더를 저장해둔다....
	char szInitDir[256] = "";
	CString szKey;
	HKEY hKey;
	DWORD dwKeyType = REG_SZ;
	if(ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_USER, "Recenet used folder", &hKey))
	{
		int nCBC = m_CBPath.GetCount();
		if(nCBC > 10) nCBC = 10;
		for(int i = 0; i < nCBC; i++)
		{
			szKey.Format("Folder%.1d", i);
			m_CBPath.GetLBText(i, szInitDir);
			int nLength = m_CBPath.GetLBTextLen(i);
			if(nLength > 0)
			{
				LONG success = RegSetValueEx(hKey, szKey, NULL, dwKeyType, (CONST BYTE*)szInitDir, nLength);
			}
		}
	}
	
	CDialog::OnOK();
}
