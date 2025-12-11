// DlgEditPartParticle.cpp : implementation file
//

#include "stdafx.h"
#include "N3FXE.h"
#include "MainFrm.h"
#include <N3Base/N3FXPartParticles.h>
#include <N3Base/N3FXShape.h>
#include "DlgNewFileName.h"
#include "DlgEditPartParticle.h"
#include "DlgPutColor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartParticle dialog


CDlgEditPartParticle::CDlgEditPartParticle(CWnd* pParent /*=nullptr*/)
	: CDialog(CDlgEditPartParticle::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEditPartParticle)
	m_bAlphaBlend = FALSE;
	m_fAccelX = 0.0f;
	m_fAccelY = 0.0f;
	m_fAccelZ = 0.0f;
	m_fFadeOut = 0.0f;
	m_fCreateInterval = 0.0f;
	m_fLife = 0.0f;
	m_fMoveX = 0.0f;
	m_fMoveY = 0.0f;
	m_fMoveZ = 0.0f;
	m_iCreateNum = 0;
	m_iNumParticle = 0;
	m_iNumTex = 0;
	m_fPosX = 0.0f;
	m_fPosY = 0.0f;
	m_fPosZ = 0.0f;
	m_fSize = 0.0f;
	m_fTexVelocity = 0.0f;
	m_strPathName = _T("");
	m_strTexName = _T("");
	m_fEmitAccel = 0.0f;
	m_fEmitGravity = 0.0f;
	m_fEmitRot = 0.0f;
	m_fEmitVel = 0.0f;
	m_fCreateMaxX = 0.0f;
	m_fCreateMaxY = 0.0f;
	m_fCreateMaxZ = 0.0f;
	m_fCreateMinX = 0.0f;
	m_fCreateMinY = 0.0f;
	m_fCreateMinZ = 0.0f;
	m_fParticleLifeMin = 0.0f;
	m_fParticleLifeMax = 0.0f;
	m_fEmitArg1 = 0.0f;
	m_fEmitArg2 = 0.0f;
	m_fEmitArg3 = 0.0f;
	m_EmitDirX = 0.0f;
	m_EmitDirY = 0.0f;
	m_EmitDirZ = 0.0f;
	m_bChangeColor = FALSE;
	m_strShapeName = _T("");
	m_fShapeFPS = 0.0f;
	m_bAnimKey = FALSE;
	m_fFadeIn = 0.0f;
	m_fSizeMax = 0.0f;
	m_fSizeMin = 0.0f;
	m_bOnGround = FALSE;
	m_bDoubleSide = TRUE;
	m_bLight = FALSE;
	m_bZBuffer = TRUE;
	m_bZWrite = TRUE;
	m_fTexRotVelocity = 0.0f;
	m_fScaleVelX = 0.0f;
	m_fScaleVelY = 0.0f;
	//}}AFX_DATA_INIT
}


