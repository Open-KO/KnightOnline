// MagicType1Set.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MagicType1Set.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagicType1Set

IMPLEMENT_DYNAMIC(CMagicType1Set, CRecordset)

CMagicType1Set::CMagicType1Set(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMagicType1Set)
	m_iNum = 0;
	m_Name = _T("");
	m_Description = _T("");
	m_Type = 0;
	m_HitRate = 0;
	m_Hit = 0;
	m_Delay = 0;
	m_ComboType = 0;
	m_ComboCount = 0;
	m_ComboDamage = 0;
	m_Range = 0;
	m_nFields = 11;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMagicType1Set::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMagicType1Set::GetDefaultSQL()
{
	return _T("[dbo].[MAGIC_TYPE1]");
}

void CMagicType1Set::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMagicType1Set)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[iNum]"), m_iNum);
	RFX_Text(pFX, _T("[Name]"), m_Name);
	RFX_Text(pFX, _T("[Description]"), m_Description);
	RFX_Byte(pFX, _T("[Type]"), m_Type);
	RFX_Int(pFX, _T("[HitRate]"), m_HitRate);
	RFX_Int(pFX, _T("[Hit]"), m_Hit);
	RFX_Byte(pFX, _T("[Delay]"), m_Delay);
	RFX_Byte(pFX, _T("[ComboType]"), m_ComboType);
	RFX_Byte(pFX, _T("[ComboCount]"), m_ComboCount);
	RFX_Int(pFX, _T("[ComboDamage]"), m_ComboDamage);
	RFX_Int(pFX, _T("[Range]"), m_Range);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMagicType1Set diagnostics

#ifdef _DEBUG
void CMagicType1Set::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMagicType1Set::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
