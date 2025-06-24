// MakeLareItemTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MakeLareItemTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMakeLareItemTableSet

IMPLEMENT_DYNAMIC(CMakeLareItemTableSet, CRecordset)

CMakeLareItemTableSet::CMakeLareItemTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMakeLareItemTableSet)
	m_byLevelGrade = 0;
	m_sLareItem = 0;
	m_sMagicItem = 0;
	m_sGereralItem = 0;
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMakeLareItemTableSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMakeLareItemTableSet::GetDefaultSQL()
{
	return _T("[dbo].[MAKE_ITEM_LARECODE]");
}

void CMakeLareItemTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMakeLareItemTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Byte(pFX, _T("[byLevelGrade]"), m_byLevelGrade);
	RFX_Int(pFX, _T("[sLareItem]"), m_sLareItem);
	RFX_Int(pFX, _T("[sMagicItem]"), m_sMagicItem);
	RFX_Int(pFX, _T("[sGereralItem]"), m_sGereralItem);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMakeLareItemTableSet diagnostics

#ifdef _DEBUG
void CMakeLareItemTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMakeLareItemTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