void CDlgEditPartParticle::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEditPartParticle)
	DDX_Control(pDX, IDC_PART_PARTICLE_CB_EMIT_TYPE, m_CBEmitType);
	DDX_Control(pDX, IDC_PART_PARTICLE_CB_SRC_BLEND, m_CBSrcBlend);
	DDX_Control(pDX, IDC_PART_PARTICLE_CB_DEST_BLEND, m_CBDestBlend);
	DDX_Check(pDX, IDC_PART_PARTICLE_CHK_ALPHABLEND, m_bAlphaBlend);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_ACCEL_X, m_fAccelX);
	DDV_MinMaxFloat(pDX, m_fAccelX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_ACCEL_Y, m_fAccelY);
	DDV_MinMaxFloat(pDX, m_fAccelY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_ACCEL_Z, m_fAccelZ);
	DDV_MinMaxFloat(pDX, m_fAccelZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_FADEOUT, m_fFadeOut);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_INTERVAL_CREATE, m_fCreateInterval);
	DDV_MinMaxFloat(pDX, m_fCreateInterval, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_LIFE, m_fLife);
	DDV_MinMaxFloat(pDX, m_fLife, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_MOVE_X, m_fMoveX);
	DDV_MinMaxFloat(pDX, m_fMoveX, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_MOVE_Y, m_fMoveY);
	DDV_MinMaxFloat(pDX, m_fMoveY, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_MOVE_Z, m_fMoveZ);
	DDV_MinMaxFloat(pDX, m_fMoveZ, -10000.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_NUM_CREATE, m_iCreateNum);
	DDV_MinMaxInt(pDX, m_iCreateNum, 0, 10000);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_NUM_PARTICLE, m_iNumParticle);
	DDV_MinMaxInt(pDX, m_iNumParticle, 0, 10000);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_NUM_TEX, m_iNumTex);
	DDV_MinMaxInt(pDX, m_iNumTex, 0, 10000);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_POSITION_X, m_fPosX);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_POSITION_Y, m_fPosY);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_POSITION_Z, m_fPosZ);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_SIZE, m_fSize);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_TEX_VELOCITY, m_fTexVelocity);
	DDX_Text(pDX, IDC_PART_PARTICLE_FILENAME, m_strPathName);
	DDX_Text(pDX, IDC_PART_PARTICLE_TEXFILENAME, m_strTexName);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_ACCELERATION, m_fEmitAccel);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_GRAVITY, m_fEmitGravity);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_ROTVELOCITY, m_fEmitRot);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_VELOCITY, m_fEmitVel);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_RANGE_MAX_X, m_fCreateMaxX);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_RANGE_MAX_Y, m_fCreateMaxY);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_RANGE_MAX_Z, m_fCreateMaxZ);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_RANGE_MIN_X, m_fCreateMinX);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_RANGE_MIN_Y, m_fCreateMinY);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_RANGE_MIN_Z, m_fCreateMinZ);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_LIFE_MIN, m_fParticleLifeMin);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_LIFE_MAX, m_fParticleLifeMax);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_ARG1, m_fEmitArg1);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_ARG2, m_fEmitArg2);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_ARG3, m_fEmitArg3);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_DIR_X, m_EmitDirX);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_DIR_Y, m_EmitDirY);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_EMIT_DIR_Z, m_EmitDirZ);
	DDX_Check(pDX, IDC_PART_PARTICLE_CHK_CHANGE_COLOR, m_bChangeColor);
	DDX_Text(pDX, IDC_PART_PARTICLE_MESH_FILENAME, m_strShapeName);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_SHAPE_FPS, m_fShapeFPS);
	DDX_Check(pDX, IDC_PART_PARTICLE_CHK_ANIM_KEY, m_bAnimKey);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_FADEIN, m_fFadeIn);
	DDV_MinMaxFloat(pDX, m_fFadeIn, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_SIZE_MAX, m_fSizeMax);
	DDV_MinMaxFloat(pDX, m_fSizeMax, 0.f, 10000.f);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_SIZE_MIN, m_fSizeMin);
	DDV_MinMaxFloat(pDX, m_fSizeMin, 0.f, 10000.f);
	DDX_Check(pDX, IDC_CHK_ONGROUND, m_bOnGround);
	DDX_Check(pDX, IDC_CHK_DOUBLESIDE, m_bDoubleSide);
	DDX_Check(pDX, IDC_CHK_LIGHT, m_bLight);
	DDX_Check(pDX, IDC_CHK_ZBUFFER, m_bZBuffer);
	DDX_Check(pDX, IDC_CHK_ZWRITE, m_bZWrite);
	DDX_Text(pDX, IDC_PART_PARTICLE_TEX_ROTVELOCITY, m_fTexRotVelocity);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_PARTICLE_SCALE_VEL_X, m_fScaleVelX);
	DDX_Text(pDX, IDC_PART_PARTICLE_EDT_PARTICLE_SCALE_VEL_Y, m_fScaleVelY);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgEditPartParticle, CDialog)
	//{{AFX_MSG_MAP(CDlgEditPartParticle)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_PART_PARTICLE_BTN_LOAD_TEX, OnPartParticleBtnLoadTex)
	ON_BN_CLICKED(IDC_PART_PARTICLE_BTN_SAVE, OnPartParticleBtnSave)
	ON_BN_CLICKED(IDC_PART_PARTICLE_BTN_SAVE_AS, OnPartParticleBtnSaveAs)
	ON_CBN_SELCHANGE(IDC_PART_PARTICLE_CB_EMIT_TYPE, OnSelchangePartParticleCbEmitType)
	ON_BN_CLICKED(IDC_PART_PARTICLE_BTN_PUT_COLOR, OnPartParticleBtnPutColor)
	ON_BN_CLICKED(IDC_PART_PARTICLE_BTN_LOAD_MESH, OnPartParticleBtnLoadMesh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditPartParticle message handlers

BOOL CDlgEditPartParticle::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_CBEmitType.ResetContent();
	m_CBEmitType.InsertString(0, "spread");
	m_CBEmitType.SetItemData(0, FX_PART_PARTICLE_EMIT_TYPE_SPREAD);
	m_CBEmitType.InsertString(1, "gather");
	m_CBEmitType.SetItemData(1, FX_PART_PARTICLE_EMIT_TYPE_GATHER);
	m_CBEmitType.SetCurSel(0);

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

	for(int i=0;i<NUM_KEY_COLOR;i++)
	{
		m_Color[i] = 0xffffffff;
		m_bColorKey[i] = false;
		m_bAlphaKey[i] = false;
	}
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgEditPartParticle::OnPartParticleBtnSave() 
{
	UpdateData(TRUE);

	if(m_strPathName == _T(""))
	{
		OnPartParticleBtnSaveAs();
		return;
	}

	FILE* file = fopen((LPCTSTR)m_strPathName, "w");
	if(!file)
	{
		MessageBox("n3fxpart파일 생성 실패..-.-;;","ERR02",MB_OK);
		return;
	}

	fprintf(file,"<N3FXPART>\n");
	fprintf(file,"<type> particle\n");
	if(m_strTexName != _T(""))
		fprintf(file,"<texture> %s %d\n", (LPCTSTR)m_strTexName, m_iNumTex);

	fprintf(file,"<life> %5.4f\n", m_fLife);

	fprintf(file,"<texture_animation_speed> %5.4f\n", m_fTexVelocity);
	fprintf(file,"<texture_rotation_speed> %5.4f\n", m_fTexRotVelocity);
	fprintf(file,"<position0> %5.4f %5.4f %5.4f\n", m_fPosX, m_fPosY, m_fPosZ);
	fprintf(file,"<velocity> %5.4f %5.4f %5.4f\n", m_fMoveX, m_fMoveY, m_fMoveZ);
	fprintf(file,"<acceleration> %5.4f %5.4f %5.4f\n", m_fAccelX, m_fAccelY, m_fAccelZ);
	
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

	if(m_bOnGround) fprintf(file,"<on_ground> true\n");
	else fprintf(file,"<on_ground> false\n");

	CString strBlend;
	m_CBSrcBlend.GetLBText(m_CBSrcBlend.GetCurSel(), strBlend);
	fprintf(file,"<src_blend> %s\n", (LPCTSTR)strBlend);

	m_CBDestBlend.GetLBText(m_CBDestBlend.GetCurSel(), strBlend);
	fprintf(file,"<dest_blend> %s\n", (LPCTSTR)strBlend);

	fprintf(file,"<fadeout> %5.4f\n", m_fFadeOut);
	fprintf(file,"<fadein> %5.4f\n", m_fFadeIn);
	fprintf(file,"<particle_count> %d\n", m_iNumParticle);
	fprintf(file,"<particle_size_range> %5.4f %5.4f\n", m_fSizeMin, m_fSizeMax);
	fprintf(file,"<particle_scale_velocity> %5.4f %5.4f\n", m_fScaleVelX, m_fScaleVelY);
	fprintf(file,"<particle_life> %5.4f %5.4f\n", m_fParticleLifeMin, m_fParticleLifeMax);

	fprintf(file,"<start_range_min> %5.4f %5.4f %5.4f\n", m_fCreateMinX, m_fCreateMinY, m_fCreateMinZ);
	fprintf(file,"<start_range_max> %5.4f %5.4f %5.4f\n", m_fCreateMaxX, m_fCreateMaxY, m_fCreateMaxZ);

	fprintf(file,"<create_count> %d\n", m_iCreateNum);
	fprintf(file,"<create_delay> %5.4f\n", m_fCreateInterval);

	DWORD EmitType = static_cast<DWORD>(m_CBEmitType.GetItemData(m_CBEmitType.GetCurSel()));
	if (EmitType == FX_PART_PARTICLE_EMIT_TYPE_SPREAD)
		fprintf(file, "<emit_type> spread %5.4f\n", m_fEmitArg1);
	else if (EmitType == FX_PART_PARTICLE_EMIT_TYPE_GATHER)
		fprintf(file, "<emit_type> gather %5.4f %5.4f %5.4f\n", m_fEmitArg1, m_fEmitArg2, m_fEmitArg3);
	
	fprintf(file,"<particle_direction> %5.4f %5.4f %5.4f\n", m_EmitDirX, m_EmitDirY, m_EmitDirZ);
	fprintf(file,"<particle_velocity> %5.4f\n", m_fEmitVel);
	fprintf(file,"<particle_acceleration> %5.4f\n", m_fEmitAccel);
	fprintf(file,"<particle_rotation_velocity> %5.4f\n", m_fEmitRot);
	fprintf(file,"<particle_gravity> %5.4f\n", m_fEmitGravity);
	
	if(m_bChangeColor) fprintf(file,"<change_color> true\n");
	else fprintf(file,"<change_color> false\n");

	for(int i=0;i<NUM_KEY_COLOR;i++)
	{
		fprintf(file,"<particle_color> %d %d\n", i, m_Color[i]);
		if(m_bColorKey[i]) fprintf(file,"<color_key> %d\n", i);
		if(m_bAlphaKey[i]) fprintf(file,"<alpha_key> %d\n", i);
	}

	fprintf(file,"<shape_name> %s\n", (LPCTSTR)m_strShapeName);
	fprintf(file,"<shape_fps> %5.4f\n", m_fShapeFPS);

	if(m_bAnimKey) fprintf(file,"<shape_apply> true\n");
	else fprintf(file,"<shape_apply> false\n");	

	fprintf(file,"<end>\n");
	fclose(file);
}

bool CDlgEditPartParticle::LoadPartScript(const char* szPath)
{
	m_strPathName = szPath;
	CN3FXPartParticles* pPart = new CN3FXPartParticles;
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
	
	m_fAccelX = pPart->m_vAcceleration.x;
	m_fAccelY = pPart->m_vAcceleration.y;
	m_fAccelZ = pPart->m_vAcceleration.z;

	m_fFadeOut = pPart->m_fFadeOut;
	m_fFadeIn = pPart->m_fFadeIn;
	m_fLife = pPart->m_fLife;
		
	m_fMoveX = pPart->m_vVelocity.x;
	m_fMoveY = pPart->m_vVelocity.y;
	m_fMoveZ = pPart->m_vVelocity.z;
		
	m_iNumTex = pPart->m_iNumTex;
		
	m_fPosX = pPart->m_vPos.x;
	m_fPosY = pPart->m_vPos.y;
	m_fPosZ = pPart->m_vPos.z;

	m_fTexVelocity = pPart->m_fTexFPS;
	m_fTexRotVelocity = pPart->m_fTexRotateVelocity;

	if(m_iNumTex>0 && pPart->m_ppRefTex[0])
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

	m_fCreateInterval = pPart->m_fCreateDelay;
	m_iCreateNum = pPart->m_iNumCreate;
	m_iNumParticle = pPart->m_iNumParticle;
	m_fSizeMin = pPart->m_pair_fParticleSize.first;
	m_fSizeMax = pPart->m_pair_fParticleSize.second;

	m_fScaleVelX = pPart->m_fScaleVelX;
	m_fScaleVelY = pPart->m_fScaleVelY;

	m_fParticleLifeMin = pPart->m_pair_fParticleLife.first;
	m_fParticleLifeMax = pPart->m_pair_fParticleLife.second;

	m_fCreateMaxX = pPart->m_MaxCreateRange.x;
	m_fCreateMaxY = pPart->m_MaxCreateRange.y;
	m_fCreateMaxZ = pPart->m_MaxCreateRange.z;

	m_fCreateMinX = pPart->m_MinCreateRange.x;
	m_fCreateMinY = pPart->m_MinCreateRange.y;
	m_fCreateMinZ = pPart->m_MinCreateRange.z;

	if(pPart->m_dwEmitType==FX_PART_PARTICLE_EMIT_TYPE_SPREAD)
	{
		m_fEmitArg1 = pPart->m_uEmitCon.fEmitAngle;
	}
	else if(pPart->m_dwEmitType==FX_PART_PARTICLE_EMIT_TYPE_GATHER)
	{
		m_fEmitArg1 = pPart->m_uEmitCon.vGatherPoint.x;
		m_fEmitArg2 = pPart->m_uEmitCon.vGatherPoint.y;
		m_fEmitArg3 = pPart->m_uEmitCon.vGatherPoint.z;
	}

	int EmitTypeCount = m_CBEmitType.GetCount();
	for (int i = 0; i < EmitTypeCount; i++)
	{
		DWORD Type = static_cast<DWORD>(m_CBEmitType.GetItemData(i));
		if (pPart->m_dwEmitType == Type)
		{
			m_CBEmitType.SetCurSel(i);
			break;
		}
	}
	SetEmitTypeState(pPart->m_dwEmitType);

	m_fEmitAccel = pPart->m_fPtAccel;
	m_fEmitGravity = pPart->m_fPtGravity;
	m_fEmitRot = D3DXToDegree(pPart->m_fPtRotVelocity);
	m_fEmitVel = pPart->m_fPtVelocity;

	m_EmitDirX = pPart->m_vPtEmitDir.x;
	m_EmitDirY = pPart->m_vPtEmitDir.y;
	m_EmitDirZ = pPart->m_vPtEmitDir.z;

	m_bChangeColor = (BOOL)pPart->m_bChangeColor;

	uint32_t color;
	for(int i=0;i<NUM_KEY_COLOR;i++)
	{
		if(pPart->GetColor(i, color)) m_Color[i] = color;
		if(pPart->m_bChangeColorKey[i]) m_bColorKey[i] = true;
		if(pPart->m_bChangeAlphaKey[i]) m_bAlphaKey[i] = true;
	}

	if(pPart->m_pShape) m_strShapeName = pPart->m_pShape->FileName().c_str();
	m_fShapeFPS = pPart->m_fMeshFPS;
	m_bAnimKey = pPart->m_bAnimKey;

	UpdateData(FALSE);
	//
	//////////////////////////////////////////////////

	delete pPart;
	return true;
}

void CDlgEditPartParticle::OnPartParticleBtnSaveAs() 
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
		OnPartParticleBtnSave();
	}	
}

