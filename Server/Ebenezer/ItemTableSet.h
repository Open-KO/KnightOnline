#if !defined(AFX_ITEMTABLESET_H__9C214CAF_6316_4E33_84A4_8DEBC3D83176__INCLUDED_)
#define AFX_ITEMTABLESET_H__9C214CAF_6316_4E33_84A4_8DEBC3D83176__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ItemTableSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CItemTableSet recordset

class CItemTableSet : public CRecordset
{
public:
	CItemTableSet(CDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CItemTableSet)

// Field/Param Data
	//{{AFX_FIELD(CItemTableSet, CRecordset)
	long	m_Num;
	CString	m_strName;
	BYTE	m_Kind;
	BYTE	m_Slot;
	BYTE	m_Race;
	BYTE	m_Class;
	int		m_Damage;
	int		m_Delay;
	int		m_Range;
	int		m_Weight;
	int		m_Duration;
	long	m_BuyPrice;
	long	m_SellPrice;
	int		m_Ac;
	BYTE	m_Countable;
	long	m_Effect1;
	long	m_Effect2;
	BYTE	m_ReqLevel;
	BYTE	m_ReqRank;
	BYTE	m_ReqTitle;
	BYTE	m_ReqStr;
	BYTE	m_ReqSta;
	BYTE	m_ReqDex;
	BYTE	m_ReqIntel;
	BYTE	m_ReqCha;
	BYTE	m_SellingGroup;
	BYTE	m_ItemType;
	int		m_Hitrate;
	int		m_Evasionrate;
	int		m_DaggerAc;
	int		m_SwordAc;
	int		m_MaceAc;
	int		m_AxeAc;
	int		m_SpearAc;
	int		m_BowAc;
	BYTE	m_FireDamage;
	BYTE	m_IceDamage;
	BYTE	m_LightningDamage;
	BYTE	m_PoisonDamage;
	BYTE	m_HPDrain;
	BYTE	m_MPDamage;
	BYTE	m_MPDrain;
	BYTE	m_MirrorDamage;
	BYTE	m_Droprate;
	int		m_StrB;
	int		m_StaB;
	int		m_DexB;
	int		m_IntelB;
	int		m_ChaB;
	int		m_MaxHpB;
	int		m_MaxMpB;
	int		m_FireR;
	int		m_ColdR;
	int		m_LightningR;
	int		m_MagicR;
	int		m_PoisonR;
	int		m_CurseR;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CItemTableSet)
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

#endif // !defined(AFX_ITEMTABLESET_H__9C214CAF_6316_4E33_84A4_8DEBC3D83176__INCLUDED_)
