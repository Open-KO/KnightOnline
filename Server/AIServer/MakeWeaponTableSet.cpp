// MakeWeaponTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MakeWeaponTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMakeWeaponTableSet

IMPLEMENT_DYNAMIC(CMakeWeaponTableSet, CRecordset)

CMakeWeaponTableSet::CMakeWeaponTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMakeWeaponTableSet)
	m_byLevel = 0;
	m_sClass_1 = 0;
	m_sClass_2 = 0;
	m_sClass_3 = 0;
	m_sClass_4 = 0;
	m_sClass_5 = 0;
	m_sClass_6 = 0;
	m_sClass_7 = 0;
	m_sClass_8 = 0;
	m_sClass_9 = 0;
	m_sClass_10 = 0;
	m_sClass_11 = 0;
	m_sClass_12 = 0;
	m_nFields = 13;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMakeWeaponTableSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMakeWeaponTableSet::GetDefaultSQL()
{
	return _T("[dbo].[MAKE_WEAPON]");
}

void CMakeWeaponTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMakeWeaponTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Byte(pFX, _T("[byLevel]"), m_byLevel);
	RFX_Int(pFX, _T("[sClass_1]"), m_sClass_1);
	RFX_Int(pFX, _T("[sClass_2]"), m_sClass_2);
	RFX_Int(pFX, _T("[sClass_3]"), m_sClass_3);
	RFX_Int(pFX, _T("[sClass_4]"), m_sClass_4);
	RFX_Int(pFX, _T("[sClass_5]"), m_sClass_5);
	RFX_Int(pFX, _T("[sClass_6]"), m_sClass_6);
	RFX_Int(pFX, _T("[sClass_7]"), m_sClass_7);
	RFX_Int(pFX, _T("[sClass_8]"), m_sClass_8);
	RFX_Int(pFX, _T("[sClass_9]"), m_sClass_9);
	RFX_Int(pFX, _T("[sClass_10]"), m_sClass_10);
	RFX_Int(pFX, _T("[sClass_11]"), m_sClass_11);
	RFX_Int(pFX, _T("[sClass_12]"), m_sClass_12);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMakeWeaponTableSet diagnostics

#ifdef _DEBUG
void CMakeWeaponTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMakeWeaponTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
