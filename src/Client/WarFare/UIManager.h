// UIManager.h: interface for the CUIManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIMANAGER_H__1EED48B7_90A3_4242_99BD_ABC12040CDDE__INCLUDED_)
#define AFX_UIMANAGER_H__1EED48B7_90A3_4242_99BD_ABC12040CDDE__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

struct __IconItemSkill;

class CDFont;
class CUIManager : public CN3UIBase
{
protected:
	struct __RenderStateForUI
	{
		DWORD dwZEnable, dwAlphaBlend, dwSrcBlend, dwDestBlend, dwFog;
		DWORD dwMagFilter, dwMinFilter, dwMipFilter;
#ifdef _DEBUG
		BOOL bSet; // 이미 값이 3D device의 값으로 세팅이 되었는지 판단.
#endif
	};

	static __RenderStateForUI s_sRSFU; // RenderStateForUI
	uint32_t m_dwMouseFlagsCur;
	bool m_bEnableOperation;           // UI 조작이 가능한 상태인가?
	CN3UIBase* m_pUIFocused;
#ifdef _DEBUG
	CDFont* m_pDFont;
#endif

public:
	bool m_bDoneSomething; // UI 에서 조작을 했다...

public:
	void UserMoveHideUIs();
	CN3UIBase* GetEnableFocusTopUI(bool bVisible);
	static void RenderStateRestore();
	static void RenderStateSet();

	// UI 조작 가능/불가능..
	void EnableOperationSet(bool bEnable)
	{
		m_bEnableOperation = bEnable;
	}

	// UI 조작이 가능한 상태인가?
	bool EnableOperation()
	{
		return m_bEnableOperation;
	}

	void Release() override;
	void Tick() override;
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;

	uint32_t GetCurMouseFlags() const
	{
		return m_dwMouseFlagsCur;
	}

	// 찍은 UI 가져오기..
	CN3UIBase* GetFocusedUI()
	{
		return m_pUIFocused;
	}

	void SetFocusedUI(CN3UIBase* pUI);  // 찍은 UI 가져오기..
	CN3UIBase* GetTopUI(bool bVisible); // 최상위에 있는 UI 가져오기.
	void SetVisibleFocusedUI(CN3UIBase* pUI);
	CN3UIBase* ReFocusUI();
	void Render() override;
	void ReorderChildList();

	bool BroadcastIconDropMsg(__IconItemSkill* spItem);

	CUIManager();
	~CUIManager() override;
};

#endif // !defined(AFX_UIMANAGER_H__1EED48B7_90A3_4242_99BD_ABC12040CDDE__INCLUDED_)
