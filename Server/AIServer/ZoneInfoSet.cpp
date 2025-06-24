// ZoneInfoSet.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ZoneInfoSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZoneInfoSet

IMPLEMENT_DYNAMIC(CZoneInfoSet, CRecordset)

CZoneInfoSet::CZoneInfoSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CZoneInfoSet)
	m_ServerNo = 0;
	m_ZoneNo = 0;
	m_strZoneName = _T("");
	m_InitX = 0;
	m_InitZ = 0;
	m_InitY = 0;
	m_Type = 0;
	// dungeon work
	m_RoomEvent = 0;
	m_nFields = 8;
	//m_nFields = 7;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CZoneInfoSet::GetDefaultConnect()
{
	return _T("ODBC;DSN=KN_Online;UID=knight;PWD=knight");
}

CString CZoneInfoSet::GetDefaultSQL()
{
//	return _T("[dbo].[ZONE_INFO2]");
	return _T("[dbo].[ZONE_INFO]");
}

void CZoneInfoSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CZoneInfoSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[ServerNo]"), m_ServerNo);
	RFX_Int(pFX, _T("[ZoneNo]"), m_ZoneNo);
	RFX_Text(pFX, _T("[strZoneName]"), m_strZoneName);
	RFX_Long(pFX, _T("[InitX]"), m_InitX);
	RFX_Long(pFX, _T("[InitZ]"), m_InitZ);
	RFX_Long(pFX, _T("[InitY]"), m_InitY);
	RFX_Byte(pFX, _T("[Type]"), m_Type);
	// dungeon work
	RFX_Byte(pFX, _T("[RoomEvent]"), m_RoomEvent);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CZoneInfoSet diagnostics

#ifdef _DEBUG
void CZoneInfoSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CZoneInfoSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
