// PlayerNPC.cpp: implementation of the CPlayerNPC class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PlayerNPC.h"
#include "GameProcMain.h"
#include "N3WorldManager.h"
#include "PlayerMySelf.h"

#include <N3Base/N3Shape.h>
#include <N3Base/N3ShapeMgr.h>
#include <N3Base/N3SndObj.h>

CPlayerNPC::CPlayerNPC()
{
	m_ePlayerType    = PLAYER_NPC;  // Player Type ... Base, NPC, OTher, MySelf
	m_vPosFromServer = m_Chr.Pos(); // 서버에게 받은 위치.
}

CPlayerNPC::~CPlayerNPC()
{
}

void CPlayerNPC::Tick()
{
	if (m_pShapeExtraRef) // 오브젝트이면..
	{
		CPlayerBase::Tick();
		return;
	}

	if (m_fTimeAfterDeath > 0)              // 죽어야하거나 죽은 넘이면...
	{
		if (m_fTimeAfterDeath > 3.0f)
			this->Action(PSA_DYING, false); // 5 초가 지나야 죽는다.
		CPlayerBase::Tick();                // 회전, 지정된 에니메이션 Tick 및 색깔 지정 처리.. 등등..
		return;
	}

	__Vector3 vPos = m_Chr.Pos();
	if (m_vPosFromServer.x != vPos.x || m_vPosFromServer.z != vPos.z) // 조금 더 가야한다.
	{
		if (m_fMoveSpeedPerSec == 0)
		{
			__Vector3 vT1(m_vPosFromServer.x, 0, m_vPosFromServer.z), vT2(vPos.x, 0, vPos.z);
			m_fMoveSpeedPerSec = (vT2 - vT1).Magnitude();
		}

		__Vector3 vOffset = m_vPosFromServer - vPos;
		vOffset.y         = 0;
		__Vector3 vDir    = vOffset;
		vDir.Normalize();                                                                           // 방향..

		float fSpeedAbsolute = (m_fMoveSpeedPerSec > 0) ? m_fMoveSpeedPerSec : -m_fMoveSpeedPerSec; // 속도 절대값
		float fDist          = vOffset.Magnitude();                                                 // 거리
		if (fDist < fSpeedAbsolute * CN3Base::s_fSecPerFrm)                                         // 움직이는 거리가 거의 다온거면..
		{
			vPos.x = m_vPosFromServer.x;                                                            // 위치를 고정해주고..
			vPos.z = m_vPosFromServer.z;                                                            // 위치를 고정해주고..
			//			m_fMoveSpeedPerSec = 0; // 움직이는 속도를 0으로!
			this->ActionMove(PSM_STOP); // 움직이는 모션 처리..
		}
		else
		{
			float fYaw = (m_fMoveSpeedPerSec < 0) ? ::_Yaw2D(-vDir.x, -vDir.z) : ::_Yaw2D(vDir.x, vDir.z); // 방향을 계산해서..
			this->RotateTo(fYaw, false);                                                                   // 진행 방향으로 돌리고..

			e_StateMove eMove = PSM_STOP;                                                                  // 움직임...
			// 플레이어면 걷는 속도가 기준 나머지는 덩치에 반비례..
			float fStandWalk  = ((PLAYER_OTHER == m_ePlayerType) ? (MOVE_SPEED_WHEN_WALK * 2.0f)
																 : (MOVE_SPEED_WHEN_WALK * m_Chr.Radius() * 2.0f));
			if (m_fMoveSpeedPerSec < 0)
				eMove = PSM_WALK_BACKWARD;                                 // 뒤로 걷기..
			else if (m_fMoveSpeedPerSec < fStandWalk)
				eMove = PSM_WALK;                                          // 앞으로 걷기..
			else
				eMove = PSM_RUN;                                           // if(fDN > 5.0f) // 다음 위치의 거리가 일정 이상이면 뛰어간다.
			this->ActionMove(eMove);                                       // 움직이는 모션 처리..

			vPos += vDir * (fSpeedAbsolute * s_fSecPerFrm);                // 이동..
		}

		float fYTerrain = ACT_WORLD->GetHeightWithTerrain(vPos.x, vPos.z); // 지면의 높이값..
		float fYMesh    = ACT_WORLD->GetHeightNearstPosWithShape(vPos);    // 충돌 체크 오브젝트의 높이값..
		if (fYMesh != -FLT_MAX && fYMesh > fYTerrain && fYMesh < m_fYNext + 1.0f)
			m_fYNext = fYMesh;                                             // 올라갈수 있는 오브젝트이고 높이값이 지면보다 높으면.
		else
			m_fYNext = fYTerrain;
		this->PositionSet(vPos, false);                                    // 위치 최종 적용..
	}

	// 공격 중이거나 스킬 사용중이면..
	if (PSA_ATTACK == m_eState || PSA_SPELLMAGIC == m_eState)
	{
		CPlayerBase* pTarget = this->TargetPointerCheck(false);
		CPlayerBase::ProcessAttack(pTarget); // 공격에 관한 루틴 처리.. 에니메이션 세팅과 충돌만 처리할뿐 패킷은 처리 안한다..
	}

	CPlayerBase::Tick();                     // 회전. 이동, 에니메이션 틱.. 상태 바뀜 등을 처리한다.
}

void CPlayerNPC::MoveTo(float fPosX, float fPosY, float fPosZ, float fSpeed, int iMoveMode)
{
	m_vPosFromServer.Set(fPosX, fPosY, fPosZ);
	if (m_pShapeExtraRef)
		return; // 오브젝트 형식이면 움직일수가 없다..

	// iMoveMode : 현재 움직이는 상태.. 0 -정지 1 움직임시작 2 - 1초마다 한번 연속움직임..
	if (iMoveMode == 0)
		return;

	m_fMoveSpeedPerSec = fSpeed;

	__Vector3 vPos     = m_Chr.Pos();
	vPos.y             = 0;

	__Vector3 vPosS(fPosX, 0, fPosZ);

	if (fSpeed)
		m_fMoveSpeedPerSec *= ((vPosS - vPos).Magnitude() / (fSpeed * PACKET_INTERVAL_MOVE))
							  * 0.85f; // 속도보간.. 동기화때문에 그런다.. 약간 줄여주는 이유는 멈칫하는걸 방지하기 위해서이다..
	else
		m_fMoveSpeedPerSec = ((vPosS - vPos).Magnitude() / (fSpeed * PACKET_INTERVAL_MOVE))
							 * 0.85f;  // 속도보간.. 동기화때문에 그런다.. 약간 줄여주는 이유는 멈칫하는걸 방지하기 위해서이다..
	if (fSpeed < 0)
		m_fMoveSpeedPerSec *= -1.0f;   // 뒤로 간다..
}

void CPlayerNPC::SetSoundAndInitFont(uint32_t dwFontFlag)
{
	CPlayerBase::SetSoundAndInitFont(dwFontFlag);

	if (s_pPlayer->m_InfoBase.iAuthority == AUTHORITY_MANAGER)
	{
		// NOTE: Type is the AI state (not the NPC's type as you'd expect), but it's not sent at all here,
		// and is rarely ever even sent officially, so we'll just placeholder it to 0 for now.
		std::string szInfo = fmt::format("ID({}) : TYPE({})", m_InfoBase.iID, 0);
		InfoStringSet(szInfo, D3DCOLOR_XRGB(0, 255, 0));
	}
}
