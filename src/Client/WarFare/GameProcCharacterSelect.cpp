// GameProcCharacterSelect.cpp: implementation of the CGameProcCharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameProcCharacterSelect.h"
#include "GameProcedure.h"
#include "text_resources.h"
#include "PacketDef.h"
#include "GameEng.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "PlayerMySelf.h"
#include "UIManager.h"
#include "UICharacterSelect.h"
#include "UIMessageBox.h"
#include "UILoading.h"

#include <N3Base/N3SndObj.h>
#include <N3Base/N3Shape.h>
#include <N3Base/N3Camera.h>
#include <N3Base/N3Light.h>

CGameProcCharacterSelect::CGameProcCharacterSelect()
{
	m_pCamera = nullptr;

	for (int i = 0; i < 8; i++)
		m_pLights[i] = nullptr;

	for (int i = 0; i < 3; i++)
		m_lgt[i] = {};

	for (int i = 0; i < MAX_AVAILABLE_CHARACTER; i++)
		m_pChrs[i] = nullptr;

	m_pActiveBg                = nullptr;

	m_eCurPos                  = POS_CENTER;
	m_eDestPos                 = POS_CENTER;
	m_eCurProcess              = PROCESS_PRESELECT;
	m_fCurTheta                = 0.0f;
	m_fFadeOut                 = 0.0f;
	m_bFadeOutRender           = false;

	m_bReceivedCharacterSelect = false;
	m_pUICharacterSelect       = nullptr;

	m_pSnd_Rotate              = nullptr;

	m_vEyeBackup               = {};
	m_vEye                     = {};
	m_vAt                      = {};
}

CGameProcCharacterSelect::~CGameProcCharacterSelect()
{
	delete m_pCamera;
	for (int i = 0; i < 8; i++)
		delete m_pLights[i];
	for (int i = 0; i < MAX_AVAILABLE_CHARACTER; i++)
		delete m_pChrs[i];
	delete m_pActiveBg;
	delete m_pUICharacterSelect;

	::ShowCursor(TRUE);
}

void CGameProcCharacterSelect::Release()
{
	CGameProcedure::Release();

	delete m_pCamera;
	m_pCamera = nullptr;
	for (int i = 0; i < 8; i++)
	{
		delete m_pLights[i];
		m_pLights[i] = nullptr;
	}
	for (int i = 0; i < MAX_AVAILABLE_CHARACTER; i++)
	{
		delete m_pChrs[i];
		m_pChrs[i] = nullptr;
		m_InfoChrs[i].clear();
	}

	delete m_pActiveBg;
	m_pActiveBg = nullptr;
	delete m_pUICharacterSelect;
	m_pUICharacterSelect = nullptr;
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Rotate);

	::ShowCursor(TRUE);
}

void CGameProcCharacterSelect::Init()
{
	//..
	m_pCamera = nullptr;
	for (int i = 0; i < 8; i++)
		m_pLights[i] = nullptr;
	for (int i = 0; i < MAX_AVAILABLE_CHARACTER; i++)
	{
		m_pChrs[i] = nullptr;
	}
	m_pActiveBg                = nullptr;

	m_eCurPos                  = POS_CENTER;
	m_eDestPos                 = POS_CENTER;
	m_eCurProcess              = PROCESS_PRESELECT;
	m_fCurTheta                = 0.0f;
	m_fFadeOut                 = 0.0f;
	m_bFadeOutRender           = false;

	m_bReceivedCharacterSelect = false;
	m_pUICharacterSelect       = nullptr;

	m_pSnd_Rotate              = nullptr;
	//..

	CGameProcedure::Init();
	//	CGameProcedure::s_pEng->BackupReleaseLighting();

	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Rotate);
	m_pSnd_Rotate = CN3Base::s_SndMgr.CreateObj(ID_SOUND_CHR_SELECT_ROTATE);
	s_pUIMgr->EnableOperationSet(false); // 기존의 캐릭터 정보 패킷이 들어올때까지 UI 를 Disable 시킨다...

	m_pCamera = new CN3Camera();
	for (int i = 0; i < 8; i++)
		m_pLights[i] = new CN3Light();
	for (int i = 0; i < MAX_AVAILABLE_CHARACTER; i++)
		m_pChrs[i] = nullptr;

	m_eCurPos                         = POS_CENTER;
	m_eDestPos                        = POS_CENTER;
	m_eCurProcess                     = PROCESS_PRESELECT;
	m_fCurTheta                       = 0.0f;

	CGameProcedure::s_iChrSelectIndex = 0;

	e_Nation eNation                  = s_pPlayer->m_InfoBase.eNation;
	__TABLE_UI_RESRC* pTbl            = s_pTbl_UI.Find(eNation);

	m_pUICharacterSelect              = new CUICharacterSelect();
	m_pUICharacterSelect->Init(s_pUIMgr);
	if (pTbl)
		m_pUICharacterSelect->LoadFromFile(pTbl->szCharacterSelect); // UI Manager 에 넣지 않고 따로 처리한다... 이유가 있다..
	m_pUICharacterSelect->SetPos(0, 0);
	s_pUIMgr->SetFocusedUI((CN3UIBase*) m_pUICharacterSelect);

	switch (eNation)
	{
		case NATION_KARUS:
			m_vEye.Set(0.0f, -0.2f, 7.4f);
			m_vAt.Set(0.0f, -0.4f, -0.0f);
			m_vUp.Set(0.0f, 1.0f, 0.0f);
			break;

		case NATION_ELMORAD:
			m_vEye.Set(0.0f, -0.2f, 7.0f), m_vAt.Set(0.0f, -0.4f, -0.0f), m_vUp.Set(0.0f, 1.0f, 0.0f);
			break;

		default:
			break;
	}

	m_vEyeBackup = m_vEye;

	// 배경..
	m_pActiveBg  = new CN3Shape;
	for (int i = 0; i < 3; i++)
		m_lgt[i] = {};

	// 0가운데.. 1왼쪽..
	m_lgt[2].Type = m_lgt[1].Type = m_lgt[0].Type = D3DLIGHT_SPOT;
	m_lgt[2].Attenuation0 = m_lgt[1].Attenuation0 = m_lgt[0].Attenuation0 = 0.1f;
	m_lgt[2].Attenuation1 = m_lgt[1].Attenuation1 = m_lgt[0].Attenuation1 = 0.0f;
	m_lgt[2].Attenuation2 = m_lgt[1].Attenuation2 = m_lgt[0].Attenuation2 = 0.0f;
	m_lgt[2].Range = m_lgt[1].Range = m_lgt[0].Range = 6.0f;
	m_lgt[2].Diffuse.r = m_lgt[1].Diffuse.r = m_lgt[0].Diffuse.r = 255 / 255.0f;
	m_lgt[2].Diffuse.g = m_lgt[1].Diffuse.g = m_lgt[0].Diffuse.g = 255 / 255.0f;
	m_lgt[2].Diffuse.b = m_lgt[1].Diffuse.b = m_lgt[0].Diffuse.b = 255 / 255.0f;
	m_lgt[2].Falloff = m_lgt[1].Falloff = m_lgt[0].Falloff = 20.0f;

	__Vector3 vTemp;
	switch (s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_KARUS:
			m_pActiveBg->LoadFromFile("ChrSelect\\ka_chairs.n3shape");
			//			m_pActiveBg->LoadFromFile("Misc\\itembox.n3shape");

			// Light..
			m_pLights[0]->LoadFromFile("ChrSelect\\ka_light_0.n3light");
			m_pLights[1]->LoadFromFile("ChrSelect\\ka_light_1.n3light");
			m_pLights[2]->LoadFromFile("ChrSelect\\ka_light_2.n3light");

			m_lgt[0].Position    = m_vEye; // 카루스
			m_lgt[0].Position.y += 2.0f;   // 카루스
			vTemp                = { 0.0f, 0.0f, 3.5f };
			vTemp               -= m_lgt[0].Position;
			m_lgt[0].Direction   = vTemp;
			m_lgt[0].Phi         = 0.6f;

			m_lgt[1].Position    = { 5.87f, 2.4f, 4.73f }; // 카루스
			vTemp                = { 2.32f, 0.0f, 2.54f };
			vTemp               -= m_lgt[1].Position;
			m_lgt[1].Direction   = vTemp;
			m_lgt[1].Phi         = 0.6f;

			m_lgt[2].Position    = { -5.87f, 2.4f, 4.73f }; // 카루스
			vTemp                = { -2.32f, 0.0f, 2.54f };
			vTemp               -= m_lgt[2].Position;
			m_lgt[2].Direction   = vTemp;
			m_lgt[2].Phi         = 0.6f;
			break;

		case NATION_ELMORAD:
			m_pActiveBg->LoadFromFile("ChrSelect\\el_chairs.n3shape");

			// Light..
			m_pLights[0]->LoadFromFile("ChrSelect\\el_light_0.n3light");
			m_pLights[1]->LoadFromFile("ChrSelect\\el_light_1.n3light");
			m_pLights[2]->LoadFromFile("ChrSelect\\el_light_2.n3light");

			m_lgt[0].Position    = m_vEye; // 카루스
			m_lgt[0].Position.y += 2.0f;   // 카루스
			vTemp                = { 0.0f, -0.1f, 3.0f };
			vTemp               -= m_lgt[0].Position;
			m_lgt[0].Direction   = vTemp;
			m_lgt[0].Phi         = 0.45f;

			m_lgt[1].Position    = { 5.6f, 2.4f, 4.68f }; // 카루스
			vTemp                = { 2.2f, -0.1f, 2.36f };
			vTemp               -= m_lgt[1].Position;
			m_lgt[1].Direction   = vTemp;
			m_lgt[1].Phi         = 0.45f;

			m_lgt[2].Position    = { -5.6f, 2.4f, 4.68f }; // 카루스
			vTemp                = { -2.4f, -0.1f, 2.23f };
			vTemp               -= m_lgt[2].Position;
			m_lgt[2].Direction   = vTemp;
			m_lgt[2].Phi         = 0.45f;
			break;

		default:
			break;
	}

	if (s_bIsRestarting)
		MsgSend_VersionCheck();
	else
		MsgSend_RequestAllCharacterInfo(); // 캐릭터 정보 요청..
}

