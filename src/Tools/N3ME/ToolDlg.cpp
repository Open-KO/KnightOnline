// ToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "ToolDlg.h"
#include "../N3Base/N3Transform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolDlg dialog


CToolDlg::CToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CToolDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CToolDlg)
	m_fRotY = 0.0f;
	//}}AFX_DATA_INIT
	m_pSelectedObj = NULL;
}


void CToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToolDlg)
	DDX_Control(pDX, IDC_SLIDER_ROTATE, m_RatateSlider);
	DDX_Text(pDX, IDC_EDIT_ROTATE, m_fRotY);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToolDlg, CDialog)
	//{{AFX_MSG_MAP(CToolDlg)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_ROTATE, OnCustomdrawSliderRotate)
	ON_EN_CHANGE(IDC_EDIT_ROTATE, OnChangeEditRotate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolDlg message handlers

BOOL CToolDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_RatateSlider.SetRange(-1799, 1800);
	m_RatateSlider.SetPos(-1799);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CToolDlg::OnCustomdrawSliderRotate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int iPos = m_RatateSlider.GetPos();
	m_fRotY = iPos*0.1f;
	UpdateData(FALSE);

	if (m_pSelectedObj)
	{
		m_pSelectedObj->m_vRot.y = D3DXToRadian(m_fRotY);
		((CFrameWnd*)AfxGetMainWnd())->GetActiveView()->Invalidate(FALSE);
	}
	
	*pResult = 0;
}

void CToolDlg::OnChangeEditRotate() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	UpdateData(TRUE);
	int iPos = (int)(m_fRotY/0.1f);
	m_RatateSlider.SetPos(iPos);
}

void CToolDlg::SetObj(CN3Transform* pObj)
{
	m_pSelectedObj = pObj;
	if (m_pSelectedObj)
	{
		m_fRotY = D3DXToDegree(m_pSelectedObj->m_vRot.y);
	}
	else
	{
		m_fRotY = 0;
	}
	int iPos = (int)(m_fRotY/0.1f);
	m_RatateSlider.SetPos(iPos);
}
