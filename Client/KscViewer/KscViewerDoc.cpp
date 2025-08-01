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
	m_pJpegFile = NULL;
	m_pJpegFile = new CN3JpegFile;
}

CKscViewerDoc::~CKscViewerDoc()
{
	if(m_pJpegFile) delete m_pJpegFile; m_pJpegFile = NULL;
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
	if(m_pJpegFile == NULL)
		return FALSE;

	char* szExt = NULL;
	int nLen = strlen(lpszPathName);

	szExt = (char*)lpszPathName + nLen - 3;
	
	if(0 == lstrcmpi(szExt,"ksc"))
	{
		if(m_pJpegFile->DecryptJPEG(lpszPathName))
		{
			m_szKscPath = lpszPathName;
		}
	}
	else if(0 == lstrcmpi(szExt,"jpg"))
	{
		if(m_pJpegFile->LoadJpegFile(lpszPathName))
		{
			m_szKscPath = lpszPathName;
		}
	}

	UpdateAllViews(NULL);
	return TRUE;
}

CN3JpegFile* CKscViewerDoc::GetJpegFile()
{
	return m_pJpegFile;
}

BOOL CKscViewerDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if(m_pJpegFile == NULL) return FALSE;
	if(m_szKscPath.IsEmpty()) return FALSE;

	CString szTemp = m_szKscPath.Right(3);
	if(szTemp.CompareNoCase("ksc") != 0) return FALSE;

	m_pJpegFile->SaveFromDecryptToJpeg(m_szKscPath.GetBuffer(m_szKscPath.GetLength()), lpszPathName);

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
	if(m_szKscPath.IsEmpty()) return;
	CString szTemp = m_szKscPath.Right(3);
	if(szTemp.CompareNoCase("ksc") != 0) return;

	szTemp = m_szKscPath.Right(m_szKscPath.GetLength() - (m_szKscPath.ReverseFind('\\') + 1));
	szTemp = szTemp.Left(szTemp.GetLength() - 3) + "jpg";

	CString fileName;
	CString filt="Jpeg File (*.jpg)|*.jpg||";
    
    // OPENFILENAME - so i can get to its Help page easily
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog fileDlg(FALSE,szTemp,szTemp,dwFlags,filt);

	CString initial_dir;
	GetCurrentDirectory(MAX_PATH, initial_dir.GetBuffer(MAX_PATH));
	fileDlg.m_ofn.lpstrInitialDir = (LPCTSTR)initial_dir;
	fileDlg.m_ofn.Flags|=OFN_FILEMUSTEXIST;

	if (fileDlg.DoModal()==IDOK) 
	{
		AfxGetApp()->DoWaitCursor(1);

		fileName = fileDlg.GetPathName();

		OnSaveDocument(fileName);
		AfxGetApp()->DoWaitCursor(-1);
	}            
}
