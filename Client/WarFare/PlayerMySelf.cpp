// PlayerMgr.cpp: implementation of the CPlayerMySelf class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlayerMySelf.h"
#include "PacketDef.h"
#include "PlayerOtherMgr.h"
#include "N3WorldManager.h"
#include "GameProcMain.h"
#include "UIInventory.h"
#include "MagicSkillMng.h"
#include "GameEng.h"
#include "text_resources.h"

#include <N3Base/DFont.h>
#include <N3Base/N3Camera.h>
#include <N3Base/N3ShapeMgr.h>
#include <N3Base/N3SndObj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CPlayerMySelf::CPlayerMySelf()
{
	m_ePlayerType = PLAYER_MYSELF; // Player Type ... Base, NPC, OTher, MySelf

	m_bRunning = false;				// 뛰는지..
	m_bMoveContinous = false;		// 계속 움직이는지..
	m_bAttackContinous = false;		// 계속 공격하는 상태인지..
	m_bSitDown = false;				// 앉아있는 상태인지....
	m_bRecruitParty = false;		// 파티모집중??
	
	m_bStun = false;				// 기절..
	m_fStunTime = 0.0f;				// 기절한 시간..


	m_fAttackTimeRecent = CN3Base::TimeGet();	// 최근에 공격한 시간..
	m_bTempMoveTurbo = false;

	m_InfoExt.Init();

	// Inventory..
	m_ChrInv.PartAlloc(PART_POS_COUNT);
	m_ChrInv.PlugAlloc(PLUG_POS_COUNT);

	m_iSendRegeneration = 0;	// 한번 보내면 다시 죽을때까지 안보내는 플래그	

	m_dwMagicID = 0xffffffff;
	m_fCastingTime = 0.0f;
	m_pObjectTarget = nullptr; // 타겟 오브젝트 포인터..
}

CPlayerMySelf::~CPlayerMySelf()
{
}



//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////



