// DFont.cpp: implementation of the CDFont class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "DFont.h"

const int MAX_NUM_VERTICES   = 50 * 6;
const float Z_DEFAULT        = 0.9f;
const float RHW_DEFAULT      = 1.0f;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HDC CDFont::s_hDC            = nullptr;
int CDFont::s_iInstanceCount = 0;
HFONT CDFont::s_hFontOld     = nullptr;

CDFont::CDFont(const std::string& szFontName, uint32_t dwHeight, uint32_t dwFlags)
{
	if (0 == s_iInstanceCount)
	{
		s_hDC       = CreateCompatibleDC(nullptr);

		// 임시 폰트를 만들고 s_hFontOld를 얻는다.
		HFONT hFont = CreateFont(0, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, "굴림");
		if (hFont != nullptr)
		{
			s_hFontOld = (HFONT) (SelectObject(s_hDC, hFont));
			SelectObject(s_hDC, s_hFontOld);
			DeleteObject(hFont);
		}
	}
	s_iInstanceCount++;

	__ASSERT(!szFontName.empty(), "??");

	m_szFontName      = szFontName;
	m_dwFontHeight    = dwHeight;
	m_dwFontFlags     = dwFlags;

	m_dwTexHeight     = 0;
	m_dwTexWidth      = 0;
	m_fTextScale      = 1.0f;

	m_pd3dDevice      = nullptr;
	m_pTexture        = nullptr;
	m_pVB             = nullptr;

	m_iPrimitiveCount = 0;
	m_PrevLeftTop.x = m_PrevLeftTop.y = 0;

	m_hFont                           = nullptr;
	m_dwFontColor                     = 0xffffffff;
	m_Size.cx                         = 0;
	m_Size.cy                         = 0;
}

CDFont::~CDFont()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();

	s_iInstanceCount--;
	if (s_iInstanceCount <= 0)
	{
		if (s_hFontOld)
			SelectObject(s_hDC, s_hFontOld);
		DeleteDC(s_hDC);
		s_hDC = nullptr;
	}
}

HRESULT CDFont::SetFont(const std::string& szFontName, uint32_t dwHeight, uint32_t dwFlags)
{
	__ASSERT(!szFontName.empty(), "");
	if (nullptr == s_hDC)
	{
		__ASSERT(0, "NULL DC Handle");
		return E_FAIL;
	}

	m_szFontName   = szFontName;
	m_dwFontHeight = dwHeight;
	m_dwFontFlags  = dwFlags;

	if (m_hFont)
	{
		if (s_hFontOld)
			SelectObject(s_hDC, s_hFontOld);
		DeleteObject(m_hFont);
		m_hFont = nullptr;
	}

	// Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
	// antialiased font, but this is not guaranteed.
	INT nHeight = -MulDiv(
		m_dwFontHeight, (INT) (GetDeviceCaps(s_hDC, LOGPIXELSY) * m_fTextScale), 72);
	uint32_t dwBold   = (m_dwFontFlags & D3DFONT_BOLD) ? FW_BOLD : FW_NORMAL;
	uint32_t dwItalic = (m_dwFontFlags & D3DFONT_ITALIC) ? TRUE : FALSE;
	m_hFont = CreateFont(nHeight, 0, 0, 0, dwBold, dwItalic, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH,
		m_szFontName.c_str());
	if (nullptr == m_hFont)
	{
		__ASSERT(0, "NULL Font Handle");
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CDFont::InitDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
	// Keep a local copy of the device
	m_pd3dDevice = pd3dDevice;
	m_fTextScale = 1.0f; // Draw fonts into texture without scaling

	return S_OK;
}

HRESULT CDFont::RestoreDeviceObjects()
{
	HRESULT hr        = S_OK;

	m_iPrimitiveCount = 0;

	//	__ASSERT(nullptr == s_hDC && nullptr == m_hFont, "??");
	//	m_hDC = CreateCompatibleDC(nullptr);
	__ASSERT(nullptr == m_hFont, "??");

	if (nullptr == s_hDC)
	{
		__ASSERT(0, "Can't Create DC");
		return E_FAIL;
	}
	SetMapMode(s_hDC, MM_TEXT);

	// Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
	// antialiased font, but this is not guaranteed.
	INT nHeight = -MulDiv(
		m_dwFontHeight, (INT) (GetDeviceCaps(s_hDC, LOGPIXELSY) * m_fTextScale), 72);
	uint32_t dwBold   = (m_dwFontFlags & D3DFONT_BOLD) ? FW_BOLD : FW_NORMAL;
	uint32_t dwItalic = (m_dwFontFlags & D3DFONT_ITALIC) ? TRUE : FALSE;
	m_hFont = CreateFont(nHeight, 0, 0, 0, dwBold, dwItalic, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH,
		m_szFontName.c_str());
	if (nullptr == m_hFont)
		return E_FAIL;

	// Create vertex buffer for the letters
	__ASSERT(m_pVB == nullptr, "??");
	int iVBSize    = MAX_NUM_VERTICES * sizeof(__VertexTransformed);
	uint32_t dwFVF = FVF_TRANSFORMED;
	hr = m_pd3dDevice->CreateVertexBuffer(iVBSize, 0, dwFVF, D3DPOOL_MANAGED, &m_pVB, nullptr);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT CDFont::InvalidateDeviceObjects()
{
	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}

	if (m_hFont)
	{
		if (s_hDC && s_hFontOld)
			SelectObject(s_hDC, s_hFontOld);
		DeleteObject(m_hFont);
		m_hFont = nullptr;
	}
	return S_OK;
}

HRESULT CDFont::DeleteDeviceObjects()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
	m_pd3dDevice = nullptr;

	return S_OK;
}

