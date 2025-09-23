// N3Chr.h: interface for the CN3Chr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Chr_h__INCLUDED_)
#define AFX_N3Chr_h__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include "N3TransformCollision.h"
#include "N3AnimControl.h"
#include "N3Skin.h"
#include "N3Cloak.h"
#include "N3PMeshInstance.h"
#include "N3Texture.h"
#include "N3Joint.h"
#include <list>
#include <vector>

const int MAX_CHR_JOINT = 256;
const int MAX_CHR_LOD = 4; // 4 단계 Level Of Detail
const int MAX_CHR_LOD_DELTA = 4;

class CN3CPartSkins : public CN3BaseFileAccess
{
	friend class CN3Chr;
	friend class CN3CPart;
protected:
	CN3Skin			m_Skins[MAX_CHR_LOD];

public:
	bool			Load(HANDLE hFile);
#ifdef _N3TOOL
	bool			Save(HANDLE hFile);
#endif // end of _N3TOOL

	void Release();
	CN3CPartSkins();
	virtual ~CN3CPartSkins();
};

class CN3CPart : public CN3BaseFileAccess
{
	friend class CN3Chr;
public:
	__Material			m_Mtl;
	__Material			m_MtlOrg;
	uint32_t				m_dwReserved;


	bool				m_bVisible;
protected:
	CN3Texture*			m_pTexRef;
	CN3Texture*			m_pTexOverlapRef; // 위에 덧칠할 텍스처
//	std::vector<class CN3AnimatedTexture*> m_AnimatedTextures;
	CN3CPartSkins*		m_pSkinsRef;
public:
	bool			Load(HANDLE hFile);
	void			Render(int nLOD);

	CN3Texture*		Tex() { return m_pTexRef; }
	CN3Texture*		TexSet(const std::string& szFN) { s_MngTex.Delete(&m_pTexRef); m_pTexRef = s_MngTex.Get(szFN, true, s_Options.iTexLOD_Chr); return m_pTexRef; }
	void			TexSet(CN3Texture* pTex) { s_MngTex.Delete(&m_pTexRef); m_pTexRef = pTex; }
	
	CN3Texture*		TexOverlap() { return m_pTexOverlapRef; }  // 위에 덧칠할 텍스처
	CN3Texture*		TexOverlapSet(const std::string& szFN);
	void			TexOverlapSet(CN3Texture* pTex);
	
	CN3Skin*		Skin(int nLOD)
	{
		if(m_pSkinsRef && nLOD >= 0 && nLOD < MAX_CHR_LOD)
			return &(m_pSkinsRef->m_Skins[nLOD]);
		else
			return nullptr;
	}
	CN3CPartSkins*	Skins() { return m_pSkinsRef; }
	CN3CPartSkins*	SkinsSet(const std::string& szFN);

#ifdef _N3TOOL
	void			RenderSelected(int nLOD);
	bool			Save(HANDLE hFile);
#endif // end of _N3TOOL

	void Release();
	
	CN3CPart();
	virtual ~CN3CPart();
};

enum e_PlugType{PLUGTYPE_NORMAL=0, PLUGTYPE_CLOAK, PLUGTYPE_MAX=10, PLUGTYPE_UNDEFINED=0xffffffff};
class CN3CPlugBase : public CN3BaseFileAccess
{
	friend class CN3Chr;
public:
	e_PlugType			m_ePlugType;
	bool				m_bVisible;
	int					m_nJointIndex; // 붙는 Mesh 의 Joint Index.. 어느 조인트에 붙느냐.. 그것이 문제로다...
	__Vector3			m_vPosition; // 붙는 Mesh 의 Offset Vector.
	__Vector3			m_vScale; // 붙는 Mesh 의 스케일.
	__Material			m_Mtl;
	__Matrix44			m_Matrix;	

protected:
	CN3PMeshInstance	m_PMeshInst; // Progressive Mesh Instance
	CN3Texture*			m_pTexRef; // Texture Reference Pointer
	CN3Texture*			m_pTexOverlapRef; // 위에 덧칠할 Texture Reference Pointer
//		__Vector3 m_vRotation; // 붙는 Mesh 의 Rotation일.
	__Matrix44			m_MtxRot; // Rotation Matrix;

public:
	CN3CPlugBase();
	virtual ~CN3CPlugBase();
	static e_PlugType	GetPlugTypeByFileName(const std::string& szFN);

