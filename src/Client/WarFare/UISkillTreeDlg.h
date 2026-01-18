// UISkillTreeDlg.h: interface for the CUISkillTreeDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UISKILLTREEDLG_H__2A724E44_B3A7_41E4_B588_8AF6BC7FB911__INCLUDED_)
#define AFX_UISKILLTREEDLG_H__2A724E44_B3A7_41E4_B588_8AF6BC7FB911__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"

#include <optional>

inline constexpr int SKILL_DEF_BASIC    = 0;
inline constexpr int SKILL_DEF_SPECIAL0 = 1;
inline constexpr int SKILL_DEF_SPECIAL1 = 2;
inline constexpr int SKILL_DEF_SPECIAL2 = 3;
inline constexpr int SKILL_DEF_SPECIAL3 = 4;

//////////////////////////////////////////////////////////////////////

class CUISkillTreeDlg : public CN3UIWndBase
{
protected:
	bool m_bOpenningNow; // 열리고 있다..
	bool m_bClosingNow;  // 닫히고 있다..
	float m_fMoveDelta;  // 부드럽게 열리고 닫히게 만들기 위해서 현재위치 계산에 부동소수점을 쓴다..

	int m_iRBtnDownOffs;

	CN3UIString* m_pStr_info;        // Skill description
	CN3UIString* m_pStr_skill_mp;    // MP Consumed
	CN3UIString* m_pStr_skill_point; // Required skill points
	CN3UIString* m_pStr_skill_item0; // Required item (weapon)
	CN3UIString* m_pStr_skill_item1; // Required item (eg. scrolls or arrows)
	CN3UIString* m_pStr_skill_item2; // Required item (consumables eg. master stones)

public:
	int m_iCurKindOf;
	int m_iCurSkillPage;

	int m_iSkillInfo[MAX_SKILL_FROM_SERVER];                                                   // 서버로 받는 슬롯 정보..
	__IconItemSkill* m_pMySkillTree[MAX_SKILL_KIND_OF][MAX_SKILL_PAGE_NUM][MAX_SKILL_IN_PAGE]; // 총 스킬 정보..

protected:
	void AllClearImageByName(std::string_view svHeaderID, bool bVisible, std::string_view svCategoryID = {});
	RECT GetSampleRect();
	void PageButtonInitialize();
	bool IsSkillUsable(const __TABLE_UPC_SKILL* pUSkill) const;

public:
	void SetVisible(bool bVisible) override;
	CUISkillTreeDlg();
	~CUISkillTreeDlg() override;

	void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false) override;
	bool OnKeyPress(int iKey) override;
	void Release() override;
	void Tick() override;
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnMouseWheelEvent(short delta) override;
	void Render() override;
	void Open();
	void Close();

	void InitIconWnd(e_UIWND eWnd) override;
	void InitIconUpdate() override;

	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) override;
	int GetSkilliOrder(__IconItemSkill* spSkill);

	std::optional<std::pair<int, int>> FindSlotForSkill(const __TABLE_UPC_SKILL* pUSkill, int iCategoryOffset = 0) const;
	void AddSkillToPage(__TABLE_UPC_SKILL* pUSkill, int iCategoryOffset = 0, bool bHasLevelToUse = true);

	void SetPageInIconRegion(int iKindOf, int iPageNum); // 아이콘 역역에서 현재 페이지 설정..
	void SetPageInCharRegion();                          // 문자 역역에서 현재 페이지 설정..

	CN3UIImage* GetChildImageByName(const std::string& szID);
	CN3UIButton* GetChildButtonByName(const std::string& szID);

	void PageLeft();
	void PageRight();

	void PointPushUpButton(int iValue);

	bool HasIDSkill(int iID) const;
	void ButtonVisibleStateSet();

	void TooltipRenderEnable(__IconItemSkill* spSkill);
	void TooltipRenderDisable();
	void CheckButtonTooltipRenderEnable();
	void ButtonTooltipRender(int iIndex);

	void UpdateDisableCheck();
	int GetIndexInArea(POINT pt);
};

#endif // !defined(AFX_UISKILLTREEDLG_H__2A724E44_B3A7_41E4_B588_8AF6BC7FB911__INCLUDED_)
