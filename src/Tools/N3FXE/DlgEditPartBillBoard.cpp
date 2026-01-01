// DlgEditPartBillBoard.cpp : implementation file
//

#include "stdafx.h"
#include "N3FXE.h"
#include "MainFrm.h"
#include "DlgEditPartBillBoard.h"
#include <N3Base/N3FXPartBillBoard.h>
#include "DlgNewFileName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartBillBoard dialog


CDlgEditPartBillBoard::CDlgEditPartBillBoard(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgEditPartBillBoard::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditPartBillBoard)
	m_bAlphaBlend = FALSE;
	m_bTexLoop = FALSE;
	m_fAccelX = 0.0f;
	m_fAccelY = 0.0f;
	m_fAccelZ = 0.0f;
	m_fDist4Vol = 0.0f;
	m_fFadeOut = 0.0f;
	m_fLife = 0.0f;
	m_fMoveX = 0.0f;
	m_fMoveY = 0.0f;
	m_fMoveZ = 0.0f;
	m_iNumTex = 0;
	m_fPosX = 0.0f;
	m_fPosY = 0.0f;
	m_fPosZ = 0.0f;
	m_strPathName = _T("");
	m_fScaleX = 0.0f;
	m_fScaleY = 0.0f;
	m_fTexVelocity = 0.0f;
	m_strTexName = _T("");
	m_bRotateY = FALSE;
	m_fFadeIn = 0.0f;
	m_fScaleAccelX = 0.0f;
	m_fScaleAccelY = 0.0f;
	m_fScaleVelX = 0.0f;
	m_fScaleVelY = 0.0f;
	m_bOnGround = FALSE;
	m_fRotVelocity = 0.0f;
	m_bDoubleSide = TRUE;
	m_bLight = FALSE;
	m_bZBuffer = TRUE;
	m_bZWrite = TRUE;
	m_fRotBillBoardX = 0.0f;
	m_fRotBillBoardY = 0.0f;
	m_fRotBillBoardZ = 0.0f;
	//}}AFX_DATA_INIT
}


