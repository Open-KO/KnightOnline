// N3TexViewerDoc.h : interface of the CN3TexViewerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TEXVIEWERDOC_H__7AF248CB_9159_488E_8B75_9A719F9F36FF__INCLUDED_)
#define AFX_N3TEXVIEWERDOC_H__7AF248CB_9159_488E_8B75_9A719F9F36FF__INCLUDED_

#pragma once

#include <N3Base/N3Texture.h>

#include <vector>

class CN3TexViewerDoc : public CDocument
{
protected: // create from serialization only
	CN3TexViewerDoc();
	DECLARE_DYNCREATE(CN3TexViewerDoc)

	// Attributes
public:
	CN3Texture* m_pTex;
	CN3Texture* m_pTexAlpha;

	std::vector<CN3Texture> m_gttTextures;
	size_t m_gttTextureIndex;

	CString m_szLoadedFileName;

	int m_nCurFile;
	std::filesystem::path m_loadedDirectory;
	CStringArray m_szFiles;

	// Operations
public:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3TexViewerDoc)
public:
	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	BOOL OnSaveDocument(LPCTSTR lpszPathName) override;
	void Serialize(CArchive& ar) override;
	void SetTitle(LPCTSTR lpszTitle) override;
	//}}AFX_VIRTUAL

	// Implementation
public:
	bool HasMultipleTextures() const
	{
		return !m_gttTextures.empty();
	}

	void OpenLastFile();
	void OpenFirstFile();
	void OpenPrevFile();
	void OpenNextFile();
	void FindFiles(const std::filesystem::path& loadedFilename);
	void SelectNextTexture();
	void SelectPreviousTexture();
	void LoadSelectedTexture();
	void ReleaseTexture();
	~CN3TexViewerDoc() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected:
	// Generated message map functions
protected:
	//{{AFX_MSG(CN3TexViewerDoc)
	afx_msg void OnFileSaveAsBitmap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3TEXVIEWERDOC_H__7AF248CB_9159_488E_8B75_9A719F9F36FF__INCLUDED_)