HRESULT CDFont::SetText(const std::string& szText, uint32_t dwFlags)
{
	if (nullptr == s_hDC || nullptr == m_hFont)
		return E_FAIL;

	if (szText.empty())
	{
		m_iPrimitiveCount = 0;
		if (m_pTexture)
		{
			m_pTexture->Release();
			m_pTexture = nullptr;
		}
		return S_OK;
	}

	int iStrLen    = static_cast<int>(szText.size());

	HRESULT hr     = S_OK;
	// \n을 빼고 한줄로 만들어서 글자 길이 계산하기
	int iCount     = 0;
	int iTempCount = 0;
	SIZE size;

	std::string szTemp(iStrLen, '?');
	while (iCount < iStrLen)
	{
		if ('\n' == szText[iCount]) // \n
		{
			++iCount;
		}
		else if (0x80 & szText[iCount]) // 2BYTE 문자
		{
			if ((iCount + 2) > iStrLen) // 이상한 문자열이다..
			{
				//				__ASSERT(0, "이상한 문자열이다.!!!");
				break;
			}
			else
			{
				memcpy(&(szTemp[iTempCount]), &(szText[iCount]), 2);
				iTempCount += 2;
				iCount     += 2;
			}
		}
		else // 1BYTE 문자
		{
			memcpy(&(szTemp[iTempCount]), &(szText[iCount]), 1);
			++iTempCount;
			++iCount;
		}
		__ASSERT(iCount <= iStrLen, "??"); // 이상한 문자가 들어왔을 경우
	}

										   //	szTemp[iTempCount] = 0x00;

	// 텍스쳐 사이즈 결정하기
	SelectObject(s_hDC, m_hFont);
	GetTextExtentPoint32(s_hDC, szTemp.c_str(), static_cast<int>(szTemp.size()), &size);
	szTemp = "";

	if (size.cx <= 0 || size.cy <= 0)
	{
		__ASSERT(0, "Invalid Text Size - ?????");
		return E_FAIL;
	}
	int iExtent = size.cx * size.cy;

	SIZE size2; // 한글 반글자의 크기..
	GetTextExtentPoint32(s_hDC, "진", lstrlen("진"), &size2);
	size2.cx         = ((size2.cx / 2) + (size2.cx % 2));

	int iTexSizes[7] = { 32, 64, 128, 256, 512, 1024, 2048 };
	for (int i = 0; i < 7; ++i)
	{
		if (iExtent <= (iTexSizes[i] - size2.cx - size2.cy - 1) * iTexSizes[i])
		{
			m_dwTexWidth = m_dwTexHeight = iTexSizes[i];
			break;
		}
	}

	// Establish the font and texture size
	m_fTextScale = 1.0f; // Draw fonts into texture without scaling

	// If requested texture is too big, use a smaller texture and smaller font,
	// and scale up when rendering.
	D3DCAPS9 d3dCaps;
	m_pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (m_dwTexWidth > d3dCaps.MaxTextureWidth)
	{
		m_fTextScale = (float) d3dCaps.MaxTextureWidth / (float) m_dwTexWidth;
		m_dwTexWidth = m_dwTexHeight = d3dCaps.MaxTextureWidth;
	}

	// 기존 텍스쳐 크기가 새로 만들 텍스쳐 크기와 다를 경우 다시 만든다.
	if (m_pTexture)
	{
		D3DSURFACE_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		m_pTexture->GetLevelDesc(0, &sd);
		if (sd.Width != m_dwTexWidth)
		{
			m_pTexture->Release();
			m_pTexture = nullptr;
		}
	}

	// Create a new texture for the font
	if (nullptr == m_pTexture)
	{
		int iMipMapCount = 1;
		if (dwFlags & D3DFONT_FILTERED)
			iMipMapCount = 0; // 필터링 텍스트는 밉맵을 만든다..

		hr = m_pd3dDevice->CreateTexture(m_dwTexWidth, m_dwTexHeight, iMipMapCount, 0,
			D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pTexture, nullptr);
		if (FAILED(hr))
			return hr;
	}

	// Prepare to create a bitmap
	uint32_t* pBitmapBits = nullptr;
	BITMAPINFO bmi {};
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = (int) m_dwTexWidth;
	bmi.bmiHeader.biHeight      = -(int) m_dwTexHeight;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount    = 32;

	// Create a DC and a bitmap for the font
	HBITMAP hbmBitmap           = CreateDIBSection(
        s_hDC, &bmi, DIB_RGB_COLORS, (VOID**) &pBitmapBits, nullptr, 0);

	if (nullptr == hbmBitmap)
	{
		__ASSERT(0, "CreateDIBSection 실패");
		if (m_pTexture)
		{
			m_pTexture->Release();
			m_pTexture = nullptr;
		}
		return E_FAIL;
	}

	HGDIOBJ hObjPrev = ::SelectObject(s_hDC, hbmBitmap);

	// Set text properties
	SetTextColor(s_hDC, RGB(255, 255, 255));
	SetBkColor(s_hDC, RGB(0, 0, 0));
	SetTextAlign(s_hDC, TA_TOP);

	// Loop through all printable character and output them to the bitmap..
	// Meanwhile, keep track of the corresponding tex coords for each character.
	Make2DVertex(size.cy, szText);

	// Lock the surface and write the alpha values for the set pixels
	D3DLOCKED_RECT d3dlr;
	m_pTexture->LockRect(0, &d3dlr, nullptr, 0);
	uint16_t* pDst16 = (uint16_t*) d3dlr.pBits;

	for (uint32_t y = 0; y < m_dwTexHeight; y++)
	{
		for (uint32_t x = 0; x < m_dwTexWidth; x++)
		{
			// 4-bit measure of pixel intensity
			uint8_t bAlpha = (uint8_t) ((pBitmapBits[m_dwTexWidth * y + x] & 0xff) >> 4);
			if (bAlpha > 0)
				*pDst16++ = (bAlpha << 12) | 0x0fff;
			else
				*pDst16++ = 0;
		}
	}

	// Done updating texture, so clean up used objects
	m_pTexture->UnlockRect(0);

	::SelectObject(s_hDC, hObjPrev); // 반드시 전의걸 선택해야..
	DeleteObject(hbmBitmap);         // 제대로 지워진다..

	////////////////////////////////////////////////////////////
	// 필터링 텍스처는... MipMap 만든다..
	if (dwFlags & D3DFONT_FILTERED)
	{
		int iMMC = m_pTexture->GetLevelCount();
		for (int i = 1; i < iMMC; i++)
		{
			LPDIRECT3DSURFACE9 lpSurfSrc  = nullptr;
			LPDIRECT3DSURFACE9 lpSurfDest = nullptr;
			m_pTexture->GetSurfaceLevel(i - 1, &lpSurfSrc);
			m_pTexture->GetSurfaceLevel(i, &lpSurfDest);

			if (lpSurfSrc && lpSurfDest)
			{
				::D3DXLoadSurfaceFromSurface(lpSurfDest, nullptr, nullptr, lpSurfSrc, nullptr,
					nullptr, D3DX_FILTER_TRIANGLE, 0); // 서피스 복사
			}

			if (lpSurfSrc)
				lpSurfSrc->Release();
			if (lpSurfDest)
				lpSurfDest->Release();
		}
	}
	// 필터링 텍스처는... MipMap 만든다..
	////////////////////////////////////////////////////////////

	return S_OK;
}

