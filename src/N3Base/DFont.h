#if !defined(AFX_DFONT_H__B1A14901_0027_40BC_8A6C_7FC78DE38686__INCLUDED_)
#define AFX_DFONT_H__B1A14901_0027_40BC_8A6C_7FC78DE38686__INCLUDED_

#pragma once

#include "N3Base.h"

enum e_D3DFontFlags : uint8_t
{
	// Font creation flags
	D3DFONT_BOLD     = 0x0001,
	D3DFONT_ITALIC   = 0x0002,

	// set text flag
	D3DFONT_CENTERED = 0x0004, // 3D font에서만..

	// Font rendering flags
	D3DFONT_TWOSIDED = 0x0010, // 3D font에서만..
	D3DFONT_FILTERED = 0x0020, // texture를 D3DTSS_MAGFILTER 로 찍기
};

class CDFont : public CN3Base
{
public:
	CDFont(const std::string& szFontName, uint32_t dwHeight, uint32_t dwFlags = 0L);
	~CDFont() override;

	// Attributes
public:
	const std::string& GetFontName()
	{
		return m_szFontName;
	}

	uint32_t GetFontHeight() const
	{
		return m_dwFontHeight;
	}

	int GetFontHeightInLogicalUnit() const
	{
		return -MulDiv(m_dwFontHeight, GetDeviceCaps(s_hDC, LOGPIXELSY), 72);
	}

	uint32_t GetFontFlags() const
	{
		return m_dwFontFlags;
	}

	SIZE GetSize() const
	{
		return m_Size;
	}

	uint32_t GetFontColor() const
	{
		return m_dwFontColor;
	}

protected:
	static HDC s_hDC;               // DC handle
	static int s_iInstanceCount;    // Class Instance Count
	static HFONT s_hFontOld;        // default font

	std::string m_szFontName;       // Font properties
	uint32_t m_dwFontHeight;        // Font Size
	uint32_t m_dwFontFlags;

	LPDIRECT3DDEVICE9 m_pd3dDevice; // A D3DDevice used for rendering
	LPDIRECT3DTEXTURE9 m_pTexture;  // The d3d texture for this font
	LPDIRECT3DVERTEXBUFFER9 m_pVB;  // VertexBuffer for rendering text
	uint32_t m_dwTexWidth;          // Texture dimensions
	uint32_t m_dwTexHeight;         // Texture dimensions
	FLOAT m_fTextScale;             // 쓸 폰트가 너무 클경우 비디오 카드에
									// 따른 texture 크기 제한을 넘어버리기 때문에
									// 이런 경우 Scale을 이용하여 크게 늘려 찍는다.

									//	HDC			m_hDC;							// DC handle
	HFONT m_hFont;           // Font handle
	UINT m_iPrimitiveCount;  // 글씨 찍을 판의 갯수
	__Vector2 m_PrevLeftTop; // DrawText의 경우 찍는 곳의 위치가 변경되었을때를 위한 변수
	uint32_t m_dwFontColor;  // 글씨 색
	SIZE m_Size;             // 쓴 글씨들이 차지하는 크기(pixel단위, 가로 세로)

							 // Operations
public:
	bool IsSetText() const
	{
		return (m_pTexture != nullptr);
	}

	HRESULT SetFontColor(uint32_t dwColor); // 글씨 색을 바꾼다.
	HRESULT InitDeviceObjects(
		LPDIRECT3DDEVICE9 pd3dDevice);      // d3d device를 정해주는 초기화 함수 (Init할때 호출)
	HRESULT RestoreDeviceObjects();    // resource를 메모리에 세팅하는 초기화 함수 (Init할때 호출)
	HRESULT InvalidateDeviceObjects(); // resource등을 무효화시키는 함수 (release할때 호출)
	HRESULT DeleteDeviceObjects();     // resource등을 메모리에서 해제 (release할때 호출)

	HRESULT SetText(const std::string& szText,
		uint32_t dwFlags = 0L);        // 출력할 글씨가 달라졌을때만 호출하는 것이 중요.
	HRESULT DrawText(
		FLOAT sx, FLOAT sy, uint32_t dwColor, uint32_t dwFlags); // 버퍼에 저장된 글씨를 그린다.(2d)

	HRESULT SetFont(const std::string& szFontName, uint32_t dwHeight,
		uint32_t dwFlags = 0L); // Font를 바꾸고 싶을때 호출한다. (dwHeight는 point size를 넣는다.)
	BOOL GetTextExtent(const std::string& szString, int iStrLen, SIZE* pSize);

protected:
	void Make2DVertex(const int iFontHeight,
		const std::string& szText); // 입력 받은 문자를 적절하게 배치된 2d 폴리곤으로 만든다.
};

#endif // !defined(AFX_DFONT_H__B1A14901_0027_40BC_8A6C_7FC78DE38686__INCLUDED_)