void CPlayerMySelf::Release()
{
	m_bTargetOrPosMove = false;
	m_iMoveTarget = -1;

	m_bRunning = false;				// 뛰는지..
	m_bMoveContinous = false;		// 계속 움직이는지..
	m_bAttackContinous = false;		// 계속 공격하는 상태인지..
	m_bSitDown = false;				// 앉아있는 상태인지....
	m_bRecruitParty = false;		// 파티모집중??

	m_bStun = false;				// 기절..
	m_fStunTime = 0.0f;				// 기절한 시간..

	m_fAttackTimeRecent = CN3Base::TimeGet();	// 최근에 공격한 시간..
	m_bTempMoveTurbo = false;

	m_InfoExt.Init();

	// Inventory..
	m_ChrInv.Release();
	m_ChrInv.PartAlloc(PART_POS_COUNT);
	m_ChrInv.PlugAlloc(PLUG_POS_COUNT);

	m_pObjectTarget = nullptr; // 타겟 오브젝트 포인터..

	CPlayerBase::Release();
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

void CPlayerMySelf::SetMoveTargetID(int iID)
{
	m_bTargetOrPosMove	= true;
	m_iMoveTarget		= iID;
}

void CPlayerMySelf::SetMoveTargetPos(__Vector3 vPos)
{
	m_vTargetPos		= vPos;
	m_bTargetOrPosMove	= true;
	m_iMoveTarget		= -1;
}

void CPlayerMySelf::Tick()
{
	BOOL bAnim = TRUE;

	if (PSA_DEATH == m_eState)  // 죽는 상태이고... 죽는 에니메이션이 끝나면.. // 한번 보내면 다시 죽을때까지 안보내는 플래그
	{
		if (0 == m_iSendRegeneration)
		{
//			std::string szMsg = fmt::format_text_resource(IDS_REGENERATION);
//			CGameProcedure::MessageBoxPost(szMsg, "", MB_OK, BEHAVIOR_REGENERATION);
//			CLogWriter::Write("Dead!!!");
			m_iSendRegeneration = 1;
		}
		return;
	}

	if(m_fTimeAfterDeath > 0) // 강제로 죽여야 한다..
	{
		if(m_fTimeAfterDeath > 3.0f)
			this->Action(PSA_DYING, false); // 3 초가 지나야 죽는다.
		CPlayerBase::Tick();  // 회전, 지정된 에니메이션 Tick 및 색깔 지정 처리.. 등등..
		return;
	}
	
	if(IsDead()) // 죽은 상태면 돌아간다.
	{
		CGameProcedure::s_pProcMain->CommandEnableAttackContinous(false, nullptr);
		CPlayerBase::Tick();  // 회전, 지정된 에니메이션 Tick 및 색깔 지정 처리.. 등등..
		return;
	}

	////////////////////////////////////////////////////////////////////////////////
	// 내 플레이어는 움직이는게 다른넘들과 다르기 때문에 특별하게 처리..
	if(	PSM_WALK == m_eStateMove ||
		PSM_WALK_BACKWARD  == m_eStateMove ||
		PSM_RUN == m_eStateMove ) // 앞뒤로 걸어가거나 달려가면.
	{
		this->MoveSpeedCalculationAndCheckCollision(); // 움직이는 속도 및 충돌체크...
		if(m_fMoveSpeedPerSec == 0.0f) // 속도가 없으면 충돌체크 결과 못가는 거다...
		{
			this->ActionMove(PSM_STOP);
			CGameProcedure::s_pProcMain->MsgSend_Move(false, false); // 정지 패킷 보내기..
		}

		__Vector3 vPos = this->Position();
		CPlayerBase::m_pSnd_MyMove = m_pSnd_Move;
		if(m_pSnd_Move && m_Chr.NeedPlaySound0()) 
			m_pSnd_Move->Play(&vPos);
		if(m_pSnd_Move && m_Chr.NeedPlaySound1()) 
			m_pSnd_Move->Play(&vPos);

		TargetOrPosMove();
	}
	else 
	{
		m_pSnd_MyMove = nullptr;
		m_fMoveSpeedPerSec = 0;
	}
	// 내 플레이어는 움직이는게 다른넘들과 다르기 때문에 특별하게 처리..
	////////////////////////////////////////////////////////////////////////////////

	CPlayerBase* pTarget = nullptr;
	if (m_bAttackContinous)
	{
		pTarget = TargetPointerCheck(false);
		if(pTarget == nullptr)
		{
			CGameProcedure::s_pProcMain->CommandEnableAttackContinous(false, nullptr);
		}
		else
		{
			float fTime = TimeGet();
			
			// process an archery auto-attack
			if(	(m_pItemPlugBasics[PLUG_POS_LEFTHAND] && ITEM_CLASS_BOW == m_pItemPlugBasics[PLUG_POS_LEFTHAND]->byClass ) || 
				(m_pItemPlugBasics[PLUG_POS_LEFTHAND] && ITEM_CLASS_BOW_LONG == m_pItemPlugBasics[PLUG_POS_LEFTHAND]->byClass ) || 
				(m_pItemPlugBasics[PLUG_POS_RIGHTHAND] && ITEM_CLASS_BOW_CROSS == m_pItemPlugBasics[PLUG_POS_RIGHTHAND]->byClass) )
			{
				__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(102003); // 스킬 테이블에서 기본 활 스킬을 찾고
				if(pSkill)
				{
					if(pTarget->IsAlive())//임시 유저가 살아 있는 상태에서만...
					{
						float fInterval = (pSkill->iCastTime / 10.f) + 0.15f;
						if(m_fAttackDelta > 0) fInterval /= m_fAttackDelta;
						if(fTime > m_fAttackTimeRecent + fInterval) // 공격 간격이 넘으면.. 공격!
						{
							if(CGameProcedure::s_pProcMain->m_pMagicSkillMng->MsgSend_MagicProcess(m_iIDTarget, pSkill))
							{// 스킬 패킷 보내기에 성공하면.
								//TRACE("%.2f\n", fTime - m_fAttackTimeRecent);
							}
							m_fAttackTimeRecent = fTime;	// 최근에 공격한 시간..
						}
					}
				}
			}
			// a "javelin launcher" attack
			else if( m_pItemPlugBasics[PLUG_POS_RIGHTHAND]
				&& ITEM_CLASS_LAUNCHER == m_pItemPlugBasics[PLUG_POS_RIGHTHAND]->byClass )
			{
				__TABLE_UPC_SKILL* pSkill = s_pTbl_Skill.Find(102009); // 스킬 테이블에서 기본 활 스킬을 찾고
				if(pSkill && fTime > m_fAttackTimeRecent + (pSkill->iCastTime / 10.f)) // 공격 간격이 넘으면.. 공격!
				{
					if(pTarget->IsAlive())//임시 유저가 살아 있는 상태에서만
					{
						CGameProcedure::s_pProcMain->m_pMagicSkillMng->MsgSend_MagicProcess(m_iIDTarget, pSkill); // 스킬 패킷 보내기에 성공하면.
						//TRACE("%.2f\n", fTime - m_fAttackTimeRecent);
						m_fAttackTimeRecent = fTime;	// 최근에 공격한 시간..
					}
				}
			}
			// melee weapon auto-attack
			else
			{
				float fIntervalTable = 1.0f;
				if(m_pItemPlugBasics[PLUG_POS_RIGHTHAND] && m_pItemPlugExts[PLUG_POS_RIGHTHAND]) // 공격 간격 정의
				{
					fIntervalTable = (m_pItemPlugBasics[PLUG_POS_RIGHTHAND]->siAttackInterval / 100.0f)
						* (m_pItemPlugExts[PLUG_POS_RIGHTHAND]->siAttackIntervalPercentage / 100.0f); // 100 은 1초 110 이면 1.1 초등.. 퍼센트로 간다...
				}
				
				float fInterval = fIntervalTable;
				// scale for attack speed modifiers
				if(m_fAttackDelta > 0)
					fInterval /= m_fAttackDelta; 

				if(	fTime > m_fAttackTimeRecent + fInterval)
				{
					// cannot auto-attack while casting
					bool bCastingNow = CGameProcedure::s_pProcMain->m_pMagicSkillMng->IsCasting();
					if(!bCastingNow)
					{
						if(IsAttackableTarget(pTarget))
						{
							float fDistance = s_pPlayer->DistanceExceptRadius(pTarget);
							CGameProcedure::s_pProcMain->MsgSend_Attack(pTarget->IDNumber(), fIntervalTable, fDistance);

							// start the auto-attack animation loop if it isn't already playing
							if (PSA_SPELLMAGIC != m_eState
								&& PSA_ATTACK != m_eState
								&& m_fFlickeringFactor == 1.0f)
								this->Action(PSA_ATTACK, true, pTarget);
							
							m_fAttackTimeRecent = fTime;
						}
						else 
						{
							if(CGameProcedure::s_pEng->ViewPoint() == VP_THIRD_PERSON)
							{
								RotateTo(pTarget); // 방향을 돌린다.

								float fDist = (pTarget->Position() - m_Chr.Pos()).Magnitude(); // 공격 거리를 구하고..
								float fDistLimit = this->AttackableDistance(pTarget);

								if(fDist > fDistLimit && !m_bTargetOrPosMove)
								{
									CGameProcedure::s_pProcMain->CommandMove(MD_FORWARD, true);
									s_pPlayer->SetMoveTargetID(pTarget->IDNumber());
								}
							}

							// If not casting a spell or sitting down, loop auto-attack animation
							if (PSA_SPELLMAGIC != m_eState
								&& PSA_SITDOWN != m_eState)
								this->Action(PSA_BASIC, true);
						}
					}
				}
			}

			// if an attack was triggered this tick, make the character stand
			if(fTime == m_fAttackTimeRecent)
				CGameProcedure::s_pProcMain->CommandSitDown(false, false);
		}
	}


	// process stun cooldown
	if(m_bStun) 
	{
		m_fStunTime -= s_fSecPerFrm;
		if(m_fStunTime < 0)
			this->StunRelease();
	}

	// if the character is in an attack animation state, attempt to process an auto-attack
	if (PSA_ATTACK == m_eState
		|| PSA_SPELLMAGIC == m_eState)
	{
		if(pTarget == nullptr)
			pTarget = TargetPointerCheck(false);
		CPlayerBase::ProcessAttack(pTarget); 
	}

	if(m_dwMagicID != 0xffffffff) 
		m_fCastingTime += CN3Base::s_fSecPerFrm;

	// call super to rotate, animation tick, etc
	CPlayerBase::Tick();
}

void CPlayerMySelf::Render(float fSunAngle)
{
	// 강제로 LOD 레벨 + 1
	m_Chr.m_nLOD--;
	if(m_Chr.m_nLOD < 0) m_Chr.m_nLOD = 0;

	int iLODDeltaPrev = CN3Chr::LODDelta(); // 내캐릭은 좀더 정교하게 그리자..
	CN3Chr::LODDeltaSet(0);
	CPlayerBase::Render(fSunAngle);
	CN3Chr::LODDeltaSet(iLODDeltaPrev);
}




bool CPlayerMySelf::ToggleAttackContinous()
{
	if(!IsAlive()) return false;

	if(false == m_bAttackContinous) // 타겟이 있는지 본다..
	{
		CPlayerNPC* pTarget = s_pOPMgr->CharacterGetByID(m_iIDTarget, true);
		if(pTarget) //  && !IsOutOfAttackRange(pTarget)) // 타겟이 있고 공격 가능한 범위에 있을때만
		{
			this->m_bAttackContinous = true;
		}
	}
	else
	{
		m_bAttackContinous = false;
	}

	return m_bAttackContinous;
}

bool CPlayerMySelf::ToggleRunMode()
{
	m_bRunning = !m_bRunning; // 뛰는지..
	if(true == m_bRunning)
	{
		if(PSM_WALK == m_eStateMove) 
		{
			m_eStateMove = PSM_RUN;
			m_Chr.AniCurSet(ANI_RUN);
		}
	}
	else
	{
		if(PSM_RUN == m_eStateMove) 
		{
			m_eStateMove = PSM_WALK;
			m_Chr.AniCurSet(ANI_WALK);
		}
	}

	return m_bRunning;
}

void CPlayerMySelf::ToggleMoveMode()
{
	m_bMoveContinous = !m_bMoveContinous; // 계속 움직이는지..
	
	int nAni = ANI_BREATH;
	if(m_bMoveContinous) // 계속 움직여야 하면..
	{
		if(m_bRunning) // 뛰면..
		{
			m_eStateMove = PSM_RUN;
			nAni = ANI_RUN;
		}
		else
		{
			m_eStateMove = PSM_WALK;
			nAni = ANI_WALK;
		}

		float fSpeed = this->MoveSpeedCalculationAndCheckCollision(); // 움직이는 속도 및 충돌체크...
		if(0 == fSpeed) // 갈수 없으면...
		{
			m_bMoveContinous = false;
			m_eStateMove = PSM_STOP;
			m_eState = PSA_BASIC;
			nAni = this->JudgeAnimationBreath(); // 타겟이 있으면. 들고 있는 무기에 따라 기본 에니메이션 판다.. 없으면 걍 기본 에니메이션
		}
	} 
	else
	{
		m_eStateMove = PSM_STOP;
		m_eState = PSA_BASIC;
		nAni = this->JudgeAnimationBreath(); // 타겟이 있으면. 들고 있는 무기에 따라 기본 에니메이션 판다.. 없으면 걍 기본 에니메이션
	}

	this->AnimationClear();
	m_Chr.AniCurSet(nAni); // 통째로 걷고 뒤기, 멈추기 적용
}

__Vector3 CPlayerMySelf::NextPos(float fTimeAfter)
{
	__Matrix44 mtxRot = m_Chr.Rot();
	__Vector3 vDir(0,0,1);
	__Vector3 vNextPos = m_Chr.Pos() + ((vDir * mtxRot) * m_fMoveSpeedPerSec * fTimeAfter);

	return vNextPos;
}

void CPlayerMySelf::RotAdd(const float fRotRadianPerSec)			// y 축을 기준으로 초당 회전하는 속도를 넣어준다.
{
	m_fYawCur += fRotRadianPerSec * s_fSecPerFrm;

	if(m_fYawCur >= D3DXToRadian(180.0f) * 2) m_fYawCur -= D3DXToRadian(180.0f) * 2;
	if(m_fYawCur <= -D3DXToRadian(180.0f) * 2) m_fYawCur += D3DXToRadian(180.0f) * 2;
	m_fYawToReach = m_fYawCur; // 바로 돌린다..
}

void CPlayerMySelf::InventoryChrRender(const RECT& Rect)
{
/*
	uint32_t dwUsefog = TRUE;
	CN3Base::s_lpD3DDev->GetRenderState( D3DRS_FOGENABLE , &dwUsefog );

	uint32_t dwUseLighting=TRUE;
	CN3Base::s_lpD3DDev->GetRenderState( D3DRS_LIGHTING, &dwUseLighting );

	int	bLight[8];
	for ( int i = 0; i < 8; i++ )	CN3Base::s_lpD3DDev->GetLightEnable(i, &bLight[i]);

	if (dwUseLighting) CN3Base::s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, TRUE );
	if (dwUsefog) CN3Base::s_lpD3DDev->SetRenderState( D3DRS_FOGENABLE, FALSE );
	// set render states
	for ( i = 1; i < 8; i++ )	CN3Base::s_lpD3DDev->LightEnable(i, FALSE);
	CN3Base::s_lpD3DDev->LightEnable(0, TRUE);

	D3DLIGHT8 lgt0;
	
	memset(&lgt0, 0, sizeof(D3DLIGHT8));
	lgt0.Type = D3DLIGHT_POINT;
	lgt0.Attenuation0 = 0.5f;
	lgt0.Range = 100.0f;
	lgt0.Position = __Vector3(0.0f, 2.0f, -10.0f);
	lgt0.Diffuse.r = 220/255.0f; lgt0.Diffuse.g = 255/255.0f; lgt0.Diffuse.b = 220/255.0f;
	CN3Base::s_lpD3DDev->SetLight(0, &lgt0);

	//캐릭터 찍기..
	//
	__Matrix44 mtxproj, mtxview, mtxworld, mtxprojback, mtxviewback, mtxworldback;

	CN3Base::s_lpD3DDev->GetTransform( D3DTS_PROJECTION, &mtxprojback );
	CN3Base::s_lpD3DDev->GetTransform( D3DTS_VIEW, &mtxviewback );
	CN3Base::s_lpD3DDev->GetTransform( D3DTS_WORLD, &mtxworldback );

	D3DXMatrixOrthoLH(&mtxproj, 12.0f, 9.0f, 0, 100);  
    CN3Base::s_lpD3DDev->SetTransform( D3DTS_PROJECTION, &mtxproj );

    D3DXMatrixLookAtLH( &mtxview, &D3DXVECTOR3( 0.0f, 2.0f,-10.0f ),
                                  &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
                                  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
    CN3Base::s_lpD3DDev->SetTransform( D3DTS_VIEW, &mtxview );

	mtxworld.Identity();
	CN3Base::s_lpD3DDev->SetTransform( D3DTS_WORLD, &mtxworld );


///////////////////////////////////////////////////////////////
// 2d 좌표 구함..

	// 2d -> 3d..
	__Vector3 vPos;
	vPos.x = (float)((Rect.right - Rect.left)/2.0f + Rect.left);
	vPos.y = (float)(Rect.bottom) - (Rect.bottom - Rect.top)/16.0f;
	vPos.z = 0.11f;

	float fWidth, fHeight;
	fWidth = s_CameraData.vp.Width;	fHeight = s_CameraData.vp.Height;	fWidth *= 0.5f;	fHeight *= 0.5f;

	vPos.x = (vPos.x - fWidth)/fWidth;
	vPos.y = (fHeight - vPos.y)/fHeight;

	__Matrix44 mtxProjInv, mtxViewInv;
	D3DXMatrixInverse(&mtxProjInv, nullptr, &mtxproj);
	D3DXMatrixInverse(&mtxViewInv, nullptr, &mtxview);

	vPos *= mtxProjInv;
	vPos *= mtxViewInv;

	m_ChrInv.PosSet(vPos.x, vPos.y, 1.0f);
	__Quaternion qt;
	qt.RotationAxis(0.0f, 1.0f, 0.0f, D3DXToRadian(198.0f));
	m_ChrInv.RotSet(qt);

	CGameProcedure::s_pEng->ClearZBuffer(nullptr);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_ChrInv.m_nLOD = 0;
	m_ChrInv.Render();

	CN3Base::s_lpD3DDev->SetTransform( D3DTS_PROJECTION, &mtxprojback );
	CN3Base::s_lpD3DDev->SetTransform( D3DTS_VIEW, &mtxviewback );
	CN3Base::s_lpD3DDev->SetTransform( D3DTS_WORLD, &mtxworldback );

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_AMBIENT, 0x00000000);

	CN3Base::s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, dwUseLighting );
	CN3Base::s_lpD3DDev->SetRenderState( D3DRS_FOGENABLE , dwUsefog );
	for ( i = 0; i < 8; i++ )	CN3Base::s_lpD3DDev->LightEnable(i, bLight[i]);
*/
	// 아래로 dino수정
	// backup render state
	DWORD dwLighting;
	D3DLIGHT9 BackupLight0;

	s_lpD3DDev->GetRenderState( D3DRS_LIGHTING, &dwLighting );
	BOOL bLight[8];
	for ( int i = 0; i < 8; ++i )	s_lpD3DDev->GetLightEnable(i, &(bLight[i]));
	s_lpD3DDev->GetLight(0, &BackupLight0);

	// set render state
	if (TRUE != dwLighting) s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, TRUE );
	for ( int i = 1; i < 8; ++i )	s_lpD3DDev->LightEnable(i, FALSE);
	s_lpD3DDev->LightEnable(0, TRUE);

	// 0번 light 설정
	D3DLIGHT9 Light0;
	memset(&Light0, 0, sizeof(D3DLIGHT9));
	Light0.Type = D3DLIGHT_POINT;
	Light0.Attenuation0 = 0.5f;
	Light0.Range = 100.0f;
	Light0.Position = __Vector3(0.0f, 2.0f, 10.0f);
	Light0.Diffuse.r = 220/255.0f; Light0.Diffuse.g = 255/255.0f; Light0.Diffuse.b = 220/255.0f;
	s_lpD3DDev->SetLight(0, &Light0);

	// 캐릭터 위치와 방향 세팅
	m_ChrInv.PosSet(__Vector3(0,0,0));
	__Quaternion qt;
	qt.RotationAxis(0.0f, 1.0f, 0.0f, D3DXToRadian(18.0f));	// 약간 비스듬하게 서있게..
	m_ChrInv.RotSet(qt);

	// render
	RenderChrInRect(&m_ChrInv, Rect);

	// restore
	if (TRUE != dwLighting) s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, dwLighting );
	for (int i = 0; i < 8; ++i )	s_lpD3DDev->LightEnable(i, bLight[i]);
	s_lpD3DDev->SetLight(0, &BackupLight0);
}