void CGameProcCharacterSelect::Tick()
{
	CGameProcedure::Tick(); // 키, 마우스 입력 등등..

	__Vector3 vDir = CN3Base::s_CameraData.vAt - CN3Base::s_CameraData.vEye;
	vDir.Normalize();

	const __Vector3 vEye = CN3Base::s_CameraData.vEye;
	const __Vector3 vUp  = { 0, 1, 0 };

	CN3SndObj::SetListenerPos(vEye);
	CN3SndObj::SetListenerOrientation(vDir, vUp);

	// 배경..
	m_pActiveBg->Tick();

	if (s_pUIMgr->m_bDoneSomething == false && s_pUIMgr->EnableOperation()) // 패킷을 받기 전에 아무짓도 못하게 한다.
	{
		s_pUIMgr->SetFocusedUI(m_pUICharacterSelect);
		int nMFlags = s_pLocalInput->MouseGetFlag();                        // Mouse 상태 플래그..

		// 누르는 순간..
		if ((nMFlags & MOUSE_LBCLICK) && (m_eCurProcess != PROCESS_ROTATEING))
		{
			D3DVIEWPORT9 vp {};
			CN3Base::s_lpD3DDev->GetViewport(&vp);

			RECT rc  = { (int) (vp.Width * 0.36f), (int) (vp.Height * 0.44f), (int) (vp.Width * 0.64f), (int) (vp.Height * 0.86f) };
			POINT pt = s_pLocalInput->MouseGetPos();

			if (::PtInRect(&rc, pt))
				CharacterSelectOrCreate();
		}
	}

	// 라이트..
	for (int i = 0; i < 8; i++)
		s_pEng->s_lpD3DDev->LightEnable(i, FALSE); // 일단 라이트 다 끄고..

	for (int i = 0; i < 2; i++)
	{
		m_pLights[i]->Tick(m_pLights[i]->m_fFrmCur);
		m_pLights[i]->Apply(); // 라이트 적용
	}

	CheckJobState();

	if (!IsUIKeyOperated() && s_pLocalInput->IsKeyPress(DIK_RETURN))
		ProcessOnReturn();
}

void CGameProcCharacterSelect::Render()
{
	D3DCOLOR crEnv = 0x00000000;
	s_pEng->Clear(crEnv);             // 배경은 검은색
	s_pEng->s_lpD3DDev->BeginScene(); // 씬 렌더 ㅅ작...

	__Matrix44 mtxWorld;
	mtxWorld.Identity();
	CN3Base::s_lpD3DDev->SetTransform(D3DTS_WORLD, mtxWorld.toD3D());

	m_pCamera->EyePosSet(m_vEye);
	m_pCamera->AtPosSet(m_vAt);
	m_pCamera->UpVectorSet(0, 1, 0);
	m_pCamera->m_Data.fNP  = 0.1f;
	m_pCamera->m_Data.fFP  = 100.0f;
	m_pCamera->m_Data.fFOV = 0.96f;

	m_pCamera->Tick();
	m_pCamera->Apply();

	// 배경..
	m_pActiveBg->Render();

	// 캐릭터..
	if (m_pChrs[0])
	{
		m_pChrs[0]->m_nLOD = 0;
		m_pChrs[0]->Render();
	}
	if (m_pChrs[1])
	{
		m_pChrs[1]->m_nLOD = 0;
		m_pChrs[1]->Render();
	}
	if (m_pChrs[2])
	{
		m_pChrs[2]->m_nLOD = 0;
		m_pChrs[2]->Render();
	}

	CUIManager::RenderStateSet();
	m_pUICharacterSelect->Render();
	CUIManager::RenderStateRestore();

	CGameProcedure::Render(); // UI 나 그밖의 기본적인 것들 렌더링..

	if (m_bFadeOutRender)
		FadeOutRender();

	s_pEng->s_lpD3DDev->EndScene(); // 씬 렌더 시작...
	s_pEng->Present(CN3Base::s_hWndBase);
}

