#include "StdAfx.h"
#include "N3FXMgr.h"
#include "GameBase.h"
#include "GameProcmain.h"
#include "GameProcedure.h"
#include "PlayerOtherMgr.h"
#include "PlayerNPC.h"
#include "PlayerMySelf.h"
#include "N3FXBundleGame.h"
#include "N3WorldManager.h"
#include "MagicSkillMng.h"
#include "APISocket.h"
#include "PacketDef.h"

#include <N3Base/N3ShapeExtra.h>

CN3FXMgr::CN3FXMgr()
{
	m_fOriginLimitedTime = 60.0f;
}

CN3FXMgr::~CN3FXMgr()
{
	stlLIST_BUNDLEGAME_IT it;
	for(it=m_ListBundle.begin(); it!=m_ListBundle.end(); it++)
	{
		delete (*it);
	}
	m_ListBundle.clear();

	stlMAP_BUNDLEORIGIN_IT itOrigin;
	for(itOrigin=m_OriginBundle.begin(); itOrigin!=m_OriginBundle.end(); itOrigin++)
	{
		LPFXBUNDLEORIGIN pSrc = itOrigin->second;
		delete pSrc->pBundle;
		delete pSrc;
	}
	m_OriginBundle.clear();	
}


//
//
//
void CN3FXMgr::TriggerBundle(int SourceID, int SourceJoint, int FXID, int TargetID, int Joint, int idx, int MoveType)
{
	__TABLE_FX* pFX = s_pTbl_FXSource.Find(FXID);
	if(!pFX) return;

	std::string strTmp = pFX->szFN;
	_strlwr(&strTmp[0]);
	
	stlMAP_BUNDLEORIGIN_IT itOrigin = m_OriginBundle.find(strTmp);
	
	if(itOrigin != m_OriginBundle.end())	//같은 효과가 있다..
	{
		LPFXBUNDLEORIGIN pSrc = itOrigin->second;
		
		CN3FXBundleGame* pBundle = new CN3FXBundleGame;
		
		pBundle->SetPreBundlePos(SourceID, Joint);
		pSrc->pBundle->Duplicate(pBundle);
		pBundle->m_iID = FXID;
		pBundle->m_iIdx = idx;
		pBundle->m_iMoveType = MoveType;
		pBundle->m_iSourceJoint = SourceJoint;

		pBundle->Trigger(SourceID, TargetID, Joint, pFX->dwSoundID);
		
		m_ListBundle.push_back(pBundle);
		pSrc->iNum++;
	}
	else	//같은 효과가 없다..
	{
		LPFXBUNDLEORIGIN pSrc = new FXBUNDLEORIGIN;
		pSrc->pBundle = new CN3FXBundleGame;
		pSrc->pBundle->LoadFromFile(strTmp);

		CN3FXBundleGame* pBundle = new CN3FXBundleGame;

		pBundle->SetPreBundlePos(SourceID, Joint);
		pSrc->pBundle->Duplicate(pBundle);	
		pBundle->m_iID = FXID;
		pBundle->m_iIdx = idx;
		pBundle->m_iMoveType = MoveType;
		pBundle->m_iSourceJoint = SourceJoint;

		pBundle->Trigger(SourceID, TargetID, Joint, pFX->dwSoundID);
		
		m_ListBundle.push_back(pBundle);
		
		pSrc->iNum++;
		m_OriginBundle.insert(stlMAP_BUNDLEORIGIN_VALUE(strTmp, pSrc));
	}
}


