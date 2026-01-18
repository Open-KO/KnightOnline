// N3UIList.cpp: implementation of the CN3UIList class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIList.h"
#include "N3UIString.h"
#include "N3UIScrollBar.h"

CN3UIList::CN3UIList()
{
	m_eType         = UI_TYPE_LIST;

	m_iCurSel       = 0; // 현재 선택..
	m_pScrollBarRef = nullptr;

	m_szFontName    = "굴림체";
	m_dwFontHeight  = 10;
	m_bFontBold     = FALSE;
	m_bFontItalic   = FALSE;
	m_crFont        = 0xffffffff;
}

CN3UIList::~CN3UIList()
{
}

void CN3UIList::Release()
{
	CN3UIBase::Release();

	//	it_pString it = m_ListString.begin(), itEnd = m_ListString.end();
	//	for(; it != itEnd; it++)
	//	{
	//		delete (*it);
	//	}

	m_ListString.clear(); // 어차피 자식은 다지우니까... 리스트의 포인터를 Delete 할 필요 없다..
	m_iCurSel       = 0;
	m_pScrollBarRef = nullptr;

	m_szFontName    = "굴림체";
	m_dwFontHeight  = 10;
	m_bFontBold     = FALSE;
	m_bFontItalic   = FALSE;
	m_crFont        = 0xffffffff;
}

void CN3UIList::SetFont(const std::string& szFontName, uint32_t dwHeight, BOOL bBold, BOOL bItalic)
{
	m_szFontName   = szFontName;
	m_dwFontHeight = dwHeight;
	m_bFontBold    = bBold;
	m_bFontItalic  = bItalic;

	for (CN3UIString* pUIString : m_ListString)
		pUIString->SetFont(m_szFontName, m_dwFontHeight, m_bFontBold, m_bFontItalic);

	UpdateChildRegions();
}

void CN3UIList::SetFontColor(int iIndex, D3DCOLOR color)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_ListString.size()))
		return;

	auto it = m_ListString.begin();
	std::advance(it, iIndex);
	(*it)->SetColor(color);
}

void CN3UIList::SetFontColor(D3DCOLOR color)
{
	m_crFont = color;

	for (CN3UIString* pUIString : m_ListString)
		pUIString->SetColor(m_crFont);
}

int CN3UIList::AddStrings(const std::string* pszStrings, int iStringCount)
{
	for (int i = 0; i < iStringCount; i++)
	{
		CN3UIString* pString = new CN3UIString();
		pString->Init(this);
		pString->SetFont(m_szFontName, m_dwFontHeight, m_bFontBold, m_bFontItalic);
		pString->SetColor(m_crFont);
		pString->SetString(pszStrings[i]);

		m_ListString.push_back(pString);
	}

	UpdateChildRegions();

	return static_cast<int>(m_ListString.size()) - 1;
}

int CN3UIList::AddString(const std::string& szString)
{
	CN3UIString* pString = new CN3UIString();
	pString->Init(this);
	pString->SetFont(m_szFontName, m_dwFontHeight, m_bFontBold, m_bFontItalic);
	pString->SetColor(m_crFont);
	pString->SetString(szString);

	m_ListString.push_back(pString);
	UpdateChildRegions();

	return static_cast<int>(m_ListString.size()) - 1;
}

bool CN3UIList::InsertString(int iIndex, const std::string& szString)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_ListString.size()))
		return false;

	CN3UIString* pString = new CN3UIString();
	pString->Init(this);
	pString->SetFont(m_szFontName, m_dwFontHeight, m_bFontBold, m_bFontItalic);
	pString->SetColor(m_crFont);
	pString->SetString(szString);

	auto it = m_ListString.begin();
	std::advance(it, iIndex);
	m_ListString.insert(it, pString);

	UpdateChildRegions();
	return true;
}

bool CN3UIList::DeleteString(int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_ListString.size()))
		return false;

	auto it = m_ListString.begin();
	std::advance(it, iIndex);
	delete (*it);
	m_ListString.erase(it);

	int iSC = static_cast<int>(m_ListString.size());

	if (m_pScrollBarRef != nullptr)
	{
		int iScrollPos = m_pScrollBarRef->GetCurrentPos();
		if (iScrollPos >= iSC)
			m_pScrollBarRef->SetCurrentPos(iSC - 1);
	}

	if (m_iCurSel >= iSC)
		m_iCurSel = iSC - 1;

	UpdateChildRegions();
	return true;
}

bool CN3UIList::GetString(int iIndex, std::string& szString)
{
	szString.clear();

	if (iIndex < 0 || iIndex >= static_cast<int>(m_ListString.size()))
		return false;

	auto it = m_ListString.begin();
	std::advance(it, iIndex);

	CN3UIString* pUIString = (*it);
	szString               = pUIString->GetString();

	return true;
}

