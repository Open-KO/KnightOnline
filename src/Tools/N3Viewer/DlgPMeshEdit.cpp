// DlgPMeshEdit.cpp : implementation file
//

#include "stdafx.h"
#include "n3viewer.h"
#include "MainFrm.h"
#include "DlgPMeshCreateOption.h"
#include "DlgPMeshEdit.h"

#include <N3Base/N3Texture.h>
#include <N3Base/N3PMeshCreate.h>
#include <N3Base/N3PMeshInstance.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPMeshEdit dialog

CDlgPMeshEdit::CDlgPMeshEdit(CWnd* pParent /*=nullptr*/) : CDialog(CDlgPMeshEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPMeshEdit)
	m_iNumVertices = 0;
	m_bPreview     = FALSE;
	//}}AFX_DATA_INIT
	m_pShapeRef    = nullptr;
}

void CDlgPMeshEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPMeshEdit)
	DDX_Control(pDX, IDC_LB_LOD, m_LBLOD);
	DDX_Control(pDX, IDC_CB_PART, m_CBPart);
	DDX_Control(pDX, IDC_SCROLLBAR_VERTEX, m_ScrollBar);
	DDX_Text(pDX, IDC_EDIT_NUMVERTICES, m_iNumVertices);
	DDX_Check(pDX, IDC_CHECK_PREVIEW, m_bPreview);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgPMeshEdit, CDialog)
//{{AFX_MSG_MAP(CDlgPMeshEdit)
ON_BN_CLICKED(IDC_CHECK_PREVIEW, OnCheckPreview)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
ON_BN_CLICKED(IDC_BUTTON_MAKE_PMESH, OnButtonMakePmesh)
ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
ON_WM_DESTROY()
ON_CBN_SELCHANGE(IDC_CB_PART, OnSelchangeCbPart)
ON_BN_CLICKED(IDC_B_MAKE_PMESH_ALL, OnBMakePmeshAll)
ON_WM_CLOSE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPMeshEdit message handlers

void CDlgPMeshEdit::OnCancel()
{
	// TODO: Add extra cleanup here

	//	CDialog::OnCancel();
}

void CDlgPMeshEdit::OnOK()
{
	// TODO: Add extra validation here

	//	CDialog::OnOK();
}

BOOL CDlgPMeshEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->UpdateAllInfo();

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPMeshEdit::OnCheckPreview()
{
	UpdateData(TRUE);
	if (m_bPreview)
	{
		GetDlgItem(IDC_CHECK_PREVIEW)->SetWindowText("Preview Mode");
	}
	else
	{
		GetDlgItem(IDC_CHECK_PREVIEW)->SetWindowText("Setting Mode");
	}
	m_ScrollBar.EnableWindow(!m_bPreview);
	GetDlgItem(IDC_EDIT_NUMVERTICES)->EnableWindow(!m_bPreview);
}

void CDlgPMeshEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	if (pScrollBar && pScrollBar->GetDlgCtrlID() == IDC_SCROLLBAR_VERTEX && nSBCode != SB_ENDSCROLL)
	{
		int iMin, iMax, iCurPos, iToPos;
		pScrollBar->GetScrollRange(&iMin, &iMax);
		iCurPos = pScrollBar->GetScrollPos();
		switch (nSBCode)
		{
			case SB_LEFT:
				iToPos = iCurPos - 100;
				break;
			case SB_RIGHT:
				iToPos = iCurPos + 100;
				break;
			case SB_ENDSCROLL:
				iToPos = iMax;
				break;
			case SB_LINELEFT:
				iToPos = iCurPos - 1;
				break;
			case SB_LINERIGHT:
				iToPos = iCurPos + 1;
				break;
			case SB_PAGELEFT:
				iToPos = iCurPos - 10;
				break;
			case SB_PAGERIGHT:
				iToPos = iCurPos + 10;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				iToPos = nPos;
				break;
		}
		if (iToPos < iMin)
			iToPos = iMin;
		if (iToPos > iMax)
			iToPos = iMax;
		pScrollBar->SetScrollPos(iToPos);
		m_iNumVertices = iToPos;

		UpdateData(FALSE);

		CN3SPart* pPD = m_pShapeRef->Part(m_CBPart.GetCurSel());
		if (pPD)
			SetDlgItemInt(IDC_EDIT_NUMFACES, pPD->MeshInstance()->GetNumIndices() / 3);

		((CMainFrame*) AfxGetMainWnd())->GetView()->InvalidateRect(nullptr, FALSE);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDlgPMeshEdit::OnButtonAdd()
{
	this->LOD_Add();
}

void CDlgPMeshEdit::OnButtonDelete()
{
	this->LOD_Delete();
}

void CDlgPMeshEdit::OnButtonMakePmesh()
{
	int nPart              = m_CBPart.GetCurSel();
	CN3SPart* pPD          = m_pShapeRef->Part(nPart);
	CN3PMesh* pPMesh       = pPD->Mesh();
	CN3PMeshInstance* pPMI = pPD->MeshInstance();
	if (nullptr == pPMI || nullptr == pPMesh)
		return;

	CDlgPMeshCreateOption OptDlg; // Progressive Mesh Create Option
	if (OptDlg.DoModal() == IDCANCEL)
		return;

	CN3PMeshCreate PMC;

	PMC.m_PMCOption.bUseSumOfLoss   = OptDlg.m_iLoss ? true : false;
	PMC.m_PMCOption.bTriangleWeight = OptDlg.m_bTriangleLoss ? true : false;
	PMC.m_PMCOption.bArea           = OptDlg.m_iTriangle ? false : true;
	PMC.m_PMCOption.fWeight         = OptDlg.m_fWeight;

	PMC.ReGenerate(pPMesh);
	pPMesh->SaveToFile(); // 저장

	pPMI->Create(pPMesh);

	int nVC = pPMesh->GetMaxNumVertices();
	int nFC = pPMI->GetNumIndices() / 3; // 현재 Face Count
	pPMI->SetLODByNumVertices(nVC);

	m_ScrollBar.SetScrollRange(0, nVC);
	m_ScrollBar.SetScrollPos(nVC);

	this->UpdateInfo();
}

void CDlgPMeshEdit::OnButtonSave()
{
	int nPC = m_pShapeRef->PartCount();
	for (int i = 0; i < nPC; i++)
	{
		CN3SPart* pPD    = m_pShapeRef->Part(i);
		CN3PMesh* pPMesh = pPD->Mesh();
		if (nullptr == pPMesh)
			continue;

		pPMesh->SaveToFile();
	}
}

void CDlgPMeshEdit::UpdateInfo()
{
	int nPart     = m_CBPart.GetCurSel();
	CN3SPart* pPD = m_pShapeRef->Part(nPart);
	if (nullptr == pPD)
		return;

	CN3PMesh* pPMesh       = pPD->Mesh();
	CN3PMeshInstance* pPMI = pPD->MeshInstance();
	if (nullptr == pPMI || nullptr == pPMesh)
		return;

	CString szTmp;

	int nVC    = pPMesh->GetMaxNumVertices();
	int nVCCur = pPMI->GetNumVertices();
	int nFC    = pPMI->GetNumIndices() / 3; // 현재 Face Count
	m_ScrollBar.SetScrollRange(0, nVC);
	//	m_ScrollBar.SetScrollPos(nVC);
	if (TRUE == pPMI->IsLOD())
	{
		m_ScrollBar.EnableWindow(TRUE);
		szTmp = "LOD 처리 됨";
	}
	else
	{
		m_ScrollBar.EnableWindow(FALSE);
		szTmp = "LOD 처리 안됨";
	}
	SetDlgItemText(IDC_STATIC_LOD, szTmp);

	SetDlgItemInt(IDC_EDIT_NUMVERTICES, nVCCur);
	SetDlgItemInt(IDC_EDIT_NUMFACES, nFC);

	int nSelPrev = m_LBLOD.GetCurSel();
	m_LBLOD.ResetContent();

	int nLODCount = pPMesh->LODCtrlCount();
	for (int i = 0; i < nLODCount; i++)
	{
		CN3PMesh::__LODCtrlValue* pLOD = pPMesh->LODCtrlGet(i);
		szTmp.Format("Distance:%7.2f Vertex:%d", pLOD->fDist, pLOD->iNumVertices);
		m_LBLOD.AddString(szTmp);
	}

	float fFOV  = pPMesh->s_CameraData.fFOV;
	float fDist = (pPMesh->s_CameraData.vEye - pPD->m_Matrix.Pos()).Magnitude();
	float fVal  = fFOV * fDist;

	szTmp.Format("%f", fFOV);
	SetDlgItemText(IDC_E_LOD_FOV, szTmp);
	szTmp.Format("%f", fDist);
	SetDlgItemText(IDC_E_LOD_DISTANCE, szTmp);
	szTmp.Format("%f", fVal);
	SetDlgItemText(IDC_E_LOD_VAL, szTmp);

	if (nSelPrev < 0)
		nSelPrev = 0;
	if (nSelPrev >= nLODCount)
		nSelPrev = nLODCount - 1;
	m_LBLOD.SetCurSel(nSelPrev);
}

void CDlgPMeshEdit::OnClose()
{
	m_pShapeRef = nullptr;

	CDialog::OnClose();
}

void CDlgPMeshEdit::OnDestroy()
{
	CDialog::OnDestroy();
	m_pShapeRef = nullptr;
}

void CDlgPMeshEdit::OnSelchangeCbPart()
{
	this->UpdateInfo();
	CView* pView = ((CMainFrame*) AfxGetMainWnd())->GetActiveView();
	if (pView)
		pView->InvalidateRect(nullptr, FALSE);
}

void CDlgPMeshEdit::OnBMakePmeshAll()
{
	CDlgPMeshCreateOption OptDlg; // Progressive Mesh Create Option
	if (OptDlg.DoModal() == IDCANCEL)
		return;

	int nPMC = m_pShapeRef->PartCount();
	for (int i = 0; i < nPMC; i++)
	{
		CN3SPart* pPD          = m_pShapeRef->Part(i);
		CN3PMeshInstance* pPMI = pPD->MeshInstance();
		CN3PMesh* pPMesh       = pPD->Mesh();
		if (nullptr == pPMI || nullptr == pPMesh)
			continue;

		CN3PMeshCreate PMC;

		PMC.m_PMCOption.bUseSumOfLoss   = OptDlg.m_iLoss ? true : false;
		PMC.m_PMCOption.bTriangleWeight = OptDlg.m_bTriangleLoss ? true : false;
		PMC.m_PMCOption.bArea           = OptDlg.m_iTriangle ? false : true;
		PMC.m_PMCOption.fWeight         = OptDlg.m_fWeight;

		PMC.ReGenerate(pPMesh);

		pPMesh->SaveToFile(); // 저장
		pPMI->Create(pPMesh);
	}

	this->UpdateInfo();
}

void CDlgPMeshEdit::UpdateAllInfo()
{
	if (nullptr == m_pShapeRef)
		return;

	m_ScrollBar.SetScrollRange(0, 0);
	CString szTmp;
	m_CBPart.ResetContent();

	int nPC = m_pShapeRef->PartCount();
	for (int i = 0; i < nPC; i++)
	{
		CN3PMesh* pPMesh = m_pShapeRef->Part(i)->Mesh();

		if (pPMesh)
			szTmp.Format("Part %d : %s", i, pPMesh->m_szName.c_str());
		else
			szTmp.Format("Part %d : NULL PMesh!!", i);
		m_CBPart.AddString(szTmp);
	}
	m_CBPart.SetCurSel(0);

	this->UpdateInfo();
}

void CDlgPMeshEdit::SavePMeshes()
{
	if (nullptr == m_pShapeRef)
		return;

	int nPC = m_pShapeRef->PartCount();
	for (int i = 0; i < nPC; i++)
	{
		CN3PMesh* pPMesh = m_pShapeRef->Part(i)->Mesh();

		if (pPMesh)
			pPMesh->SaveToFile();
	}

	this->UpdateInfo();
}

void CDlgPMeshEdit::LOD_Add()
{
	int nPart        = m_CBPart.GetCurSel();
	CN3SPart* pPD    = m_pShapeRef->Part(nPart);
	CN3PMesh* pPMesh = pPD->Mesh();
	if (nullptr == pPMesh)
		return;

	CN3PMesh::__LODCtrlValue LODs[32]; // LOD Control Value 복사..
	int nLODCount = pPMesh->LODCtrlCount();
	for (int i = 0; i < nLODCount; i++)
		LODs[i] = *(pPMesh->LODCtrlGet(i));

	__Vector3 vDist              = pPD->m_Matrix.Pos() - pPMesh->s_CameraData.vEye; // 추가..
	LODs[nLODCount].fDist        = vDist.Magnitude() * pPMesh->s_CameraData.fFOV;
	LODs[nLODCount].iNumVertices = GetDlgItemInt(IDC_EDIT_NUMVERTICES);

	pPMesh->LODCtrlSet(LODs, nLODCount + 1);
	pPMesh->SaveToFile(); // 저장..

	this->UpdateInfo();
}

void CDlgPMeshEdit::LOD_Delete()
{
	int nSel = m_LBLOD.GetCurSel();
	if (nSel < 0)
		return;
	int nLODCount    = m_LBLOD.GetCount();

	int nPart        = m_CBPart.GetCurSel();
	CN3SPart* pPD    = m_pShapeRef->Part(nPart);
	CN3PMesh* pPMesh = pPD->Mesh();
	if (nullptr == pPMesh)
		return;

	CN3PMesh::__LODCtrlValue LODs[32]; // LOD Control Value 복사..
	int nLOD = 0;
	for (int i = 0; i < nSel; i++)
		LODs[nLOD++] = *(pPMesh->LODCtrlGet(i));
	for (int i = nSel + 1; i < nLODCount; i++)
		LODs[nLOD++] = *(pPMesh->LODCtrlGet(i));
	pPMesh->LODCtrlSet(LODs, nLOD);
	pPMesh->SaveToFile(); // 저장..

	this->UpdateInfo();
}
