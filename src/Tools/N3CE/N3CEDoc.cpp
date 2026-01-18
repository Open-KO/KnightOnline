// N3CEDoc.cpp : implementation of the CN3CEDoc class
//

#include "stdafx.h"
#include "N3CE.h"
#include "N3CEDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3CEDoc

IMPLEMENT_DYNCREATE(CN3CEDoc, CDocument)

BEGIN_MESSAGE_MAP(CN3CEDoc, CDocument)
//{{AFX_MSG_MAP(CN3CEDoc)
ON_COMMAND(ID_FILE_SAVE_AS_ONE_FOLDER, OnFileSaveAsOneFolder)
ON_COMMAND(ID_TOOL_OPTIMIZE_ANIMATION_DATA, OnToolOptimizeAnimationData)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3CEDoc construction/destruction

CN3CEDoc::CN3CEDoc()
{
	if (m_Scene.CameraCount() <= 0)
		m_Scene.DefaultCameraAdd();
	if (m_Scene.LightCount() <= 0)
		m_Scene.DefaultLightAdd();

	m_bLoadingNow = FALSE;

	// Character 초기화..
	CN3Chr* pChr  = new CN3Chr();
	m_Scene.ChrAdd(pChr);
}

CN3CEDoc::~CN3CEDoc()
{
}

BOOL CN3CEDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (m_Scene.ChrGet(0))
	{
		m_Scene.ChrGet(0)->Release();
		m_Scene.ChrGet(0)->AniDefaultSet(); // 기본 에니메이션 세팅..
	}
	this->UpdateAllViews(nullptr);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CN3CEDoc serialization

void CN3CEDoc::Serialize(CArchive& ar)
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
// CN3CEDoc diagnostics

#ifdef _DEBUG
void CN3CEDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CN3CEDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3CEDoc commands

BOOL CN3CEDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO: Add your specialized creation code here
	char szPath[512]; // 경로를 잘 가려야 한다..
	lstrcpy(szPath, lpszPathName);
	int nFind = 0;
	for (int i = lstrlen(szPath) - 1; i >= 0; i--)
	{
		if ('\\' == szPath[i] || '/' == szPath[i])
			nFind++;
		if (nFind >= 2)
		{
			szPath[i] = '\0';
			break;
		}
	}

	m_bLoadingNow = TRUE;

	m_Scene.PathSet(szPath); // 경로를 정해주고..
	m_Scene.ChrGet(0)->Release();
	m_Scene.ChrGet(0)->LoadFromFile(lpszPathName);
	CN3Joint* pJoint = m_Scene.ChrGet(0)->Joint();
	if (pJoint)
	{
		m_Scene.m_fFrmEnd = pJoint->m_KeyPos.Count() * 30.0f / pJoint->m_KeyRot.SamplingRate();
		if (m_Scene.m_fFrmEnd <= 0 && pJoint->Child(0))
		{
			m_Scene.m_fFrmEnd = pJoint->Child(0)->m_KeyRot.Count() * 30.0f
								/ pJoint->Child(0)->m_KeyRot.SamplingRate();
		}
	}

	if (m_Scene.ChrGet(0)->AniCtrl() == nullptr)
	{
		m_Scene.ChrGet(0)->AniDefaultSet(); // 기본 에니메이션 세팅..
	}

	m_bLoadingNow = FALSE;

	this->UpdateAllViews(nullptr);

	return TRUE;
}

BOOL CN3CEDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	CDocument::OnSaveDocument(lpszPathName);

	// TODO: Add your specialized creation code here
	char szPath[512]; // 경로를 잘 가려야 한다..
	lstrcpy(szPath, lpszPathName);
	int nFind = 0;
	for (int i = lstrlen(szPath) - 1; i >= 0; i--)
	{
		if ('\\' == szPath[i] || '/' == szPath[i])
			nFind++;
		if (nFind >= 2)
		{
			szPath[i] = '\0';
			break;
		}
	}

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME];
	_splitpath(lpszPathName, szDrive, szDir, szFName, nullptr);

	m_Scene.PathSet(szPath);       // 경로를 정해주고..
	CN3Chr* pChr        = m_Scene.ChrGet(0);
	pChr->m_szName      = szFName; // 이름을 정하고..

								   //	CN3Joint* pJoint = pChr->Joint();
	//	if(pJoint)
	//	{
	//		pJoint->SaveToFile(); // 관절도 저장해준다..
	//	}

	CN3CPlugBase* pPlug = nullptr;
	int nCPC            = pChr->PlugCount();
	for (int i = 0; i < nCPC; i++)
	{
		pPlug = pChr->Plug(i);
		pPlug->SaveToFile();
	}

	CN3AnimControl* pAni = pChr->AniCtrl();
	if (pAni)
		pAni->SaveToFile();

	m_Scene.ChrGet(0)->SaveToFile(lpszPathName);
	m_Scene.SaveResrc();

	return TRUE; // 그냥 저장하면 안된다.. 0 Byte 가 된다.
}

