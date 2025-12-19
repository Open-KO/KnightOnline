// N3ShapeMod.h: interface for the CN3ShapeMod class.
// 만든이 : dino
// n3shape의 한 일부분을 움직일 수 있게 한 클래스
// 상태(0번 상태, 1번상태, 2번상태)를 변경할 수 있다.
// 상태 변경시 (시간을 넣어주면 상태간의 pos rot scale 정보를 linear하게 변화시킨다.

// 개선점 : Shape 전체가 움직이는 상태를 추가하자
//          설계시 추가하려다 너무 복잡하여 보류함. 리마크 된 부분이 그에 관련된 것임. 추가 보강해야함.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3SHAPEMOD_H__6F3C87B4_D10B_412B_B385_0ECE2D4A19A8__INCLUDED_)
#define AFX_N3SHAPEMOD_H__6F3C87B4_D10B_412B_B385_0ECE2D4A19A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3Shape.h"

class CN3ShapeMod : public CN3Shape  
{
public:
	CN3ShapeMod();
	~CN3ShapeMod() override;

// Structures
protected:
	struct __ModPosRotScale
	{
		__Vector3 vPos;
		__Quaternion qRot;
		__Vector3 vScale;

		__ModPosRotScale() {vPos.Set(0,0,0); qRot.Set(0,0,0,0); vScale.Set(1,1,1);}
	};
	struct __ModPart
	{
		CN3SPart*			pPart;				// 움직일 Part
		bool				bPos;				// Pos 변화가 있는가?
		bool				bRot;				// Rot 변화가 있는가?
		bool				bScale;				// Scale 변화가 있는가?
		__ModPosRotScale*	pStateInfos;		// 움직일 상태 정보들(이동,회전,확대축소 정보 m_iStateCount개)
		__ModPosRotScale	CurStateInfo;		// 현재 움직인 상태 정보(tick에 따라 변화한다)

		__ModPart() {pPart = nullptr; pStateInfos = nullptr; bPos = false; bRot = false; bScale = false; }
		~__ModPart() {pPart = nullptr; if (pStateInfos) { delete [] pStateInfos; pStateInfos = nullptr;}}
	};

	struct __ModShape
	{
		bool				bShapePos;
		bool				bShapeRot;
		bool				bShapeScale;		// Shape가 위치, 회전, 스케일 변화가 있는가?
		__ModPosRotScale*	pShapeStateInfos;	// Shape 전체의 변화 정보
		__ModPosRotScale	CurShapeStateInfo;	// 현재 움직인 상태 정보(tick에 따라 변화한다)
		__ModShape() {pShapeStateInfos = nullptr; bShapePos = false; bShapeRot = false; bShapeScale = false;}
		~__ModShape() {if (pShapeStateInfos) {delete [] pShapeStateInfos; pShapeStateInfos = nullptr;}}
	};

// Attributes
public:
protected:
//	enum	{
//			N3SHAPEMOD_TYPE_NOT_MOD = 0,		// 아무 정보도 변화하지 않는 것
//			N3SHAPEMOD_TYPE_ONLY_SHAPE = 1,		// Shape정보만 변화하는 것
//			N3SHAPEMOD_TYPE_ONLY_PART = 2,		// Part 정보만 변화하는 것
//			N3SHAPEMOD_TYPE_NORMAL = 4			// 둘 다 변화하는 것
//	};
//	int		m_iModType;			// type..
	int		m_iStateCount;		// 상태가 몇개가 있는지 나타낸다.
	int		m_iCurState;		// 현재 상태
	int		m_iPrevState;		// 이전 상태(새로운 상태를 설정해주면 Animation되는 동안 이전상태를 저장해둔다.
	float	m_fTimeChanged;		// 상태가 완전히 바뀌는 시간
	float	m_fTimeChanging;	// 상태가 바뀌는 시작시간부터 지금까지의 경과 시간

	__ModShape	m_ModShape;			// shape의 상태 변화 정보
	int			m_iModPartCount;		// 변화하는 파트의 갯수
	__ModPart*	m_pModParts;		// 변화하는 파트들 정보

	__ModPart**	m_pMatchPart2ModPart;		// CPart리스트 배열순으로 __ModPart*와 매치시킨 배열 (load시 CPart 갯수만큼 배열로 잡고 정보를 넣는다)

// Operations
public:
	BOOL	SetState(int iState, float fSec);	// fSec시간동안 상태를 바꾼다. (fSec이 0일경우 즉시 바뀐다.)
	BOOL	LoadStateInfo(FILE* stream);	// 상태 정보를 읽어온다.(text로부터)
	void	Release() override;
	void	ReCalcMatrix() override;
	void	Tick(float fFrm = FRAME_SELFPLAY) override;
	bool	Load(File& file) override;

protected:
	CN3SPart* GetPartByPMeshFileName(const std::string& szFN);	// 이름으로 PMesh포인터 구하기
};

#endif // !defined(AFX_N3SHAPEMOD_H__6F3C87B4_D10B_412B_B385_0ECE2D4A19A8__INCLUDED_)