	virtual bool	Load(HANDLE hFile);
#ifdef _N3TOOL
	virtual bool	Save(HANDLE hFile);
#endif // end of _N3TOOL

	virtual void Render(const __Matrix44& mtxParent, const __Matrix44& mtxJoint);
	virtual void Release();

	void ReCalcMatrix();

	__Vector3& Position() { return m_vPosition; }
	__Matrix44& Rotation() { return m_MtxRot; }
	__Vector3& Scale() { return m_vScale; }

	void PositionSet(const __Vector3& vPos) { m_vPosition = vPos; this->ReCalcMatrix(); }
	void RotationSet(const __Matrix44& mtx) { m_MtxRot = mtx; this->ReCalcMatrix(); }
	void ScaleSet(const __Vector3& vScale2) { m_vScale = vScale2; this->ReCalcMatrix(); }
	
	CN3Texture* Tex() { return m_pTexRef; }
	CN3Texture* TexSet(const std::string& szFN) { s_MngTex.Delete(&m_pTexRef); m_pTexRef = s_MngTex.Get(szFN, true, s_Options.iTexLOD_Chr); return m_pTexRef; }
	void		TexSet(CN3Texture* pTex) { s_MngTex.Delete(&m_pTexRef); m_pTexRef = pTex; }

	CN3Texture* TexOverlap() { return m_pTexOverlapRef; }  // 위에 덧칠할 텍스처
	CN3Texture* TexOverlapSet(const std::string& szFN);
	void		TexOverlapSet(CN3Texture* pTex);
	
	CN3PMesh* PMesh() { return m_PMeshInst.GetMesh(); }
	CN3PMeshInstance* PMeshInst() { return &m_PMeshInst; }
	void PMeshSet(const std::string& szFN);
};

const int MAX_PLUG_FX_POSITION = 5;
const int MAX_FXTAIL = 3;
enum e_PlugAttr{PLUGATTR_FIRE=0, PLUGATTR_ICE, PLUGATTR_LIGHTENNING, PLUGATTR_POISON, PLUGATTR_TOTAL};
class CN3CPlug: public CN3CPlugBase
{
public:
	bool			m_bRenderTrace;
	int 			m_nTraceStep; // 궤적의 길이..
	D3DCOLOR		m_crTrace; // 궤적 색깔.. 검은색이면 없다..
	float			m_fTrace0; // 궤적 위치..
	float			m_fTrace1; // 궤적 위치..

	//fx.......
	CN3PMeshInstance			m_PMeshInstFX; // FX 에 쓸 PMesh Instance
	class CN3FXBundle*			m_pFXMainBundle;
	class CN3FXBundle*			m_pFXTailBundle[MAX_FXTAIL];
	class CN3FXPartBillBoard*	m_pFXPart;
	std::string					m_strFXMainName;
	std::string					m_strFXTailName;
	
public:
	virtual bool	Load(HANDLE hFile);
#ifdef _N3TOOL
	virtual bool	Save(HANDLE hFile);
	void			ImportPMesh(const std::string& szFileName);
	void			RenderFXLines(const __Matrix44& mtxParent, const __Matrix44& mtxJoint); // FX 들어갈 곳에 선을 그려준다.
#endif // end of _N3TOOL
	virtual void	Render(const __Matrix44& mtxParent, const __Matrix44& mtxJoint);
	virtual void	Release();

	void RenderFX(const __Matrix44& mtxParent, const __Matrix44& mtxJoint);
	void InitFX(std::string& szFXMain, std::string& szFXTail, D3DCOLOR TraceCR = 0xffffffff);

public:
	CN3CPlug();
	virtual ~CN3CPlug();
};

