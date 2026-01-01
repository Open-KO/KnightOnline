// DlgRegenUser.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgRegenUser.h"
#include "RegenUser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgRegenUser dialog


CDlgRegenUser::CDlgRegenUser(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgRegenUser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgRegenUser)
	m_pRefRegenUser = nullptr;
	m_PathName = _T("");
	//}}AFX_DATA_INIT
}


void CDlgRegenUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgRegenUser)
	DDX_Control(pDX, IDC_LIST_REGENUSERREGION, m_LBRegion);
	DDX_Text(pDX, IDC_EDT_PATHNAME, m_PathName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgRegenUser, CDialog)
	//{{AFX_MSG_MAP(CDlgRegenUser)
	ON_BN_CLICKED(IDC_BTN_LOAD_PATHSET, OnBtnLoadPathset)
	ON_BN_CLICKED(IDC_BTN_PATH_DELETE, OnBtnPathDelete)
	ON_BN_CLICKED(IDC_BTN_SAVE_PATHSET, OnBtnSavePathset)
	ON_LBN_SELCHANGE(IDC_LIST_REGENUSERREGION, OnSelchangeListRegenuserregion)
	ON_LBN_DBLCLK(IDC_LIST_REGENUSERREGION, OnDblclkListRegenuserregion)
	ON_BN_CLICKED(IDC_BTN_SAVE_AS, OnBtnSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRegenUser message handlers

void CDlgRegenUser::OnBtnLoadPathset() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "trur", nullptr, dwFlags, "trur파일(*.trur)|*.trur||", nullptr);

	if(dlg.DoModal() == IDCANCEL) return;

	m_PathName = dlg.GetPathName();

	m_pRefRegenUser->LoadFromFile((LPCTSTR)m_PathName);
	UpdateData(FALSE);
}

void CDlgRegenUser::OnBtnPathDelete() 
{
	int idx = m_LBRegion.GetCurSel();
	if(idx<0) return;

	m_LBRegion.SetItemDataPtr(idx, nullptr);
	m_LBRegion.DeleteString(idx);
	m_pRefRegenUser->DeleteSel();
}

void CDlgRegenUser::OnBtnSavePathset() 
{
	if(m_PathName.IsEmpty())
	{
		CFileDialog dlg(FALSE, "trur", "Noname", OFN_EXPLORER | OFN_LONGNAMES | OFN_OVERWRITEPROMPT, "trur파일(*.trur)|*.trur||");

		if(dlg.DoModal()==IDOK)
		{
			m_PathName = dlg.GetPathName();
			m_pRefRegenUser->SaveToFile((LPCTSTR)m_PathName);
			UpdateData(FALSE);
		}
	}
	else m_pRefRegenUser->SaveToFile((LPCTSTR)m_PathName);	
}

void CDlgRegenUser::OnSelchangeListRegenuserregion() 
{
	int idx = m_LBRegion.GetCurSel();
	if(idx<0) return;

	VERTEXRECT* pVR = (VERTEXRECT*)m_LBRegion.GetItemDataPtr(idx);
	m_pRefRegenUser->m_vrSelRegion = pVR;
	UpdateData(FALSE);
}

void CDlgRegenUser::OnDblclkListRegenuserregion() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CDlgRegenUser::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_LBRegion.ResetContent();
	m_PathName.Empty();

	UpdateData(FALSE);	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgRegenUser::PutRegion(VERTEXRECT* pVR)
{
	CString str;
	POINT LB, TR;
	LB.x = pVR->m_vLB.x;
	LB.y = pVR->m_vLB.z;

	TR.x = pVR->m_vRT.x;
	TR.y = pVR->m_vRT.z;

	str.Format("(%d, %d) -> (%d, %d)", LB.x, LB.y, TR.x, TR.y);

	int idx = m_LBRegion.AddString((LPCTSTR)str);
	m_LBRegion.SetItemDataPtr(idx, pVR);
}

void CDlgRegenUser::OnBtnSaveAs() 
{
	CFileDialog dlg(FALSE, "trur", "Noname", OFN_EXPLORER | OFN_LONGNAMES | OFN_OVERWRITEPROMPT, "trur파일(*.trur)|*.trur||");

	if(dlg.DoModal()==IDOK)
	{
		m_PathName = dlg.GetPathName();
		m_pRefRegenUser->SaveToFile((LPCTSTR)m_PathName);
	}

	UpdateData(FALSE);
}

void CDlgRegenUser::ClearList()
{
	m_LBRegion.ResetContent();
}