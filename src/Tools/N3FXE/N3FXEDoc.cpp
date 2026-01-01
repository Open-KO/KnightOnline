// N3FXEDoc.cpp : implementation of the CN3FXEDoc class
//

#include "stdafx.h"
#include "N3FXE.h"

#include "N3FXEDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CN3FXEDoc

IMPLEMENT_DYNCREATE(CN3FXEDoc, CDocument)

BEGIN_MESSAGE_MAP(CN3FXEDoc, CDocument)
	//{{AFX_MSG_MAP(CN3FXEDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CN3FXEDoc construction/destruction

CN3FXEDoc::CN3FXEDoc()
{
	// TODO: add one-time construction code here

}

CN3FXEDoc::~CN3FXEDoc()
{
}

BOOL CN3FXEDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CN3FXEDoc serialization

void CN3FXEDoc::Serialize(CArchive& ar)
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
// CN3FXEDoc diagnostics

#ifdef _DEBUG
void CN3FXEDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CN3FXEDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CN3FXEDoc commands
