// UIPartyOrForce.h: interface for the CUIPartyOrForce class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPartyOrForce_H__7B2732B7_C9CA_46A3_89BC_C59934ED3F13__INCLUDED_)
#define AFX_UIPartyOrForce_H__7B2732B7_C9CA_46A3_89BC_C59934ED3F13__INCLUDED_

#pragma once

#include <list>

#include "GameDef.h"
#include <N3Base/N3UIBase.h>

class CUIPartyOrForce : public CN3UIBase                      // 파티에 관한 UI, 부대와 같은 클래스로 쓴다..
{
protected:
	CN3UIProgress* m_pProgress_HPs[MAX_PARTY_OR_FORCE];       // 부대원갯수 만큼... HP Gauge
	CN3UIProgress* m_pProgress_HPReduce[MAX_PARTY_OR_FORCE];  // 부대원갯수 만큼... HP Reduce
	CN3UIProgress* m_pProgress_HPSlow[MAX_PARTY_OR_FORCE];    // HP Slow
	CN3UIProgress* m_pProgress_HPLasting[MAX_PARTY_OR_FORCE]; // HP Lasting
	CN3UIProgress* m_pProgress_MP[MAX_PARTY_OR_FORCE];        // MP Bar
	CN3UIStatic* m_pStatic_IDs[MAX_PARTY_OR_FORCE];           // 부대원갯수 만큼... 이름들..
	CN3UIArea* m_pAreas[MAX_PARTY_OR_FORCE];                  // 부대원갯수 만큼... 이름들..

	std::list<__InfoPartyOrForce> m_Members;                  // 파티 멤버
	int m_iIndexSelected;                                     // 현재 선택된 멤버인덱스..

public:
	bool OnKeyPress(int iKey) override;
	void Tick() override;
	void MemberClassChange(int iID, e_Class eClass);
	void MemberLevelChange(int iID, int iLevel);
	void MemberHPChange(int iID, int iHP, int iHPMax, int iMP, int iMPMax);
	void MemberStatusChange(int iID, e_PartyStatus ePS, bool bSuffer);

	void MemberInfoReInit();        // 파티원 구성이 변경될때.. 순서 및 각종 정보 업데이트..
	bool TargetByIndex(int iIndex); // 순서대로 타겟 잡기..

	int MemberCount() const
	{
		return static_cast<int>(m_Members.size());
	}

	const __InfoPartyOrForce* MemberInfoGetByID(int iID, int& iIndexResult);
	const __InfoPartyOrForce* MemberInfoGetByIndex(int iIndex);
	const __InfoPartyOrForce* MemberInfoGetSelected(); // 현재 선택된 멤버인덱스..
	const __InfoPartyOrForce* MemberAdd(
		int iID, const std::string& szID, int iLevel, e_Class eClass, int iHP, int iHPMax, int iMP, int iMPMax);
	class CPlayerOther* MemberGetByNearst(const __Vector3& vPosPlayer);
	bool MemberRemove(int iID);
	void MemberDestroy();

	void MemberSelect(int iMemberIndex)
	{
		if (iMemberIndex < 0 || iMemberIndex >= static_cast<int>(m_Members.size()))
			return;

		m_iIndexSelected = iMemberIndex;
	}

	bool Load(File& file) override;
	bool ReceiveMessage(class CN3UIBase* pSender, uint32_t dwMsg) override;
	void Render() override;

	void Release() override;
	CUIPartyOrForce();
	~CUIPartyOrForce() override;
};

#endif // !defined(AFX_UIPartyOrForce_H__7B2732B7_C9CA_46A3_89BC_C59934ED3F13__INCLUDED_)
