// ItemTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "AujardDlg.h"
#include "ItemTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemTableSet

IMPLEMENT_DYNAMIC(CItemTableSet, CRecordset)

CItemTableSet::CItemTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CItemTableSet)
	m_Num = 0;
	m_strName = _T("");
	m_Kind = 0;
	m_Slot = 0;
	m_Race = 0;
	m_Class = 0;
	m_Damage = 0;
	m_Delay = 0;
	m_Range = 0;
	m_Weight = 0;
	m_Duration = 0;
	m_BuyPrice = 0;
	m_SellPrice = 0;
	m_Ac = 0;
	m_Countable = 0;
	m_Effect1 = 0;
	m_Effect2 = 0;
	m_ReqLevel = 0;
	m_ReqRank = 0;
	m_ReqTitle = 0;
	m_ReqStr = 0;
	m_ReqSta = 0;
	m_ReqDex = 0;
	m_ReqIntel = 0;
	m_ReqCha = 0;
	m_SellingGroup = 0;
	m_ItemType = 0;
	m_Hitrate = 0;
	m_Evasionrate = 0;
	m_DaggerAc = 0;
	m_SwordAc = 0;
	m_MaceAc = 0;
	m_AxeAc = 0;
	m_SpearAc = 0;
	m_BowAc = 0;
	m_FireDamage = 0;
	m_IceDamage = 0;
	m_LightningDamage = 0;
	m_PoisonDamage = 0;
	m_HPDrain = 0;
	m_MPDamage = 0;
	m_MPDrain = 0;
	m_MirrorDamage = 0;
	m_Droprate = 0;
	m_StrB = 0;
	m_StaB = 0;
	m_DexB = 0;
	m_IntelB = 0;
	m_ChaB = 0;
	m_FireR = 0;
	m_ColdR = 0;
	m_LightningR = 0;
	m_MagicR = 0;
	m_PoisonR = 0;
	m_CurseR = 0;
	m_nFields = 57;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CItemTableSet::GetDefaultConnect()
{
	return CAujardDlg::GetInstance()->GetGameDBConnectionString();
}

CString CItemTableSet::GetDefaultSQL()
{
	return _T("[dbo].[ITEM]");
}

void CItemTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CItemTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[Num]"), m_Num);
	RFX_Text(pFX, _T("[strName]"), m_strName);
	RFX_Byte(pFX, _T("[Kind]"), m_Kind);
	RFX_Byte(pFX, _T("[Slot]"), m_Slot);
	RFX_Byte(pFX, _T("[Race]"), m_Race);
	RFX_Byte(pFX, _T("[Class]"), m_Class);
	RFX_Int(pFX, _T("[Damage]"), m_Damage);
	RFX_Int(pFX, _T("[Delay]"), m_Delay);
	RFX_Int(pFX, _T("[Range]"), m_Range);
	RFX_Int(pFX, _T("[Weight]"), m_Weight);
	RFX_Int(pFX, _T("[Duration]"), m_Duration);
	RFX_Long(pFX, _T("[BuyPrice]"), m_BuyPrice);
	RFX_Long(pFX, _T("[SellPrice]"), m_SellPrice);
	RFX_Int(pFX, _T("[Ac]"), m_Ac);
	RFX_Byte(pFX, _T("[Countable]"), m_Countable);
	RFX_Long(pFX, _T("[Effect1]"), m_Effect1);
	RFX_Long(pFX, _T("[Effect2]"), m_Effect2);
	RFX_Byte(pFX, _T("[ReqLevel]"), m_ReqLevel);
	RFX_Byte(pFX, _T("[ReqRank]"), m_ReqRank);
	RFX_Byte(pFX, _T("[ReqTitle]"), m_ReqTitle);
	RFX_Byte(pFX, _T("[ReqStr]"), m_ReqStr);
	RFX_Byte(pFX, _T("[ReqSta]"), m_ReqSta);
	RFX_Byte(pFX, _T("[ReqDex]"), m_ReqDex);
	RFX_Byte(pFX, _T("[ReqIntel]"), m_ReqIntel);
	RFX_Byte(pFX, _T("[ReqCha]"), m_ReqCha);
	RFX_Byte(pFX, _T("[SellingGroup]"), m_SellingGroup);
	RFX_Byte(pFX, _T("[ItemType]"), m_ItemType);
	RFX_Int(pFX, _T("[Hitrate]"), m_Hitrate);
	RFX_Int(pFX, _T("[Evasionrate]"), m_Evasionrate);
	RFX_Int(pFX, _T("[DaggerAc]"), m_DaggerAc);
	RFX_Int(pFX, _T("[SwordAc]"), m_SwordAc);
	RFX_Int(pFX, _T("[MaceAc]"), m_MaceAc);
	RFX_Int(pFX, _T("[AxeAc]"), m_AxeAc);
	RFX_Int(pFX, _T("[SpearAc]"), m_SpearAc);
	RFX_Int(pFX, _T("[BowAc]"), m_BowAc);
	RFX_Byte(pFX, _T("[FireDamage]"), m_FireDamage);
	RFX_Byte(pFX, _T("[IceDamage]"), m_IceDamage);
	RFX_Byte(pFX, _T("[LightningDamage]"), m_LightningDamage);
	RFX_Byte(pFX, _T("[PoisonDamage]"), m_PoisonDamage);
	RFX_Byte(pFX, _T("[HPDrain]"), m_HPDrain);
	RFX_Byte(pFX, _T("[MPDamage]"), m_MPDamage);
	RFX_Byte(pFX, _T("[MPDrain]"), m_MPDrain);
	RFX_Byte(pFX, _T("[MirrorDamage]"), m_MirrorDamage);
	RFX_Byte(pFX, _T("[Droprate]"), m_Droprate);
	RFX_Int(pFX, _T("[StrB]"), m_StrB);
	RFX_Int(pFX, _T("[StaB]"), m_StaB);
	RFX_Int(pFX, _T("[DexB]"), m_DexB);
	RFX_Int(pFX, _T("[IntelB]"), m_IntelB);
	RFX_Int(pFX, _T("[ChaB]"), m_ChaB);
	RFX_Int(pFX, _T("[MaxHpB]"), m_MaxHpB);
	RFX_Int(pFX, _T("[MaxMpB]"), m_MaxMpB);
	RFX_Int(pFX, _T("[FireR]"), m_FireR);
	RFX_Int(pFX, _T("[ColdR]"), m_ColdR);
	RFX_Int(pFX, _T("[LightningR]"), m_LightningR);
	RFX_Int(pFX, _T("[MagicR]"), m_MagicR);
	RFX_Int(pFX, _T("[PoisonR]"), m_PoisonR);
	RFX_Int(pFX, _T("[CurseR]"), m_CurseR);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CItemTableSet diagnostics

#ifdef _DEBUG
void CItemTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CItemTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