void CPlayerMySelf::InventoryChrTick()
{
	// Inventory Animation Tick..
	m_ChrInv.PosSet(m_Chr.Pos());
	m_ChrInv.Tick();
}



void CPlayerMySelf::InventoryChrAnimationInitialize()
{
	int iAniTmp = m_ChrInv.AniCtrl()->Count() - 1;
	m_ChrInv.AniCurSet(iAniTmp, false, 0);
	m_ChrInv.AniCurSet(ANI_BREATH, false, 0);
	m_ChrInv.Tick(); // 한번 해준다..
}

CN3CPlugBase* CPlayerMySelf::PlugSet(e_PlugPosition ePos, const std::string& szFN, __TABLE_ITEM_BASIC* pItemBasic, __TABLE_ITEM_EXT* pItemExt)
{
	int iJoint = 0;
	if(PLUG_POS_RIGHTHAND == ePos)
	{
		m_pItemPlugBasics[ePos] = pItemBasic;
		m_pItemPlugExts[ePos] = pItemExt;
		iJoint = m_pLooksRef->iJointRH;
	}
	else if(PLUG_POS_LEFTHAND == ePos)
	{
		m_pItemPlugBasics[ePos] = pItemBasic;
		m_pItemPlugExts[ePos] = pItemExt;
		if(pItemBasic)
		{
			if(pItemBasic->byClass == ITEM_CLASS_SHIELD) iJoint = m_pLooksRef->iJointLH2; // 방패인 경우
			else iJoint = m_pLooksRef->iJointLH; // 왼손 끝..
		}
	}
	else if (PLUG_POS_KNIGHTS_GRADE == ePos)
	{
		m_pItemPlugBasics[ePos] = pItemBasic;
		m_pItemPlugExts[ePos] = pItemExt;
		iJoint = m_pLooksRef->iJointRH - 2; // 오른쪽 팔목
	}
	else if (PLUG_POS_BACK == ePos)
	{
		//m_pItemBasicPlugRefs[PLUG_POS_BACK] = pItem;
	}
	else { __ASSERT(0, "Invalid Plug Item position"); }

	CN3CPlugBase* pPlug = m_ChrInv.PlugSet(ePos, szFN);
	if(nullptr == pPlug) return nullptr;

	if(PLUG_POS_LEFTHAND == ePos || PLUG_POS_RIGHTHAND == ePos) // 키에 비례해서 크게 키운다. 기본키는 1.8 미터이다. 
	{
		float fScale = m_Chr.Height() / 1.8f;
		fScale *= pPlug->Scale().y / m_Chr.Scale().y;
		pPlug->ScaleSet(__Vector3(fScale, fScale, fScale));
		pPlug->m_nJointIndex = iJoint; // 관절 번호 세팅..
	}
//	else if(PLUG_POS_BACK == ePos)
//	{
//		CN3CPlug_Cloak *pPlugCloak = (CN3CPlug_Cloak*)pPlug;
//	}

	this->SetSoundPlug(pItemBasic);
	return CPlayerBase::PlugSet(ePos, szFN, pItemBasic, pItemExt);
}

