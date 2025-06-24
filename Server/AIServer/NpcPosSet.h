#if !defined(AFX_NPCPOSSET_H__0F6AE0D2_2327_44B1_8D11_AEB3D8DE2CB9__INCLUDED_)
#define AFX_NPCPOSSET_H__0F6AE0D2_2327_44B1_8D11_AEB3D8DE2CB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NpcPosSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNpcPosSet recordset

class CNpcPosSet : public CRecordset
{
public:
	CNpcPosSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CNpcPosSet)

// Field/Param Data
	//{{AFX_FIELD(CNpcPosSet, CRecordset)
	int		m_ZoneID;
	long	m_NpcID;
	BYTE	m_ActType;
	BYTE	m_RegenType;
	BYTE	m_DungeonFamily;
	BYTE	m_SpecialType;
	BYTE	m_TrapNumber;
	long	m_LeftX;
	long	m_TopZ;
	long	m_RightX;
	long	m_BottomZ;
	long	m_LimitMinX;
	long	m_LimitMinZ;
	long	m_LimitMaxX;
	long	m_LimitMaxZ;
	BYTE	m_NumNPC;
	int		m_RegTime;
	BYTE	m_DotCnt;
	CString	m_path;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNpcPosSet)
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

#endif // !defined(AFX_NPCPOSSET_H__0F6AE0D2_2327_44B1_8D11_AEB3D8DE2CB9__INCLUDED_)
