// N3UIBase.cpp: implementation of the CN3UIBase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIBase.h"

#include <vector>
#include "N3UIButton.h"
#include "N3UIProgress.h"
#include "N3UIImage.h"
#include "N3UIScrollBar.h"
#include "N3UIString.h"
#include "N3UITrackBar.h"
#include "N3UIStatic.h"
#include "N3UIEdit.h"
#include "N3UITooltip.h"
#include "N3UIArea.h"
#include "N3UIList.h"
#ifdef _REPENT
#include "N3UIIconSlot.h"
#endif

#include "N3SndMgr.h"
#include "N3SndObj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#ifdef _N3GAME
bool CN3UIBase::s_bWaitFromServer = false;
#endif

CN3UIEdit* CN3UIBase::s_pFocusedEdit = nullptr;
CN3UITooltip* CN3UIBase::s_pTooltipCtrl = nullptr;
std::string CN3UIBase::s_szStringTmp; // 임시변수..

CN3UIBase::CN3UIBase()
{
	m_eType = UI_TYPE_BASE;
	m_pParent = nullptr;
	m_pChildUI	= nullptr;
	m_pParentUI = nullptr;

	m_iChildID	= -1;

	memset(&m_rcRegion, 0, sizeof(m_rcRegion));
	memset(&m_rcMovable, 0, sizeof(m_rcMovable));
	m_eState = UI_STATE_COMMON_NONE;
	m_dwStyle = UISTYLE_NONE;

	m_dwReserved = 0;
	m_bVisible = true;
	m_pSnd_OpenUI = nullptr;
	m_pSnd_CloseUI = nullptr;

	m_crToolTip = DefaultTooltipColor;
}

CN3UIBase::~CN3UIBase()
{
	if(m_pParent) m_pParent->RemoveChild(this);	// 부모의 자식에서 나를 지우기

	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_OpenUI);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_CloseUI);

	while(!m_Children.empty())
	{
		CN3UIBase* pChild = m_Children.front();
		if (pChild) delete pChild;	// 자식이 delete되면서 부모의 list에서는 자동으로 제거된다.
									// 따라서 리스트에서 따로 지우는 부분이 없어도 된다.
	}
}

void CN3UIBase::Release()
{
	if (m_pParent != nullptr)
		m_pParent->RemoveChild(this);

	memset(&m_rcRegion, 0, sizeof(m_rcRegion));
	memset(&m_rcMovable, 0, sizeof(m_rcMovable));
	
	m_szID.clear();
	m_szToolTip.clear();
	m_crToolTip = DefaultTooltipColor;

	m_eState = UI_STATE_COMMON_NONE;
	m_dwStyle = UISTYLE_NONE;
	m_dwReserved = 0;
	m_bVisible = true;
	s_SndMgr.ReleaseObj(&m_pSnd_OpenUI);
	s_SndMgr.ReleaseObj(&m_pSnd_CloseUI);

	while (!m_Children.empty())
	{
		// 자식이 delete되면서 부모의 list에서는 자동으로 제거된다.
		// 따라서 리스트에서 따로 지우는 부분이 없어도 된다.
		delete m_Children.front();
	}

	CN3BaseFileAccess::Release();
}

void CN3UIBase::Init(CN3UIBase* pParent)
{
	Release();
	SetParent(pParent);
}

void CN3UIBase::RemoveChild(CN3UIBase* pChild)
{
	if(nullptr == pChild) return;

	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor;)
	{
		if((*itor) == pChild)
		{
			itor = m_Children.erase(itor);
			break;
		}
		else
		{
			++itor;
		}
	}
}

void CN3UIBase::SetParent(CN3UIBase* pParent)
{
	if(m_pParent) m_pParent->RemoveChild(this);
	m_pParent = pParent;
	if (m_pParent) m_pParent->AddChild(this);

	if(pParent) m_iFileFormatVersion = pParent->m_iFileFormatVersion;
}

POINT CN3UIBase::GetPos() const
{
	POINT p;
	p.x = m_rcRegion.left;
	p.y = m_rcRegion.top;
	return p;
}

// Reposition
void CN3UIBase::SetPos(int x, int y)
{
	// Find the delta
	int dx, dy;
	dx = x - m_rcRegion.left;
	dy = y - m_rcRegion.top;

	MoveOffset(dx, dy);
}

void CN3UIBase::SetPosCenter()
{
	// Target location of our UI Element
	const int elementCenterX = static_cast<int>((s_CameraData.vp.Width - GetWidth()) / 2);
	const int elementCenterY = static_cast<int>((s_CameraData.vp.Height - GetHeight()) / 2);
	
	// Delta is the diff between our target and our current position
	POINT currentPos = GetPos();
	const int deltaX = elementCenterX - currentPos.x;
	const int deltaY = elementCenterY - currentPos.y;

	// Shift the UI element; We call MoveOffset as it will also shift all child elements
	MoveOffset(deltaX, deltaY);
}