void CDlgEditPartParticle::OnPartParticleBtnLoadTex() 
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
		}
		else
		{
			m_strTexName = _T("");
			MessageBox("파일 이름끝이 0000이 아니던데요..-.-;;","ERR05",MB_OK);
		}		

		//파일 갯수 세는 기능 넣을까 말까..^^

		UpdateData(FALSE);
	}
	else
		MessageBox("Texture파일은 fx폴더 아래, 혹은 fx폴더 아래에 있는 폴더에 위치해야 합니다..-.-;;","ERR04",MB_OK);

	delete pBaseFileAccess;
}

void CDlgEditPartParticle::OnClose() 
{
	CDialog::OnClose();
	m_pRefFrm->DestroyPartParticle(this);
}

void CDlgEditPartParticle::OnOK() 
{
	return;	
	//CDialog::OnOK();
}

void CDlgEditPartParticle::SetEmitTypeState(int EmitType)
{
	CWnd* pWnd;
	
	if(EmitType==FX_PART_PARTICLE_EMIT_TYPE_SPREAD)
	{
		pWnd = GetDlgItem(IDC_STATIC_GATHER);
		pWnd->ShowWindow(FALSE);
		pWnd = GetDlgItem(IDC_STATIC_SPREAD);
		pWnd->ShowWindow(TRUE);
		pWnd = GetDlgItem(IDC_PART_PARTICLE_EDT_EMIT_ARG2);
		pWnd->ShowWindow(FALSE);
		pWnd = GetDlgItem(IDC_PART_PARTICLE_EDT_EMIT_ARG3);
		pWnd->ShowWindow(FALSE);
	}
	else if(EmitType==FX_PART_PARTICLE_EMIT_TYPE_GATHER)
	{
		pWnd = GetDlgItem(IDC_STATIC_GATHER);
		pWnd->ShowWindow(TRUE);
		pWnd = GetDlgItem(IDC_STATIC_SPREAD);
		pWnd->ShowWindow(FALSE);
		pWnd = GetDlgItem(IDC_PART_PARTICLE_EDT_EMIT_ARG2);
		pWnd->ShowWindow(TRUE);
		pWnd = GetDlgItem(IDC_PART_PARTICLE_EDT_EMIT_ARG3);
		pWnd->ShowWindow(TRUE);
	}
}

