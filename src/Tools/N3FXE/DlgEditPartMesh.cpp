// DlgEditPartMesh.cpp : implementation file
//

#include "stdafx.h"
#include "N3FXE.h"
#include "MainFrm.h"
#include "DlgEditPartMesh.h"
#include <N3Base/N3FXPartMesh.h>
#include <N3Base/N3FXShape.h>
#include "DlgNewFileName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartMesh dialog


CDlgEditPartMesh::CDlgEditPartMesh(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgEditPartMesh::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditPartMesh)
	m_strPathName = _T("");
	m_bAlphaBlend = FALSE;
	m_bTexLoop = FALSE;
	m_strShapeName = _T("");
	m_fTexUVVelocity = 0.0f;
	m_fScaleZ = 0.0f;
	m_fScaleY = 0.0f;
	m_fScaleX = 0.0f;
	m_fAccelX = 0.0f;
	m_fAccelY = 0.0f;
	m_fAccelZ = 0.0f;
	m_fBaseScaleX = 0.0f;
	m_fBaseScaleY = 0.0f;
	m_fBaseScaleZ = 0.0f;
	m_fFadeOut = 0.0f;
	m_fLife = 0.0f;
	m_fMoveX = 0.0f;
	m_fMoveY = 0.0f;
	m_fMoveZ = 0.0f;
	m_fPosX = 0.0f;
	m_fPosY = 0.0f;
	m_fPosZ = 0.0f;
	m_fRotX = 0.0f;
	m_fRotY = 0.0f;
	m_fRotZ = 0.0f;
	m_fScaleAccelX = 0.0f;
	m_fScaleAccelY = 0.0f;
	m_fScaleAccelZ = 0.0f;
	m_fMeshFPS = 0.0f;
	m_fTexVelocity = 0.0f;
	m_fFadeIn = 0.0f;
	m_bOnGround = FALSE;
	m_bDoubleSide = TRUE;
	m_bLight = FALSE;
	m_bZBuffer = TRUE;
	m_bZWrite = TRUE;
	//}}AFX_DATA_INIT
}


