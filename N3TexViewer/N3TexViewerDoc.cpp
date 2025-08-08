// N3TexViewerDoc.cpp : implementation of the CN3TexViewerDoc class
//

#include "stdafx.h"
#include "N3TexViewer.h"
#include "N3TexViewerDoc.h"

#include <N3Base/BitmapFile.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerDoc

IMPLEMENT_DYNCREATE(CN3TexViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CN3TexViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CN3TexViewerDoc)
	ON_COMMAND(ID_FILE_SAVE_AS_BITMAP, OnFileSaveAsBitmap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerDoc construction/destruction

CN3TexViewerDoc::CN3TexViewerDoc()
{
	// TODO: add one-time construction code here
	m_pTex = new CN3Texture();
	m_pTexAlpha = new CN3Texture();

	m_nCurFile = NULL;
}

CN3TexViewerDoc::~CN3TexViewerDoc()
{
	delete m_pTex;
	delete m_pTexAlpha;
}

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerDoc serialization
void CN3TexViewerDoc::Serialize(CArchive& ar)
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
// CN3TexViewerDoc diagnostics

#ifdef _DEBUG
void CN3TexViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CN3TexViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerDoc commands

BOOL CN3TexViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_pTex->Release();
	m_pTexAlpha->Release();

	this->UpdateAllViews(NULL);

	return TRUE;
}

BOOL CN3TexViewerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	this->FindFiles(); // 파일을 찾고..
	
	// TODO: Add your specialized creation code here
	m_pTexAlpha->Release();
	if(NULL == m_pTex->LoadFromFile(lpszPathName)) return FALSE;
	
	////////////////////////////////////////////////////////////////////////////////
	// Alpha Texture 생성...
	m_pTexAlpha->Create(m_pTex->Width(), m_pTex->Height(), D3DFMT_A8R8G8B8, FALSE);
	LPDIRECT3DSURFACE9 lpSurf, lpSurf2;
	if(m_pTexAlpha->Get())
	{
		m_pTexAlpha->Get()->GetSurfaceLevel(0, &lpSurf);
		m_pTex->Get()->GetSurfaceLevel(0, &lpSurf2);
		::D3DXLoadSurfaceFromSurface(lpSurf, NULL, NULL, lpSurf2, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
		lpSurf2->Release(); lpSurf2 = NULL;

		D3DLOCKED_RECT LR;
		lpSurf->LockRect(&LR, NULL, 0);
		int width = m_pTexAlpha->Width(), height = m_pTexAlpha->Height();
		DWORD dwAlpha = 0;
		DWORD* pPixel = NULL;
		for(int y = 0; y < height; y++)
		{
			for(int x = 0; x < width; x++)
			{
				pPixel = (DWORD*)LR.pBits + y*(LR.Pitch/4) + x;
				dwAlpha = (*pPixel) >> 24;
				*pPixel = dwAlpha | (dwAlpha << 8) | (dwAlpha << 16) | 0xff000000;
			}
		}
		lpSurf->UnlockRect();
		lpSurf->Release(); lpSurf = NULL;
	}
	// Alpha Texture 생성...
	////////////////////////////////////////////////////////////////////////////////

	char szDrv[_MAX_DRIVE], szDir[_MAX_DIR], szFN[_MAX_FNAME], szExt[_MAX_EXT];
	::_splitpath(lpszPathName, szDrv, szDir, szFN, szExt);
	CString szFileName = szFN;
	szFileName += szExt;

	this->SetTitle(szFileName);
	this->UpdateAllViews(NULL);

	return TRUE;
}

BOOL CN3TexViewerDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	char szDrv[_MAX_DRIVE], szDir[_MAX_DIR], szFN[_MAX_FNAME], szExt[_MAX_EXT];
	::_splitpath(lpszPathName, szDrv, szDir, szFN, szExt);

	if(lstrcmpi(szExt, ".DXT") == 0) // 확장자가 DXT 면 그냥 저장..
	{
		CDocument::OnSaveDocument(lpszPathName);

		if(false == m_pTex->SaveToFile(lpszPathName)) return FALSE;

		return TRUE;
	}
	else 
	{
		MessageBox(::GetActiveWindow(), "확장자를 DXT 로 바꾸어야 합니다. Save As 로 저장해주세요.", "저장 실패", MB_OK);

		return FALSE;
	}
}

