#if !defined(AFX_MAKEWEAPONTABLESET_H__F80460C1_B7D6_4B77_884F_D225DDF9E9C2__INCLUDED_)
#define AFX_MAKEWEAPONTABLESET_H__F80460C1_B7D6_4B77_884F_D225DDF9E9C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MakeWeaponTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMakeWeaponTableSet recordset

class CMakeWeaponTableSet : public CRecordset
{
public:
	CMakeWeaponTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMakeWeaponTableSet)

// Field/Param Data
	//{{AFX_FIELD(CMakeWeaponTableSet, CRecordset)
	BYTE	m_byLevel;
	int		m_sClass_1;
	int		m_sClass_2;
	int		m_sClass_3;
	int		m_sClass_4;
	int		m_sClass_5;
	int		m_sClass_6;
	int		m_sClass_7;
	int		m_sClass_8;
	int		m_sClass_9;
	int		m_sClass_10;
	int		m_sClass_11;
	int		m_sClass_12;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMakeWeaponTableSet)
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

#endif // !defined(AFX_MAKEWEAPONTABLESET_H__F80460C1_B7D6_4B77_884F_D225DDF9E9C2__INCLUDED_)
