#if !defined(AFX_HOMESET_H__496F83FF_F4B8_4DBF_803B_BA8E741D9FDB__INCLUDED_)
#define AFX_HOMESET_H__496F83FF_F4B8_4DBF_803B_BA8E741D9FDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HomeSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHomeSet recordset

class CHomeSet : public CRecordset
{
public:
	CHomeSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CHomeSet)

// Field/Param Data
	//{{AFX_FIELD(CHomeSet, CRecordset)
	BYTE	m_Nation;
	long	m_ElmoZoneX;
	long	m_ElmoZoneZ;
	BYTE	m_ElmoZoneLX;
	BYTE	m_ElmoZoneLZ;
	long	m_KarusZoneX;
	long	m_KarusZoneZ;
	BYTE	m_KarusZoneLX;
	BYTE	m_KarusZoneLZ;
	long	m_FreeZoneX;
	long	m_FreeZoneZ;
	BYTE	m_FreeZoneLX;
	BYTE	m_FreeZoneLZ;
//
	long	m_BattleZoneX;
	long	m_BattleZoneZ;
	BYTE	m_BattleZoneLX;
	BYTE	m_BattleZoneLZ;
//
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHomeSet)
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

#endif // !defined(AFX_HOMESET_H__496F83FF_F4B8_4DBF_803B_BA8E741D9FDB__INCLUDED_)
