// UIWarp.h: interface for the UIWarp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIWarp_H__27F6610C_5D61_4A22_97F2_93211B77BF9C__INCLUDED_)
#define AFX_UIWarp_H__27F6610C_5D61_4A22_97F2_93211B77BF9C__INCLUDED_

#pragma once

#include <string>
#include <list>
#include <N3Base/N3UIBase.h>

struct __WarpInfo
{
	int iID                 = 0;  // 이 번호를 서버에게 날린다..
	std::string szName      = {};
	std::string szAgreement = {}; // 유저한테 알리고 동의를 구하는 글..
	int iZone               = 0;
	int iMaxUser            = 0;  // 최대 제한 인원수..
	int iGold               = 0;
	__Vector3 vPos          = {};
};

typedef std::list<__WarpInfo>::iterator it_WI;

class CUIWarp : public CN3UIBase
{
protected:
	class CN3UIButton* m_pBtn_Ok;
	class CN3UIButton* m_pBtn_Cancel;

	class CN3UIList* m_pList_Infos;
	class CN3UIString* m_pText_Agreement; // 동의 사항..

	std::list<__WarpInfo> m_ListInfos;

public:
	bool OnKeyPress(int iKey) override;
	void SetVisible(bool bVisible) override;
	void Reset();
	CUIWarp();
	~CUIWarp() override;

	void InfoAdd(__WarpInfo&& ZW);
	bool InfoGetCur(__WarpInfo& ZW);
	void UpdateList();
	void UpdateAgreement();

	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
};

#endif // !defined(AFX_UIWarp_H__27F6610C_5D61_4A22_97F2_93211B77BF9C__INCLUDED_)
