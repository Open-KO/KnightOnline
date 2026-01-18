// N3ViewerDoc.cpp : implementation of the CN3ViewerDoc class
//

#include "stdafx.h"
#include "N3Viewer.h"
#include "MainFrm.h"
#include "N3ViewerDoc.h"

#include <N3Base/N3Camera.h>
#include <N3Base/N3Light.h>
#include <N3Base/N3Chr.h>
#include <N3Base/N3Shape.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerDoc

IMPLEMENT_DYNCREATE(CN3ViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CN3ViewerDoc, CDocument)
//{{AFX_MSG_MAP(CN3ViewerDoc)
ON_COMMAND(ID_EDIT_INSERT_CAMERA, OnEditInsertCamera)
ON_COMMAND(ID_EDIT_INSERT_LIGHT, OnEditInsertLight)
ON_COMMAND(ID_EDIT_INSERT_SHAPE, OnEditInsertShape)
ON_COMMAND(ID_EDIT_INSERT_CHARACTER, OnEditInsertCharacter)
ON_COMMAND(ID_EDIT_DELETE_FROM_SCENE, OnEditDeleteFromScene)
ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
ON_COMMAND(ID_FILE_SAVE_TO_SAME_FOLDER, OnFileSaveToSameFolder)
ON_COMMAND(ID_VIEW_AMBIENT_LIGHT, OnViewAmbientLight)
ON_COMMAND(ID_VIEW_DISABLE_DEFAULT_LIGHT, OnViewDisableDefaultLight)
ON_UPDATE_COMMAND_UI(ID_VIEW_DISABLE_DEFAULT_LIGHT, OnUpdateViewDisableDefaultLight)
ON_COMMAND(ID_FILE_SAVE_TO_INDOOR, OnFileSaveToIndoor)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerDoc construction/destruction

CN3ViewerDoc::CN3ViewerDoc()
{
	// TODO: add one-time construction code here
	m_pSelectedObj = nullptr;
}

CN3ViewerDoc::~CN3ViewerDoc()
{
}

BOOL CN3ViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_Scene.Release();
	m_Scene.DefaultCameraAdd();
	m_Scene.DefaultLightAdd();

	m_pSelectedObj   = nullptr;
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->m_DlgPMeshEdit.m_pShapeRef = nullptr;

	this->UpdateAllViews(nullptr);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerDoc serialization

void CN3ViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerDoc diagnostics

#ifdef _DEBUG
void CN3ViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CN3ViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3ViewerDoc commands

BOOL CN3ViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO: Add your specialized creation code here
	m_Scene.Release();
	m_Scene.LoadDataAndResourcesFromFile(lpszPathName);
	if (m_Scene.CameraCount() <= 0)
		m_Scene.DefaultCameraAdd();
	if (m_Scene.LightCount() <= 0)
		m_Scene.DefaultLightAdd();

	m_pSelectedObj   = nullptr;
	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	if (pFrm)
		pFrm->m_DlgPMeshEdit.m_pShapeRef = nullptr;

	this->UpdateAllViews(nullptr);
	return TRUE;
}

BOOL CN3ViewerDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	CDocument::OnSaveDocument(lpszPathName);

	m_Scene.SaveDataAndResourcesToFile(lpszPathName);

	return TRUE;
}

