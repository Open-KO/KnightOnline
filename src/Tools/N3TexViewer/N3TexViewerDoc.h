// N3TexViewerDoc.h : interface of the CN3TexViewerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TEXVIEWERDOC_H__7AF248CB_9159_488E_8B75_9A719F9F36FF__INCLUDED_)
#define AFX_N3TEXVIEWERDOC_H__7AF248CB_9159_488E_8B75_9A719F9F36FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Texture.h>

class CN3TexViewerDoc : public CDocument
{
protected: // create from serialization only
	CN3TexViewerDoc();
	DECLARE_DYNCREATE(CN3TexViewerDoc)

// Attributes
public:
	CN3Texture* m_pTex;
	CN3Texture* m_pTexAlpha;

	int				m_nCurFile;
	CString			m_szPath;
	CStringArray	m_szFiles;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3TexViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);
	virtual void SetTitle(LPCTSTR lpszTitle);
	//}}AFX_VIRTUAL

// Implementation
public:
	void OpenLastFile();
	void OpenFirstFile();
	void OpenPrevFile();
	void OpenNextFile();
	void FindFiles();
	virtual ~CN3TexViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
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
