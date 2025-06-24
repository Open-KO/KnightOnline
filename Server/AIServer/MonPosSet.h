#if !defined(AFX_MONPOSSET_H__2202E742_7755_437E_B97F_CF0F2F5A6578__INCLUDED_)
#define AFX_MONPOSSET_H__2202E742_7755_437E_B97F_CF0F2F5A6578__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MonPosSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMonPosSet recordset

class CMonPosSet : public CRecordset
{
public:
	CMonPosSet(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CMonPosSet)

// Field/Param Data
	//{{AFX_FIELD(CMonPosSet, CRecordset)
	int		m_ZoneID;
	long	m_NpcID;
	BYTE	m_ActType;
	long	m_LeftX;
	long	m_TopZ;
	long	m_RightX;
	long	m_BottomZ;
	BYTE	m_NumNPC;
	int		m_RegTime;
	BYTE	m_DotCnt;
	CString	m_path;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMonPosSet)
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

#endif // !defined(AFX_MONPOSSET_H__2202E742_7755_437E_B97F_CF0F2F5A6578__INCLUDED_)
