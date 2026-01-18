// N3UITooltip.cpp: implementation of the CN3UITooltip class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UITooltip.h"
#include "N3UIString.h"
#include "N3UIStatic.h"

CN3UITooltip::CN3UITooltip()
{
	m_eType      = UI_TYPE_TOOLTIP;

	m_fHoverTime = 0.0f;
	m_bVisible   = false;
	m_bSetText   = false;
	m_ptCursor   = {};
}

CN3UITooltip::~CN3UITooltip()
{
}

void CN3UITooltip::Release()
{
	CN3UIStatic::Release();

	m_fHoverTime = 0.0f;
	m_bVisible   = false;
	m_bSetText   = false;
	m_ptCursor   = {};
}

void CN3UITooltip::Render()
{
	if (!IsVisible() || !m_bSetText)
		return;

	if (m_pImageBkGnd != nullptr)
	{
		CN3UIStatic::Render();
	}
	// 이미지가 없으면 디폴트로 그려주자
	else
	{
		__VertexTransformedColor pVB[8] {};

		constexpr WORD pIB[16]            = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4 };
		constexpr D3DCOLOR BkColor        = 0x80000000;
		constexpr D3DCOLOR BorderColorOut = 0xff808080;
		constexpr D3DCOLOR BorderColorIn  = 0xffc0c0c0;

		pVB[0].Set(static_cast<float>(m_rcRegion.left), static_cast<float>(m_rcRegion.top),
			UI_DEFAULT_Z, UI_DEFAULT_RHW, BkColor);
		pVB[1].Set(static_cast<float>(m_rcRegion.right), static_cast<float>(m_rcRegion.top),
			UI_DEFAULT_Z, UI_DEFAULT_RHW, BkColor);
		pVB[2].Set(static_cast<float>(m_rcRegion.right), static_cast<float>(m_rcRegion.bottom),
			UI_DEFAULT_Z, UI_DEFAULT_RHW, BkColor);
		pVB[3].Set(static_cast<float>(m_rcRegion.left), static_cast<float>(m_rcRegion.bottom),
			UI_DEFAULT_Z, UI_DEFAULT_RHW, BkColor);
		pVB[4].Set(static_cast<float>(m_rcRegion.left) + 1, static_cast<float>(m_rcRegion.top) + 1,
			UI_DEFAULT_Z, UI_DEFAULT_RHW, BorderColorIn);
		pVB[5].Set(static_cast<float>(m_rcRegion.right) - 1, static_cast<float>(m_rcRegion.top) + 1,
			UI_DEFAULT_Z, UI_DEFAULT_RHW, BorderColorIn);
		pVB[6].Set(static_cast<float>(m_rcRegion.right) - 1,
			static_cast<float>(m_rcRegion.bottom) - 1, UI_DEFAULT_Z, UI_DEFAULT_RHW, BorderColorIn);
		pVB[7].Set(static_cast<float>(m_rcRegion.left) + 1,
			static_cast<float>(m_rcRegion.bottom) - 1, UI_DEFAULT_Z, UI_DEFAULT_RHW, BorderColorIn);

		// set texture stage state
		s_lpD3DDev->SetTexture(0, nullptr);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

		// draw
		s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR);
		s_lpD3DDev->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN, 2, pVB, sizeof(__VertexTransformedColor)); // 배경색 칠하기

		__VertexTransformedColor* pTemp = pVB;
		for (int i = 0; i < 4; ++i, ++pTemp)
			pTemp->color = BorderColorOut;     // 바깥 테두리 색을 바꾼다.

		s_lpD3DDev->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 8, 8, pIB, D3DFMT_INDEX16, pVB,
			sizeof(__VertexTransformedColor)); // 테두리 칠하기

		// 글씨 그리기
		m_pBuffOutRef->Render();
	}
}