void CDFont::Make2DVertex(const int iFontHeight, const std::string& szText)
{
	if (m_pVB == nullptr || s_hDC == nullptr || m_hFont == nullptr)
	{
		__ASSERT(0, "NULL Vertex Buffer or DC or Font Handle ");
		return;
	}

	if (szText.empty())
		return;

	int iStrLen                    = static_cast<int>(szText.size());

	// lock vertex buffer
	__VertexTransformed* pVertices = nullptr;
	uint32_t dwNumTriangles        = 0;
	if (FAILED(m_pVB->Lock(0, 0, (void**) &pVertices, 0)))
		return;

	uint32_t sx        = 0; // start x y
	uint32_t x         = 0;
	uint32_t y         = 0;
	float vtx_sx       = 0;
	float vtx_sy       = 0; //	vertex start x y
	int iCount         = 0;

	char szTempChar[3] = "";
	uint32_t dwColor   = 0xffffffff; // 폰트의 색
	m_dwFontColor      = 0xffffffff;
	SIZE size;

	float fMaxX = 0.0f, fMaxY = 0.0f; // 글씨가 찍히는 범위의 최대 최소값을 조사하기 위해서.

	while (iCount < iStrLen)
	{
		if ('\n' == szText[iCount]) // \n
		{
			++iCount;

			// vertex 만들기
			if (sx != x)
			{
				FLOAT tx1 = ((FLOAT) (sx)) / m_dwTexWidth;
				FLOAT ty1 = ((FLOAT) (y)) / m_dwTexHeight;
				FLOAT tx2 = ((FLOAT) (x)) / m_dwTexWidth;
				FLOAT ty2 = ((FLOAT) (y + iFontHeight)) / m_dwTexHeight;

				FLOAT w   = (tx2 - tx1) * m_dwTexWidth / m_fTextScale;
				FLOAT h   = (ty2 - ty1) * m_dwTexHeight / m_fTextScale;

				__ASSERT(dwNumTriangles + 2 < MAX_NUM_VERTICES, "??"); // Vertex buffer가 모자란다.
				if (dwNumTriangles + 2 >= MAX_NUM_VERTICES)
					break;

				FLOAT fLeft   = vtx_sx + 0 - 0.5f;
				FLOAT fRight  = vtx_sx + w - 0.5f;
				FLOAT fTop    = vtx_sy + 0 - 0.5f;
				FLOAT fBottom = vtx_sy + h - 0.5f;
				pVertices->Set(fLeft, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty2);
				++pVertices;
				pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1);
				++pVertices;
				pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2);
				++pVertices;
				pVertices->Set(fRight, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty1);
				++pVertices;
				pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2);
				++pVertices;
				pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1);
				++pVertices;

				dwNumTriangles += 2;

				if (fMaxX < fRight)
					fMaxX = fRight;
				if (fMaxY < fBottom)
					fMaxY = fBottom;
			}
			// 화면의 다음 줄로 넘기기
			sx     = x;
			vtx_sx = 0;
			vtx_sy = vtx_sy + ((float) (iFontHeight)) / m_fTextScale;
			continue;
		}
		else if (0x80 & szText[iCount]) // 2BYTE 문자
		{
			memcpy(szTempChar, &(szText[iCount]), 2);
			iCount        += 2;
			szTempChar[2]  = 0x00;
		}
		else // 1BYTE 문자
		{
			memcpy(szTempChar, &(szText[iCount]), 1);
			iCount        += 1;
			szTempChar[1]  = 0x00;
		}

		SelectObject(s_hDC, m_hFont);
		GetTextExtentPoint32(s_hDC, szTempChar, lstrlen(szTempChar), &size);
		if ((x + size.cx) > m_dwTexWidth)
		{ // vertex 만들고 다음 줄로 넘기기..
			// vertex 만들기
			if (sx != x)
			{
				FLOAT tx1 = ((FLOAT) (sx)) / m_dwTexWidth;
				FLOAT ty1 = ((FLOAT) (y)) / m_dwTexHeight;
				FLOAT tx2 = ((FLOAT) (x)) / m_dwTexWidth;
				FLOAT ty2 = ((FLOAT) (y + iFontHeight)) / m_dwTexHeight;

				FLOAT w   = (tx2 - tx1) * m_dwTexWidth / m_fTextScale;
				FLOAT h   = (ty2 - ty1) * m_dwTexHeight / m_fTextScale;

				__ASSERT(dwNumTriangles + 2 < MAX_NUM_VERTICES, "??"); // Vertex buffer가 모자란다.
				if (dwNumTriangles + 2 >= MAX_NUM_VERTICES)
					break;

				FLOAT fLeft   = vtx_sx + 0 - 0.5f;
				FLOAT fRight  = vtx_sx + w - 0.5f;
				FLOAT fTop    = vtx_sy + 0 - 0.5f;
				FLOAT fBottom = vtx_sy + h - 0.5f;
				pVertices->Set(fLeft, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty2);
				++pVertices;
				pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1);
				++pVertices;
				pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2);
				++pVertices;
				pVertices->Set(fRight, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty1);
				++pVertices;
				pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2);
				++pVertices;
				pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1);
				++pVertices;
				dwNumTriangles += 2;

				if (fMaxX < fRight)
					fMaxX = fRight;
				if (fMaxY < fBottom)
					fMaxY = fBottom;

				// 텍스쳐의 다음 줄로 넘기기
				x = sx  = 0;
				y      += iFontHeight;
				vtx_sx  = vtx_sx + w;
			}
			else
			{
				x = sx  = 0;
				y      += iFontHeight;
			}
		}

		// dc에 찍기
		SelectObject(s_hDC, m_hFont);
		ExtTextOut(s_hDC, x, y, ETO_OPAQUE, nullptr, szTempChar, lstrlen(szTempChar), nullptr);
		x += size.cx;
	}

	// 마지막 남은 vertex 만들기
	if (sx != x)
	{
		FLOAT tx1 = ((FLOAT) (sx)) / m_dwTexWidth;
		FLOAT ty1 = ((FLOAT) (y)) / m_dwTexHeight;
		FLOAT tx2 = ((FLOAT) (x)) / m_dwTexWidth;
		FLOAT ty2 = ((FLOAT) (y + iFontHeight)) / m_dwTexHeight;

		FLOAT w   = (tx2 - tx1) * m_dwTexWidth / m_fTextScale;
		FLOAT h   = (ty2 - ty1) * m_dwTexHeight / m_fTextScale;

		__ASSERT(dwNumTriangles + 2 < MAX_NUM_VERTICES, "??"); // Vertex buffer가 모자란다.

		FLOAT fLeft   = vtx_sx + 0 - 0.5f;
		FLOAT fRight  = vtx_sx + w - 0.5f;
		FLOAT fTop    = vtx_sy + 0 - 0.5f;
		FLOAT fBottom = vtx_sy + h - 0.5f;
		pVertices->Set(fLeft, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty2);
		++pVertices;
		pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1);
		++pVertices;
		pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2);
		++pVertices;
		pVertices->Set(fRight, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty1);
		++pVertices;
		pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2);
		++pVertices;
		pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1);
		++pVertices;
		dwNumTriangles += 2;

		if (fMaxX < fRight)
			fMaxX = fRight;

		if (fMaxY < fBottom)
			fMaxY = fBottom;
	}

	// Unlock and render the vertex buffer
	m_pVB->Unlock();

	m_iPrimitiveCount = dwNumTriangles;
	m_PrevLeftTop     = {};
	m_Size.cx         = (long) fMaxX;
	m_Size.cy         = (long) fMaxY;
}