//
//
//
void CN3FXMgr::TriggerBundle(int SourceID, int SourceJoint, int FXID, __Vector3 TargetPos, int idx, int MoveType)
{
	__TABLE_FX* pFX = s_pTbl_FXSource.Find(FXID);
	if(!pFX) return; 

	std::string strTmp = pFX->szFN;
	_strlwr(&strTmp[0]);

	stlMAP_BUNDLEORIGIN_IT itOrigin = m_OriginBundle.find(strTmp);

	if(itOrigin != m_OriginBundle.end())	//같은 효과가 있다..
	{
		LPFXBUNDLEORIGIN pSrc = itOrigin->second;		
		CN3FXBundleGame* pBundle = new CN3FXBundleGame;
				
		pBundle->SetPreBundlePos(SourceID, SourceJoint);
		pSrc->pBundle->Duplicate(pBundle);
		pBundle->m_iID = FXID;
		pBundle->m_iIdx = idx;
		pBundle->m_iMoveType = MoveType;
		pBundle->m_iSourceJoint = SourceJoint;

		pBundle->Trigger(SourceID, TargetPos, pFX->dwSoundID);		
		m_ListBundle.push_back(pBundle);
		pSrc->iNum++;
	}
	else	//같은 효과가 없다..
	{
		LPFXBUNDLEORIGIN pSrc = new FXBUNDLEORIGIN;
		pSrc->pBundle = new CN3FXBundleGame;
		pSrc->pBundle->LoadFromFile(pFX->szFN);

		CN3FXBundleGame* pBundle = new CN3FXBundleGame;

		pBundle->SetPreBundlePos(SourceID, SourceJoint);
		pSrc->pBundle->Duplicate(pBundle);	
		pBundle->m_iID = FXID;
		pBundle->m_iIdx = idx;
		pBundle->m_iMoveType = MoveType;
		pBundle->m_iSourceJoint = SourceJoint;

		pBundle->Trigger(SourceID, TargetPos, pFX->dwSoundID);
		m_ListBundle.push_back(pBundle);

		pSrc->iNum++;
		m_OriginBundle.insert(stlMAP_BUNDLEORIGIN_VALUE(strTmp, pSrc));
	}
}


//
//
//
void CN3FXMgr::Stop(int SourceID, int TargetID, int FXID, int idx, bool immediately)
{
	CN3FXBundleGame* pBundle;
	if(FXID < 0)
	{
		stlLIST_BUNDLEGAME_IT it = m_ListBundle.begin();	
		while(it!=m_ListBundle.end())
		{
			pBundle = (*it);
			if(!pBundle)
			{
				it = m_ListBundle.erase(it);
				continue;
			}
			if(pBundle->m_iSourceID==SourceID && pBundle->m_iIdx==idx)
			{
				pBundle->Stop(immediately);
			}
			it++;
		}
	}
	else
	{
		stlLIST_BUNDLEGAME_IT it = m_ListBundle.begin();	
		while(it!=m_ListBundle.end())
		{
			pBundle = (*it);
			if(!pBundle)
			{
				it = m_ListBundle.erase(it);
				continue;
			}
			if(pBundle->m_iSourceID==SourceID && pBundle->m_iID == FXID && pBundle->m_iIdx==idx)
			{
				pBundle->Stop(immediately);
			}
			it++;
		}
	}
}

//
//
//
void CN3FXMgr::SetBundlePos(int FXID, int idx, __Vector3& vPos)
{
	stlLIST_BUNDLEGAME_IT it = m_ListBundle.begin();
	CN3FXBundleGame* pBundle;
	while(it!=m_ListBundle.end())
	{
		pBundle = (*it);
		if(pBundle && pBundle->m_iID == FXID && pBundle->m_iIdx==idx)
		{
			pBundle->m_vDestPos = vPos;
			return;
		}
		it++;
	}
}

//
//
//
void CN3FXMgr::StopMine()
{
	CN3FXBundleGame* pBundle;

	stlLIST_BUNDLEGAME_IT it = m_ListBundle.begin();	
	while(it!=m_ListBundle.end())
	{
		pBundle = (*it);
		if(!pBundle)
		{
			it = m_ListBundle.erase(it);
			continue;
		}
		if(pBundle->m_iSourceID==CGameBase::s_pPlayer->IDNumber())
		{
			pBundle->Stop(true);
		}
		it++;
	}
}


