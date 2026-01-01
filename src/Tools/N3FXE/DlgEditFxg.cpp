// DlgEditFxg.cpp : implementation file
//

#include "stdafx.h"
#include "n3fxe.h"
#include "MainFrm.h"
#include "DlgEditFxg.h"
#include <N3Base/N3FXGroup.h>
#include "DlgNewFileName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditFxg dialog


CDlgEditFxg::CDlgEditFxg(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgEditFxg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditFxg)
	m_bLoop_0 = FALSE;
	m_bLoop_1 = FALSE;
	m_bLoop_2 = FALSE;
	m_bLoop_3 = FALSE;
	m_strFileName = _T("");
	m_strFXBName_0 = _T("");
	m_strFXBName_1 = _T("");
	m_strFXBName_2 = _T("");
	m_strFXBName_3 = _T("");
	//}}AFX_DATA_INIT

	m_pJoint[0] = &m_CBJoint_0;
	m_pJoint[1] = &m_CBJoint_1;
	m_pJoint[2] = &m_CBJoint_2;
	m_pJoint[3] = &m_CBJoint_3;

	m_pLoop[0] = &m_bLoop_0;
	m_pLoop[1] = &m_bLoop_1;
	m_pLoop[2] = &m_bLoop_2;
	m_pLoop[3] = &m_bLoop_3;

	m_pFXBName[0] = &m_strFXBName_0;
	m_pFXBName[1] = &m_strFXBName_1;
	m_pFXBName[2] = &m_strFXBName_2;
	m_pFXBName[3] = &m_strFXBName_3;
}


void CDlgEditFxg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditFxg)
	DDX_Control(pDX, IDC_CB_JOINT_3, m_CBJoint_3);
	DDX_Control(pDX, IDC_CB_JOINT_2, m_CBJoint_2);
	DDX_Control(pDX, IDC_CB_JOINT_1, m_CBJoint_1);
	DDX_Control(pDX, IDC_CB_JOINT_0, m_CBJoint_0);
	DDX_Check(pDX, IDC_CHK_LOOP_0, m_bLoop_0);
	DDX_Check(pDX, IDC_CHK_LOOP_1, m_bLoop_1);
	DDX_Check(pDX, IDC_CHK_LOOP_2, m_bLoop_2);
	DDX_Check(pDX, IDC_CHK_LOOP_3, m_bLoop_3);
	DDX_Text(pDX, IDC_EDT_FILENAME, m_strFileName);
	DDX_Text(pDX, IDC_EDT_FXB_NAME_0, m_strFXBName_0);
	DDX_Text(pDX, IDC_EDT_FXB_NAME_1, m_strFXBName_1);
	DDX_Text(pDX, IDC_EDT_FXB_NAME_2, m_strFXBName_2);
	DDX_Text(pDX, IDC_EDT_FXB_NAME_3, m_strFXBName_3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgEditFxg, CDialog)
	//{{AFX_MSG_MAP(CDlgEditFxg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_SAVE_AS, OnBtnSaveAs)
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBtnSave)
	ON_BN_CLICKED(IDC_BTN_LOAD_FXB_0, OnBtnLoadFxb0)
	ON_BN_CLICKED(IDC_BTN_LOAD_FXB_1, OnBtnLoadFxb1)
	ON_BN_CLICKED(IDC_BTN_LOAD_FXB_2, OnBtnLoadFxb2)
	ON_BN_CLICKED(IDC_BTN_LOAD_FXB_3, OnBtnLoadFxb3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditFxg message handlers

BOOL CDlgEditFxg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for(int i=0;i<MAX_FXB;i++)
	{
		(*m_pLoop[i]) = FALSE;
		m_pFXBName[i]->Empty();

		m_pJoint[i]->ResetContent();		
		m_pJoint[i]->Clear();
		m_pJoint[i]->InsertString(0,"배꼽");
		m_pJoint[i]->SetCurSel(0);

		//	m_CBPartName들 채우기..
		m_pJoint[i]->InsertString(1, "오른손");
		m_pJoint[i]->InsertString(2, "왼손");		
	}
	m_strFileName.Empty();

	UpdateData(FALSE);
	
	return TRUE;
}