void CDlgEditPartBillBoard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditPartBillBoard)
	DDX_Control(pDX, IDC_PART_BOARD_CB_SRC_BLEND, m_CBSrcBlend);
	DDX_Control(pDX, IDC_PART_BOARD_CB_DEST_BLEND, m_CBDestBlend);
	DDX_Check(pDX, IDC_PART_BOARD_CHK_ALPHABLEND, m_bAlphaBlend);
	DDX_Check(pDX, IDC_PART_BOARD_CHK_TEX_LOOP, m_bTexLoop);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_ACCEL_X, m_fAccelX);
	DDV_MinMaxFloat(pDX, m_fAccelX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_ACCEL_Y, m_fAccelY);
	DDV_MinMaxFloat(pDX, m_fAccelY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_ACCEL_Z, m_fAccelZ);
	DDV_MinMaxFloat(pDX, m_fAccelZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_DISTANCE4VOLUME, m_fDist4Vol);
	DDV_MinMaxFloat(pDX, m_fDist4Vol, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_FADEOUT, m_fFadeOut);
	DDV_MinMaxFloat(pDX, m_fFadeOut, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_LIFE, m_fLife);
	DDV_MinMaxFloat(pDX, m_fLife, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_MOVE_X, m_fMoveX);
	DDV_MinMaxFloat(pDX, m_fMoveX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_MOVE_Y, m_fMoveY);
	DDV_MinMaxFloat(pDX, m_fMoveY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_MOVE_Z, m_fMoveZ);
	DDV_MinMaxFloat(pDX, m_fMoveZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_NUM_TEX, m_iNumTex);
	DDV_MinMaxInt(pDX, m_iNumTex, 0, 200);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_POSITION_X, m_fPosX);
	DDV_MinMaxFloat(pDX, m_fPosX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_POSITION_Y, m_fPosY);
	DDV_MinMaxFloat(pDX, m_fPosY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_POSITION_Z, m_fPosZ);
	DDV_MinMaxFloat(pDX, m_fPosZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_FILENAME, m_strPathName);
	DDV_MaxChars(pDX, m_strPathName, 256);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_SCALE_X, m_fScaleX);
	DDV_MinMaxFloat(pDX, m_fScaleX, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_SCALE_Y, m_fScaleY);
	DDV_MinMaxFloat(pDX, m_fScaleY, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_TEX_VELOCITY, m_fTexVelocity);
	DDV_MinMaxFloat(pDX, m_fTexVelocity, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_TEXFILENAME, m_strTexName);
	DDV_MaxChars(pDX, m_strTexName, 256);
	DDX_Check(pDX, IDC_PART_BOARD_CHK_YROTATE, m_bRotateY);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_FADEIN, m_fFadeIn);
	DDV_MinMaxFloat(pDX, m_fFadeIn, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_SCALE_ACCEL_X, m_fScaleAccelX);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_SCALE_ACCEL_Y, m_fScaleAccelY);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_SCALE_VEL_X, m_fScaleVelX);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_SCALE_VEL_Y, m_fScaleVelY);
	DDX_Check(pDX, IDC_CHK_ONGROUND, m_bOnGround);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_ROT_VEL, m_fRotVelocity);
	DDX_Check(pDX, IDC_CHK_DOUBLESIDE, m_bDoubleSide);
	DDX_Check(pDX, IDC_CHK_LIGHT, m_bLight);
	DDX_Check(pDX, IDC_CHK_ZBUFFER, m_bZBuffer);
	DDX_Check(pDX, IDC_CHK_ZWRITE, m_bZWrite);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_ROT_X, m_fRotBillBoardX);
	DDV_MinMaxFloat(pDX, m_fRotBillBoardX, -90.f, 90.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_ROT_Y, m_fRotBillBoardY);
	DDV_MinMaxFloat(pDX, m_fRotBillBoardY, -90.f, 90.f);
	DDX_Text(pDX, IDC_PART_BOARD_EDT_ROT_Z, m_fRotBillBoardZ);
	DDV_MinMaxFloat(pDX, m_fRotBillBoardZ, -90.f, 90.f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgEditPartBillBoard, CDialog)
	//{{AFX_MSG_MAP(CDlgEditPartBillBoard)
	ON_BN_CLICKED(IDC_PART_BOARD_BTN_LOAD_TEX, OnPartBoardBtnLoadTex)
	ON_BN_CLICKED(IDC_PART_BOARD_BTN_SAVE, OnPartBoardBtnSave)
	ON_BN_CLICKED(IDC_PART_BOARD_BTN_SAVE_AS, OnPartBoardBtnSaveAs)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartBillBoard message handlers

void CDlgEditPartBillBoard::OnPartBoardBtnLoadTex() 
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

void CDlgEditPartBillBoard::OnPartBoardBtnSave() 
{
	UpdateData(TRUE);

	if(m_strPathName == _T(""))
	{
		OnPartBoardBtnSaveAs();
		return;
	}

	FILE* file = fopen((LPCTSTR)m_strPathName, "w");
	if(!file)
	{
		MessageBox("n3fxpart파일 생성 실패..-.-;;","ERR02",MB_OK);
		return;
	}

	fprintf(file,"<N3FXPART>\n");
	fprintf(file,"<type> board\n");
	if(m_strTexName != _T(""))
		fprintf(file,"<texture> %s %d\n", (LPCTSTR)m_strTexName, m_iNumTex);

	fprintf(file,"<life> %5.4f\n", m_fLife);

	fprintf(file,"<texture_animation_speed> %5.4f\n", m_fTexVelocity);
	fprintf(file,"<position0> %5.4f %5.4f %5.4f\n", m_fPosX, m_fPosY, m_fPosZ);
	fprintf(file,"<distance> %5.4f\n", m_fDist4Vol);
	fprintf(file,"<velocity> %5.4f %5.4f %5.4f\n", m_fMoveX, m_fMoveY, m_fMoveZ);
	fprintf(file,"<rot_velocity> %5.4f 0 0\n", m_fRotVelocity);
	fprintf(file,"<acceleration> %5.4f %5.4f %5.4f\n", m_fAccelX, m_fAccelY, m_fAccelZ);

	if(m_bOnGround) fprintf(file,"<on_ground> true\n");
	else fprintf(file,"<on_ground> false\n");
	
	if(m_bAlphaBlend) fprintf(file,"<alpha> true\n");
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

	fprintf(file,"<billboard_count> 1\n");

	fprintf(file,"<billboard_size> %5.4f %5.4f\n", m_fScaleX, m_fScaleY);
	
	if(m_bTexLoop) fprintf(file,"<tex_loop> true\n");
	else fprintf(file,"<tex_loop> false\n");

	if(m_bRotateY) fprintf(file,"<Rotate_only_y> true\n");
	else fprintf(file,"<Rotate_only_y> false\n");

	fprintf(file,"<billboard_scale_velocity> %5.4f %5.4f\n", m_fScaleVelX, m_fScaleVelY);
	fprintf(file,"<billboard_scale_acceleration> %5.4f %5.4f\n", m_fScaleAccelX, m_fScaleAccelY);

	fprintf(file,"<billboard_Rotation> %5.4f %5.4f %5.4f\n", m_fRotBillBoardX, m_fRotBillBoardY, m_fRotBillBoardZ);

	fprintf(file,"<end>\n");
	fclose(file);	
}

