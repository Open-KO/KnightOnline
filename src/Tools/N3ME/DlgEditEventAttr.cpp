// DlgEditEventAttr.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgEditEventAttr.h"
#include "EventCell.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditEventAttr dialog

CDlgEditEventAttr::CDlgEditEventAttr(CWnd* pParent /*=nullptr*/) :
	CDialog(CDlgEditEventAttr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditEventAttr)
	m_EventType = 0;
	m_EventID   = 0;
	m_pRefEvent = nullptr;
	m_ZoneID    = 0;
	//}}AFX_DATA_INIT
}

void CDlgEditEventAttr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditEventAttr)
	DDX_Text(pDX, IDC_EDIT_EVENT_ATTR_TYPE, m_EventType);
	DDV_MinMaxInt(pDX, m_EventType, 0, 99999);
	DDX_Text(pDX, IDC_EDIT_EVENT_ATTR_ID, m_EventID);
	DDV_MinMaxInt(pDX, m_EventID, 1, 99999);
	DDX_Text(pDX, IDC_EDIT_EVENT_ZONE_ID, m_ZoneID);
	DDV_MinMaxInt(pDX, m_ZoneID, 0, 32000);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgEditEventAttr, CDialog)
//{{AFX_MSG_MAP(CDlgEditEventAttr)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditEventAttr message handlers

void CDlgEditEventAttr::OnOK()
{
	UpdateData(TRUE);

	m_pRefEvent->m_EventType = m_EventType;
	m_pRefEvent->m_EventID   = m_EventID;
	m_pRefEvent->m_ZoneID    = m_ZoneID;

	GetDlgItemText(IDC_EDIT_EVENT_ATTR_NAME, m_pRefEvent->m_Name, 80);

	GetDlgItemText(IDC_EDIT_EVENT_ATTR_CON1, m_pRefEvent->m_Con[0], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_CON2, m_pRefEvent->m_Con[1], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_CON3, m_pRefEvent->m_Con[2], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_CON4, m_pRefEvent->m_Con[3], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_CON5, m_pRefEvent->m_Con[4], 256);

	GetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE1, m_pRefEvent->m_Exe[0], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE2, m_pRefEvent->m_Exe[1], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE3, m_pRefEvent->m_Exe[2], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE4, m_pRefEvent->m_Exe[3], 256);
	GetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE5, m_pRefEvent->m_Exe[4], 256);

	CDialog::OnOK();
}

void CDlgEditEventAttr::SetEventCell(CEventCell* pEvent)
{
	m_pRefEvent = pEvent;
}

BOOL CDlgEditEventAttr::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_pRefEvent)
	{
		m_EventType = m_pRefEvent->m_EventType;
		m_EventID   = m_pRefEvent->m_EventID;
		m_ZoneID    = m_pRefEvent->m_ZoneID;

		SetDlgItemText(IDC_EDIT_EVENT_ATTR_NAME, m_pRefEvent->m_Name);

		SetDlgItemText(IDC_EDIT_EVENT_ATTR_CON1, m_pRefEvent->m_Con[0]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_CON2, m_pRefEvent->m_Con[1]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_CON3, m_pRefEvent->m_Con[2]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_CON4, m_pRefEvent->m_Con[3]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_CON5, m_pRefEvent->m_Con[4]);

		SetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE1, m_pRefEvent->m_Exe[0]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE2, m_pRefEvent->m_Exe[1]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE3, m_pRefEvent->m_Exe[2]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE4, m_pRefEvent->m_Exe[3]);
		SetDlgItemText(IDC_EDIT_EVENT_ATTR_EXE5, m_pRefEvent->m_Exe[4]);

		UpdateData(FALSE);
	}

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}
