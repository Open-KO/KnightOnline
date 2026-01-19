// N3TexViewerDoc.cpp : implementation of the CN3TexViewerDoc class
//

#include "StdAfx.h"
#include "N3TexViewer.h"
#include "N3TexViewerDoc.h"

#include <FileIO/FileReader.h>
#include <FileIO/FileWriter.h>
#include <N3Base/BitMapFile.h>

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
ON_COMMAND(ID_FILE_SAVE_AS_BITMAP, &CN3TexViewerDoc::OnFileSaveAsBitmap)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerDoc construction/destruction

CN3TexViewerDoc::CN3TexViewerDoc()
{
	m_pTex            = new CN3Texture();
	m_pTexAlpha       = new CN3Texture();

	m_gttTextureIndex = 0;
	m_nCurFile        = 0;
}

CN3TexViewerDoc::~CN3TexViewerDoc()
{
	// GTT textures refer back to their stored instance.
	if (m_gttTextures.empty())
		delete m_pTex;

	delete m_pTexAlpha;
}

void CN3TexViewerDoc::ReleaseTexture()
{
	// GTT textures refer back to their stored instance.
	if (!m_gttTextures.empty())
	{
		m_gttTextures.clear();
		m_pTex = new CN3Texture();
	}
	// Otherwise they're managed independently.
	else
	{
		m_pTex->Release();
	}

	m_gttTextureIndex = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CN3TexViewerDoc serialization
void CN3TexViewerDoc::Serialize(CArchive&)
{
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

	ReleaseTexture();
	m_pTexAlpha->Release();

	m_szLoadedFileName.Empty();

	UpdateAllViews(nullptr);

	return TRUE;
}

BOOL CN3TexViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	std::filesystem::path texturePath = lpszPathName;
	FindFiles(texturePath); // 파일을 찾고..

	ReleaseTexture();
	m_pTexAlpha->Release();

	m_szLoadedFileName.Empty();

	if (texturePath.has_extension()
		&& _strcmpi(texturePath.extension().string().c_str(), ".gtt") == 0)
	{
		// Open GTT file for reading
		// This contains multiple textures.
		FileReader file;
		if (!file.OpenExisting(texturePath))
			return FALSE;

		// Determine how many textures we have in this GTT file.
		// It should be 8, but we have no reason to assume.
		size_t textureCount = 0;
		try
		{
			while (file.Offset() < file.Size() && m_pTex->SkipFileHandle(file))
				++textureCount;

			if (textureCount == 0)
				return FALSE;

			// Now let's load up each of them in memory.
			m_gttTextures.resize(textureCount);

			file.Seek(0, SEEK_SET);
			for (size_t i = 0; i < m_gttTextures.size(); i++)
				m_gttTextures[i].Load(file);
		}
		catch (const std::runtime_error& ex)
		{
			std::string szErr = texturePath.string() + " - Failed to read file ("
								+ std::string(ex.what()) + ")";
			MessageBoxA(CN3Base::s_hWndBase, szErr.c_str(), "Failed to read file", MB_OK);
			return FALSE;
		}
	}
	else
	{
		if (!m_pTex->LoadFromFile(lpszPathName))
			return FALSE;
	}

	m_szLoadedFileName = lpszPathName;

	LoadSelectedTexture();
	return TRUE;
}