void CGameProcCharacterSelect::AddChr(e_ChrPos eCP, __CharacterSelectInfo* pInfo)
{
	int iPosIndex = 0;
	switch (eCP)
	{
		case POS_CENTER:
			iPosIndex = 0;
			break;

		case POS_LEFT:
			iPosIndex = 1;
			break;

		case POS_RIGHT:
			iPosIndex = 2;
			break;

		default:
			return;
	}

	if (nullptr == m_pChrs[iPosIndex])
		m_pChrs[iPosIndex] = new CN3Chr();
	else
		m_pChrs[iPosIndex]->Release();

	__TABLE_PLAYER_LOOKS* pLooks = s_pTbl_UPC_Looks.Find(pInfo->eRace); // User Player Character Skin 구조체 포인터..
	__ASSERT(pLooks, "Table 에서 기본 리소스 찾기 실패.");

	m_pChrs[iPosIndex]->PartAlloc(PART_POS_COUNT);
	m_pChrs[iPosIndex]->PlugAlloc(PLUG_POS_COUNT);
	std::string szJointFN, szAniFN, szPlug0FN, szPlug1FN;

	switch (pInfo->eRace)
	{
		case RACE_EL_BABARIAN:
			// 남자 바바리안은 직업이 하나..
			szJointFN = "ChrSelect\\upc_el_ba_wa.n3joint"; // 관절 세팅..
			szAniFN   = "ChrSelect\\upc_el_ba_wa.n3anim";  // 에니메이션 파일 이름.. :-D;
			szPlug0FN = "ChrSelect\\wea_el_great_sword.n3cplug";
			szPlug1FN = "";
			break;
		case RACE_EL_WOMEN:
			// 엘모 여자는 직업이 현재 세개(전사, 로그, 성직자).. 성직자는 그래픽이 없다..
			switch (pInfo->eClass)
			{
				case CLASS_EL_WARRIOR:
				case CLASS_EL_BLADE:
				case CLASS_EL_PROTECTOR:
					szJointFN = "ChrSelect\\upc_el_rf_wa.n3joint";
					szAniFN   = "ChrSelect\\upc_el_rf_wa.n3anim";            // 에니메이션 파일 이름.. :-D;
					szPlug0FN = "ChrSelect\\wea_el_long_sword_left.n3cplug"; // 왼손에 검을 찬다..
					szPlug1FN = "";
					break;
				case CLASS_EL_ROGUE:
				case CLASS_EL_RANGER:
				case CLASS_EL_ASSASIN:
					szJointFN = "ChrSelect\\upc_el_rf_rog.n3joint"; // 관절 세팅..
					szAniFN   = "ChrSelect\\upc_el_rf_wa.n3anim";   // 에니메이션 파일 이름.. :-D;
					szPlug0FN = "ChrSelect\\wea_el_rf_rog_bow.n3cplug";
					szPlug1FN = "ChrSelect\\wea_el_quiver.n3cplug";
					break;
				case CLASS_EL_WIZARD:
				case CLASS_EL_MAGE:
				case CLASS_EL_ENCHANTER:
					szJointFN = "ChrSelect\\upc_el_rf_wiz.n3joint"; // 관절 세팅..
					szAniFN   = "ChrSelect\\upc_el_rf_wa.n3anim";   // 에니메이션 파일 이름.. :-D;
					szPlug0FN = "ChrSelect\\upc_el_rf_wiz.n3cplug";
					szPlug1FN = "";
					break;
				case CLASS_EL_PRIEST:
				case CLASS_EL_CLERIC:
				case CLASS_EL_DRUID:
					szJointFN = "ChrSelect\\upc_el_rf_pri.n3joint"; // 관절 세팅..
					szAniFN   = "ChrSelect\\upc_el_rf_wa.n3anim";   // 에니메이션 파일 이름.. :-D;
					szPlug0FN = "ChrSelect\\wea_el_wand.n3cplug";
					szPlug1FN = "";
					break;
				default:
					return;
			}
			break;
		case RACE_EL_MAN:
			// 엘모 남자는 현재 직업이 세개..(로그, 성직자, 전사)
			switch (pInfo->eClass)
			{
				case CLASS_EL_WARRIOR:
				case CLASS_EL_BLADE:
				case CLASS_EL_PROTECTOR:
					szJointFN = "ChrSelect\\upc_el_rm_wa.n3joint";
					szAniFN   = "ChrSelect\\upc_el_rm_wa.n3anim";
					szPlug0FN = "ChrSelect\\wea_el_long_sword.n3cplug";
					break;
				case CLASS_EL_ROGUE:
				case CLASS_EL_RANGER:
				case CLASS_EL_ASSASIN:
					szJointFN = "ChrSelect\\upc_el_rm_rog.n3joint";
					szAniFN   = "ChrSelect\\upc_el_rm_rog.n3anim";
					szPlug0FN = "ChrSelect\\upc_el_rm_rog_bow.n3cplug";
					szPlug1FN = "ChrSelect\\wea_el_quiver.n3cplug";
					break;
				case CLASS_EL_WIZARD:
				case CLASS_EL_MAGE:
				case CLASS_EL_ENCHANTER:
					szJointFN = "ChrSelect\\upc_el_rm_ma.n3joint";
					szAniFN   = "ChrSelect\\upc_el_rm_rog.n3anim";
					szPlug0FN = "ChrSelect\\upc_el_rm_wiz.n3cplug";
					szPlug1FN = "";
					break;
				case CLASS_EL_PRIEST:
				case CLASS_EL_CLERIC:
				case CLASS_EL_DRUID:
					szJointFN = "ChrSelect\\upc_el_rm_pri.n3joint";
					szAniFN   = "ChrSelect\\upc_el_rm_rog.n3anim";
					szPlug0FN = "ChrSelect\\wea_el_wand.n3cplug";
					szPlug1FN = "";
					break;
				default:
					return;
			}
			break;
		case RACE_KA_ARKTUAREK:
			// 카루스 직업 하나.. 온리 전사..
			szJointFN = "ChrSelect\\upc_ka_at_wa.n3joint";
			szAniFN   = "ChrSelect\\upc_ka_at_wa.n3anim";
			szPlug0FN = "ChrSelect\\wea_ka_great_axe.n3cplug";
			szPlug1FN = "";
			break;
		case RACE_KA_TUAREK:
			switch (pInfo->eClass)
			{
				case CLASS_KA_ROGUE:
				case CLASS_KA_HUNTER:
				case CLASS_KA_PENETRATOR:
					szJointFN = "ChrSelect\\upc_ka_tu_rog.n3joint";
					szAniFN   = "ChrSelect\\upc_ka_at_wa.n3anim";
					szPlug0FN = "ChrSelect\\wea_ka_bow.n3cplug";
					szPlug1FN = "ChrSelect\\wea_ka_quiver.n3cplug";
					break;
				case CLASS_KA_PRIEST:
				case CLASS_KA_SHAMAN:
				case CLASS_KA_DARKPRIEST:
					szJointFN = "ChrSelect\\upc_ka_tu_pri.n3joint";
					szAniFN   = "ChrSelect\\upc_ka_at_wa.n3anim";
					szPlug0FN = "ChrSelect\\wea_ka_mace.n3cplug";
					szPlug1FN = "";
					break;
				default:
					return;
			}
			break;
		case RACE_KA_WRINKLETUAREK:
			szJointFN = "ChrSelect\\upc_ka_wt_ma.n3joint";
			szAniFN   = "ChrSelect\\upc_ka_at_wa.n3anim";
			szPlug0FN = "ChrSelect\\wea_ka_staff.n3cplug";
			szPlug1FN = "";
			break;
		case RACE_KA_PURITUAREK:
			szJointFN = "ChrSelect\\upc_el_rf_pri.n3joint"; // 관절 세팅..
			szAniFN   = "ChrSelect\\upc_el_rf_wa.n3anim";   // 에니메이션 파일 이름.. :-D;
			szPlug0FN = "ChrSelect\\wea_ka_mace.n3cplug";
			szPlug1FN = "";
			break;
		default:
			return;
	}

	m_pChrs[iPosIndex]->JointSet(szJointFN); // 관절 세팅..
	m_pChrs[iPosIndex]->AniCtrlSet(szAniFN); // 에니메이션 파일 이름.. :-D;
	m_pChrs[iPosIndex]->PlugSet(0, szPlug0FN);
	m_pChrs[iPosIndex]->PlugSet(1, szPlug1FN);

	// 상체..
	this->AddChrPart(iPosIndex, pLooks, PART_POS_UPPER, pInfo->dwItemUpper, pInfo->iItemUpperDurability);
	// 하체
	__TABLE_ITEM_BASIC* pItemUpper = this->s_pTbl_Items_Basic.Find(pInfo->dwItemUpper);
	if (pItemUpper && pItemUpper->byIsRobeType)
		m_pChrs[iPosIndex]->PartSet(PART_POS_LOWER, ""); // 하체일 경우 상체에 로브를 입었으면 ..
	else
		this->AddChrPart(iPosIndex, pLooks, PART_POS_LOWER, pInfo->dwItemLower, pInfo->iItemLowerDurability); //아님 입히고...
	// 팔
	this->AddChrPart(iPosIndex, pLooks, PART_POS_HANDS, pInfo->dwItemGloves, pInfo->iItemGlovesDurability);
	// 다리
	this->AddChrPart(iPosIndex, pLooks, PART_POS_FEET, pInfo->dwItemShoes, pInfo->iItemShoesDurability);

	std::string szResrcFN;

	// 얼굴 -
	if (!pLooks->szPartFNs[PART_POS_FACE].empty())
	{
		char szDir[_MAX_DIR] {}, szFName[_MAX_FNAME] {}, szExt[_MAX_EXT] {};
		_splitpath(pLooks->szPartFNs[PART_POS_FACE].c_str(), nullptr, szDir, szFName, szExt);
		szResrcFN = fmt::format("{}{}{:02}{}", szDir, szFName, pInfo->iFace, szExt);
		m_pChrs[iPosIndex]->PartSet(PART_POS_FACE, szResrcFN);
	}

	// 머리카락 혹은 헬멧 -
	__TABLE_ITEM_BASIC* pItemHelmet = this->s_pTbl_Items_Basic.Find(pInfo->dwItemHelmet);
	if (pItemHelmet && pItemHelmet->dwIDResrc) // 헬멧아이템의 리소스(그림이 있는거면..)
	{
		this->AddChrPart(iPosIndex, pLooks, PART_POS_HAIR_HELMET, pInfo->dwItemHelmet, pInfo->iItemHelmetDurability);
	}
	else if (!pLooks->szPartFNs[PART_POS_HAIR_HELMET].empty()) // 아이템이 없으면 기본 머리..
	{
		char szDir[_MAX_DIR] {}, szFName[_MAX_FNAME] {}, szExt[_MAX_EXT] {};
		_splitpath(pLooks->szPartFNs[PART_POS_HAIR_HELMET].c_str(), nullptr, szDir, szFName, szExt);
		szResrcFN = fmt::format("{}{}{:02}{}", szDir, szFName, pInfo->iHair, szExt);
		m_pChrs[iPosIndex]->PartSet(PART_POS_HAIR_HELMET, szResrcFN);
	}
	else
	{
		m_pChrs[iPosIndex]->PartSet(PART_POS_HAIR_HELMET, ""); // 헬멧도 아니고 머리카락도 없으면.. 대머리다!
	}

	__Quaternion qt;

	switch (s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_KARUS:
			switch (iPosIndex)
			{
				case 0:
					m_pChrs[0]->PosSet(0.0f, -1.16f, 2.72f); // ok..
					qt.RotationAxis(0.0f, 1.0f, 0.0f, DegreesToRadians(0.0f));
					m_pChrs[0]->RotSet(qt);
					break;

				case 1:
					m_pChrs[1]->PosSet(1.86f, -1.16f, 2.1f); // ok..
					qt.RotationAxis(0.0f, 1.0f, 0.0f, DegreesToRadians(42.0f));
					m_pChrs[1]->RotSet(qt);
					break;

				case 2:
					m_pChrs[2]->PosSet(-1.9f, -1.16f, 2.1f); // ok..
					qt.RotationAxis(0.0f, 1.0f, 0.0f, DegreesToRadians(-42.0f));
					m_pChrs[2]->RotSet(qt);
					break;

				default:
					break;
			}
			break;

		case NATION_ELMORAD:
			switch (iPosIndex)
			{
				case 0:
					m_pChrs[0]->PosSet(0.0f, -1.20f, 2.74f); // ok..
					qt.RotationAxis(0.0f, 1.0f, 0.0f, DegreesToRadians(0.0f));
					m_pChrs[0]->RotSet(qt);
					break;

				case 1:
					m_pChrs[1]->PosSet(1.86f, -1.20f, 2.0f); // ok
					qt.RotationAxis(0.0f, 1.0f, 0.0f, DegreesToRadians(42.0f));
					m_pChrs[1]->RotSet(qt);
					break;

				case 2:
					m_pChrs[2]->PosSet(-1.9f, -1.20f, 2.0f); // ok
					qt.RotationAxis(0.0f, 1.0f, 0.0f, DegreesToRadians(-46.0f));
					m_pChrs[2]->RotSet(qt);
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	for (int i = 0; i < 3; i++)
	{
		if (m_pChrs[i] != nullptr)
		{
			m_pChrs[i]->AniCurSet(SELECT_ANIM_PRE_SELECT, true, 0);
			m_pChrs[i]->Tick();
		}
	}
}

void CGameProcCharacterSelect::AddChrPart(
	int iPosIndex, const __TABLE_PLAYER_LOOKS* pLooks, e_PartPosition ePartPos, uint32_t dwItemID, int iItemDurability)
{
	if (nullptr == m_pChrs[iPosIndex] || nullptr == pLooks)
	{
		__ASSERT(0, "Invalid Chr Part");
		return;
	}

	__TABLE_ITEM_BASIC* pItem = s_pTbl_Items_Basic.Find(dwItemID / 1000 * 1000);
	if (dwItemID && nullptr == pItem)
	{
		__ASSERT(0, "NULL Item!!!");
		return;
	}

	std::string szResrcFN;
	CN3CPart* pPart            = nullptr;
	e_PartPosition ePartPosTmp = PART_POS_UNKNOWN;
	e_PlugPosition ePlugPosTmp = PLUG_POS_UNKNOWN;

	__TABLE_ITEM_EXT* pItemExt = nullptr;
	if (pItem != nullptr)
		pItemExt = s_pTbl_Items_Exts[pItem->byExtIndex].Find(dwItemID % 1000);

	MakeResrcFileNameForUPC(pItem, pItemExt, &szResrcFN, nullptr, ePartPosTmp, ePlugPosTmp, m_InfoChrs[iPosIndex].eRace);
	if (szResrcFN.empty())
		pPart = m_pChrs[iPosIndex]->PartSet(ePartPos, pLooks->szPartFNs[ePartPos]); // 기본 파트
	else
		pPart = m_pChrs[iPosIndex]->PartSet(ePartPos, szResrcFN);
	if (pPart && pItem && pItem->siMaxDurability > 0)
	{
		int iPercentage = iItemDurability * 100 / pItem->siMaxDurability;
		if (iPercentage <= 30)
			pPart->TexOverlapSet("Misc\\Dust_Hard.dxt");
		else if (iPercentage <= 70)
			pPart->TexOverlapSet("Misc\\Dust_Soft.dxt");
		else
			pPart->TexOverlapSet("");
	}
}

void CGameProcCharacterSelect::MsgRecv_DeleteChr(Packet& pkt)
{
	uint8_t byResult = 0, byIndex = 0;
	byResult = pkt.read<uint8_t>();
	byIndex  = pkt.read<uint8_t>();

	if (byResult == 0x01)
	{
		if (m_pChrs[byIndex] != nullptr)
		{
			m_pChrs[byIndex]->Release();
			delete m_pChrs[byIndex];
			m_pChrs[byIndex] = nullptr;
			m_InfoChrs[byIndex].clear();
		}

		if ((m_pChrs[0] == nullptr) && (m_pChrs[1] == nullptr) && (m_pChrs[2] == nullptr))
			CGameProcedure::ProcActiveSet((CGameProcedure*) s_pProcNationSelect); // 국가 선택으로 간다..
	}
}

int CGameProcCharacterSelect::MsgRecv_VersionCheck(Packet& pkt) // virtual
{
	int iVersion = CGameProcedure::MsgRecv_VersionCheck(pkt);
	if (iVersion == CURRENT_VERSION)
	{
		if (s_bIsRestarting)
			MsgSend_GameServerLogIn();
		else
			MsgSend_CharacterSelect(); // 게임 서버에 로그인..
	}

	return iVersion;
}

int CGameProcCharacterSelect::MsgRecv_GameServerLogIn(Packet& pDataPack)
{
	int iNation = CGameProcedure::MsgRecv_GameServerLogIn(pDataPack);
	if (0xff == iNation)
	{
		std::string szMsg = fmt::format_text_resource(IDS_FMT_GAME_SERVER_LOGIN_ERROR, "Current", iNation);
		MessageBoxPost(szMsg, "", MB_OK, BEHAVIOR_EXIT);
	}
	else
	{
		MsgSend_RequestAllCharacterInfo();
		s_bIsRestarting = false;
	}

	return iNation;
}

bool CGameProcCharacterSelect::MsgRecv_CharacterSelect(Packet& pkt) // virtual
{
	bool bSuccess   = CGameProcedure::MsgRecv_CharacterSelect(pkt);
	s_bIsRestarting = false;

	if (bSuccess)
		CharacterSelect(); // 캐릭터를 일으킨다..
	else
		CharacterSelectFailed();

	return bSuccess;
}

void CGameProcCharacterSelect::ProcessOnReturn()
{
	if (!m_bReceivedCharacterSelect)
		return;

	//엔터키 눌렸을때 라이트 때문에 깜빡이는것 없애기 위해...
	if (m_eCurProcess != PROCESS_ROTATEING)
	{
		int iIndex = 0;
		switch (m_eCurPos)
		{
			case POS_CENTER:
				iIndex = 0;
				break;

			case POS_LEFT:
				iIndex = 1;
				break;

			case POS_RIGHT:
				iIndex = 2;
				break;

			default:
				return;
		}

		// Light..
		switch (s_pPlayer->m_InfoBase.eNation)
		{
			case NATION_KARUS:
				m_lgt[iIndex].Theta = KARUS_THETA_MAX;

				s_pEng->s_lpD3DDev->LightEnable(iIndex + 4, TRUE);
				s_pEng->s_lpD3DDev->SetLight(iIndex + 4, m_lgt[iIndex].toD3D());
				break;

			case NATION_ELMORAD:
				m_lgt[iIndex].Theta = ELMORAD_THERA_MAX;

				s_pEng->s_lpD3DDev->LightEnable(iIndex + 4, TRUE);
				s_pEng->s_lpD3DDev->SetLight(iIndex + 4, m_lgt[iIndex].toD3D());
				break;

			default:
				break;
		}

		s_SndMgr.ReleaseStreamObj(&s_pSnd_BGM);
		CGameProcedure::ProcActiveSet((CGameProcedure*) s_pProcMain); // 캐릭터 고르기에 성공하면.. 메인으로 가자!!
		s_pUILoading->Render("Loading data...", 0);
	}
}

bool CGameProcCharacterSelect::CheckRotateLeft()
{
	__Matrix44 mtxrot;
	mtxrot.Identity();
	__Vector3 vOffs, v1, v2;
	__Vector3 vEyeTemp;
	bool bReturn = false;

	vEyeTemp     = m_vEye;
	mtxrot.RotationY(0.01f);
	vOffs     = vEyeTemp - m_vAt;
	vOffs    *= mtxrot;
	vEyeTemp  = vOffs + m_vAt;

	v1        = m_vEyeBackup - m_vAt;
	v2        = vEyeTemp - m_vAt;
	v1.Normalize();
	v2.Normalize();

	switch (s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_KARUS:
			if (v1.Dot(v2) < 0.75f)
			{
				bReturn       = true;
				m_eCurProcess = PROCESS_PRESELECT;
			}
			break;

		case NATION_ELMORAD:
			if (v1.Dot(v2) < 0.74f)
			{
				bReturn       = true;
				m_eCurProcess = PROCESS_PRESELECT;
			}
			break;

		default:
			break;
	}

	return bReturn;
}

bool CGameProcCharacterSelect::CheckRotateCenterToRight()
{
	if (m_vEye.x > 0.0f)
		return false;

	m_vEye        = m_vEyeBackup;
	m_eCurProcess = PROCESS_PRESELECT;
	return true;
}

bool CGameProcCharacterSelect::CheckRotateCenterToLeft()
{
	if (m_vEye.x < 0.0f)
		return false;

	m_vEye        = m_vEyeBackup;
	m_eCurProcess = PROCESS_PRESELECT;
	return true;
}

bool CGameProcCharacterSelect::CheckRotateRight()
{
	__Matrix44 mtxrot;
	mtxrot.Identity();
	__Vector3 vOffs, v1, v2;
	__Vector3 vEyeTemp;
	bool bReturn = false;

	vEyeTemp     = m_vEye;
	mtxrot.RotationY(-0.01f);
	vOffs     = vEyeTemp - m_vAt;
	vOffs    *= mtxrot;
	vEyeTemp  = vOffs + m_vAt;

	v1        = m_vEyeBackup - m_vAt;
	v2        = vEyeTemp - m_vAt;
	v1.Normalize();
	v2.Normalize();

	switch (s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_KARUS:
			if (v1.Dot(v2) < 0.77f)
			{
				bReturn       = true;
				m_eCurProcess = PROCESS_PRESELECT;
			}
			break;

		case NATION_ELMORAD:
			if (v1.Dot(v2) < 0.73f)
			{
				bReturn       = true;
				m_eCurProcess = PROCESS_PRESELECT;
			}
			break;

		default:
			break;
	}

	return bReturn;
}

void CGameProcCharacterSelect::DoJobLeft()
{
	if (m_eCurPos != POS_LEFT)
	{
		if (m_eCurProcess == PROCESS_ROTATEING)
			return;

		m_eCurProcess = PROCESS_ROTATEING;
	}

	switch (m_eCurPos)
	{
		case POS_CENTER:
			m_eDestPos        = POS_LEFT;
			s_iChrSelectIndex = 1;
			break;

		case POS_RIGHT:
			m_eDestPos        = POS_CENTER;
			s_iChrSelectIndex = 0;
			break;

		case POS_LEFT:
			break;
	}

	if (m_eCurPos != POS_LEFT && m_pSnd_Rotate != nullptr)
		m_pSnd_Rotate->Play();
}

void CGameProcCharacterSelect::DojobRight()
{
	if (m_eCurPos != POS_RIGHT)
	{
		if (m_eCurProcess == PROCESS_ROTATEING)
			return;

		m_eCurProcess = PROCESS_ROTATEING;
	}

	switch (m_eCurPos)
	{
		case POS_CENTER:
			m_eDestPos        = POS_RIGHT;
			s_iChrSelectIndex = 2;
			break;

		case POS_LEFT:
			m_eDestPos        = POS_CENTER;
			s_iChrSelectIndex = 0;
			break;

		case POS_RIGHT:
			break;
	}

	if (m_eCurPos != POS_RIGHT && m_pSnd_Rotate != nullptr)
		m_pSnd_Rotate->Play();
}

void CGameProcCharacterSelect::MsgSend_DeleteChr(const std::string& szKey)
{
	if (szKey.empty() || szKey.size() >= 32)
		return;

	// 현재 상태가 캐릭터를 선택하지 않은 상태..
	if (m_eCurProcess != PROCESS_PRESELECT)
		return;

	int iIndex = -1;
	switch (m_eCurPos)
	{
		case POS_CENTER:
			iIndex = 0;
			break;
		case POS_LEFT:
			iIndex = 1;
			break;
		case POS_RIGHT:
			iIndex = 2;
			break;
		default:
			return;
	}

	uint8_t byBuff[64];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_DEL_CHAR);                              // 커멘드.
	CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) iIndex);                          // 인덱스 - b
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) m_InfoChrs[iIndex].szID.size()); // 아이디 길이
	CAPISocket::MP_AddString(byBuff, iOffset, m_InfoChrs[iIndex].szID);                 // 아이디 문자열
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) szKey.size());                   // 주민등록번호 길이
	CAPISocket::MP_AddString(byBuff, iOffset, szKey);                                   // 주민등록번호 문자열

	s_pSocket->Send(byBuff, iOffset);                                                   // 보낸다
}