// MoveOffset shifts the UI element and all of its children by the given offsets
BOOL CN3UIBase::MoveOffset(int iOffsetX, int iOffsetY)
{
	if (iOffsetX == 0
		&& iOffsetY == 0)
		return FALSE;
	
	// Shift UI Element Bounding Box
	m_rcRegion.left += iOffsetX;
	m_rcRegion.top += iOffsetY;
	m_rcRegion.right += iOffsetX;
	m_rcRegion.bottom += iOffsetY;

	// movable 영역
	if(	m_rcMovable.right - m_rcMovable.left != 0 &&
		m_rcMovable.bottom - m_rcMovable.top != 0 )
	{
		m_rcMovable.left += iOffsetX;
		m_rcMovable.top += iOffsetY;
		m_rcMovable.right += iOffsetX;
		m_rcMovable.bottom += iOffsetY;
	}

	// Shift child elements
	for (CN3UIBase* pCUI : m_Children)
	{
		__ASSERT(pCUI, "child UI pointer is NULL!");
		pCUI->MoveOffset(iOffsetX, iOffsetY);
	}
	return TRUE;
}

//	점 (x,y)가 영역안에 있으면 true..
bool CN3UIBase::IsIn(int x, int y)
{
	if(x<m_rcRegion.left || x>m_rcRegion.right || y<m_rcRegion.top || y>m_rcRegion.bottom) return false;
	return true;
}

void CN3UIBase::SetSize(int iWidth, int iHeight)
{
	RECT rc;
	SetRect(&rc, m_rcRegion.left, m_rcRegion.top, m_rcRegion.left + iWidth, m_rcRegion.top + iHeight);
	SetRegion(rc);
}

bool CN3UIBase::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) // 메시지를 받는다.. 보낸놈, msg
{
	if(m_pParent && pSender) m_pParent->ReceiveMessage(pSender, dwMsg); // 부모가 있으면 그넘에게도 보낸다..
	return true;
}

void CN3UIBase::CallBackProc(int iID, uint32_t dwFlag)
{
}

void CN3UIBase::ShowWindow(int iID, CN3UIBase* pParent)
{
	if(pParent)
		pParent->m_pChildUI = this;

	m_pParentUI = pParent;
	m_iChildID = iID;

	SetVisible(true);
}

bool CN3UIBase::Load(File& file)
{
	CN3BaseFileAccess::Load(file);

	// children 정보
	int iCC = 0;
	if (m_iFileFormatVersion >= N3FORMAT_VER_1264)
	{
		int16_t sCC, sIdk0;
		file.Read(&sCC, sizeof(int16_t)); // children count
		file.Read(&sIdk0, sizeof(int16_t));
		iCC = (int) sCC;
	}
	else
	{
		file.Read(&iCC, sizeof(iCC)); // children count
	}

	eUI_TYPE eChildUIType;
	for (int i = 0; i < iCC; i++)
	{
		CN3UIBase* pChild = nullptr;
		file.Read(&eChildUIType, sizeof(eChildUIType)); // child의 ui type

		switch (eChildUIType)
		{
		case UI_TYPE_BASE:			pChild = new CN3UIBase();			break;
		case UI_TYPE_IMAGE:			pChild = new CN3UIImage();			break;
		case UI_TYPE_STRING:		pChild = new CN3UIString();			break;
		case UI_TYPE_BUTTON:		pChild = new CN3UIButton();			break;
		case UI_TYPE_STATIC:		pChild = new CN3UIStatic();			break;
		case UI_TYPE_PROGRESS:		pChild = new CN3UIProgress();		break;
		case UI_TYPE_SCROLLBAR:		pChild = new CN3UIScrollBar();		break;
		case UI_TYPE_TRACKBAR:		pChild = new CN3UITrackBar();		break;
		case UI_TYPE_EDIT:			pChild = new CN3UIEdit();			break;
		case UI_TYPE_AREA:			pChild = new CN3UIArea();			break;
#ifdef _REPENT
		case UI_TYPE_ICONSLOT:		pChild = new CN3UIIconSlot();		break;
#endif
		case UI_TYPE_LIST:			pChild = new CN3UIList();			break;
		}
		__ASSERT(pChild, "Unknown type UserInterface!!!");
		pChild->Init(this);
		pChild->Load(file);
	}

	// base 정보
	int iIDLen = 0;
	file.Read(&iIDLen, sizeof(iIDLen));				// ui id length
	if (iIDLen > 0)
	{
		m_szID.assign(iIDLen, '\0');
		file.Read(&m_szID[0], iIDLen);				// ui id
	}
	else
	{
		m_szID.clear();
	}

	file.Read(&m_rcRegion, sizeof(m_rcRegion));		// m_rcRegion
	file.Read(&m_rcMovable, sizeof(m_rcMovable));	// m_rcMovable
	file.Read(&m_dwStyle, sizeof(m_dwStyle));		// style
	file.Read(&m_dwReserved, sizeof(m_dwReserved));	// m_dwReserved

	int iTooltipLen;
	file.Read(&iTooltipLen, sizeof(iTooltipLen));	//	tooltip문자열 길이
	if (iTooltipLen > 0)
	{
		m_szToolTip.assign(iTooltipLen, '\0');
		file.Read(&m_szToolTip[0], iTooltipLen);
	}

	std::string szSoundFN;
	// 이전 uif파일을 컨버팅 하려면 사운드 로드 하는 부분 막기
	int iSndFNLen = 0;
	file.Read(&iSndFNLen, sizeof(iSndFNLen));		//	사운드 파일 문자열 길이
	if (iSndFNLen > 0)
	{
		szSoundFN.assign(iSndFNLen, '\0');
		file.Read(&szSoundFN[0], iSndFNLen);

		__ASSERT(nullptr == m_pSnd_OpenUI, "memory leak");
		m_pSnd_OpenUI = s_SndMgr.CreateObj(szSoundFN, SNDTYPE_2D);
	}

	file.Read(&iSndFNLen, sizeof(iSndFNLen));		//	사운드 파일 문자열 길이
	if (iSndFNLen > 0)
	{
		szSoundFN.assign(iSndFNLen, '\0');
		file.Read(&szSoundFN[0], iSndFNLen);

		__ASSERT(nullptr == m_pSnd_CloseUI, "memory leak");
		m_pSnd_CloseUI = s_SndMgr.CreateObj(szSoundFN, SNDTYPE_2D);
	}

	return true;
}


