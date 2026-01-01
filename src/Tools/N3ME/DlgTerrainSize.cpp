// DlgTerrainSize.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgTerrainSize.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTerrainSize dialog

CDlgTerrainSize::CDlgTerrainSize(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgTerrainSize::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTerrainSize)
	m_fSize = 0.0f;
	//}}AFX_DATA_INIT
}


void CDlgTerrainSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTerrainSize)
	DDX_Text(pDX, IDC_EDIT1, m_fSize);
	DDV_MinMaxFloat(pDX, m_fSize, 0.f, 1.e+006f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgTerrainSize, CDialog)
	//{{AFX_MSG_MAP(CDlgTerrainSize)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTerrainSize message handlers