CN3CPart* CPlayerMySelf::PartSet(e_PartPosition ePos, const std::string& szFN, __TABLE_ITEM_BASIC* pItemBasic, __TABLE_ITEM_EXT* pItemExt)
{
	if(ePos < PART_POS_UPPER || ePos >= PART_POS_COUNT)
	{
		__ASSERT(0, "Invalid Item Position");
		return nullptr;
	}

	if(PART_POS_UPPER == ePos) // 상체일 경우 특별한 처리가 필요..
	{
		if(pItemBasic) // 입히는 경우
		{
			if(pItemBasic->byIsRobeType && m_Chr.Part(PART_POS_LOWER)) // 로브 타입의 통짜 윗옷이고 아래에 뭔가 입고 있으면..
			{
				m_ChrInv.PartSet(PART_POS_LOWER, ""); // 아래를 비워준다..
				m_Chr.PartSet(PART_POS_LOWER, "");
			}
		}
		else // 상체를 벗는 경우
		{
			if(m_pItemPartBasics[ePos] && m_pItemPartBasics[ePos]->byIsRobeType) // 전에 착용했던게 로브면..
			{
				if(m_pItemPartBasics[PART_POS_LOWER]) // 하체에 아이템이 입혀있으면..
				{
					std::string szFN2;
					e_PartPosition ePartPos2 = PART_POS_UNKNOWN;
					e_PlugPosition ePlugPos2 = PLUG_POS_UNKNOWN;
					MakeResrcFileNameForUPC(m_pItemPartBasics[PART_POS_LOWER], m_pItemPartExts[PART_POS_LOWER], &szFN2, nullptr, ePartPos2, ePlugPos2, m_InfoBase.eRace);
					
					m_ChrInv.PartSet(PART_POS_LOWER, szFN2); // 하체에 전의 옷을 입힌다..
					m_Chr.PartSet(PART_POS_LOWER, szFN2);
				}
				else // 하체에 입고 있었던 아이템이 없다면..
				{
					__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(m_InfoBase.eRace);	// User Player Character Skin 구조체 포인터..
					m_ChrInv.PartSet(PART_POS_LOWER, pLooks->szPartFNs[PART_POS_LOWER]); // 하체에 기본옷을 입힌다.
					m_Chr.PartSet(PART_POS_LOWER, pLooks->szPartFNs[PART_POS_LOWER]); // 하체에 기본옷을 입힌다.
				}
			}
		}
	}
	else if(PART_POS_LOWER == ePos) // 하체일 경우..
	{
		// 아이템을 착용하건 벗건간에...
		if(m_pItemPartBasics[PART_POS_UPPER] && m_pItemPartBasics[PART_POS_UPPER]->byIsRobeType) // 전에 상체에 착용했던게 로브면..
		{
			m_pItemPartBasics[ePos] = pItemBasic; // 아이템 적용
			m_pItemPartExts[ePos] = pItemExt;
			m_ChrInv.PartSet(ePos, "");
			m_Chr.PartSet(ePos, ""); // 하체는 벗기고(?)..
			return nullptr; // 돌아간다.
		}
	}

	CN3CPart* pPart = nullptr;
	if(szFN.empty()) // 파일 이름이 없는거면.. 기본 착용..
	{
		if(PART_POS_HAIR_HELMET == ePos)
		{
			this->InitHair();
			pPart = m_Chr.Part(ePos);
		}
		else if(PART_POS_FACE == ePos)
		{
			this->InitFace();
			pPart = m_Chr.Part(ePos);
		}
		else
		{
			__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(m_InfoBase.eRace);	// Player Character Skin 구조체 포인터..
			if(pLooks)
			{
				m_ChrInv.PartSet(ePos, pLooks->szPartFNs[ePos]);
				pPart = m_Chr.PartSet(ePos, pLooks->szPartFNs[ePos]);
				if(pPart) pPart->TexOverlapSet("");
			}
		}
	}
	else
	{
		m_ChrInv.PartSet(ePos, szFN);
		pPart = m_Chr.PartSet(ePos, szFN);
	}

	m_pItemPartBasics[ePos] = pItemBasic; // 아이템 적용
	m_pItemPartExts[ePos] = pItemExt;

	return pPart;
}