void CN3UIBase::Tick()
{
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		pChild->Tick();
	}
}

void CN3UIBase::Render()
{
	if (!m_bVisible) return;	// 보이지 않으면 자식들을 render하지 않는다.

	for(UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		pChild->Render();

		//this_ui
		CN3UIBase* pCUI = nullptr;
		pCUI = pChild->m_pChildUI;
		while(pCUI)
		{
			pCUI->Render();
			pCUI = pCUI->m_pChildUI;
		}
	}
}

uint32_t CN3UIBase::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld )
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE == m_eState)
	{
		if (dwFlags&UI_MOUSE_LBCLICKED)
		{
			SetState(UI_STATE_COMMON_NONE);
		}
		else
		{
			MoveOffset(ptCur.x - ptOld.x, ptCur.y - ptOld.y);
		}
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	if(false == IsIn(ptCur.x, ptCur.y))	// 영역 밖이면
	{
		if(false == IsIn(ptOld.x, ptOld.y))
		{
			return dwRet;// 이전 좌표도 영역 밖이면 
		}
		dwRet |= UI_MOUSEPROC_PREVINREGION;	// 이전 좌표는 영역 안이었다.
	}
	else
	{
		// tool tip 관련
		if (s_pTooltipCtrl != nullptr)
			s_pTooltipCtrl->SetText(m_szToolTip, m_crToolTip);
	}

	dwRet |= UI_MOUSEPROC_INREGION;	// 이번 좌표는 영역 안이다.


	//this_ui
	if(m_pChildUI && m_pChildUI->IsVisible())
		return dwRet;

	// child에게 메세지 전달
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		uint32_t dwChildRet = pChild->MouseProc(dwFlags, ptCur, ptOld);
		if (UI_MOUSEPROC_DONESOMETHING & dwChildRet)
		{	// 이경우에는 먼가 포커스를 받은 경우이다.
			// (아래 코드는 dialog를 관리하는 곳에서 해야 한다. 따라서 막아놓음)
//			m_Children.erase(itor);			// 우선 리스트에서 지우고
//			m_Children.push_front(pChild);	// 맨앞에 넣는다. 그리는 순서를 맨 나중에 그리도록 하려고

			dwRet |= (UI_MOUSEPROC_CHILDDONESOMETHING|UI_MOUSEPROC_DONESOMETHING);
			return dwRet;
		}
	}

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE != m_eState && 
			PtInRect(&m_rcMovable, ptCur) && (dwFlags&UI_MOUSE_LBCLICK) )
	{
		SetState(UI_STATE_COMMON_MOVE);
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	return dwRet;
}

bool CN3UIBase::EnableTooltip(const std::string& szFN)
{
	delete s_pTooltipCtrl;
	s_pTooltipCtrl = nullptr;
	if (szFN.empty()) return false;
	
	s_pTooltipCtrl = new CN3UITooltip();
	s_pTooltipCtrl->Init(nullptr);
	s_pTooltipCtrl->LoadFromFile(szFN);
	return true;
}

void CN3UIBase::DestroyTooltip()
{
	if (s_pTooltipCtrl) {delete s_pTooltipCtrl; s_pTooltipCtrl = nullptr;}
}