bool CN3UIList::SetString(int iIndex, const std::string& szString)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_ListString.size()))
		return false;

	auto it = m_ListString.begin();
	std::advance(it, iIndex);

	CN3UIString* pUIString = (*it);
	pUIString->SetString(szString);

	return false;
}

CN3UIString* CN3UIList::GetChildStrFromList(const std::string& str)
{
	for (CN3UIString* pUIString : m_ListString)
	{
		if (pUIString->GetString() == str)
			return pUIString;
	}

	return nullptr;
}

void CN3UIList::UpdateChildRegions()
{
	RECT rc     = GetRegion();
	RECT rcThis = rc;
	POINT pt {};
	SIZE size {};
	int iScrollPos = 0;
	if (m_pScrollBarRef != nullptr)
	{
		iScrollPos = m_pScrollBarRef->GetCurrentPos();
		RECT rcTmp = m_pScrollBarRef->GetRegion();
		rc.right   = rcTmp.left;
	}

	it_pString it = m_ListString.begin(), itEnd = m_ListString.end();
	for (int i = 0; it != itEnd; it++, i++)
	{
		CN3UIString* pStr = *it;
		if (i < iScrollPos)
		{
			pStr->SetVisibleWithNoSound(false);
			continue;
		}

		pt                     = pStr->GetPos();

		const std::string& str = pStr->GetString();

		pStr->GetTextExtent(str, static_cast<int>(str.size()), &size);
		pStr->SetPos(rc.left, rc.top);
		RECT rcTmp = { rc.left, rc.top, rc.left + (rc.right - rc.left), rc.top + size.cy };
		pStr->SetRegion(rcTmp);
		rc.top += size.cy;

		if (rc.top >= rcThis.bottom)
			pStr->SetVisibleWithNoSound(false);
		else
			pStr->SetVisibleWithNoSound(true);
	}

	if (m_pScrollBarRef)
	{
		if (rc.bottom <= rcThis.bottom)
		{
			m_pScrollBarRef->SetCurrentPos(0);
			m_pScrollBarRef->SetVisibleWithNoSound(false);
		}
		else
		{
			m_pScrollBarRef->SetVisibleWithNoSound(true);
		}

		m_pScrollBarRef->SetRange(0, static_cast<int>(m_ListString.size()));
	}
}

int CN3UIList::GetScrollPos() const
{
	if (m_pScrollBarRef == nullptr)
		return 0;

	return m_pScrollBarRef->GetCurrentPos();
}

bool CN3UIList::SetScrollPos(int iScrollPos)
{
	if (m_pScrollBarRef == nullptr)
		return false;

	m_pScrollBarRef->SetCurrentPos(iScrollPos);
	UpdateChildRegions();
	return true;
}

bool CN3UIList::Load(File& file)
{
	bool bSuccess = CN3UIBase::Load(file);

	// font 정보
	int iStrLen   = 0;
	file.Read(&iStrLen, sizeof(iStrLen)); // font 이름 길이
	__ASSERT(iStrLen > 0, "No font name");
	if (iStrLen > 0)
	{
		m_szFontName.assign(iStrLen, ' ');
		file.Read(&m_szFontName[0], iStrLen); // string
		file.Read(&m_dwFontHeight, 4);        // font height
		file.Read(&m_crFont, 4);              // font color
		file.Read(&m_bFontBold, 4);           // font flag (bold, italic)
		file.Read(&m_bFontItalic, 4);         // font flag (bold, italic)
	}

	// Child 중에 Scroll Bar 가 있는지 찾아본다.
	for (CN3UIBase* pUI : m_Children)
	{
		if (pUI->UIType() == UI_TYPE_SCROLLBAR)
			m_pScrollBarRef = (CN3UIScrollBar*) pUI;
	}

	return bSuccess;
}

#ifdef _N3TOOL
bool CN3UIList::Save(File& file)
{
	if (!CN3UIBase::Save(file))
		return false;

	// font 정보
	int iStrLen = static_cast<int>(m_szFontName.size());
	__ASSERT(iStrLen > 0, "No font name");
	file.Write(&iStrLen, sizeof(iStrLen));         // font 이름 길이
	if (iStrLen > 0)
	{
		file.Write(m_szFontName.c_str(), iStrLen); // string
		file.Write(&m_dwFontHeight, 4);            // font height
		file.Write(&m_crFont, 4);                  // font color
		file.Write(&m_bFontBold, 4);               // font flag (bold, italic)
		file.Write(&m_bFontItalic, 4);             // font flag (bold, italic)
	}

	return true;
}
#endif // #ifdef _N3TOOL

