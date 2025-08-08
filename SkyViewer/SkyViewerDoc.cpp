// SkyViewerDoc.cpp : implementation of the CSkyViewerDoc class
//

#include "stdafx.h"
#include "SkyViewer.h"

#include "SkyViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerDoc

IMPLEMENT_DYNCREATE(CSkyViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CSkyViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CSkyViewerDoc)
	ON_COMMAND(ID_FILE_SAVE_AS_GAME_FORMAT, OnFileSaveAsGameFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerDoc construction/destruction

CSkyViewerDoc::CSkyViewerDoc()
{
}

CSkyViewerDoc::~CSkyViewerDoc()
{
}

BOOL CSkyViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_Sky.InitToDefaultHardCoding();

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CSkyViewerDoc serialization

void CSkyViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerDoc diagnostics

#ifdef _DEBUG
void CSkyViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSkyViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSkyViewerDoc commands

BOOL CSkyViewerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	return m_Sky.LoadFromTextFile(lpszPathName);
}

BOOL CSkyViewerDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class
	CDocument::OnSaveDocument(lpszPathName);

	return m_Sky.SaveToTextFile(lpszPathName);
}

void CSkyViewerDoc::OnFileSaveAsGameFormat() 
{
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialog dlg(FALSE, "N3Sky", NULL, dwFlags, "Sky Binary File(*.N3Sky)|*.N3Sky||", NULL);
	if(dlg.DoModal() == IDCANCEL) return;
	
	CT2A szFN(dlg.GetPathName());
	m_Sky.SaveToFile(szFN.m_psz);
}
