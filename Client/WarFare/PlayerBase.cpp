// CPlayerBase.cpp: implementation of the CPlayerBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlayerBase.h"
#include "N3WorldManager.h"
#include "N3FXMgr.h"

#include "text_resources.h"

#include <N3Base/DFont.h>
#include <N3Base/N3ShapeExtra.h>
#include <N3Base/N3SndObj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

static const __Vector3 s_vLightOffset = __Vector3(10.0f, 40.0f, 30.0f);
CN3SndObj*	CPlayerBase::m_pSnd_MyMove = nullptr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerBase::CPlayerBase()
{
	m_ePlayerType = PLAYER_BASE; // Player Type ... Base, NPC, OTher, MySelf

	m_Chr.PartAlloc(PART_POS_COUNT); // 기본적으로 파트와 플러그 세팅..
	m_Chr.PlugAlloc(PLUG_POS_COUNT);

	m_Chr.PosSet(__Vector3(0.0f, 0.0f, 0.0f));

	m_pLooksRef = nullptr;				// 기본적인 모습과 기본 정보 참조 테이블
	memset(m_pItemPlugBasics, 0, sizeof(m_pItemPlugBasics)); // 캐릭터에 붙은 무기들..
	memset(m_pItemPlugExts, 0, sizeof(m_pItemPlugExts));
	memset(m_pItemPartBasics, 0, sizeof(m_pItemPartBasics)); // 캐릭터에 붙은 갑옷들..
	memset(m_pItemPartExts, 0, sizeof(m_pItemPartExts)); // 캐릭터에 붙은 갑옷들..

	m_iIDTarget	= -1;
	m_iMagicAni = 0;
	m_bGuardSuccess	= false;	// 방어에 성공했는지에 대한 플래그..
	m_bVisible = true;			// 보이는지??

	m_iDroppedItemID				= 0;	// 죽은후 떨어트린 아이템

	m_Chr.m_nJointPartStarts[0]		= 1;	// 상체 - 에니메이션을 분리해 처리하는데... 조인트 인덱스이다.
	m_Chr.m_nJointPartEnds[0]		= 15;	// 상체

	m_Chr.m_nJointPartStarts[1]		= 16;	// 하체
	m_Chr.m_nJointPartEnds[1]		= 23;	// 하체

	m_cvDuration.a = m_cvDuration.r = m_cvDuration.g = m_cvDuration.b = 1.0f; // 지속 컬러 값
	m_fDurationColorTimeCur = 0; // 현재 시간..
	m_fDurationColorTime = 0; // 지속시간..

	m_fFlickeringFactor = 1.0f; // 깜박거림 알파 값 1.0 이면 깜박이지 않는다....
	m_fFlickeringTime = 0; // 깜박거림 시간..

	m_fTimeAfterDeath = 0; // 죽는 모션을 취하는 타이머 - 5초정도면 적당한가?? 그전에 공격을 받으면 바로 죽는다.

	m_eStateNext = m_eState = PSA_BASIC;
	m_eStateMove = PSM_STOP; // 첨에 정지..
	m_eStateDying = PSD_UNKNOWN; // 죽을때 방법
	m_fTimeDying = 0; // 죽는 모션 취할때 지난 시간..

	m_fRotRadianPerSec = D3DXToRadian(270.0f); // 초당 회전 라디안값
	m_fMoveSpeedPerSec = 0; // 초당 움직임 값.. 이값은 기본값이고 상태(걷기, 달리기, 뒤로, 저주등) 에 따라 가감해서 쓴다..
	m_fYawCur = 0; // 현재 회전값..
	m_fYawToReach = 0;

	m_fGravityCur = 0;			// 중력값..
	m_fYNext = 0;				// 오브젝트 혹은 지형의 충돌 체크에 따른 높이값..

	m_fScaleToSet = 1.0f;			// 점차 스케일 값변화..
	m_fScalePrev = 1.0f;

	m_pSnd_Move = nullptr;
	m_pSnd_Attack_0 = nullptr;
//	m_pSnd_Attack_1 = nullptr;
	m_pSnd_Struck_0 = nullptr;
//	m_pSnd_Struck_1 = nullptr;
	m_pSnd_Breathe_0 = nullptr;
//	m_pSnd_Breathe_1 = nullptr;
	m_pSnd_Blow = nullptr;
	m_bSoundAllSet = false;

	m_InfoBase.Init();				// 첨엔 종족값을 모른다..

	m_pShapeExtraRef = nullptr;					// 이 NPC 가 성문이나 집등 오브젝트의 형태이면 이 포인터를 세팅해서 쓴,다..

	m_fCastFreezeTime = 0.0f;
	m_fAttackDelta = 1.0f;			// 스킬이나 마법에 의해 변하는 공격 속도.. 1.0 이 기본이고 클수록 더 빨리 공격한다.
	m_fMoveDelta = 1.0f;			// 스킬이나 마법에 의해 변하는 이동 속도 1.0 이 기본이고 클수록 더 빨리 움직인다.

	m_vDirDying.Set(0,0,1); // 죽을때 밀리는 방향..

	// 그림자 초기화
//	By : Ecli666 ( On 2002-03-29 오후 4:23:36 )
/*
	m_pTexShadow = nullptr;
	m_pTexShadow = s_MngTex.Get("Chr\\Shadow_Character.tga"); 
	m_vShadows[0].Set(-0.7f, 0, 0.7f, 0, 0);
	m_vShadows[1].Set( 0.7f, 0, 0.7f, 1, 0);
	m_vShadows[2].Set( 0.7f, 0,-0.7f, 1, 1);
	m_vShadows[3].Set(-0.7f, 0,-0.7f, 0, 1);
*/
//	~(By Ecli666 On 2002-03-29 오후 4:23:36 )

	// 폰트 초기화... // 정보 표시용 폰트와 풍선용은 따로 생성한다..
	m_pIDFont = nullptr;
	m_pClanFont = nullptr;
	m_pInfoFont = nullptr;
	m_pBalloonFont = nullptr;
	m_fTimeBalloon = 0;

	m_bAnimationChanged = false;	// 큐에 넣은 에니메이션이 변하는 순간만 세팅된다..

	m_pvVertex[0].Set(-SHADOW_PLANE_SIZE, 0.0f,  SHADOW_PLANE_SIZE, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	m_pvVertex[1].Set( SHADOW_PLANE_SIZE, 0.0f,  SHADOW_PLANE_SIZE, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	m_pvVertex[2].Set( SHADOW_PLANE_SIZE, 0.0f, -SHADOW_PLANE_SIZE, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	m_pvVertex[3].Set(-SHADOW_PLANE_SIZE, 0.0f, -SHADOW_PLANE_SIZE, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

	uint16_t*		pIdx = m_pIndex;

	// 아랫면.
	*pIdx++ = 0;  *pIdx++ = 1;  *pIdx++ = 3;
	*pIdx++ = 2;  *pIdx++ = 3;  *pIdx++ = 1;

	m_N3Tex.Create(SHADOW_SIZE, SHADOW_SIZE, D3DFMT_A4R4G4B4, 0);	
}

CPlayerBase::~CPlayerBase()
{
//	By : Ecli666 ( On 2002-03-29 오후 4:24:14 )
//
//	s_MngTex.Delete(m_pTexShadow);

//	~(By Ecli666 On 2002-03-29 오후 4:24:14 )
	delete m_pClanFont; m_pClanFont = nullptr;
	delete m_pIDFont; m_pIDFont = nullptr;
	delete m_pInfoFont; m_pInfoFont = nullptr;
	delete m_pBalloonFont; m_pBalloonFont = nullptr;

	m_bSoundAllSet = false;
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Move);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Struck_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Struck_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Breathe_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Breathe_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Blow);

	if(m_pShapeExtraRef) m_pShapeExtraRef->m_bVisible = false; // 안보이게 한다..
}

void CPlayerBase::Release()
{
	m_AnimationDeque.clear();

	m_Chr.Release();
	m_Chr.PartAlloc(PART_POS_COUNT); // 기본적으로 파트와 플러그 세팅..
	m_Chr.PlugAlloc(PLUG_POS_COUNT);

	m_bSoundAllSet = false;
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Move);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Struck_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Struck_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Breathe_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Breathe_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Blow);

	m_pLooksRef = nullptr;				// 기본적인 모습과 기본 정보 참조 테이블
	memset(m_pItemPlugBasics, 0, sizeof(m_pItemPlugBasics)); // 캐릭터에 붙은 무기들..
	memset(m_pItemPlugExts, 0, sizeof(m_pItemPlugExts));
	memset(m_pItemPartBasics, 0, sizeof(m_pItemPartBasics)); // 캐릭터에 붙은 갑옷들..
	memset(m_pItemPartExts, 0, sizeof(m_pItemPartExts)); // 캐릭터에 붙은 갑옷들..

	m_iIDTarget	= -1;
	m_bGuardSuccess = false; // 방어에 성공했는지에 대한 플래그..
	m_bVisible = true;			// 보이는지??

	m_cvDuration.a = m_cvDuration.r = m_cvDuration.g = m_cvDuration.b = 1.0f; // 지속 컬러 값
	m_fDurationColorTimeCur = 0; // 현재 시간..
	m_fDurationColorTime = 0; // 지속시간..

	m_fFlickeringFactor = 1.0f; // 깜박거림 알파 값 1.0 이면 깜박이지 않는다....
	m_fFlickeringTime = 0; // 깜박거림 시간..

	m_fTimeAfterDeath = 0; // 죽는 모션을 취하는 타이머 - 5초정도면 적당한가?? 그전에 공격을 받으면 바로 죽는다.

	m_eStateNext = m_eState = PSA_BASIC;
	m_eStateMove = PSM_STOP; // 첨에 정지..
	m_eStateDying = PSD_UNKNOWN; // 죽을때 방법
	m_fTimeDying = 0; // 죽는 모션 취할때 지난 시간..

	m_fRotRadianPerSec = D3DXToRadian(270.0f); // 초당 회전 라디안값
	m_fMoveSpeedPerSec = 0; // 초당 움직임 값.. 이값은 기본값이고 상태(걷기, 달리기, 뒤로, 저주등) 에 따라 가감해서 쓴다..
	m_fYawCur = 0; // 현재 회전값..
	m_fYawToReach = 0;

	m_fGravityCur = 0;			// 중력값..
	m_fYNext = 0;				// 오브젝트 혹은 지형의 충돌 체크에 따른 높이값..

	m_fScaleToSet = 1.0f;			// 점차 스케일 값변화..
	m_fScalePrev = 1.0f;

	m_InfoBase.Init();				// 첨엔 종족값을 모른다..

	m_pShapeExtraRef = nullptr;					// 이 NPC 가 성문이나 집등 오브젝트의 형태이면 이 포인터를 세팅해서 쓴,다..

	m_fCastFreezeTime = 0.0f;
	m_fAttackDelta = 1.0f;		// 스킬이나 마법에 의해 변하는 공격 속도.. 1.0 이 기본이고 클수록 더 빨리 공격한다.
	m_fMoveDelta = 1.0f;			// 스킬이나 마법에 의해 변하는 이동 속도 1.0 이 기본이고 클수록 더 빨리 움직인다.
	m_vDirDying.Set(0,0,1); // 죽을때 밀리는 방향..

	m_bAnimationChanged = false;	// 큐에 넣은 에니메이션이 변하는 순간만 세팅된다..

	if(m_pShapeExtraRef) m_pShapeExtraRef->m_bVisible = false; // 안보이게 한다..

	CGameBase::Release();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayerBase::SetSoundAndInitFont(uint32_t dwFontFlag)
{
	if (m_pLooksRef == nullptr)
		return;

	if (m_bSoundAllSet)
		return;

	m_bSoundAllSet = true;
	
	if (m_pSnd_Move == nullptr)
		m_pSnd_Move = s_SndMgr.CreateObj(m_pLooksRef->iSndID_Move);

	if (m_pSnd_Struck_0 == nullptr)
		m_pSnd_Struck_0 = s_SndMgr.CreateObj(m_pLooksRef->iSndID_Struck0);

	if (m_pSnd_Breathe_0 == nullptr)
		m_pSnd_Breathe_0 = s_SndMgr.CreateObj(m_pLooksRef->iSndID_Breathe0);

	if (m_pSnd_Blow == nullptr)
		m_pSnd_Blow = s_SndMgr.CreateObj(100);

	// 무기에 해당되는 사운드...
	__TABLE_ITEM_BASIC* pItemBasic = m_pItemPlugBasics[PLUG_POS_RIGHTHAND];
	if (pItemBasic == nullptr)
		pItemBasic = m_pItemPlugBasics[PLUG_POS_LEFTHAND];

	SetSoundPlug(pItemBasic);

	// Font 초기화..
	if (m_pIDFont == nullptr) 
	{
		std::string szFontID = fmt::format_text_resource(IDS_FONT_ID);

		m_pIDFont = new CDFont(szFontID, 12, dwFontFlag);//D3DFONT_BOLD);
		m_pIDFont->InitDeviceObjects( s_lpD3DDev );
		m_pIDFont->RestoreDeviceObjects();

		m_pIDFont->SetText(m_InfoBase.szID); // 폰트에 텍스트 지정.
		m_pIDFont->SetFontColor(m_InfoBase.crID);
	}
}

void CPlayerBase::SetSoundPlug(__TABLE_ITEM_BASIC* pItemBasic)
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_1);
	
	if(pItemBasic) //사운드 넣어라..
	{
		m_pSnd_Attack_0 = CN3Base::s_SndMgr.CreateObj(pItemBasic->dwSoundID0);
//		m_pSnd_Attack_1 = CN3Base::s_SndMgr.CreateObj(pItemBasic->dwSoundID1); // 맞는 소리..
	}
	else
	{
		m_pSnd_Attack_0 = CN3Base::s_SndMgr.CreateObj(m_pLooksRef->iSndID_Attack0); // 휘두르는 소리.
//		m_pSnd_Attack_1 = CN3Base::s_SndMgr.CreateObj(m_pLooksRef->iSndID_Attack1); // 휘두르는 소리.
	}
}

void CPlayerBase::ReleaseSoundAndFont()
{
	if(false == m_bSoundAllSet) return;
	m_bSoundAllSet = false;

	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Move);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Attack_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Struck_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Struck_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Breathe_0);
//	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Breathe_1);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Blow);

	delete m_pIDFont; m_pIDFont = nullptr;
	delete m_pInfoFont; m_pInfoFont = nullptr;
	delete m_pClanFont; m_pClanFont = nullptr;
	delete m_pBalloonFont; m_pBalloonFont = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

