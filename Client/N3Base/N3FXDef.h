//////////////////////////////////////////////////////////////////////////////////////
//
//	N3FXDef.h
//	Effect에서 쓰이는 상수들이나 자료형들 정의...
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __N3FXDEF_H__
#define __N3FXDEF_H__

#include "N3Base.h"
#include "My_3DStruct.h"

constexpr int MAX_FX_PART_V0		= 8;	// V0
constexpr int MAX_FX_PART_V1_ORIG	= 16;	// V1 originally, before they changed it
constexpr int MAX_FX_PART_V1		= 26;	// V1(+) after it was changed - no version change to identify this change

// TODO: Fix this. The UI is just hardcoded for 16 (which got later updated to 26 without a version change).
constexpr int MAX_FX_PART_TOOL		= MAX_FX_PART_V1_ORIG;

// 한 인스턴스가 동시에 표현할수 있는 갯수..
constexpr int MAX_FX_PART			= MAX_FX_PART_V1;

constexpr int NUM_VERTEX_PARTICLE = 4;	// 파티클 하나에 들어가는 점수..
constexpr int NUM_VERTEX_BOTTOM = 10;	//
constexpr int NUM_KEY_COLOR = 100;

enum e_FXPartType	// 이펙트 스타일...매쉬를 이용한 건지, 파티클을 이용한 건지..등등..
{
	FX_PART_TYPE_NONE = 0,
	FX_PART_TYPE_PARTICLE = 1,		//'particle'
	FX_PART_TYPE_BOARD = 2,			//'board'
	FX_PART_TYPE_MESH = 3,			//'mesh'
	FX_PART_TYPE_BOTTOMBOARD = 4	//'bottomboard'
};

enum e_FXPartState	//파트의 상태..
{
	FX_PART_STATE_DEAD = 0,
	FX_PART_STATE_DYING = 1,
	FX_PART_STATE_LIVE = 2,
	FX_PART_STATE_READY = 3
};

enum e_FXBundleState	//번들의 상태..
{
	FX_BUNDLE_STATE_DEAD = 0,
	FX_BUNDLE_STATE_DYING = 1,
	FX_BUNDLE_STATE_LIVE = 2	
};

enum e_FXBundleAct	//번들이 어케 동작하는지..
{
	FX_BUNDLE_MOVE_DIR_FIXEDTARGET = 0,
	FX_BUNDLE_MOVE_DIR_FLEXABLETARGET = 1,
	FX_BUNDLE_MOVE_DIR_FLEXABLETARGET_RATIO = 2,
	FX_BUNDLE_MOVE_CURVE_FIXEDTARGET = 3,	
	FX_BUNDLE_MOVE_DIR_SLOW = 4,
	FX_BUNDLE_REGION_POISON = 5,
	FX_BUNDLE_MOVE_NONE = 0xffffffff	
};

//
enum e_FXPartParticleEmitType	// 이펙트 파트가 어떤 모양으로 전개되는지...
{
	FX_PART_PARTICLE_EMIT_TYPE_NORMAL = 0,	//'normal'
	FX_PART_PARTICLE_EMIT_TYPE_SPREAD = 1,	//'spread'
	FX_PART_PARTICLE_EMIT_TYPE_GATHER = 2,	//'gather'	
};

//
/////////////////////////////////////////////////////////////////
//structures.....

/*
typedef struct __TABLE_FX	// FX 리소스 레코드...
{
	uint32_t		dwID;		// 고유 ID
	std::string	szFN;		// file name
	uint32_t		dwSoundID;	// 효과에 쓰는 사운드 아디.
} TABLE_FX;
*/

typedef struct Point3D
{
	float x;
	float y;
	float z;
} POINT3D;

typedef union __ParticleEmitCondition	//파티클 분사시 필요정보..
{
	POINT3D	vGatherPoint;	//EmitType이 gather일때 모아지는 점..
	float	fEmitAngle;		//EmitType이 spread일때 뿌려지는 각..
} PARTICLEEMITCONDITION;

typedef struct __FXPartWithStartTime	// 번들에서 파트들 관리할때..
{
	class CN3FXPartBase*	pPart;
	float					fStartTime;
	
	__FXPartWithStartTime()
	{
		pPart = NULL;
		fStartTime = 0.0f;
	}
} FXPARTWITHSTARTTIME, *LPFXPARTWITHSTARTTIME;

typedef struct __FXBInfo
{
	char	FXBName[MAX_PATH];
	int		joint;
	BOOL	IsLooping;

	__FXBInfo()
	{
		ZeroMemory(FXBName, MAX_PATH);
		joint = -1;
		IsLooping = FALSE;
	}	
} FXBINFO, *LPFXBINFO;

#endif // #ifndef __N3FXDEF_H__