bool CPlayerMySelf::InitChr(__TABLE_PLAYER_LOOKS* pTbl)
{
	CPlayerBase::InitChr(pTbl);

	m_ChrInv.JointSet(pTbl->szJointFN);
	m_ChrInv.AniCtrlSet(pTbl->szAniFN);
	
	float fScale = 2.1f / m_Chr.Height();
	m_ChrInv.ScaleSet(fScale, fScale, fScale); // 인벤토리 창에 들어가게 크기를 줄여준다..

	return true;
}

float CPlayerMySelf::AttackableDistance(CPlayerBase* pTarget)
{
	if(nullptr == pTarget) return 0;

	float fDistLimit = (m_Chr.Radius() + pTarget->Radius())/2.0f; // 공격 거리제한..
	if(m_pItemPlugBasics[0])
		fDistLimit += m_pItemPlugBasics[0]->siAttackRange / 10.0f; // 아이템을 들고 있으면..
	else if(m_pItemPlugBasics[1] && ITEM_POS_TWOHANDLEFT == m_pItemPlugBasics[1]->byAttachPoint) // 왼손에 투핸드레프트(활) 아이템을 들고 있으면..
		fDistLimit += m_pItemPlugBasics[1]->siAttackRange / 10.0f;

	return fDistLimit;
}