void CPlayerBase::InfoStringSet(const std::string& szInfo, D3DCOLOR crFont)
{
	if (szInfo.empty())
	{
		delete m_pInfoFont;
		m_pInfoFont = nullptr;
		return;
	}

	if (m_pInfoFont == nullptr)
	{
		std::string szFontInfo = fmt::format_text_resource(IDS_FONT_INFO);

		m_pInfoFont = new CDFont(szFontInfo, 12, D3DFONT_BOLD);
		m_pInfoFont->InitDeviceObjects(s_lpD3DDev);
		m_pInfoFont->RestoreDeviceObjects();
	}
	m_pInfoFont->SetText(szInfo, D3DFONT_BOLD); // 폰트에 텍스트 지정.
	m_pInfoFont->SetFontColor(crFont);
}

void CPlayerBase::BalloonStringSet(const std::string& szBalloon, D3DCOLOR crFont)
{
	if (szBalloon.empty())
	{
		delete m_pBalloonFont;
		m_pBalloonFont = nullptr;
		m_fTimeBalloon = 0; // 풍선말 표시시간..
		return;
	}

	if (m_pBalloonFont == nullptr)
	{
		std::string szFontBalloon = fmt::format_text_resource(IDS_FONT_BALLOON);

		m_pBalloonFont = new CDFont(szFontBalloon, 12);
		m_pBalloonFont->InitDeviceObjects(s_lpD3DDev);
		m_pBalloonFont->RestoreDeviceObjects();
	}

	m_fTimeBalloon = szBalloon.size() * 0.2f;

	m_pBalloonFont->SetText(szBalloon.c_str(), 0); // 폰트에 텍스트 지정.
	m_pBalloonFont->SetFontColor(crFont);
}

void CPlayerBase::IDSet(int iID, const std::string& szID, D3DCOLOR crID)
{
	m_InfoBase.iID = iID;
	m_InfoBase.szID = szID; // 이름으로 ID 를 대체한다.
	m_InfoBase.crID = crID;

#ifdef _DEBUG
	m_Chr.m_szName = szID; // 디버깅을 위해서 이름을 넣어논다.. 그래야 구별 가능하다..
#endif
}

void CPlayerBase::KnightsInfoSet(int iID, const std::string& szName, int iGrade, int iRank)
{
	std::string szPlug;
	if (iGrade > 0 && iGrade <= 5)
	{
		// 종족과 등급으로 플러그 이름을 만든다..
		szPlug = fmt::format("Item\\ClanAddOn_{:03}_{}.n3cplug",
			static_cast<int>(m_InfoBase.eRace), iGrade);
	}

	m_InfoBase.iKnightsID = iID;

	CN3CPlugBase* pPlug = PlugSet(PLUG_POS_KNIGHTS_GRADE, szPlug, nullptr, nullptr);
	if (pPlug == nullptr)
		return;

	__TABLE_FX* pFXClanRank = s_pTbl_FXSource.Find(FXID_CLAN_RANK_1);

	std::string szFXMain, szFXTail;
	if (pFXClanRank != nullptr
		&& iRank <= 5
		&& iRank >= 1)
		szFXMain = pFXClanRank->szFN;

	static_cast<CN3CPlug*>(pPlug)->InitFX(szFXMain, szFXTail, 0xffffffff);
}

/*
void CPlayerBase::RenderShadow()
{
//	By : Ecli666 ( On 2002-03-29 오후 4:22:59 )
/*
	LPDIRECT3DTEXTURE8 lpTex = nullptr;
	if(m_pTexShadow) lpTex = m_pTexShadow->Get();

	__AlphaPrimitive* pAP = s_AlphaMgr.Add();
	if(pAP)
	{
		pAP->bUseVB				= FALSE;
		pAP->dwBlendDest		= D3DBLEND_INVSRCALPHA;
		pAP->dwBlendSrc			= D3DBLEND_SRCALPHA;
		pAP->dwFVF				= FVF_XYZT1;
		pAP->nPrimitiveCount	= 2;
		pAP->ePrimitiveType		= D3DPT_TRIANGLEFAN;
		pAP->dwPrimitiveSize	= sizeof(__VertexXyzT1);
		pAP->fCameraDistance	= (s_CameraData.vEye - m_Chr.m_Matrix.Pos()).Magnitude();
		pAP->lpTex				= lpTex;
		pAP->nRenderFlags		= RF_NOTZWRITE;
		pAP->nVertexCount		= 4;
	//	pAP->MtxWorld			= &m_Matrix;
		pAP->pVertices			= m_vShadows;
		pAP->pwIndices			= nullptr;

		pAP->MtxWorld.Identity();
		pAP->MtxWorld.PosSet(m_Chr.m_Matrix.Pos());
		pAP->MtxWorld._42 = 0.05f;
	}

	for(int i = 0; i < 4; i++)
		m_vShadows[i].y = s_pTerrain->GetHeight(pAP->MtxWorld._41 + m_vShadows[i].x, pAP->MtxWorld._43 + m_vShadows[i].z);

	return; // 렌더링 안하지롱.
*/

//	~(By Ecli666 On 2002-03-29 오후 4:22:59 )

/*
	// 그림자 렌더링.
	// backup
	uint32_t dwAlpha;
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);

	// render state 세팅
	if(FALSE == dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);

	// texture state 세팅 (alpha)
	LPDIRECT3DTEXTURE8 lpTex = m_pTexShadow->Get();
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	s_lpD3DDev->SetTexture(0, lpTex);
	
	static __Matrix44 mtx;
	mtx.Identity();
	mtx.PosSet(m_Chr.m_Matrix.Pos());
	mtx._41 += 0.1f;
	mtx._42 = 0.05f;
	mtx._43 -= 0.1f;
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtx);
	s_lpD3DDev->SetFVF(FVF_XYZT1);

	for(int i = 0; i < 4; i++)
		m_vShadows[i].y = s_pTerrain->GetHeight(mtx._41 + m_vShadows[i].x, mtx._43 + m_vShadows[i].z);
	
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_vShadows, sizeof(__VertexXyzT1));

	// restore
	if(FALSE == dwAlpha) s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);

}*/

void CPlayerBase::RenderChrInRect(CN3Chr* pChr, const RECT& Rect)
{
	if (nullptr == pChr) return;

	// 2002년 2월 15일 Dino 재수정..
	// viewport 영역을 0보다 작은곳에서부터 (즉 front buffer의 영역을 벗어나게) 지정할 수 없기 때문에 계산이 복잡해졌다.

	RECT rcViewport;	// viewport 영역을 다시 계산한다. (front buffer의 크기를 벗어나지 않게..)
	if ( Rect.left < 0 ) rcViewport.left = 0;
	else rcViewport.left = Rect.left;
	if ( Rect.top < 0 ) rcViewport.top = 0;
	else rcViewport.top = Rect.top;
	if ( Rect.right > (int)(s_CameraData.vp.X + s_CameraData.vp.Width)) rcViewport.right = s_CameraData.vp.X + s_CameraData.vp.Width;
	else rcViewport.right = Rect.right;
	if ( Rect.bottom > (int)(s_CameraData.vp.Y + s_CameraData.vp.Height)) rcViewport.bottom = s_CameraData.vp.Y + s_CameraData.vp.Height;
	else rcViewport.bottom = Rect.bottom;

	// set viewport
	D3DVIEWPORT9 vp;
	vp.X = rcViewport.left;
	vp.Y = rcViewport.top;
	vp.Width = rcViewport.right - rcViewport.left;
	vp.Height = rcViewport.bottom - rcViewport.top;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	if (vp.Width <= 0 ||
		vp.Height <= 0 || 
		vp.Width > s_CameraData.vp.Width || 
		vp.Height > s_CameraData.vp.Height) return;	// front buffer보다 Width또는 Height가 크면 그리지 않는다.

	s_lpD3DDev->SetViewport(&vp);

	// set matrix
	__Matrix44 mtxProj, mtxView;
	float fChrHeight = pChr->Height()+0.2f;	// 캐릭터의 키에 20cm정도 더한값
	float fVCenter = fChrHeight*0.5f;
	int iWidth = Rect.right - Rect.left;
	int iHeight = Rect.bottom - Rect.top;
	float fViewVolumeHeight = fChrHeight * vp.Height / iHeight;	// 캐릭터의 키(클리핑 될 경우 클리핑 되는 비율에 맞게 좁혀준다.)
	float fViewVolumeWidth = fChrHeight * vp.Width / iHeight;	// 가로는 pRect의 가로 세로 비율에 맞게 (클리핑 될 경우 클리핑 되는 비율에 맞게 좁혀준다.)
																// 원래는 이거 : fChrHeight * iWidth / iHeight * vp.Width / iWidth;
	D3DXMatrixOrthoLH(&mtxProj, fViewVolumeWidth, fViewVolumeHeight, 0, 20);
	
	float fCameraMoveX = ((fChrHeight*iWidth/iHeight)-fViewVolumeWidth)/2.0f;	// 클리핑에 따른 카메라 이동 수치
	float fCameraMoveY = (fChrHeight-fViewVolumeHeight)/2.0f;
	if (rcViewport.left != Rect.left) fCameraMoveX = -fCameraMoveX;		// 왼쪽 영역이 짤리게 그려야 하므로 카메라를 오른쪽(카메라가 -Z축을 바라보기 때문에 카메라의 오른쪽은 -X쪽이다.)으로 이동
	if (rcViewport.top != Rect.top) fCameraMoveY = -fCameraMoveY;			// 위쪽 영역이 짤리게 그려야 하므로 카메라를 아래쪽으로 이동

//	D3DXMatrixLookAtLH( &mtxView, &D3DXVECTOR3( 0.0f + fCameraMoveX, fVCenter+2.0f + fCameraMoveY, 10.0f ),	// 여기서 View matrix는 카메라 각도와 상관있다. 거리는 원근에 아무 영향을 미치지 않는다.
//								  &D3DXVECTOR3( 0.0f + fCameraMoveX, fVCenter + fCameraMoveY, 0.0f ),	// fVCenter: 캐릭터 키의 중간을 바라보기
//								  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
	const __Vector3& vChrPos = pChr->Pos();

	// 여기서 View matrix는 카메라 각도와 상관있다. 거리는 원근에 아무 영향을 미치지 않는다.
	// fVCenter: 캐릭터 키의 중간을 바라보기
	const __Vector3 vEye(
		vChrPos.x + fCameraMoveX,
		vChrPos.y + fVCenter + 2.0f + fCameraMoveY,
		vChrPos.z + 10.0f);

	const __Vector3 vAt(
		vChrPos.x + fCameraMoveX,
		vChrPos.y + fVCenter + fCameraMoveY,
		vChrPos.z + 0.0f);

	const __Vector3 vUp(
		0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&mtxView, &vEye, &vAt, &vUp);
	s_lpD3DDev->SetTransform( D3DTS_VIEW, &mtxView );
	s_lpD3DDev->SetTransform( D3DTS_PROJECTION, &mtxProj);

	// backup render state
	DWORD dwFog, dwZEnable;//, dwLighting;
	s_lpD3DDev->GetRenderState( D3DRS_ZENABLE, &dwZEnable );
//	s_lpD3DDev->GetRenderState( D3DRS_LIGHTING, &dwLighting );	// lighting은 외부에서 조정할 수 있게 하자.
	s_lpD3DDev->GetRenderState( D3DRS_FOGENABLE , &dwFog );

	// set render state
	if (D3DZB_TRUE != dwZEnable) s_lpD3DDev->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE);
//	if (FALSE != dwLighting) s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, FALSE);
	if (FALSE != dwFog) s_lpD3DDev->SetRenderState( D3DRS_FOGENABLE, FALSE);

	// render
	D3DRECT rc = { Rect.left, Rect.top, Rect.right, Rect.bottom };
	s_lpD3DDev->Clear(1, &rc, D3DCLEAR_ZBUFFER, 0, 1.0f, 0); // Z Buffer Clear

	int iLODPrev = CN3Chr::LODDelta();
	CN3Chr::LODDeltaSet(0);
	pChr->m_nLOD = 1;	// LOD를 0으로 만든다.(최대한 디테일하게..)
	pChr->Render();
	CN3Chr::LODDeltaSet(iLODPrev);

	// restore
	if (D3DZB_TRUE != dwZEnable) s_lpD3DDev->SetRenderState( D3DRS_ZENABLE, dwZEnable);
//	if (FALSE != dwLighting) s_lpD3DDev->SetRenderState( D3DRS_LIGHTING, dwLighting);
	if (FALSE != dwFog) s_lpD3DDev->SetRenderState( D3DRS_FOGENABLE, dwFog);
    s_lpD3DDev->SetTransform( D3DTS_PROJECTION, &(s_CameraData.mtxProjection));
	s_lpD3DDev->SetTransform( D3DTS_VIEW, &(s_CameraData.mtxView) );
	s_lpD3DDev->SetViewport(&(s_CameraData.vp));
}

void CPlayerBase::DurationColorSet(const _D3DCOLORVALUE &color, float fDurationTime)
{
	m_fDurationColorTime = fDurationTime;
	m_fDurationColorTimeCur = 0;
	m_cvDuration = color;
}

void CPlayerBase::FlickerFactorSet(float fAlpha)
{
	if(fAlpha < 0.1f) fAlpha = 0.1f;
	if(fAlpha > 1.0f) fAlpha = 1.0f;

	m_fFlickeringFactor = fAlpha; // 깜박거림 알파 값 1.0 이면 깜박이지 않는다....
	m_fFlickeringTime = 0; // 깜박거림 시간..
}

void CPlayerBase::RotateTo(float fYaw, bool bImmediately)
{
	int iLot = (int)(fYaw/__PI2);
	if(iLot) fYaw -= iLot * __PI2; // 0 ~ 360 도 사이로 맞추고..

	iLot = (int)(fYaw/__PI);
	if(iLot) fYaw -= iLot * __PI2; // -180 ~ 180 사이로 맞춘다..

	m_fYawToReach = fYaw;
	if(bImmediately) m_fYawCur = fYaw; // 바로 돌려야 하면..
}

void CPlayerBase::RotateTo(CPlayerBase* pOther)
{
	if(nullptr == pOther || pOther == this) return;

	__Vector3 vDir = pOther->Position() - m_Chr.Pos();
	vDir.y = 0.0f;
	vDir.Normalize();

	float fYaw = ::_Yaw2D(vDir.x, vDir.z);
	this->RotateTo(fYaw, false); // 방향을 돌리고

}

