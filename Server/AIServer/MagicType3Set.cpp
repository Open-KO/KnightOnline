// MagicType3Set.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MagicType3Set.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagicType3Set

IMPLEMENT_DYNAMIC(CMagicType3Set, CRecordset)

CMagicType3Set::CMagicType3Set(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMagicType3Set)
	m_iNum = 0;
	m_Name = _T("");
	m_Description = _T("");
	m_Radius = 0;
	m_Angle = 0;
	m_DirectType = 0;
	m_FirstDamage = 0;
	m_EndDamage = 0;
	m_TimeDamage = 0;
	m_Duration = 0;
	m_Attribute = 0;
	m_nFields = 11;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMagicType3Set::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMagicType3Set::GetDefaultSQL()
{
	return _T("[dbo].[MAGIC_TYPE3]");
}

void CMagicType3Set::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMagicType3Set)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[iNum]"), m_iNum);
	RFX_Text(pFX, _T("[Name]"), m_Name);
	RFX_Text(pFX, _T("[Description]"), m_Description);
	RFX_Byte(pFX, _T("[Radius]"), m_Radius);
	RFX_Int(pFX, _T("[Angle]"), m_Angle);
	RFX_Byte(pFX, _T("[DirectType]"), m_DirectType);
	RFX_Int(pFX, _T("[FirstDamage]"), m_FirstDamage);
	RFX_Int(pFX, _T("[EndDamage]"), m_EndDamage);
	RFX_Int(pFX, _T("[TimeDamage]"), m_TimeDamage);
	RFX_Byte(pFX, _T("[Duration]"), m_Duration);
	RFX_Byte(pFX, _T("[Attribute]"), m_Attribute);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMagicType3Set diagnostics

#ifdef _DEBUG
void CMagicType3Set::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMagicType3Set::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