//
//
//
void CN3FXMgr::Tick()
{
	stlMAP_BUNDLEORIGIN_IT itOrigin = m_OriginBundle.begin();
	while(itOrigin!=m_OriginBundle.end())
	{
		LPFXBUNDLEORIGIN pSrc = itOrigin->second;
		if(pSrc && pSrc->iNum<=0)
		{
			pSrc->fLimitedTime += CN3Base::s_fSecPerFrm;
			if(pSrc->fLimitedTime > m_fOriginLimitedTime)
			{
				if(pSrc->pBundle) delete pSrc->pBundle;
				delete pSrc;
				
				itOrigin = m_OriginBundle.erase(itOrigin);
				continue;
			}
		}
		itOrigin++;
	}
//	TRACE("Origin Bundle Count : %d \n", m_OriginBundle.size());
//	TRACE("현재쓰고 있는 효과의 수 : %d \n", m_ListBundle.size());

	stlLIST_BUNDLEGAME_IT it = m_ListBundle.begin();
	//std::list<CN3FXBundleGame*>::iterator it = m_ListBundle.begin();
	while(it!=m_ListBundle.end())
	{
		CN3FXBundleGame* pBundle = (*it);
		if(!pBundle)
		{
			it = m_ListBundle.erase(it);
			continue;
		}
		if(pBundle->m_dwState==FX_BUNDLE_STATE_DEAD)
		{
			stlMAP_BUNDLEORIGIN_IT itOrigin = m_OriginBundle.find(pBundle->FileName());
			if(itOrigin!=m_OriginBundle.end())	//같은 효과가 있다..
			{
				LPFXBUNDLEORIGIN pSrc = itOrigin->second;
				pSrc->iNum--;
			}
//			else
//			{
//				TRACE("Invalid Bundle-.- \n");
//			}
			
			delete pBundle;
			it = m_ListBundle.erase(it);
			continue;
		}		
		
		//내가 쏜 것이고..
		//pBundle->m_iMoveType과 살아있는지를 체크한 다음
		//시야권 검사는 보류....만약 한다면...view frustum으로 하는게 아니라...
		//player와 obj의 거리를 구해서 일정거리 이상이면 없애는 걸로해라..
		//충돌검사
/*	
		if(pBundle->m_iMoveType != FX_BUNDLE_MOVE_NONE &&
			pBundle->m_dwState==FX_BUNDLE_STATE_LIVE &&
			( s_pPlayer->IDNumber()==pBundle->m_iSourceID ||
			( s_pPlayer->IDNumber()==pBundle->m_iTargetID &&
			s_pOPMgr->UPCGetByID(pBundle->m_iSourceID, true)==nullptr)))
*/
		if(pBundle->m_iMoveType != FX_BUNDLE_MOVE_NONE && pBundle->m_dwState==FX_BUNDLE_STATE_LIVE)
		{
			if(s_pOPMgr->UPCGetByID(pBundle->m_iSourceID, true)==nullptr &&
				s_pOPMgr->NPCGetByID(pBundle->m_iSourceID, true)==nullptr &&
				s_pPlayer->IDNumber()!=pBundle->m_iSourceID	)
			{
				pBundle->Stop();
			}

			uint32_t dwToMe = 0; //dwToMe==1이면 내가 쏜거.. dwToMe==2이면 내가 타겟..
			if(s_pPlayer->IDNumber()==pBundle->m_iSourceID) dwToMe = 1;
			else if( s_pPlayer->IDNumber()==pBundle->m_iTargetID &&
					s_pOPMgr->UPCGetByID(pBundle->m_iSourceID, true)==nullptr) dwToMe = 2;
			
			if(dwToMe == 1 || dwToMe == 2)
			{
				__Vector3 vCol;
				float fHeight = 0.5f;

				// npc or player와 충돌체크..
				bool bCol = false;
				it_UPC it = s_pOPMgr->m_UPCs.begin();
				it_UPC itEnd = s_pOPMgr->m_UPCs.end();
				CPlayerOther* pUPC = nullptr;

				if( dwToMe==2 && ((pBundle->m_vPos - s_pPlayer->Position()).Magnitude()<16.0f) )
				{
					__Vector3 vCp = s_pPlayer->Center();
					if( true == s_pPlayer->CheckCollisionByBox(pBundle->m_vPos, pBundle->m_vPos + pBundle->m_vDir*pBundle->m_fVelocity*CN3Base::s_fSecPerFrm, &vCol, nullptr) )
					//if( true == CheckCollisionSphere(pBundle->m_vPos, pBundle->m_vPos + pBundle->m_vDir*pBundle->m_fVelocity*CN3Base::s_fSecPerFrm, vCp, s_pPlayer->Radius(), &vCol) )
					{
						bCol = true;
						pBundle->m_vPos = vCol;
						pBundle->Stop();
						int iMagicID = CGameProcedure::s_pProcMain->m_pMagicSkillMng->GetMagicID(pBundle->m_iIdx);
						
						uint8_t byBuff[32];
						int iOffset=0;
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
						CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_pPlayer->IDNumber());

						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						
						CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);	//?
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						
						CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..	

						iOffset=0;
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FAIL);
						CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_pPlayer->IDNumber());

						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.x);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.y);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.z);

						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SKILLMAGIC_FAIL_KILLFLYING);			
						CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);					
						
						CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

						break;
					}
				}

				for(; it != itEnd; it++)
				{
					pUPC = it->second;
					if (dwToMe == 1
						&& !s_pPlayer->IsHostileTarget(pUPC))
						continue;
					
					if((pBundle->m_vPos - pUPC->Position()).Magnitude() > 16.0f) continue; // 16 미터 이상 떨어져 있음 지나간다..

					__Vector3 vCp = pUPC->Center();
					//if( true == CheckCollisionSphere(pBundle->m_vPos, pBundle->m_vPos + pBundle->m_vDir*pBundle->m_fVelocity*CN3Base::s_fSecPerFrm, vCp, pUPC->Radius(), &vCol) )
					if ( true == pUPC->CheckCollisionByBox(pBundle->m_vPos, pBundle->m_vPos + pBundle->m_vDir*pBundle->m_fVelocity*CN3Base::s_fSecPerFrm, &vCol, nullptr) )
					{
						bCol = true;
						pBundle->m_vPos = vCol;
						pBundle->Stop();
						int iMagicID = CGameProcedure::s_pProcMain->m_pMagicSkillMng->GetMagicID(pBundle->m_iIdx);
						
						uint8_t byBuff[32];
						int iOffset=0;
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
						CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pUPC->IDNumber());

						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						
						CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);	//?
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);
						
						CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..	

						iOffset=0;
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
						CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FAIL);
						CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pUPC->IDNumber());

						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.x);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.y);
						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.z);

						CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SKILLMAGIC_FAIL_KILLFLYING);			
						CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);
						CAPISocket::MP_AddShort(byBuff, iOffset, 0);					
						
						CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

						break;
					}
				}

				if(false == bCol)
				{
					__Vector3 vNext = pBundle->m_vPos + pBundle->m_vDir * (pBundle->m_fVelocity * CN3Base::s_fSecPerFrm * 1.2f);

					it_NPC it2 = s_pOPMgr->m_NPCs.begin();
					it_NPC itEnd2 = s_pOPMgr->m_NPCs.end();
					CPlayerNPC* pSNPC = s_pOPMgr->NPCGetByID(pBundle->m_iSourceID, FALSE);
					CPlayerNPC* pNPC = nullptr;
					for(; it2 != itEnd2; it2++)
					{
						pNPC = (*it2).second;

						if (dwToMe == 1
							&& !s_pPlayer->IsHostileTarget(pNPC))
							continue;

						if (pSNPC != nullptr
							&& dwToMe == 2
							&& !pSNPC->IsHostileTarget(pNPC))
							continue;

						if((pBundle->m_vPos - pNPC->Position()).Magnitude() > 16.0f) continue; // 16 미터 이상 떨어져 있음 지나간다..

						if(pNPC->IDNumber()==pBundle->m_iTargetID && pNPC->m_pShapeExtraRef)
						{
							__Vector3 vMin = pNPC->m_pShapeExtraRef->Min();
							__Vector3 vMax = pNPC->m_pShapeExtraRef->Max();
							__Vector3 vDestPos = vMin + ((vMax-vMin)*0.5f);

							float fDistTmp = pBundle->m_fVelocity * CN3Base::s_fSecPerFrm * 1.2f;
							
							if((pBundle->m_vPos - vDestPos).Magnitude() <= fDistTmp)
							{
								bCol = true;
								pBundle->Stop();
								int iMagicID = CGameProcedure::s_pProcMain->m_pMagicSkillMng->GetMagicID(pBundle->m_iIdx);
								
								uint8_t byBuff[32];
								int iOffset=0;
								CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
								CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
								CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pNPC->IDNumber());

								CAPISocket::MP_AddShort(byBuff, iOffset, 0);
								CAPISocket::MP_AddShort(byBuff, iOffset, 0);
								CAPISocket::MP_AddShort(byBuff, iOffset, 0);
								
								CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);	//?
								CAPISocket::MP_AddShort(byBuff, iOffset, 0);
								CAPISocket::MP_AddShort(byBuff, iOffset, 0);
								
								CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
								
								iOffset=0;
								CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
								CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FAIL);
								CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pNPC->IDNumber());

								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_vPos.x);
								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_vPos.y);
								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_vPos.z);

								CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SKILLMAGIC_FAIL_KILLFLYING);			
								CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);
								CAPISocket::MP_AddShort(byBuff, iOffset, 0);					
								
								CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

								break;
							}
						}

						__Vector3 vCp = pNPC->Center();
						//if( true == CheckCollisionSphere(pBundle->m_vPos, vNext, vCp, pNPC->Radius(), &vCol) )
						if ( true == pNPC->CheckCollisionByBox(pBundle->m_vPos, vNext, &vCol, nullptr) )
						{
							bCol = true;
							pBundle->m_vPos = vCol;
							pBundle->Stop();
							int iMagicID = CGameProcedure::s_pProcMain->m_pMagicSkillMng->GetMagicID(pBundle->m_iIdx);
							
							uint8_t byBuff[32];
							int iOffset=0;
							CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
							CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
							CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pNPC->IDNumber());

							CAPISocket::MP_AddShort(byBuff, iOffset, 0);
							CAPISocket::MP_AddShort(byBuff, iOffset, 0);
							CAPISocket::MP_AddShort(byBuff, iOffset, 0);
							
							CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);	//?
							CAPISocket::MP_AddShort(byBuff, iOffset, 0);
							CAPISocket::MP_AddShort(byBuff, iOffset, 0);
							
							CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
							
							iOffset=0;
							CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
							CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FAIL);
							CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pNPC->IDNumber());

							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.x);
							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.y);
							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.z);

							CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SKILLMAGIC_FAIL_KILLFLYING);			
							CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);
							CAPISocket::MP_AddShort(byBuff, iOffset, 0);					
							
							CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

							break;						
						}
					}
				}
				
				// Object 와 충돌 체크..
				if(bCol == false &&
					true == ACT_WORLD->CheckCollisionWithShape(pBundle->m_vPos, pBundle->m_vDir, pBundle->m_fVelocity * CN3Base::s_fSecPerFrm, &vCol))
				{
					bCol = true;
					pBundle->m_vPos = vCol;

					pBundle->Stop();
					int iMagicID = CGameProcedure::s_pProcMain->m_pMagicSkillMng->GetMagicID(pBundle->m_iIdx);
					
					uint8_t byBuff[32];
					int iOffset=0;

					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
					CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)-1);

					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.x);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.y);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.z);
					
					CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);	//?
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);

					CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

					iOffset=0;
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FAIL);
					CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)-1);

					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.x);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.y);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.z);

					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SKILLMAGIC_FAIL_KILLFLYING);			
					CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);	
					
					CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..			
				}
				// 지형과 충돌체크..
				if(bCol == false &&
					ACT_WORLD->CheckCollisionWithTerrain(pBundle->m_vPos, pBundle->m_vDir, pBundle->m_fVelocity, &vCol))
				{
					//충돌...
					//여기서 패킷 날려야 겠구만...
					bCol = true;
					pBundle->m_vPos = vCol;
					pBundle->Stop();
					int iMagicID = CGameProcedure::s_pProcMain->m_pMagicSkillMng->GetMagicID(pBundle->m_iIdx);
					
					uint8_t byBuff[32];
					int iOffset=0;

					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_EFFECTING);
					CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)-1);

					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.x);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.y);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.z);
					
					CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);	//?
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);

					CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..

					iOffset=0;
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_FAIL);
					CAPISocket::MP_AddDword(byBuff, iOffset, iMagicID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)pBundle->m_iSourceID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)-1);

					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.x);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.y);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)vCol.z);

					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)SKILLMAGIC_FAIL_KILLFLYING);			
					CAPISocket::MP_AddShort(byBuff, iOffset, pBundle->m_iIdx);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
								
					CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
				}
			}
		}			
		pBundle->Tick();
		it++;
	}
}