void CDlgEditPartMesh::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditPartMesh)
	DDX_Control(pDX, IDC_PART_MESH_CB_TEX_UV_MOVE, m_CBTexUVMove);
	DDX_Control(pDX, IDC_PART_MESH_CB_SRC_BLEND, m_CBSrcBlend);
	DDX_Control(pDX, IDC_PART_MESH_CB_DEST_BLEND, m_CBDestBlend);
	DDX_Text(pDX, IDC_PART_MESH_FILENAME, m_strPathName);
	DDV_MaxChars(pDX, m_strPathName, 256);
	DDX_Check(pDX, IDC_PART_MESH_CHK_ALPHABLEND, m_bAlphaBlend);
	DDX_Check(pDX, IDC_PART_MESH_CHK_TEX_LOOP, m_bTexLoop);
	DDX_Text(pDX, IDC_PART_MESH_SHAPEFILENAME, m_strShapeName);
	DDV_MaxChars(pDX, m_strShapeName, 256);
	DDX_Text(pDX, IDC_PART_MESH_EDT_TEX_UV_VELOCITY, m_fTexUVVelocity);
	DDV_MinMaxFloat(pDX, m_fTexUVVelocity, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_SCALE_Z, m_fScaleZ);
	DDV_MinMaxFloat(pDX, m_fScaleZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_SCALE_Y, m_fScaleY);
	DDV_MinMaxFloat(pDX, m_fScaleY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_SCALE_X, m_fScaleX);
	DDV_MinMaxFloat(pDX, m_fScaleX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_ACCEL_X, m_fAccelX);
	DDV_MinMaxFloat(pDX, m_fAccelX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_ACCEL_Y, m_fAccelY);
	DDV_MinMaxFloat(pDX, m_fAccelY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_ACCEL_Z, m_fAccelZ);
	DDV_MinMaxFloat(pDX, m_fAccelZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_BASESCALE_X, m_fBaseScaleX);
	DDV_MinMaxFloat(pDX, m_fBaseScaleX, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_BASESCALE_Y, m_fBaseScaleY);
	DDV_MinMaxFloat(pDX, m_fBaseScaleY, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_BASESCALE_Z, m_fBaseScaleZ);
	DDV_MinMaxFloat(pDX, m_fBaseScaleZ, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_FADEOUT, m_fFadeOut);
	DDV_MinMaxFloat(pDX, m_fFadeOut, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_LIFE, m_fLife);
	DDV_MinMaxFloat(pDX, m_fLife, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_MOVE_X, m_fMoveX);
	DDV_MinMaxFloat(pDX, m_fMoveX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_MOVE_Y, m_fMoveY);
	DDV_MinMaxFloat(pDX, m_fMoveY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_MOVE_Z, m_fMoveZ);
	DDV_MinMaxFloat(pDX, m_fMoveZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_POSITION_X, m_fPosX);
	DDV_MinMaxFloat(pDX, m_fPosX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_POSITION_Y, m_fPosY);
	DDV_MinMaxFloat(pDX, m_fPosY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_POSITION_Z, m_fPosZ);
	DDV_MinMaxFloat(pDX, m_fPosZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_ROT_X, m_fRotX);
	DDV_MinMaxFloat(pDX, m_fRotX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_ROT_Y, m_fRotY);
	DDV_MinMaxFloat(pDX, m_fRotY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_ROT_Z, m_fRotZ);
	DDV_MinMaxFloat(pDX, m_fRotZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_SCALEACCEL_X, m_fScaleAccelX);
	DDX_Text(pDX, IDC_PART_MESH_EDT_SCALEACCEL_Y, m_fScaleAccelY);
	DDX_Text(pDX, IDC_PART_MESH_EDT_SCALEACCEL_Z, m_fScaleAccelZ);
	DDX_Text(pDX, IDC_PART_MESH_EDT_MESH_VELOCITY, m_fMeshFPS);
	DDV_MinMaxFloat(pDX, m_fMeshFPS, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_MESH_TEX_VEL, m_fTexVelocity);
	DDV_MinMaxFloat(pDX, m_fTexVelocity, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_MESH_EDT_FADEIN, m_fFadeIn);
	DDV_MinMaxFloat(pDX, m_fFadeIn, 0.f, 1.e+007f);
	DDX_Check(pDX, IDC_CHK_ONGROUND, m_bOnGround);
	DDX_Check(pDX, IDC_CHK_DOUBLESIDE, m_bDoubleSide);
	DDX_Check(pDX, IDC_CHK_LIGHT, m_bLight);
	DDX_Check(pDX, IDC_CHK_ZBUFFER, m_bZBuffer);
	DDX_Check(pDX, IDC_CHK_ZWRITE, m_bZWrite);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgEditPartMesh, CDialog)
	//{{AFX_MSG_MAP(CDlgEditPartMesh)
	ON_BN_CLICKED(IDC_PART_MESH_BTN_SAVE, OnPartMeshBtnSave)
	ON_BN_CLICKED(IDC_PART_MESH_BTN_SAVE_AS, OnPartMeshBtnSaveAs)
	ON_BN_CLICKED(IDC_PART_MESH_BTN_LOAD_SHAPE, OnPartMeshBtnLoadShape)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartMesh message handlers