void CPlayerBase::TickYaw()
{
	if(m_fYawCur != m_fYawToReach && this->IsAlive()) // 회전을 해야 한다면.. 살아 있는 넘만...
	{
		float fYawDiff = m_fYawToReach - m_fYawCur; // 회전값 차이.
		float fYawDelta = m_fRotRadianPerSec * s_fSecPerFrm; // 회전할 양
		if(std::abs(fYawDiff) <= fYawDelta)
		{
			m_fYawCur = m_fYawToReach; // 회전할 양이 작으면.. 바로 세팅
		}
		else
		{
			if(fYawDiff > 0)
			{
				if(fYawDiff < __PI) fYawDelta *= 1.0f;
				else if(fYawDiff >= __PI) fYawDelta *= -1.0f;
			}
			else
			{
				if(fYawDiff > -__PI) fYawDelta *= -1.0f;
				else if(fYawDiff <= -__PI) fYawDelta *= 1.0f;
			}

			m_fYawCur += fYawDelta; // 회전..

			if(std::abs(m_fYawCur) > __PI)
			{
				int iLot = (int)(m_fYawCur/__PI);
				if(iLot) m_fYawCur -= iLot * __PI2; // 0 ~ 360 도 사이로 맞추고..

				iLot = (int)(m_fYawCur/__PI);
				if(iLot) m_fYawCur -= iLot * __PI2; // -180 ~ 180 사이로 맞춘다..
			}
		}
	}

	// 회전값 적용..
	__Quaternion qtRot;
	qtRot.RotationAxis(0,1,0,m_fYawCur); // 회전 ....
	m_Chr.RotSet(qtRot);
}

void CPlayerBase::TickAnimation()
{
	// 걷고 뛰고, 에니메이션등... 속도 적용
	float fAniSpeedDelta = 1.0f;
	if(PSM_STOP != m_eStateMove) fAniSpeedDelta = m_fMoveDelta; // 이동중이면 스피드 적용..
	else if(PSA_ATTACK == m_eState) fAniSpeedDelta = m_fAttackDelta; // 공격중이면 공격 스피드 적용..
	__ASSERT(fAniSpeedDelta >= 0.1f && fAniSpeedDelta < 10.0f, "Invalid Animation Speed Delta!!!");
	m_Chr.AniSpeedDeltaSet(fAniSpeedDelta); // 에니메이션 스피드 실제 적용..
	m_Chr.Tick(); // 에니메이션 틱..

	m_bAnimationChanged = false;	// 큐에 넣은 에니메이션이 변하는 순간만 세팅된다..
	if(	m_Chr.IsAnimEnd() )// 에니메이션이 끝나면..
	{
		m_bAnimationChanged = true;	// 큐에 넣은 에니메이션이 변하는 순간만 세팅된다..
		if(m_AnimationDeque.empty()) // 에니메이션 데크가 비어 있으면..
		{
			CPlayerBase* pTarget = this->CharacterGetByID(m_iIDTarget, true);
			this->Action(m_eStateNext, true, pTarget); // 상태 돌리기..
		}
		else // 에니메이션 데크가 비어 있지 않고 시킬 동작이 있으면..
		{
			e_Ani eAniToSet = m_AnimationDeque[0]; // 데크에서 하나 빼오고..
			m_AnimationDeque.pop_front();
			m_Chr.AniCurSet(eAniToSet);
//			TRACE("      Animation : %d\n", eAniToSet);
		}
	}
}

void CPlayerBase::TickDurationColor()
{
	if(m_fDurationColorTime <= 0) return;

	if(m_fDurationColorTimeCur > m_fDurationColorTime) 
	{
		m_fDurationColorTime = 0;
		m_fDurationColorTimeCur = 0;

		CN3CPart* pPart = nullptr;
		int iPC = m_Chr.m_Parts.size();
		for(int i = 0; i < iPC; i++)
		{
			pPart = m_Chr.m_Parts[i];
			pPart->m_Mtl = pPart->m_MtlOrg; // 원래 색대로 돌린다.
		}
	}
	else
	{
		float fD = m_fDurationColorTimeCur / m_fDurationColorTime;
		CN3CPart* pPart = nullptr;
		int iPC = m_Chr.m_Parts.size();
		for(int i = 0; i < iPC; i++)
		{
			pPart = m_Chr.m_Parts[i];

//			pPart->m_Mtl.Ambient.a = pPart->m_MtlOrg.Ambient.a * fD + m_cvDuration.a * (1.0f - fD);
			pPart->m_Mtl.Ambient.r = pPart->m_MtlOrg.Ambient.r * fD + m_cvDuration.r * (1.0f - fD);
			pPart->m_Mtl.Ambient.g = pPart->m_MtlOrg.Ambient.g * fD + m_cvDuration.g * (1.0f - fD);
			pPart->m_Mtl.Ambient.b = pPart->m_MtlOrg.Ambient.b * fD + m_cvDuration.b * (1.0f - fD);

//			pPart->m_Mtl.Diffuse.a = pPart->m_MtlOrg.Diffuse.a * fD + m_cvDuration.a * (1.0f - fD);
			pPart->m_Mtl.Diffuse.r = pPart->m_MtlOrg.Diffuse.r * fD + m_cvDuration.r * (1.0f - fD);
			pPart->m_Mtl.Diffuse.g = pPart->m_MtlOrg.Diffuse.g * fD + m_cvDuration.g * (1.0f - fD);
			pPart->m_Mtl.Diffuse.b = pPart->m_MtlOrg.Diffuse.b * fD + m_cvDuration.b * (1.0f - fD);
		}
	}

	m_fDurationColorTimeCur += s_fSecPerFrm;
}

void CPlayerBase::TickSound()
{
	__Vector3 vPos = this->Position();

	if(PSA_ATTACK == m_eState) // 공격 일때..
	{
//		if(m_pSnd_Attack_0 && m_Chr.NeedPlaySound0())
//			m_pSnd_Attack_0->Play(&vPos); // 공격 1 중일때..
//		if(m_pSnd_Attack_1 && m_Chr.NeedPlaySound1())
//			m_pSnd_Attack_1->Play(&vPos); // 공격 2 중일때..
		if(m_pSnd_Attack_0 && (m_Chr.NeedPlaySound0() || m_Chr.NeedPlaySound1()))
			m_pSnd_Attack_0->Play(&vPos); // 공격 1 중일때..
//		if(m_pSnd_Attack_1 && m_Chr.NeedPlaySound1())
//			m_pSnd_Attack_1->Play(&vPos); // 공격 2 중일때..
	}
	
	if(PSM_STOP == m_eStateMove)
	{
		if(PSA_DYING == m_eState && m_Chr.NeedPlaySound0())
		{
			int DeadSoundID = (rand()%2) ? m_pLooksRef->iSndID_Dead0 : m_pLooksRef->iSndID_Dead1;
			CN3Base::s_SndMgr.PlayOnceAndRelease(DeadSoundID, &vPos);
		}
		else if(PSA_BASIC == m_eState && m_Chr.NeedPlaySound0())
		{
//			CN3SndObj* pSndBreath = (rand()%2) ? m_pSnd_Breathe_0 : m_pSnd_Breathe_1;
//			if(pSndBreath) 
//				pSndBreath->Play(&vPos);
			if(m_pSnd_Breathe_0)
				m_pSnd_Breathe_0->Play(&vPos);
		}		
	}
	else if(PSM_WALK == m_eStateMove ||
			PSM_RUN == m_eStateMove ||
			PSM_WALK_BACKWARD == m_eStateMove )
	{
		if(m_pSnd_Move && m_pSnd_Move != m_pSnd_MyMove && m_Chr.NeedPlaySound0()) 
			m_pSnd_Move->Play(&vPos);
		if(m_pSnd_Move && m_pSnd_Move != m_pSnd_MyMove && m_Chr.NeedPlaySound1()) 
			m_pSnd_Move->Play(&vPos);
	}
}

void CPlayerBase::Tick()  // 회전, 지정된 에니메이션 Tick 및 색깔 지정 처리.. 등등..
{
	if(m_pShapeExtraRef) // 오브젝트이면..
	{
		m_pShapeExtraRef->Tick(FRAME_SELFPLAY);
		return;
	}

	// 중력값 적용.
	__Vector3 vPos = this->Position();
	if(vPos.y > m_fYNext)
	{
		m_fGravityCur += 9.8f * CN3Base::s_fSecPerFrm;
		vPos.y -= m_fGravityCur;
		this->PositionSet(vPos, false);
	}

	/*
	// TEMP search "player position" to get here
	static int loopCount = 0;
	if(loopCount==0) {
		//vPos.x = vPos.x+5.0f;
		//vPos.z = vPos.z + 5.0f;
		//this->PositionSet(vPos, false);

		// NOTE: old karus_start
		vPos.x = 358.0f;
		vPos.y = 120.0f;
		vPos.z = 1610.0f;
		this->PositionSet(vPos, false);

		// NOTE: old/new moradon
		//vPos.x = 361.278503f;
		//vPos.y = 2.822370f;
		//vPos.z = 137.339859f;
		//this->PositionSet(vPos, false);

		loopCount++;
	}
	// TEMP
	*/

	if(vPos.y < m_fYNext)
	{
		vPos.y = m_fYNext;
		this->PositionSet(vPos, false);
		m_fGravityCur = 0;
	}

	// 점차 커지는 스케일 적용
	if(m_fScaleToSet != m_fScalePrev)
	{
		float fScale = m_Chr.Scale().y;
		if(m_fScaleToSet > m_fScalePrev) // 커져야 한다..
		{
			fScale += (m_fScaleToSet - m_fScalePrev) * s_fSecPerFrm;
			if(fScale > m_fScaleToSet)
			{
				m_fScalePrev = fScale = m_fScaleToSet;
			}
		}
		else // 작아져야 한다면..
		{
			fScale -= (m_fScalePrev - m_fScaleToSet) * s_fSecPerFrm;
			if(fScale < m_fScaleToSet)
			{
				m_fScalePrev = fScale = m_fScaleToSet;
			}
		}
		m_Chr.ScaleSet(fScale, fScale, fScale);
	}

	this->TickYaw();
	this->TickAnimation();
	this->TickDurationColor();
	this->TickSound();

	// 죽는 것 처리..
	if(m_fTimeAfterDeath > 0) m_fTimeAfterDeath += s_fSecPerFrm; // 죽는 모션을 취하는 타이머 - 5초정도면 적당한가?? 그전에 공격을 받으면 바로 죽는다.
	if(PSA_DYING == m_eState) 
	{
		if(PSD_DISJOINT == m_eStateDying || PSD_KNOCK_DOWN == m_eStateDying) // 뒤로 밀린다..
		{
			float fAD = 0;
			if(m_fTimeDying <= 0.2f) fAD = 5.0f;
			else if(m_fTimeDying < 0.4f) fAD = 5.0f * (0.4f - m_fTimeDying);

			if(fAD > 0) 
			{
				float fDelta = (fAD * (0.3f + 0.7f / m_Chr.Radius()));
				vPos -= m_vDirDying * (fDelta * s_fSecPerFrm); // 덩치에 반비례하게 밀린다..
				vPos.y = ACT_WORLD->GetHeightWithTerrain(vPos.x, vPos.z);
				m_Chr.PosSet(vPos);
			}
		}
		else
		{
		}

		m_fTimeDying += s_fSecPerFrm; // 죽는 동작을 취하는 시간
	}

	// 풍선말 처리..
	if(m_fTimeBalloon > 0)
	{
		m_fTimeBalloon -= s_fSecPerFrm;
		if(m_fTimeBalloon < 0)
		{
			m_fTimeBalloon = 0;
			this->BalloonStringSet("", 0); // 시간 榮?!!
		}
	}
}

void CPlayerBase::Render(float fSunAngle)
{
	if(m_Chr.m_nLOD < 0 || m_Chr.m_nLOD >= MAX_CHR_LOD) return;

#ifdef _DEBUG
	if(m_pShapeExtraRef) // 오브젝트 형식이면...
	{
		m_pShapeExtraRef->RenderCollisionMesh();
		return;
	}
#endif

	
	
	
	
	
	float fFactorToApply = 1.0f;
	if(m_fTimeAfterDeath > TIME_CORPSE_REMAIN - TIME_CORPSE_REMOVE) // 투명하게 만든다..
		fFactorToApply = (TIME_CORPSE_REMAIN - m_fTimeAfterDeath) / TIME_CORPSE_REMOVE;
	else if(m_fFlickeringFactor != 1.0f)
	{
		m_fFlickeringTime += s_fSecPerFrm;
		if(m_fFlickeringTime < 0.5f)
			fFactorToApply = m_fFlickeringFactor;
		else if(m_fFlickeringTime < 1.0f)
			fFactorToApply = m_fFlickeringFactor * 0.5f;
		else
		{
			m_fFlickeringTime = 0;
			fFactorToApply = m_fFlickeringFactor;
		}
	}


	if(fFactorToApply != 0) // 투명도가 적용되야 한다면..
	{
		DWORD dwAlphaBlend, dwAlphaOP, dwAlphaArg1, dwTexFactor, dwSrcBlend, dwDestBlend; // , dwZEnable;
		
		// backup state
		s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
		s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
		s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
//			s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
		s_lpD3DDev->GetRenderState(D3DRS_TEXTUREFACTOR, &dwTexFactor);	// alpha factor 설정
		s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOP);
		s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg1);

		DWORD dwFactorToApply = ((DWORD)(255.0f * fFactorToApply)) << 24; // 투명도 계산..

		// render state 세팅
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//			s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
		s_lpD3DDev->SetRenderState(D3DRS_TEXTUREFACTOR, dwFactorToApply);	// alpha factor 설정
		
		// texture state 세팅(alpha)
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);

		m_Chr.Render();

		// restore state
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
		s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
		s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	//			s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
		s_lpD3DDev->SetRenderState(D3DRS_TEXTUREFACTOR, dwTexFactor);	// alpha factor 설정
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAlphaOP);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg1);
	}
	else
	{
		m_Chr.Render();
	}

	if (s_Options.iUseShadow) this->RenderShadow(fSunAngle);




















