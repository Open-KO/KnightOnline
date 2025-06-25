// NpcPosSet.cpp : implementation file
//

#include "stdafx.h"
#include "ServerDlg.h"
#include "NpcPosSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNpcPosSet

IMPLEMENT_DYNAMIC(CNpcPosSet, CRecordset)

CNpcPosSet::CNpcPosSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CNpcPosSet)
	m_ZoneID = 0;
	m_NpcID = 0;
	m_ActType = 0;
	m_RegenType = 0;
	m_DungeonFamily = 0;
	m_SpecialType = 0;
	m_TrapNumber = 0;
	m_LeftX = 0;
	m_TopZ = 0;
	m_RightX = 0;
	m_BottomZ = 0;
	m_LimitMinX = 0;
	m_LimitMinZ = 0;
	m_LimitMaxX = 0;
	m_LimitMaxZ = 0;
	m_NumNPC = 0;
	m_RegTime = 0;
	m_Direction = 0;
	m_DotCnt = 0;
	m_path = _T("");
	m_nFields = 20;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CNpcPosSet::GetDefaultConnect()
{
	return CServerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CNpcPosSet::GetDefaultSQL()
{
	return _T("[dbo].[K_NPCPOS]");
	//return _T("[dbo].[K_NPCPOS_BACK]");	// sungyong test
}

void CNpcPosSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CNpcPosSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[ZoneID]"), m_ZoneID);
	RFX_Long(pFX, _T("[NpcID]"), m_NpcID);
	RFX_Byte(pFX, _T("[ActType]"), m_ActType);
	RFX_Byte(pFX, _T("[RegenType]"), m_RegenType);
	RFX_Byte(pFX, _T("[DungeonFamily]"), m_DungeonFamily);
	RFX_Byte(pFX, _T("[SpecialType]"), m_SpecialType);
	RFX_Byte(pFX, _T("[TrapNumber]"), m_TrapNumber);
	RFX_Long(pFX, _T("[LeftX]"), m_LeftX);
	RFX_Long(pFX, _T("[TopZ]"), m_TopZ);
	RFX_Long(pFX, _T("[RightX]"), m_RightX);
	RFX_Long(pFX, _T("[BottomZ]"), m_BottomZ);
	RFX_Long(pFX, _T("[LimitMinX]"), m_LimitMinX);
	RFX_Long(pFX, _T("[LimitMinZ]"), m_LimitMinZ);
	RFX_Long(pFX, _T("[LimitMaxX]"), m_LimitMaxX);
	RFX_Long(pFX, _T("[LimitMaxZ]"), m_LimitMaxZ);
	RFX_Byte(pFX, _T("[NumNPC]"), m_NumNPC);
	RFX_Int(pFX, _T("[RegTime]"), m_RegTime);
	RFX_Byte(pFX, _T("[byDirection]"), m_Direction);
	RFX_Byte(pFX, _T("[DotCnt]"), m_DotCnt);
	RFX_Text(pFX, _T("[path]"), m_path);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CNpcPosSet diagnostics

#ifdef _DEBUG
void CNpcPosSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CNpcPosSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
