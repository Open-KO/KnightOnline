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

	m_nCurFile = 0;
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

BOOL CN3TexViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_pTex->Release();
	m_pTexAlpha->Release();

	this->UpdateAllViews(nullptr);

	return TRUE;
}

BOOL CN3TexViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	if (!m_pTex->LoadFromFile(lpszPathName))
		return FALSE;

	FindFiles(lpszPathName);

	if (m_pTexAlpha) m_pTexAlpha->Release();

	D3DFORMAT fmt = m_pTex->PixelFormat();
	if (fmt == D3DFMT_DXT3 || fmt == D3DFMT_DXT5 || fmt == D3DFMT_A8R8G8B8)
	{
		m_pTexAlpha->Create(m_pTex->Width(), m_pTex->Height(), D3DFMT_A8R8G8B8, FALSE);

		LPDIRECT3DSURFACE9 lpSurf = nullptr, lpSurf2 = nullptr;
		if (m_pTexAlpha && m_pTexAlpha->Get())
		{
			if (SUCCEEDED(m_pTexAlpha->Get()->GetSurfaceLevel(0, &lpSurf)) &&
				SUCCEEDED(m_pTex->Get()->GetSurfaceLevel(0, &lpSurf2)))
			{
				if (SUCCEEDED(::D3DXLoadSurfaceFromSurface(lpSurf, nullptr, nullptr,
					lpSurf2, nullptr, nullptr,
					D3DX_FILTER_TRIANGLE, 0)))
				{
					D3DLOCKED_RECT LR;
					if (SUCCEEDED(lpSurf->LockRect(&LR, nullptr, 0)))
					{
						const int width = m_pTexAlpha->Width();
						const int height = m_pTexAlpha->Height();

						for (int y = 0; y < height; ++y)
						{
							DWORD* pRow = reinterpret_cast<DWORD*>((BYTE*)LR.pBits + y * LR.Pitch);
							for (int x = 0; x < width; ++x)
							{
								DWORD dwAlpha = (pRow[x] >> 24) & 0xFF;
								pRow[x] = (dwAlpha << 24) | (dwAlpha << 16) | (dwAlpha << 8) | dwAlpha;
							}
						}
						lpSurf->UnlockRect();
					}
				}
				if (lpSurf2) { lpSurf2->Release(); lpSurf2 = nullptr; }
				if (lpSurf) { lpSurf->Release();  lpSurf = nullptr; }
			}
		}
	}

	char szDrv[_MAX_DRIVE], szDir[_MAX_DIR], szFN[_MAX_FNAME], szExt[_MAX_EXT];
	::_splitpath(lpszPathName, szDrv, szDir, szFN, szExt);
	CString szFileName = szFN; szFileName += szExt;
	SetTitle(szFileName);

	UpdateAllViews(nullptr);
	return TRUE;
}


BOOL CN3TexViewerDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	char szDrv[_MAX_DRIVE], szDir[_MAX_DIR], szFN[_MAX_FNAME], szExt[_MAX_EXT];
	::_splitpath(lpszPathName, szDrv, szDir, szFN, szExt);

	if(lstrcmpi(szExt, ".DXT") == 0 || lstrcmpi(szExt, ".dxt") == 0 || lstrcmpi(szExt, ".gtt") == 0) // 확장자가 DXT 면 그냥 저장..
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
	CString szFmt;
	szFmt.Format("%s - %d, %d", lpszTitle, m_pTex->Width(), m_pTex->Height());

	switch (m_pTex->PixelFormat()) { 
		case D3DFMT_DXT1: szFmt += " DXT1"; break;
		case D3DFMT_DXT2: szFmt += " DXT2"; break; 
		case D3DFMT_DXT3: szFmt += " DXT3"; break; 
		case D3DFMT_DXT4: szFmt += " DXT4"; break; 
		case D3DFMT_DXT5: szFmt += " DXT5"; break; 

		case D3DFMT_A1R5G5B5: szFmt += " A1R5G5B5"; break; 
		case D3DFMT_A4R4G4B4: szFmt += " A4R4G4B4"; break; 
		case D3DFMT_R8G8B8: szFmt += " R8G8B8"; break; 
		case D3DFMT_A8R8G8B8: szFmt += " A8R8G8B8"; break; 
		case D3DFMT_X8R8G8B8: szFmt += " X8R8G8B8"; break;
		default: szFmt += " Unknown Format"; break;
	}

	szFmt += (m_pTex->MipMapCount() > 1) ? " - has MipMap" : " - has no MipMap";
	CDocument::SetTitle(szFmt);
}

void CN3TexViewerDoc::FindFiles(LPCTSTR lpszPathName)
{
	if (!lpszPathName || _tcslen(lpszPathName) == 0) return;

	CString filePath = lpszPathName; 
	int pos = filePath.ReverseFind('\\');
	if (pos == -1) return;

	CString dirPath = filePath.Left(pos); 
	CString szPath2 = dirPath; 
	szPath2.MakeLower();

	if (m_szPath == szPath2) return;

	m_szPath = szPath2;
	m_szFiles.RemoveAll();
	m_nCurFile = 0;
	 
	const char* patterns[] = { "*.DXT", "*.GTT" };
	for (auto& pat : patterns)
	{
		CFileFind finder;
		CString searchPattern = dirPath + "\\" + pat;
		BOOL bFound = finder.FindFile(searchPattern);
		while (bFound)
		{
			bFound = finder.FindNextFile();

			if (!finder.IsDots() && !finder.IsDirectory())
			{
				CString szPathTmp = finder.GetFilePath();
				m_szFiles.Add(szPathTmp);

				if (szPathTmp.CompareNoCase(filePath) == 0) 
					m_nCurFile = static_cast<int>(m_szFiles.GetSize() - 1);
			}
		}
		finder.Close();
	}
}

void CN3TexViewerDoc::OpenNextFile()
{
	if (m_szFiles.IsEmpty()) return; 
	
	if (m_nCurFile + 1 < m_szFiles.GetSize())
		m_nCurFile++; 
	
	OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenPrevFile()
{
	if (m_szFiles.IsEmpty()) return; 
	
	if (m_nCurFile > 0) 
		m_nCurFile--; 
	
	OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenFirstFile()
{
	if (m_szFiles.IsEmpty()) 
		return;

	m_nCurFile = 0;

	OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenLastFile()
{
	if (m_szFiles.IsEmpty()) 
		return;

	m_nCurFile = static_cast<int>(m_szFiles.GetSize()) - 1;

	OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OnFileSaveAsBitmap()
{
	if (nullptr == m_pTex || nullptr == m_pTex->Get()) return;

	CFileDialog dlg(FALSE, _T("bmp"), nullptr,
		OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
		_T("Bitmap file (*.bmp)|*.bmp||"), nullptr);

	if (dlg.DoModal() != IDOK) return;

	CString szPath = dlg.GetPathName();
	CT2A szPathA(szPath);
	m_pTex->SaveToBitmapFile(szPathA.m_psz);
}