#ifdef _DEBUG
	if(m_Chr.CollisionMesh()) // 충돌 체크용 박스..
	{
		s_lpD3DDev->SetTransform(D3DTS_WORLD, &(m_Chr.m_Matrix));
		m_Chr.CollisionMesh()->Render(0xffff0000);
	}

	__Vector3 vLine[3];
	vLine[0] = m_Chr.Pos(); vLine[0].y += 1.3f;
	vLine[1] = m_vPosFromServer; vLine[1].y += 1.3f;
	vLine[2] = vLine[1]; vLine[2].y += 3.0f;
	__Matrix44 mtx; mtx.Identity();
	CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtx);
	CN3Base::RenderLines(vLine, 2, 0xff00ffff);
#endif

	if(m_InfoBase.bRenderID && m_pIDFont)
	{
		float fDist = (m_Chr.Pos() - s_CameraData.vEye).Magnitude();

		// NOTE(srmeier): this will prevent the NPC text from displaying past a certain distance
		if(fDist < 48.0f)
		{
			__Vector3 vHead = this->HeadPosition();
			vHead.y += this->Height() / 10.0f;
			if(PSA_SITDOWN == m_eState) vHead.y += this->RootPosition().y - this->Height()/2.0f; // 앉아 있으면..
			POINT pt = ::_Convert3D_To_2DCoordinate(vHead, s_CameraData.mtxView, s_CameraData.mtxProjection, s_CameraData.vp.Width, s_CameraData.vp.Height);

			SIZE size = m_pIDFont->GetSize();
			pt.y -= size.cy + 5;
			D3DCOLOR crFont = m_pIDFont->GetFontColor();

			m_pIDFont->DrawText(pt.x - (size.cx / 2) - 1.0f, pt.y - 1.0f, 0xff000000, 0);
			m_pIDFont->DrawText(pt.x - (size.cx / 2) + 1.0f, pt.y + 1.0f, 0xff000000, 0);
			m_pIDFont->DrawText(pt.x - (size.cx / 2) + 0.0f, pt.y + 0.0f, crFont, 0);

			//Knight & clan 렌더링..
			if(m_pClanFont && m_pClanFont->IsSetText())
			{
				size = m_pClanFont->GetSize();
				pt.y -= size.cy + 5;
				crFont = m_pClanFont->GetFontColor();
				m_pClanFont->DrawText(pt.x - (size.cx / 2) - 1.0f, pt.y - 1.0f, 0xff000000, 0);
				m_pClanFont->DrawText(pt.x - (size.cx / 2) + 1.0f, pt.y + 1.0f, 0xff000000, 0);
				m_pClanFont->DrawText(pt.x - (size.cx / 2) + 0.0f, pt.y + 0.0f, crFont, 0);
			}
			
			// 파티 모집...
			if(m_pInfoFont && m_pInfoFont->IsSetText())//->GetFontHeight() > 0)
			{
				size = m_pInfoFont->GetSize();
				pt.y -= size.cy + 5;
				crFont = m_pInfoFont->GetFontColor();
				m_pInfoFont->DrawText(pt.x - (size.cx / 2) - 1.0f, pt.y - 1.0f, 0xff000000, 0);
				m_pInfoFont->DrawText(pt.x - (size.cx / 2) + 1.0f, pt.y + 1.0f, 0xff000000, 0);
				m_pInfoFont->DrawText(pt.x - (size.cx / 2) + 0.0f, pt.y + 0.0f, crFont, 0);
			}

			// 풍선 메시지..
			if(m_pBalloonFont && m_pBalloonFont->IsSetText())//->GetFontHeight())
			{
				crFont = m_pBalloonFont->GetFontColor();
				if(m_fTimeBalloon < 2.0f) // 천천히 흐릿하게 없앤다..
				{
					uint32_t crFont = m_pBalloonFont->GetFontColor();
					crFont = (crFont & 0x00ffffff) | ((uint32_t)(255 * (m_fTimeBalloon/2.0f))<<24);
					m_pBalloonFont->SetFontColor(crFont);
				}

				size = m_pBalloonFont->GetSize();
				pt.y -= size.cy + 5;
				m_pBalloonFont->DrawText(pt.x - (size.cx / 2) - 1.0f, pt.y - 1.0f, 0xff000000, 0);
				m_pBalloonFont->DrawText(pt.x - (size.cx / 2) + 1.0f, pt.y + 1.0f, 0xff000000, 0);
				m_pBalloonFont->DrawText(pt.x - (size.cx / 2) + 0.0f, pt.y + 0.0f, crFont, D3DFONT_BOLD);
			}
		}
	}
}

__Vector3 CPlayerBase::HeadPosition()
{
	__Vector3 vHead = m_Chr.Pos();
	vHead.y += m_Chr.Height();

	return vHead;
}

float CPlayerBase::Height()
{
	if(m_pShapeExtraRef) 
	{
		return (m_pShapeExtraRef->Max().y - m_pShapeExtraRef->Min().y) * m_pShapeExtraRef->Scale().y;
	}
	else
	{
		return m_Chr.Height();
	}
}

float CPlayerBase::Radius()
{
	if(m_pShapeExtraRef) 
	{
		return m_pShapeExtraRef->Radius();
	}
	else
	{
		return m_Chr.Radius();
	}
}

__Vector3 CPlayerBase::Direction()
{
	__Vector3 vDir(0,0,1);
	__Matrix44 mtxRot = m_Chr.Rot();
	vDir *= mtxRot;

	return vDir;
}

bool CPlayerBase::Action(e_StateAction eState, bool bLooping, CPlayerBase* pTarget, bool bForceSet)
{
	if(false == bForceSet)
	{
		// State Table Action
		static BOOL sTableAction[PSA_COUNT][PSA_COUNT] =
		{
			//---------------------------------------------------------------------------------------------------------------------------
			//	BASIC,	   ATTACK,		GUARD,	   STRUCK, 		DYING,		DEATH, SPELL_MAGIC,	  SITDOWN
			//---------------------------------------------------------------------------------------------------------------------------
			{		1,			1,			1,			1,			1,			0,			1,			1 }, // PSA_BASIC		-> ??
			//---------------------------------------------------------------------------------------------------------------------------
			{		1,			1,			0,			0, 			1,			0,			1,			0 }, // PSA_ATTACK		-> ??
			//---------------------------------------------------------------------------------------------------------------------------
			{		1,			1,			1,			0, 			1,			0,			1,			0 }, // PSA_GUARD		-> ??
			//---------------------------------------------------------------------------------------------------------------------------
			{		1,			1,			1,			1,			1,			0,			1,			0 }, // PSA_STRUCK		-> ??
			//---------------------------------------------------------------------------------------------------------------------------
			{		0,			0,			0,			0,			0,			1,			0,			0 }, // PSA_DYING		-> ??
			//---------------------------------------------------------------------------------------------------------------------------
			{		0,			0,			0,			0,			0,			0,			0,			0 }, // PSA_DEATH		-> ??
			//---------------------------------------------------------------------------------------------------------------------------
			{		1,			0,			0,			0,			1,			1,			1,			0 }, // PSA_SPELL_MAGIC	-> ??
			//---------------------------------------------------------------------------------------------------------------------------
			{		1,			0,			0,			0,			1,			0,			0,			0 } // PSA_SITDOWN		-> ??
			//---------------------------------------------------------------------------------------------------------------------------
		}; 

		if(FALSE == sTableAction[m_eState][eState])
			return false;
	}

//	if(m_szName == "알라딘")
//	{
//		bool bBReak = true;
//		std::string szSt1 = "??";
//		if(PSA_BASIC == m_eState) szSt1 = "보통";
//		else if(PSA_SPELLMAGIC == m_eState) szSt1 = "매직쇼~";
//		TRACE("%s(%.1f) - %s\n", m_szName.c_str(), CN3Base::TimeGet(), szSt1.c_str());
//	}

	bool bNPC = (RACE_NPC == m_InfoBase.eRace ? true : false);
	bool bNeedUpperAnimationOnly = false; // 몸 전체에 에니메이션을 적요하는가... -1 : 전체, 0 : 하체 1 : 상체
	bool bOnceAndFreeze = false;
	e_Ani eAni = ANI_UNKNOWN;
	e_Ani eAniToRestore = ANI_UNKNOWN;
	float fFreezeTime = 0.0f;
	e_StateAction eStatePrev = m_eState;
	m_eStateNext = m_eState = eState; // 일단 캐릭터의 상태 유지..
	
	switch(eState) 
	{
	case PSA_BASIC:
		if(PSM_STOP == m_eStateMove)
		{
			if(PSA_SITDOWN == eStatePrev) // 앉아 있던중이면..
			{
				eAni = ANI_STANDUP; // 일어선다.
				if(PSM_STOP == m_eStateMove) eAniToRestore = this->JudgeAnimationBreath(); // 멈추어 있으면 기본자세.. 판단.
				bOnceAndFreeze = true;
			}
			else eAni = this->JudgeAnimationBreath(); // 멈추어 있으면 기본자세.. 판단.
		}
		else if(PSM_WALK == m_eStateMove) eAni = this->JudgeAnimationWalk(); // 걷고 있으면 걷는 자세
		else if(PSM_RUN == m_eStateMove) eAni = this->JudgeAnimationRun(); // 뛰고 있으면...
		else if(PSM_WALK_BACKWARD == m_eStateMove) eAni = this->JudgeAnimationWalkBackward(); // 뒤로 걷고 있으면...
		else eAni = this->JudgeAnimationBreath(); // 그밖에 ??
		break;

	case PSA_ATTACK:
		if (pTarget != nullptr)
		{
			// 같은 국가이면
			if (!IsHostileTarget(pTarget))
				return false;
			
			if(	!bNPC && IsMovingNow() )	// 플레이어이고.. 이동중이었으면..
			{
				if(m_pItemPlugBasics[0]) eAni = ANI_ATTACK_WITH_WEAPON_WHEN_MOVE;		// 오른손에 무기를 들고 있으면.. 에니메이션 정해주기..
				else eAni = ANI_ATTACK_WITH_NAKED_WHEN_MOVE;						// 에니메이션 정해주기..
				bNeedUpperAnimationOnly = true;										// 상체는 한번 플레이하고 멈춘다..
			}
			else
			{
				eAni = this->JudgeAnimationAttack();			// 에니메이션 정해주기..
			}

			if(!bLooping) // 루핑이 아니면..
			{
				m_eStateNext = PSA_BASIC;	// 돌아갈 상태를 기억
			}
		}
		break;

	case PSA_GUARD:
		{
			m_eStateNext = PSA_BASIC;
			eAni = this->JudgeAnimationGuard();

			if(!bNPC) bNeedUpperAnimationOnly = true; // NPC 가 아니면.. 상체
			fFreezeTime = 1.5f; // 좀 길게 막는다.
		}
		break;
	
	case PSA_STRUCK:
		{
			m_eStateNext = PSA_BASIC;
			eAni = this->JudgeAnimationStruck();
			if(!bNPC) bNeedUpperAnimationOnly = true; // NPC 가 아니면.. 상체
		}
		break;
	
	case PSA_DYING:
		m_eStateNext = PSA_DEATH; // 죽인다..!!
		m_eStateMove = PSM_STOP;
		eAni = this->JudgeAnimationDying();
		bOnceAndFreeze = true; // 죽고 바닥에 뻗어서 멈춰라...
		break;

	case PSA_DEATH:
		m_eStateNext = PSA_DEATH;
		if(true == bForceSet) // 강제로 걍 죽이는 거면..
		{
			eAni = this->JudgeAnimationDying(); // 죽는 동작...
			m_Chr.AniFixToLastFrame(eAni); // 마지막 프레임으로 고정..
		}
		this->RegenerateCollisionMesh();  // 최대 최소값을 다시 찾고 충돌메시를 다시 만든다.. -> 죽은 넘 피킹을 정확하게 하기위해서이다.
		return true; // 돌아간다!!!

	case PSA_SITDOWN:
		eAni = ANI_SITDOWN;
		eAniToRestore = ANI_SITDOWN_BREATH;
		bOnceAndFreeze = true;
		break;

	case PSA_SPELLMAGIC:
		m_eStateNext = PSA_BASIC;
		fFreezeTime = m_fCastFreezeTime;
		eAni = this->JudgetAnimationSpellMagic();
		bOnceAndFreeze = true;
		break;

	default: 
		return false;
	}

	float fBlendTime = FLT_MIN;
	if(bForceSet) // 강제 설정이면..
	{
		bNeedUpperAnimationOnly = false; // NPC 가 아니면.. 상체
		fBlendTime = 0;
		m_Chr.m_FrmCtrl.Init();
	}

	// 에니메이션 세팅..
	this->AnimationClear(); // 에니메이션 큐의 내용을 지운다.. 그래야 바로 에니메이션이 나간다.
	if(ANI_UNKNOWN != eAniToRestore) this->AnimationAdd(eAniToRestore, false); // 다음 에니메이션이 있으면 데크에 집어 넣는다.

	if(bNeedUpperAnimationOnly) // 상체만 한다..
		m_Chr.AniUpperSet(eAni, fFreezeTime);
	else // 걍 에니메이션..
	{
		m_Chr.AniCurSet(eAni, bOnceAndFreeze, fBlendTime, fFreezeTime);
	}

	if(bForceSet && m_Chr.m_FrmCtrl.pAniData) // 강제 설정이면..
	{
		m_Chr.m_FrmCtrl.fFrmCur = m_Chr.m_FrmCtrl.pAniData->fFrmEnd; // 끝프레임으로 강제 설정한다..
	}

	return true;
}

bool CPlayerBase::ActionMove(e_StateMove eMove)
{
	if(this->IsDead()) return false;
	
	static int sStateTableMove[PSM_COUNT][PSM_COUNT] = // State Table Move
	{
		//---------------------------------------------------------------------------------------------------------------------------------------
		//	STOP,	WALK,	RUN,	WALK_BACKWARD
		//---------------------------------------------------------------------------------------------------------------------------------------
		{	0,		1,		1,		1 }, // PSM_STOP
		//---------------------------------------------------------------------------------------------------------------------------------------
		{	1,		0,		1,		1 }, // PSM_WALK
		//---------------------------------------------------------------------------------------------------------------------------------------
		{	1,		1,		0,		1 }, // PSM_RUN
		//---------------------------------------------------------------------------------------------------------------------------------------
		{	1,		1,		1,		0 } // PSM_WALK_BACKWARD
		//---------------------------------------------------------------------------------------------------------------------------------------
	};

	if(FALSE == sStateTableMove[m_eStateMove][eMove]) return false;
	
	m_eStateNext = m_eState; // 돌아갈 상태를 현재 상태와 맞게 강제 지정..
	m_eStateMove = eMove;

	e_Ani eAni = ANI_UNKNOWN;
	switch(eMove)
	{
	case PSM_STOP:
		eAni = this->JudgeAnimationBreath();
		break;

	case PSM_WALK:
		eAni = this->JudgeAnimationWalk();
		break;

	case PSM_RUN:
		eAni = this->JudgeAnimationRun();
		break;

	case PSM_WALK_BACKWARD:
		eAni = this->JudgeAnimationWalkBackward();
		break;

	default:
		return false;
	}

	this->Action(PSA_BASIC, true); // 딴짓 못하게 건다..

	// 에니메이션 세팅..
	this->AnimationClear(); // 에니메이션 큐의 내용을 지운다.. 그래야 바로 에니메이션이 나간다.
	m_Chr.AniCurSet(eAni, false, FLT_MIN, 0, false); // 상체 에니메이션을 중지시키지 않고 걷거나 뛴다...

	return true;
}

