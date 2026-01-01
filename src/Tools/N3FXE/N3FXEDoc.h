// N3FXEDoc.h : interface of the CN3FXEDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3FXEDOC_H__CA8FE555_2AC9_43F1_8021_5808FB81B803__INCLUDED_)
#define AFX_N3FXEDOC_H__CA8FE555_2AC9_43F1_8021_5808FB81B803__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CN3FXEDoc : public CDocument
{
protected: // create from serialization only
	CN3FXEDoc();
	DECLARE_DYNCREATE(CN3FXEDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CN3FXEDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CN3FXEDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CN3FXEDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_N3FXEDOC_H__CA8FE555_2AC9_43F1_8021_5808FB81B803__INCLUDED_)
