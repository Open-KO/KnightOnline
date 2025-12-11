// DlgUnusedFileList.cpp : implementation file
//

#include "stdafx.h"
#include "uie.h"
#include "DlgUnusedFileList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUnusedFileList dialog


CDlgUnusedFileList::CDlgUnusedFileList(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgUnusedFileList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgUnusedFileList)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgUnusedFileList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUnusedFileList)
	DDX_Control(pDX, IDC_LIST_FILES, m_ListFiles);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgUnusedFileList, CDialog)
	//{{AFX_MSG_MAP(CDlgUnusedFileList)
	ON_BN_CLICKED(IDC_B_DELETE_SELECT, OnBDeleteSelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgUnusedFileList message handlers

void CDlgUnusedFileList::OnBDeleteSelect() 
{
	int iSC = m_ListFiles.GetSelCount();
	if(iSC <= 0) return;

	int* piSels = new int[iSC];
	m_ListFiles.GetSelItems(iSC, piSels);
	for(int i = 0; i < iSC; i++)
	{
		CString szPath;
		m_ListFiles.GetText(piSels[i], szPath);
		::DeleteFile(szPath);
	}
	delete piSels;
}

BOOL CDlgUnusedFileList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int iFC = static_cast<int>(m_szFileNames.GetSize());
	for (int i = 0; i < iFC; i++)
		m_ListFiles.AddString(m_szFileNames[i]);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
