// DlgChangeImage.cpp : implementation file
//

#include "stdafx.h"
#include "uie.h"
#include "DlgChangeImage.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgChangeImage dialog


CDlgChangeImage::CDlgChangeImage(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgChangeImage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgChangeImage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgChangeImage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgChangeImage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgChangeImage, CDialog)
	//{{AFX_MSG_MAP(CDlgChangeImage)
	ON_BN_CLICKED(IDC_B_BROWSE_OLD_FILE, OnBBrowseOldFile)
	ON_BN_CLICKED(IDC_B_BROWSE_NEW_FILE, OnBBrowseNewFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChangeImage message handlers

void CDlgChangeImage::OnBBrowseOldFile() 
{
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(TRUE, "dxt", nullptr, dwFlags, "Generic Image Files(*.dxt, *.bmp, *.tga)|*.dxt;*.bmp;*.tga;||", nullptr);

	if (IDCANCEL == dlg.DoModal()) return;
	CString szFN_Old = dlg.GetPathName();
	szFN_Old.MakeLower(); // 소문자로 만들고..

	CString szPath = CN3Base::PathGet().c_str();
	szPath.MakeLower();
	int i = szFN_Old.Find(szPath);
	if(i >= 0) szFN_Old = szFN_Old.Mid(i + szPath.GetLength());

	SetDlgItemText(IDC_E_PATH_OLD, szFN_Old);
}

void CDlgChangeImage::OnBBrowseNewFile() 
{
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(TRUE, "dxt", nullptr, dwFlags, "Generic Image Files(*.dxt, *.bmp, *.tga)|*.dxt;*.bmp;*.tga;||", nullptr);

	if (IDCANCEL == dlg.DoModal()) return;
	CString szFN_New = dlg.GetPathName();
	szFN_New.MakeLower(); // 소문자로 만들고..

	CString szPath = CN3Base::PathGet().c_str();
	szPath.MakeLower(); // 소문자로 만든다
	int i = szFN_New.Find(szPath);
	if(i >= 0) szFN_New = szFN_New.Mid(i + szPath.GetLength());

	SetDlgItemText(IDC_E_PATH_NEW, szFN_New);
}

void CDlgChangeImage::OnOK() 
{
	// TODO: Add extra validation here

	GetDlgItemText(IDC_E_PATH_OLD, m_szFN_Old);
	GetDlgItemText(IDC_E_PATH_NEW, m_szFN_New);
	
	CDialog::OnOK();
}