void CPlayerBase::ActionDying(e_StateDying eSD, const __Vector3& vDir)
{
	this->ActionMove(PSM_STOP);
	this->Action(PSA_DYING, false); // 죽이고..
	m_eStateDying = eSD;
	m_vDirDying = vDir; // 죽을때 밀리는 방향..

	e_Ani eAni = ANI_DEAD_NEATLY;
	if(eSD == PSD_KEEP_POSITION)
	{
		if(RACE_NPC == m_InfoBase.eRace) eAni = ANI_NPC_DEAD0; // NPC 일경우 
		else eAni = ANI_DEAD_NEATLY; // 플레이어 일경우..
	}
	else if(eSD == PSD_DISJOINT)
	{
		if(RACE_NPC == m_InfoBase.eRace) eAni = ANI_NPC_DEAD1; // NPC 일경우 
		else eAni = ANI_DEAD_ROLL; // 플레이어 일경우.. 몸이 휙 돌아가서 죽기..
	}
	else if(eSD == PSD_KNOCK_DOWN) // 뒤로 밀리며 죽기.
	{
		if(RACE_NPC == m_InfoBase.eRace) eAni = ANI_NPC_DEAD1; // NPC 일경우 
		else eAni = ANI_DEAD_KNOCKDOWN; // 플레이어 일경우..
	}
	else
	{
		if(RACE_NPC == m_InfoBase.eRace) eAni = ANI_NPC_DEAD0; // NPC 일경우 
		else eAni = ANI_DEAD_NEATLY; // 플레이어 일경우..
	}

	m_Chr.AniCurSet(eAni, true, FLT_MIN, 0, true);
}

bool CPlayerBase::ProcessAttack(CPlayerBase* pTarget)
{
	bool bAttackSuccess = false;
	// we assign/check our target ourselves
	//if (pTarget == nullptr) 
	//	return false;

	// check character animation state
	if (PSA_ATTACK != m_eState
		&& PSA_SPELLMAGIC != m_eState)
		return bAttackSuccess;

	
	// make sure it's time to process an auto-attack
	if(!(m_Chr.NeedStrike0() || m_Chr.NeedStrike1()))
	{
		return bAttackSuccess;
	}
	
	//CPlayerBase*
	pTarget = TargetPointerCheck(false);
	if(pTarget == nullptr)
		return bAttackSuccess;

	__Vector3 vCol(0,0,0);

	// Check that our weapons can collide with the target's mesh
	if(!this->CheckCollisionToTargetByPlug(pTarget, 0, &vCol)
		&& !this->CheckCollisionToTargetByPlug(pTarget, 1, &vCol)) 
	{
		// Get the transform of this object and its target
		const __Matrix44* pMtxMine = m_Chr.MatrixGet(0); 
		const __Matrix44* pMtxTarget = pTarget->m_Chr.MatrixGet(0);

		__Vector3 v0, v1;
		if(pMtxMine)
		{
			// Use the transform of our weapon if available, otherwise use character position
			CN3CPlug* pPlug = m_Chr.Plug(0); 
			if(pPlug != nullptr)
			{
				__Matrix44 mtx = *(m_Chr.MatrixGet(pPlug->m_nJointIndex));
				v0.Set( 0.0f, pPlug->m_fTrace1, 0.0f );
				v0 *= pPlug->m_Matrix;
				v0 *= mtx;
				v0 *= m_Chr.m_Matrix;
			}
			else
				v0 = m_Chr.Pos() + pMtxMine->Pos();
		}
		else
		{
			v0 = m_Chr.Pos();
			v0.y += m_Chr.Height() / 2.0f;
		}

		if(pMtxTarget != nullptr)
		{
			v1 = pMtxTarget->Pos() + pTarget->Position();
			// does the target's mesh collide with a line between v0---v1
			if(!pTarget->CheckCollisionByBox(v0, v1, &vCol, nullptr)) 
			{
				__Vector3 vDir = v0 - v1;
				vDir.Normalize();
				// 안되면 캐릭 사이의 값으로 한다..
				vCol = v1 + vDir * (pTarget->Height() / 3.0f); 
			}
		}
		else if(pTarget->m_pShapeExtraRef && pTarget->m_pShapeExtraRef->CollisionMesh())
		{
			__Vector3 vDir = this->Direction(); vDir.Normalize();
			v1 = v0 + (vDir * 256.0f);

			CN3VMesh* pVMesh = pTarget->m_pShapeExtraRef->CollisionMesh();
			
			if(pVMesh == nullptr || !pVMesh->CheckCollision(pTarget->m_pShapeExtraRef->m_Matrix, v0, v1, &vCol))
			{
				v1 = pTarget->m_pShapeExtraRef->Pos(); v1.y += pTarget->Height() / 2.0f;
				vDir = v1 - v0;
				vDir.Normalize();
				// 안되면 캐릭 사이의 값으로 한다..
				vCol = v0 + vDir * Radius(); 
			}
		}
		else
		{
			return false;
		}
	}
	bAttackSuccess = true;

	if(pTarget->m_fTimeAfterDeath > 0 && !pTarget->IsDead())
	{
		// set target death animation state based off of weapon type
		e_ItemClass eICR = this->ItemClass_RightHand(); 
		e_StateDying deathState = PSD_KEEP_POSITION;
		if(ITEM_CLASS_SWORD_2H == eICR || ITEM_CLASS_AXE_2H == eICR || ITEM_CLASS_MACE_2H == eICR || ITEM_CLASS_POLEARM == eICR)
			deathState = PSD_DISJOINT;
		else if(ITEM_CLASS_SWORD == eICR || ITEM_CLASS_AXE == eICR || ITEM_CLASS_MACE == eICR || ITEM_CLASS_SPEAR == eICR)
			deathState = PSD_KNOCK_DOWN;
		
		__Vector3 vTarget = pTarget->Position();
		// 퍽하고 무기 맞는 소리..
		if(pTarget->m_pSnd_Blow)
			pTarget->m_pSnd_Blow->Play(&vTarget); 

		// trigger weapon-based visual effects
		bool bAffected = false;
		if(m_pItemPlugExts[PLUG_POS_RIGHTHAND])
		{
			bAffected = TryWeaponElementEffect(PLUG_POS_RIGHTHAND, *pTarget, vCol);
		}
		if(m_pItemPlugExts[PLUG_POS_LEFTHAND] && !bAffected)
		{
			bAffected = TryWeaponElementEffect(PLUG_POS_LEFTHAND, *pTarget, vCol);
		}
		
		// if no elemental effect played, trigger the standard effect
		if(!bAffected)
			CGameProcedure::s_pFX->TriggerBundle(pTarget->IDNumber(), 0, FXID_BLOOD, vCol);

		__Vector3 vDirDeath = this->Position() - vTarget;
		vDirDeath.Normalize();
		pTarget->ActionDying(deathState, vDirDeath);
	}
	else if(pTarget->IsAlive())
	{
		// if the target failed to defend, process hit
		if(!pTarget->m_bGuardSuccess)
		{
			__Vector3 vTarget = pTarget->Position();

			// play on-hit sound effect
			if(pTarget->m_pSnd_Blow)
				pTarget->m_pSnd_Blow->Play(&vTarget);

			// trigger weapon-based visual effects
			bool bAffected = false;
			if(m_pItemPlugExts[PLUG_POS_RIGHTHAND])
			{
				bAffected = TryWeaponElementEffect(PLUG_POS_RIGHTHAND, *pTarget, vCol);
			}
			if(m_pItemPlugExts[PLUG_POS_LEFTHAND] && !bAffected)
			{
				bAffected = TryWeaponElementEffect(PLUG_POS_LEFTHAND, *pTarget, vCol);
			}

			// if no elemental effect played, trigger the standard effect
			if (!bAffected) {
				CGameProcedure::s_pFX->TriggerBundle(pTarget->IDNumber(), 0, FXID_BLOOD, vCol);	//일반무기...
				//CGameProcedure::s_pFX->TriggerBundle(pTarget->IDNumber(), 0, 13000, vCol, 0, -1, N3FORMAT_VER_2062);
			}

			D3DCOLORVALUE crHit = { 1.0f, 0.2f, 0.2f, 1.0f };
			pTarget->DurationColorSet(crHit, 0.3f);

//				int iRand = rand()%2; // 얻어 맞아 신음 소리..
//				if(iRand == 0) { if(pTarget->m_pSnd_Struck_0) pTarget->m_pSnd_Struck_0->Play(&vTarget); }
//				else if(iRand == 1) { if(pTarget->m_pSnd_Struck_1) pTarget->m_pSnd_Struck_1->Play(&vTarget); }
			if(pTarget->m_pSnd_Struck_0)
				pTarget->m_pSnd_Struck_0->Play(&vTarget);

			// TODO: Update this (and all of its outer logic)
			pTarget->Action(PSA_STRUCK, false);
		}
		// target successfully defended
		else 
		{
			// pTarget->Action(PSA_GUARD, false);
		}
	}

	return bAttackSuccess;
}

/// \brief applies any on-hit elemental effects associated with a weapon
bool CPlayerBase::TryWeaponElementEffect(e_PlugPosition plugPosition, const CPlayerBase& target, __Vector3 collisionPosition)
{
	bool affected = false;
	// only process for weapons
	if (plugPosition != PLUG_POS_LEFTHAND
		&& plugPosition != PLUG_POS_RIGHTHAND)
	{
		return affected;
	}
	int iFXID = -1;
	if((m_pItemPlugExts[plugPosition]->byMagicOrRare==ITEM_ATTRIB_UNIQUE
		&& m_pItemPlugExts[plugPosition]->byDamageFire > 0)
		|| m_pItemPlugExts[plugPosition]->byDamageFire >= LIMIT_FX_DAMAGE)
		iFXID = FXID_SWORD_FIRE_TARGET;
	else if((m_pItemPlugExts[plugPosition]->byMagicOrRare==ITEM_ATTRIB_UNIQUE
		&& m_pItemPlugExts[plugPosition]->byDamageIce > 0)
		|| m_pItemPlugExts[plugPosition]->byDamageIce >= LIMIT_FX_DAMAGE)
		iFXID = FXID_SWORD_ICE_TARGET;
	else if((m_pItemPlugExts[plugPosition]->byMagicOrRare==ITEM_ATTRIB_UNIQUE
		&& m_pItemPlugExts[plugPosition]->byDamagePoison > 0)
		|| m_pItemPlugExts[plugPosition]->byDamagePoison >= LIMIT_FX_DAMAGE)
		iFXID = FXID_SWORD_POISON_TARGET;
	else if((m_pItemPlugExts[plugPosition]->byMagicOrRare==ITEM_ATTRIB_UNIQUE
		&& m_pItemPlugExts[plugPosition]->byDamageThuner > 0)
		|| m_pItemPlugExts[plugPosition]->byDamageThuner >= LIMIT_FX_DAMAGE)
		iFXID = FXID_SWORD_LIGHTNING_TARGET;

	if(iFXID >= 0)
	{
		affected = true;
		CGameProcedure::s_pFX->TriggerBundle(target.IDNumber(), 0, iFXID, collisionPosition);
	}

	return affected;
}

e_Ani CPlayerBase::JudgeAnimationAttack()
{
	e_Ani eAni = ANI_BREATH;

	if(RACE_NPC == m_InfoBase.eRace) // NPC 일경우 
	{
		eAni = (e_Ani)(ANI_NPC_ATTACK0 + rand()%2);
	}
	else // 플레이어 일경우..
	{
		if(-1 != m_iIDTarget) // 타겟이 있으면..
		{
			e_ItemClass eICR = this->ItemClass_RightHand(); // 오른손 무기에 따라서..
			if(ITEM_CLASS_STAFF == eICR) // 지팡이 일경우 창 공격으로 한다. ???
			{
				eAni = ANI_DAGGER_ATTACK_A0;
			}
			else
			{
				eAni = (e_Ani)(this->JudgeAnimationBreath() + 1 + rand()%2);
			}
		}
	}

	return eAni;
}

