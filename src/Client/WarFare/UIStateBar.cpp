// UIStateBar.cpp: implementation of the CUIStateBar class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIStateBar.h"
#include "GameDef.h"
#include "LocalInput.h"
#include "GameProcMain.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "MagicSkillMng.h"
#include "UIManager.h"
#include "UILevelGuide.h"
#include "N3UIDBCLButton.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3Texture.h>

CUIStateBar::CUIStateBar()
{
	m_pText_Position       = nullptr;
	m_pProgress_HP         = nullptr;
	m_pProgress_HP_slow    = nullptr;
	m_pProgress_HP_drop    = nullptr;
	m_pProgress_HP_lasting = nullptr;
	m_pProgress_MSP        = nullptr;
	m_pProgress_ExpC       = nullptr;
	m_pProgress_ExpP       = nullptr;

	m_pText_FPS            = nullptr;
	m_pText_MP             = nullptr;
	m_pText_HP             = nullptr;
	m_pText_Exp            = nullptr;

	// 미니맵...
	m_pGroup_MiniMap       = nullptr;
	m_pImage_Map           = nullptr;
	m_pBtn_ZoomIn          = nullptr;
	m_pBtn_ZoomOut         = nullptr;
	m_pBtn_Quest           = nullptr;
	m_pBtn_Power           = nullptr;

	for (int i = 0; i < 6; i++)
		m_vArrows[i] = {};

	m_fZoom      = 6.0f;
	m_fMapSizeX  = 0.0f;
	m_fMapSizeZ  = 0.0f;
	m_fYawPlayer = 0;
	m_vPosPlayer.Zero();
	m_vViewPos.Zero();

	m_pMagic.clear();
}

CUIStateBar::~CUIStateBar()
{
	it_MagicImg it, ite;
	ite = m_pMagic.end();
	for (it = m_pMagic.begin(); it != ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		if (pMagicImg)
		{
			delete pMagicImg->pIcon;
			delete pMagicImg;
		}
	}

	m_pMagic.clear();
}

void CUIStateBar::Release()
{
	it_MagicImg it, ite;
	ite = m_pMagic.end();
	for (it = m_pMagic.begin(); it != ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		if (pMagicImg)
		{
			delete pMagicImg->pIcon;
			delete pMagicImg;
		}
	}

	m_pMagic.clear();

	CN3UIBase::Release();

	m_pText_Position       = nullptr;
	m_pText_MP             = nullptr;
	m_pText_HP             = nullptr;
	m_pText_Exp            = nullptr;

	m_pProgress_HP         = nullptr;
	m_pProgress_HP_slow    = nullptr;
	m_pProgress_HP_drop    = nullptr;
	m_pProgress_HP_lasting = nullptr;
	m_pProgress_MSP        = nullptr;
	m_pProgress_ExpC       = nullptr;
	m_pProgress_ExpP       = nullptr;

	// 미니맵...
	m_pGroup_MiniMap       = nullptr;
	m_pImage_Map           = nullptr;
	m_pBtn_ZoomIn          = nullptr;
	m_pBtn_ZoomOut         = nullptr;
	m_pBtn_Power           = nullptr;
	m_pBtn_Quest           = nullptr;

	for (int i = 0; i < 6; i++)
		m_vArrows[i] = {};

	m_fZoom      = 6.0f;
	m_fMapSizeX  = 0.0f;
	m_fMapSizeZ  = 0.0f;
	m_fYawPlayer = 0;
	m_vPosPlayer.Zero();
	m_vViewPos.Zero();
}

