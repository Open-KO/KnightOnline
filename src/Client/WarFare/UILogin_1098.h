#ifndef CLIENT_WARFARE_UILOGIN_1098_H
#define CLIENT_WARFARE_UILOGIN_1098_H

#pragma once

#if defined(LOGIN_SCENE_VERSION) && LOGIN_SCENE_VERSION == 1098
#include <string>
#include <vector>

#include <N3Base/N3UIBase.h>

class CUILogIn_1098 : public CN3UIBase
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
	CN3UIEdit* m_pEdit_id;
	CN3UIEdit* m_pEdit_pw;

	CN3UIButton* m_pBtn_LogIn;
	CN3UIButton* m_pBtn_Connect;
	CN3UIButton* m_pBtn_Cancel;
	CN3UIButton* m_pBtn_Option;
	CN3UIButton* m_pBtn_Join;

	CN3UIBase* m_pGroup_ServerList;
	CN3UIBase* m_pGroup_LogIn;

	CN3UIBase* m_pText_Rights;
	CN3UIBase* m_pImg_MGameLogo;
	CN3UIBase* m_pImg_DaumLogo;
	CN3UIBase* m_pImg_GradeLogo;

	CN3UIList* m_pList_Server;

	std::vector<__GameServerInfo> m_ListServerInfos;

	bool m_bOpenningNow; // 위에서 아래로 스르륵...열려야 한다면..
	float m_fMoveDelta;
	bool m_bLogIn;       // 로그인 중복 방지..

public:
	void SetRequestedLogIn(bool bLogIn)
	{
		m_bLogIn = bLogIn;
	}
	bool OnKeyPress(int iKey) override;
	void RecalcGradePos();
	void SetVisibleLogInUIs(bool bEnable); // 계정 LogIn 에 필요한 UI 들을 숨긴다..
	void OpenServerList();
	void Tick() override;

	void InitEditControls();
	void FocusCircular();
	void FocusToID();
	bool Load(File& file) override;
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

	CUILogIn_1098();
	~CUILogIn_1098() override;
};

#endif

#endif // CLIENT_WARFARE_UILOGIN_1098_H
