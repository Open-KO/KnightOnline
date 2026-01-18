// UIPartyBBS.h: interface for the CUIPartyBBS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPartyBBS_H__7B2732B7_C9CA_46A3_89BC_C59934ED3F13__INCLUDED_)
#define AFX_UIPartyBBS_H__7B2732B7_C9CA_46A3_89BC_C59934ED3F13__INCLUDED_

#pragma once

#include <list>

#include <N3Base/N3UIBase.h>
#include "GameDef.h" // __InfoPartyBBS

typedef std::list<__InfoPartyBBS>::iterator it_PartyBBS;

class CUIPartyBBS : public CN3UIBase // 파티에 관한 UI, 부대와 같은 클래스로 쓴다..
{
	static constexpr int PARTY_BBS_MAXSTRING = 69;
	static constexpr int PARTY_BBS_MAXLINE   = 23;

protected:
	CN3UIButton* m_pBtn_PageUp;
	CN3UIButton* m_pBtn_PageDown;
	CN3UIButton* m_pBtn_Refresh;

	CN3UIButton* m_pBtn_Close;
	CN3UIButton* m_pBtn_Register;
	CN3UIButton* m_pBtn_RegisterCancel;
	CN3UIButton* m_pBtn_Whisper;
	CN3UIButton* m_pBtn_Party;

	CN3UIString* m_pText_Page;
	CN3UIString* m_pText[PARTY_BBS_MAXSTRING];

	std::list<__InfoPartyBBS> m_Datas; // BBS Data
	int m_iCurPage;                    // 현재 페이지..
	int m_iMaxPage;                    // 총 페이지..
	int m_iCurIndex;
	bool m_bProcessing;
	float m_fTime;

public:
	bool OnKeyPress(int iKey) override;
	void SetVisible(bool bVisible) override;
	void RequestParty();
	void RequestWhisper();
	void SetStringColor(int iIndex, uint32_t dwColor);
	void RenderSelectContent();
	void Render() override;
	void SetContentString(int iIndex, const std::string& szID, int iLevel, const std::string& szClass);
	void ResetContent();
	void MsgSend_Register();
	void MsgSend_RegisterCancel();
	void MsgSend_RefreshData(int iCurPage);

	void MsgRecv_RefreshData(Packet& pkt);

	void PartyStringSet(uint8_t byType);
	void RefreshPage();
	bool Load(File& file) override;
	bool ReceiveMessage(class CN3UIBase* pSender, uint32_t dwMsg) override;
	bool SelectedString(CN3UIBase* pSender, int& iID);

	CUIPartyBBS();
	~CUIPartyBBS() override;
};

#endif // !defined(AFX_UIPartyBBS_H__7B2732B7_C9CA_46A3_89BC_C59934ED3F13__INCLUDED_)
