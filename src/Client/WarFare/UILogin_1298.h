#ifndef CLIENT_WARFARE_UILOGIN_1298_H
#define CLIENT_WARFARE_UILOGIN_1298_H

#pragma once

#include <string>
#include <vector>

#include <N3Base/N3UIBase.h>

class CUILogIn_1298 : public CN3UIBase
{
public:
	struct __GameServerInfo
	{
		std::string szName;
		std::string szIP;
		int iConcurrentUserCount;

		void Init()
		{
			szName.clear();
			szIP.clear();
			iConcurrentUserCount = 0;
		}

		bool operator()(const __GameServerInfo& x, const __GameServerInfo& y) const
		{
			return (x.iConcurrentUserCount >= y.iConcurrentUserCount);
		}

		__GameServerInfo()
		{
			Init();
		}

		__GameServerInfo(const std::string& szName2, const std::string& szIP2, int iConcurrentUserCount2)
		{
			szName               = szName2;
			szIP                 = szIP2;
			iConcurrentUserCount = iConcurrentUserCount2;
		}
	};

protected:
	static constexpr int MAX_SERVERS = 20; // max number of servers in UIF file.

	CN3UIEdit* m_pEdit_id;
	CN3UIEdit* m_pEdit_pw;

	CN3UIButton* m_pBtn_LogIn;
	CN3UIButton* m_pBtn_Connect;
	CN3UIButton* m_pBtn_Cancel;
	CN3UIButton* m_pBtn_Option;
	CN3UIButton* m_pBtn_Join;

	CN3UIButton* m_pBtn_NoticeOK_1;
	CN3UIButton* m_pBtn_NoticeOK_2;
	CN3UIButton* m_pBtn_NoticeOK_3;

	CN3UIString* m_pStr_Premium;

	CN3UIBase* m_pGroup_ServerList;
	CN3UIBase* m_pGroup_LogIn;

	CN3UIBase* m_pGroup_Notice_1;        //notice group
	CN3UIString* m_pText_Notice1_Name_1; //notice title
	CN3UIString* m_pText_Notice1_Text_1; //notice text

	CN3UIBase* m_pGroup_Notice_2;
	CN3UIString* m_pText_Notice2_Name_1;
	CN3UIString* m_pText_Notice2_Text_1;
	CN3UIString* m_pText_Notice2_Name_2;
	CN3UIString* m_pText_Notice2_Text_2;

	CN3UIBase* m_pGroup_Notice_3;
	CN3UIString* m_pText_Notice3_Name_1;
	CN3UIString* m_pText_Notice3_Text_1;
	CN3UIString* m_pText_Notice3_Name_2;
	CN3UIString* m_pText_Notice3_Text_2;
	CN3UIString* m_pText_Notice3_Name_3;
	CN3UIString* m_pText_Notice3_Text_3;

	CN3UIBase* m_pServer_Group[MAX_SERVERS];
	CN3UIBase* m_pArrow_Group[MAX_SERVERS];
	CN3UIString* m_pList_Group[MAX_SERVERS];

	std::vector<__GameServerInfo> m_ListServerInfos;

	bool m_bIsNewsVisible;
	bool m_bLogIn; // 로그인 중복 방지..
	int m_iSelectedServerIndex;
	std::string m_strNoticeText;

public:
	void SetRequestedLogIn(bool bLogIn)
	{
		m_bLogIn = bLogIn;
	}
	void SelectServer(int iServerListIndex);
	bool OnKeyPress(int iKey) override;
	void SetVisibleLogInUIs(bool bEnable); // 계정 LogIn 에 필요한 UI 들을 숨긴다..
	void OpenServerList();
	void OpenNews();
	void AddNews(const std::string& strNews);

	void InitEditControls();
	void FocusCircular();
	void FocusToID();
	bool Load(File& file) override;
	void PositionGroups();

	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override; // 메시지를 받는다.. 보낸놈, msg

	int ServerInfoCount()
	{
		return (int) m_ListServerInfos.size();
	}
	bool ServerInfoAdd(const __GameServerInfo& GSI);
	bool ServerInfoGet(int iIndex, __GameServerInfo& GSI);
	bool ServerInfoGetCur(__GameServerInfo& GSI);
	void ServerInfoUpdate();

	void AccountIDGet(std::string& szID);
	void AccountPWGet(std::string& szPW);

	void ConnectButtonSetEnable(bool bEnable);

	CUILogIn_1298();
	~CUILogIn_1298() override;
};

#endif // CLIENT_WARFARE_UILOGIN_1298_H
