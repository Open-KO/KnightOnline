// GameCursor.h: interface for the CGameCursor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMECURSOR_H__3BC4F8E1_6DEC_4848_A6EB_0352007AC780__INCLUDED_)
#define AFX_GAMECURSOR_H__3BC4F8E1_6DEC_4848_A6EB_0352007AC780__INCLUDED_

#pragma once

#include "GameDef.h"
#include <N3Base/N3UIBase.h>

class CGameCursor : public CN3UIBase
{
private:
	class CN3UIImage* m_pImageCursor[CURSOR_COUNT];
	bool m_bCursorLocked;
	e_Cursor m_ePrevGameCursor;
	e_Cursor m_eCurGameCursor;
	HCURSOR m_hCursor;

public:
	void Tick() override;
	void Render() override;
	void RestoreGameCursor();
	void SetGameCursor(e_Cursor eCursor, bool bLocked = false);
	bool Load(File& file) override;
	CGameCursor();
	~CGameCursor() override;
};

#endif // !defined(AFX_GAMECURSOR_H__3BC4F8E1_6DEC_4848_A6EB_0352007AC780__INCLUDED_)