void CN3UIBase::PrintChildIDs()
{
	for (CN3UIBase* pChild : m_Children)
	{
		if (!pChild->m_szID.empty())
			printf("%s\n", pChild->m_szID.c_str());
	}
}

CN3UIBase* CN3UIBase::GetChildByID(const std::string_view szID) const
{
	if (szID.empty())
		return nullptr;

	for (CN3UIBase* pChild : m_Children)
	{
		const std::string& childID = pChild->GetID();
		if (szID.length() == childID.length()
			&& _strnicmp(szID.data(), childID.data(), szID.length()) == 0)
			return pChild;
	}

	return nullptr;
}

CN3UIBase* CN3UIBase::GetChildByID(const std::string_view szID, eUI_TYPE eUIType) const
{
	if (szID.empty())
		return nullptr;

	for (CN3UIBase* pChild : m_Children)
	{
		if (eUIType != pChild->UIType())
			continue;

		const std::string& childID = pChild->GetID();
		if (szID.length() == childID.length()
			&& _strnicmp(szID.data(), childID.data(), szID.length()) == 0)
			return pChild;
	}

	return nullptr;
}

template <eUI_TYPE... UITypes>
static CN3UIBase* GetChildByIDImpl(const CN3UIBase* parent, const std::string_view szID)
{
	if (szID.empty())
		return nullptr;

	for (CN3UIBase* pChild : parent->GetChildren())
	{
		// Use a fold expression here to include all of the supported types.
		if (((pChild->UIType() != UITypes) && ...))
			continue;

		const std::string& childID = pChild->GetID();
		if (szID.length() == childID.length()
			&& _strnicmp(szID.data(), childID.data(), szID.length()) == 0)
			return pChild;
	}

	return nullptr;
}

