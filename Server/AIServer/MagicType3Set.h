#if !defined(AFX_MAGICTYPE3SET_H__BCF18D34_77DB_4B67_9556_41C14D524D57__INCLUDED_)
#define AFX_MAGICTYPE3SET_H__BCF18D34_77DB_4B67_9556_41C14D524D57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MagicType3Set.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMagicType3Set recordset

class CMagicType3Set : public CRecordset
{
public:
	CMagicType3Set(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMagicType3Set)

// Field/Param Data
	//{{AFX_FIELD(CMagicType3Set, CRecordset)
	long	m_iNum;
	CString	m_Name;
	CString	m_Description;
	BYTE	m_Radius;
	int		m_Angle;
	BYTE	m_DirectType;
	int		m_FirstDamage;
	int		m_EndDamage;
	int		m_TimeDamage;
	BYTE	m_Duration;
	BYTE	m_Attribute;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMagicType3Set)
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

#endif // !defined(AFX_MAGICTYPE3SET_H__BCF18D34_77DB_4B67_9556_41C14D524D57__INCLUDED_)