e_Ani CPlayerBase::JudgeAnimationBreath()
{
	e_Ani eAni = ANI_BREATH;

	if(RACE_NPC == m_InfoBase.eRace) // NPC 일경우 
	{
		if(rand()%10 != 0) eAni = ANI_NPC_BREATH;
		else eAni = (e_Ani)(ANI_NPC_TALK0 + rand()%4);
	}
	else // 플레이어 일경우..
	{
		CPlayerBase* pTarget = TargetPointerCheck(false);

		// 타겟이 있고 국가가 다르면..
		if (pTarget != nullptr
			&& IsHostileTarget(pTarget))
		{
			e_ItemClass eICR = this->ItemClass_RightHand();
			e_ItemClass eICL = this->ItemClass_LeftHand();

			float fIWR = 0; // , fIWL = 0; // Item Weight RightHand, LeftHand
			if(m_pItemPlugBasics[PLUG_POS_RIGHTHAND]) fIWR = m_pItemPlugBasics[PLUG_POS_RIGHTHAND]->siWeight / 10.f;
//			if(m_pItemPlugBasics[PLUG_POS_LEFTHAND]) fIWL = m_pItemPlugBasics[PLUG_POS_LEFTHAND]->siWeight / 10.f;

			if(	(ITEM_CLASS_SWORD == eICR && ITEM_CLASS_SWORD == eICL) ||
				(ITEM_CLASS_AXE == eICR && ITEM_CLASS_AXE == eICL) ||
				(ITEM_CLASS_SWORD == eICR && ITEM_CLASS_AXE == eICL) ||
				(ITEM_CLASS_AXE == eICR && ITEM_CLASS_SWORD == eICL) )
			{
				if(ITEM_CLASS_SWORD == eICR) // 오른손 기준으로 무기 무게 판단..
				{
					if(fIWR < WEAPON_WEIGHT_STAND_SWORD) eAni = ANI_DUAL_BREATH_A;
					else eAni = ANI_DUAL_BREATH_B;
				}
				else // if(ITEM_CLASS_AXE == eICR) // 오른손 기준으로 무기 무게 판단..
				{
					if(fIWR < WEAPON_WEIGHT_STAND_AXE) eAni = ANI_DUAL_BREATH_A;
					else eAni = ANI_DUAL_BREATH_B;
				}
			}
			else if(ITEM_CLASS_DAGGER == eICR) // 단검
				eAni = ANI_DAGGER_BREATH_A;
			else if(ITEM_CLASS_SWORD == eICR) // 걍 보통칼
			{
				if(fIWR < WEAPON_WEIGHT_STAND_SWORD) eAni = ANI_SWORD_BREATH_A;
				else eAni = ANI_SWORD_BREATH_B;
			}
			else if(ITEM_CLASS_SWORD_2H == eICR) // 양수검
				eAni = ANI_SWORD2H_BREATH_A;
			else if(ITEM_CLASS_AXE == eICR)
			{
				if(fIWR < WEAPON_WEIGHT_STAND_AXE) eAni = ANI_AXE_BREATH_A;
				else eAni = ANI_AXE_BREATH_B;
			}
			else if(ITEM_CLASS_AXE_2H == eICR || ITEM_CLASS_MACE_2H == eICR) // 양손 도끼 혹은 둔기..
				eAni = ANI_BLUNT2H_BREATH_A; 
			else if(ITEM_CLASS_MACE == eICR)
			{
				if(fIWR < WEAPON_WEIGHT_STAND_BLUNT) eAni = ANI_BLUNT_BREATH_A;
				else eAni = ANI_BLUNT_BREATH_B;
			}
			else if(ITEM_CLASS_SPEAR == eICR) // 창
				eAni = ANI_SPEAR_BREATH_A;
			else if(ITEM_CLASS_POLEARM == eICR)
				eAni = ANI_POLEARM_BREATH_A; 
			else if(eICR == ITEM_CLASS_UNKNOWN && eICL == ITEM_CLASS_BOW) // 왼손 장착하는 활..
				eAni = ANI_BOW_BREATH;
			else if(eICR == ITEM_CLASS_BOW_CROSS && eICL == ITEM_CLASS_UNKNOWN) // 오른손 장착하는 석궁..
				eAni = ANI_CROSS_BOW_BREATH;
			else if(eICR == ITEM_CLASS_LAUNCHER && eICL >= ITEM_CLASS_UNKNOWN) // 오른손 장착하는 창 런처..
				eAni = ANI_LAUNCHER_BREATH;
			else if(eICR == ITEM_CLASS_UNKNOWN && eICL >= ITEM_CLASS_SHIELD) // 왼손 장착하는 방패..
			{
				eAni = ANI_SHIELD_BREATH_A;
			}
			else if(eICR == ITEM_CLASS_STAFF) // 지팡이..
			{
				eAni = ANI_BREATH; // 걍숨쉰다..
			}
			else // 무기 없다..
			{
				eAni = ANI_NAKED_BREATH_A;
			}
		}
		else
		{
			eAni = ANI_BREATH;
		}
	}

	return eAni;
}

e_Ani CPlayerBase::JudgeAnimationWalk() // 걷기 모드판단하기.. 가진 아이템과 타겟이 있는냐에 따라 다른 에니메이션 인덱스를 리턴.
{
	e_Ani eAni = ANI_WALK;

	if(RACE_NPC == m_InfoBase.eRace) // NPC 일경우 
	{
		eAni = ANI_NPC_WALK; 
	}
	else // 플레이어 일경우..
	{
		eAni = ANI_WALK; // 이 자리에 가진 무기에 따라 다른 에니메이션을 판단한다.
	}

	return eAni;
}

e_Ani CPlayerBase::JudgeAnimationRun() // 걷기 모드판단하기.. 가진 아이템과 타겟이 있는냐에 따라 다른 에니메이션 인덱스를 리턴.
{
	e_Ani eAni = ANI_RUN;

	if(RACE_NPC == m_InfoBase.eRace) // NPC 일경우 
	{
		eAni = ANI_NPC_RUN; 
	}
	else // 플레이어 일경우..
	{
		eAni = ANI_RUN; // 이 자리에 가진 무기에 따라 다른 에니메이션을 판단한다.
	}

	return eAni;
}

e_Ani CPlayerBase::JudgeAnimationWalkBackward() // 걷기 모드판단하기.. 가진 아이템과 타겟이 있는냐에 따라 다른 에니메이션 인덱스를 리턴.
{
	e_Ani eAni = ANI_WALK_BACKWARD;

	if(RACE_NPC == m_InfoBase.eRace) // NPC 일경우 
	{
		eAni = ANI_NPC_WALK_BACKWARD; 
	}
	else // 플레이어 일경우..
	{
		eAni = ANI_WALK_BACKWARD; // 이 자리에 가진 무기에 따라 다른 에니메이션을 판단한다.
	}

	return eAni;
}

e_Ani CPlayerBase::JudgeAnimationStruck()
{
	if(RACE_NPC == m_InfoBase.eRace) return (e_Ani)(ANI_NPC_STRUCK0 + rand()%3); // NPC 일경우 
	else return (e_Ani)(ANI_STRUCK0 + rand()%3); // 플레이어 일경우..
}

e_Ani CPlayerBase::JudgeAnimationGuard()
{
	if(RACE_NPC == m_InfoBase.eRace) return ANI_NPC_GUARD; // NPC 일경우 
	else return ANI_GUARD; // 플레이어 일경우..
}

e_Ani CPlayerBase::JudgeAnimationDying()
{
	if(RACE_NPC == m_InfoBase.eRace) return (e_Ani)(ANI_NPC_DEAD0); // NPC 일경우 
	else return (e_Ani)(ANI_DEAD_NEATLY + rand()%3); // 플레이어 일경우..
}

e_Ani CPlayerBase::JudgetAnimationSpellMagic()
{
	if(RACE_NPC == m_InfoBase.eRace) return (e_Ani)(m_iMagicAni); // NPC 일경우 
	else return (e_Ani)(m_iMagicAni); // 플레이어 일경우..
}

bool CPlayerBase::CheckCollisionByBox(const __Vector3& v0, const __Vector3& v1, __Vector3* pVCol, __Vector3* pVNormal)
{
	CN3VMesh* pvMesh = m_Chr.CollisionMesh();
	if(pvMesh == nullptr)
		return false;
	return pvMesh->CheckCollision(m_Chr.m_Matrix, v0, v1, pVCol, pVNormal);
}

bool CPlayerBase::CheckCollisionToTargetByPlug(CPlayerBase* pTarget, int nPlug, __Vector3* pVCol)
{
	if(pTarget == nullptr)
		return false;
	
	// requires an equipped weapon
	CN3CPlug* pPlug = m_Chr.Plug(nPlug);
	if(pPlug == nullptr)
		return false;
	
	// berserk
	//	if(pPlug->m_ePlugType == PLUGTYPE_CLOAK)	return false;
	//	CN3CPlug *pPlugNormal = (CN3CPlug*)pPlug;
	//	if(pPlugNormal->m_fTrace0 >= pPlugNormal->m_fTrace1) return false; // 무기의 길이 정보가 없거나 이상하면 충돌체크 하지 않는다.
	
	// "If there is no information about the length of the weapon or if it is strange, do not check for collisions."
	if(pPlug->m_fTrace0 >= pPlug->m_fTrace1)
		return false;
	// do not check for collision against dead targets
	if(PSA_DYING == pTarget->State() || PSA_DEATH == pTarget->State())
		return false;

	////////////////////////////////////////////////////////////////////////
	// 칼 궤적이 남는 시점이 아니면 충돌체크를 하지 않는다.
	//	__AnimData* pAni = m_Chr.AniCur(0);
	//	if(nullptr == pAni) return false;
	//	float fFrmCur = m_Chr.FrmCur(0);
	//	if(fFrmCur < pAni->fFrmPlugTraceStart || fFrmCur > pAni->fFrmPlugTraceEnd) return false; 
	// 칼 궤적이 남는 시점이 아니면 충돌체크를 하지 않는다.
	////////////////////////////////////////////////////////////////////////

	__Vector3 v1, v2;
	__Matrix44 mtx = *(m_Chr.MatrixGet(pPlug->m_nJointIndex));
	
	v1.Set( 0.0f, pPlug->m_fTrace0, 0.0f );
	v2.Set( 0.0f, pPlug->m_fTrace1, 0.0f );

	v1 *= pPlug->m_Matrix;
	v1 *= mtx;
	v1 *= m_Chr.m_Matrix;

	v2 *= pPlug->m_Matrix;
	v2 *= mtx;
	v2 *= m_Chr.m_Matrix;

	// 길이를 두배로 = "double the length", but we're multiplying by 1
	v2 += (v2 - v1)*1.0f; 

#ifdef _DEBUG
	s_lpD3DDev->BeginScene();

	__Vector3 vLines[2] = { v1, v2 };
	__Matrix44 mtxTmp; mtxTmp.Identity();
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &mtxTmp);
	RenderLines(vLines, 1, (D3DCOLOR)0xffff8080); // 선을 그려본다..

	if(m_pShapeExtraRef && m_pShapeExtraRef->CollisionMesh())
	{
		s_lpD3DDev->SetTransform(D3DTS_WORLD, &(m_pShapeExtraRef->m_Matrix));
		m_pShapeExtraRef->CollisionMesh()->Render((D3DCOLOR)0xffff0000); // 충돌 박스를 그려본다.
	}
	s_lpD3DDev->EndScene();
	s_lpD3DDev->Present(nullptr, nullptr, s_hWndBase, nullptr);
#endif

	if(m_pShapeExtraRef && m_pShapeExtraRef->CollisionMesh())
	{
		CN3VMesh* pVMesh = m_pShapeExtraRef->CollisionMesh();
		if(pVMesh)
		{
			return pVMesh->CheckCollision(m_pShapeExtraRef->m_Matrix, v1, v2, pVCol);
		}
	}

	// check if the target's mesh collides with the bounds created from our plug
	return pTarget->CheckCollisionByBox(v1, v2, pVCol, nullptr);	
}

CN3CPlugBase* CPlayerBase::PlugSet(e_PlugPosition ePos, const std::string& szFN, __TABLE_ITEM_BASIC* pItemBasic, __TABLE_ITEM_EXT* pItemExt)
{
	if(ePos < PLUG_POS_RIGHTHAND || ePos >= PLUG_POS_COUNT)
	{
		__ASSERT(0, "Invalid Plug Position");
		return nullptr;
	}

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
	else if(PLUG_POS_KNIGHTS_GRADE == ePos)
	{
		m_pItemPlugBasics[ePos] = pItemBasic;
		m_pItemPlugExts[ePos] = pItemExt;
	}
	else if (PLUG_POS_BACK == ePos)
	{
		//m_pItemPlugBasics[PLUG_POS_BACK] = pItem;
	}
	else { __ASSERT(0, "Invalid Plug Item position"); }


	CN3CPlugBase* pPlug = m_Chr.PlugSet(ePos, szFN);
	if(nullptr == pPlug) return nullptr;

	if (PLUG_POS_LEFTHAND == ePos || PLUG_POS_RIGHTHAND == ePos)
	{
		float fScale = m_Chr.Height() / 1.8f;  // 키에 비례해서 크게 키운다. 기본키는 1.8 미터이다.
		fScale *= pPlug->Scale().y / m_Chr.Scale().y;
		pPlug->ScaleSet(__Vector3(fScale, fScale, fScale));
		pPlug->m_nJointIndex = iJoint; // 붙는 위치 정하기..
	}
//	else if(PLUG_POS_BACK == ePos)
//	{
//		CN3CPlug_Cloak *pPlugCloak = (CN3CPlug_Cloak*)pPlug;
//	}

	if(pPlug && nullptr == pItemBasic && nullptr == pItemExt) pPlug->TexOverlapSet(""); // 기본 착용이면..

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// plug 효과 붙여라..^^	
	if(pItemExt)
	{
		if((pItemExt->byMagicOrRare==ITEM_ATTRIB_UNIQUE && pItemExt->byDamageFire > 0) || (pItemExt->byDamageFire >= LIMIT_FX_DAMAGE)) // 17 추가데미지 - 불
		{
			CN3CPlug* pCPlug = (CN3CPlug*)pPlug;
			__TABLE_FX* pFXMain = s_pTbl_FXSource.Find(FXID_SWORD_FIRE_MAIN);
			__TABLE_FX* pFXTail = s_pTbl_FXSource.Find(FXID_SWORD_FIRE_TAIL);
			
			std::string szFXMain, szFXTail;
			if(pFXMain) szFXMain = pFXMain->szFN;
			else szFXMain = "";
			if(pFXTail) szFXTail = pFXTail->szFN;
			else szFXTail = "";
			pCPlug->InitFX(szFXMain, szFXTail, 0xffffff00);
		}
		else if((pItemExt->byMagicOrRare==ITEM_ATTRIB_UNIQUE && pItemExt->byDamageIce > 0) || (pItemExt->byDamageIce >= LIMIT_FX_DAMAGE))// 18 추가데미지 - 얼음
		{
			CN3CPlug* pCPlug = (CN3CPlug*)pPlug;
			__TABLE_FX* pFXMain = s_pTbl_FXSource.Find(FXID_SWORD_ICE_MAIN);
			__TABLE_FX* pFXTail = s_pTbl_FXSource.Find(FXID_SWORD_ICE_TAIL);
			
			std::string szFXMain, szFXTail;
			if(pFXMain) szFXMain = pFXMain->szFN;
			else szFXMain = "";
			if(pFXTail) szFXTail = pFXTail->szFN;
			else szFXTail = "";

			pCPlug->InitFX(szFXMain, szFXTail, 0xff0000ff);
		}
		else if((pItemExt->byMagicOrRare==ITEM_ATTRIB_UNIQUE && pItemExt->byDamageThuner > 0) || (pItemExt->byDamageThuner >= LIMIT_FX_DAMAGE))// 19 추가데미지 - 전격			
		{
			CN3CPlug* pCPlug = (CN3CPlug*)pPlug;
			__TABLE_FX* pFXMain = s_pTbl_FXSource.Find(FXID_SWORD_LIGHTNING_MAIN);
			__TABLE_FX* pFXTail = s_pTbl_FXSource.Find(FXID_SWORD_LIGHTNING_TAIL);
			
			std::string szFXMain, szFXTail;
			if(pFXMain) szFXMain = pFXMain->szFN;
			else szFXMain = "";
			if(pFXTail) szFXTail = pFXTail->szFN;
			else szFXTail = "";
			
			pCPlug->InitFX(szFXMain, szFXTail, 0xffffffff);
		}
		else if((pItemExt->byMagicOrRare==ITEM_ATTRIB_UNIQUE && pItemExt->byDamagePoison > 0) || (pItemExt->byDamagePoison >= LIMIT_FX_DAMAGE))// 20 추가데미지 - 독			
		{
			CN3CPlug* pCPlug = (CN3CPlug*)pPlug;
			__TABLE_FX* pFXMain = s_pTbl_FXSource.Find(FXID_SWORD_POISON_MAIN);
			__TABLE_FX* pFXTail = s_pTbl_FXSource.Find(FXID_SWORD_POISON_TAIL);

			std::string szFXMain, szFXTail;
			if(pFXMain) szFXMain = pFXMain->szFN;
			else szFXMain = "";
			if(pFXTail) szFXTail = pFXTail->szFN;
			else szFXTail = "";
			
			pCPlug->InitFX(szFXMain, szFXTail, 0xffff00ff);
		}
	}
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	return pPlug;
}