void CN3TexViewerDoc::LoadSelectedTexture()
{
	if (!m_gttTextures.empty())
	{
		if (m_gttTextureIndex >= m_gttTextures.size())
			return;

		m_pTex = &m_gttTextures[m_gttTextureIndex];
	}

	////////////////////////////////////////////////////////////////////////////////
	// Alpha Texture 생성...
	m_pTexAlpha->Create(m_pTex->Width(), m_pTex->Height(), D3DFMT_A8R8G8B8, FALSE);

	if (m_pTexAlpha->Get() != nullptr)
	{
		LPDIRECT3DSURFACE9 lpSurf = nullptr, lpSurf2 = nullptr;

		m_pTexAlpha->Get()->GetSurfaceLevel(0, &lpSurf);
		m_pTex->Get()->GetSurfaceLevel(0, &lpSurf2);
		::D3DXLoadSurfaceFromSurface(
			lpSurf, nullptr, nullptr, lpSurf2, nullptr, nullptr, D3DX_FILTER_TRIANGLE, 0);
		lpSurf2->Release();
		lpSurf2 = nullptr;

		D3DLOCKED_RECT LR;
		lpSurf->LockRect(&LR, nullptr, 0);

		int width = m_pTexAlpha->Width(), height = m_pTexAlpha->Height();
		for (int y = 0; y < height; y++)
		{
			// NOLINTNEXTLINE(performance-no-int-to-ptr)
			uint32_t* row = reinterpret_cast<uint32_t*>(
				reinterpret_cast<uintptr_t>(LR.pBits) + (y * LR.Pitch));

			// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			for (int x = 0; x < width; x++)
			{
				uint32_t dwAlpha = row[x] >> 24;
				row[x]           = dwAlpha | (dwAlpha << 8) | (dwAlpha << 16) | 0xff000000;
			}
			// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		lpSurf->UnlockRect();
		lpSurf->Release();
		lpSurf = nullptr;
	}
	// Alpha Texture 생성...
	////////////////////////////////////////////////////////////////////////////////

	TCHAR szDrv[_MAX_DRIVE] {}, szDir[_MAX_DIR] {}, szFN[_MAX_FNAME] {}, szExt[_MAX_EXT] {};
	_tsplitpath_s(m_szLoadedFileName.GetString(), szDrv, szDir, szFN, szExt);
	CString szFileName  = szFN;
	szFileName         += szExt;

	SetTitle(szFileName);
	UpdateAllViews(nullptr);
}

BOOL CN3TexViewerDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	TCHAR szDrv[_MAX_DRIVE] {}, szDir[_MAX_DIR] {}, szFN[_MAX_FNAME] {}, szExt[_MAX_EXT] {};
	_tsplitpath_s(lpszPathName, szDrv, szDir, szFN, szExt);

	// 확장자가 DXT 면 그냥 저장..
	if (lstrcmpi(szExt, ".DXT") == 0)
	{
		CDocument::OnSaveDocument(lpszPathName);

		if (!m_pTex->SaveToFile(lpszPathName))
			return FALSE;

		return TRUE;
	}
	else if (lstrcmpi(szExt, ".GTT") == 0)
	{
		CDocument::OnSaveDocument(lpszPathName);

		FileWriter file;
		if (!file.Create(lpszPathName))
			return FALSE;

		for (CN3Texture& texture : m_gttTextures)
			texture.Save(file);

		return TRUE;
	}

	MessageBox(::GetActiveWindow(), _T("확장자를 DXT 로 바꾸어야 합니다. Save As 로 저장해주세요."),
		_T("저장 실패"), MB_OK);

	return FALSE;
}

void CN3TexViewerDoc::SetTitle(LPCTSTR lpszTitle)
{
	CString szFmt;
	if (m_gttTextures.empty())
	{
		szFmt.Format(_T("%s - %d, %d"), lpszTitle, m_pTex->Width(), m_pTex->Height());
	}
	else
	{
		szFmt.Format(_T("%s [%zu/%zu] - %d, %d"), lpszTitle, m_gttTextureIndex + 1u,
			m_gttTextures.size(), m_pTex->Width(), m_pTex->Height());
	}

	D3DFORMAT fmtTex = m_pTex->PixelFormat();
	if (D3DFMT_DXT1 == fmtTex)
		szFmt += " DXT1";
	else if (D3DFMT_DXT2 == fmtTex)
		szFmt += " DXT2";
	else if (D3DFMT_DXT3 == fmtTex)
		szFmt += " DXT3";
	else if (D3DFMT_DXT4 == fmtTex)
		szFmt += " DXT4";
	else if (D3DFMT_DXT5 == fmtTex)
		szFmt += " DXT5";
	else if (D3DFMT_A1R5G5B5 == fmtTex)
		szFmt += " A1R5G5B5";
	else if (D3DFMT_A4R4G4B4 == fmtTex)
		szFmt += " A4R4G4B4";
	else if (D3DFMT_R8G8B8 == fmtTex)
		szFmt += " R8G8B8";
	else if (D3DFMT_A8R8G8B8 == fmtTex)
		szFmt += " A8R8G8B8";
	else if (D3DFMT_X8R8G8B8 == fmtTex)
		szFmt += " X8R8G8B8";
	else
		szFmt += " Unknown Format";

	if (m_pTex->MipMapCount() > 1)
		szFmt += " - has MipMap";
	else
		szFmt += " - has no MipMap";

	CDocument::SetTitle(szFmt);
}