void CDlgEditPartMesh::OnPartMeshBtnSave() 
{
	UpdateData(TRUE);

	if(m_strPathName == _T(""))
	{
		OnPartMeshBtnSaveAs();
		return;
	}

	FILE* file = fopen((LPCTSTR)m_strPathName, "w");
	if(!file)
	{
		MessageBox("n3fxpart파일 생성 실패..-.-;;","ERR02",MB_OK);
		return;
	}

	fprintf(file,"<N3FXPART>\n");
	fprintf(file,"<type> mesh\n");
	fprintf(file,"<shape_name> %s\n", (LPCTSTR)m_strShapeName);
	CString TexUVMove;
	m_CBTexUVMove.GetLBText(m_CBTexUVMove.GetCurSel(), TexUVMove);
	fprintf(file,"<texture_move> %s %5.4f\n", (LPCTSTR)TexUVMove, m_fTexUVVelocity);
	fprintf(file,"<scale_velocity> %5.4f %5.4f %5.4f\n", m_fScaleX, m_fScaleY, m_fScaleZ);
	fprintf(file,"<scale> %5.4f %5.4f %5.4f\n", m_fBaseScaleX, m_fBaseScaleY, m_fBaseScaleZ);
	fprintf(file,"<scale_accelerate> %5.4f %5.4f %5.4f\n", m_fScaleAccelX, m_fScaleAccelY, m_fScaleAccelZ);
	
	fprintf(file,"<tex_fps> %5.4f\n", m_fTexVelocity);

	if(m_bTexLoop) fprintf(file,"<tex_loop> true\n");
	else fprintf(file,"<tex_loop> false\n");

	if(m_bAlphaBlend) fprintf(file,"<alpha> true\n");
	else fprintf(file,"<alpha> false\n");

	if(m_bOnGround) fprintf(file,"<on_ground> true\n");
	else fprintf(file,"<on_ground> false\n");

	if(m_bDoubleSide) fprintf(file,"<doubleside> true\n");
	else fprintf(file,"<doubleside> false\n");

	if(m_bLight) fprintf(file,"<light> true\n");
	else fprintf(file,"<light> false\n");

	if(m_bZBuffer) fprintf(file,"<zbuffer> true\n");
	else fprintf(file,"<zbuffer> false\n");

	if(m_bZWrite) fprintf(file,"<zwrite> true\n");
	else fprintf(file,"<zwrite> false\n");

	fprintf(file,"<mesh_fps> %5.4f\n", m_fMeshFPS);

	fprintf(file,"<acceleration> %5.4f %5.4f %5.4f\n", m_fAccelX, m_fAccelY, m_fAccelZ);
	fprintf(file,"<position0> %5.4f %5.4f %5.4f\n", m_fPosX, m_fPosY, m_fPosZ);
	fprintf(file,"<velocity> %5.4f %5.4f %5.4f\n", m_fMoveX, m_fMoveY, m_fMoveZ);
	fprintf(file,"<rot_velocity> %5.4f %5.4f %5.4f\n", m_fRotX, m_fRotY, m_fRotZ);

	fprintf(file,"<life> %5.4f\n", m_fLife);
	fprintf(file,"<fadeout> %5.4f\n", m_fFadeOut);
	fprintf(file,"<fadein> %5.4f\n", m_fFadeIn);

	CString strBlend;
	m_CBSrcBlend.GetLBText(m_CBSrcBlend.GetCurSel(), strBlend);
	fprintf(file,"<src_blend> %s\n", (LPCTSTR)strBlend);

	m_CBDestBlend.GetLBText(m_CBDestBlend.GetCurSel(), strBlend);
	fprintf(file,"<dest_blend> %s\n", (LPCTSTR)strBlend);

	fprintf(file,"<end>\n");
	fclose(file);
}

void CDlgEditPartMesh::OnPartMeshBtnSaveAs() 
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
		OnPartMeshBtnSave();
	}
}