CN3CPart* CPlayerBase::PartSet(e_PartPosition ePos, const std::string& szFN, __TABLE_ITEM_BASIC* pItemBasic, __TABLE_ITEM_EXT* pItemExt)
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
				this->PartSet(PART_POS_LOWER, "", m_pItemPartBasics[PART_POS_LOWER], m_pItemPartExts[PART_POS_LOWER]); // 아래를 비워준다..
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
					this->PartSet(PART_POS_LOWER, szFN2, m_pItemPartBasics[PART_POS_LOWER], m_pItemPartExts[PART_POS_LOWER]); // 하체에 전의 옷을 입힌다..
				}
				else // 하체에 입고 있었던 아이템이 없다면..
				{
					__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(m_InfoBase.eRace);	// User Player Character Skin 구조체 포인터..
					this->PartSet(PART_POS_LOWER, pLooks->szPartFNs[PART_POS_LOWER], nullptr, nullptr); // 하체에 기본옷을 입힌다.
				}
			}
		}
	}
	else if(PART_POS_LOWER == ePos) // 하체일 경우..
	{
		if(pItemBasic) // 착용하는 경우
		{
			if(m_pItemPartBasics[PART_POS_UPPER] && m_pItemPartBasics[PART_POS_UPPER]->byIsRobeType) // 전에 상체에 착용했던게 로브면..
			{
				m_pItemPartBasics[ePos] = pItemBasic;
				m_pItemPartExts[ePos] = pItemExt;
				return m_Chr.PartSet(ePos, ""); // 하체는 벗기고(?) 돌아간다.
			}
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
				pPart = m_Chr.PartSet(ePos, pLooks->szPartFNs[ePos]);
				if(pPart) pPart->TexOverlapSet("");
			}
		}
	}
	else
	{
		pPart = m_Chr.PartSet(ePos, szFN);
	}

	m_pItemPartBasics[ePos] = pItemBasic; // 아이템 적용
	m_pItemPartExts[ePos] = pItemExt;

	if(pPart && nullptr == pItemBasic && nullptr == pItemExt) pPart->TexOverlapSet(""); // 기본 착용이면..

	return pPart;
}

void CPlayerBase::DurabilitySet(e_ItemSlot eSlot, int iDurability)
{
	e_PartPosition ePartPos = PART_POS_UNKNOWN;

	if(ITEM_SLOT_HAND_RIGHT == eSlot || ITEM_SLOT_HAND_LEFT == eSlot)
	{
		e_PlugPosition ePos = PLUG_POS_LEFTHAND;
		if(ITEM_SLOT_HAND_RIGHT == eSlot) ePos = PLUG_POS_RIGHTHAND;
		if(nullptr == m_pItemPlugBasics[ePos] || nullptr == m_pItemPlugExts[ePos])
		{
			if(0 != iDurability)
			{
				__ASSERT(0, "NULL Item");
			}
			return;
		}

		int iDuMax = m_pItemPlugBasics[ePos]->siMaxDurability + m_pItemPlugExts[ePos]->siMaxDurability;
		if(iDuMax <= 0)
		{
			__ASSERT(0, "최대 내구력 없음");
			return;
		}

		CN3CPlug* pPlug = m_Chr.Plug(ePos);
		if(nullptr == pPlug) 
		{
			__ASSERT(0, "NULL Plug");
			return;
		}

		int iPercentage = iDurability * 100 / iDuMax;
		std::string szFN;
		if(iPercentage <= 30) szFN = "Misc\\Dust_Hard.dxt";
		else if(iPercentage <= 70) szFN = "Misc\\Dust_Soft.dxt";

		pPlug->TexOverlapSet(szFN);
	}
	else if(ITEM_SLOT_UPPER == eSlot) ePartPos = PART_POS_UPPER;
	else if(ITEM_SLOT_LOWER == eSlot) ePartPos = PART_POS_LOWER;
	else if(ITEM_SLOT_GLOVES == eSlot) ePartPos = PART_POS_HANDS;
	else if(ITEM_SLOT_SHOES == eSlot) ePartPos = PART_POS_FEET;
	else if(ITEM_SLOT_HEAD == eSlot) ePartPos = PART_POS_HAIR_HELMET;

	if(PART_POS_UNKNOWN != ePartPos)
	{
		CN3CPart* pPart = m_Chr.Part(ePartPos);
		if(pPart)
		{
			if(m_pItemPartBasics[ePartPos] && m_pItemPartExts[ePartPos])
			{
				int iDuMax = m_pItemPartBasics[ePartPos]->siMaxDurability + m_pItemPartExts[ePartPos]->siMaxDurability; // 기본내구력 + 확장 내구력
				int iPercentage = iDurability * 100 / iDuMax;
			
				std::string szFN;
				if(iPercentage <= 30) szFN = "Misc\\Dust_Hard.dxt";
				else if(iPercentage <= 70) szFN = "Misc\\Dust_Soft.dxt";
				pPart->TexOverlapSet(szFN);
			}
			else
			{
				pPart->TexOverlapSet("");
			}
		}
		else
		{
			__ASSERT(0, "Invalid Item Position");
		}
	}
}

bool CPlayerBase::InitChr(__TABLE_PLAYER_LOOKS* pTbl)
{
	if (pTbl == nullptr)
		return false;

	m_pLooksRef = pTbl;

	if (!pTbl->szChrFN.empty())
	{
		__Vector3 vPos = Position();
		m_Chr.LoadFromFile(pTbl->szChrFN);
		m_Chr.PosSet(vPos);
	}
	else
	{
		m_Chr.JointSet(pTbl->szJointFN);
		m_Chr.AniCtrlSet(pTbl->szAniFN);

		// 상,하체 따로 놀 준비..
		if (RACE_NPC != m_InfoBase.eRace)
		{
			m_Chr.JointPartSet(0, 16, 23); // 하체
			m_Chr.JointPartSet(1, 1, 15); // 상체
		}

		if (!pTbl->szFXPlugFN.empty())
			m_Chr.FXPlugSet(pTbl->szFXPlugFN);
	}

	return true;
}


void CPlayerBase::RegenerateCollisionMesh() // 최대 최소값을 다시 찾고 충돌메시를 다시 만든다..
{
	m_Chr.RegenerateCollisionMesh();
}

CPlayerBase* CPlayerBase::TargetPointerCheck(bool bMustAlive)
{
	CPlayerBase* pTarget = this->CharacterGetByID(m_iIDTarget, bMustAlive);
//	if(pTarget && (PSA_DEATH == pTarget->State())) pTarget = nullptr; //픽킹에 의해서 선택된 캐릭을 중간의 임의적으로 수정 못하게 막음

//	if(nullptr == pTarget) { m_iIDTarget = -1; }
	return pTarget;
}

bool CPlayerBase::JointPosGet(int nJointIdx, __Vector3& vPos)
{
	const __Matrix44* pMtx = m_Chr.MatrixGet(nJointIdx);
	if(!pMtx) return false;

	vPos = pMtx->Pos();
	vPos *= m_Chr.m_Matrix;

	return true;
}

void CPlayerBase::AnimationAdd(e_Ani eAni, bool bImmediately)
{
	if(bImmediately)
	{
		this->AnimationClear(); // 데크에서 지우고..
		m_Chr.AniCurSet(eAni); // 바로 세팅..
	}
	else
	{
		m_AnimationDeque.push_back(eAni);
	}
}

const __Matrix44 CPlayerBase::CalcShadowMtxBasicPlane(__Vector3 vOffs)
{
	__Matrix44	mtx;	
	mtx.Identity();

	mtx._21 = -vOffs.x/vOffs.y;
	mtx._22 = 0.0f;
	mtx._23 = -vOffs.z/vOffs.y;

	mtx._41	= vOffs.x/vOffs.y;
	mtx._42	= 1.0f;
	mtx._43	= vOffs.z/vOffs.y;

	return mtx;
}

void CPlayerBase::RenderShadow(float fAngle)
{
	if(nullptr == m_Chr.Joint()) return;
	if(m_Chr.m_nLOD < 0 || m_Chr.m_nLOD > MAX_CHR_LOD) return;
	
	int iLODTemp = m_Chr.m_nLOD; // 그림자는 부하를 적게 줄여 찍는다..
	iLODTemp += 2;
	if(iLODTemp >= MAX_CHR_LOD) return;

	__Matrix44 mV, mVvar, mVBack, mtxRotX, mtxRotZ;	mV.Identity(); 
	__Vector3 vPosBack, vNom;
	CN3Base::s_lpD3DDev->GetTransform(D3DTS_WORLD, (_D3DMATRIX* )&mVBack); 
	vPosBack = m_Chr.m_Matrix.Pos();	
	s_lpD3DDev->SetTransform(D3DTS_WORLD, &mV);
	ACT_WORLD->GetNormalWithTerrain(vPosBack.x, vPosBack.z, vNom);	vNom.Normalize();

	mV.PosSet(0.0f, 0.0f, 0.0f);
	float fXRad, fZRad;
	fZRad = asin(vNom.z);
	mtxRotZ.RotationX(fZRad);
	fXRad = asin(-vNom.x);
	mtxRotX.RotationZ(fXRad);
	mV *= mtxRotX;
	mV *= mtxRotZ;
	vPosBack.y += 0.1f;

	mV.PosSet(vPosBack);
	mVvar = mV;

	for( int i = 0; i < SHADOW_SIZE; i++ )
		m_bitset[i].Resize(SHADOW_SIZE);

	for ( int i = 0; i < 4; i++ )
	{
		m_vTVertex[i]  = m_pvVertex[i];	
		m_vTVertex[i] *= mV;
	}

	// 캐릭터의 크기에 따라 스캐일링 한다..
	float fScale = Height() * Radius();
	m_fShaScale = 1.0f;
	__Matrix44 mtPos;	mtPos.Identity(); mtPos.PosSet(mV.Pos());
	if (fScale > 6.6f)
	{
		m_fShaScale = 2.2f;
		for ( int i = 0; i < 4; i++ )
		{
			m_vTVertex[i]  = m_pvVertex[i];	
			m_vTVertex[i] *= m_fShaScale;
			m_vTVertex[i] *= mtPos;
		}
	}
	else
	{
		for ( int i = 0; i < 4; i++ )
		{
			m_vTVertex[i]  = m_pvVertex[i];	
			m_vTVertex[i] *= mtPos;
		}
	}

	// 태양 위치를 제대로 계산해서 넣기만 하면 된다..
	if ( (fAngle < 4.0f) || (fAngle > 6.1f) )
	{
		if (fAngle > 3.14f)
		{
			fAngle -= 3.14f;
			fAngle = 3.14f - fAngle;
		}
	}
	float fAngleDeg = D3DXToDegree(fAngle);

	float zVal = s_vLightOffset.Magnitude();
	int iDiv = (int)((int)fAngleDeg)%((int)(180));
	fAngleDeg = (float)iDiv;

	if ( (fAngleDeg >= 0.0f) && (fAngleDeg < 50.0f) )
		fAngleDeg = 50.0f;
	else if ( (fAngleDeg > 130.0f) && (fAngleDeg <= 180.0f) )
		fAngleDeg = 130.0f;

	__Matrix44 mtxRZ; mtxRZ.RotationZ(D3DXToRadian(fAngleDeg));
	__Vector3 vLP; vLP.Set(-zVal, 0.0f, 0.0f );	vLP *= mtxRZ;	vLP.Normalize();

	int iPC = m_Chr.PartCount();
	for( int i = 0; i < iPC; i++)
	{
		CalcPart(m_Chr.Part(i), iLODTemp, vLP);
	}

	iPC = m_Chr.PlugCount();
	for(int i = 0; i < iPC; i++)
	{
		CalcPlug(m_Chr.Plug(i), m_Chr.MatrixGet(m_Chr.Plug(i)->m_nJointIndex), mVvar, vLP);
	}

	// 렌더링하기 전에 스케일을 줄인다..
	if (fScale > 6.6f)
	{
		for ( int i = 0; i < 4; i++ )
		{
			m_vTVertex[i]  = m_pvVertex[i];	
			m_vTVertex[i] *= 0.82f;	
			m_vTVertex[i] *= m_fShaScale;
			m_vTVertex[i] *= mV;
		}
	}
	else
	{
		for ( int i = 0; i < 4; i++ )
		{
			m_vTVertex[i]  = m_pvVertex[i];	
			m_vTVertex[i] *= 0.5f;	
			m_vTVertex[i] *= mV;
		}
	}

	D3DLOCKED_RECT LR;
	if (!m_N3Tex.Get()) return;
	HRESULT hr = m_N3Tex.Get()->LockRect(0, &LR, 0, 0); 
	if (hr != D3D_OK)
		return;

	LPWORD pDst16 = (LPWORD)LR.pBits;
	uint16_t dwColor = SHADOW_COLOR;
	dwColor = dwColor << 12;
	for( int i = 0; i < SHADOW_SIZE; i++ )
	{
		for(int j = 0; j < SHADOW_SIZE; j++ )
		{
			if (m_bitset[i].On(j))
				pDst16[SHADOW_SIZE*i+j] = dwColor;
			else
				pDst16[SHADOW_SIZE*i+j] = 0x0000;
		}
	}

	m_N3Tex.Get()->UnlockRect(0);
 
	DWORD dwAlpha, dwFog, dwCull, dwColorVertex, dwMaterial, dwZWrite,
		dwColorOp0, dwColorArg01, dwColorArg02, dwColorOp1, dwColorArg11, dwColorArg12, 
		dwAlphaOp0, dwAlphaArg01, dwAlphaArg02, dwAlphaOp1, dwAlphaArg11, dwAlphaArg12, dwSrcBlend, dwDestBlend, dwBlendOp,
		dwMagFilter0, dwMinFilter0, dwMagFilter1, dwMinFilter1;

	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_CULLMODE, &dwCull);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_COLORVERTEX, &dwColorVertex);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, &dwMaterial);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWrite);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwColorOp0);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorArg01);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG2, &dwColorArg02);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLOROP, &dwColorOp1);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG1, &dwColorArg11);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_COLORARG2, &dwColorArg12);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOp0);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg01);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG2, &dwAlphaArg02);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_ALPHAOP, &dwAlphaOp1);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_ALPHAARG1, &dwAlphaArg11);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_ALPHAARG2, &dwAlphaArg12);
	/*
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_MAGFILTER, &dwMagFilter0);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_MINFILTER, &dwMinFilter0);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_MAGFILTER, &dwMagFilter1);
	CN3Base::s_lpD3DDev->GetTextureStageState(1, D3DTSS_MINFILTER, &dwMinFilter1);
	*/
	CN3Base::s_lpD3DDev->GetSamplerState(0, D3DSAMP_MAGFILTER, &dwMagFilter0);
	CN3Base::s_lpD3DDev->GetSamplerState(0, D3DSAMP_MINFILTER, &dwMinFilter0);
	CN3Base::s_lpD3DDev->GetSamplerState(1, D3DSAMP_MAGFILTER, &dwMagFilter1);
	CN3Base::s_lpD3DDev->GetSamplerState(1, D3DSAMP_MINFILTER, &dwMinFilter1);

	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_BLENDOP, &dwBlendOp);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_COLORVERTEX, TRUE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP,  D3DTOP_MODULATE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP,  D3DTOP_MODULATE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	CN3Base::s_lpD3DDev->SetTexture(0, m_N3Tex.Get());

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	*/
	CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	CN3Base::s_lpD3DDev->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	CN3Base::s_lpD3DDev->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	CN3Base::s_lpD3DDev->SetFVF(FVF_VNT1);
	CN3Base::s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, m_pIndex, D3DFMT_INDEX16, m_vTVertex, sizeof(__VertexT1) );

	//..
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwColorOp0);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwColorArg01);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, dwColorArg02);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, dwColorOp1);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, dwColorArg11);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, dwColorArg12);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAlphaOp0);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg01);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, dwAlphaArg02);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_ALPHAOP, dwAlphaOp1);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_ALPHAARG1, dwAlphaArg11);
	CN3Base::s_lpD3DDev->SetTextureStageState(1, D3DTSS_ALPHAARG2, dwAlphaArg12);

	CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, dwMagFilter0);
	CN3Base::s_lpD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, dwMinFilter0);
	CN3Base::s_lpD3DDev->SetSamplerState(1, D3DSAMP_MAGFILTER, dwMagFilter1);
	CN3Base::s_lpD3DDev->SetSamplerState(1, D3DSAMP_MINFILTER, dwMinFilter1);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_COLORVERTEX, dwColorVertex);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, dwMaterial);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog); 
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_CULLMODE, dwCull);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_BLENDOP, dwBlendOp);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);

	CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, (_D3DMATRIX *)&mVBack); 
}

