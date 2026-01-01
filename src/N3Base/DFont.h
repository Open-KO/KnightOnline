// DFont.h: interface for the CDFont class.
// 사용법 및 사용제한
// 1. SetText함수는 계산량이 많다.(글씨를 바꿀때만 부르자(중요))
// 2. DrawText 함수는 2d평면상에 그려준다.
// 3. DrawText3D 함수는 3d로 그릴때 사용한다.(SetText함수에서 3D라고 명시해줘야 한다.)
// 제한 : 판때기를 현재 50개밖에 생성하지 못한다.(이 이상 나오게 하려면 MAX_NUM_VERTICES 를 고쳐주면 된다.)
// 4. InvalidateDeviceObjects();를 호출한후 다시 글씨를 쓰려면 다시 SetText함수를 호출해줘야 한다.
// 5. 3D공간상에 글씨를 찍을때 가운데 정렬해서 찍고 싶으면 SetText함수에 두번째 파라미터에 (D3DFONT_3D|D3DFONT_CENTERED)를 넣어준다.
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4786)

#if !defined(AFX_DFONT_H__B1A14901_0027_40BC_8A6C_7FC78DE38686__INCLUDED_)
#define AFX_DFONT_H__B1A14901_0027_40BC_8A6C_7FC78DE38686__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3Base.h"

// Font creation flags
#define D3DFONT_BOLD        0x0001
#define D3DFONT_ITALIC      0x0002
#define D3DFONT_3D		    0x0008		// 3D text

// set text flag
#define D3DFONT_CENTERED    0x0004		// 3D font에서만..

// Font rendering flags (drawtext or DrawText3D 함수에서 쓰는 flag)
#define D3DFONT_TWOSIDED    0x0010		// 3D font에서만..
#define D3DFONT_FILTERED    0x0020		// texture를 D3DTSS_MAGFILTER 로 찍기


class CDFont  : public CN3Base
{
public:
	CDFont(const std::string& szFontName, uint32_t dwHeight, uint32_t dwFlags=0L);
	virtual ~CDFont();

// Attributes
public:
	const std::string& GetFontName() { return m_szFontName; }
	uint32_t	GetFontHeight() const {return m_dwFontHeight;}
	int		GetFontHeightInLogicalUnit() const {return -MulDiv(m_dwFontHeight, GetDeviceCaps(s_hDC, LOGPIXELSY), 72);}
	uint32_t	GetFontFlags() const {return m_dwFontFlags;}
	SIZE	GetSize() const {return m_Size;}
	uint32_t	GetFontColor() const {return m_dwFontColor;}

protected:
	static HDC s_hDC;						// DC handle
	static int s_iInstanceCount;			// Class Instance Count
	static HFONT s_hFontOld;				// default font


	std::string	m_szFontName;				// Font properties
    uint32_t		m_dwFontHeight;					// Font Size
    uint32_t		m_dwFontFlags;

    LPDIRECT3DDEVICE9       m_pd3dDevice;	// A D3DDevice used for rendering
    LPDIRECT3DTEXTURE9      m_pTexture;		// The d3d texture for this font
    LPDIRECT3DVERTEXBUFFER9 m_pVB;			// VertexBuffer for rendering text
    uint32_t	m_dwTexWidth;					// Texture dimensions
    uint32_t	m_dwTexHeight;					// Texture dimensions
    FLOAT	m_fTextScale;					// 쓸 폰트가 너무 클경우 비디오 카드에
											// 따른 texture 크기 제한을 넘어버리기 때문에
											// 이런 경우 Scale을 이용하여 크게 늘려 찍는다.

//	HDC			m_hDC;							// DC handle
	HFONT		m_hFont;						// Font handle
	UINT		m_iPrimitiveCount;				// 글씨 찍을 판의 갯수
	__Vector2	m_PrevLeftTop;				// DrawText의 경우 찍는 곳의 위치가 변경되었을때를 위한 변수
	uint32_t	m_dwFontColor;				// 글씨 색
	BOOL		m_Is2D;						// 2D Text인가?
	SIZE		m_Size;						// 쓴 글씨들이 차지하는 크기(pixel단위, 가로 세로)

// Operations
public:
	bool	IsSetText() { if(m_pTexture) return true; return false; }
	void	AddToAlphaManager(uint32_t dwColor, float fDist, __Matrix44& mtxWorld, uint32_t dwFlags);
	HRESULT	SetFontColor(uint32_t dwColor);						// 글씨 색을 바꾼다.
	HRESULT InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );	// d3d device를 정해주는 초기화 함수 (Init할때 호출)
	HRESULT RestoreDeviceObjects();								// resource를 메모리에 세팅하는 초기화 함수 (Init할때 호출)
	HRESULT InvalidateDeviceObjects();							// resource등을 무효화시키는 함수 (release할때 호출)
	HRESULT DeleteDeviceObjects();								// resource등을 메모리에서 해제 (release할때 호출)

	HRESULT SetText(const std::string& szText, uint32_t dwFlags = 0L);	// 출력할 글씨가 달라졌을때만 호출하는 것이 중요.
	HRESULT DrawText( FLOAT sx, FLOAT sy, uint32_t dwColor, uint32_t dwFlags, FLOAT fZ = 1.0f);	// 버퍼에 저장된 글씨를 그린다.(2d)
	HRESULT DrawText3D(uint32_t dwColor, uint32_t dwFlags );		// 버퍼에 저장된 글씨를 그린다.(3d)

	HRESULT	SetFont(const std::string& szFontName, uint32_t dwHeight, uint32_t dwFlags=0L);	// Font를 바꾸고 싶을때 호출한다. (dwHeight는 point size를 넣는다.)
	BOOL	GetTextExtent(const std::string& szString, int iStrLen, SIZE* pSize );
protected:
	void Make2DVertex(const int iFontHeight, const std::string& szText);					// 입력 받은 문자를 적절하게 배치된 2d 폴리곤으로 만든다.
	void Make3DVertex(const int iFontHeight, const std::string& szText, uint32_t dwFlags);	// 입력 받은 문자를 적절하게 배치된 3d 폴리곤을 만든다.

};

#endif // !defined(AFX_DFONT_H__B1A14901_0027_40BC_8A6C_7FC78DE38686__INCLUDED_)