bool CUIStateBar::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	CN3UIString* pText = nullptr;
	N3_VERIFY_UI_COMPONENT(pText, GetChildByID<CN3UIString>("Text_Version"));
	if (pText != nullptr)
	{
		std::string version = fmt::format("Ver. {:.3f}", CURRENT_VERSION / 1000.0f);
		pText->SetString(version);
	}

	N3_VERIFY_UI_COMPONENT(m_pText_Position, GetChildByID<CN3UIString>("Text_Position"));

	N3_VERIFY_UI_COMPONENT(m_pProgress_HP, GetChildByID<CN3UIProgress>("Progress_HP"));
	N3_VERIFY_UI_COMPONENT(m_pProgress_MSP, GetChildByID<CN3UIProgress>("Progress_MSP"));
	N3_VERIFY_UI_COMPONENT(m_pProgress_ExpC, GetChildByID<CN3UIProgress>("Progress_ExpC"));
	N3_VERIFY_UI_COMPONENT(m_pProgress_ExpP, GetChildByID<CN3UIProgress>("Progress_ExpP"));

	if (m_pProgress_HP)
		m_pProgress_HP->SetRange(0, 100);
	if (m_pProgress_MSP)
		m_pProgress_MSP->SetRange(0, 100);
	if (m_pProgress_ExpC)
		m_pProgress_ExpC->SetRange(0, 100);
	if (m_pProgress_ExpP)
		m_pProgress_ExpP->SetRange(0, 100);

	// NOTE: new components not previously used

	N3_VERIFY_UI_COMPONENT(m_pProgress_HP_slow, GetChildByID<CN3UIProgress>("Progress_HP_slow"));
	if (m_pProgress_HP_slow != nullptr)
	{
		m_pProgress_HP_slow->SetRange(0, 100);
		m_pProgress_HP_slow->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pProgress_HP_drop, GetChildByID<CN3UIProgress>("Progress_HP_drop"));
	if (m_pProgress_HP_drop != nullptr)
	{
		m_pProgress_HP_drop->SetRange(0, 100);
		m_pProgress_HP_drop->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pProgress_HP_lasting, GetChildByID<CN3UIProgress>("Progress_HP_lasting"));
	if (m_pProgress_HP_lasting != nullptr)
	{
		m_pProgress_HP_lasting->SetRange(0, 100);
		m_pProgress_HP_lasting->SetVisible(false);
	}

	// NOTE: new components to display the text
	N3_VERIFY_UI_COMPONENT(m_pText_HP, GetChildByID<CN3UIString>("Text_HP"));
	N3_VERIFY_UI_COMPONENT(m_pText_MP, GetChildByID<CN3UIString>("Text_MSP"));
	N3_VERIFY_UI_COMPONENT(m_pText_Exp, GetChildByID<CN3UIString>("Text_ExpP"));

	CN3UIString* m_pText_SysTime = GetChildByID<CN3UIString>("SystemTime");
	if (m_pText_SysTime)
		m_pText_SysTime->SetVisible(false);

	m_pText_FPS = GetChildByID<CN3UIString>("string_fps");

	// MiniMap
	N3_VERIFY_UI_COMPONENT(m_pGroup_MiniMap, GetChildByID("Group_MiniMap"));
	if (m_pGroup_MiniMap != nullptr)
	{
		m_pGroup_MiniMap->SetVisible(false);

		N3_VERIFY_UI_COMPONENT(m_pImage_Map, m_pGroup_MiniMap->GetChildByID<CN3UIImage>("Img_MiniMap"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_ZoomIn, m_pGroup_MiniMap->GetChildByID<CN3UIButton>("Btn_ZoomIn"));
		N3_VERIFY_UI_COMPONENT(m_pBtn_ZoomOut, m_pGroup_MiniMap->GetChildByID<CN3UIButton>("Btn_ZoomOut"));
	}

	m_pBtn_Quest = GetChildByID<CN3UIButton>("btn_quest");
	m_pBtn_Power = GetChildByID<CN3UIButton>("btn_power");

	return true;
}

bool CUIStateBar::LoadMap(const std::string& szMiniMapFN, float fMapSizeX, float fMapSizeZ)
{
	m_fMapSizeX = fMapSizeX;
	m_fMapSizeZ = fMapSizeZ;
	if (nullptr == m_pImage_Map)
		return false;

	m_pImage_Map->SetTex(szMiniMapFN);
	return true;
}

