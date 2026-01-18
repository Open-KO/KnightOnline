// N3UIBase.h: interface for the CN3UIBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UI_H__5F1578F8_8476_4647_8C53_E22A5F6184FE__INCLUDED_)
#define AFX_N3UI_H__5F1578F8_8476_4647_8C53_E22A5F6184FE__INCLUDED_

#pragma once

#include "N3BaseFileAccess.h"
#include "N3UIDef.h"
#include <list>
#include <set>
#include <string>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define N3_VERIFY_UI_COMPONENT(varName, lookupResult)           \
	{                                                           \
		(varName) = (lookupResult);                             \
		__ASSERT((varName) != nullptr, "NULL UI Component!!!"); \
	}

class CN3UIBase;

typedef std::list<CN3UIBase*> UIList;
typedef UIList::iterator UIListItor;
typedef UIList::const_iterator UIListItorConst;
typedef UIList::reverse_iterator UIListReverseItor;

class CN3UIImage;
class CN3UIString;
class CN3UIButton;
class CN3UIStatic;
class CN3UIProgress;
class CN3UITrackBar;
class CN3UIScrollBar;
class CN3UIEdit;
class CN3UITooltip;
class CN3UIArea;
class CN3SndObj;
class CN3UIList;

class CN3UIBase : public CN3BaseFileAccess
{
#ifdef _N3TOOL
	friend class CN3UIBase;
	friend class CHierarchyView; // 툴에서 child list를 접근하기 위해서.
	friend class CPropertyView;  // 툴에서 각 변수들을 접근하기 위해서
	friend class CUIEView;       // 툴에서 child list를 접근하기 위해서.
#endif

public:
	static constexpr D3DCOLOR DefaultTooltipColor = D3DCOLOR_XRGB(255, 255, 255);

#ifdef _N3GAME
	static bool s_bWaitFromServer;
#endif

	std::string m_szID;      // UI id
	std::string m_szToolTip; // tooltip text
	D3DCOLOR m_crToolTip;

	void SetID(const std::string& szID)
	{
		m_szID = szID;
	}

	const std::string& GetID() const
	{
		return m_szID;
	}

	void SetTooltipText(const std::string& szTooltipText)
	{
		m_szToolTip = szTooltipText;
	}

	void SetTooltipColor(D3DCOLOR crTooltip)
	{
		m_crToolTip = crTooltip;
	}

	static CN3UITooltip* s_pTooltipCtrl; // tool tip

	CN3UIBase* m_pChildUI;               // UI 부속이 아니라 다른 UI를 자식으로 갖는다..
	CN3UIBase* m_pParentUI;

protected:
	static std::string s_szStringTmp; // 임시 문자열.. 포인터를 넘기기 위해서이다..

	int m_iChildID;
	CN3UIBase* m_pParent;             // parent pointer
	UIList m_Children;                // children pointer list
	eUI_TYPE m_eType;                 // UI Type - button, image .....
	eUI_STATE m_eState;               // UI state
	uint32_t m_dwStyle;               // style
	uint32_t m_dwReserved;            // 기타 임시로 넣고 싶은 정보를 넣으면 된다.

	// UI - screen coordinates (screen : main window client area) 중의 : 부모에 대한 상대좌표가 아니다.
	RECT m_rcRegion;

	// UI를 드래그 하여 움직이게 할 수 있는 영역 - (screen : main window client area)           ~~~~~~~
	RECT m_rcMovable;

	bool m_bVisible;          // 화면에 보이는가 (부모가 보이지 않으면 자식들은 render 하지 않는다.)
	CN3SndObj* m_pSnd_OpenUI; // UI가 화면에 보이는 순간 내는 소리
	CN3SndObj* m_pSnd_CloseUI; // UI가 화면에서 사라지는 순간 내는 소리

	// 현재 포커스를 가지고 있는 Edit, nullptr이면 아무도 포커스를 가지고 있지 않다.
	static CN3UIEdit* s_pFocusedEdit;

public:
	CN3UIBase();
	~CN3UIBase() override;

	// Attributes
public:
	eUI_TYPE UIType() const
	{
		return m_eType;
	}

	eUI_STATE GetState() const
	{
		return m_eState;
	}

	bool IsVisible() const
	{
		return m_bVisible;
	}

	RECT GetRegion() const
	{
		return m_rcRegion;
	}

	void SetMoveRect(const RECT& Rect)
	{
		m_rcMovable = Rect;
	}

	RECT GetMoveRect() const
	{
		return m_rcMovable;
	}

	void SetReserved(uint32_t dwReserved)
	{
		m_dwReserved = dwReserved;
	}

	uint32_t GetReserved() const
	{
		return m_dwReserved;
	}

	CN3UIBase* GetParent() const
	{
		return m_pParent;
	}

	static CN3UIEdit* GetFocusedEdit()
	{
		return s_pFocusedEdit;
	}

