// DlgNewFileName.cpp : implementation file
//

#include "stdafx.h"
#include "N3FXE.h"
#include "DlgNewFileName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgNewFileName dialog

CDlgNewFileName::CDlgNewFileName(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgNewFileName::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgNewFileName)
	m_strExt         = _T("");
	m_strNewFileName = _T("");
	//}}AFX_DATA_INIT
}

void CDlgNewFileName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgNewFileName)
	DDX_Text(pDX, IDC_NEWFILE_EXT, m_strExt);
	DDV_MaxChars(pDX, m_strExt, 255);
	DDX_Text(pDX, IDC_EDIT_NEWFILENAME, m_strNewFileName);
	DDV_MaxChars(pDX, m_strNewFileName, 255);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgNewFileName, CDialog)
//{{AFX_MSG_MAP(CDlgNewFileName)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgNewFileName message handlers
