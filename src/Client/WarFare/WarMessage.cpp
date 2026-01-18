// WarMessage.cpp: implementation of the CWarMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "WarMessage.h"
#include "GameBase.h"
#include "text_resources.h"

constexpr int MESSAGE_FONT_SIZE     = 20;
constexpr int WAR_MESSAGE_SHOW_TIME = 60;

CWarMessage::CWarMessage()
{
	m_pMessageFont = nullptr;
	m_fTime        = 0.0f;
	m_ptMessage    = {};
}

CWarMessage::~CWarMessage()
{
	Release();
}

void CWarMessage::InitFont()
{
	Release();

	std::string szFont = fmt::format_text_resource(IDS_FONT_ID);

	m_pMessageFont     = new CDFont(szFont, MESSAGE_FONT_SIZE);
	__ASSERT(m_pMessageFont, "Font Create Fail!!");

	if (m_pMessageFont)
	{
		m_pMessageFont->InitDeviceObjects(CN3Base::s_lpD3DDev);
		m_pMessageFont->RestoreDeviceObjects();

#ifdef _DEBUG
		m_ptMessage.y = 80;
#else
		m_ptMessage.y = 20;
#endif
		m_ptMessage.x = CN3Base::s_CameraData.vp.Width;
	}
}

void CWarMessage::Release()
{
	if (m_pMessageFont)
		delete m_pMessageFont;
	m_pMessageFont = nullptr;
}

void CWarMessage::SetMessage(const std::string& szText, uint32_t dwFlags, uint32_t dwColor)
{
	if (m_pMessageFont)
	{
		m_pMessageFont->SetText(szText, dwFlags); // 폰트에 텍스트 지정.
		m_pMessageFont->SetFontColor(dwColor);
		m_fTime = WAR_MESSAGE_SHOW_TIME;
	}
}

void CWarMessage::RenderMessage()
{
	if (m_fTime <= 0)
		return;

	if (m_pMessageFont)
	{
		D3DCOLOR crFont = m_pMessageFont->GetFontColor();

		m_pMessageFont->DrawText(m_ptMessage.x - 1.0f, m_ptMessage.y + 0.0f, 0xff000000, 0);
		m_pMessageFont->DrawText(m_ptMessage.x + 1.0f, m_ptMessage.y + 0.0f, 0xff000000, 0);
		m_pMessageFont->DrawText(m_ptMessage.x + 0.0f, m_ptMessage.y + 1.0f, 0xff000000, 0);
		m_pMessageFont->DrawText(m_ptMessage.x + 0.0f, m_ptMessage.y - 1.0f, 0xff000000, 0);

		m_pMessageFont->DrawText(m_ptMessage.x + 0.0f, m_ptMessage.y + 0.0f, crFont, 0);
	}
}

void CWarMessage::Tick()
{
	if (m_fTime > 0)
	{
		m_fTime                -= CN3Base::s_fSecPerFrm;

		const int iScreenWidth  = CN3Base::s_CameraData.vp.Width;

		m_ptMessage.x = iScreenWidth - ((int) (100 * (WAR_MESSAGE_SHOW_TIME - m_fTime)) % (iScreenWidth + m_pMessageFont->GetSize().cx));
	}
}