HRESULT CDFont::DrawText(FLOAT sx, FLOAT sy, uint32_t dwColor, uint32_t dwFlags)
{
	if (m_pVB == nullptr || s_hDC == nullptr || m_hFont == nullptr)
	{
		//__ASSERT(0, "NULL Vertex Buffer or DC or Font Handle ");
		return E_FAIL;
	}

	if (m_iPrimitiveCount <= 0)
		return S_OK;
	if (m_pd3dDevice == nullptr)
		return E_FAIL;

	// 위치 색 조정
	__Vector2 vDiff = __Vector2(sx, sy) - m_PrevLeftTop;
	if (fabs(vDiff.x) > 0.5f || fabs(vDiff.y) > 0.5f || dwColor != m_dwFontColor)
	{
		// lock vertex buffer
		__VertexTransformed* pVertices = nullptr;
		HRESULT hr                     = m_pVB->Lock(0, 0, (void**) &pVertices, 0);
		if (FAILED(hr))
			return E_FAIL;

		int iVC = m_iPrimitiveCount * 3;
		if (fabs(vDiff.x) > 0.5f)
		{
			m_PrevLeftTop.x = sx;

			for (int i = 0; i < iVC; i++)
				pVertices[i].x += vDiff.x;
		}

		if (fabs(vDiff.y) > 0.5f)
		{
			m_PrevLeftTop.y = sy;

			for (int i = 0; i < iVC; i++)
				pVertices[i].y += vDiff.y;
		}

		if (dwColor != m_dwFontColor)
		{
			m_dwFontColor   = dwColor;
			m_PrevLeftTop.y = sy;

			for (int i = 0; i < iVC; i++)
				pVertices[i].color = m_dwFontColor;
		}

		// Unlock
		m_pVB->Unlock();
	}

	// back up render state
	DWORD dwAlphaBlend = 0, dwSrcBlend = 0, dwDestBlend = 0, dwZEnable = 0, dwFog = 0;
	DWORD dwColorOp = 0, dwColorArg1 = 0, dwColorArg2 = 0, dwAlphaOp = 0, dwAlphaArg1 = 0,
		  dwAlphaArg2 = 0, dwMinFilter = 0, dwMagFilter = 0;

	m_pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	m_pd3dDevice->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	m_pd3dDevice->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	m_pd3dDevice->GetRenderState(D3DRS_FOGENABLE, &dwFog);

	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwColorOp);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorArg1);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG2, &dwColorArg2);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOp);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg1);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG2, &dwAlphaArg2);
	m_pd3dDevice->GetSamplerState(0, D3DSAMP_MINFILTER, &dwMinFilter);
	m_pd3dDevice->GetSamplerState(0, D3DSAMP_MAGFILTER, &dwMagFilter);

	// Set up renderstate
	if (TRUE != dwAlphaBlend)
		m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	if (D3DBLEND_SRCALPHA != dwSrcBlend)
		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend)
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	if (D3DZB_FALSE != dwZEnable)
		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	if (FALSE != dwFog)
		m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if (D3DTOP_MODULATE != dwColorOp)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	if (D3DTA_TEXTURE != dwColorArg1)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	if (D3DTA_DIFFUSE != dwColorArg2)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	if (D3DTOP_MODULATE != dwAlphaOp)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	if (D3DTA_TEXTURE != dwAlphaArg1)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	if (D3DTA_DIFFUSE != dwAlphaArg2)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	if (dwFlags & D3DFONT_FILTERED)
	{
		// Set filter states
		if (D3DTEXF_LINEAR != dwMinFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		if (D3DTEXF_LINEAR != dwMagFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}
	else
	{
		if (D3DTEXF_POINT != dwMinFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		if (D3DTEXF_POINT != dwMagFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	}

	// render
	m_pd3dDevice->SetFVF(FVF_TRANSFORMED);
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(__VertexTransformed));
	m_pd3dDevice->SetTexture(0, m_pTexture);
	m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_iPrimitiveCount);

	// Restore the modified renderstates
	if (TRUE != dwAlphaBlend)
		m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	if (D3DBLEND_SRCALPHA != dwSrcBlend)
		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	if (D3DBLEND_INVSRCALPHA != dwDestBlend)
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);

	if (D3DZB_FALSE != dwZEnable)
		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable);

	if (FALSE != dwFog)
		m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, dwFog);
	if (D3DTOP_MODULATE != dwColorOp)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwColorOp);
	if (D3DTA_TEXTURE != dwColorArg1)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwColorArg1);
	if (D3DTA_DIFFUSE != dwColorArg2)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, dwColorArg2);
	if (D3DTOP_MODULATE != dwAlphaOp)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAlphaOp);
	if (D3DTA_TEXTURE != dwAlphaArg1)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg1);
	if (D3DTA_DIFFUSE != dwAlphaArg2)
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, dwAlphaArg2);
	if (dwFlags & D3DFONT_FILTERED)
	{
		if (D3DTEXF_LINEAR != dwMinFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, dwMinFilter);
		if (D3DTEXF_LINEAR != dwMagFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, dwMagFilter);
	}
	else
	{
		if (D3DSAMP_MINFILTER != dwMinFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, dwMinFilter);
		if (D3DSAMP_MAGFILTER != dwMagFilter)
			m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, dwMagFilter);
	}

	return S_OK;
}

BOOL CDFont::GetTextExtent(const std::string& szString, int iStrLen, SIZE* pSize)
{
	if (nullptr == s_hDC)
		return FALSE;

	SelectObject(s_hDC, m_hFont);
	return ::GetTextExtentPoint32(s_hDC, szString.c_str(), iStrLen, pSize);
}

HRESULT CDFont::SetFontColor(uint32_t dwColor)
{
	if (m_iPrimitiveCount <= 0 || m_pVB == nullptr)
		return E_FAIL;

	if (dwColor == m_dwFontColor)
		return S_OK;

	__VertexTransformed* pVertices = nullptr;

	// lock vertex buffer
	HRESULT hr                     = m_pVB->Lock(0, 0, (void**) &pVertices, 0);
	if (FAILED(hr))
		return hr;

	m_dwFontColor = dwColor;

	int iVC       = m_iPrimitiveCount * 3;
	for (int i = 0; i < iVC; ++i)
		pVertices[i].color = m_dwFontColor;
	m_pVB->Unlock();

	return S_OK;
}
