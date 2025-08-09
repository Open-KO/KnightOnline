// MachineMng.cpp: implementation of the CMachineMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MachineBase.h"
#include "Catapult.h"
#include "MachineMng.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMachineMng::CMachineMng()
{
	Release();
}

CMachineMng::~CMachineMng()
{
	Release();
}

void CMachineMng::Release()
{
	CMachineBase*	pMachine = NULL;
	it_Machine it = m_Machines.begin();
	int iSize = m_Machines.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		delete *it;
	}
	m_Machines.clear();
}
/*
void CMachineMng::ReceiveReplyMsg(ReplyMsg& RPMsg)
{
	if ( RPMsg.s_iAddressLen > s_pMsgRouter->GetThisClassLevel(ID_MACHINE_MGR) )
	{
		//..
	}
	else if ( RPMsg.s_iAddressLen == s_pMsgRouter->GetThisClassLevel(ID_MACHINE_MGR) )
	{
		CMachineBase* pMachine = NULL;
		switch( RPMsg.s_iGameID )
		{
		case ID_MACHINE_IN:
			//..		Add Player..
			AddMachine(RPMsg);
			break;

		case ID_MACHINE_STOP:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->Stop();
			break;		

		case ID_MACHINE_TURNLEFT_ON:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleTurnLeft(TRUE);
			break;
		case ID_MACHINE_TURNLEFT_OFF:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleTurnLeft(FALSE);
			break;

		case ID_MACHINE_TURNRIGHT_ON:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleTurnRight(TRUE);
			break;
		case ID_MACHINE_TURNRIGHT_OFF:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleTurnRight(FALSE);
			break;

		case ID_MACHINE_MOVEFORWARD_ON:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleMoveForward(TRUE);
			break;		
		case ID_MACHINE_MOVEFORWARD_OFF:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleMoveForward(FALSE);
			break;		
		case ID_MACHINE_MOVEBACKWARD_ON:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleMoveBackward(FALSE);
			break;		
		case ID_MACHINE_MOVEBACKWARD_OFF:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine) pMachine->ToggleMoveBackward(FALSE);
			break;		
		case ID_MACHINE_FIRE:
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine)
			{
				pMachine->SetFireRadian(RPMsg.s_fFireRadian);
				pMachine->SetFireSpeed(RPMsg.s_fFireSpeed);
				pMachine->Fire();

//				g_Facade.m_pSndMgr->Looping(SND3DOBJ_CATAPULTTHROW1 + g_Facade.m_pSndMgr->m_IdxCatapultThrow, false);
//				g_Facade.m_pSndMgr->SetPos(SND3DOBJ_CATAPULTTHROW1 + g_Facade.m_pSndMgr->m_IdxCatapultThrow, (D3DVECTOR)pMachine->Pos());
//				g_Facade.m_pSndMgr->Play(SND3DOBJ_CATAPULTTHROW1 + g_Facade.m_pSndMgr->m_IdxCatapultThrow);
//				g_Facade.m_pSndMgr->m_IdxCatapultThrow++;
//				if(g_Facade.m_pSndMgr->m_IdxCatapultThrow == g_Facade.m_pSndMgr->m_NumCatapultThrow) g_Facade.m_pSndMgr->m_IdxCatapultThrow = 0;
			}
			break;		
		case ID_MACHINE_POSITION:	// 투석기 위치 재설정
			pMachine = GetMachine(RPMsg.s_pStrID);
			if (pMachine)
			{
				pMachine->Stop();
				pMachine->PosSet(RPMsg.s_fPosX, RPMsg.s_fPosY, RPMsg.s_fPosZ);
				pMachine->SetDirRadian(RPMsg.s_fRotY);
			}
		}
	}
}

void CMachineMng::AddMachine(ReplyMsg& RPMsg)
{
//	if (lstrlen(RPMsg.s_cprscID) == 0) return;

	char szFileName[_MAX_PATH] = "misc\\machines\\catapult.mcn";
	FILE* stream = fopen(szFileName, "r"); //text파일로 만든다 
#if _DEBUG
	std::string szErr;
	if(NULL == stream)
	{
		szErr = fmt::format("failed to open file - {}", szFileName);
		__ASSERT(stream, szErr.c_str());
	}
#endif

	char szMachineType[64];		// machine 종류
	int result = fscanf(stream, "Machine_Type = %s\n", szMachineType);			__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");
	CMachineBase* pMachine = NULL;

	if (lstrcmpi(szMachineType, "catapult") == 0)
	{
		pMachine = new CCatapult;
	}
	else
	{
		pMachine = new CMachineBase;
	}

	pMachine->LoadMachine(stream);
	fclose(stream);


	pMachine->SetID(RPMsg.s_pStrID);
	pMachine->PosSet(RPMsg.s_fPosX, RPMsg.s_fPosY, RPMsg.s_fPosZ);
	pMachine->SetDirRadian(RPMsg.s_fRotY);

	m_Machines.push_back(pMachine);
}
*/
void CMachineMng::Render()
{
	CMachineBase*	pMachine = NULL;
	it_Machine it = m_Machines.begin();
	int iSize = m_Machines.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pMachine = *it;
		if (pMachine) pMachine->Render();
	}
}

void CMachineMng::Tick()
{
	CMachineBase*	pMachine = NULL;
	it_Machine it = m_Machines.begin();
	int iSize = m_Machines.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pMachine = *it;
		if(NULL == pMachine) continue;

		pMachine->Tick(-1);
/*		if(pMachine->GetMachineState()!=MS_STOP)
		{
			g_Facade.m_pSndMgr->Looping(SND3DOBJ_CATAPULTMOVE1 + g_Facade.m_pSndMgr->m_IdxCatapultMove, false);
			g_Facade.m_pSndMgr->SetPos(SND3DOBJ_CATAPULTMOVE1 + g_Facade.m_pSndMgr->m_IdxCatapultMove, (D3DVECTOR)pMachine->Pos());
			g_Facade.m_pSndMgr->Play(SND3DOBJ_CATAPULTMOVE1 + g_Facade.m_pSndMgr->m_IdxCatapultMove);
			g_Facade.m_pSndMgr->m_IdxCatapultMove++;
			if(g_Facade.m_pSndMgr->m_IdxCatapultMove == g_Facade.m_pSndMgr->m_NumCatapultMove) g_Facade.m_pSndMgr->m_IdxCatapultMove = 0;
		}*/
	}
}

CMachineBase* CMachineMng::GetMachine(const std::string& szID)
{
	if(szID.empty()) return NULL;

	CMachineBase*	pMachine = NULL;
	it_Machine it = m_Machines.begin();
	int iSize = m_Machines.size();
	for(int i = 0; i < iSize; i++, it++)
	{
		pMachine = *it;
		
		if (pMachine->m_szID == szID) return pMachine;
	}
	return NULL;
}

