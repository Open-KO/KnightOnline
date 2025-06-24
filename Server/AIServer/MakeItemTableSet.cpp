// MakeItemTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MakeItemTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMakeItemTableSet

IMPLEMENT_DYNAMIC(CMakeItemTableSet, CRecordset)

CMakeItemTableSet::CMakeItemTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMakeItemTableSet)
	m_sIndex = 0;
	m_strItemInfo = _T("");
	m_sItemCode = 0;
	m_nFields = 3;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMakeItemTableSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMakeItemTableSet::GetDefaultSQL()
{
	return _T("[dbo].[MAKE_ITEM]");
}

void CMakeItemTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMakeItemTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[sIndex]"), m_sIndex);
	RFX_Text(pFX, _T("[strItemInfo]"), m_strItemInfo);
	RFX_Int(pFX, _T("[sItemCode]"), m_sItemCode);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMakeItemTableSet diagnostics

#ifdef _DEBUG
void CMakeItemTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMakeItemTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