void CGameProcCharacterSelect::CheckJobState()
{
	switch (m_eCurProcess)
	{
		case PROCESS_ROTATEING:
			if (m_eDestPos == m_eCurPos)
				return;

			switch (m_eCurPos)
			{
				case POS_CENTER:
					switch (m_eDestPos)
					{
						case POS_LEFT:
							// Check..
							if (!CheckRotateLeft())
								RotateLeft();
							else
								m_eCurPos = m_eDestPos;
							break;

						case POS_RIGHT:
							// Check..
							if (!CheckRotateRight())
								RotateRight();
							else
								m_eCurPos = m_eDestPos;
							break;

						case POS_CENTER:
							break;
					}
					break;

				case POS_LEFT:
					if (m_eDestPos == POS_CENTER)
					{
						// Check..
						if (!CheckRotateCenterToRight())
							RotateRight();
						else
							m_eCurPos = m_eDestPos;
					}
					break;

				case POS_RIGHT:
					if (m_eDestPos == POS_CENTER)
					{
						// Check..
						if (!CheckRotateCenterToLeft())
							RotateLeft();
						else
							m_eCurPos = m_eDestPos;
					}
					break;
			}
			break;

		case PROCESS_PRESELECT:
			DoProcPreselect();
			break;

		case PROCESS_SELECTED:
			DoSelectedChrProc();
			break;

		case PROCESS_COMPLETE:
			break;
	}
}