void CPlayerBase::CalcPart(CN3CPart* pPart, int nLOD, __Vector3 vDir)
{
	if(nullptr == pPart) return;
	if(nLOD < 0 || nLOD > MAX_CHR_LOD) return;
	if(nullptr == pPart->Skins() || nullptr == pPart->Skin(nLOD) || pPart->Skin(nLOD)->VertexCount() <= 0) return;

#ifdef _DEBUG
	CN3Base::s_RenderInfo.nChr_Part++; // Rendering Information Update...
	CN3Base::s_RenderInfo.nChr_Polygon += pPart->Skin(nLOD)->FaceCount();
#endif

	int iTotalCount = pPart->Skin(nLOD)->VertexCount();	
	if (iTotalCount < 0 || iTotalCount >  10000 ) return;

	__Vector3 vec, A, B, C, vPick;
	CN3Skin* part00 = pPart->Skin(nLOD);
	__VertexXyzNormal* pVDest = part00->m_pVertices;
	float t, u, v, fx, fz;

	for ( int i = 0; i < iTotalCount; i++ )
	{
		if (pVDest)
		{
			vec = pVDest[i];	
			vec *= m_Chr.m_Matrix;
			//vec.x = vec.x*m_Chr.m_Matrix._11 + vec.y*m_Chr.m_Matrix._21 + vec.z*m_Chr.m_Matrix._31 + m_Chr.m_Matrix._41;
			//vec.y = vec.x*m_Chr.m_Matrix._12 + vec.y*m_Chr.m_Matrix._22 + vec.z*m_Chr.m_Matrix._32 + m_Chr.m_Matrix._42;
			//vec.z = vec.x*m_Chr.m_Matrix._13 + vec.y*m_Chr.m_Matrix._23 + vec.z*m_Chr.m_Matrix._33 + m_Chr.m_Matrix._43;

			int iX, iZ;
			
			A = m_vTVertex[m_pIndex[0]];			B = m_vTVertex[m_pIndex[1]];			C = m_vTVertex[m_pIndex[2]];
			if (::_IntersectTriangle(vec, vDir, A, B, C, t, u, v, &vPick))
			{
				fx = (vPick.x - C.x) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				fz = (vPick.z - C.z) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				iX = (int)(fx*SHADOW_SIZE);
				iZ = (int)(fz*SHADOW_SIZE);
				if ( (iX > 0) && (iX < SHADOW_SIZE) && (iZ > 0) && (iZ < SHADOW_SIZE) )
					m_bitset[iZ].Set(iX);
			}

			A = m_vTVertex[m_pIndex[3]];			B = m_vTVertex[m_pIndex[4]];			C = m_vTVertex[m_pIndex[5]];
			if (::_IntersectTriangle(vec, vDir, A, B, C, t, u, v, &vPick))
			{
				fx = (vPick.x - B.x) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				fz = (vPick.z - B.z) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				iX = (int)(fx*SHADOW_SIZE);
				iZ = (int)(fz*SHADOW_SIZE);
				if ( (iX > 0) && (iX < SHADOW_SIZE) && (iZ > 0) && (iZ < SHADOW_SIZE) )
					m_bitset[iZ].Set(iX);
			}
		}
	}
}

void CPlayerBase::CalcPlug(CN3CPlugBase* pPlug, const __Matrix44* pmtxJoint, __Matrix44& mtxMV, __Vector3 vDir)
{
	if(nullptr == pPlug || nullptr == pPlug->PMeshInst()) return;
	if(pPlug->PMeshInst()->GetNumVertices() <= 0) return;

#ifdef _DEBUG
	CN3Base::s_RenderInfo.nChr_Plug++; // Rendering Information Update...
	CN3Base::s_RenderInfo.nChr_Plug_Polygon += pPlug->PMeshInst()->GetNumIndices() / 3; // Rendering Information Update...
#endif

	static __Matrix44 mtx, mtxBack;
	__Vector3 vOf;
	mtx = pPlug->m_Matrix;
	mtx *= (*pmtxJoint);
	mtx *= m_Chr.m_Matrix;
	
	int iTotalCount = pPlug->PMeshInst()->GetNumVertices();	
	if (iTotalCount < 0 || iTotalCount >  10000 ) return;

	__Vector3 vec, A, B, C, vPick;
	float t, u, v, fx, fz;

	__VertexT1*	pVerT1 = nullptr;
	pVerT1 = pPlug->PMeshInst()->GetVertices();

	for ( int i = 0; i < iTotalCount; i++ )
	{
		if (pVerT1)
		{
			vec = pVerT1[i];
			vec *= mtx;

			int iX, iZ;			

			A = m_vTVertex[m_pIndex[0]];			B = m_vTVertex[m_pIndex[1]];			C = m_vTVertex[m_pIndex[2]];
			if (::_IntersectTriangle(vec, vDir, A, B, C, t, u, v, &vPick))
			{
				fx = (vPick.x - C.x) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				fz = (vPick.z - C.z) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				iX = (int)(fx*SHADOW_SIZE);
				iZ = (int)(fz*SHADOW_SIZE);
				if ( (iX > 0) && (iX < SHADOW_SIZE) && (iZ > 0) && (iZ < SHADOW_SIZE) )
					m_bitset[iZ].Set(iX);
			}

			A = m_vTVertex[m_pIndex[3]];			B = m_vTVertex[m_pIndex[4]];			C = m_vTVertex[m_pIndex[5]];
			if (::_IntersectTriangle(vec, vDir, A, B, C, t, u, v, &vPick))
			{
				fx = (vPick.x - B.x) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				fz = (vPick.z - B.z) / (SHADOW_PLANE_SIZE*2.0f*m_fShaScale);
				iX = (int)(fx*SHADOW_SIZE);
				iZ = (int)(fz*SHADOW_SIZE);
				if ( (iX > 0) && (iX < SHADOW_SIZE) && (iZ > 0) && (iZ < SHADOW_SIZE) )
					m_bitset[iZ].Set(iX);
			}
		}
	}
}

__Vector3 CPlayerBase::Max()
{
	if (m_pShapeExtraRef != nullptr)
		return m_pShapeExtraRef->Max();

	return m_Chr.Max();
}

__Vector3 CPlayerBase::Min()
{
	if (m_pShapeExtraRef != nullptr)
		return m_pShapeExtraRef->Min();

	return m_Chr.Min();
}

__Vector3 CPlayerBase::Center()
{
	__Vector3 vCenter;

	if (m_pShapeExtraRef != nullptr)
	{
		vCenter = m_pShapeExtraRef->Min() + (m_pShapeExtraRef->Max() - m_pShapeExtraRef->Min()) * 0.5f;
		return vCenter;
	}

	return (m_Chr.Min() + (m_Chr.Max() - m_Chr.Min()) * 0.5f);
}

bool CPlayerBase::IsHostileTarget(const CPlayerBase* rhs) const
{
	if (rhs == nullptr)
		return false;

	if (rhs == this)
		return false;

	if (m_InfoBase.iAuthority == AUTHORITY_LIMITED_MANAGER)
		return false;

	// TODO: Tile support

	switch (GetCurrentZoneAbilityType())
	{
		case ZONE_ABILITY_SPECTATOR:
			if (rhs->PlayerType() == PLAYER_NPC
				&& rhs->Nation() != NATION_NOTSELECTED)
				return false;

			return true;

		case ZONE_ABILITY_NEUTRAL:
			if (m_InfoBase.iKnightsWarEnemyID != 0
				&& m_InfoBase.iKnightsWarEnemyID == rhs->m_InfoBase.iKnightsID)
				return true;

			if (PlayerType() != PLAYER_NPC
				&& rhs->PlayerType() != PLAYER_NPC)
				return false;

			if (Nation() != NATION_NOTSELECTED)
			{
				if (rhs->Nation() != NATION_NOTSELECTED)
					return false;
			}
			else if (rhs->Nation() != NATION_NOTSELECTED)
			{
				return false;
			}

			return true;

		case ZONE_ABILITY_PVP:
			if (m_InfoBase.iKnightsWarEnemyID != 0
				&& m_InfoBase.iKnightsWarEnemyID == rhs->m_InfoBase.iKnightsID)
				return true;

			return Nation() != rhs->Nation();

		case ZONE_ABILITY_PVP_NEUTRAL_NPCS:
			if (Nation() == rhs->Nation())
				return false;

			if (PlayerType() == PLAYER_NPC
				|| rhs->PlayerType() == PLAYER_NPC)
			{
				if (Nation() != NATION_NOTSELECTED)
				{
					if (rhs->Nation() != NATION_NOTSELECTED)
						return false;
				}
				else
				{
					if (rhs->Nation() == NATION_NOTSELECTED)
						return false;
				}
			}

			return true;

		case ZONE_ABILITY_SIEGE_TYPE_1:
			if (rhs->m_InfoBase.iKnightsID <= 0
				&& rhs->PlayerType() == PLAYER_NPC)
			{
				if (rhs->Nation() != NATION_NOTSELECTED)
					return false;
			}

			if (m_InfoBase.iKnightsID > 0
				&& m_InfoBase.iKnightsID == rhs->m_InfoBase.iKnightsID)
				return false;

			return true;

		case ZONE_ABILITY_SIEGE_TYPE_2:
			if (rhs->m_InfoBase.iKnightsID <= 0
				&& rhs->PlayerType() == PLAYER_NPC
				&& rhs->Nation() != NATION_NOTSELECTED)
				return false;

			if (m_InfoBase.iAllianceID > 0)
			{
				if (m_InfoBase.iAllianceID == rhs->m_InfoBase.iAllianceID)
					return false;
			}
			else
			{
				if (m_InfoBase.iKnightsID > 0
					&& m_InfoBase.iKnightsID == rhs->m_InfoBase.iKnightsID)
					return false;
			}

			return true;

		case ZONE_ABILITY_SIEGE_TYPE_3:
			return true;

		case ZONE_ABILITY_SIEGE_DISABLED:
			if (rhs->PlayerType() == PLAYER_NPC
				&& rhs->Nation() != NATION_NOTSELECTED)
				return false;

			if (m_InfoBase.iKnightsID > 0
				&& rhs->m_InfoBase.iKnightsID == m_InfoBase.iKnightsID)
				return false;

			return true;

		case ZONE_ABILITY_CAITHAROS_ARENA:
			if (rhs->PlayerType() == PLAYER_NPC
				&& rhs->Nation() != NATION_NOTSELECTED)
				return false;

			if (m_InfoBase.iKnightsID > 0
				&& rhs->m_InfoBase.iKnightsID == m_InfoBase.iKnightsID)
				return false;

			return true;
	}

	return true;
}
