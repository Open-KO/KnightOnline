// KscViewerDoc.cpp : implementation of the CKscViewerDoc class
//

#include "stdafx.h"
#include "KscViewer.h"

#include "KscViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKscViewerDoc

IMPLEMENT_DYNCREATE(CKscViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CKscViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CKscViewerDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKscViewerDoc construction/destruction

CKscViewerDoc::CKscViewerDoc()
{
	m_pJpegFile = nullptr;
	m_pJpegFile = new CN3JpegFile;
}

CKscViewerDoc::~CKscViewerDoc()
{
	if(m_pJpegFile) delete m_pJpegFile; m_pJpegFile = nullptr;
}

BOOL CKscViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if(m_pJpegFile)
		m_pJpegFile->Release();

	m_szKscPath.Empty();

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CKscViewerDoc serialization

void CKscViewerDoc::Serialize(CArchive& ar)
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
// CKscViewerDoc diagnostics

#ifdef _DEBUG
void CKscViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CKscViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CKscViewerDoc commands

BOOL CKscViewerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
//	if (!CDocument::OnOpenDocument(lpszPathName))
//		return FALSE;
	if (m_pJpegFile == nullptr)
		return FALSE;

	std::string pathA = CT2A(lpszPathName).m_psz;

	size_t nLen = _tcslen(lpszPathName);
	const TCHAR* szExt = lpszPathName + nLen - 3;

	if (0 == lstrcmpi(szExt, _T("ksc")))
	{
		if (m_pJpegFile->DecryptJPEG(pathA))
			m_szKscPath = lpszPathName;
	}
	else if (0 == lstrcmpi(szExt, _T("jpg")))
	{
		if (m_pJpegFile->LoadJpegFile(pathA))
			m_szKscPath = lpszPathName;
	}

	UpdateAllViews(nullptr);
	return TRUE;
}

CN3JpegFile* CKscViewerDoc::GetJpegFile()
{
	return m_pJpegFile;
}

BOOL CKscViewerDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if (m_pJpegFile == nullptr)
		return FALSE;

	if (m_szKscPath.IsEmpty())
		return FALSE;

	CString szTemp = m_szKscPath.Right(3);
	if (szTemp.CompareNoCase(_T("ksc")) != 0)
		return FALSE;

	std::string kscPath(CT2A(m_szKscPath).m_psz),
		jpegPath(CT2A(lpszPathName).m_psz);
	m_pJpegFile->SaveFromDecryptToJpeg(kscPath, jpegPath);

	return TRUE;
}

void CKscViewerDoc::OnFileSave() 
{
	AfxGetApp()->DoWaitCursor(1);

	CString fileName = m_szKscPath.Left(m_szKscPath.GetLength()-3);
	fileName += "jpg";

	OnSaveDocument(fileName);
	AfxGetApp()->DoWaitCursor(-1);
}

void CKscViewerDoc::OnFileSaveAs() 
{
	if (m_szKscPath.IsEmpty())
		return;

	CString szTemp = m_szKscPath.Right(3);
	if (szTemp.CompareNoCase(_T("ksc")) != 0)
		return;

	szTemp = m_szKscPath.Right(m_szKscPath.GetLength() - (m_szKscPath.ReverseFind(_T('\\')) + 1));
	szTemp = szTemp.Left(szTemp.GetLength() - 3) + _T("jpg");

	CString fileName;
	CString filt = _T("Jpeg File (*.jpg)|*.jpg||");

	// OPENFILENAME - so I can get to its help page easily
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog fileDlg(FALSE, szTemp, szTemp, dwFlags, filt);

	CString initial_dir;
	GetCurrentDirectory(MAX_PATH, initial_dir.GetBuffer(MAX_PATH));
	fileDlg.m_ofn.lpstrInitialDir = initial_dir.GetString();
	fileDlg.m_ofn.Flags |= OFN_FILEMUSTEXIST;

	if (fileDlg.DoModal() == IDOK)
	{
		AfxGetApp()->DoWaitCursor(1);

		fileName = fileDlg.GetPathName();

		OnSaveDocument(fileName);
		AfxGetApp()->DoWaitCursor(-1);
	}
}
