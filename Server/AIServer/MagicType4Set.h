#if !defined(AFX_MAGICTYPE4SET_H__484D4528_17D7_4F03_954C_EACDB8011DE7__INCLUDED_)
#define AFX_MAGICTYPE4SET_H__484D4528_17D7_4F03_954C_EACDB8011DE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MagicType4Set.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMagicType4Set recordset

class CMagicType4Set : public CRecordset
{
public:
	CMagicType4Set(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMagicType4Set)

// Field/Param Data
	//{{AFX_FIELD(CMagicType4Set, CRecordset)
	long	m_iNum;
	CString	m_Name;
	CString	m_Description;
	BYTE	m_BuffType;
	BYTE	m_Radius;
	int		m_Duration;
	BYTE	m_AttackSpeed;
	BYTE	m_Speed;
	int		m_AC;
	BYTE	m_Attack;
	int		m_MaxHP;
	BYTE	m_HitRate;
	int		m_AvoidRate;
	BYTE	m_Str;
	BYTE	m_Sta;
	BYTE	m_Dex;
	BYTE	m_Intel;
	BYTE	m_Cha;
	BYTE	m_FireR;
	BYTE	m_ColdR;
	BYTE	m_LightningR;
	BYTE	m_MagicR;
	BYTE	m_DiseaseR;
	BYTE	m_PoisonR;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMagicType4Set)
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

#endif // !defined(AFX_MAGICTYPE4SET_H__484D4528_17D7_4F03_954C_EACDB8011DE7__INCLUDED_)
