// GameCursor.cpp: implementation of the CGameCursor class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameProcedure.h"
#include "LocalInput.h"
#include "GameCursor.h"
#include "UIManager.h"
#include "ClientResourceFormatter.h"

#include <N3Base/N3UIImage.h>

CGameCursor::CGameCursor()
{
	m_bCursorLocked   = false;
	m_eCurGameCursor  = CURSOR_EL_NORMAL;
	m_ePrevGameCursor = CURSOR_EL_NORMAL;
	m_hCursor         = nullptr;

	for (int i = 0; i < CURSOR_COUNT; i++)
	{
		m_pImageCursor[i] = nullptr;
	}
}

CGameCursor::~CGameCursor()
{
	if (m_hCursor)
		::SetCursor(m_hCursor);
}

bool CGameCursor::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	m_hCursor = ::GetCursor();
	::SetCursor(nullptr);

	std::string szID;
	for (int i = 0; i < CURSOR_COUNT; i++)
	{
		szID = fmt::format("Image_Cursor{:02}", i);
		N3_VERIFY_UI_COMPONENT(m_pImageCursor[i], GetChildByID<CN3UIImage>(szID));
	}

	return true;
}

void CGameCursor::SetGameCursor(e_Cursor eCursor, bool bLocked)
{
	if ((m_bCursorLocked) && (!bLocked))
		return;
	else if (((m_bCursorLocked) && bLocked) || ((!m_bCursorLocked) && !bLocked))
	{
		m_eCurGameCursor = eCursor;
		return;
	}
	else if ((!m_bCursorLocked) && bLocked)
	{
		m_ePrevGameCursor = m_eCurGameCursor;
		m_bCursorLocked   = true;
		m_eCurGameCursor  = eCursor;
	}
}

void CGameCursor::RestoreGameCursor()
{
	if (m_bCursorLocked)
		m_bCursorLocked = false;

	m_eCurGameCursor = m_ePrevGameCursor;
}

void CGameCursor::Render()
{
	if (m_eCurGameCursor < 0 || m_eCurGameCursor >= CURSOR_COUNT)
		return;

	if (m_pImageCursor[m_eCurGameCursor] != nullptr)
	{
		CGameProcedure::s_pUIMgr->RenderStateSet();
		DWORD dwZ = 0;
		CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);

		m_pImageCursor[m_eCurGameCursor]->Render();
		CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
		CGameProcedure::s_pUIMgr->RenderStateRestore();
	}
}

void CGameCursor::Tick()
{
	HCURSOR hCursor = ::GetCursor();
	if (hCursor)
		::SetCursor(nullptr);

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	for (int i = 0; i < CURSOR_COUNT; i++)
	{
		if (m_pImageCursor[i])
			m_pImageCursor[i]->SetPos(ptCur.x, ptCur.y);
	}
}
