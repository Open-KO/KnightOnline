// UICharacterSelect.h: interface for the UICharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UICharacterSelect_H__665CADA6_E25B_47D6_B962_6DA49673048F__INCLUDED_)
#define AFX_UICharacterSelect_H__665CADA6_E25B_47D6_B962_6DA49673048F__INCLUDED_

#pragma once

#include <N3Base/N3UIBase.h>

struct __CharacterSelectInfo;
class CUICharacterSelect : public CN3UIBase
{
protected:
	CN3UIBase* m_pBtnLeft;
	CN3UIBase* m_pBtnRight;
	CN3UIBase* m_pBtnExit;
	CN3UIBase* m_pBtnDelete;
	CN3UIBase* m_pBtnBack;
	CN3UIString* m_pUserInfoStr;

public:
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool OnKeyPress(int iKey) override;
	CUICharacterSelect();
	~CUICharacterSelect() override;

	void Release() override;
	bool Load(File& file) override;
	void Tick() override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void DisplayChrInfo(__CharacterSelectInfo* pCSInfo);
	void DontDisplayInfo();
};

#endif // !defined(AFX_UICharacterSelect_H__665CADA6_E25B_47D6_B962_6DA49673048F__INCLUDED_)
