// UIKnightsOperation.h: interface for the CUIKnightsOperation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIKNIGHTSOPERATION_H__1AE95FA6_E4DE_45DF_9EEA_6AEDE400ECA7__INCLUDED_)
#define AFX_UIKNIGHTSOPERATION_H__1AE95FA6_E4DE_45DF_9EEA_6AEDE400ECA7__INCLUDED_

#pragma once

#include "GameDef.h"

#include <N3Base/N3UIBase.h>

#include <list>
#include <string>

struct __KnightsInfoBase
{
	int iID = 0;
	std::string szName;
};

struct __KnightsInfoExt : public __KnightsInfoBase
{
	std::string szChiefName;
	int iMemberCount = 0;
	int iPoint       = 0;
};

typedef std::map<int, __KnightsInfoBase>::iterator it_KIB;
typedef std::map<int, __KnightsInfoBase>::value_type val_KIB;

typedef std::list<__KnightsInfoExt>::iterator it_KIE;

class Packet;
class CUIKnightsOperation : public CN3UIBase
{
protected:
	int m_iPageCur;
	std::map<int, __KnightsInfoBase> m_KnightsMapBase;
	std::list<__KnightsInfoExt> m_KnightsListExt;

	CN3UIButton* m_pBtn_Up;
	CN3UIButton* m_pBtn_Down;
	CN3UIButton* m_pBtn_Close;

	CN3UIList* m_pList_Knights; //
	CN3UIButton* m_pBtn_Join;
	CN3UIButton* m_pBtn_Create;
	CN3UIButton* m_pBtn_Destroy;
	CN3UIButton* m_pBtn_Withdraw;
	CN3UIEdit* m_pEdit_KnightsName;

public:
	void ChangeUIByDuty(e_KnightsDuty eDuty); // 권한에 따라 UI 변경..
	void Open(e_KnightsDuty eDuty);
	void Close();

	__KnightsInfoBase* KnightsInfoFind(int iID);
	void KnightsInfoInsert(int iID, const std::string& szName);
	void KnightsInfoDelete(int iID);

	void KnightsListAdd(int iID, const std::string& szName, const std::string& szChiefName, int iMemberCount, int iKnightsPoint);
	void KnightsListUpdate();
	void KnightsListClear();

	bool MsgRecv_KnightsList(Packet& pkt);
	void MsgSend_KnightsCreate();
	void MsgSend_KnightsDestroy();
	void MsgSend_KnightsJoin();
	void MsgSend_KnightsWithdraw();      // 탈퇴.
	void MsgSend_KnightsList(int iPage); // 페이지 단위로 기사단들 정보 요청.. 10 개씩 온다.

	void EnableKnightsUIs(bool bEnable);

	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool Load(File& file) override;

	void Release() override;
	CUIKnightsOperation();
	~CUIKnightsOperation() override;
};

#endif // !defined(AFX_UIKNIGHTSOPERATION_H__1AE95FA6_E4DE_45DF_9EEA_6AEDE400ECA7__INCLUDED_)
