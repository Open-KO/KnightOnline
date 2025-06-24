#if !defined(AFX_MONTABLESET_H__9A52F839_5F69_4BC3_B71F_296D06461F35__INCLUDED_)
#define AFX_MONTABLESET_H__9A52F839_5F69_4BC3_B71F_296D06461F35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MonTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMonTableSet recordset

class CMonTableSet : public CRecordset
{
public:
	CMonTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CMonTableSet)

// Field/Param Data
	//{{AFX_FIELD(CMonTableSet, CRecordset)
	int		m_sSid;
	CString	m_strName;
	int		m_sPid;
	int		m_sSize;
	long 	m_iWeapon1;
	long 	m_iWeapon2;
	BYTE	m_byGroup;
	BYTE	m_byActType;
	BYTE	m_byType;
	BYTE	m_byFamily;
	BYTE	m_byRank;
	BYTE	m_byTitle;
	long	m_iSellingGroup;
	int		m_sLevel;
	long	m_iExp;
	long	m_iLoyalty;
	long	m_iHpPoint;
	int		m_sMpPoint;
	int		m_sAtk;
	int		m_sAc;
	int		m_sHitRate;
	int		m_sEvadeRate;
	int		m_sDamage;
	int		m_sAttackDelay;
	BYTE	m_bySpeed1;
	BYTE	m_bySpeed2;
	int		m_sStandtime;
	long	m_iMagic1;
	long	m_iMagic2;
	long	m_iMagic3;
	BYTE	m_byFireR;
	BYTE	m_byColdR;
	BYTE	m_byLightningR;
	BYTE	m_byMagicR;
	BYTE	m_byDiseaseR;
	BYTE	m_byPoisonR;
	BYTE	m_byLightR;
	int		m_sBulk;
	BYTE	m_byAttackRange;
	BYTE	m_bySearchRange;
	BYTE	m_byTracingRange;
	long	m_iMoney;
	int		m_sItem;
	BYTE	m_byDirectAttack;
	BYTE	m_byMagicAttack;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMonTableSet)
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

#endif // !defined(AFX_MONTABLESET_H__9A52F839_5F69_4BC3_B71F_296D06461F35__INCLUDED_)