bool CDlgEditPartMesh::LoadPartScript(const char* szPath)
{
	m_strPathName = szPath;
	CN3FXPartMesh* pPart = new CN3FXPartMesh;
	if(!pPart->DecodeScriptFile(szPath))
	{
		delete pPart;
		return false;
	}

	//////////////////////////////////////////////////
	//각 컨트롤 셋팅...
	m_bOnGround = pPart->m_bOnGround;
	m_bAlphaBlend = pPart->m_bAlpha;

	if(D3DCULL_NONE==pPart->m_dwDoubleSide) m_bDoubleSide = TRUE;
	else m_bDoubleSide = FALSE;

	m_bLight = pPart->m_dwLight;
	m_bZBuffer = pPart->m_dwZEnable;
	m_bZWrite = pPart->m_dwZWrite;

	m_bTexLoop = pPart->m_bTexLoop;
	m_strShapeName = pPart->m_pShape->FileName().c_str();
	m_fTexVelocity = pPart->m_fTexFPS;

	if(pPart->m_cTextureMoveDir==1) m_fTexUVVelocity = pPart->m_fv;
	else if(pPart->m_cTextureMoveDir==2 ) m_fTexUVVelocity = -pPart->m_fv;
	else if(pPart->m_cTextureMoveDir==3) m_fTexUVVelocity = pPart->m_fu;
	else if(pPart->m_cTextureMoveDir==4 ) m_fTexUVVelocity = -pPart->m_fu;
	else m_fTexUVVelocity = 0.0f;

	m_fMeshFPS = pPart->m_fMeshFPS;

	m_fScaleZ = pPart->m_vScaleVel.z;
	m_fScaleY = pPart->m_vScaleVel.y;
	m_fScaleX = pPart->m_vScaleVel.x;

	m_fScaleAccelX = pPart->m_vScaleAccel.x;
	m_fScaleAccelY = pPart->m_vScaleAccel.y;
	m_fScaleAccelZ = pPart->m_vScaleAccel.z;

	m_fAccelX = pPart->m_vAcceleration.x;
	m_fAccelY = pPart->m_vAcceleration.y;
	m_fAccelZ = pPart->m_vAcceleration.z;

	m_fBaseScaleX = pPart->m_vUnitScale.x;
	m_fBaseScaleY = pPart->m_vUnitScale.y;
	m_fBaseScaleZ = pPart->m_vUnitScale.z;

	m_fFadeOut = pPart->m_fFadeOut;
	m_fFadeIn = pPart->m_fFadeIn;
	m_fLife = pPart->m_fLife;
	m_fMoveX = pPart->m_vVelocity.x;
	m_fMoveY = pPart->m_vVelocity.y;
	m_fMoveZ = pPart->m_vVelocity.z;

	m_fPosX = pPart->m_vPos.x;
	m_fPosY = pPart->m_vPos.y;
	m_fPosZ = pPart->m_vPos.z;
	
	m_fRotX = pPart->m_vRotVelocity.x;
	m_fRotY = pPart->m_vRotVelocity.y;
	m_fRotZ = pPart->m_vRotVelocity.z;

	m_CBTexUVMove.SetCurSel((int)pPart->m_cTextureMoveDir);
	m_CBSrcBlend.SetCurSel((int)pPart->m_dwSrcBlend-1);
	m_CBDestBlend.SetCurSel((int)pPart->m_dwDestBlend-1);

	UpdateData(FALSE);
	//
	//////////////////////////////////////////////////

	delete pPart;
	return true;
}

BOOL CDlgEditPartMesh::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_CBTexUVMove.ResetContent();
	m_CBTexUVMove.InsertString(0, "NONE");
	m_CBTexUVMove.InsertString(1, "UP");
	m_CBTexUVMove.InsertString(2, "DOWN");
	m_CBTexUVMove.InsertString(3, "LEFT");
	m_CBTexUVMove.InsertString(4, "RIGHT");
	m_CBTexUVMove.SetCurSel(0);

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

void CDlgEditPartMesh::OnOK() 
{
	// TODO: Add extra validation here
	return;
	CDialog::OnOK();
}

void CDlgEditPartMesh::OnPartMeshBtnLoadShape() 
{
	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
	CFileDialog dlg(TRUE, "N3Shape", nullptr, dwFlags, "N3Shape File(*.n3shape)|*.n3shape||", nullptr);
	if(dlg.DoModal() == IDCANCEL) return;
	
	CString PathName = dlg.GetPathName();

	CN3BaseFileAccess* pBaseFileAccess = new CN3BaseFileAccess;
	pBaseFileAccess->FileNameSet((LPCTSTR)PathName);
	PathName = pBaseFileAccess->FileName().c_str();

	if( (PathName[0]=='F' || PathName[0]=='f') &&
		(PathName[1]=='X' || PathName[1]=='x') &&
		(PathName[2]=='/' || PathName[2]=='\\') )
	{
		m_strShapeName = PathName;
		UpdateData(FALSE);		
	}
	else
	{
		MessageBox("N3Shape파일은 fx폴더 아래, 혹은 fx폴더 아래에 있는 폴더에 위치해야 합니다..-.-;;","ERR03",MB_OK);
	}

	delete pBaseFileAccess;			
}

void CDlgEditPartMesh::OnClose() 
{
	CDialog::OnClose();
	m_pRefFrm->DestroyPartMesh(this);		
}