class CN3CPlug_Cloak : public CN3CPlugBase
{
public:
	CN3CPlug_Cloak();
	virtual ~CN3CPlug_Cloak();	

public:
	virtual bool Load(HANDLE hFile);
#ifdef _N3TOOL
	virtual bool Save(HANDLE hFile);
#endif // end of _N3TOOL
	virtual void Render(const __Matrix44& mtxParent, const __Matrix44& mtxJoint);
	virtual void Release();

	void	SetLOD(int nLOD);
#ifdef _N3GAME
	CN3Cloak*	GetCloak(){return &m_Cloak;}
#endif
protected:
#ifdef _N3GAME
	CN3Cloak		m_Cloak;
#endif
};

const int MAX_CHR_ANI_PART = 2; // 0 - 상체, 1 - 하체 ::: 관절들을 나누어서 나누어서 에니메이션 설정..
const int MAX_PLUG_TRACE = 2; // 최대 두개의 무기 궤적을 남긴다..
const int MAX_PLUG_TRACE_VERTEX = 64; // 무기 궤적 점의 수.. 점 8 개로는 잔상이 3단계로 남는다..

typedef std::vector<CN3CPart*>::iterator	it_CPart;
typedef std::vector<CN3CPlug*>::iterator	it_CPlug;

class CN3Chr : public CN3TransformCollision
{
	friend class CPlayerBase;
	friend class CPlayerMySelf;

public:
	int							m_nLOD; // Level Of Detail - 강제로 세팅할수 있도록 한다..
	int							m_nLOD_0; // Level Of Detail - 0가장 디테일한 값으로 계산된 LOD 강제로 세팅할수 있도록 한다..

protected:
	static int s_iLODDelta; // LOD 계산에 필요한 인덱스..

	CN3Joint*					m_pRootJointRef; // 한개의 뼈대만을 쓴다..
	
	std::vector<CN3Joint*>		m_JointRefs; // 각 조인트에 참조 포인터
	std::vector<__Matrix44>		m_MtxJoints; // 각 조인트의 행렬.. 포인터로 두지 않은 이유는 각 캐릭터마다 따로 에니메이션이 되기 위함이다..
	std::vector<__Matrix44>		m_MtxInverses; // 조인트에 대한 역행렬

	std::vector<CN3CPart*>		m_Parts; // 각 캐릭터의 부분별 Data Pointer List
	std::vector<CN3CPlug*>		m_Plugs; // 이 캐릭터에 붙이는 무기등의 Data Pointer List
	std::vector<__VertexColor*>	m_vTraces; // Plug Trace Polygon
	class CN3FXPlug*			m_pFXPlug;	// 캐릭터에 FX를 붙이는 것.

	int							m_nJointPartStarts[MAX_CHR_ANI_PART]; // 조인트의 일부분이 따로 에니메이션 되야 한다면.. 조인트 인덱스 시작 번호
	int							m_nJointPartEnds[MAX_CHR_ANI_PART]; // 조인트의 일부분이 따로 에니메이션 되야 한다면.. 조인트 인덱스 끝 번호
	
//	CN3Skin*	m_pSkinCollision;
	
	CN3AnimControl*				m_pAniCtrlRef; // Animation Control Reference Pointer

	struct __FrmCtrl
	{
		__AnimData*		pAniData;	// 현재 에니메이션 데이터 포인터..

		int		iAni;					// 현재 에니메이션
		bool	bOnceAndFreeze;			// 한번만 하고 멈춰야 하는가??
		bool	bProcessingDelayNow;	// 지금 지연시간을 처리하는가??
		float	fFrmCur;				// 현재 프레임
		float	fFrmPrev;				// 최근 프레임
		int		iAniLoop;				// 에니메이션이 끝났나??
		float	fBlendFrm;				// 현재 에니메이션의 시작프레임과 블렌딩할 프레임
		float	fBlendTimeCur;			// 현재 블렌딩 시간..
		float	fBlendTime;				// 블렌딩할 시간. (초단위)
		float	fFreezeTime;			// 멈출 시간..

