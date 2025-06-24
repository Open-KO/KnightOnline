// MagicType4Set.cpp : implementation file
//

#include "stdafx.h"
#include "server.h"
#include "MagicType4Set.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagicType4Set

IMPLEMENT_DYNAMIC(CMagicType4Set, CRecordset)

CMagicType4Set::CMagicType4Set(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMagicType4Set)
	m_iNum = 0;
	m_Name = _T("");
	m_Description = _T("");
	m_BuffType = 0;
	m_Radius = 0;
	m_Duration = 0;
	m_AttackSpeed = 0;
	m_Speed = 0;
	m_AC = 0;
	m_Attack = 0;
	m_MaxHP = 0;
	m_HitRate = 0;
	m_AvoidRate = 0;
	m_Str = 0;
	m_Sta = 0;
	m_Dex = 0;
	m_Intel = 0;
	m_Cha = 0;
	m_FireR = 0;
	m_ColdR = 0;
	m_LightningR = 0;
	m_MagicR = 0;
	m_DiseaseR = 0;
	m_PoisonR = 0;
	m_nFields = 24;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMagicType4Set::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMagicType4Set::GetDefaultSQL()
{
	return _T("[dbo].[MAGIC_TYPE4]");
}

void CMagicType4Set::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMagicType4Set)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[iNum]"), m_iNum);
	RFX_Text(pFX, _T("[Name]"), m_Name);
	RFX_Text(pFX, _T("[Description]"), m_Description);
	RFX_Byte(pFX, _T("[BuffType]"), m_BuffType);
	RFX_Byte(pFX, _T("[Radius]"), m_Radius);
	RFX_Int(pFX, _T("[Duration]"), m_Duration);
	RFX_Byte(pFX, _T("[AttackSpeed]"), m_AttackSpeed);
	RFX_Byte(pFX, _T("[Speed]"), m_Speed);
	RFX_Int(pFX, _T("[AC]"), m_AC);
	RFX_Byte(pFX, _T("[Attack]"), m_Attack);
	RFX_Int(pFX, _T("[MaxHP]"), m_MaxHP);
	RFX_Byte(pFX, _T("[HitRate]"), m_HitRate);
	RFX_Int(pFX, _T("[AvoidRate]"), m_AvoidRate);
	RFX_Byte(pFX, _T("[Str]"), m_Str);
	RFX_Byte(pFX, _T("[Sta]"), m_Sta);
	RFX_Byte(pFX, _T("[Dex]"), m_Dex);
	RFX_Byte(pFX, _T("[Intel]"), m_Intel);
	RFX_Byte(pFX, _T("[Cha]"), m_Cha);
	RFX_Byte(pFX, _T("[FireR]"), m_FireR);
	RFX_Byte(pFX, _T("[ColdR]"), m_ColdR);
	RFX_Byte(pFX, _T("[LightningR]"), m_LightningR);
	RFX_Byte(pFX, _T("[MagicR]"), m_MagicR);
	RFX_Byte(pFX, _T("[DiseaseR]"), m_DiseaseR);
	RFX_Byte(pFX, _T("[PoisonR]"), m_PoisonR);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMagicType4Set diagnostics

#ifdef _DEBUG
void CMagicType4Set::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMagicType4Set::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
