// DlgEditEvent.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgEditEvent.h"
#include "EventMgr.h"
#include "EventCell.h"
#include "MapMng.h"
#include "DlgLoadEvt.h"
#include "DlgSaveEvt.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditEvent dialog


CDlgEditEvent::CDlgEditEvent(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgEditEvent::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditEvent)
	m_CurrFileName = _T("");
	//}}AFX_DATA_INIT

	m_pRefEventMgr = nullptr;
	m_pSelEvent = nullptr;
}


void CDlgEditEvent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditEvent)
	DDX_Control(pDX, IDC_EVENT_LIST, m_EventList);
	DDX_Text(pDX, IDC_STATIC_CURR_FILENAME, m_CurrFileName);
	DDV_MaxChars(pDX, m_CurrFileName, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgEditEvent, CDialog)
	//{{AFX_MSG_MAP(CDlgEditEvent)
	ON_BN_CLICKED(IDC_BTN_EVENT_DEL, OnBtnEventDel)
	ON_BN_CLICKED(IDC_BTN_EVENT_EDIT, OnBtnEventEdit)
	ON_BN_CLICKED(IDC_BTN_EVENT_RECORD, OnBtnEventRecord)
	ON_LBN_SELCHANGE(IDC_EVENT_LIST, OnSelchangeEventList)
	ON_BN_CLICKED(IDC_BTN_LOAD_EVTFILE, OnBtnLoadEvtfile)
	ON_BN_CLICKED(IDC_BTN_SAVE_EVTFILE, OnBtnSaveEvtfile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditEvent message handlers

void CDlgEditEvent::OnBtnEventDel() 
{
	int idx = m_EventList.GetCurSel();
	CEventCell* pEvent = (CEventCell*)m_EventList.GetItemDataPtr(idx);
	m_EventList.DeleteString(idx);

	if(pEvent) m_pRefEventMgr->DelEvent(pEvent);
	m_pSelEvent = nullptr;
	m_pRefEventMgr->m_pRefMapMng->Invalidate();
}

void CDlgEditEvent::OnBtnEventEdit() 
{
	int idx = m_EventList.GetCurSel();
	if(idx<0) return;

	CEventCell* pEvent = (CEventCell*)m_EventList.GetItemDataPtr(idx);

	m_EventList.DeleteString(idx);

	if(pEvent)
	{
		m_pRefEventMgr->SetCurrEvent(pEvent);
	}
	m_pRefEventMgr->m_pRefMapMng->Invalidate();
}

void CDlgEditEvent::OnBtnEventRecord() 
{
	CEventCell* pEvent = m_pRefEventMgr->m_pCurrEvent;
	int idx = 0;

	if(pEvent)
	{
		CString CurrName;
		GetDlgItemText(IDC_STATIC_CURR_EVENT, CurrName);

		idx = m_EventList.AddString((LPCTSTR)CurrName);

		m_EventList.SetItemDataPtr(idx, pEvent);

		m_pRefEventMgr->UpdateEvent();
	}
	m_pRefEventMgr->m_pRefMapMng->Invalidate();
}

BOOL CDlgEditEvent::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_EventList.ResetContent();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgEditEvent::SetCurrName(char *pName)
{
	SetDlgItemText(IDC_STATIC_CURR_EVENT, pName);
}

void CDlgEditEvent::ResetAll()
{
	m_EventList.ResetContent();

	std::list<CEventCell*>::iterator itEvent;

	for(itEvent = m_pRefEventMgr->m_pEvents.begin(); itEvent != m_pRefEventMgr->m_pEvents.end(); itEvent++)
	{
		int idx = m_EventList.AddString((*itEvent)->m_Name);
		m_EventList.SetItemDataPtr(idx, (*itEvent));
	}
	m_pRefEventMgr->m_pRefMapMng->Invalidate();
}

void CDlgEditEvent::OnSelchangeEventList() 
{
	int idx = m_EventList.GetCurSel();
	m_pSelEvent = (CEventCell*)m_EventList.GetItemDataPtr(idx);	
	m_pRefEventMgr->m_pRefMapMng->Invalidate();
}

void CDlgEditEvent::OnBtnLoadEvtfile() 
{
	CDlgLoadEvt dlg;

	if(dlg.DoModal()==IDOK)
	{
		m_pRefEventMgr->LoadFromFile((LPCTSTR)dlg.m_SelFileName);
		ResetAll();

		if(dlg.m_SelFileName.IsEmpty())
		{
			m_CurrFileName = "";
		}
		else m_CurrFileName = dlg.m_SelFileName + ".evt";

		UpdateData(FALSE);
	}
}

void CDlgEditEvent::OnBtnSaveEvtfile() 
{
	CDlgSaveEvt dlg;

	if(dlg.DoModal()==IDOK)
	{
		m_pRefEventMgr->SaveToFile((LPCTSTR)dlg.m_FileName);
		
		if(dlg.m_FileName.IsEmpty())
		{
			m_CurrFileName = "";
		}
		else m_CurrFileName = dlg.m_FileName + ".evt";

		UpdateData(FALSE);
	}	
}
