// DlgMakeNPCPath.cpp : implementation file
//

#include "stdafx.h"
#include "n3me.h"
#include "DlgMakeNPCPath.h"
#include "NPCPathMgr.h"
#include "NPCPath.h"
#include "DlgLoadNPCPath.h"
#include "dlgSaveNPCPath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMakeNPCPath dialog


CDlgMakeNPCPath::CDlgMakeNPCPath(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMakeNPCPath::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMakeNPCPath)
	m_NumNPC = 0;
	m_iRegenTime = 0;
	m_PathSetFileName = _T("");
	m_State = -1;
	m_strNPCPathName = _T("");
	m_ViewNonMove = -1;
	m_fTransX = 0.0f;
	m_fTransZ = 0.0f;
	m_pSelPath = NULL;
	//}}AFX_DATA_INIT
}


void CDlgMakeNPCPath::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMakeNPCPath)
	DDX_Control(pDX, IDC_COMBO_NPC_OPTION, m_OptionList);
	DDX_Control(pDX, IDC_COMBO_NPC_ATTR_REGEN, m_AttrRegenList);
	DDX_Control(pDX, IDC_COMBO_NPC_ATTR_GROUP, m_AttrGroupList);
	DDX_Control(pDX, IDC_COMBO_NPC_ATTR_CREATE, m_AttrCreateList);
	DDX_Control(pDX, IDC_COMBO_ACTIONTYPE, m_ActTypeList);
	DDX_Control(pDX, IDC_LIST_NPCPATH, m_ListPathGroup);
	DDX_Control(pDX, IDC_COMBO_NPC, m_NPCList);
	DDX_Text(pDX, IDC_EDIT_NUM_NPC, m_NumNPC);
	DDV_MinMaxInt(pDX, m_NumNPC, 0, 15000);
	DDX_Text(pDX, IDC_EDIT_REGENTIME, m_iRegenTime);
	DDV_MinMaxInt(pDX, m_iRegenTime, 0, 33000);
	DDX_Text(pDX, IDC_PATHSETNAME, m_PathSetFileName);
	DDX_Radio(pDX, IDC_RDO_START_RGN, m_State);
	DDX_Text(pDX, IDC_EDT_NPCPATHNAME, m_strNPCPathName);
	DDV_MaxChars(pDX, m_strNPCPathName, 255);
	DDX_Radio(pDX, IDC_RDO_VIEW_NONMOVE, m_ViewNonMove);
	DDX_Text(pDX, IDC_EDT_TRANS_X, m_fTransX);
	DDX_Text(pDX, IDC_EDT_TRANS_Z, m_fTransZ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgMakeNPCPath, CDialog)
	//{{AFX_MSG_MAP(CDlgMakeNPCPath)
	ON_EN_CHANGE(IDC_EDIT_NUM_NPC, OnChangeEditNumNpc)
	ON_CBN_SELCHANGE(IDC_COMBO_NPC, OnSelchangeComboNpc)
	ON_BN_CLICKED(IDC_BTN_RECORD, OnBtnRecord)
	ON_BN_CLICKED(IDC_RDO_START_RGN, OnRdoStartRgn)
	ON_BN_CLICKED(IDC_RDO_MAKE_PATH, OnRdoMakePath)
	ON_EN_CHANGE(IDC_EDIT_REGENTIME, OnChangeEditRegentime)
	ON_CBN_SELCHANGE(IDC_COMBO_ACTIONTYPE, OnSelchangeComboActiontype)
	ON_BN_CLICKED(IDC_BTN_LOAD_PATHSET, OnBtnLoadPathset)
	ON_BN_CLICKED(IDC_BTN_SAVE_PATHSET, OnBtnSavePathset)
	ON_BN_CLICKED(IDC_BTN_SAVE_SERVER_PATHSET, OnBtnSaveServerPathset)
	ON_BN_CLICKED(IDC_BTN_PATH_DELETE, OnBtnPathDelete)
	ON_BN_CLICKED(IDC_BTN_PATH_MODIFY, OnBtnPathModify)
	ON_LBN_SELCHANGE(IDC_LIST_NPCPATH, OnSelchangeListNpcpath)
	ON_EN_CHANGE(IDC_EDT_NPCPATHNAME, OnChangeEdtNpcpathname)
	ON_BN_CLICKED(IDC_RDO_VIEW_NONE, OnRdoViewNone)
	ON_BN_CLICKED(IDC_RDO_VIEW_NONMOVE, OnRdoViewNonmove)
	ON_BN_CLICKED(IDC_BTN_SETCOLOR, OnBtnSetcolor)
	ON_BN_CLICKED(IDC_BTN_LOAD_NPCLIST, OnBtnLoadNpclist)
	ON_BN_CLICKED(IDC_BTN_TRANS, OnBtnTrans)
	ON_BN_CLICKED(IDC_RDO_MAKE_ACT_RGN, OnRdoMakeActRgn)
	ON_BN_CLICKED(IDC_BTN_LOAD_CREATE, OnBtnLoadCreate)
	ON_BN_CLICKED(IDC_BTN_LOAD_GROUP, OnBtnLoadGroup)
	ON_BN_CLICKED(IDC_BTN_LOAD_REGEN, OnBtnLoadRegen)
	ON_CBN_SELCHANGE(IDC_COMBO_NPC_ATTR_CREATE, OnSelchangeComboNpcAttrCreate)
	ON_CBN_SELCHANGE(IDC_COMBO_NPC_ATTR_GROUP, OnSelchangeComboNpcAttrGroup)
	ON_CBN_SELCHANGE(IDC_COMBO_NPC_ATTR_REGEN, OnSelchangeComboNpcAttrRegen)
	ON_BN_CLICKED(IDC_BTN_LOAD_OPTION, OnBtnLoadOption)
	ON_CBN_SELCHANGE(IDC_COMBO_NPC_OPTION, OnSelchangeComboNpcOption)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMakeNPCPath message handlers