bool CDlgEditFxg::LoadScript(const char* szPath)
{
	m_strFileName = szPath;
	CN3FXGroup* pGroup = new CN3FXGroup;
	if(!pGroup->DecodeScriptFile(szPath))
	{
		delete pGroup;
		return false;
	}

	//////////////////////////////////////////////////
	//각 컨트롤 셋팅...
	int cnt = pGroup->GetCount();
	for(int i=0;i<cnt;i++)
	{
		__FXBInfo* pInfo = pGroup->GetFXBInfo(i);

		m_pJoint[i]->SetCurSel(pInfo->joint);
		*(m_pLoop[i]) = pInfo->IsLooping;
		m_pFXBName[i]->Format(pInfo->FXBName);		
	}
	UpdateData(FALSE);
	//
	//////////////////////////////////////////////////

	delete pGroup;
	return true;
}

void CDlgEditFxg::SaveGameData()
{
	//OnBtnSave();
	if(m_strFileName.IsEmpty()) return;

	CN3FXGroup* pGroup = new CN3FXGroup;
	if(!pGroup->DecodeScriptFile((LPCTSTR)m_strFileName))
	{
		delete pGroup;
		return;
	}

	char szPath[_MAX_PATH];
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(m_strFileName, szDrive, szDir, szFName, szExt);
	_makepath(szPath, szDrive, szDir, szFName, "fxg");
	
	pGroup->SaveToFile(szPath);

	delete pGroup;
	return;	
}

void CDlgEditFxg::OnOK() 
{
	return;	//CDialog::OnOK();
}

void CDlgEditFxg::OnClose() 
{	
	CDialog::OnClose();
	m_pRefFrm->DestroyGroup(this);
}

void CDlgEditFxg::OnBtnSaveAs() 
{
	UpdateData(TRUE);

	CDlgNewFileName dlg;
	dlg.m_strExt = ".N3FXGroup";
	if(dlg.DoModal()==IDOK)
	{
		CString PathName = "fx\\";
		PathName += dlg.m_strNewFileName;
		PathName += dlg.m_strExt;
		CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
		pBaseFileAccess->FileNameSet((LPCTSTR)PathName);

		m_strFileName.Empty();
		m_strFileName = pBaseFileAccess->PathGet().c_str();
		m_strFileName += pBaseFileAccess->FileName().c_str();

		delete pBaseFileAccess;

		UpdateData(FALSE);
		OnBtnSave();
	}		
}

void CDlgEditFxg::OnBtnSave() 
{
	UpdateData(TRUE);

	if(m_strFileName == _T(""))
	{
		OnBtnSaveAs();
		return;
	}

	FILE* file = fopen((LPCTSTR)m_strFileName, "w");
	if(!file)
	{
		MessageBox("n3fxgroup파일 생성 실패..-.-;;","ERR02",MB_OK);
		return;
	}

	fprintf(file,"<N3FXGROUP>\n");

	for(int i=0;i<MAX_FXB;i++)
	{
		if(m_pFXBName[i]->IsEmpty()) continue;

		int joint = m_pJoint[i]->GetCurSel();

		if((*(m_pLoop[i])))	fprintf(file,"<fxb> %s %d TRUE\n", (LPCTSTR)(*(m_pFXBName[i])), joint);
		else fprintf(file,"<fxb> %s %d FALSE\n", (LPCTSTR)(*(m_pFXBName[i])), joint);
	}
	fclose(file);
	
	SaveGameData();
}

void CDlgEditFxg::OnBtnLoadFxb0() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "fxb", nullptr, dwFlags, "fxb File(*.fxb)|*.fxb||", nullptr);
	
	if(dlg.DoModal() == IDCANCEL) return;

	m_strFXBName_0 = "fx\\";
	m_strFXBName_0 = dlg.GetFileName();
	UpdateData(FALSE);
}

void CDlgEditFxg::OnBtnLoadFxb1() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "fxb", nullptr, dwFlags, "fxb File(*.fxb)|*.fxb||", nullptr);
	
	if(dlg.DoModal() == IDCANCEL) return;

	m_strFXBName_1 = "fx\\";
	m_strFXBName_1 = dlg.GetFileName();
	UpdateData(FALSE);
}

void CDlgEditFxg::OnBtnLoadFxb2() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "fxb", nullptr, dwFlags, "fxb File(*.fxb)|*.fxb||", nullptr);
	
	if(dlg.DoModal() == IDCANCEL) return;

	m_strFXBName_2 = "fx\\";
	m_strFXBName_2 = dlg.GetFileName();
	UpdateData(FALSE);
}

void CDlgEditFxg::OnBtnLoadFxb3() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "fxb", nullptr, dwFlags, "fxb File(*.fxb)|*.fxb||", nullptr);
	
	if(dlg.DoModal() == IDCANCEL) return;

	m_strFXBName_3 = "fx\\";
	m_strFXBName_3 = dlg.GetFileName();
	UpdateData(FALSE);
}