	static CN3UITooltip* GetTooltipCtrl()
	{
		return s_pTooltipCtrl;
	}

	uint32_t GetStyle() const
	{
		return m_dwStyle;
	}

	const UIList& GetChildren() const
	{
		return m_Children;
	}

	void SetUIType(eUI_TYPE eUIType)
	{
		m_eType = eUIType;
	}

	// Operations
public:
	bool IsIn(int x, int y);

	void AddChild(CN3UIBase* pChild)
	{
		m_Children.push_front(pChild);
	}

	void RemoveChild(
		CN3UIBase* pChild); // 자식 리스트에서 포인터만 없어지고 실제로 delete 되지는 않는다.
	void SetParent(CN3UIBase* pParent); // 부모를 바꾼다.

	int GetWidth() const
	{
		return m_rcRegion.right - m_rcRegion.left;
	}

	int GetHeight() const
	{
		return m_rcRegion.bottom - m_rcRegion.top;
	}

	POINT GetPos() const;

	// 위치 지정(chilren의 위치도 같이 바꾸어준다.) 내부적으로 MoveOffset함수를 부른다.
	virtual void SetPos(int x, int y);

	// 화면 정가운데로 맞추어준다..(chilren의 위치도 같이 바꾸어준다.) 내부적으로 MoveOffset함수를 부른다.
	void SetPosCenter();

	// Find first control matching the specified ID.
	CN3UIBase* GetChildByID(const std::string_view szID) const;

	// Find first control matching both the specified ID and UI type.
	CN3UIBase* GetChildByID(const std::string_view szID, eUI_TYPE eUIType) const;

	template <typename T>
	T* GetChildByID(const std::string_view szID) const;

	// 영역 지정
	virtual void SetRegion(const RECT& pRect)
	{
		m_rcRegion = pRect;
	}

	// offset만큼 이동해준다.(region, children, move rect 이동)
	virtual BOOL MoveOffset(int iOffsetX, int iOffsetY);
	virtual void SetSize(int iWidth, int iHeight); // 크기 지정

	virtual void SetState(eUI_STATE eState)
	{
		m_eState = eState;
	}

	// style지정
	virtual void SetStyle(uint32_t dwStyle)
	{
		m_dwStyle = dwStyle;
	}

	virtual void SetVisible(bool bVisible);
	virtual void SetVisibleWithNoSound(bool bVisible, bool bWork = false, bool bReFocus = false);

	virtual void CallBackProc(int iID, uint32_t dwFlag);
	virtual void ShowWindow(int iID = -1, CN3UIBase* pParent = nullptr);

	bool LoadSupportedVersions(File& file) override;
	bool Load(File& file) override;

	// 메시지를 받는다.. 보낸놈, msg
	virtual bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	virtual uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	virtual void Tick();
	virtual void Render();
	void Release() override; // 자식 포인터까지 delete 한다..
	virtual void Init(CN3UIBase* pParent);

	virtual bool OnKeyPress(int /*iKey*/)
	{
		return false;
	}

	virtual bool OnKeyPressed(int /*iKey*/)
	{
		return false;
	}

	virtual bool OnMouseWheelEvent(short /*delta*/)
	{
		return false;
	}

	static bool EnableTooltip(const std::string& szFN); // tooltip UI를 초기화 해준다.
	static void DestroyTooltip();                       // tooltip ui에 관련된 것을 해제해준다.

	int GetChildrenCount() const
	{
		return static_cast<int>(m_Children.size());
	}

	CN3UIBase* GetChildByIndex(int iIndex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_Children.size()))
			return nullptr;

		auto it = m_Children.begin();
		std::advance(it, iIndex);
		return *it;
	}

	CN3UIBase& operator=(const CN3UIBase& other);

protected:
#ifdef _N3TOOL
public:
	bool Save(File& file) override;
	virtual void ChangeImagePath(const std::string& szPathOld, const std::string& szPathNew);
	virtual void ChangeFont(const std::string& szFont);
	virtual void GatherImageFileName(std::set<std::string>& setImgFile);

	void ResizeAutomaticalyByChild();
	int IsMyChild(CN3UIBase* pUI);
	bool SwapChild(CN3UIBase* pChild1, CN3UIBase* pChild2);

	bool MoveToLower(CN3UIBase* pChild);
	bool MoveToUpper(CN3UIBase* pChild);
	bool MoveToLowest(CN3UIBase* pChild);
	bool MoveToHighest(CN3UIBase* pChild);

	void ArrangeZOrder();

	void SetSndOpen(const std::string& strFileName);
	void SetSndClose(const std::string& strFileName);
	std::string GetSndFName_OpenUI() const;
	std::string GetSndFName_CloseUI() const;

	virtual bool ReplaceAllTextures(const std::string& strFind, const std::string& strReplace);
#endif
};

#endif // !defined(AFX_N3UI_H__5F1578F8_8476_4647_8C53_E22A5F6184FE__INCLUDED_)