void CDlgMakeNPCPath::OnOK() 
{
	// TODO: Add extra validation here
	//CDialog::OnOK();
}

BOOL CDlgMakeNPCPath::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_NPCList.ResetContent();
	m_ListPathGroup.ResetContent();
	m_ActTypeList.ResetContent();
	
	m_AttrRegenList.ResetContent();
	m_AttrGroupList.ResetContent();
	m_AttrCreateList.ResetContent();
	m_OptionList.ResetContent();
	
	int idx = m_AttrRegenList.AddString("NONE");
	m_AttrRegenList.SetItemData(idx, (DWORD)0);

	idx = m_AttrGroupList.AddString("NONE");
	m_AttrGroupList.SetItemData(idx, (DWORD)0);

	idx = m_AttrCreateList.AddString("NONE");
	m_AttrCreateList.SetItemData(idx, (DWORD)0);

	idx = m_OptionList.AddString("NONE");
	m_OptionList.SetItemData(idx, (DWORD)0);
	
	char szLine[512] = "", szID[80] = "", szBuff[4][80] = {"", "", "", ""};
	char szName[512] = "";
	
	//NPC종류에 관한 정의...
	char szNpcFileName[MAX_PATH];
	wsprintf(szNpcFileName, "npclist\\npclist.txt");
	FILE* stream = fopen(szNpcFileName, "r");
	if(stream)
	{
		//int result;
		while(true)
		{
			int id;
			char* pResult = fgets(szLine, 512, stream);
			if(pResult)
			{
				sprintf(szID,"");
				sprintf(szBuff[0],"");
				sprintf(szBuff[1],"");
				sprintf(szBuff[2],"");
				sprintf(szBuff[3],"");
				sscanf(szLine, "%s %s %s %s %s", szID, szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				id = atoi(szID);
				if(id==0) continue;
				sprintf(szName,"%s %s %s %s", szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				idx = m_NPCList.AddString(szName);
				m_NPCList.SetItemData(idx, (DWORD)id);
				//m_NPCList.SetCurSel(0);
			}
			else break;
		}
		fclose(stream);
	}

	//NPC움직임에 관한 정의..
	wsprintf(szNpcFileName, "npclist\\npcacttypelist.txt");
	stream = fopen(szNpcFileName, "r");
	if(stream)
	{
		//int result;
		while(true)
		{
			int id;
			char* pResult = fgets(szLine, 512, stream);
			if(pResult)
			{
				sprintf(szID,"");
				sprintf(szBuff[0],"");
				sprintf(szBuff[1],"");
				sprintf(szBuff[2],"");
				sprintf(szBuff[3],"");
				sscanf(szLine, "%s %s %s %s %s", szID, szBuff[0], szBuff[1], szBuff[2], szBuff[3]);

				id = atoi(szID);
				sprintf(szName,"%s %s %s %s", szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				int idx = m_ActTypeList.AddString(szName);
				m_ActTypeList.SetItemData(idx, (DWORD)id);
			}
			else break;
		}
		fclose(stream);
	}

	m_NPCList.SetCurSel(0);
	m_NPCList.GetLBText(0, m_strSelNPCName);
	m_iSelNPCID = (int)m_NPCList.GetItemData(0);

	//m_ActTypeList.AddString("움직이는");
	//m_ActTypeList.AddString("안움직이는");
	m_ActTypeList.SetCurSel(0);
	m_iSelActType = (int)m_ActTypeList.GetItemData(0);

	m_NumNPC = 10;
	m_State = 0;
	m_iRegenTime = 1;
	m_PathSetFileName.Empty();

	m_pSelPath = NULL;
	m_strNPCPathName.Format("noname0000");

	SetNumPoint(0);
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMakeNPCPath::OnChangeEditNumNpc() 
{
	UpdateData(TRUE);
}

void CDlgMakeNPCPath::OnSelchangeComboNpc() 
{
	int idx = m_NPCList.GetCurSel();
	m_iSelNPCID = (int)m_NPCList.GetItemData(idx);
	m_NPCList.GetLBText(idx, m_strSelNPCName);	
}

void CDlgMakeNPCPath::OnBtnRecord() 
{
	CNPCPath* pPath = m_pRefNPCPathMgr->m_pCurrPath;
	int idx = 0;
	if(pPath)
	{
		strcpy(pPath->m_strPathName, (LPCTSTR)m_strNPCPathName);
		idx = m_ListPathGroup.AddString(m_strNPCPathName);
		m_ListPathGroup.SetItemDataPtr(idx, m_pRefNPCPathMgr->m_pCurrPath); 
		m_pRefNPCPathMgr->UpdatePath();		
	}
	m_State = 0;
	m_strNPCPathName.Format("noname%04d", (idx+1));
	UpdateData(FALSE);

}

void CDlgMakeNPCPath::OnRdoStartRgn() 
{
	m_State = 0;		
}

void CDlgMakeNPCPath::OnRdoMakePath() 
{
	m_State = 1;
}

void CDlgMakeNPCPath::OnRdoMakeActRgn() 
{
	m_State = 2;	
}

void CDlgMakeNPCPath::OnChangeEditRegentime() 
{
	UpdateData(TRUE);	
}

void CDlgMakeNPCPath::OnSelchangeComboActiontype() 
{
	int idx = m_ActTypeList.GetCurSel();
	m_iSelActType = m_ActTypeList.GetItemData(idx);	
}

void CDlgMakeNPCPath::OnBtnLoadPathset() 
{
	CDlgLoadNPCPath dlg;

	m_pSelPath = NULL;
	
	if(dlg.DoModal()==IDOK)
	{
		m_pRefNPCPathMgr->LoadFromFile((LPCTSTR)dlg.m_SelFileName);
		m_PathSetFileName = dlg.m_SelFileName + ".npi";

		m_ListPathGroup.ResetContent();

		int size = m_pRefNPCPathMgr->GetSize();
		CNPCPath* pPath;
		for(int i=0;i<size;i++)
		{
			pPath = m_pRefNPCPathMgr->GetpPath(i);
			if(pPath)
			{
				__Vector3 vs, ve;
				pPath->GetPath(0, &vs);
				pPath->GetPath(pPath->GetSize()-1, &ve);
								
				int idx = m_ListPathGroup.AddString(pPath->m_strPathName);
				m_ListPathGroup.SetItemDataPtr(idx, pPath);
			}
		}
		UpdateData(FALSE);
	}
}

void CDlgMakeNPCPath::OnBtnSavePathset() 
{
	CDlgSaveNPCPath dlg;

	if(dlg.DoModal()==IDOK)
	{
		m_PathSetFileName = dlg.m_NewFileName + ".npi";
		m_pRefNPCPathMgr->SaveToFile((LPCTSTR)dlg.m_NewFileName);
		UpdateData(FALSE);
	}	
}

void CDlgMakeNPCPath::OnBtnSaveServerPathset() 
{
	CFileDialog dlg(FALSE, "snr", "Noname", OFN_EXPLORER | OFN_LONGNAMES | OFN_OVERWRITEPROMPT, "서버 NPC Route파일(*.snr)|*.snr||");

	if(dlg.DoModal()==IDOK)
	{
		CString str = dlg.GetPathName();
		m_pRefNPCPathMgr->MakeServerDataFile((LPCTSTR)str);
	}
}

void CDlgMakeNPCPath::OnBtnPathDelete() 
{
	int idx = m_ListPathGroup.GetCurSel();
	CNPCPath* pPath = (CNPCPath*)m_ListPathGroup.GetItemDataPtr(idx);
	m_ListPathGroup.DeleteString(idx);
	if(pPath)
	{
		m_pRefNPCPathMgr->DelPath(pPath);
	}
	m_pSelPath = NULL;
}

void CDlgMakeNPCPath::OnBtnPathModify() 
{
	int idx = m_ListPathGroup.GetCurSel();
	if(idx<0) return;

	m_ListPathGroup.GetText(idx, m_strNPCPathName);
	CNPCPath* pPath = (CNPCPath*)m_ListPathGroup.GetItemDataPtr(idx);

	m_ListPathGroup.DeleteString(idx);
	if(pPath)
	{
		m_pRefNPCPathMgr->SetCurrPath(pPath);
	}

	//
	m_iSelActType = pPath->m_iActType;
	int i;
	for(i=0;i<m_ActTypeList.GetCount(); i++)
	{
		int id = (int)m_ActTypeList.GetItemData(i);
		if(id==m_iSelActType)
		{
			m_ActTypeList.SetCurSel(i);			
			break;
		}
	}
	
	m_iRegenTime = pPath->m_iRegenTime;

	m_iSelNPCID = pPath->m_iNPCID;
	
	for(i=0;i<m_NPCList.GetCount(); i++)
	{
		int id = (int)m_NPCList.GetItemData(i);
		if(id==m_iSelNPCID)
		{
			m_NPCList.SetCurSel(i);
			m_NPCList.GetLBText(i, m_strSelNPCName);
			break;
		}
	}
	if(i==m_NPCList.GetCount())
	{
		ZeroMemory(m_strSelNPCName, 80);
		m_NPCList.SetCurSel(-1);
	}

	m_cSelAttrRegen = pPath->m_cAttr_Regen;

	for(i=0;i<m_AttrRegenList.GetCount(); i++)
	{
		unsigned char cId = (int)m_AttrRegenList.GetItemData(i);
		if(cId==m_cSelAttrRegen)
		{
			m_AttrRegenList.SetCurSel(i);
			break;
		}
	}

	m_cSelAttrGroup = pPath->m_cAttr_Group;

	for(i=0;i<m_AttrGroupList.GetCount(); i++)
	{
		unsigned char cId = (int)m_AttrGroupList.GetItemData(i);
		if(cId==m_cSelAttrGroup)
		{
			m_AttrGroupList.SetCurSel(i);
			break;
		}
	}

	m_cSelAttrCreate = pPath->m_cAttr_Create;

	for(i=0;i<m_AttrCreateList.GetCount(); i++)
	{
		unsigned char cId = (int)m_AttrCreateList.GetItemData(i);
		if(cId==m_cSelAttrCreate)
		{
			m_AttrCreateList.SetCurSel(i);
			break;
		}
	}

	m_cSelOption = pPath->m_cAttr_Option;

	for(i=0;i<m_OptionList.GetCount(); i++)
	{
		unsigned char cId = (int)m_OptionList.GetItemData(i);
		if(cId==m_cSelOption)
		{
			m_OptionList.SetCurSel(i);
			break;
		}
	}

	m_NumNPC = pPath->m_iNumNPC;	

	UpdateData(FALSE);
}

void CDlgMakeNPCPath::OnSelchangeListNpcpath() 
{
	int idx = m_ListPathGroup.GetCurSel();
	if(idx<0) m_pSelPath = NULL;
	else m_pSelPath = (CNPCPath*)m_ListPathGroup.GetItemDataPtr(idx);
}

void CDlgMakeNPCPath::OnChangeEdtNpcpathname() 
{
	UpdateData(TRUE);
}

void CDlgMakeNPCPath::SetNumPoint(int NumPoint)
{
	char NumP[5];
	_itoa((MAX_NPCPATH - NumPoint), NumP, 10);
	SetDlgItemText(IDC_NUM_POINT, NumP);
}

void CDlgMakeNPCPath::OnRdoViewNone() 
{
	m_pRefNPCPathMgr->m_bRenderMovableRegion = false;
}

void CDlgMakeNPCPath::OnRdoViewNonmove() 
{
	m_pRefNPCPathMgr->m_bRenderMovableRegion = true;	
}

void CDlgMakeNPCPath::OnBtnSetcolor() 
{
	CColorDialog dlg;
	DWORD dwColor = 0xff0000ff;
	if(dlg.DoModal()==IDOK)
	{
		COLORREF color = dlg.GetColor();
		DWORD r,g,b;
		r = (color<<24)>>8;
		g = ((color>>8)<<24)>>16;
		b = ((color>>16)<<24)>>24;
		dwColor = 0xff000000;
		dwColor += r;
		dwColor += g;
		dwColor += b;
	}
	if(m_pRefNPCPathMgr->m_pCurrPath)
	{
		m_pRefNPCPathMgr->m_pCurrPath->m_dwColor = dwColor;
	}
}

void CDlgMakeNPCPath::OnBtnLoadNpclist() 
{
	m_NPCList.ResetContent();

	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "txt", NULL, dwFlags, "NPCList file(*.txt)|*.txt||", NULL);
	if(dlg.DoModal() == IDCANCEL) return;

	CString szNpcFileName = dlg.GetPathName();

	char szLine[512] = "", szID[80] = "", szBuff[4][80] = {"", "", "", ""};
	char szName[512] = "";

	FILE* stream = fopen((LPCTSTR)szNpcFileName, "r");
	if(stream)
	{
		//int result;
		while(true)
		{
			int id;
			char* pResult = fgets(szLine, 512, stream);
			if(pResult)
			{
				sprintf(szID,"");
				sprintf(szBuff[0],"");
				sprintf(szBuff[1],"");
				sprintf(szBuff[2],"");
				sprintf(szBuff[3],"");
				sscanf(szLine, "%s %s %s %s %s", szID, szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				id = atoi(szID);
				sprintf(szName,"%s %s %s %s", szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				if(id==0) continue;
				int idx = m_NPCList.AddString(szName);
				m_NPCList.SetItemData(idx, (DWORD)id);
				m_NPCList.SetCurSel(0);
			}
			else break;
		}
		fclose(stream);
	}	
}

void CDlgMakeNPCPath::OnBtnTrans() 
{
	UpdateData(TRUE);
	m_pRefNPCPathMgr->TransPos(m_fTransX, m_fTransZ);
}



void CDlgMakeNPCPath::OnBtnLoadCreate() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "txt", NULL, dwFlags, "Create Attr List file(*.txt)|*.txt||", NULL);

	if(dlg.DoModal() == IDCANCEL) return;

	m_AttrCreateList.ResetContent();

	int idx = m_AttrCreateList.AddString("NONE");
	m_AttrCreateList.SetItemData(idx, (DWORD)0);

	char szLine[512] = "", szID[80] = "", szBuff[4][80] = {"", "", "", ""};
	char szName[512] = "";

	FILE* stream = fopen((LPCTSTR)dlg.GetPathName(), "r");
	if(stream)
	{
		while(true)
		{
			int id;
			char* pResult = fgets(szLine, 512, stream);
			if(pResult)
			{
				sprintf(szID,"");
				sprintf(szBuff[0],"");
				sprintf(szBuff[1],"");
				sprintf(szBuff[2],"");
				sprintf(szBuff[3],"");
				sscanf(szLine, "%s %s %s %s %s", szID, szBuff[0], szBuff[1], szBuff[2], szBuff[3]);

				id = atoi(szID);
				sprintf(szName,"%s %s %s %s", szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				int idx = m_AttrCreateList.AddString(szName);
				m_AttrCreateList.SetItemData(idx, (DWORD)id);
			}
			else break;
		}
		fclose(stream);
	}
}

void CDlgMakeNPCPath::OnBtnLoadGroup() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "txt", NULL, dwFlags, "Group Attr List file(*.txt)|*.txt||", NULL);

	if(dlg.DoModal() == IDCANCEL) return;

	m_AttrGroupList.ResetContent();

	int idx = m_AttrGroupList.AddString("NONE");
	m_AttrGroupList.SetItemData(idx, (DWORD)0);

	char szLine[512] = "", szID[80] = "", szBuff[4][80] = {"", "", "", ""};
	char szName[512] = "";

	FILE* stream = fopen((LPCTSTR)dlg.GetPathName(), "r");
	if(stream)
	{
		while(true)
		{
			int id;
			char* pResult = fgets(szLine, 512, stream);
			if(pResult)
			{
				sprintf(szID,"");
				sprintf(szBuff[0],"");
				sprintf(szBuff[1],"");
				sprintf(szBuff[2],"");
				sprintf(szBuff[3],"");
				sscanf(szLine, "%s %s %s %s %s", szID, szBuff[0], szBuff[1], szBuff[2], szBuff[3]);

				id = atoi(szID);
				sprintf(szName,"%s %s %s %s", szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				int idx = m_AttrGroupList.AddString(szName);
				m_AttrGroupList.SetItemData(idx, (DWORD)id);
			}
			else break;
		}
		fclose(stream);
	}
}

void CDlgMakeNPCPath::OnBtnLoadRegen() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "txt", NULL, dwFlags, "Regen Attr List file(*.txt)|*.txt||", NULL);

	if(dlg.DoModal() == IDCANCEL) return;
	
	m_AttrRegenList.ResetContent();

	int idx = m_AttrRegenList.AddString("NONE");
	m_AttrRegenList.SetItemData(idx, (DWORD)0);

	char szLine[512] = "", szID[80] = "", szBuff[4][80] = {"", "", "", ""};
	char szName[512] = "";

	FILE* stream = fopen((LPCTSTR)dlg.GetPathName(), "r");
	if(stream)
	{
		while(true)
		{
			int id;
			char* pResult = fgets(szLine, 512, stream);
			if(pResult)
			{
				sprintf(szID,"");
				sprintf(szBuff[0],"");
				sprintf(szBuff[1],"");
				sprintf(szBuff[2],"");
				sprintf(szBuff[3],"");
				sscanf(szLine, "%s %s %s %s %s", szID, szBuff[0], szBuff[1], szBuff[2], szBuff[3]);

				id = atoi(szID);
				sprintf(szName,"%s %s %s %s", szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				int idx = m_AttrRegenList.AddString(szName);
				m_AttrRegenList.SetItemData(idx, (DWORD)id);
			}
			else break;
		}
		fclose(stream);
	}
}

void CDlgMakeNPCPath::OnBtnLoadOption() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(TRUE, "txt", NULL, dwFlags, "Option Attr List file(*.txt)|*.txt||", NULL);

	if(dlg.DoModal() == IDCANCEL) return;

	m_OptionList.ResetContent();

	int idx = m_OptionList.AddString("NONE");
	m_OptionList.SetItemData(idx, (DWORD)0);

	char szLine[512] = "", szID[80] = "", szBuff[4][80] = {"", "", "", ""};
	char szName[512] = "";

	FILE* stream = fopen((LPCTSTR)dlg.GetPathName(), "r");
	if(stream)
	{
		while(true)
		{
			int id;
			char* pResult = fgets(szLine, 512, stream);
			if(pResult)
			{
				sprintf(szID,"");
				sprintf(szBuff[0],"");
				sprintf(szBuff[1],"");
				sprintf(szBuff[2],"");
				sprintf(szBuff[3],"");
				sscanf(szLine, "%s %s %s %s %s", szID, szBuff[0], szBuff[1], szBuff[2], szBuff[3]);

				id = atoi(szID);
				sprintf(szName,"%s %s %s %s", szBuff[0], szBuff[1], szBuff[2], szBuff[3]);
				
				int idx = m_OptionList.AddString(szName);
				m_OptionList.SetItemData(idx, (DWORD)id);
			}
			else break;
		}
		fclose(stream);
	}
}

void CDlgMakeNPCPath::OnSelchangeComboNpcAttrCreate() 
{
	int idx = m_AttrCreateList.GetCurSel();
	m_cSelAttrCreate = (unsigned char)m_AttrCreateList.GetItemData(idx);
}

void CDlgMakeNPCPath::OnSelchangeComboNpcAttrGroup() 
{	
	int idx = m_AttrGroupList.GetCurSel();
	m_cSelAttrGroup = (unsigned char)m_AttrGroupList.GetItemData(idx);
}

void CDlgMakeNPCPath::OnSelchangeComboNpcAttrRegen() 
{
	int idx = m_AttrRegenList.GetCurSel();
	m_cSelAttrRegen = (unsigned char)m_AttrRegenList.GetItemData(idx);	
}

void CDlgMakeNPCPath::OnSelchangeComboNpcOption() 
{
	int idx = m_OptionList.GetCurSel();
	m_cSelOption = (unsigned char)m_OptionList.GetItemData(idx);	
}
