// N3UIButton.h: interface for the CN3UIButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3UIBUTTON_H__7A7B3E89_9D17_45E8_8405_87877F3E6FF0__INCLUDED_)
#define AFX_N3UIBUTTON_H__7A7B3E89_9D17_45E8_8405_87877F3E6FF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3UIBase.h"
class CN3UIImage;
class CN3SndObj;
class CN3UIButton : public CN3UIBase  
{
public:
	CN3UIButton();
	virtual ~CN3UIButton();

// Attributes
public:
	enum eBTN_STATE	{BS_NORMAL=0, BS_DOWN, BS_ON, BS_DISABLE, NUM_BTN_STATE};	// button state
	void			SetClickRect(const RECT& Rect) {m_rcClick = Rect;}
	RECT			GetClickRect() const {return m_rcClick;}
protected:
	CN3UIImage*		m_ImageRef[NUM_BTN_STATE];	// 버튼의 각 상태별 image의 참조 포인터(참조인 이유는  children list로 관리하므로 참조만 한다.)
	RECT			m_rcClick;					// click되는 영역

	CN3SndObj*		m_pSnd_On;		// 버튼 위에 마우스가 올라가는 순간 내는 소리
	CN3SndObj*		m_pSnd_Click;	// 버튼이 눌리는 순간 내는 소리

// Operations
public:
	bool	Load(File& file) override;
	void	Release() override;
	void	SetRegion(const RECT& Rect) override;
	BOOL	MoveOffset(int iOffsetX, int iOffsetY) override;

	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	void	Render() override;

// 툴에서 사용하기 위한 함수
public:
	virtual void	operator = (const CN3UIButton& other);
	void			SetSndOn(const std::string& strFileName);
	void			SetSndClick(const std::string& strFileName);

	std::string GetSndFName_On() const;
	std::string GetSndFName_Click() const;

#ifdef _N3TOOL
	bool			Save(File& file) override;
	void			CreateImages();
	CN3UIImage*		GetImageRef(eBTN_STATE eState) const {	return m_ImageRef[eState];}
#endif
};

#endif // !defined(AFX_N3UIBUTTON_H__7A7B3E89_9D17_45E8_8405_87877F3E6FF0__INCLUDED_)

