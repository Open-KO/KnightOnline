#if !defined(AFX_MAGICTYPE1SET_H__115E8D26_2D4B_41F6_A83A_14ABB0AFDBB3__INCLUDED_)
#define AFX_MAGICTYPE1SET_H__115E8D26_2D4B_41F6_A83A_14ABB0AFDBB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MagicType1Set.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMagicType1Set recordset

class CMagicType1Set : public CRecordset
{
public:
	CMagicType1Set(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMagicType1Set)

// Field/Param Data
	//{{AFX_FIELD(CMagicType1Set, CRecordset)
	long	m_iNum;
	CString	m_Name;
	CString	m_Description;
	BYTE	m_Type;
	int		m_HitRate;
	int		m_Hit;
	BYTE	m_Delay;
	BYTE	m_ComboType;
	BYTE	m_ComboCount;
	int		m_ComboDamage;
	int		m_Range;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMagicType1Set)
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

#endif // !defined(AFX_MAGICTYPE1SET_H__115E8D26_2D4B_41F6_A83A_14ABB0AFDBB3__INCLUDED_)