void CN3TexViewerDoc::SetTitle(LPCTSTR lpszTitle) 
{
	// TODO: Add your specialized code here and/or call the base class
	CString szFmt;
	szFmt.Format("%s - %d, %d", lpszTitle, m_pTex->Width(), m_pTex->Height());
	D3DFORMAT fmtTex = m_pTex->PixelFormat();
	if(D3DFMT_DXT1 == fmtTex) szFmt += " DXT1";
	else if(D3DFMT_DXT2 == fmtTex) szFmt += " DXT2";
	else if(D3DFMT_DXT3 == fmtTex) szFmt += " DXT3";
	else if(D3DFMT_DXT4 == fmtTex) szFmt += " DXT4";
	else if(D3DFMT_DXT5 == fmtTex) szFmt += " DXT5";
	else if(D3DFMT_A1R5G5B5 == fmtTex) szFmt += " A1R5G5B5";
	else if(D3DFMT_A4R4G4B4 == fmtTex) szFmt += " A4R4G4B4";
	else if(D3DFMT_R8G8B8 == fmtTex) szFmt += " R8G8B8";
	else if(D3DFMT_A8R8G8B8 == fmtTex) szFmt += " A8R8G8B8";
	else if(D3DFMT_X8R8G8B8 == fmtTex) szFmt += " X8R8G8B8";
	else  szFmt += " Unknown Format";

	if(m_pTex->MipMapCount() > 1) szFmt += " - has MipMap";
	else szFmt += " - has no MipMap";
	
	CDocument::SetTitle(szFmt);
//	CDocument::SetTitle(lpszTitle);
}

void CN3TexViewerDoc::FindFiles()
{
	char szPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szPath);

	CString szPath2 = szPath;
	szPath2.MakeLower();

	if(m_szPath == szPath2) return;

	m_szPath = szPath2;
	m_szFiles.RemoveAll();

	CFileFind find;

	int i = 0;
	m_nCurFile = 0;
	if(FALSE == find.FindFile("*.DXT")) return;

	for(i = 0; find.FindNextFile(); i++)
	{
		CString szPathTmp = find.GetFilePath();
		m_szFiles.Add(szPathTmp);

		if(szPathTmp == this->GetPathName()) m_nCurFile = i;
	}
}

void CN3TexViewerDoc::OpenNextFile()
{
	m_nCurFile++;
	int nFC = m_szFiles.GetSize();
	if(m_nCurFile < 0 || m_nCurFile >= nFC)
	{
		m_nCurFile = nFC - 1;
		return;
	}

	this->OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenPrevFile()
{
	m_nCurFile--;
	int nFC = m_szFiles.GetSize();
	if(m_nCurFile < 0 || m_nCurFile >= nFC)
	{
		m_nCurFile = 0;
		return;
	}

	this->OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenFirstFile()
{
	m_nCurFile = 0;
	int nFC = m_szFiles.GetSize();
	if(m_nCurFile < 0 || m_nCurFile >= nFC)
	{
		m_nCurFile = 0;
		return;
	}

	this->OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenLastFile()
{
	m_nCurFile = m_szFiles.GetSize() - 1;
	int nFC = m_szFiles.GetSize();
	if(m_nCurFile < 0 || m_nCurFile >= nFC)
	{
		m_nCurFile = 0;
		return;
	}

	this->OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OnFileSaveAsBitmap() 
{
	if(NULL == m_pTex || NULL == m_pTex->Get()) return;

	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(FALSE, "bmp", NULL, dwFlags, "Bitmap file(*.bmp)|*.bmp||", NULL);
	if(dlg.DoModal() == IDCANCEL) return;

	CString szPath = dlg.GetPathName();
	CT2A szPathA(szPath);
	m_pTex->SaveToBitmapFile(szPathA.m_psz);
}