void CUIStateBar::UpdateExp(int64_t iExp, int64_t iExpNext, bool bUpdateImmediately)
{
	__ASSERT(iExpNext, "Next Exp is 0");
	if (iExpNext <= 0)
		return;

	if (m_pProgress_ExpC == nullptr || m_pProgress_ExpP == nullptr)
		return;

	int iPercentage = (int) (100.0 * ((double) iExp / (double) iExpNext));

	if (iExpNext > 10)
	{
		uint64_t iExpNext2 = iExpNext / 10;
		uint64_t iExp2     = iExp % iExpNext2;
		int iPercentage2   = (int) (100 * iExp2 / iExpNext2);

		if (bUpdateImmediately)
			m_pProgress_ExpC->SetCurValue(iPercentage2); //SetCurValue --> set경우
		else
			m_pProgress_ExpC->SetCurValue(iPercentage2, 0.7f, 50.0f);
	}
	else
	{
		m_pProgress_ExpC->SetCurValue(0); //SetCurValue --> set경우
	}

	if (bUpdateImmediately)
		m_pProgress_ExpP->SetCurValue(iPercentage); //SetCurValue --> set경우
	else
		m_pProgress_ExpP->SetCurValue(iPercentage, 0.3f, 100.0f);

	// NOTE: adding the EXP text
	__ASSERT(iExp >= 0 && iExpNext > 0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (m_pText_Exp == nullptr)
		return;

	double iPercentage2 = 100.0 * ((double) iExp / (double) iExpNext);

	std::string buff    = fmt::format("{:.2f} %", iPercentage2);
	m_pText_Exp->SetString(buff);
}

void CUIStateBar::UpdateMSP(int iMSP, int iMSPMax, bool bUpdateImmediately)
{
	__ASSERT(iMSPMax, "Max MP is 0");
	if (iMSPMax <= 0)
		return;

	if (m_pProgress_MSP == nullptr)
		return;

	int iPercentage = 100 * iMSP / iMSPMax;

	if (bUpdateImmediately)
		m_pProgress_MSP->SetCurValue(iPercentage); //SetCurValue --> set경우
	else
		m_pProgress_MSP->SetCurValue(iPercentage, 0.3f, 100.0f);

	// NOTE: adding the MP text
	__ASSERT(iMSP >= 0 && iMSPMax > 0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (m_pText_MP == nullptr)
		return;

	std::string buff = fmt::format("{} / {}", iMSP, iMSPMax);
	m_pText_MP->SetString(buff);
}

void CUIStateBar::UpdateHP(int iHP, int iHPMax, bool bUpdateImmediately)
{
	__ASSERT(iHPMax, "Max HP is 0");
	if (iHPMax <= 0)
		return;

	int iPercentage = 100 * iHP / iHPMax;

	if (bUpdateImmediately)
		m_pProgress_HP->SetCurValue(iPercentage); //SetCurValue --> set경우
	else
		m_pProgress_HP->SetCurValue(iPercentage, 0.3f, 100.0f);

	// NOTE: adding the HP text
	__ASSERT(iHP >= 0 && iHP < 10000 && iHPMax >= 0 && iHPMax < 10000, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (m_pText_HP == nullptr)
		return;

	std::string buff = fmt::format("{} / {}", iHP, iHPMax);
	m_pText_HP->SetString(buff);
}

void CUIStateBar::UpdatePosition(const __Vector3& vPos, float fYaw)
{
	if (m_pText_Position == nullptr)
		return;

	std::string pos = fmt::format("{:.1f}, {:.1f}", vPos.x, vPos.z);
	m_pText_Position->SetString(pos);

	// 미니맵.
	m_vPosPlayer = vPos;
	m_fYawPlayer = fYaw;
}

void CUIStateBar::Render()
{
	if (false == m_bVisible)
		return;

	CN3UIBase::Render();

	if (m_pGroup_MiniMap == nullptr || !m_pGroup_MiniMap->IsVisible())
		return; // 미니맵이 안켜져 있음 돌아간다..

	if (m_pImage_Map == nullptr)
		return;

	if (m_fMapSizeX <= 0 || m_fMapSizeZ <= 0)
		return;

	__VertexTransformedColor vPositions[4] {}, vOutLines[4] {};

	vPositions[0].Set(0, 0, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xffffffff);
	vPositions[1] = vPositions[0];
	vPositions[2] = vPositions[0];
	vPositions[3] = vPositions[0];
	vOutLines[0].Set(0, 0, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff000000);
	vOutLines[1] = vOutLines[0];
	vOutLines[2] = vOutLines[0];
	vOutLines[3] = vOutLines[0];

	__PositionInfo info;

	RECT rc        = m_pImage_Map->GetRegion();
	float fWidth   = (float) (rc.right - rc.left);
	float fHeight  = (float) (rc.bottom - rc.top);
	float fCenterX = (float) (rc.left) + fWidth / 2.0f;
	float fCenterY = (float) (rc.top) + fHeight / 2.0f;

	DWORD dwZ = 0, dwFog = 0, dwAlpha = 0, dwCOP = 0, dwCA1 = 0;
	DWORD dwSrcBlend = 0, dwDestBlend = 0, dwVertexShader = 0, dwAOP = 0, dwAA1 = 0;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwCOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwCA1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAA1);
	CN3Base::s_lpD3DDev->GetFVF(&dwVertexShader);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	CN3Base::s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR); // 플레이어의 위치및 방향 표시..

	__Vector3 vPos;
	it_PositionInfo it = m_Positions.begin(), itEnd = m_Positions.end();
	for (; it != itEnd; it++)
	{
		info   = *it;

		vPos   = m_vViewPos - info.vPos;
		vPos.x = (float) (int) (fCenterX - m_fZoom * fWidth * (vPos.x / m_fMapSizeX));
		vPos.y = (float) (int) (fCenterY + m_fZoom * fHeight * (vPos.z / m_fMapSizeZ));

		if (vPos.x < rc.left || vPos.x > rc.right || vPos.y < rc.top || vPos.y > rc.bottom)
		{
			continue;
		}

		vPositions[0].color = info.crType;
		vPositions[1].color = info.crType;
		vPositions[2].color = info.crType;
		vPositions[3].color = info.crType;

		vPositions[0].x     = vPos.x - 1.0f;
		vPositions[0].y     = vPos.y - 1.0f;
		vPositions[1].x     = vPos.x + 1.0f;
		vPositions[1].y     = vPos.y - 1.0f;
		vPositions[2].x     = vPos.x + 1.0f;
		vPositions[2].y     = vPos.y + 1.0f;
		vPositions[3].x     = vPos.x - 1.0f;
		vPositions[3].y     = vPos.y + 1.0f;

		vOutLines[0].x      = vPos.x - 2.0f;
		vOutLines[0].y      = vPos.y - 2.0f;
		vOutLines[1].x      = vPos.x + 2.0f;
		vOutLines[1].y      = vPos.y - 2.0f;
		vOutLines[2].x      = vPos.x + 2.0f;
		vOutLines[2].y      = vPos.y + 2.0f;
		vOutLines[3].x      = vPos.x - 2.0f;
		vOutLines[3].y      = vPos.y + 2.0f;

		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vOutLines, sizeof(__VertexTransformedColor));
		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vPositions, sizeof(__VertexTransformedColor));
	}

	CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, m_vArrows, sizeof(__VertexTransformedColor));

	it = m_PositionsTop.begin(), itEnd = m_PositionsTop.end(); // 나중에 그릴 점덜...
	for (; it != itEnd; it++)
	{
		info   = *it;

		vPos   = m_vViewPos - info.vPos;
		vPos.x = (float) (int) (fCenterX - m_fZoom * fWidth * (vPos.x / m_fMapSizeX));
		vPos.y = (float) (int) (fCenterY + m_fZoom * fHeight * (vPos.z / m_fMapSizeZ));

		if (vPos.x < rc.left || vPos.x > rc.right || vPos.y < rc.top || vPos.y > rc.bottom)
		{
			continue;
		}

		vPositions[0].color = info.crType;
		vPositions[1].color = info.crType;
		vPositions[2].color = info.crType;
		vPositions[3].color = info.crType;

		vPositions[0].x     = vPos.x - 1.0f;
		vPositions[0].y     = vPos.y - 1.0f;
		vPositions[1].x     = vPos.x + 1.0f;
		vPositions[1].y     = vPos.y - 1.0f;
		vPositions[2].x     = vPos.x + 1.0f;
		vPositions[2].y     = vPos.y + 1.0f;
		vPositions[3].x     = vPos.x - 1.0f;
		vPositions[3].y     = vPos.y + 1.0f;

		vOutLines[0].x      = vPos.x - 2.0f;
		vOutLines[0].y      = vPos.y - 2.0f;
		vOutLines[1].x      = vPos.x + 2.0f;
		vOutLines[1].y      = vPos.y - 2.0f;
		vOutLines[2].x      = vPos.x + 2.0f;
		vOutLines[2].y      = vPos.y + 2.0f;
		vOutLines[3].x      = vPos.x - 2.0f;
		vOutLines[3].y      = vPos.y + 2.0f;

		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vOutLines, sizeof(__VertexTransformedColor));
		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vPositions, sizeof(__VertexTransformedColor));
	}

	//Render..
	//m_pMagic.clear();
	it_MagicImg itMagic, iteMagic;
	iteMagic = m_pMagic.end();
	for (itMagic = m_pMagic.begin(); itMagic != iteMagic; itMagic++)
	{
		__DurationMagicImg* pMagicImg = (*itMagic);
		pMagicImg->pIcon->Render();
	}

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwCOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwCA1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAA1);
	CN3Base::s_lpD3DDev->SetFVF(dwVertexShader);
}