//
//
//
void CN3FXMgr::Render()
{
	//온갖 renderstate설정...	
	DWORD dwLgt, dwAlpha, dwZEnable;
	DWORD dwSrcBlend, dwDestBlend;

	s_lpD3DDev->GetRenderState( D3DRS_LIGHTING, &dwLgt );
	s_lpD3DDev->GetRenderState( D3DRS_ALPHABLENDENABLE,	&dwAlpha );
	s_lpD3DDev->GetRenderState( D3DRS_SRCBLEND, &dwSrcBlend );
    s_lpD3DDev->GetRenderState( D3DRS_DESTBLEND, &dwDestBlend );
	s_lpD3DDev->GetRenderState( D3DRS_ZWRITEENABLE, &dwZEnable );

	s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, FALSE );
	s_lpD3DDev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	
	__Matrix44 mtx;
	mtx.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtx);
	
	stlLIST_BUNDLEGAME_IT itBegin = m_ListBundle.begin();
	stlLIST_BUNDLEGAME_IT itEnd = m_ListBundle.end();
	stlLIST_BUNDLEGAME_IT it;
	
	for(it=itBegin; it!=itEnd; it++) (*it)->Render();

	s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, dwLgt );
	s_lpD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	dwAlpha );
	s_lpD3DDev->SetRenderState( D3DRS_SRCBLEND, dwSrcBlend );
    s_lpD3DDev->SetRenderState( D3DRS_DESTBLEND, dwDestBlend );
	s_lpD3DDev->SetRenderState( D3DRS_ZWRITEENABLE, dwZEnable );
}