void CDlgEditPartBillBoard::OnPartBoardBtnSaveAs() 
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
		OnPartBoardBtnSave();
	}		
}

void CDlgEditPartBillBoard::OnOK() 
{
	return;	
	CDialog::OnOK();
}

void CDlgEditPartBillBoard::OnClose() 
{
	CDialog::OnClose();
	m_pRefFrm->DestroyPartBillBoard(this);
}

BOOL CDlgEditPartBillBoard::OnInitDialog() 
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

bool CDlgEditPartBillBoard::LoadPartScript(const char* szPath)
{
	m_strPathName = szPath;
	CN3FXPartBillBoard* pPart = new CN3FXPartBillBoard;
	if(!pPart->DecodeScriptFile(szPath))
	{
		delete pPart;
		return false;
	}

	//////////////////////////////////////////////////
	//각 컨트롤 셋팅...
	m_bOnGround = pPart->m_bOnGround;
	m_bAlphaBlend = pPart->m_bAlpha;
	m_bTexLoop = pPart->m_bTexLoop;

	if(D3DCULL_NONE==pPart->m_dwDoubleSide) m_bDoubleSide = TRUE;
	else m_bDoubleSide = FALSE;

	m_bLight = pPart->m_dwLight;
	m_bZBuffer = pPart->m_dwZEnable;
	m_bZWrite = pPart->m_dwZWrite;

	m_fAccelX = pPart->m_vAcceleration.x;
	m_fAccelY = pPart->m_vAcceleration.y;
	m_fAccelZ = pPart->m_vAcceleration.z;
	
	m_fDist4Vol = pPart->m_fRadius;
	
	m_fFadeOut = pPart->m_fFadeOut;
	m_fFadeIn = pPart->m_fFadeIn;
	
	m_fLife = pPart->m_fLife;
	m_fRotVelocity = pPart->m_vRotVelocity.x;
	
	m_fMoveX = pPart->m_vVelocity.x;
	m_fMoveY = pPart->m_vVelocity.y;
	m_fMoveZ = pPart->m_vVelocity.z;
	
	m_iNumTex = pPart->m_iNumTex;
	
	m_fPosX = pPart->m_vPos.x;
	m_fPosY = pPart->m_vPos.y;
	m_fPosZ = pPart->m_vPos.z;
	
	m_fScaleX = pPart->m_fSizeX;
	m_fScaleY = pPart->m_fSizeY;
	
	m_fTexVelocity = pPart->m_fTexFPS;

	m_bRotateY = pPart->m_bRoateOnlyY;

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

	m_fScaleAccelX = pPart->m_fScaleAccelX;
	m_fScaleAccelY = pPart->m_fScaleAccelY;
	m_fScaleVelX = pPart->m_fScaleVelX;
	m_fScaleVelY = pPart->m_fScaleVelY;

	m_fRotBillBoardX = pPart->m_fRotBillBoardX;
	m_fRotBillBoardY = pPart->m_fRotBillBoardY;
	m_fRotBillBoardZ = pPart->m_fRotBillBoardZ;

	UpdateData(FALSE);
	//
	//////////////////////////////////////////////////

	delete pPart;
	return true;
}
