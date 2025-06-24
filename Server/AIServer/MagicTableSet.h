#if !defined(AFX_MAGICTABLESET_H__EEE22857_0064_4E58_AD5A_E33D8E9C90E9__INCLUDED_)
#define AFX_MAGICTABLESET_H__EEE22857_0064_4E58_AD5A_E33D8E9C90E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MagicTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMagicTableSet recordset

class CMagicTableSet : public CRecordset
{
public:
	CMagicTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMagicTableSet)

// Field/Param Data
	//{{AFX_FIELD(CMagicTableSet, CRecordset)
	long	m_MagicNum;
	CString	m_EnName;
	CString	m_KrName;
	CString	m_Description;
	BYTE	m_BeforeAction;
	BYTE	m_TargetAction;
	BYTE	m_SelfEffect;
	BYTE	m_FlyingEffect;
	int		m_TargetEffect;
	BYTE	m_Moral;
	int		m_SkillLevel;
	int		m_Skill;
	int		m_Msp;
	int     m_HP;
	BYTE	m_ItemGroup;
	long	m_UseItem;
	BYTE	m_CastTime;
	BYTE	m_ReCastTime;
	BYTE	m_SuccessRate;
	BYTE	m_Type1;
	BYTE	m_Type2;
	int     m_Range;
	BYTE	m_Etc;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMagicTableSet)
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

#endif // !defined(AFX_MAGICTABLESET_H__EEE22857_0064_4E58_AD5A_E33D8E9C90E9__INCLUDED_)
