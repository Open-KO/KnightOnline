// BkDialog.cpp : Defines the class behaviors for the application.
//

#include "BkDialog.h"
#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBkDialog::CBkDialog(CWnd *pParent /*=NULL*/)
{
    //{{AFX_DATA_INIT(CBkDialog)
    // NOTE: the ClassWizard will add member initialization here

    //}}AFX_DATA_INIT
}

CBkDialog::CBkDialog(UINT uResourceID, CWnd *pParent) : CDialog(uResourceID, pParent)
{
}

CBkDialog::CBkDialog(LPCTSTR pszResourceID, CWnd *pParent) : CDialog(pszResourceID, pParent)
{
}

CBkDialog::~CBkDialog()
{
}

void CBkDialog::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CBkDialog)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBkDialog, CDialog)
//{{AFX_MSG_MAP(CBkDialog)
ON_WM_ERASEBKGND()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBkDialog::OnEraseBkgnd(CDC *pDC)
{
    CRect rc;

    GetClientRect(rc);

    m_pDC = pDC;

    if (TileBitmap(pDC, rc) == TRUE)
        return TRUE;
    else
        return CDialog::OnEraseBkgnd(pDC);

} // End of OnEraseBkgnd