		void Init()
		{
			pAniData = nullptr;
			iAni = -1;						// 현재 에니메이션
			bOnceAndFreeze = false;			// 돌아갈 에니메이션..
			bProcessingDelayNow = false;	// 지금 지연시간을 처리하는가??

			fFrmCur = 0;
			fFrmPrev = 0;					// 최근 프레임
			iAniLoop = 0;					// 에니메이션이 끝났는가?
			fBlendFrm = 0;					// 블렌딩할 에니메이션 데이터 포인터..
			fBlendTimeCur = 0;				// 현재 블렌딩 시간..
			fBlendTime = 0;					// 블렌딩할 시간. (초단위)
		}
		__FrmCtrl()
		{
			__FrmCtrl::Init();
		}
	};
	__FrmCtrl		m_FrmCtrl;
	__FrmCtrl		m_FrmCtrlUpper;	// 상체용...
	float			m_fAniSpeedDelta; // 에니메이션 속도 조정 변수 1 이보통, 더 크면 빨라진다..

public:
	int				CheckCollisionPrecisely(const __Vector3& vPos, const __Vector3& vDir, __Vector3* pvPick = nullptr);
	int				CheckCollisionPrecisely(int ixScreen, int iyScreen, __Vector3* pvPick = nullptr);
	static void		LODDeltaSet(int iLODDelta) { if(s_iLODDelta >= 0 && iLODDelta <= 3) s_iLODDelta = iLODDelta; }
	static int 		LODDelta() { return s_iLODDelta; }

	void			FindMinMax(); // 최대 최소값을 찾는다.
	int				JointPartStart(int nAniPart) { if(nAniPart < 0 || nAniPart >= MAX_CHR_ANI_PART) return -1; return m_nJointPartStarts[nAniPart]; }
	int				JointPartEnd(int nAniPart) { if(nAniPart < 0 || nAniPart >= MAX_CHR_ANI_PART) return -1; return m_nJointPartEnds[nAniPart]; }
	void			JointPartSet(int nAniPart, int nJS, int nJE);

	const __Matrix44* MatrixGet(int nJointIndex) const
	{
		if (nJointIndex < 0
			|| nJointIndex >= static_cast<int>(m_MtxJoints.size()))
			return nullptr;

		return &m_MtxJoints[nJointIndex];
	}

//	void		CollisionSkinSet(const std::string& szFN);
//	CN3Skin*	CollisionSkin() { return m_pSkinCollision; }

	int PartCount() const
	{
		return static_cast<int>(m_Parts.size());
	}

	void		PartDelete(int iIndex);
	void		PartAlloc(int nCount);
	CN3CPart*	PartSet(int iIndex, const std::string& szFN);
	CN3CPart*  	PartAdd() { CN3CPart* pPart = new CN3CPart(); m_Parts.push_back(pPart); return pPart; }

	CN3CPart*	Part(int iIndex)
	{
		if (iIndex < 0
			|| iIndex >= static_cast<int>(m_Parts.size()))
			return nullptr;

		return m_Parts[iIndex];
	}

	int PlugCount() const
	{
		return static_cast<int>(m_Plugs.size());
	}

	void		PlugDelete(int iIndex);
	void		PlugAlloc(int nCount);
	CN3CPlug*	PlugSet(int iIndex, const std::string& szFN);
	CN3CPlug*	PlugAdd(e_PlugType eType=PLUGTYPE_NORMAL) { CN3CPlug* pPlug = new CN3CPlug(); m_Plugs.push_back(pPlug); return pPlug; }
	CN3CPlug*	Plug(int iIndex)
	{
		if (iIndex < 0
			|| iIndex >= static_cast<int>(m_Plugs.size()))
			return nullptr;

		return m_Plugs[iIndex];
	}

	void		Tick(float fFrm = FRAME_SELFPLAY);
	void		TickAnimationFrame();
	void		TickJoints();
	void		TickPlugs(float fLOD);
	void		RemakePlugTracePolygons();

	float		Height() { return m_vMax.y * m_vScale.y; } // 스케일을 적용한 키 ...
	float		Radius() { return m_fRadius * m_vScale.y; } // 스케일을 적용한 너비...

	CN3Joint*	Joint() { return m_pRootJointRef; }
	void		JointSet(const std::string& szFN);