void CGameProcCharacterSelect::RotateLeft()
{
	__Matrix44 mtxrot;
	mtxrot.Identity();
	__Vector3 vOffs;

	mtxrot.RotationY(CN3Base::s_fSecPerFrm * 1.2f);
	vOffs   = m_vEye - m_vAt;
	vOffs  *= mtxrot;
	m_vEye  = vOffs + m_vAt;
}

void CGameProcCharacterSelect::RotateRight()
{
	__Matrix44 mtxrot;
	mtxrot.Identity();
	__Vector3 vOffs;

	mtxrot.RotationY(-CN3Base::s_fSecPerFrm * 1.2f);
	vOffs   = m_vEye - m_vAt;
	vOffs  *= mtxrot;
	m_vEye  = vOffs + m_vAt;
}

void CGameProcCharacterSelect::CharacterSelect()
{
	int iIndex = 0;
	switch (m_eCurPos)
	{
		case POS_CENTER:
			iIndex = 0;
			break;

		case POS_LEFT:
			iIndex = 1;
			break;

		case POS_RIGHT:
			iIndex = 2;
			break;

		default:
			return;
	}

	if (m_eCurProcess == PROCESS_PRESELECT)
	{
		if (m_pChrs[iIndex] != nullptr)
		{
			m_eCurProcess = PROCESS_SELECTED;
			m_pChrs[iIndex]->AniCurSet(SELECT_ANIM_SELECTED, true, 0.6f);
			m_pChrs[iIndex]->Tick();

			switch (s_pPlayer->m_InfoBase.eNation)
			{
				case NATION_KARUS:
					m_fCurTheta = KARUS_THETA_MAX;
					break;

				case NATION_ELMORAD:
					m_fCurTheta = ELMORAD_THERA_MAX;
					break;

				default:
					break;
			}

			m_lgt[iIndex].Theta = m_fCurTheta;
			s_pEng->s_lpD3DDev->LightEnable(iIndex + 4, TRUE);
			s_pEng->s_lpD3DDev->SetLight(iIndex + 4, m_lgt[iIndex].toD3D());
		}
	}

	m_bReceivedCharacterSelect = true;   // 캐릭터 고르기 완료..
	s_pUIMgr->EnableOperationSet(false); // 일단 고르면 UI 안되게 한다...
}

