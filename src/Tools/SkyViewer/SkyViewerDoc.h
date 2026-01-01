// SkyViewerDoc.h : interface of the CSkyViewerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKYVIEWERDOC_H__A52B394A_9F09_456F_830B_4737EACCAF3E__INCLUDED_)
#define AFX_SKYVIEWERDOC_H__A52B394A_9F09_456F_830B_4737EACCAF3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3SkyMng.h>
#include <N3Base/N3GlobalEffectMng.h>

class CSkyViewerDoc : public CDocument
{
public:
	CN3SkyMng			m_Sky;
	CN3GlobalEffectMng	m_Weather;
protected: // create from serialization only
	CSkyViewerDoc();
	DECLARE_DYNCREATE(CSkyViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkyViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSkyViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSkyViewerDoc)
	afx_msg void OnFileSaveAsGameFormat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKYVIEWERDOC_H__A52B394A_9F09_456F_830B_4737EACCAF3E__INCLUDED_)
