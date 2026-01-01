// N3MEDoc.h : interface of the CN3MEDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3MEDOC_H__F966DC17_6B9F_4DF5_AA11_3A85CC37EB0D__INCLUDED_)
#define AFX_N3MEDOC_H__F966DC17_6B9F_4DF5_AA11_3A85CC37EB0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CN3MEDoc : public CDocument
{
protected: // create from serialization only
	CN3MEDoc();
	DECLARE_DYNCREATE(CN3MEDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3MEDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CN3MEDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3MEDoc)
	afx_msg void OnFileImportPostDataFromScene();
	afx_msg void OnFileImportShapes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3MEDOC_H__F966DC17_6B9F_4DF5_AA11_3A85CC37EB0D__INCLUDED_)