void CN3ViewerDoc::OnFileImport()
{
	CString szExt = "";
	//	CString szFilter = "N3D Object File|*.*|카메라 Data(*.N3Camera)|*.N3Camera|Light Data(*.N3Light)|*.N3Light|Shape Data(*.N3Shape)|*.N3Shape|\
// Progressive Mesh Data(*.N3PMesh)|*.N3Mesh|Indexed Mesh Data(*.N3IMesh)|*.N3IMesh|Joint Data(*.N3Joint)|*.N3Joint|Skinning Data(*.N3Skin)|*.N3Skin|Character Data(*.N3Chr)|*.N3Chr||";
	CString szFilter =
		"N3D Object File|*.*|카메라(*.N3Camera)|*.N3Camera|Light(*.N3Light)|*.N3Light|Progressive "
		"Mesh(*.N3PMesh)|*.N3PMesh|Shape(*.N3Shape)|*.N3Shape|Character(*.N3Chr)|*.N3Chr|Plug(*."
		"N3CPlug)|*.N3CPlug||";

	CString FileName;
	char szBuff[512000] = "";
	DWORD dwFlags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;
	CFileDialog dlg(TRUE, szExt, nullptr, dwFlags, szFilter, nullptr);
	dlg.m_ofn.nMaxFile  = 512000;
	dlg.m_ofn.lpstrFile = szBuff;
	if (dlg.DoModal() == IDCANCEL)
		return;

	POSITION pos = dlg.GetStartPosition();
	for (int i = 0; pos != nullptr; i++)
	{
		FileName      = dlg.GetNextPathName(pos);
		CString szExt = FileName.Right(FileName.GetLength() - FileName.ReverseFind('.') - 1);

		CN3BaseFileAccess* pBase = nullptr;

		if (lstrcmpi(szExt, "N3Camera") == 0)
		{
			CN3Camera* pCamera = new CN3Camera();
			m_Scene.CameraAdd(pCamera);
			pBase = pCamera;
		}
		else if (lstrcmpi(szExt, "N3Light") == 0)
		{
			CN3Light* pLight = new CN3Light();
			m_Scene.LightAdd(pLight);
			pBase = pLight;
		}
		else if (lstrcmpi(szExt, "N3PMesh") == 0)
		{
			CN3PMesh* pMesh = new CN3PMesh();
			m_Scene.s_MngPMesh.Add(pMesh);
			pBase = pMesh;
		}
		//		else if(lstrcmpi(szExt, "N3Joint") == 0)
		//		{
		//			CN3Joint* pJoint = new CN3Joint();
		//			m_Scene.s_MngJoint.Add(pJoint);
		//			pBase = pJoint;
		//		}
		else if (lstrcmpi(szExt, "N3Shape") == 0)
		{
			CN3Shape* pShape = new CN3Shape();
			m_Scene.ShapeAdd(pShape);
			pBase = pShape;
		}
		else if (lstrcmpi(szExt, "N3Chr") == 0)
		{
			CN3Chr* pChr = new CN3Chr();
			m_Scene.ChrAdd(pChr);
			pBase = pChr;
		}
		else if (lstrcmpi(szExt, "N3CPlug") == 0)
		{
			CN3Shape* pShape = new CN3Shape();
			m_Scene.ShapeAdd(pShape);
			CN3SPart* pPart = pShape->PartAdd();

			CN3CPlug plug;
			plug.LoadFromFile(std::string(FileName));

			pPart->m_szName  = plug.m_szName;
			pShape->m_szName = plug.m_szName;
			pShape->FileNameSet(plug.m_szName + ".N3Shape");

			CN3PMesh* pPMesh = plug.PMesh();
			m_Scene.s_MngPMesh.Add(pPMesh);
			CN3Texture* pTex = plug.Tex();
			m_Scene.s_MngTex.Add(pTex);

			pPart->MeshSet(pPMesh->FileName());
			pPart->TexAlloc(1);
			pPart->TexSet(0, pTex->FileName());

			pShape->FindMinMax(); // 큰값, 작은값 찾기..

			continue;
		}
		else
			continue;

		pBase->LoadFromFile(std::string(FileName)); // 파일에서 읽는다..
		m_pSelectedObj = pBase;

		if (lstrcmpi(szExt, "N3PMesh") == 0)
		{
			CN3Shape* pShape = new CN3Shape();
			m_Scene.ShapeAdd(pShape);
			CN3SPart* pPart = pShape->PartAdd();
			pPart->MeshSet(pBase->FileName());
			m_pSelectedObj = pShape;

			delete pBase;
			pBase = nullptr;
		}
	}

	this->UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnFileExport()
{
	if (nullptr == m_pSelectedObj)
		return;
	DWORD dwType = m_pSelectedObj->Type();
	if (!(dwType & OBJ_BASE_FILEACCESS))
		return;

	CN3BaseFileAccess* pBase = (CN3BaseFileAccess*) m_pSelectedObj;

	CString szExt, szFilter;

	if (dwType & OBJ_CAMERA)
	{
		szExt    = "Camera";
		szFilter = "카메라 Data(*.N3Camera)|*.N3Camera||";
	}
	else if (dwType & OBJ_LIGHT)
	{
		szExt    = "N3Light";
		szFilter = "Light Data(*.N3Light)|*.N3Light||";
	}
	else if (dwType & OBJ_MESH)
	{
		szExt    = "N3Mesh";
		szFilter = "Mesh Data(*.N3Mesh)|*.N3Mesh||";
	}
	else if (dwType & OBJ_MESH_PROGRESSIVE)
	{
		szExt    = "N3PMesh";
		szFilter = "Progressive Mesh Data(*.N3PMesh)|*.N3PMesh||";
	}
	else if (dwType & OBJ_MESH_INDEXED)
	{
		szExt    = "N3IMesh";
		szFilter = "Indexed Mesh Data(*.N3IMesh)|*.N3IMesh||";
	}
	else if (dwType & OBJ_JOINT)
	{
		szExt    = "N3Joint";
		szFilter = "Joint Data(*.N3Joint)|*.N3Joint||";
	}
	else if (dwType & OBJ_SKIN)
	{
		szExt    = "N3Skin";
		szFilter = "Skinning Data(*.N3Skin)|*.N3Skin||";
	}
	else if (dwType & OBJ_SHAPE)
	{
		szExt    = "N3Shape";
		szFilter = "Shape Data(*.N3Shape)|*.N3Shape||";
	}
	else if (dwType & OBJ_CHARACTER)
	{
		szExt    = "N3Chr";
		szFilter = "Character Data(*.N3Chr)|*.N3Chr||";
	}

	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(FALSE, szExt, nullptr, dwFlags, szFilter, nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	std::string szOldFN = pBase->FileName();
	CString szFullPath  = dlg.GetPathName();

	CString szOldName   = pBase->m_szName.c_str();
	if (dwType & OBJ_SHAPE)
	{
		CN3Shape* pShape = (CN3Shape*) pBase;
		pShape->SaveToFile(std::string(szFullPath));
	}
	else
	{
		pBase->SaveToFile(std::string(szFullPath));
	}

	pBase->FileNameSet(szOldFN);
	this->UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnFileSaveToSameFolder()
{
	if (nullptr == m_pSelectedObj)
		return;
	DWORD dwType = m_pSelectedObj->Type();
	if (!(dwType & OBJ_SHAPE))
		return;

	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		FALSE, ".N3Shape", nullptr, dwFlags, "Shape Data(*.N3Shape)|*.N3Shape||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString szFullPath = dlg.GetPathName();

	CN3Shape* pShape   = (CN3Shape*) m_pSelectedObj;
	pShape->SaveToSameFolder(std::string(szFullPath));
}

void CN3ViewerDoc::OnFileSaveToIndoor()
{
	// TODO: Add your command handler code here
	int iCount = m_Scene.ShapeCount();
	if (iCount <= 0)
		return;

	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		FALSE, ".N3Scene", nullptr, dwFlags, "Scene Data(*.N3Scene)|*.N3Scene||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;
	CString szFullPath = dlg.GetPathName();

	for (int i = 0; i < iCount; i++)
	{
		CN3Shape* pShape = m_Scene.ShapeGet(i);
		pShape->SaveToSameFolderAndMore(std::string(szFullPath), "N3Indoor\\");
	}
	OnSaveDocument(std::string(szFullPath).c_str());
}

void CN3ViewerDoc::OnEditInsertCamera()
{
	CN3Camera* pCamera = new CN3Camera();
	pCamera->m_szName  = "DefaultCamera";

	char szFN[256];
	wsprintf(szFN, "Chr\\DefaultCamera_%d.N3Camera", m_Scene.CameraCount() + 1);
	pCamera->FileNameSet(szFN);
	m_Scene.CameraAdd(pCamera);

	m_pSelectedObj = pCamera;
	this->UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnEditInsertLight()
{
	CN3Light* pLight = new CN3Light();
	pLight->m_szName = "DefaultLight";

	char szFN[256];
	wsprintf(szFN, "Data\\DefaultLight_%d.N3Light", m_Scene.LightCount() + 1);
	pLight->FileNameSet(szFN);
	m_Scene.LightAdd(pLight);

	m_pSelectedObj = pLight;
	this->UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnEditInsertShape()
{
	CN3Shape* pShape = new CN3Shape();
	pShape->m_szName = "DefaultShape";

	char szFN[256];
	wsprintf(szFN, "Object\\DefaultShape_%d.N3Shape", m_Scene.ShapeCount() + 1);
	pShape->FileNameSet(szFN);
	m_Scene.ShapeAdd(pShape);

	m_pSelectedObj = pShape;
	this->UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnEditInsertCharacter()
{
	CN3Chr* pChr   = new CN3Chr();
	pChr->m_szName = "DefaultChr";

	char szFN[256];
	wsprintf(szFN, "Chr\\DefaultChr_%d.N3Chr", m_Scene.ChrCount() + 1);
	pChr->FileNameSet(szFN);
	m_Scene.ChrAdd(pChr);

	m_pSelectedObj = pChr;
	this->UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnEditDeleteFromScene()
{
	if (nullptr == m_pSelectedObj)
		return;

	if (m_pSelectedObj->Type() & OBJ_CAMERA)
		m_Scene.CameraDelete((CN3Camera*) m_pSelectedObj);
	else if (m_pSelectedObj->Type() & OBJ_LIGHT)
		m_Scene.LightDelete((CN3Light*) m_pSelectedObj);
	else if (m_pSelectedObj->Type() & OBJ_SHAPE)
		m_Scene.ShapeDelete((CN3Shape*) m_pSelectedObj);
	else if (m_pSelectedObj->Type() & OBJ_CHARACTER)
		m_Scene.ChrDelete((CN3Chr*) m_pSelectedObj);

	m_pSelectedObj = nullptr;
	this->UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnCloseDocument()
{
	m_Scene.Release();

	CDocument::OnCloseDocument();
}

void CN3ViewerDoc::OnViewAmbientLight()
{
	D3DCOLOR d3dcolor = m_Scene.m_AmbientLightColor;
	CColorDialog dlg(
		RGB((d3dcolor & 0x00ff0000) >> 16, (d3dcolor & 0x0000ff00) >> 8, d3dcolor & 0x000000ff));
	if (IDCANCEL == dlg.DoModal())
		return;
	COLORREF colorref           = dlg.GetColor();
	m_Scene.m_AmbientLightColor = D3DCOLOR_ARGB(
		0xff, GetRValue(colorref), GetGValue(colorref), GetBValue(colorref));
	UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnViewDisableDefaultLight()
{
	m_Scene.m_bDisableDefaultLight = !m_Scene.m_bDisableDefaultLight;
	UpdateAllViews(nullptr);
}

void CN3ViewerDoc::OnUpdateViewDisableDefaultLight(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_Scene.m_bDisableDefaultLight);
}

