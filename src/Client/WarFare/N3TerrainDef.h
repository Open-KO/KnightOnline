////////////////////////////////////////////////////////////////////////////////////////
//
//	N3TerrainDef.h
//	- 이것저것 Terrain에 관련된 자료형정의, 상수정의...
//	- 게임에 쓰는 지형 (쿼드트리 아님...^^)
//	- 2001. 10. 22.
//
//	By Donghoon..
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __N3TERRAINDEF_H__
#define __N3TERRAINDEF_H__

// Constant Definitions..
const int PATCH_TILE_SIZE = 8;		//	패치 하나에 타일이 8x8개 들어간다.
const float TILE_SIZE = 4.0f;			//	타일하나당 길이 4m
const int MAX_LOD_LEVEL = 10;		//	LOD수준이 가장 높은 단계 가장 Detail...
const int MIN_LOD_LEVEL = 0;		//	LOD수준이 가장 낮은 단계 가장 Rough...
const int MAX_PATCH_LEVEL = 3;		//	2 ^ 3 = 8.
const int PATCH_PIXEL_SIZE = 32;	//	패치하나에 들어가는 픽셀수..
const int UNITUV = 32;				//	컬러맵 텍스쳐 한장에 들어가는 타일수..
const int LIGHTMAP_TEX_SIZE = 16;	//	타일 하나당 들어가는 라이트맵 텍스쳐 크기.
const int TILE_PIXEL_SIZE = 128;	//	타일하나의 실제 픽셀 사이즈.
const int COLORMAPTEX_SIZE = 128;	//	컬러맵 텍스쳐의 픽셀크기..128x128
const int DISTANCE_TABLE_SIZE = 64; //	셀단위의 거리 테이블 크기(64 x 64)..
const int MAX_GRASS	= 8;
const int MAX_TERRAIN_SOUND_EFFECT = 4;

enum { DIR_LT = 0, DIR_CT = 1, DIR_RT = 2,
		DIR_LM = 3, DIR_CM = 4, DIR_RM = 5,
		DIR_LB = 6, DIR_CB = 7, DIR_RB = 8,
		DIR_WARP = 9 }; 

// Structure Definitions..
typedef struct __MapData			// 맵데이터...
{
	float	fHeight;				//타일텍스쳐가 꽉차는지 아닌지..
	uint32_t	bIsTileFull	: 1;		//지형의 높이값..
	uint32_t	Tex1Dir		: 5;		//첫번째 타일 텍스쳐 찍는 방향.
	uint32_t	Tex2Dir		: 5;		//두번째 타일 텍스쳐 찍는 방향.
	uint32_t	Tex1Idx		: 10;		//첫번째 타일 텍스쳐의 인덱스.
	uint32_t	Tex2Idx		: 10;		//두번째 타일 텍스쳐의 인덱스.
	
	__MapData()
	{
		bIsTileFull = true;
		fHeight = FLT_MIN;
		Tex1Idx = 1023;
		Tex1Dir = 0;
		Tex2Idx = 1023;
		Tex2Dir = 0;
	}
} MAPDATA, *LPMAPDATA;

typedef struct __FanInfo
{
	int		NumFace;
	int		NumVertex;
} FANINFO, *LPFANINFO;

typedef struct __TerrainTileTexInfo
{
	class CN3Texture* pTex;
	float u;
	float v;
	
	__TerrainTileTexInfo()
	{
		pTex = nullptr;
		u = v = 0.0f;
	}
} TERRAINTILETEXINFO, *LPTERRAINTILETEXINFO;

typedef struct __TerrainBGMInfo
{
	std::string szBGM[MAX_TERRAIN_SOUND_EFFECT];
	float		fBGMRegenTime[MAX_TERRAIN_SOUND_EFFECT];
	std::string szBGE[MAX_TERRAIN_SOUND_EFFECT];
	float		fBGERegenTime[MAX_TERRAIN_SOUND_EFFECT];

	__TerrainBGMInfo()
	{
		for(int i=0;i<MAX_TERRAIN_SOUND_EFFECT;i++)
		{
			szBGM[i] = "";
			fBGMRegenTime[i] = 0.0f;
			szBGE[i] = "";
			fBGERegenTime[i] = 0.0f;
		}
	}
} TERRAINBGMINFO, *LPTERRAINBGMINFO;

#include <list>
typedef std::list<FANINFO> FanInfoList;
typedef FanInfoList::iterator FIIt;
//typedef CellInfoList::value_type DTValue;

#include <map>
typedef std::map<uint32_t,class CN3Texture*> stlMap_N3Tex;
typedef stlMap_N3Tex::iterator stlMap_N3TexIt;
typedef stlMap_N3Tex::value_type stlMap_N3TexValue;

#endif	//end of #ifndef __LYTERRAINDEF_H__