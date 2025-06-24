// MonPosSet.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "MonPosSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMonPosSet

IMPLEMENT_DYNAMIC(CMonPosSet, CRecordset)

CMonPosSet::CMonPosSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMonPosSet)
	m_ZoneID = 0;
	m_NpcID = 0;
	m_ActType = 0;
	m_LeftX = 0;
	m_TopZ = 0;
	m_RightX = 0;
	m_BottomZ = 0;
	m_NumNPC = 0;
	m_RegTime = 0;
	m_DotCnt = 0;
	m_path = _T("");
	m_nFields = 11;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CMonPosSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=kn_online;UID=knight;PWD=knight");
}

CString CMonPosSet::GetDefaultSQL()
{
	return _T("[dbo].[NPC_POS]");
}

void CMonPosSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CMonPosSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[ZoneID]"), m_ZoneID);
	RFX_Long(pFX, _T("[NpcID]"), m_NpcID);
	RFX_Byte(pFX, _T("[ActType]"), m_ActType);
	RFX_Long(pFX, _T("[LeftX]"), m_LeftX);
	RFX_Long(pFX, _T("[TopZ]"), m_TopZ);
	RFX_Long(pFX, _T("[RightX]"), m_RightX);
	RFX_Long(pFX, _T("[BottomZ]"), m_BottomZ);
	RFX_Byte(pFX, _T("[NumNPC]"), m_NumNPC);
	RFX_Int(pFX, _T("[RegTime]"), m_RegTime);
	RFX_Byte(pFX, _T("[DotCnt]"), m_DotCnt);
	RFX_Text(pFX, _T("[path]"), m_path);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMonPosSet diagnostics

#ifdef _DEBUG
void CMonPosSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CMonPosSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
