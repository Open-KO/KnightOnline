// EventMgr.h: interface for the CEventMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTMGR_H__73272CD9_F059_4001_A116_871CEC8B25AE__INCLUDED_)
#define AFX_EVENTMGR_H__73272CD9_F059_4001_A116_871CEC8B25AE__INCLUDED_

#pragma once

#include <N3Base/N3Base.h>
#include <list>

class CMapMng;
class CEventCell;
class CDlgEditEvent;

class CEventMgr : public CN3Base
{
public:
	std::list<CEventCell*> m_pEvents;
	CEventCell* m_pCurrEvent;
	int m_MapSize;
	short** m_ppEvent;
	CMapMng* m_pRefMapMng;
	bool m_bActive; // 이기능이 활성화 되어 있는지...1:활성화, 0:비활성화..
	CDlgEditEvent* m_pDlgEventList;

public:
	bool MakeGameFile(File& file, int iSize);
	bool MakeGameFile(char* szEventName, int iSize);
	void LoadFromFile(const char* RealFileName);
	void SaveToFile(const char* RealFileName);
	void SaveInfoTextFile(char* szEvent);
	void MakeEventArray();
	void DelEvent(CEventCell* pEvent);
	void SetCurrEvent(CEventCell* pEvent);
	void UpdateEvent();
	BOOL MouseMsgFilter(LPMSG pMsg);
	void SetActive(bool active);
	void Render();

	virtual bool Load(File& file);
	virtual bool Save(File& file);

	CEventMgr();
	~CEventMgr() override;
};

#endif // !defined(AFX_EVENTMGR_H__73272CD9_F059_4001_A116_871CEC8B25AE__INCLUDED_)
