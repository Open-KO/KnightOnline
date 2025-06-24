// HomeSet.cpp : implementation file
//

#include "stdafx.h"
#include "ebenezer.h"
#include "HomeSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHomeSet

IMPLEMENT_DYNAMIC(CHomeSet, CRecordset)

CHomeSet::CHomeSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CHomeSet)
	m_Nation = 0;
	m_ElmoZoneX = 0;
	m_ElmoZoneZ = 0;
	m_ElmoZoneLX = 0;
	m_ElmoZoneLZ = 0;
	m_KarusZoneX = 0;
	m_KarusZoneZ = 0;
	m_KarusZoneLX = 0;
	m_KarusZoneLZ = 0;
	m_FreeZoneX = 0;
	m_FreeZoneZ = 0;
	m_FreeZoneLX = 0;
	m_FreeZoneLZ = 0;
//
	m_BattleZoneX = 0;
	m_BattleZoneZ = 0;
	m_BattleZoneLX = 0;
	m_BattleZoneLZ = 0;
//

//	m_nFields = 13;
	m_nFields = 17;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CHomeSet::GetDefaultConnect()
{
//	return _T("ODBC;DSN=KN_Online");
	return _T("ODBC;DSN=KN_Online;UID=knight;PWD=knight");
}

CString CHomeSet::GetDefaultSQL()
{
//	return _T("[dbo].[HOME_BACK]");
	return _T("[dbo].[HOME]");
}

void CHomeSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CHomeSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Byte(pFX, _T("[Nation]"), m_Nation);
	RFX_Long(pFX, _T("[ElmoZoneX]"), m_ElmoZoneX);
	RFX_Long(pFX, _T("[ElmoZoneZ]"), m_ElmoZoneZ);
	RFX_Byte(pFX, _T("[ElmoZoneLX]"), m_ElmoZoneLX);
	RFX_Byte(pFX, _T("[ElmoZoneLZ]"), m_ElmoZoneLZ);
	RFX_Long(pFX, _T("[KarusZoneX]"), m_KarusZoneX);
	RFX_Long(pFX, _T("[KarusZoneZ]"), m_KarusZoneZ);
	RFX_Byte(pFX, _T("[KarusZoneLX]"), m_KarusZoneLX);
	RFX_Byte(pFX, _T("[KarusZoneLZ]"), m_KarusZoneLZ);
	RFX_Long(pFX, _T("[FreeZoneX]"), m_FreeZoneX);
	RFX_Long(pFX, _T("[FreeZoneZ]"), m_FreeZoneZ);
	RFX_Byte(pFX, _T("[FreeZoneLX]"), m_FreeZoneLX);
	RFX_Byte(pFX, _T("[FreeZoneLZ]"), m_FreeZoneLZ);
//
	RFX_Long(pFX, _T("[BattleZoneX]"), m_BattleZoneX);
	RFX_Long(pFX, _T("[BattleZoneZ]"), m_BattleZoneZ);
	RFX_Byte(pFX, _T("[BattleZoneLX]"), m_BattleZoneLX);
	RFX_Byte(pFX, _T("[BattleZoneLZ]"), m_BattleZoneLZ);
//
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CHomeSet diagnostics

#ifdef _DEBUG
void CHomeSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CHomeSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