	// Animation 관련 함수
#ifdef _N3TOOL
	void		AniDefaultSet();
#endif // #ifdef _N3TOOL
	CN3AnimControl* AniCtrl() { return m_pAniCtrlRef; }
	void		AniCtrlSet(const std::string& szFN);
	int			AniIndexCur() { return m_FrmCtrl.iAni; }
	int			AniCurSet(	int iAni,							// Animation 번호,
							bool bOnceAndFreeze = false,		// 한번만 돌고 멈추어야 하는가??
							float fBlendTime = FLT_MIN,			// 블렌딩하는 시간(초단위), 
							float fFreezeTime = 0, 				// 멈출시간...
							bool bStopUpperAnimation = true);	// 상체 에니메이션이 있으면.. 멈추도록
	void		AniUpperSet(int nAni, float fFreezeTime = 0);
	void		AniFixToLastFrame( int iAni ); // 마지막 프레임으로 고정 시켜 버린다.

	bool		IsAnimEnd(); // 한번 하고 멈추는 에니메이션 일경우 .. 끝났는지?? 혹은 한번하고 전의 동작으로 돌아가는 에니메이션일 경우 돌아갔는지?

	float		FrmCur() { return m_FrmCtrl.fFrmCur; }
	float		FrmPrev() { return m_FrmCtrl.fFrmPrev; }
	float		AniBlendDelta();
	float		AniSpeedDelta() { return m_fAniSpeedDelta; } // 에니메이션 속도 조정 변수 1 이보통, 더 크면 빨라진다..
	void		AniSpeedDeltaSet(float fDelta) { if(fDelta > 0.1f && fDelta < 10.0f) m_fAniSpeedDelta = fDelta; } // 에니메이션 속도 조정 변수 1 이보통, 더 크면 빨라진다..

	// Sound Player 관련 함수
	bool		NeedPlaySound0();
	bool		NeedPlaySound1();

	// 가짜 타격을 위한 함수..
	bool		NeedStrike0();
	bool		NeedStrike1();

	// 루핑이 다시 시작되는 타이밍인가?
	bool		IsLoopingAgain();
	
//////////////////////////////////////////////////
//	Coded (By Dino On 2002-10-10 오후 2:35:32 )
//	FXPlug
	class CN3FXPlug*	FXPlugSet(const std::string& strFN);
	class CN3FXPlug*	FXPlugCreate();
	class CN3FXPlug*	FXPlugGet() {return m_pFXPlug;}
	void				FXPlugDelete();
//	End Of Code (By Dino On 2002-10-10 오후 2:35:32 )
//////////////////////////////////////////////////

	void		Init();
	void		RegenerateCollisionMesh();
	void		BuildMesh();
	void		BuildMesh(int nLOD);
	void		Render();

	__AnimData* AniDataCur();
#ifdef _N3TOOL	
	__AnimData* AniDataUpper();
	bool		Save(HANDLE hFile);
#endif // end of _N3TOOL
	bool		Load(HANDLE hFile);

	void		Release();
	
	CN3Chr();
	virtual ~CN3Chr();

};

inline bool CN3Chr::IsAnimEnd() // 한번 하고 멈추는 에니메이션 일경우 .. 끝났는지?? 혹은 한번하고 전의 동작으로 돌아가는 에니메이션일 경우 돌아갔는지?
{
	if(m_FrmCtrlUpper.pAniData)
	{
		if(m_FrmCtrlUpper.iAniLoop > 0)	return true;
		return false;
	}
	else
	{
		if(m_FrmCtrl.iAniLoop > 0) return true;
		return false;
	}
}

inline float CN3Chr::AniBlendDelta()
{
	if(m_FrmCtrl.fBlendTime == 0) return 0.0f;
	return m_FrmCtrl.fBlendTimeCur / m_FrmCtrl.fBlendTime;
}

