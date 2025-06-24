// NpcTableSet.cpp : implementation file
//

#include "stdafx.h"
#include "ServerDlg.h"
#include "NpcTableSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNpcTableSet

IMPLEMENT_DYNAMIC(CNpcTableSet, CRecordset)

CNpcTableSet::CNpcTableSet(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CNpcTableSet)
	m_sSid = 0;
	m_strName = _T("");
	m_sPid = 0;
	m_sSize = 0;
	m_iWeapon1 = 0;
	m_iWeapon2 = 0;
	m_byGroup = 0;
	m_byActType = 0;
	m_byType = 0;
	m_byFamily = 0;
	m_byRank = 0;
	m_byTitle = 0;
	m_iSellingGroup = 0;
	m_sLevel = 0;
	m_iExp = 0;
	m_iLoyalty = 0;
	m_iHpPoint = 0;
	m_sMpPoint = 0;
	m_sAtk = 0;
	m_sAc = 0;
	m_sHitRate = 0;
	m_sEvadeRate = 0;
	m_sDamage = 0;
	m_sAttackDelay = 0;
	m_bySpeed1 = 0;
	m_bySpeed2 = 0;
	m_sStandtime = 0;
	m_iMagic1 = 0;
	m_iMagic2 = 0;
	m_iMagic3 = 0;
	m_sFireR = 0;
	m_sColdR = 0;
	m_sLightningR = 0;
	m_sMagicR = 0;
	m_sDiseaseR = 0;
	m_sPoisonR = 0;
	m_sLightR = 0;
	m_sBulk = 0;
	m_byAttackRange = 0;
	m_bySearchRange = 0;
	m_byTracingRange = 0;
	m_iMoney = 0;
	m_sItem = 0;
	m_byDirectAttack = 0;
	m_byMagicAttack = 0;
	m_nFields = 45;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}

CString CNpcTableSet::GetDefaultConnect()
{
	return CServerDlg::GetInstance()->GetGameDBConnectionString();
}

CString CNpcTableSet::GetDefaultSQL()
{
	return _T("[dbo].[K_NPC]");
}

void CNpcTableSet::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CNpcTableSet)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[sSid]"), m_sSid);
	RFX_Text(pFX, _T("[strName]"), m_strName);
	RFX_Int(pFX, _T("[sPid]"), m_sPid);
	RFX_Int(pFX, _T("[sSize]"), m_sSize);
	RFX_Long(pFX, _T("[iWeapon1]"), m_iWeapon1);
	RFX_Long(pFX, _T("[iWeapon2]"), m_iWeapon2);
	RFX_Byte(pFX, _T("[byGroup]"), m_byGroup);
	RFX_Byte(pFX, _T("[byActType]"), m_byActType);
	RFX_Byte(pFX, _T("[byType]"), m_byType);
	RFX_Byte(pFX, _T("[byFamily]"), m_byFamily);
	RFX_Byte(pFX, _T("[byRank]"), m_byRank);
	RFX_Byte(pFX, _T("[byTitle]"), m_byTitle);
	RFX_Long(pFX, _T("[iSellingGroup]"), m_iSellingGroup);
	RFX_Int(pFX, _T("[sLevel]"), m_sLevel);
	RFX_Long(pFX, _T("[iExp]"), m_iExp);
	RFX_Long(pFX, _T("[iLoyalty]"), m_iLoyalty);
	RFX_Long(pFX, _T("[iHpPoint]"), m_iHpPoint);
	RFX_Int(pFX, _T("[sMpPoint]"), m_sMpPoint);
	RFX_Int(pFX, _T("[sAtk]"), m_sAtk);
	RFX_Int(pFX, _T("[sAc]"), m_sAc);
	RFX_Int(pFX, _T("[sHitRate]"), m_sHitRate);
	RFX_Int(pFX, _T("[sEvadeRate]"), m_sEvadeRate);
	RFX_Int(pFX, _T("[sDamage]"), m_sDamage);
	RFX_Int(pFX, _T("[sAttackDelay]"), m_sAttackDelay);
	RFX_Byte(pFX, _T("[bySpeed1]"), m_bySpeed1);
	RFX_Byte(pFX, _T("[bySpeed2]"), m_bySpeed2);
	RFX_Int(pFX, _T("[sStandtime]"), m_sStandtime);
	RFX_Long(pFX, _T("[iMagic1]"), m_iMagic1);
	RFX_Long(pFX, _T("[iMagic2]"), m_iMagic2);
	RFX_Long(pFX, _T("[iMagic3]"), m_iMagic3);
	RFX_Int(pFX, _T("[sFireR]"), m_sFireR);
	RFX_Int(pFX, _T("[sColdR]"), m_sColdR);
	RFX_Int(pFX, _T("[sLightningR]"), m_sLightningR);
	RFX_Int(pFX, _T("[sMagicR]"), m_sMagicR);
	RFX_Int(pFX, _T("[sDiseaseR]"), m_sDiseaseR);
	RFX_Int(pFX, _T("[sPoisonR]"), m_sPoisonR);
	RFX_Int(pFX, _T("[sLightR]"), m_sLightR);
	RFX_Int(pFX, _T("[sBulk]"), m_sBulk);
	RFX_Byte(pFX, _T("[byAttackRange]"), m_byAttackRange);
	RFX_Byte(pFX, _T("[bySearchRange]"), m_bySearchRange);
	RFX_Byte(pFX, _T("[byTracingRange]"), m_byTracingRange);
	RFX_Long(pFX, _T("[iMoney]"), m_iMoney);
	RFX_Int(pFX, _T("[sItem]"), m_sItem);
	RFX_Byte(pFX, _T("[byDirectAttack]"), m_byDirectAttack);
	RFX_Byte(pFX, _T("[byMagicAttack]"), m_byMagicAttack);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CNpcTableSet diagnostics

#ifdef _DEBUG
void CNpcTableSet::AssertValid() const
{
	CRecordset::AssertValid();
}

void CNpcTableSet::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