void CGameProcCharacterSelect::CharacterSelectFailed()
{
	m_bReceivedCharacterSelect = false; // 캐릭터 고르기 실패..
	std::string szErr          = fmt::format_text_resource(IDS_ERR_CHARACTER_SELECT);
	MessageBoxPost(szErr, "", MB_OK, BEHAVIOR_EXIT);
	s_pUIMgr->EnableOperationSet(true);
}

void CGameProcCharacterSelect::DoSelectedChrProc()
{
	int iIndex = 0;
	switch (m_eCurPos)
	{
		case POS_CENTER:
			iIndex = 0;
			break;

		case POS_LEFT:
			iIndex = 1;
			break;

		case POS_RIGHT:
			iIndex = 2;
			break;

		default:
			return;
	}

	// Light..
	s_lpD3DDev->LightEnable(iIndex + 4, TRUE);
	s_lpD3DDev->SetLight(iIndex + 4, m_lgt[iIndex].toD3D());

	if (!m_pChrs[iIndex]->IsAnimEnd())
	{
		m_pChrs[iIndex]->AniCurSet(SELECT_ANIM_SELECTED, true, 0.6f);
		m_pChrs[iIndex]->Tick();
		m_bFadeOutRender = true;
		FadeOutProcess();
	}
	else
	{
		s_SndMgr.ReleaseStreamObj(&s_pSnd_BGM);
		CGameProcedure::ProcActiveSet((CGameProcedure*) s_pProcMain); // 캐릭터 받았고.. 에니메이션도 끝났으면 메인으로 넘어가자..!!
	}
}