void CUIStateBar::Tick()
{
	if (!m_bVisible)
		return;

	CN3UIBase::Tick();

	// NOTE(srmeier): set the FPS string to be displayed
	static int iCount = 0;
	static std::string strFPS;

	if (iCount++ == 60)
	{
		iCount = 0;
		strFPS = fmt::format("{:.1f}", CN3Base::s_fFrmPerSec);
		if (m_pText_FPS != nullptr)
			m_pText_FPS->SetString(strFPS);
	}

	TickMiniMap();   // 맵 이미지...
	TickMagicIcon(); // 아이콘 처리..
}

void CUIStateBar::TickMiniMap()
{
	if (nullptr == m_pImage_Map)
		return;
	if (m_fMapSizeX <= 0 || m_fMapSizeZ <= 0)
		return;

	m_vViewPos         = m_vPosPlayer;

	RECT rc            = m_pImage_Map->GetRegion();

	int minimapWidth   = rc.right - rc.left;
	int minimapWidth2  = minimapWidth / 2;
	int minimapHeight  = rc.bottom - rc.top;
	int minimapHeight2 = minimapHeight / 2;

	float factorX      = m_fZoom * minimapWidth / m_fMapSizeX;
	float factorY      = m_fZoom * minimapHeight / m_fMapSizeZ;

	// limit map from scrolling out of bounds

	if (minimapWidth2 > factorX * m_vViewPos.x)
	{
		m_vViewPos.x = minimapWidth2 / factorX;
	}

	if (m_fZoom * minimapWidth - minimapWidth2 < factorX * m_vViewPos.x)
	{
		m_vViewPos.x = (m_fZoom * minimapWidth - minimapWidth2) / factorX;
	}

	if (minimapHeight2 > factorY * m_vViewPos.z)
	{
		m_vViewPos.z = minimapHeight2 / factorY;
	}

	if (m_fZoom * minimapHeight - minimapHeight2 < factorY * m_vViewPos.z)
	{
		m_vViewPos.z = (m_fZoom * minimapHeight - minimapHeight2) / factorY;
	}

	float fOffset = (0.5f / m_fZoom);
	float fX      = (m_vViewPos.x / m_fMapSizeX);
	float fY      = (m_vViewPos.z / m_fMapSizeZ);

	float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0;
	x1 = fX - fOffset;
	y1 = fY + fOffset;
	x2 = fX + fOffset;
	y2 = fY - fOffset;

	m_pImage_Map->SetUVRect(x1, 1.0f - y1, x2, 1.0f - y2);

	float fH = (rc.bottom - rc.top) / 30.0f;

	__Matrix44 mtxRot;
	mtxRot.RotationZ(m_fYawPlayer);
	mtxRot.PosSet(rc.left + minimapWidth2 + factorX * (m_vPosPlayer.x - m_vViewPos.x),
		rc.top + minimapHeight2 - factorY * (m_vPosPlayer.z - m_vViewPos.z), 0);

	// 화살표 세팅...
	m_vArrows[0].Set(0, -fH, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	m_vArrows[1].Set(0, fH / 2.0f, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	m_vArrows[2].Set(-fH, fH, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);

	m_vArrows[3]   = m_vArrows[0];
	m_vArrows[4]   = m_vArrows[2];
	m_vArrows[4].x = -m_vArrows[4].x;
	m_vArrows[5]   = m_vArrows[1];

	for (int i = 0; i < 6; i++)
		m_vArrows[i] *= mtxRot; // 위치 및 회전 변환..
}

void CUIStateBar::TickMagicIcon()
{
	__TABLE_UPC_SKILL* pRemoveSkill = nullptr;

	for (__DurationMagicImg* pMagicImg : m_pMagic)
	{
		pMagicImg->fDuration -= CN3Base::s_fSecPerFrm;

		if (pMagicImg->fDuration <= 0.0f)
		{
			pMagicImg->pIcon->SetVisible(false);
			pRemoveSkill = CGameBase::s_pTbl_Skill.Find(pMagicImg->dwSkillID);
			break;
		}

		if (pMagicImg->fDuration <= 10.0f)
			pMagicImg->pIcon->SetVisible(pMagicImg->fDuration - (int) pMagicImg->fDuration < 0.5f);
	}

	if (pRemoveSkill != nullptr)
		DelMagic(pRemoveSkill);
}

bool CUIStateBar::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_ZoomIn)
		{
			ZoomSet(m_fZoom * 1.1f);
			return true;
		}
		else if (pSender == m_pBtn_ZoomOut)
		{
			ZoomSet(m_fZoom * 0.9f);
			return true;
		}
		else if (pSender == m_pBtn_Quest)
		{
			if (CGameProcedure::s_pProcMain->m_pUILevelGuide != nullptr)
				CGameProcedure::s_pProcMain->CommandToggleLevelGuide();

			return true;
		}
		else if (pSender == m_pBtn_Power)
		{
			//TODO: Launch msgbox with warning message, on confirm -> open store (Web_Browser UI or maybe external?)
			return false;
		}
	}
	else if (dwMsg == UIMSG_ICON_DBLCLK)
	{
		it_MagicImg it, ite;
		ite = m_pMagic.end();
		for (it = m_pMagic.begin(); it != ite; it++)
		{
			__DurationMagicImg* pMagicImg = (*it);

			if (pSender == pMagicImg->pIcon)
			{
				uint32_t MagicID          = pMagicImg->dwSkillID;
				__TABLE_UPC_SKILL* pSkill = CGameBase::s_pTbl_Skill.Find(MagicID);
				if (!pSkill)
					return false;

				if (CGameProcedure::s_pProcMain->m_pMagicSkillMng->IsPositiveMagic(MagicID))
				{
					if (pSkill->dw1stTableType == 3 || pSkill->dw2ndTableType == 3)
						pMagicImg->fDuration = 0.0f;

					//없애라..
					uint8_t byBuff[32];
					int iOffset = 0;
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) WIZ_MAGIC_PROCESS);
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t) N3_SP_MAGIC_CANCEL);
					CAPISocket::MP_AddDword(byBuff, iOffset, (int) MagicID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) CGameBase::s_pPlayer->IDNumber());
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) CGameBase::s_pPlayer->IDNumber());

					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);

					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);

					CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..
					return true;
				}
			}
		}
	}
	return false;
}