void CN3TexViewerDoc::SelectPreviousTexture()
{
	if (m_gttTextureIndex == 0)
		return;

	--m_gttTextureIndex;
	LoadSelectedTexture();
}

void CN3TexViewerDoc::SelectNextTexture()
{
	size_t nextIndex = m_gttTextureIndex + 1;
	if (nextIndex >= m_gttTextures.size())
		return;

	m_gttTextureIndex = nextIndex;
	LoadSelectedTexture();
}

void CN3TexViewerDoc::FindFiles(const std::filesystem::path& loadedFilename)
{
	std::filesystem::path newDirectory = loadedFilename.parent_path();

	if (newDirectory == m_loadedDirectory)
		return;

	m_loadedDirectory = newDirectory;
	m_szFiles.RemoveAll();
	m_nCurFile = 0;

	for (const auto& entry : std::filesystem::directory_iterator(newDirectory))
	{
		if (!entry.is_regular_file())
			continue;

		const auto& file = entry.path();
		std::string ext  = file.extension().string();

		if (_strcmpi(ext.c_str(), ".dxt") == 0 || _strcmpi(ext.c_str(), ".gtt") == 0)
		{
			CString szPathTmp = file.c_str();
			if (file == loadedFilename)
				m_nCurFile = static_cast<int>(m_szFiles.GetCount());

			m_szFiles.Add(szPathTmp);
		}
	}
}

void CN3TexViewerDoc::OpenNextFile()
{
	m_nCurFile++;

	int nFC = static_cast<int>(m_szFiles.GetSize());
	if (m_nCurFile >= nFC)
	{
		m_nCurFile = nFC - 1;
		return;
	}

	OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenPrevFile()
{
	m_nCurFile--;

	int nFC = static_cast<int>(m_szFiles.GetSize());
	if (m_nCurFile < 0 || m_nCurFile >= nFC)
	{
		m_nCurFile = 0;
		return;
	}

	OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenFirstFile()
{
	m_nCurFile = 0;

	if (!m_szFiles.IsEmpty())
		OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OpenLastFile()
{
	int nFC    = static_cast<int>(m_szFiles.GetSize());
	m_nCurFile = nFC - 1;

	if (m_nCurFile < 0 || m_nCurFile >= nFC)
	{
		m_nCurFile = 0;
		return;
	}

	OnOpenDocument(m_szFiles[m_nCurFile]);
}

void CN3TexViewerDoc::OnFileSaveAsBitmap()
{
	if (m_pTex == nullptr || m_pTex->Get() == nullptr)
		return;

	DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	CFileDialog dlg(FALSE, _T("bmp"), nullptr, dwFlags, _T("Bitmap file(*.bmp)|*.bmp||"), nullptr);
	if (dlg.DoModal() == IDCANCEL)
		return;

	std::filesystem::path outputPath = dlg.GetPathName().GetString();
	std::filesystem::path extension  = outputPath.extension();

	if (HasMultipleTextures())
	{
		std::filesystem::path baseFilename = outputPath.filename().replace_extension("");

		for (size_t i = 0; i < m_gttTextures.size(); i++)
		{
			outputPath.replace_filename(baseFilename.string() + std::to_string(i));
			outputPath.replace_extension(extension);
			m_gttTextures[i].SaveToBitmapFile(outputPath.string());
		}
	}
	else
	{
		m_pTex->SaveToBitmapFile(outputPath.string());
	}
}
