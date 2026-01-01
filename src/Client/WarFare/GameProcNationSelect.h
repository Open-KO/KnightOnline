// GameProcNationSelect.h: interface for the CGameProcNationSelect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEPROCNATIONSELECT_H__83F05883_F45C_4119_BD8F_EBF2C2CF0A29__INCLUDED_)
#define AFX_GAMEPROCNATIONSELECT_H__83F05883_F45C_4119_BD8F_EBF2C2CF0A29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameProcedure.h"

class CGameProcNationSelect : public CGameProcedure   
{
public:
	class CUINationSelectDlg* m_pUINationSelectDlg;	

protected:
	virtual bool ProcessPacket(Packet& pkt);

public:
	void MsgSendNationSelect(e_Nation eNation);
	
	void Release();
	void Init();
	void Tick();
	void Render();

	CGameProcNationSelect();
	virtual ~CGameProcNationSelect();
};

#endif // !defined(AFX_GAMEPROCNATIONSELECT_H__83F05883_F45C_4119_BD8F_EBF2C2CF0A29__INCLUDED_)