// NOTE: appears to add NPC/Players as dots on map?
void CUIStateBar::PositionInfoAdd(int iID, const __Vector3& vPos, D3DCOLOR crID, bool bDrawTop)
{
	__PositionInfo info;
	info.iID    = iID;
	info.vPos   = vPos;
	info.crType = crID;

	if (bDrawTop)
		m_PositionsTop.push_back(info);
	else
		m_Positions.push_back(info);
}

void CUIStateBar::PositionInfoClear()
{
	m_Positions.clear();
	m_PositionsTop.clear();
}

void CUIStateBar::ZoomSet(float fZoom)
{
	if (fZoom < 1.0f)
	{
		m_fZoom = 1.0f;
	}
	else if (fZoom > 6.0f)
	{
		m_fZoom = 6.0f;
	}
	else
	{
		m_fZoom = fZoom;
	}
}

bool CUIStateBar::ToggleMiniMap()
{
	if (nullptr == m_pGroup_MiniMap)
		return false;

	bool bVisible = m_pGroup_MiniMap->IsVisible();
	m_pGroup_MiniMap->SetVisible(!bVisible);

	return !bVisible;
}

void CUIStateBar::AddMagic(__TABLE_UPC_SKILL* pSkill, float fDuration)
{
	std::string buffer            = fmt::format("UI\\skillicon_{:02}_{}.dxt", pSkill->dwID % 100, pSkill->dwID / 100);

	__DurationMagicImg* pMagicImg = new __DurationMagicImg;
	pMagicImg->fDuration          = fDuration;
	pMagicImg->pIcon              = new CN3UIDBCLButton;
	pMagicImg->dwSkillID          = pSkill->dwID;

	CN3UIDBCLButton* pIcon        = pMagicImg->pIcon;
	pIcon->Init(this);
	pIcon->SetTex(buffer);
	pIcon->SetTooltipText(pSkill->szName);
	pIcon->SetUVRect(0, 0, 1, 1);

	CN3Texture* pTex = pIcon->GetTex();
	if (!pTex)
	{
		delete pIcon;
		delete pMagicImg;
		return;
	}

	RECT rt;
	rt.left = rt.top = 0;
	rt.right         = pTex->Width();
	rt.bottom        = pTex->Height();

	int iconCount    = static_cast<int>(m_pMagic.size());
	int PosX         = static_cast<int>(s_CameraData.vp.Width) - (rt.right * (iconCount + 1));
	pIcon->SetRegion(rt);
	pIcon->SetPos(PosX, 0);

	m_pMagic.push_back(pMagicImg);
}