void CN3CEDoc::OnFileSaveAsOneFolder()
{
	// TODO: Add your specialized creation code here
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(
		FALSE, "N3Chr", nullptr, dwFlags, "Character Data File(*.N3Chr)|*.N3Chr||", nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;
	CString szFullFileName = dlg.GetPathName();

	char szPath[512]; // 경로를 잘 가려야 한다..
	lstrcpy(szPath, szFullFileName);
	int nFind = 0;
	for (int i = lstrlen(szPath) - 1; i >= 0; i--)
	{
		if ('\\' == szPath[i] || '/' == szPath[i])
			nFind++;
		if (nFind >= 2)
		{
			szPath[i] = '\0';
			break;
		}
	}

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	_splitpath(szFullFileName, szDrive, szDir, nullptr, nullptr);

	m_Scene.PathSet(szPath); // 경로를 정해주고..
	CN3Chr* pChr     = m_Scene.ChrGet(0);

	CN3Joint* pJoint = pChr->Joint();
	if (pJoint)
	{
		char szFN[256];
		_makepath(szFN, szDrive, szDir, pChr->m_szName.c_str(), ".N3Joint");
		pJoint->SaveToFile(szFN); // 관절도 저장해준다..
	}

	CN3CPlugBase* pPlug = nullptr;
	int nCPC            = pChr->PlugCount();
	for (int i = 0; i < nCPC; i++)
	{
		pPlug = pChr->Plug(i);
		if (nullptr == pPlug)
			continue;

		char szFN[256];
		if (pPlug->PMesh())
		{
			_makepath(szFN, szDrive, szDir, pPlug->PMesh()->m_szName.c_str(), ".N3PMesh");
			pPlug->PMesh()->SaveToFile(szFN);
		}
		if (pPlug->Tex())
		{
			_makepath(szFN, szDrive, szDir, pPlug->Tex()->m_szName.c_str(), nullptr);
			pPlug->Tex()->SaveToFile(szFN);
		}

		_makepath(szFN, szDrive, szDir, pPlug->m_szName.c_str(), ".N3CPlug");
		pPlug->SaveToFile(szFN);
	}

	CN3CPart* pPart = nullptr;
	nCPC            = pChr->PartCount();
	for (int i = 0; i < nCPC; i++)
	{
		pPart = pChr->Part(i);
		if (nullptr == pPart)
			continue;

		char szFN[256];

		if (pPart->Skins())
		{
			_makepath(szFN, szDrive, szDir, pPart->Skins()->m_szName.c_str(), ".N3CSkins");
			pPart->Skins()->SaveToFile(szFN);
		}

		if (pPart->Tex())
		{
			_makepath(szFN, szDrive, szDir, pPart->Tex()->m_szName.c_str(), ".DXT");
			pPart->Tex()->SaveToFile(szFN);
		}

		_makepath(szFN, szDrive, szDir, pPart->m_szName.c_str(), ".N3CPart");
		pPart->SaveToFile(szFN);
	}

	CN3AnimControl* pAni = pChr->AniCtrl();
	if (pAni && pAni->Count() > 0) // 에니메이션은 같은 이름으로 저장해준다..
	{
		pAni->m_szName = pChr->m_szName;

		char szFN[256];
		_makepath(szFN, szDrive, szDir, pChr->m_szName.c_str(), ".N3Anim");
		CN3Base::s_MngAniCtrl.Add(pAni);
		pAni->SaveToFile(szFN);
	}

	m_Scene.ChrGet(0)->SaveToFile(std::string(szFullFileName));
	this->SetPathName(szFullFileName);
	this->UpdateAllViews(nullptr); // 모두 업데이트..
}

void CN3CEDoc::OnCloseDocument()
{
	if (m_Scene.ChrGet(0))
		m_Scene.ChrGet(0)->Release(); // 이렇게 해야.. 엔진이 없어지기 전에 깔끔하게 없어진다..

	CDocument::OnCloseDocument();
}

void CN3CEDoc::OnToolOptimizeAnimationData()
{
	CN3Chr* pChr = m_Scene.ChrGet(0);
	if (nullptr == pChr)
		return;
	CN3AnimControl* pAniCtrl = pChr->AniCtrl();
	if (nullptr == pAniCtrl)
		return;
	CN3Joint* pJointSrc = pChr->Joint();
	if (nullptr == pJointSrc)
		return;

	float fSamplingRate = 30.0f / pJointSrc->m_KeyRot.SamplingRate();
	if (fSamplingRate != 6.0f && fSamplingRate != 3.0f && fSamplingRate != 2.0f
		&& fSamplingRate != 1.0f)
	{
		MessageBox(AfxGetMainWnd()->m_hWnd, "SamplingRate 는 5, 10, 15, 30 만 지원됩니다.",
			"SamplingRate 오류", MB_OK);
		return;
	}

	MessageBox(AfxGetMainWnd()->m_hWnd,
		"원본 Joint 와 Animation File 을 다른 이름으로 저장해 둡니다. 이 파일은 나중에 다시 "
		"편집할때 필요합니다.",
		"Joint 및 Animation 파일 다른이름으로 저장", MB_OK);

	CString FileName;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg1(
		FALSE, "N3Anim", nullptr, dwFlags, "Animation Data(*.N3Anim)|*.n3Anim||", nullptr);
	if (dlg1.DoModal() == IDCANCEL)
		return;
	FileName = dlg1.GetPathName();

	std::string szFN(FileName);
	std::string szFNPrev = pAniCtrl->FileName();
	pAniCtrl->SaveToFile(szFN); // 저장..
	pAniCtrl->FileNameSet(szFNPrev);

	CFileDialog dlg2(
		FALSE, "N3Joint", nullptr, dwFlags, "Joint File(*.N3Joint)|*.N3Joint||", nullptr);
	if (dlg2.DoModal() == IDCANCEL)
		return;
	FileName = dlg2.GetPathName();

	szFNPrev = pJointSrc->FileName();
	pChr->Tick(0.0f);            // 초기위치로 Tick
	szFN = FileName;
	pJointSrc->SaveToFile(szFN); // 원래걸 다른 이름으로 저장..
	pJointSrc->FileNameSet(szFNPrev);

	int nK = pAniCtrl->Count();
	CN3Joint JointDest;

	pChr->Tick(0.0f);                            // 초기위치로 Tick
	JointDest.CopyExceptAnimationKey(pJointSrc); // 뼈대 구조 및 값들만 복사한다음..
	JointDest.AddKey(pJointSrc, 0, 0);           // 바인딩 포즈 키를 추가하고 복사한다..

	int iAniCount = pAniCtrl->Count();
	std::vector<float> fFrmOffsets(iAniCount, 0);
	std::vector<float> fFrmStarts(iAniCount, 0);
	for (int i = 0; i < iAniCount; i++) // Animation Key 추가.
	{
		__AnimData* pAD = pAniCtrl->DataGet(i);
		if (pAD->fFrmEnd < pAD->fFrmStart)
			pAD->fFrmEnd = pAD->fFrmStart;             // 같은 프레임으로 만든다..

		int iS = (int) (pAD->fFrmStart / fSamplingRate);
		int iE = (int) (pAD->fFrmEnd / fSamplingRate); // 반올림하려고 0.5f 넣었다..

		if (iE < iS)
			continue;

		JointDest.AddKey(pJointSrc, iS, iE);            // 키를 추가하고 복사한다..
		fFrmOffsets[i] = (iE - iS + 1) * fSamplingRate; // 각 데이터의 Offset 계산..
		fFrmStarts[i]  = pAD->fFrmStart - (iS * fSamplingRate);
	}

	float fOffsetCur = fSamplingRate;
	for (int i = 0; i < iAniCount; i++) // Animation Key Frame 다시 계산..
	{
		__AnimData* pAD = pAniCtrl->DataGet(i);

		pAD->Offset(
			fOffsetCur - pAD->fFrmStart + fFrmStarts[i]); // 모든 데이터를 Offset만큼 값을 적용..

		fOffsetCur += fFrmOffsets[i];
	}

	pAniCtrl->SaveToFile();
	JointDest.SaveToFile();

	pChr->Tick(0.0f); // 초기위치로 Tick
	pChr->JointSet("");
	pChr->JointSet(JointDest.FileName());

	UpdateAllViews(nullptr); // 모든 뷰 초기화..
}