float CPlayerMySelf::DistanceExceptRadius(CPlayerBase* pTarget)
{
	if(nullptr == pTarget) return 0;

	float fDist = (pTarget->Position() - m_Chr.Pos()).Magnitude();
	float fDistRadius = (m_Chr.Radius() + pTarget->Radius())/2.0f; // 공격 거리제한..

	return fDist - fDistRadius;
}

/// \brief checks to see if an attack can be performed on the target
/// \param pTarget
/// \param bMeasureAngle does the target need to be in front of the attacker?
/// \returns true if attack would be valid, false otherwise
bool CPlayerMySelf::IsAttackableTarget(CPlayerBase* pTarget, bool bMeasureAngle)
{
	if (pTarget == nullptr || pTarget->IsDead())
		return false;

	// cannot attack when invulnerable 
	if (m_fFlickeringFactor != 1.0f)
		return false;

	if (!IsHostileTarget(pTarget))
		return false;

	// make sure the target is within range
	float fDist = (pTarget->Position() - m_Chr.Pos()).Magnitude();
	float fDistLimit = this->AttackableDistance(pTarget);
	if(fDist > fDistLimit)
		return false;

	// check that the target is in front of the attacker
	if(bMeasureAngle)
	{
		__Vector3 vDir = this->Direction();
		vDir.y = 0;
		vDir.Normalize();
		__Vector3 vDirTarget = pTarget->Position() - m_Chr.Pos();
		vDirTarget.y = 0;
		vDirTarget.Normalize();
		float fDot = vDir.Dot(vDirTarget);
		if(fDot > 0.7f)
			return true;
		
		return false;
	}

	return true;
}

// TODO(Gilad): It'd probably be better to refactor this one as it modifies the m_fYNext variable which is part of the PlayerBase
// class and when the packet is sent to the game server, it reads the next Y coordinates to climb the terrain height.
// Why? Because we just checking if collision detected and it should just check if collision detected and not modify any global members
// or pointers. It would be more partical to check the whole thing if authority = GM when more code is implemented here in the future.
// Another option would be to send a packet to the server validate the collision through the game server and let the game server check 
// against wall hacking.
bool CPlayerMySelf::CheckCollision()
{
	float fSpeed = m_fMoveSpeedPerSec * CN3Base::s_fSecPerFrm; // The speed at which the current moving speed is calculated.
	if(0 == fSpeed) return false; // If it does not move, do not check collision.

	__Vector3 vPos = this->Position();

	__Vector3 vDir(0,0,1); // Find the rotation value ...
	__Matrix44 mtxRot = this->Rotation(); 
	vDir *= mtxRot; // Find the direction of rotation.
	if(fSpeed < 0)
	{
		fSpeed *= -1.0f;
		vDir *= -1.0f;
	}
	__Vector3 vPosNext = vPos + (vDir * fSpeed); // 다음 위치 계산..
	if (s_pPlayer->m_InfoBase.iAuthority != AUTHORITY_MANAGER && 
		false == ACT_WORLD->IsInTerrainWithTerrain(vPosNext.x, vPosNext.z, vPos) )
		return true; // 경계 안에 있지 않으면..


	//////////////////////////////////
	// 다른 플레이어와 체크..
	CPlayerOther* pUPC = nullptr;
	float fHeightSum, fMag;
	it_UPC it = s_pOPMgr->m_UPCs.begin(), itEnd = s_pOPMgr->m_UPCs.end();
	for(; it != itEnd; it++)
	{
		pUPC = it->second;

		//죽어 있는 상태의 캐릭터는 충돌체크를 하지 않는다.
		if (pUPC->IsDead())
			continue;

		// 같은 국가...
		if (!IsHostileTarget(pUPC))
			continue;
		
		fMag = (pUPC->Position() - vPos).Magnitude();
		if(fMag < 32.0f)
		{
			fHeightSum = (pUPC->Height() + m_Chr.Height()) / 2.5f;
			if(fMag < fHeightSum) // 거리가 키의 합보다 작으면..
			{
				float fMag2 = (pUPC->Position() - vPosNext).Magnitude(); // 다음위치가 더 가까우면.
				if(s_pPlayer->m_InfoBase.iAuthority != AUTHORITY_MANAGER && fMag2 < fMag)
					return true;
			}
		}
	}
	// 다른 플레이어와 체크..
	//////////////////////////////////

//	__TABLE_ZONE* pZoneInfo = s_pTbl_Zones.Find(m_InfoExt.iZoneCur);
//	if(pZoneInfo && pZoneInfo->bNPCCollisionCheck) //this_zone

	//적국 엔피씨는 충돌 체크를 한다.
	CPlayerNPC* pNPC = nullptr;
	it_NPC it_N = s_pOPMgr->m_NPCs.begin(),	it_NEnd	= s_pOPMgr->m_NPCs.end();
	for(; it_N != it_NEnd; it_N++)
	{
		pNPC = it_N->second;

		if(pNPC->m_pShapeExtraRef) continue; // 성문등의 형태이면 충돌체크를 하지 않는다..

		// 같은 국가...
		if (!IsHostileTarget(pNPC))
			continue;

		fMag = (pNPC->Position() - vPos).Magnitude();
		if(fMag < 32.0f)
		{
			fHeightSum = (pNPC->Height() + m_Chr.Height()) / 2.5f;
			if(fMag < fHeightSum) // 거리가 키의 합보다 작으면..
			{
				float fMag2 = (pNPC->Position() - vPosNext).Magnitude(); // 다음위치가 더 가까우면.
				if(s_pPlayer->m_InfoBase.iAuthority != AUTHORITY_MANAGER && fMag2 < fMag)
					return true;
			}
		}
	}//for(


	// 오브젝트와 충돌체크..
	__Vector3 vPos2 = vPos, vCol, vNormal;
	if (!s_pWorldMgr->IsIndoor())
		vPos2.y += 0.5f; // 캐릭터 발높이에서 0.5 미터 높이 위에서 충돌체크한다.
	else
		vPos2.y += 0.6f; // 캐릭터 발높이에서 0.6 미터 높이 위에서 충돌체크한다.	이 함수 내에서 쓰는 0.6은 PvsMgr의 m_fVolumeOffs.. ^^
	bool bColShape = ACT_WORLD->CheckCollisionWithShape(vPos2, vDir, fSpeed, &vCol, &vNormal);
	if(s_pPlayer->m_InfoBase.iAuthority != AUTHORITY_MANAGER && bColShape) return true; // 오브젝트와 충돌값이 있으면 

	////////////////////////////////////////////////////////////////////////////////
	// 지면과 오브젝트의 높이값 구하기..
	float fYTerrain = ACT_WORLD->GetHeightWithTerrain(vPosNext.x, vPosNext.z);		// 지면의 높이값..
	float fYClimb = ACT_WORLD->GetHeightNearstPosWithShape(vPosNext, CN3Base::s_fSecPerFrm * 30.0f, &vNormal); // 충돌 체크 오브젝트의 높이값..
	vNormal.y = 0; // 이래야 정상적인 경사를 얻을수 있다..
	
	if (!s_pWorldMgr->IsIndoor())
	{
		if(fYClimb > fYTerrain && fYClimb < vPosNext.y + ((30.0f/CN3Base::s_fFrmPerSec) * 0.5f)) // 충돌 체크 오브젝트 높이값이 있고 지형보다 높을 경우만 높이값 적용
		{
			if(s_pPlayer->m_InfoBase.iAuthority != AUTHORITY_MANAGER && 
				vNormal.Magnitude() > MAX_INCLINE_CLIMB && vNormal.Dot(vDir) <= 0.0f) // 경사 체크..
			{
				return true;
			}
			m_fYNext = fYClimb;
		}
		else
		{
			// 지형의 경사가 45 도 이하인지 체크
			if(s_pPlayer->m_InfoBase.iAuthority != AUTHORITY_MANAGER && 
				true == ACT_WORLD->CheckInclineWithTerrain(vPosNext, vDir, MAX_INCLINE_CLIMB))
			{
				return true;
			}
			m_fYNext = fYTerrain; // 다음 위치를 맞추고..
		}
	}
	else	// 일단..
	{
		if ((fYClimb > fYTerrain) && (fYClimb < vPosNext.y + 0.6f))
			m_fYNext = fYClimb;
		else
			m_fYNext = fYTerrain; // 다음 위치를 맞추고..

		if ((m_fYNext > vPos.y + 0.6f) || (m_fYNext < vPos.y - 0.6f*2.0f))
		{
			m_fYNext = vPos.y;
			if (s_pPlayer->m_InfoBase.iAuthority != AUTHORITY_MANAGER) return true;
		}
	}

//	else // 올라갈수 없는 곳이면 지형과의 기울기 체크..
//	{
//		// 방향을 구해서.. 기울기에 따라 다른 속도를 적용
//		s_pTerrain->GetNormal(vPos.x, vPos.z, vNormal);
//		vNormal.Normalize();
//		vNormal.y	= 0.0f;
//		float fM = vNormal.Magnitude();
//		float fD = vNormal.Dot(vDir);
//		if(fSpeed < 0) fD *= -1.0f;
//		if(fD < 0) fSpeed *= 1.0f - (fM / 0.7071f); // 기울기에 따른 팩터 적용
//
//		vPosNext = vPos + (vDir * fSpeed); // 다음 위치 계산..
//		m_fYNext = s_pTerrain->GetHeight(vPosNext.x, vPosNext.z);
//	}

	this->PositionSet(vPosNext, false);

	///////////////////////////////////////////////////////////////
	// 캐릭터 충돌 체크..
//	int iSize = s_pOPMgr->m_OPCs.size();
//	it_UPC it = s_pOPMgr->m_OPCs.begin();
//	for( int i = 0; i < iSize; i++, it++ )
//	{
//		if ( ((*it)->Position() - vPosAfter).Magnitude() < 1.2f )
//			return vPosBefore;
//	}

	return false;
}