bool CN3FXMgr::CheckCollisionSphere(__Vector3 vSp, __Vector3 vEp, __Vector3 vCp, float fRadius, __Vector3* vCol)
{
	__Vector3 vSpCp = vCp - vSp;
	float DistSpCp = vSpCp.Magnitude();
	if(DistSpCp <= fRadius)
	{
		(*vCol) = vSp;
		return true;
	}

	__Vector3 vDir = vEp - vSp;
	float DistSpEp = vDir.Magnitude();
	vDir.Normalize();

	__Vector3 vCross;
	vCross.Cross(vSpCp, vDir);
	float DistCross = vCross.Magnitude();

	if(DistCross <= fRadius)
	{
		float sqDistCross = DistCross*DistCross;
		float DistSpCross = sqrt((DistSpCp*DistSpCp)-sqDistCross);

		if(DistSpCross < DistSpEp)
		{
			float DistCol = DistSpCross - sqrt((fRadius*fRadius)-sqDistCross);
			
			(*vCol) = vSp + DistCol*vDir;
			return true;
		}
	}
	return false;	
}

void CN3FXMgr::ClearAll()
{
	stlLIST_BUNDLEGAME_IT it;
	for(it=m_ListBundle.begin(); it!=m_ListBundle.end(); it++)
	{
		CN3FXBundleGame* pBundle = (*it);
		if(pBundle) delete pBundle;
	}
	m_ListBundle.clear();

	stlMAP_BUNDLEORIGIN_IT itOrigin;
	for(itOrigin=m_OriginBundle.begin(); itOrigin!=m_OriginBundle.end(); itOrigin++)
	{
		LPFXBUNDLEORIGIN pSrc = itOrigin->second;
		if(pSrc)
		{
			if(pSrc->pBundle) delete pSrc->pBundle;
			delete pSrc;
		}
	}
	m_OriginBundle.clear();
}