void CN3UITooltip::SetText(const std::string& szText, D3DCOLOR crTooltip)
{
	if (!m_bVisible)
		return;

	static std::string szPrevText;

	if (lstrcmpA(szPrevText.c_str(), szText.c_str()) != 0)
	{
		m_bSetText   = false;
		m_fHoverTime = 0.0f;
	}

	if (m_bSetText)
		return;

	int iStrLen = static_cast<int>(szText.size());
	if (iStrLen == 0 || m_pBuffOutRef == nullptr)
		return;

	m_pBuffOutRef->ClearOnlyStringBuffer();

	SIZE size {};
	if (m_pBuffOutRef->GetTextExtent(szText, iStrLen, &size))
	{
		int iRealWidth = m_pBuffOutRef->GetStringRealWidth(szText);
		if (size.cx < iRealWidth)
			size.cx = iRealWidth;

		bool offsetString   = false;

		uint32_t dwNewStyle = 0;
		if (szText.find('\n') != std::string::npos)
		{
			dwNewStyle = UISTYLE_STRING_ALIGNLEFT | UISTYLE_STRING_ALIGNTOP;
		}
		else if (iStrLen < 25)
		{
			dwNewStyle = UISTYLE_STRING_SINGLELINE | UISTYLE_STRING_ALIGNCENTER
						 | UISTYLE_STRING_ALIGNVCENTER;
		}
		// single line이므로 적당한 크기를 계산한다.
		else
		{
			SIZE CharSize = { 0, 0 };
			if (!m_pBuffOutRef->GetTextExtent("가", 2, &CharSize))
				return;

			constexpr int MAX_WIDTH = 500;
			int iLineCount          = (size.cx / MAX_WIDTH) + 1;
			if (iLineCount > 1)
			{
				dwNewStyle    = UISTYLE_STRING_ALIGNLEFT | UISTYLE_STRING_ALIGNTOP;
				size.cx       = MAX_WIDTH;
				size.cy      *= iLineCount;
				offsetString  = true;
			}
			else
			{
				dwNewStyle = UISTYLE_STRING_SINGLELINE | UISTYLE_STRING_ALIGNCENTER
							 | UISTYLE_STRING_ALIGNVCENTER;
			}
		}

		m_pBuffOutRef->SetStyle(dwNewStyle);

		constexpr int Padding  = 12;
		size.cx               += Padding;
		size.cy               += Padding;
		SetSize(size.cx, size.cy);

		if (offsetString)
		{
			m_pBuffOutRef->SetPos(m_rcRegion.left + Padding / 2, m_rcRegion.top + (Padding / 2));
		}
	}

	m_pBuffOutRef->SetString(szText);
	m_pBuffOutRef->SetColor(crTooltip);

	// 위치 조정
	POINT ptNew             = m_ptCursor;
	ptNew.x                -= (m_rcRegion.right - m_rcRegion.left) / 2;
	ptNew.y                -= (m_rcRegion.bottom - m_rcRegion.top) + 10;

	const D3DVIEWPORT9& vp  = s_CameraData.vp;
	int iRegionWidth        = m_rcRegion.right - m_rcRegion.left;
	int iRegionHeight       = m_rcRegion.bottom - m_rcRegion.top;

	if (ptNew.x + iRegionWidth > static_cast<int>(vp.X + vp.Width))
		ptNew.x = static_cast<int>(vp.X + vp.Width) - iRegionWidth;

	if (ptNew.x < static_cast<int>(vp.X))
		ptNew.x = static_cast<int>(vp.X);

	if (ptNew.y + iRegionHeight > static_cast<int>(vp.Y + vp.Height))
		ptNew.y = static_cast<int>(vp.Y + vp.Height) - iRegionHeight;

	if (ptNew.y < static_cast<int>(vp.Y))
		ptNew.y = static_cast<int>(vp.Y);

	SetPos(ptNew.x, ptNew.y);

	m_bSetText = true;
}

void CN3UITooltip::Tick()
{
	int iOldTime                  = static_cast<int>(m_fHoverTime);
	m_fHoverTime                 += s_fSecPerFrm;

	constexpr float fDisplayTime  = 0.3f;
	if (iOldTime < fDisplayTime && m_fHoverTime >= iOldTime)
		SetVisible(true); // tool tip 표시
}

uint32_t CN3UITooltip::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!IsVisible())
		return dwRet;

	// 마우스를 움직이면 m_fHoverTime를 0으로 만들기
	if (ptCur.x != ptOld.x || ptCur.y != ptOld.y)
	{
		m_fHoverTime = 0.0f;
		m_bSetText   = false;

		SetVisible(false); // tool tip을 없앤다.
	}
	// 안움직이면 커서 위치 저장
	else
	{
		m_ptCursor = ptCur;
	}

	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}