void CPlayerMySelf::InitFace()
{
	__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(m_InfoBase.eRace);

	// 아이템이 있고 얼굴 이름이 있으면..
	if (pLooks != nullptr
		&& !pLooks->szPartFNs[PART_POS_FACE].empty())
	{
		char szDir[_MAX_DIR] = {}, szFName[_MAX_FNAME] = {}, szExt[_MAX_EXT] = {};
		_splitpath(pLooks->szPartFNs[PART_POS_FACE].c_str(), nullptr, szDir, szFName, szExt);
		
		std::string szFN = fmt::format("{}{}{:02}{}", szDir, szFName, m_InfoExt.iFace, szExt);
		PartSet(PART_POS_FACE, szFN, nullptr, nullptr);
	}
}

void CPlayerMySelf::InitHair()
{
	__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(m_InfoBase.eRace);

	// 아이템이 있고 얼굴 이름이 있으면..
	if (pLooks != nullptr
		&& !pLooks->szPartFNs[PART_POS_HAIR_HELMET].empty())
	{
		char szDir[_MAX_DIR] = {}, szFName[_MAX_FNAME] = {}, szExt[_MAX_EXT] = {};
		_splitpath(pLooks->szPartFNs[PART_POS_HAIR_HELMET].c_str(), nullptr, szDir, szFName, szExt);

		std::string szFN = fmt::format("{}{}{:02}{}", szDir, szFName, m_InfoExt.iHair, szExt);
		PartSet(PART_POS_HAIR_HELMET, szFN, nullptr, nullptr);
	}
	else
	{
		m_Chr.PartSet(PART_POS_HAIR_HELMET, "");
		m_ChrInv.PartSet(PART_POS_HAIR_HELMET, "");
	}
}

