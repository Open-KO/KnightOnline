// N3CEDoc.h : interface of the CN3CEDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3CEDOC_H__9C6D7E3F_BEA2_4683_A46C_A930C420F6A8__INCLUDED_)
#define AFX_N3CEDOC_H__9C6D7E3F_BEA2_4683_A46C_A930C420F6A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Scene.h>

class CN3CEDoc : public CDocument
{
public:
	CN3Scene	m_Scene;
	BOOL		m_bLoadingNow;

protected: // create from serialization only
	CN3CEDoc();
	DECLARE_DYNCREATE(CN3CEDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3CEDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CN3CEDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3CEDoc)
	afx_msg void OnFileSaveAsOneFolder();
	afx_msg void OnToolOptimizeAnimationData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3CEDOC_H__9C6D7E3F_BEA2_4683_A46C_A930C420F6A8__INCLUDED_)
