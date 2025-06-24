#if !defined(AFX_ZONEINFOSET_H__57E9D448_ABEC_4D91_9933_B1DEBCF3D915__INCLUDED_)
#define AFX_ZONEINFOSET_H__57E9D448_ABEC_4D91_9933_B1DEBCF3D915__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZoneInfoSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CZoneInfoSet recordset

class CZoneInfoSet : public CRecordset
{
public:
	CZoneInfoSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CZoneInfoSet)

// Field/Param Data
	//{{AFX_FIELD(CZoneInfoSet, CRecordset)
	int		m_ServerNo;
	int		m_ZoneNo;
	CString	m_strZoneName;
	long	m_InitX;
	long	m_InitZ;
	long	m_InitY;
	BYTE	m_Type;
	BYTE    m_RoomEvent;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoneInfoSet)
public:
	virtual CString GetDefaultConnect();    // Default connection string
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZONEINFOSET_H__57E9D448_ABEC_4D91_9933_B1DEBCF3D915__INCLUDED_)