#define IMPL_GETCHILDBYID(Class, MainUIType, ...)							\
template <>																	\
Class* CN3UIBase::GetChildByID<Class>(const std::string_view szID) const	\
{																			\
    return static_cast<Class*>(												\
		GetChildByIDImpl<MainUIType, ##__VA_ARGS__>(this, szID));			\
}

// Preferred behaviour for this specialization would be to just use the base class, and not require UI_TYPE_BASE.
// This is achievable with the method it already resolves to.
// If we truly require verifying it is in fact UI_TYPE_BASE (which is pointless), the caller can pass it to the
// base call themselves.
// IMPL_GETCHILDBYID(CN3UIBase,		UI_TYPE_BASE);

IMPL_GETCHILDBYID(CN3UIArea,		UI_TYPE_AREA);
IMPL_GETCHILDBYID(CN3UIButton,		UI_TYPE_BUTTON);
IMPL_GETCHILDBYID(CN3UIEdit,		UI_TYPE_EDIT);
IMPL_GETCHILDBYID(CN3UIImage,		UI_TYPE_IMAGE);
IMPL_GETCHILDBYID(CN3UIList,		UI_TYPE_LIST);
IMPL_GETCHILDBYID(CN3UIProgress,	UI_TYPE_PROGRESS);
IMPL_GETCHILDBYID(CN3UIScrollBar,	UI_TYPE_SCROLLBAR);
IMPL_GETCHILDBYID(CN3UIStatic,		UI_TYPE_STATIC, UI_TYPE_EDIT);	// CN3UIEdit inherits from CN3UIStatic
IMPL_GETCHILDBYID(CN3UIString,		UI_TYPE_STRING);
IMPL_GETCHILDBYID(CN3UITooltip,		UI_TYPE_TOOLTIP);
IMPL_GETCHILDBYID(CN3UITrackBar,	UI_TYPE_TRACKBAR);

#undef IMPL_GETCHILDBYID

void CN3UIBase::SetVisible(bool bVisible)
{
	// No change
	if (bVisible == m_bVisible)
		return;

	// update state
	m_bVisible = bVisible;

	// UI element made visible
	if (bVisible)
	{
		// play open sound, if defined
		if (m_pSnd_OpenUI != nullptr)
			m_pSnd_OpenUI->Play();
	}
	// UI element hidden
	else
	{
		// play close sound, if defined
		if (m_pSnd_CloseUI != nullptr)
			m_pSnd_CloseUI->Play();

		// hide child UI and delink pointer
		if (m_pChildUI != nullptr)
			m_pChildUI->SetVisible(false);

		m_pChildUI = nullptr;

		// delink pointer to parent
		if (m_pParentUI != nullptr
			&& m_pParentUI->m_pChildUI == this)
			m_pParentUI->m_pChildUI = nullptr;

		m_pParentUI = nullptr;
		m_iChildID	= -1;
	}
}

void CN3UIBase::SetVisibleWithNoSound(bool bVisible, bool bWork, bool bReFocus)
{ 
	m_bVisible = bVisible; 
	if(!m_bVisible)
	{
		if(m_pChildUI)
		{
			m_pChildUI->SetVisible(false);
		}
		m_pChildUI	= nullptr;
		if(m_pParentUI)
		{
			if(m_pParentUI->m_pChildUI == this)
				m_pParentUI->m_pChildUI = nullptr;
		}
		m_pParentUI = nullptr;
		m_iChildID	= -1;
	}
}

#ifndef _N3TOOL
void CN3UIBase::operator = (const CN3UIBase& other)
{
	Init(nullptr);	// 일단 부모는 없게 초기화

	UIListItorConst it = other.m_Children.begin();
	UIListItorConst itEnd = other.m_Children.end();
	CN3UIBase* pOtherChild = nullptr;
	CN3UIBase* pChild = nullptr;
	for(; it != itEnd; it++)
	{
		pOtherChild = *it;

		if(nullptr == pOtherChild) continue;

		pChild = nullptr;
		switch(pOtherChild->UIType())
		{
		case UI_TYPE_BASE:
			{ 
				pChild = new CN3UIBase();
				*pChild = *pOtherChild;
			}
			break;
		case UI_TYPE_BUTTON:
			{
				CN3UIButton *pUINew = new CN3UIButton();
				*pUINew = *((CN3UIButton*)pOtherChild);
				pChild = pUINew;
			}
			break;	// button
		case UI_TYPE_STATIC:	
			{ 
				CN3UIStatic* pUINew = new CN3UIStatic();		
				*pUINew = *((CN3UIStatic*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// static (배경그림과 글자가 나오는 클래스)
		case UI_TYPE_PROGRESS:	
			{ 
				CN3UIProgress* pUINew = new CN3UIProgress();	
				*pUINew = *((CN3UIProgress*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// progress
		case UI_TYPE_IMAGE:		
			{ 
				CN3UIImage* pUINew = new CN3UIImage();		
				*pUINew = *((CN3UIImage*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// image
		case UI_TYPE_SCROLLBAR:	
			{ 
				CN3UIScrollBar* pUINew = new CN3UIScrollBar();	
				*pUINew = *((CN3UIScrollBar*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// scroll bar
		case UI_TYPE_STRING:	
			{ 
				CN3UIString* pUINew = new CN3UIString();		
				*pUINew = *((CN3UIString*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// string
		case UI_TYPE_TRACKBAR:	
			{ 
				CN3UITrackBar* pUINew = new CN3UITrackBar();	
				*pUINew = *((CN3UITrackBar*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// track bar
		case UI_TYPE_EDIT:		
			{ 
				CN3UIEdit* pUINew = new CN3UIEdit();		
				*pUINew = *((CN3UIEdit*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// edit
		case UI_TYPE_AREA:		
			{ 
				CN3UIArea* pUINew = new CN3UIArea();		
				*pUINew = *((CN3UIArea*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// area
		case UI_TYPE_TOOLTIP:
			{ 
				CN3UITooltip* pUINew = new CN3UITooltip();
				*pUINew = *((CN3UITooltip*)pOtherChild);
				pChild = pUINew;
			} 
			break;	// tooltip
		case UI_TYPE_LIST:
			{ 
				CN3UIList* pUINew = new CN3UIList();
				*pUINew = *((CN3UIList*)pOtherChild);
				pChild = pUINew;
			} 
			break;	// tooltip
//		case UI_TYPE_ICON:		pUIDest = new CN3UIIcon();		*pUIDest = *((CN3UIBase*)pUISrc); break;	// icon
//		case UI_TYPE_ICON_MANAGER:	pUIDest = new CN3UIIconManager();	*pUIDest = *((CN3UIBase*)pUISrc); break;	// icon manager.. 
#ifdef _REPENT
		case UI_TYPE_ICONSLOT:
			{
				CN3UIIconSlot* pUINew = new CN3UIIconSlot();
				*pUINew = *((CN3UIIconSlot*)pOtherChild);
				pChild = pUINew;
			}
			break;	// icon slot
#endif
		}
		if(pChild) pChild->SetParent(this);	// 부모 지정
	}

	m_bVisible = other.m_bVisible;
	m_dwReserved = other.m_dwReserved;
	m_dwStyle = other.m_dwStyle;
	m_eState = other.m_eState;
	m_eType = other.m_eType;

	if (other.m_pSnd_OpenUI != nullptr)
	{
		s_SndMgr.ReleaseObj(&m_pSnd_OpenUI);
		m_pSnd_OpenUI = s_SndMgr.CreateObj(other.m_pSnd_OpenUI->FileName(), SNDTYPE_2D);
	}

	if (other.m_pSnd_CloseUI != nullptr)
	{
		s_SndMgr.ReleaseObj(&m_pSnd_CloseUI);
		m_pSnd_CloseUI = s_SndMgr.CreateObj(other.m_pSnd_CloseUI->FileName(), SNDTYPE_2D);
	}

	m_rcMovable = other.m_rcMovable;
	m_rcRegion = other.m_rcRegion;
	m_szID = other.m_szID;
	m_szToolTip = other.m_szToolTip;
}
#endif

#ifdef _N3TOOL
bool CN3UIBase::Save(File& file)
{
	CN3BaseFileAccess::Save(file);

	// child 정보
	int iCC = static_cast<int>(m_Children.size());

	if (m_iFileFormatVersion >= N3FORMAT_VER_1264)
	{	
		int16_t sCC = static_cast<int16_t>(iCC);
		int16_t sIdk0 = 1; // unknown

		file.Write(&sCC, sizeof(int16_t)); // children count
		file.Write(&sIdk0, sizeof(int16_t)); //unknown
	}
	else
	{
		file.Write(&iCC, sizeof(iCC));
	}

	//file.Write(&iCC, sizeof(iCC)); // Child 갯수 ㅆ고..고..

	for(UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	// childadd할때 push_front이므로 저장할 때 거꾸로 저장해야 한다.
	{
		CN3UIBase* pChild = (*itor);
		eUI_TYPE eUIType = pChild->UIType();

		file.Write(&eUIType, sizeof(eUIType)); // UI Type 쓰고..
		pChild->Save(file);
	}

	// base 정보
	int iIDLen = static_cast<int>(m_szID.size());
	file.Write(&iIDLen, sizeof(iIDLen));					// id length
	if (iIDLen > 0)
		file.Write(m_szID.c_str(), iIDLen);				// ui id
	file.Write(&m_rcRegion, sizeof(m_rcRegion));			// m_rcRegion
	file.Write(&m_rcMovable, sizeof(m_rcMovable));		// m_rcMovable
	file.Write(&m_dwStyle, sizeof(m_dwStyle));			// style
	file.Write(&m_dwReserved, sizeof(m_dwReserved));		//	m_dwReserved

	int iTooltipLen = static_cast<int>(m_szToolTip.size());
	file.Write(&iTooltipLen, sizeof(iTooltipLen));		//	tooltip문자열 길이
	if (iTooltipLen > 0)
		file.Write(m_szToolTip.c_str(), iTooltipLen);

	int iSndFNLen = 0;
	if (m_pSnd_OpenUI != nullptr)
		iSndFNLen = static_cast<int>(m_pSnd_OpenUI->FileName().size());
	file.Write(&iSndFNLen, sizeof(iSndFNLen));			//	사운드 파일 문자열 길이
	if (iSndFNLen>0) file.Write(m_pSnd_OpenUI->FileName().c_str(), iSndFNLen);

	iSndFNLen = 0;
	if (m_pSnd_CloseUI != nullptr)
		iSndFNLen = static_cast<int>(m_pSnd_CloseUI->FileName().size());
	file.Write(&iSndFNLen, sizeof(iSndFNLen));			//	사운드 파일 문자열 길이
	if (iSndFNLen > 0)
		file.Write(m_pSnd_CloseUI->FileName().c_str(), iSndFNLen);
	
	return true;
}

void CN3UIBase::ChangeImagePath(const std::string& szPathOld, const std::string& szPathNew)
{
	// child 정보
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		pChild->ChangeImagePath(szPathOld, szPathNew);
	}
}

void CN3UIBase::ChangeFont(const std::string& szFont)
{
	// child 정보
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		pChild->ChangeFont(szFont);
	}
}

void CN3UIBase::GatherImageFileName(std::set<std::string>& setImgFile)
{
	// child 정보
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		pChild->GatherImageFileName(setImgFile);
	}
}


void CN3UIBase::ResizeAutomaticalyByChild()
{
	if(m_Children.empty()) return;

	RECT rcMax = { 100000000, 100000000, -100000000, -100000000 };
	int iIndex = 0;
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; itor++, iIndex++)
	{
		CN3UIBase* pChild = (*itor);
		RECT rcTmp = pChild->GetRegion();
		if(rcTmp.left < rcMax.left) rcMax.left = rcTmp.left;
		if(rcTmp.top < rcMax.top) rcMax.top = rcTmp.top;
		if(rcTmp.right > rcMax.right) rcMax.right = rcTmp.right;
		if(rcTmp.bottom > rcMax.bottom) rcMax.bottom = rcTmp.bottom;
	}

	RECT rcCur = this->GetRegion();
	if(rcCur.left > rcMax.left) rcCur.left = rcMax.left;
	if(rcCur.top > rcMax.top) rcCur.top = rcMax.top;
	if(rcCur.right < rcMax.right) rcCur.right = rcMax.right;
	if(rcCur.bottom < rcMax.bottom) rcCur.bottom = rcMax.bottom;
//	this->SetRegion(rcCur);
	m_rcRegion = rcCur;	// SetRegion을 해버리면 child의 영역을 바꿔버리는 경우가 있으므로 내 영역만 바꾸기위해 직접 넣는다.
}

int CN3UIBase::IsMyChild(CN3UIBase* pUI)
{
	int iIndex = 0;
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; itor++, iIndex++)
	{
		CN3UIBase* pChild = (*itor);
		if(pChild == pUI) return iIndex;
	}

	return -1;
}

bool CN3UIBase::SwapChild(CN3UIBase* pChild1, CN3UIBase* pChild2)
{
	if(this->IsMyChild(pChild1) < 0 || IsMyChild(pChild2) < 0) return false;
	
	UIListItor itor1;
	for(itor1 = m_Children.begin(); m_Children.end() != itor1; itor1++)
		if(*itor1 == pChild1) break;
	if(itor1 == m_Children.end()) return false;

	UIListItor itor2;
	for(itor2 = m_Children.begin(); m_Children.end() != itor2; itor2++)
		if(*itor2 == pChild2) break;
	if(itor2 == m_Children.end()) return false;

	std::swap(*itor1, *itor2);

	return true;
}

bool CN3UIBase::MoveToHighest(CN3UIBase* pChild)
{
	if(this->IsMyChild(pChild) < 0) return false;
	
	for(UIListItor itor1 = m_Children.begin(); m_Children.end() != itor1; itor1++)
	{
		if(*itor1 == pChild) 
		{
			m_Children.erase(itor1);
			m_Children.push_front(pChild);
			return true;
		}
	}

	return false;
}

bool CN3UIBase::MoveToLowest(CN3UIBase* pChild)
{
	if(this->IsMyChild(pChild) < 0) return false;
	
	for(UIListItor itor1 = m_Children.begin(); m_Children.end() != itor1; itor1++)
	{
		if(*itor1 == pChild) 
		{
			m_Children.erase(itor1);
			m_Children.push_back(pChild);
			return true;
		}
	}

	return false;
}

bool CN3UIBase::MoveToLower(CN3UIBase* pChild)
{
	if(this->IsMyChild(pChild) < 0) return false;
	
	for(UIListItor itor1 = m_Children.begin(); itor1 != m_Children.end(); itor1++)
	{
		if(*itor1 == pChild)
		{
			UIListItor itNext = itor1; itNext++;
			if(itNext != m_Children.end())
			{
				std::swap(*itNext, *itor1);
				return true;
			}
			break;
		}
	}

	return false;
}

bool CN3UIBase::MoveToUpper(CN3UIBase* pChild)
{
	if(this->IsMyChild(pChild) < 0) return false;
	
	for(UIListItor itor1 = m_Children.begin(); itor1 != m_Children.end(); itor1++)
	{
		if(*itor1 == pChild) 
		{
			if(itor1 != m_Children.begin())
			{
				UIListItor itPrev = itor1; itPrev--;
				std::swap(*itPrev, *itor1);
				return true;
			}
			break;
		}
	}

	return false;
}

void CN3UIBase::ArrangeZOrder()
{
	// 보통 image가 배경그림이 되므로 child list에서 맨 뒤로 보낸다.
	// 왜냐하면 맨 뒤에 있는것이 맨 먼저 그려지므로
	UIList tempList;
	UIListItor itor;
	for(itor = m_Children.begin(); m_Children.end() != itor;)
	{
		CN3UIBase* pChild = (*itor);
		if(UI_TYPE_IMAGE == pChild->UIType())
		{
			itor = m_Children.erase(itor);	// 현재 위치에서 지우고
			tempList.push_back(pChild);		// 임시 버퍼에 저장
		}
		else ++itor;
	}

	for(itor = tempList.begin(); tempList.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		m_Children.push_back(pChild);		// child list맨 뒤에 넣기
	}
	tempList.clear();
}

void CN3UIBase::operator = (const CN3UIBase& other)
{
	Init(nullptr);	// 일단 부모는 없게 초기화

	UIListItorConst it = other.m_Children.begin();
	UIListItorConst itEnd = other.m_Children.end();
	CN3UIBase* pOtherChild = nullptr;
	CN3UIBase* pChild = nullptr;
	for(; it != itEnd; it++)
	{
		pOtherChild = *it;

		if(nullptr == pOtherChild) continue;

		pChild = nullptr;
		switch(pOtherChild->UIType())
		{
		case UI_TYPE_BASE:
			{ 
				pChild = new CN3UIBase();
				*pChild = *pOtherChild;
			}
			break;
		case UI_TYPE_BUTTON:
			{
				CN3UIButton *pUINew = new CN3UIButton();
				*pUINew = *((CN3UIButton*)pOtherChild);
				pChild = pUINew;
			}
			break;	// button
		case UI_TYPE_STATIC:	
			{ 
				CN3UIStatic* pUINew = new CN3UIStatic();		
				*pUINew = *((CN3UIStatic*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// static (배경그림과 글자가 나오는 클래스)
		case UI_TYPE_PROGRESS:	
			{ 
				CN3UIProgress* pUINew = new CN3UIProgress();	
				*pUINew = *((CN3UIProgress*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// progress
		case UI_TYPE_IMAGE:		
			{ 
				CN3UIImage* pUINew = new CN3UIImage();		
				*pUINew = *((CN3UIImage*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// image
		case UI_TYPE_SCROLLBAR:	
			{ 
				CN3UIScrollBar* pUINew = new CN3UIScrollBar();	
				*pUINew = *((CN3UIScrollBar*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// scroll bar
		case UI_TYPE_STRING:	
			{ 
				CN3UIString* pUINew = new CN3UIString();		
				*pUINew = *((CN3UIString*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// string
		case UI_TYPE_TRACKBAR:	
			{ 
				CN3UITrackBar* pUINew = new CN3UITrackBar();	
				*pUINew = *((CN3UITrackBar*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// track bar
		case UI_TYPE_EDIT:		
			{ 
				CN3UIEdit* pUINew = new CN3UIEdit();		
				*pUINew = *((CN3UIEdit*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// edit
		case UI_TYPE_AREA:		
			{ 
				CN3UIArea* pUINew = new CN3UIArea();		
				*pUINew = *((CN3UIArea*)pOtherChild); 
				pChild = pUINew;
			} 
			break;	// area
		case UI_TYPE_TOOLTIP:
			{ 
				CN3UITooltip* pUINew = new CN3UITooltip();
				*pUINew = *((CN3UITooltip*)pOtherChild);
				pChild = pUINew;
			} 
			break;	// tooltip
		case UI_TYPE_LIST:
			{ 
				CN3UIList* pUINew = new CN3UIList();
				*pUINew = *((CN3UIList*)pOtherChild);
				pChild = pUINew;
			} 
			break;	// tooltip
//		case UI_TYPE_ICON:		pUIDest = new CN3UIIcon();		*pUIDest = *((CN3UIBase*)pUISrc); break;	// icon
//		case UI_TYPE_ICON_MANAGER:	pUIDest = new CN3UIIconManager();	*pUIDest = *((CN3UIBase*)pUISrc); break;	// icon manager.. 
#ifdef _REPENT
		case UI_TYPE_ICONSLOT:
			{
				CN3UIIconSlot* pUINew = new CN3UIIconSlot();
				*pUINew = *((CN3UIIconSlot*)pOtherChild);
				pChild = pUINew;
			}
			break;	// icon slot
#endif
		}
		if(pChild) pChild->SetParent(this);	// 부모 지정
	}

	m_bVisible = other.m_bVisible;
	m_dwReserved = other.m_dwReserved;
	m_dwStyle = other.m_dwStyle;
	m_eState = other.m_eState;
	m_eType = other.m_eType;

	SetSndOpen(other.GetSndFName_OpenUI());
	SetSndClose(other.GetSndFName_CloseUI());

	m_rcMovable = other.m_rcMovable;
	m_rcRegion = other.m_rcRegion;
	m_szID = other.m_szID;
	m_szToolTip = other.m_szToolTip;
}

void CN3UIBase::SetSndOpen(const std::string& strFileName)
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_OpenUI);
	if (0 == strFileName.size()) return;

	CN3BaseFileAccess tmpBase;
	tmpBase.FileNameSet(strFileName);	// Base경로에 대해서 상대적 경로를 넘겨준다.

	SetCurrentDirectory(tmpBase.PathGet().c_str());
	m_pSnd_OpenUI = s_SndMgr.CreateObj(tmpBase.FileName(), SNDTYPE_2D);
}

void CN3UIBase::SetSndClose(const std::string& strFileName)
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_CloseUI);
	if (0 == strFileName.size()) return;

	CN3BaseFileAccess tmpBase;
	tmpBase.FileNameSet(strFileName);	// Base경로에 대해서 상대적 경로를 넘겨준다.

	SetCurrentDirectory(tmpBase.PathGet().c_str());
	m_pSnd_CloseUI = s_SndMgr.CreateObj(tmpBase.FileName(), SNDTYPE_2D);
}

std::string CN3UIBase::GetSndFName_OpenUI() const
{
	if (m_pSnd_OpenUI == nullptr)
		return {};
	
	return m_pSnd_OpenUI->FileName();
}

std::string CN3UIBase::GetSndFName_CloseUI() const
{
	if (m_pSnd_CloseUI == nullptr)
		return {};
	
	return m_pSnd_CloseUI->FileName();
}

bool CN3UIBase::ReplaceAllTextures(const std::string& strFind, const std::string& strReplace)
{
	if (strFind.empty() || strReplace.empty())
		return false;

	for (CN3UIBase* pChild : m_Children)
	{
		if (!pChild->ReplaceAllTextures(strFind, strReplace))
			return false;
	}

	return true;
}

#endif	// _N3TOOL