// Sound Player 관련 함수
inline bool	CN3Chr::NeedPlaySound0()
{
	if(nullptr == m_FrmCtrl.pAniData) return false;
	
	if(	m_FrmCtrlUpper.pAniData)
	{
		if(m_FrmCtrlUpper.pAniData->fFrmSound0 <= 0) return false;
		if(	m_FrmCtrlUpper.fFrmPrev < m_FrmCtrlUpper.pAniData->fFrmSound0 &&
			m_FrmCtrlUpper.fFrmCur >= m_FrmCtrlUpper.pAniData->fFrmSound0 ) return true;
		return false;
	}

	if(m_FrmCtrl.pAniData->fFrmSound0 <= 0) return false;
	if(	m_FrmCtrl.fFrmPrev < m_FrmCtrl.pAniData->fFrmSound0 &&
		m_FrmCtrl.fFrmCur >= m_FrmCtrl.pAniData->fFrmSound0 ) return true;

	return false;
}

inline bool CN3Chr::IsLoopingAgain()	// 루핑이 다시 시작되는 타이밍인가?
{
	if(nullptr == m_FrmCtrl.pAniData) return false;

	if(	m_FrmCtrlUpper.pAniData)
	{
		if (m_FrmCtrlUpper.iAniLoop <= 0) return false;
		if(	m_FrmCtrlUpper.fFrmPrev > m_FrmCtrlUpper.fFrmCur) return true;
	}

	if(m_FrmCtrl.iAniLoop <= 0) return false;	// 루핑이 아니다.
	if(	m_FrmCtrl.fFrmPrev > m_FrmCtrl.fFrmCur) return true;

	return false;
}

inline bool CN3Chr::NeedPlaySound1()
{
	if(nullptr == m_FrmCtrl.pAniData) return false;

	if(	m_FrmCtrlUpper.pAniData)
	{
		if(m_FrmCtrlUpper.pAniData->fFrmSound1 <= 0) return false;
		if(	m_FrmCtrlUpper.fFrmPrev < m_FrmCtrlUpper.pAniData->fFrmSound1 &&
			m_FrmCtrlUpper.fFrmCur >= m_FrmCtrlUpper.pAniData->fFrmSound1 ) return true;
		return false;
	}

	if(m_FrmCtrl.pAniData->fFrmSound1 <= 0) return false;
	if(	m_FrmCtrl.fFrmPrev < m_FrmCtrl.pAniData->fFrmSound1 &&
		m_FrmCtrl.fFrmCur >= m_FrmCtrl.pAniData->fFrmSound1 ) return true;

	return false;
}

inline bool	CN3Chr::NeedStrike0()
{
	if(nullptr == m_FrmCtrl.pAniData) return false;

	if(	m_FrmCtrlUpper.pAniData)
	{
		if(m_FrmCtrlUpper.pAniData->fFrmStrike0 <= 0) return false;
		if(	m_FrmCtrlUpper.fFrmPrev < m_FrmCtrlUpper.pAniData->fFrmStrike0 &&
			m_FrmCtrlUpper.fFrmCur >= m_FrmCtrlUpper.pAniData->fFrmStrike0 ) return true;
		return false;
	}

	if(m_FrmCtrl.pAniData->fFrmStrike0 <= 0) return false;
	if(	m_FrmCtrl.fFrmPrev < m_FrmCtrl.pAniData->fFrmStrike0 &&
		m_FrmCtrl.fFrmCur >= m_FrmCtrl.pAniData->fFrmStrike0 ) return true;

	return false;
}

inline bool	CN3Chr::NeedStrike1()
{
	if(nullptr == m_FrmCtrl.pAniData) return false;

	if(	m_FrmCtrlUpper.pAniData)
	{
		if(m_FrmCtrlUpper.pAniData->fFrmStrike1 <= 0) return false;
		if(	m_FrmCtrlUpper.fFrmPrev < m_FrmCtrlUpper.pAniData->fFrmStrike1 &&
			m_FrmCtrlUpper.fFrmCur >= m_FrmCtrlUpper.pAniData->fFrmStrike1 ) return true;
		return false;
	}

	if(m_FrmCtrl.pAniData->fFrmStrike1 <= 0) return false;
	if(	m_FrmCtrl.fFrmPrev < m_FrmCtrl.pAniData->fFrmStrike1 &&
		m_FrmCtrl.fFrmCur >= m_FrmCtrl.pAniData->fFrmStrike1 ) return true;

	return false;
}

#endif // !defined(AFX_N3Chr_h__INCLUDED_)