void CGameProcCharacterSelect::FadeOutProcess()
{
	m_fFadeOut += (30.0f / CN3Base::s_fFrmPerSec) * 1.5f;

	if (m_fFadeOut > 255)
		m_fFadeOut = 255;
}

void CGameProcCharacterSelect::FadeOutRender()
{
	__VertexTransformedColor pVertices[4] {};

	pVertices[0].Set(0.0f, 0.0f, 0.000002f, 0.99f, D3DCOLOR_ARGB((uint8_t) m_fFadeOut, 0x00, 0x00, 0x00));
	pVertices[1].Set((float) s_CameraData.vp.Width, 0.0f, 0.000002f, 0.99f, D3DCOLOR_ARGB((uint8_t) m_fFadeOut, 0x00, 0x00, 0x00));
	pVertices[2].Set((float) s_CameraData.vp.Width, (float) s_CameraData.vp.Height, 0.000002f, 0.99f,
		D3DCOLOR_ARGB((uint8_t) m_fFadeOut, 0x00, 0x00, 0x00));
	pVertices[3].Set(0.0f, (float) s_CameraData.vp.Height, 0.000002f, 0.99f, D3DCOLOR_ARGB((uint8_t) m_fFadeOut, 0x00, 0x00, 0x00));

	DWORD dwUsefog = TRUE;
	s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwUsefog);

	DWORD dwUseLighting = TRUE;
	s_lpD3DDev->GetRenderState(D3DRS_LIGHTING, &dwUseLighting);

	DWORD dwUseColorVertex = FALSE;
	s_lpD3DDev->GetRenderState(D3DRS_COLORVERTEX, &dwUseColorVertex);

	DWORD bUseAlphaBlend = TRUE;
	s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &bUseAlphaBlend);

	BOOL bLight[8] = {};
	for (int i = 0; i < 8; i++)
		s_lpD3DDev->GetLightEnable(i, &bLight[i]);

	if (bUseAlphaBlend == FALSE)
		s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	if (dwUseLighting)
		s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	if (dwUsefog)
		s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);

	// set render states
	if (dwUseColorVertex == FALSE)
		s_lpD3DDev->SetRenderState(D3DRS_COLORVERTEX, TRUE);

	for (int i = 0; i < 8; i++)
		s_lpD3DDev->LightEnable(i, FALSE);

	DWORD dwTexStageCO = 0, dwTexStageCARG1 = 0, dwTexStageAO = 0, dwTexStageAARG1 = 0, dwRSSB = 0, dwRSDB = 0;

	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwTexStageCO);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwTexStageCARG1);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwTexStageAO);
	s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwTexStageAARG1);
	s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwRSSB);
	s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwRSDB);

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	s_lpD3DDev->SetTexture(0, nullptr);

	s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);
	s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, pVertices, sizeof(__VertexTransformedColor));

	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwTexStageCO);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwTexStageCARG1);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwTexStageAO);
	s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwTexStageAARG1);
	s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwRSSB);
	s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwRSDB);

	s_lpD3DDev->SetRenderState(D3DRS_COLORVERTEX, dwUseColorVertex);
	s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, bUseAlphaBlend);
	s_lpD3DDev->SetRenderState(D3DRS_LIGHTING, dwUseLighting);
	s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwUsefog);
	for (int i = 0; i < 8; i++)
		s_lpD3DDev->LightEnable(i, bLight[i]);
}

void CGameProcCharacterSelect::DoProcPreselect()
{
	D3DVIEWPORT9 vp {};
	s_lpD3DDev->GetViewport(&vp);

	float left = 0.0f, right = 0.0f, top = 0.0f, bottom = 0.0f;
	left          = vp.Width * 0.36f;
	right         = vp.Width * 0.64f;
	top           = vp.Height * 0.44f;
	bottom        = vp.Height * 0.86f;

	POINT pt      = s_pLocalInput->MouseGetPos();

	int iPosIndex = 0;
	switch (m_eCurPos)
	{
		case POS_CENTER:
			iPosIndex = 0;
			break;

		case POS_LEFT:
			iPosIndex = 1;
			break;

		case POS_RIGHT:
			iPosIndex = 2;
			break;

		default:
			return;
	}

	if ((pt.x > left) && (pt.x < right) && (pt.y < bottom) && (pt.y > top))
	{
		// Doing..
		if (m_pChrs[iPosIndex] != nullptr)
		{
			m_pChrs[iPosIndex]->AniCurSet(SELECT_ANIM_PRE_SELECT, true);
			m_pChrs[iPosIndex]->Tick();
		}

		IncreseLightFactor();

		m_pUICharacterSelect->DisplayChrInfo(&m_InfoChrs[iPosIndex]);
	}
	else
	{
		// Doing..
		if (m_pChrs[iPosIndex] != nullptr)
		{
			m_pChrs[iPosIndex]->AniCurSet(SELECT_ANIM_DUMMY, true);
			m_pChrs[iPosIndex]->Tick();
		}

		DecreseLightFactor();

		m_pUICharacterSelect->DontDisplayInfo();
	}

	// Light..
	for (int i = 0; i < 3; i++)
		m_lgt[i].Theta = 0.0f;
	m_lgt[iPosIndex].Theta = m_fCurTheta;

	if (m_lgt[iPosIndex].Theta != 0.0f)
	{
		s_lpD3DDev->LightEnable(iPosIndex + 4, TRUE);
		s_lpD3DDev->SetLight(iPosIndex + 4, m_lgt[iPosIndex].toD3D());
	}
	else
	{
		s_lpD3DDev->LightEnable(iPosIndex + 4, FALSE);
	}
}

