// KscViewerDoc.h : interface of the CKscViewerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_KSCVIEWERDOC_H__F42EC8EB_E49B_4638_B2B2_F25E9C226D55__INCLUDED_)
#define AFX_KSCVIEWERDOC_H__F42EC8EB_E49B_4638_B2B2_F25E9C226D55__INCLUDED_

#include "N3JpegFile.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CKscViewerDoc : public CDocument
{
protected: // create from serialization only
	CKscViewerDoc();
	DECLARE_DYNCREATE(CKscViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKscViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString			m_szKscPath;
	CN3JpegFile*	m_pJpegFile;
	CN3JpegFile*	GetJpegFile();
	virtual ~CKscViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CKscViewerDoc)
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KSCVIEWERDOC_H__F42EC8EB_E49B_4638_B2B2_F25E9C226D55__INCLUDED_)
