// Dlg_River.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "Dlg_River.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlg_River dialog


CDlg_River::CDlg_River(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_River::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlg_River)
	//}}AFX_DATA_INIT	
}


void CDlg_River::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlg_River)
	DDX_Control(pDX, IDC_LIST_RIVER_PROPERTY, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlg_River, CDialog)
	//{{AFX_MSG_MAP(CDlg_River)
	ON_EN_CHANGE(IDC_EDIT_HEIGHT, OnChangeEditHeight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlg_River message handlers

void CDlg_River::OnOK() 
{
	return;	
	CDialog::OnOK();
}

void CDlg_River::OnCancel() 
{
	return;	
	CDialog::OnCancel();
}

BOOL CDlg_River::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	UpdateData(FALSE);
	m_List.AddPropItem("Alpha", "255", PIT_EDIT, "");
	m_List.AddPropItem("Water Color", "", PIT_COLOR, "0xffffffff");
	m_List.AddPropItem("Height", "0.0", PIT_EDIT, "");

	CPropertyItem *pItem = m_List.GetPropItem("Water Color");
	pItem->D3DColorSet(D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_River::OnChangeEditHeight() 
{
	UpdateData(TRUE);	
}

BOOL CDlg_River::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	if((void*)wParam == &m_List)
	{
		CPropertyItem* pItem = (CPropertyItem*)lParam;
		if (pItem->m_propName == "Alpha")
		{
			CPropertyItem *pItem2 = m_List.GetPropItem("Water Color");
			D3DCOLOR Color = pItem2->D3DColorGet();
			int nAlpha = atoi(pItem->m_curValue);

			Color &= 0x00ffffff;
			Color |= ((nAlpha&0xff)<<24);
			pItem2->D3DColorSet(Color);
		}
		else if (pItem->m_propName == "Water Color")
		{
			CPropertyItem *pItem_Alpha = m_List.GetPropItem("Alpha");

			D3DCOLOR dwColor = pItem->D3DColorGet();
			int nAlpha = (dwColor&0xff000000)>>24;
			pItem_Alpha->m_curValue.Format("%d", nAlpha);
		}
//		else if (pItem->m_propName == "Height")
//		{
//			m_fHeight = atof(pItem->m_curValue);
//		}
//		CPropertyItem *pItem1 = m_List.GetPropItem("Water Color");
//		D3DCOLOR Color = pItem1->D3DColorGet();
//		TRACE("Color = %0x\n", Color);
		Invalidate(FALSE);
	}
	
	return TRUE;
	//return CDialog::OnNotify(wParam, lParam, pResult);
}