void CGameProcCharacterSelect::IncreseLightFactor()
{
	switch (s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_KARUS:
			m_fCurTheta += KARUS_INCRESE_OFFSET;
			if (m_fCurTheta > KARUS_THETA_MAX)
				m_fCurTheta = KARUS_THETA_MAX;
			break;

		case NATION_ELMORAD:
			m_fCurTheta += ELMORAD_INCRESE_OFFSET;
			if (m_fCurTheta > ELMORAD_THERA_MAX)
				m_fCurTheta = ELMORAD_THERA_MAX;
			break;

		default:
			break;
	}
}

void CGameProcCharacterSelect::DecreseLightFactor()
{
	switch (s_pPlayer->m_InfoBase.eNation)
	{
		case NATION_KARUS:
			m_fCurTheta -= KARUS_INCRESE_OFFSET;
			if (m_fCurTheta < 0.0f)
				m_fCurTheta = 0.0f;
			break;

		case NATION_ELMORAD:
			m_fCurTheta -= ELMORAD_INCRESE_OFFSET;
			if (m_fCurTheta < 0.0f)
				m_fCurTheta = 0.0f;
			break;

		default:
			break;
	}
}

void CGameProcCharacterSelect::MsgRecv_AllCharacterInfo(Packet& pkt)
{
	int iResult = pkt.read<uint8_t>(); // 결과..
	if (0x1 == iResult)
	{
		for (int i = 0; i < MAX_AVAILABLE_CHARACTER; i++)
		{
			auto& chr     = m_InfoChrs[i];
			int iIDLength = pkt.read<int16_t>();                            // 캐릭터 아이디 길이 s,

			pkt.readString(chr.szID, iIDLength);                            // 캐릭터 아이디 문자열 str

			chr.eRace                    = (e_Race) (pkt.read<uint8_t>());  // 종족 b
			chr.eClass                   = (e_Class) (pkt.read<int16_t>()); // 직업 b
			chr.iLevel                   = pkt.read<uint8_t>();             // 레벨 b
			chr.iFace                    = pkt.read<uint8_t>();             // 얼굴모양 b
			chr.iHair                    = pkt.read<uint8_t>();             // 머리모양 b
			chr.iZone                    = pkt.read<uint8_t>();             // zone number

			chr.dwItemHelmet             = pkt.read<uint32_t>();            // 투구 dw
			chr.iItemHelmetDurability    = pkt.read<int16_t>();             // 내구성값
			chr.dwItemUpper              = pkt.read<uint32_t>();            // 상체 dw
			chr.iItemUpperDurability     = pkt.read<int16_t>();             // 내구성값
			chr.dwItemCloak              = pkt.read<uint32_t>();            // 어깨(망토) dw
			chr.iItemCloakDurability     = pkt.read<int16_t>();             // 내구성값

			chr.dwRightHand              = pkt.read<uint32_t>();
			chr.iItemRightHandDurability = pkt.read<int16_t>();
			chr.dwLeftHand               = pkt.read<uint32_t>();
			chr.iItemLeftHandDurability  = pkt.read<int16_t>();

			chr.dwItemLower              = pkt.read<uint32_t>(); // 하체 dw
			chr.iItemLowerDurability     = pkt.read<int16_t>();  // 내구성값
			chr.dwItemGloves             = pkt.read<uint32_t>(); // 장갑 dw
			chr.iItemGlovesDurability    = pkt.read<int16_t>();  // 내구성값
			chr.dwItemShoes              = pkt.read<uint32_t>(); // 신발 dw
			chr.iItemShoesDurability     = pkt.read<int16_t>();  // 내구성값
		}

		// 캐릭터 추가..
		if (!m_InfoChrs[0].szID.empty())
			AddChr(POS_CENTER, &m_InfoChrs[0]);

		if (!m_InfoChrs[1].szID.empty())
			AddChr(POS_LEFT, &m_InfoChrs[1]);

		if (!m_InfoChrs[2].szID.empty())
			AddChr(POS_RIGHT, &m_InfoChrs[2]);
	}
	else
	{
		MsgSend_RequestAllCharacterInfo(); // 다시 정보 요청..
	}
}

void CGameProcCharacterSelect::MsgSend_RequestAllCharacterInfo()
{
	uint8_t byBuff[4];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_ALLCHAR_INFO_REQ); // 커멘드.
	s_pSocket->Send(byBuff, iOffset);                              // 보낸다
}

void CGameProcCharacterSelect::MsgSend_CharacterSelect()           // virtual
{
	CGameProcedure::MsgSend_CharacterSelect();
	s_pUIMgr->EnableOperationSet(false);                           // UI 를 조작 못하게 한다..
}

bool CGameProcCharacterSelect::ProcessPacket(Packet& pkt)
{
	size_t rpos = pkt.rpos();
	if (CGameProcedure::ProcessPacket(pkt))
		return true;

	pkt.rpos(rpos);

	int iCmd = pkt.read<uint8_t>();             // 커멘드 파싱..
	switch (iCmd)                               // 커멘드에 다라서 분기..
	{
		case WIZ_ALLCHAR_INFO_REQ:              // 캐릭터 선택 메시지..
			MsgRecv_AllCharacterInfo(pkt);
			s_pUIMgr->EnableOperationSet(true); // 캐릭터 정보가 다오면 UI 조작하게 한다..
			return true;

		case WIZ_DEL_CHAR:
			MsgRecv_DeleteChr(pkt);
			return true;

		default:
			break;
	}

	return false;
}

void CGameProcCharacterSelect::CharacterSelectOrCreate()
{
	CGameProcedure::MessageBoxClose(-1);

	int iIndex = CGameProcedure::s_iChrSelectIndex;
	if (nullptr == m_pChrs[iIndex])                                              // 캐릭터가 없으면..
	{
		CGameProcedure::ProcActiveSet((CGameProcedure*) s_pProcCharacterCreate); // 캐릭터 생성 프로시저를 호출한다..
	}
	else
	{
		s_pPlayer->m_InfoExt.iZoneInit = 0x01;
		s_pPlayer->m_InfoExt.iZoneCur  = m_InfoChrs[iIndex].iZone;
		s_pPlayer->IDSet(-1, m_InfoChrs[iIndex].szID, 0xffffffff);
		this->MsgSend_CharacterSelect();
	}
}
