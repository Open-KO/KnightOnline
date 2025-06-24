// MagicTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MagicTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagicTableSet

IMPLEMENT_DYNAMIC(CMagicTableSet, CRecordset)

CMagicTableSet::CMagicTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMagicTableSet)
	m_MagicNum = 0;
	m_EnName = _T("");
	m_KrName = _T("");
	m_Description = _T("");
	m_BeforeAction = 0;
	m_TargetAction = 0;
	m_SelfEffect = 0;
	m_FlyingEffect = 0;
	m_TargetEffect = 0;
	m_Moral = 0;
	m_SkillLevel = 0;
	m_Skill = 0;
	m_Msp = 0;
	m_HP = 0;
	m_ItemGroup = 0;
	m_UseItem = 0;
	m_CastTime = 0;
	m_ReCastTime = 0;
	m_SuccessRate = 0;
	m_Type1 = 0;
	m_Type2 = 0;
	m_Range = 0;
	m_Etc = 0;
	m_nFields = 23;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMagicTableSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMagicTableSet::GetDefaultSQL()
{
	return _T("[dbo].[MAGIC]");
}

void CMagicTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMagicTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[MagicNum]"), m_MagicNum);
	RFX_Text(pFX, _T("[EnName]"), m_EnName);
	RFX_Text(pFX, _T("[KrName]"), m_KrName);
	RFX_Text(pFX, _T("[Description]"), m_Description);
	RFX_Byte(pFX, _T("[BeforeAction]"), m_BeforeAction);
	RFX_Byte(pFX, _T("[TargetAction]"), m_TargetAction);
	RFX_Byte(pFX, _T("[SelfEffect]"), m_SelfEffect);
	RFX_Byte(pFX, _T("[FlyingEffect]"), m_FlyingEffect);
	RFX_Int(pFX, _T("[TargetEffect]"), m_TargetEffect);
	RFX_Byte(pFX, _T("[Moral]"), m_Moral);
	RFX_Int(pFX, _T("[SkillLevel]"), m_SkillLevel);
	RFX_Int(pFX, _T("[Skill]"), m_Skill);
	RFX_Int(pFX, _T("[Msp]"), m_Msp);
	RFX_Int(pFX, _T("[HP]"), m_HP);
	RFX_Byte(pFX, _T("[ItemGroup]"), m_ItemGroup);
	RFX_Long(pFX, _T("[UseItem]"), m_UseItem);
	RFX_Byte(pFX, _T("[CastTime]"), m_CastTime);
	RFX_Byte(pFX, _T("[ReCastTime]"), m_ReCastTime);
	RFX_Byte(pFX, _T("[SuccessRate]"), m_SuccessRate);
	RFX_Byte(pFX, _T("[Type1]"), m_Type1);
	RFX_Byte(pFX, _T("[Type2]"), m_Type2);
	RFX_Int(pFX, _T("[Range]"), m_Range);
	RFX_Byte(pFX, _T("[Etc]"), m_Etc);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMagicTableSet diagnostics

#ifdef _DEBUG
void CMagicTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMagicTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
