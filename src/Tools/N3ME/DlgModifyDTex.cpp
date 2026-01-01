// DlgModifyDTex.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgModifyDTex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgModifyDTex dialog


CDlgModifyDTex::CDlgModifyDTex(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgModifyDTex::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgModifyDTex)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgModifyDTex::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgModifyDTex)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgModifyDTex, CDialog)
	//{{AFX_MSG_MAP(CDlgModifyDTex)
	ON_BN_CLICKED(ID_MODIFY_DTEX_MIRROR, OnModifyDtexMirror)
	ON_BN_CLICKED(ID_MODIFY_DTEX_CW90D, OnModifyDtexCw90d)
	ON_BN_CLICKED(ID_MODIFY_DTEX_CW270D, OnModifyDtexCw270d)
	ON_BN_CLICKED(ID_MODIFY_DTEX_CW180D, OnModifyDtexCw180d)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgModifyDTex message handlers

void CDlgModifyDTex::OnModifyDtexMirror() 
{
	m_State = 1;
	OnOK();	
}

BOOL CDlgModifyDTex::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowPos(nullptr, m_Pos.x, m_Pos.y,0,0, SWP_NOSIZE );
	m_State = 0;
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgModifyDTex::OnModifyDtexCw90d() 
{
	m_State = 2;
	OnOK();
}

void CDlgModifyDTex::OnModifyDtexCw270d() 
{
	m_State = 4;
	OnOK();
}

void CDlgModifyDTex::OnModifyDtexCw180d() 
{
	m_State = 3;
	OnOK();
}
