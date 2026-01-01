// DlgFolderSelect.cpp : implementation file
//

#include "stdafx.h"
#include "DlgFolderSelect.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFolderSelect dialog


CDlgFolderSelect::CDlgFolderSelect(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFolderSelect::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgFolderSelect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgFolderSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFolderSelect)
	DDX_Control(pDX, IDC_BROWSE_TREE, m_DirTree);
	DDX_Control(pDX, IDC_FOLDER_LIST, m_CBFolder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFolderSelect, CDialog)
	//{{AFX_MSG_MAP(CDlgFolderSelect)
	ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSE_TREE, OnSelchangedBrowseTree)
	ON_BN_CLICKED(IDC_TUNNEL_TREE, OnTunnelTree)
	ON_CBN_SELCHANGE(IDC_FOLDER_LIST, OnSelchangeFolderList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFolderSelect message handlers

BOOL CDlgFolderSelect::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// 최근에 쓴 폴더를 가져온다...
	char szInitDir[256];
	DWORD dwLength = 256;
	memset(szInitDir, 0, 256);
	CString szKey;
	HKEY hKey;
	DWORD dwKeyType = REG_SZ;
	if(ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, "N3D Project", &hKey))
	{
		for(int i = 0; i < 10; i++)
		{
			szKey.Format("RecentFolder%.1d", i);
			LONG success = RegQueryValueEx(hKey, szKey, NULL, &dwKeyType, (BYTE*)szInitDir, &dwLength);
			if(success == ERROR_SUCCESS && lstrlen(szInitDir) != 0)
			{
				m_CBFolder.AddString(szInitDir);
			}
		}
	}

	CString szDir;
	char szCurPath[256];
	GetCurrentDirectory(256, szCurPath);
	int nSel = m_CBFolder.AddString(szCurPath);
	m_CBFolder.SetCurSel(nSel);
	m_CBFolder.GetLBText(nSel, szDir);

	m_DirTree.Initialize();
	m_DirTree.SetPath(szDir);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFolderSelect::OnSelchangedBrowseTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// 선택이 바뀌면... 파일 리스트를 바꾸어준다..
	HTREEITEM hItem = m_DirTree.GetSelectedItem();
	CString Path = m_DirTree.GetFullPath(hItem);

	Path = Path.Left(Path.GetLength() - 1);
	m_FolderName = Path;
	m_CBFolder.SetWindowText(Path);

	*pResult = 0;
}

void CDlgFolderSelect::OnTunnelTree() 
{
	// TODO: Add your control notification handler code here
	CString szFindFolder;
	m_CBFolder.GetWindowText(szFindFolder);
	m_DirTree.SetPath(szFindFolder);
}

void CDlgFolderSelect::OnOK() 
{
	// TODO: Add extra validation here
	// 최근에 쓴 폴더를 저장해둔다....
	char szInitDir[256] = "";
	DWORD dwLength = 256;
	CString szKey;
	HKEY hKey;
	DWORD dwKeyType = REG_SZ;
	if(ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_USER, "N3D Project", &hKey))
	{
		int nCBC = m_CBFolder.GetCount();
		for(int i = 0; i < nCBC; i++)
		{
			szKey.Format("RecentFolder%.1d", i);
			m_CBFolder.GetLBText(i, szInitDir);
			dwLength = m_CBFolder.GetLBTextLen(i);
			LONG success = RegSetValueEx(hKey, szKey, NULL, dwKeyType, (CONST BYTE*)szInitDir, dwLength);
		}
	}
	
	m_CBFolder.GetWindowText(m_FolderName);

	CDialog::OnOK();
}

void CDlgFolderSelect::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDlgFolderSelect::SetInitFolderName(CString& FolderName)
{
	m_FolderName = FolderName;
}

CString CDlgFolderSelect::GetFolderName()
{
	return m_FolderName;
}

void CDlgFolderSelect::OnSelchangeFolderList() 
{
	CString Path;
	m_CBFolder.GetWindowText(Path);
	m_DirTree.SetPath(Path);
}
