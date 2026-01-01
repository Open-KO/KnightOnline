// DlgEditPartGround.cpp : implementation file
//

#include "stdafx.h"
#include "n3fxe.h"
#include "MainFrm.h"
#include "DlgEditPartGround.h"
#include <N3Base/N3FXPartBottomBoard.h>
#include "DlgNewFileName.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartGround dialog


CDlgEditPartGround::CDlgEditPartGround(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgEditPartGround::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditPartGround)
	m_chkAlpha = FALSE;
	m_chkTexLoop = FALSE;
	m_fAccelX = 0.0f;
	m_fAccelZ = 0.0f;
	m_fFadeIn = 0.0f;
	m_fFadeOut = 0.0f;
	m_fLife = 0.0f;
	m_fMoveX = 0.0f;
	m_fMoveZ = 0.0f;
	m_iNumTex = 0;
	m_fPosX = 0.0f;
	m_fPosZ = 0.0f;
	m_fRot = 0.0f;
	m_fScaleAccelX = 0.0f;
	m_fScaleAccelZ = 0.0f;
	m_fScaleVelX = 0.0f;
	m_fScaleVelZ = 0.0f;
	m_fScaleX = 0.0f;
	m_fScaleZ = 0.0f;
	m_fTexVelocity = 0.0f;
	m_strTexName = _T("");
	m_strPathName = _T("");
	m_fGap = 0.0f;
	m_bDoubleSide = TRUE;
	m_bLight = FALSE;
	m_bZBuffer = TRUE;
	m_bZWrite = TRUE;
	//}}AFX_DATA_INIT
}


void CDlgEditPartGround::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditPartGround)
	DDX_Control(pDX, IDC_PART_GROUND_CB_SRC_BLEND, m_CBSrcBlend);
	DDX_Control(pDX, IDC_PART_GROUND_CB_DEST_BLEND, m_CBDestBlend);
	DDX_Check(pDX, IDC_PART_GROUND_CHK_ALPHABLEND, m_chkAlpha);
	DDX_Check(pDX, IDC_PART_GROUND_CHK_TEX_LOOP, m_chkTexLoop);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_ACCEL_X, m_fAccelX);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_ACCEL_Z, m_fAccelZ);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_FADEIN, m_fFadeIn);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_FADEOUT, m_fFadeOut);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_LIFE, m_fLife);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_MOVE_X, m_fMoveX);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_MOVE_Z, m_fMoveZ);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_NUM_TEX, m_iNumTex);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_POSITION_X, m_fPosX);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_POSITION_Z, m_fPosZ);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_ROTATE_VELOCITY, m_fRot);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_SCALE_ACCEL_X, m_fScaleAccelX);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_SCALE_ACCEL_Z, m_fScaleAccelZ);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_SCALE_VEL_X, m_fScaleVelX);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_SCALE_VEL_Z, m_fScaleVelZ);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_SCALE_X, m_fScaleX);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_SCALE_Z, m_fScaleZ);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_TEX_VELOCITY, m_fTexVelocity);
	DDX_Text(pDX, IDC_PART_GROUND_TEXFILENAME, m_strTexName);
	DDX_Text(pDX, IDC_PART_GROUND_FILENAME, m_strPathName);
	DDX_Text(pDX, IDC_PART_GROUND_EDT_GAP, m_fGap);
	DDV_MinMaxFloat(pDX, m_fGap, 0.f, 10000.f);
	DDX_Check(pDX, IDC_CHK_DOUBLESIDE, m_bDoubleSide);
	DDX_Check(pDX, IDC_CHK_LIGHT, m_bLight);
	DDX_Check(pDX, IDC_CHK_ZBUFFER, m_bZBuffer);
	DDX_Check(pDX, IDC_CHK_ZWRITE, m_bZWrite);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgEditPartGround, CDialog)
	//{{AFX_MSG_MAP(CDlgEditPartGround)
	ON_BN_CLICKED(IDC_PART_GROUND_BTN_LOAD_TEX, OnPartGroundBtnLoadTex)
	ON_BN_CLICKED(IDC_PART_GROUND_BTN_SAVE, OnPartGroundBtnSave)
	ON_BN_CLICKED(IDC_PART_GROUND_BTN_SAVE_AS, OnPartGroundBtnSaveAs)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartGround message handlers