void CDlgEditPartParticle::OnSelchangePartParticleCbEmitType() 
{
	int idx = m_CBEmitType.GetCurSel();
	SetEmitTypeState(static_cast<DWORD>(m_CBEmitType.GetItemData(idx)));
}

void CDlgEditPartParticle::OnPartParticleBtnPutColor() 
{
	CDlgPutColor Dlg;
	
	for(int i=0;i<NUM_KEY_COLOR;i++)
	{
		DWORD color = m_Color[i];
		Dlg.m_Color[i] = 0x00000000;
		Dlg.m_Color[i] += ((color<<8)>>24);
		Dlg.m_Color[i] += (((color<<16)>>24)<<8);
		Dlg.m_Color[i] += ((color<<24)>>8);
		
		Dlg.m_Opacity[i] = ((color>>24)<<24);

		Dlg.m_bAlphaKey[i] = m_bAlphaKey[i];
		Dlg.m_bColorKey[i] = m_bColorKey[i];
	}

	if(Dlg.DoModal()==IDCANCEL)
	{
		for(int i=0;i<NUM_KEY_COLOR;i++)
		{
			DWORD color = Dlg.m_Color[i];
			
			m_Color[i] = 0x00000000;
			m_Color[i] += ((color<<8)>>24);
			m_Color[i] += (((color<<16)>>24)<<8);
			m_Color[i] += ((color<<24)>>8);
			m_Color[i] += Dlg.m_Opacity[i];
			
			m_bAlphaKey[i] = Dlg.m_bAlphaKey[i];
			m_bColorKey[i] = Dlg.m_bColorKey[i];
		}
	}
}

void CDlgEditPartParticle::OnPartParticleBtnLoadMesh() 
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