void CUIStateBar::DelMagic(__TABLE_UPC_SKILL* pSkill)
{
	std::string buffer = fmt::format("UI\\skillicon_{:02}_{}.dxt", pSkill->dwID % 100, pSkill->dwID / 100);

	it_MagicImg it, ite, itRemove;
	itRemove = ite = m_pMagic.end();
	for (it = m_pMagic.begin(); it != ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		CN3UIDBCLButton* pIcon        = pMagicImg->pIcon;
		CN3Texture* pTex              = pIcon->GetTex();
		if (pTex && lstrcmpi(pTex->FileName().c_str(), buffer.c_str()) == 0)
		{
			itRemove = it;
		}
		if (itRemove != ite)
		{
			POINT pos = pIcon->GetPos();
			RECT rt   = pIcon->GetRegion();
			pIcon->SetPos(pos.x + (rt.right - rt.left), 0);
		}
	}
	if (itRemove != ite)
	{
		__DurationMagicImg* pMagicImg = (*itRemove);
		CN3UIDBCLButton* pIcon        = pMagicImg->pIcon;
		delete pIcon;
		delete pMagicImg;
		m_pMagic.erase(itRemove);
	}
}

void CUIStateBar::ClearMagic()
{
	it_MagicImg it, ite;
	ite = m_pMagic.end();
	for (it = m_pMagic.begin(); it != ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		CN3UIDBCLButton* pIcon        = pMagicImg->pIcon;
		delete pIcon;
		delete pMagicImg;
	}
	m_pMagic.clear();
}

uint32_t CUIStateBar::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;

	for (const __DurationMagicImg* pMagicImg : m_pMagic)
	{
		dwRet |= pMagicImg->pIcon->MouseProc(CGameProcedure::s_pLocalInput->MouseGetFlag(), CGameProcedure::s_pLocalInput->MouseGetPos(),
			CGameProcedure::s_pLocalInput->MouseGetPosOld());
	}

	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);

	return dwRet;
}

bool CUIStateBar::OnKeyPress(int iKey)
{
	// hotkey가 포커스 잡혀있을때는 다른 ui를 닫을수 없으므로 DIK_ESCAPE가 들어오면 포커스를 다시잡고
	if (iKey == DIK_ESCAPE)
	{
		// 열려있는 다른 유아이를 닫아준다.
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
		CN3UIBase* pFocus = CGameProcedure::s_pUIMgr->GetFocusedUI();
		if (pFocus != nullptr && pFocus != this)
			pFocus->OnKeyPress(iKey);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