bool CDlgEditPartGround::LoadPartScript(const char* szPath)
{
	m_strPathName = szPath;
	CN3FXPartBottomBoard* pPart = new CN3FXPartBottomBoard;
	if(!pPart->DecodeScriptFile(szPath))
	{
		delete pPart;
		return false;
	}

	//////////////////////////////////////////////////
	//각 컨트롤 셋팅...
	m_chkAlpha = pPart->m_bAlpha;
	m_chkTexLoop = pPart->m_bTexLoop;

	if(D3DCULL_NONE==pPart->m_dwDoubleSide) m_bDoubleSide = TRUE;
	else m_bDoubleSide = FALSE;

	m_bLight = pPart->m_dwLight;
	m_bZBuffer = pPart->m_dwZEnable;
	m_bZWrite = pPart->m_dwZWrite;

	m_fAccelX = pPart->m_vAcceleration.x;
	m_fAccelZ = pPart->m_vAcceleration.z;

	m_fFadeOut = pPart->m_fFadeOut;
	m_fFadeIn = pPart->m_fFadeIn;
	
	m_fLife = pPart->m_fLife;
	
	m_fMoveX = pPart->m_vVelocity.x;
	m_fMoveZ = pPart->m_vVelocity.z;
	
	m_iNumTex = pPart->m_iNumTex;
	
	m_fPosX = pPart->m_vPos.x;
	m_fPosZ = pPart->m_vPos.z;

	m_fRot = pPart->m_vRotVelocity.y;
	
	m_fScaleX = pPart->m_fSizeX;
	m_fScaleZ = pPart->m_fSizeZ;

	m_fScaleAccelX = pPart->m_fScaleAccelX;
	m_fScaleAccelZ = pPart->m_fScaleAccelZ;

	m_fScaleVelX = pPart->m_fScaleVelX;
	m_fScaleVelZ = pPart->m_fScaleVelZ;

	m_fTexVelocity = pPart->m_fTexFPS;

	m_fGap = pPart->m_fGap;

	if(m_iNumTex>0)
	{
		char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
		_splitpath((LPCTSTR)pPart->m_ppRefTex[0]->FileName().c_str(), szDrive, szDir, szFName, szExt);

		CString strFName = szFName;
		strFName.TrimRight("0000");

		char szPath[_MAX_PATH];
		_makepath(szPath, szDrive, szDir, (LPCTSTR)strFName, szExt);
		m_strTexName = szPath;		
	}
	else m_strTexName = _T("");
	
	m_CBSrcBlend.SetCurSel((int)pPart->m_dwSrcBlend-1);
	m_CBDestBlend.SetCurSel((int)pPart->m_dwDestBlend-1);
		
	UpdateData(FALSE);
	//
	//////////////////////////////////////////////////

	delete pPart;
	return true;
}

void CDlgEditPartGround::OnPartGroundBtnSave() 
{
	UpdateData(TRUE);

	if(m_strPathName == _T(""))
	{
		OnPartGroundBtnSaveAs();
		return;
	}

	FILE* file = fopen((LPCTSTR)m_strPathName, "w");
	if(!file)
	{
		MessageBox("n3fxpart파일 생성 실패..-.-;;","ERR02",MB_OK);
		return;
	}

	fprintf(file,"<N3FXPART>\n");
	fprintf(file,"<type> ground\n");
	if(m_strTexName != _T(""))
		fprintf(file,"<texture> %s %d\n", (LPCTSTR)m_strTexName, m_iNumTex);

	fprintf(file,"<life> %5.4f\n", m_fLife);

	fprintf(file,"<texture_animation_speed> %5.4f\n", m_fTexVelocity);
	fprintf(file,"<position0> %5.4f %5.4f %5.4f\n", m_fPosX, 0.0f, m_fPosZ);
	fprintf(file,"<velocity> %5.4f %5.4f %5.4f\n", m_fMoveX, 0.0f, m_fMoveZ);
	fprintf(file,"<acceleration> %5.4f %5.4f %5.4f\n", m_fAccelX, 0.0f, m_fAccelZ);
	fprintf(file,"<rot_velocity> %5.4f %5.4f %5.4f\n", 0.0f, m_fRot, 0.0f);
	
	if(m_chkAlpha) fprintf(file,"<alpha> true\n");
	else fprintf(file,"<alpha> false\n");

	if(m_bDoubleSide) fprintf(file,"<doubleside> true\n");
	else fprintf(file,"<doubleside> false\n");

	if(m_bLight) fprintf(file,"<light> true\n");
	else fprintf(file,"<light> false\n");

	if(m_bZBuffer) fprintf(file,"<zbuffer> true\n");
	else fprintf(file,"<zbuffer> false\n");

	if(m_bZWrite) fprintf(file,"<zwrite> true\n");
	else fprintf(file,"<zwrite> false\n");

	CString strBlend;
	m_CBSrcBlend.GetLBText(m_CBSrcBlend.GetCurSel(), strBlend);
	fprintf(file,"<src_blend> %s\n", (LPCTSTR)strBlend);

	m_CBDestBlend.GetLBText(m_CBDestBlend.GetCurSel(), strBlend);
	fprintf(file,"<dest_blend> %s\n", (LPCTSTR)strBlend);

	fprintf(file,"<fadeout> %5.4f\n", m_fFadeOut);
	
	fprintf(file,"<fadein> %5.4f\n", m_fFadeIn);

	fprintf(file,"<ground_size> %5.4f %5.4f\n", m_fScaleX, m_fScaleZ);
		
	if(m_chkTexLoop) fprintf(file,"<tex_loop> true\n");
	else fprintf(file,"<tex_loop> false\n");

	fprintf(file,"<ground_scale_velocity> %5.4f %5.4f\n", m_fScaleVelX, m_fScaleVelZ);
	fprintf(file,"<ground_scale_acceleration> %5.4f %5.4f\n", m_fScaleAccelX, m_fScaleAccelZ);
	fprintf(file,"<ground_gap> %5.4f\n", m_fGap);

	fprintf(file,"<end>\n");
	fclose(file);
}