#ifdef _N3TOOL
CN3UIList& CN3UIList::operator=(const CN3UIList& other)
{
	if (this == &other)
		return *this;

	CN3UIBase::operator=(other);

	m_szFontName   = other.m_szFontName;
	m_dwFontHeight = other.m_dwFontHeight;
	m_bFontBold    = other.m_bFontBold;
	m_bFontItalic  = other.m_bFontItalic;
	m_crFont       = other.m_crFont;

	return *this;
}
#endif // #ifdef _N3TOOL

uint32_t CN3UIList::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible || UI_STATE_LIST_DISABLE == m_eState)
		return dwRet;

	// 특정 이벤트에 대해 메시지 전송..
	if (IsIn(ptCur.x, ptCur.y) && ((dwFlags & UI_MOUSE_LBCLICK) || (dwFlags & UI_MOUSE_LBDBLCLK)))
	{
		RECT rc = GetRegion(), rcStr;
		SIZE size {};

		it_pString it = m_ListString.begin(), itEnd = m_ListString.end();
		for (int i = 0; it != itEnd; it++, i++)
		{
			CN3UIString* pStr = (*it);
			if (!pStr->IsVisible())
				continue;

			pStr->GetTextExtent("1", 1, &size);
			rcStr      = pStr->GetRegion();

			rc.top    += size.cy;
			rc.bottom += size.cy;

			if (::PtInRect(&rcStr, ptCur))
			{
				m_iCurSel = i;
				if (dwFlags & UI_MOUSE_LBCLICK)
				{
					if (m_pParent)
						m_pParent->ReceiveMessage(
							this, UIMSG_LIST_SELCHANGE); // 부모에게 버튼 클릭 통지..
					dwRet |= UIMSG_LIST_SELCHANGE;
				}
				else
				{
					if (m_pParent)
						m_pParent->ReceiveMessage(
							this, UIMSG_LIST_DBLCLK); // 부모에게 버튼 클릭 통지..
					dwRet |= UIMSG_LIST_DBLCLK;
				}
				dwRet |= UI_MOUSEPROC_DONESOMETHING;
				return dwRet;
			}
		}
	}

	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

void CN3UIList::Render()
{
	CN3UIBase::Render();

	if (m_iCurSel >= 0 && m_iCurSel < static_cast<int>(m_ListString.size()))
	{
		auto it = m_ListString.begin();
		std::advance(it, m_iCurSel);
		CN3UIString* pStr = *it;
		if (pStr != nullptr)
		{
			RECT rc = pStr->GetRegion(); // 선택 표시

			__VertexTransformedColor vLines[5] {};
			vLines[0].Set(
				(float) rc.left, (float) rc.top, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
			vLines[1].Set(
				(float) rc.right, (float) rc.top, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
			vLines[2].Set(
				(float) rc.right, (float) rc.bottom, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
			vLines[3].Set(
				(float) rc.left, (float) rc.bottom, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
			vLines[4] = vLines[0];

			DWORD dwZ = 0, dwFog = 0, dwAlpha = 0, dwCOP = 0, dwCA1 = 0, dwSrcBlend = 0,
				  dwDestBlend = 0, dwVertexShader = 0, dwAOP = 0, dwAA1 = 0;
			CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
			CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
			CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
			CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
			CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
			CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwCOP);
			CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwCA1);
			CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAOP);
			CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAA1);
			CN3Base::s_lpD3DDev->GetFVF(&dwVertexShader);

			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

			CN3Base::s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);
			CN3Base::s_lpD3DDev->DrawPrimitiveUP(
				D3DPT_LINESTRIP, 4, vLines, sizeof(__VertexTransformedColor));

			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
			CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwCOP);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwCA1);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAOP);
			CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAA1);
			CN3Base::s_lpD3DDev->SetFVF(dwVertexShader);
		}
	}
}

bool CN3UIList::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (UIMSG_SCROLLBAR_POS == dwMsg)
	{
		if (pSender == m_pScrollBarRef)
		{
			this->SetScrollPos(m_pScrollBarRef->GetCurrentPos());
			//			return m_pParent->ReceiveMessage(this, UIMSG_SCROLLBAR_POS);
		}
	}

	return true;
}

void CN3UIList::ResetContent()
{
	it_pString it = m_ListString.begin(), itEnd = m_ListString.end();
	for (; it != itEnd; it++)
	{
		CN3UIString* pString = *it;
		delete pString;
	}
	m_ListString.clear();
	m_iCurSel = 0;
	if (m_pScrollBarRef)
	{
		m_pScrollBarRef->SetRange(0, 0);
		m_pScrollBarRef->SetCurrentPos(0);
		m_pScrollBarRef->SetVisibleWithNoSound(false);
	}
}