void CPlayerMySelf::KnightsInfoSet(int iID, const std::string& szName, int iGrade, int iRank)
{
	CPlayerBase::KnightsInfoSet(iID, szName, iGrade, iRank);

	m_InfoExt.szKnights = szName;
	m_InfoExt.iKnightsGrade = iGrade;
	m_InfoExt.iKnightsRank = iRank;

	if (m_InfoExt.szKnights.empty())
	{
		delete m_pClanFont;
		m_pClanFont = nullptr;
		return;
	}

	if (m_pClanFont == nullptr)
	{
		std::string szFontID = fmt::format_text_resource(IDS_FONT_ID);

		m_pClanFont = new CDFont(szFontID, 12, D3DFONT_BOLD);
		m_pClanFont->InitDeviceObjects(s_lpD3DDev);
		m_pClanFont->RestoreDeviceObjects();
	}

	m_pClanFont->SetText(m_InfoExt.szKnights.c_str()); // 폰트에 텍스트 지정.
	m_pClanFont->SetFontColor(KNIGHTS_FONT_COLOR);
}

void CPlayerMySelf::SetSoundAndInitFont(uint32_t dwFontFlag)
{
	CPlayerBase::SetSoundAndInitFont();

	if (m_InfoExt.szKnights.empty())
	{
		delete m_pClanFont;
		m_pClanFont = nullptr;
		return;
	}

	if (m_pClanFont == nullptr)
	{
		std::string szFontID = fmt::format_text_resource(IDS_FONT_ID);

		m_pClanFont = new CDFont(szFontID, 12, D3DFONT_BOLD);
		m_pClanFont->InitDeviceObjects(s_lpD3DDev);
		m_pClanFont->RestoreDeviceObjects();
	}

	m_pClanFont->SetText(m_InfoExt.szKnights.c_str()); // 폰트에 텍스트 지정.
	m_pClanFont->SetFontColor(KNIGHTS_FONT_COLOR);
}

float CPlayerMySelf::MoveSpeedCalculationAndCheckCollision() // 속도를 구하고 그 속도로 충돌 체크를 한다. 리턴값이 0 이면 충돌이다..
{
	m_fMoveSpeedPerSec = MOVE_SPEED_WHEN_WALK;
	if(PSM_RUN == m_eStateMove) m_fMoveSpeedPerSec *= MOVE_DELTA_WHEN_RUNNING;
	else if(PSM_STOP == m_eStateMove) m_fMoveSpeedPerSec = 0.0f;
	else if(PSM_WALK_BACKWARD == m_eStateMove) m_fMoveSpeedPerSec *= -1.0f;
	m_fMoveSpeedPerSec *= m_fMoveDelta; // 이동 속도 변하기에 따라서... 곱해준다.

	if(m_bTempMoveTurbo) // 개발용 플래그...
	{
		m_fMoveSpeedPerSec *= 10.0f;
	}

	if(this->CheckCollision()) // 충돌 체크면..
	{
		if(CGameProcedure::s_pProcMain)
			CGameProcedure::s_pProcMain->CommandMove(MD_STOP, true);
		m_fMoveSpeedPerSec = 0;
	}

	// 방향을 구해서.. 기울기에 따라 다른 속도를 적용
/*	__Vector3 vDir = this->Direction();
	__Vector3 vPos = m_Chr.Pos(), vNormal(0,1,0);
	s_pTerrain->GetNormal(vPos.x, vPos.z, vNormal);
	vNormal.Normalize();
	vNormal.y	= 0.0f;
	float fM = vNormal.Magnitude();
	float fD = vNormal.Dot(vDir);
	if(fSpeed < 0) fD *= -1.0f;
//	if (fM > MAX_INCLINE_CLIMB && fD <= 0.0f )
//	{
//	}
	if(fD < 0) fSpeed *= 1.0f - (fM / 0.7071f); // 기울기에 따른 팩터 적용
*/	
	return m_fMoveSpeedPerSec;

}

void CPlayerMySelf::Stun(float fTime) // 일정한 시간동안 기절 시키기.
{
	m_bStun = true;				// 기절..
	m_fStunTime = fTime;		// 기절한 시간..

	CGameProcedure::s_pProcMain->CommandEnableAttackContinous(false, nullptr); // 공격 멈추고..
	CGameProcedure::s_pProcMain->CommandMove(MD_STOP, true); // 움직임을 멈추게 하고..
}

void CPlayerMySelf::StunRelease() // 기절 풀기..
{
	m_bStun = false;			// 기절..
	m_fStunTime = 0.0f;			// 기절한 시간..
}

void CPlayerMySelf::TargetOrPosMove()
{
	if (!m_bTargetOrPosMove)
		return;

	m_vTargetPos.y = m_Chr.Pos().y;

	if (m_iMoveTarget >= 0)
	{
		CPlayerNPC* pTarget = s_pOPMgr->CharacterGetByID(m_iMoveTarget, true);
		if(pTarget != nullptr)
		{
			m_vTargetPos = pTarget->Position();
		}
		else
		{
			this->ActionMove(PSM_STOP);
			CGameProcedure::s_pProcMain->MsgSend_Move(false, false); // 정지 패킷 보내기..
		}
	}

	__Vector3 vDir = m_vTargetPos - s_pPlayer->Position();
	vDir.y = 0.0f;
	vDir.Normalize();

	float fYaw = ::_Yaw2D(vDir.x, vDir.z);
	this->RotateTo(fYaw, true); // 방향을 돌리고

	if( m_iMoveTarget >= 0 )
	{
		CPlayerNPC* pTarget = s_pOPMgr->CharacterGetByID(m_iMoveTarget, true);

		float fDist = (pTarget->Position() - m_Chr.Pos()).Magnitude(); // 공격 거리를 구하고..
		float fDistLimit = this->AttackableDistance(pTarget);

		if(fDist < fDistLimit)
		{
			CGameProcedure::s_pProcMain->CommandEnableAttackContinous(true, pTarget);
			this->ActionMove(PSM_STOP);
			CGameProcedure::s_pProcMain->MsgSend_Move(false, false); // 정지 패킷 보내기..
		}
	}
	else
	{
		float fDist = (m_vTargetPos - Position()).Magnitude();
		if( fDist < 0.5f )
		{
			this->ActionMove(PSM_STOP);
			CGameProcedure::s_pProcMain->MsgSend_Move(false, false); // 정지 패킷 보내기..
		}
	}
}

bool CPlayerMySelf::HasWeaponEquipped() const
{
	// TODO: Handle transformations (should use the inventory in that case)
	if (IsItemClassWeapon(ItemClass_RightHand())
		|| IsItemClassWeapon(ItemClass_LeftHand()))
		return true;

	return false;
}