void CDlgEditPartGround::OnPartGroundBtnLoadTex() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, nullptr, nullptr, dwFlags, "TGA (*.tga),BMP (*.bmp), DXT (*.dxt) |*.tga;*.bmp;*.dxt||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;
	
	CString PathName = dlg.GetPathName();

	CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
	pBaseFileAccess->FileNameSet((LPCTSTR)PathName);
	PathName = pBaseFileAccess->FileName().c_str();

	if( (PathName[0]=='F' || PathName[0]=='f') &&
		(PathName[1]=='X' || PathName[1]=='x') &&
		(PathName[2]=='/' || PathName[2]=='\\') )
	{
		char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
		_splitpath((LPCTSTR)PathName, szDrive, szDir, szFName, szExt);

		CString strFName = szFName;
		if(strFName.Right(4) == _T("0000"))
		{
			strFName.TrimRight("0000");
			char szPath[_MAX_PATH];
			_makepath(szPath, szDrive, szDir, (LPCTSTR)strFName, szExt);
			m_strTexName = szPath;

			//파일 갯수 세는 기능 넣을까 말까..
		}
		else
		{
			m_strTexName = _T("");
			MessageBox("파일 이름끝이 0000이 아니던데요..-.-;;","ERR05",MB_OK);
		}
		
		UpdateData(FALSE);
	}
	else
		MessageBox("Texture파일은 fx폴더 아래, 혹은 fx폴더 아래에 있는 폴더에 위치해야 합니다..-.-;;","ERR04",MB_OK);

	delete pBaseFileAccess;	
}

void CDlgEditPartGround::OnPartGroundBtnSaveAs() 
{
	UpdateData(TRUE);

	CDlgNewFileName dlg;
	dlg.m_strExt = ".N3FXPart";
	if(dlg.DoModal()==IDOK)
	{
		CString PathName = "fx\\";
		PathName += dlg.m_strNewFileName;
		PathName += dlg.m_strExt;
		CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
		pBaseFileAccess->FileNameSet((LPCTSTR)PathName);

		m_strPathName.Empty();
		m_strPathName = pBaseFileAccess->PathGet().c_str();
		m_strPathName += pBaseFileAccess->FileName().c_str();

		delete pBaseFileAccess;

		UpdateData(FALSE);
		OnPartGroundBtnSave();
	}
}

void CDlgEditPartGround::OnClose() 
{
	CDialog::OnClose();
	m_pRefFrm->DestroyPartGround(this);
}

BOOL CDlgEditPartGround::OnInitDialog() 
{
	CDialog::OnInitDialog();

	/*
	D3DBLEND_ZERO               =  1,
    D3DBLEND_ONE                =  2,
    D3DBLEND_SRCCOLOR           =  3,
    D3DBLEND_INVSRCCOLOR        =  4,
    D3DBLEND_SRCALPHA           =  5,
    D3DBLEND_INVSRCALPHA        =  6,
    */
	m_CBSrcBlend.ResetContent();
	m_CBSrcBlend.InsertString(0, "ZERO");
	m_CBSrcBlend.InsertString(1, "ONE");
	m_CBSrcBlend.InsertString(2, "SRCCOLOR");
	m_CBSrcBlend.InsertString(3, "INVSRCCOLOR");
	m_CBSrcBlend.InsertString(4, "SRCALPHA");
	m_CBSrcBlend.InsertString(5, "INVSRCALPHA");
	m_CBSrcBlend.SetCurSel(1);

	m_CBDestBlend.ResetContent();
	m_CBDestBlend.InsertString(0, "ZERO");
	m_CBDestBlend.InsertString(1, "ONE");
	m_CBDestBlend.InsertString(2, "SRCCOLOR");
	m_CBDestBlend.InsertString(3, "INVSRCCOLOR");
	m_CBDestBlend.InsertString(4, "SRCALPHA");
	m_CBDestBlend.InsertString(5, "INVSRCALPHA");
	m_CBDestBlend.SetCurSel(1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgEditPartGround::OnOK() 
{
	return;	
	CDialog::OnOK();
}
