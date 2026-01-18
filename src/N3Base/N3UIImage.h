// N3UIImage.h: interface for the CN3UIImage class.
// 보강해야 할 점 : animation되는 이미지도 그리도록 가능하게 하자..
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UIIMAGE_H__80AE123C_5A12_4A0E_9CF0_511ED98BB896__INCLUDED_)
#define AFX_N3UIIMAGE_H__80AE123C_5A12_4A0E_9CF0_511ED98BB896__INCLUDED_

#pragma once

#include "N3UIBase.h"

class N3Texture;
class CN3UIImage : public CN3UIBase
{
#ifdef _N3TOOL
	friend class CPropertyView; // 툴에서 각 변수들을 접근하기 위해서
#endif

public:
	CN3UIImage();
	~CN3UIImage() override;

	// Attributes
public:
	__FLOAT_RECT* GetUVRect()
	{
		return &m_frcUVRect;
	}

protected:
	LPDIRECT3DVERTEXBUFFER9 m_pVB; // vertex buffer
	CN3Texture* m_pTexRef;         // texture 참조 포인터 (s_TexMng에서 관리하므로 참조포인터이다.)

	// 텍스처 이름.. 따로 갖고 있는 이유는 툴에서 텍스처 부르기가 실패할 경우를 대비해서이다.
	std::string m_szTexFN;

	__FLOAT_RECT m_frcUVRect;      // uv 좌표를 저장
	D3DCOLOR m_Color;              // 칼라(배경 이미지가 없을경우 사용된다.)

	float m_fAnimFrame;            // 1초당 그려지는 이미지의 수
	int m_iAnimCount;              // Animate되는 이미지 수(Load할 때 child의 숫자로 파악)
	float m_fCurAnimFrame;         // 현재 그려질 animate frame index (소수는 버리고 사용하자)
	CN3UIImage** m_pAnimImagesRef; // Animate 될 자식 이미지들 참조 포인터들

								   //	Operations
public:
	D3DCOLOR GetColor()
	{
		return m_Color;
	}

	// Texture 포인터 얻기
	CN3Texture* GetTex() const
	{
		return m_pTexRef;
	}

	void SetTex(const std::string& szFN);                             // Texture 지정
	void SetUVRect(float left, float top, float right, float bottom); // image의 uv좌표 지정
	void SetColor(D3DCOLOR color);                                    // 칼라 지정

	void SetRegion(const RECT& Rect) override;                        // 영역 지정
	BOOL MoveOffset(int iOffsetX, int iOffsetY) override;
	void Release() override;                                          // Release
	void Tick() override;                                             // Tick
	void Render() override;                                           // 그리기
	virtual void RenderIconWrapper();
	void Init(CN3UIBase* pParent) override;                           // 초기화
	bool Load(File& file) override;

	CN3UIImage& operator=(const CN3UIImage& other);

protected:
	bool CreateVB();      // 4개의 vertex를 가진 vertex buffer 생성
	virtual void SetVB(); // vertex buffer 다시 세팅
// tool에서 사용하는 함수들
#ifdef _N3TOOL
public:
	bool Save(File& file) override;
	virtual void ChangeImagePath(const std::string& szPathOld, const std::string& szPathNew);
	void GatherImageFileName(std::set<std::string>& setImgFile);
	std::string GetTexFN()
	{
		return m_szTexFN;
	}

	void ReorderChildImage();
	void SetAnimImage(int iAnimCount);
	CN3UIImage* GetChildImage(int iIndex);
	virtual bool ReplaceAllTextures(const std::string& strFind, const std::string& strReplace);
#endif
};

#endif // !defined(AFX_N3UIIMAGE_H__80AE123C_5A12_4A0E_9CF0_511ED98BB896__INCLUDED_)
