////////////////////////////////////////////////////////////////////////////////////////
//
//	LyTerrainDef.h
//	- 이것저것 Terrain에 관련된 자료형정의, 상수정의...
//	- 쿼드트리 이용한 지형...^^
//
//	By Donghoon..
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __LYTERRAINDEF_H__
#define __LYTERRAINDEF_H__

// common include header...
#include <N3Base/N3Base.h>

// Constant Definitions..
const int N3ME_DATA_VERSION      = 3;
//version info...
//이전 데이터들은 버전 정보를 갖고 있지 않다.
//1 : 초기 데이터에 lightmap정보를 더한 것..
//2 : version 1에 풀 정보(풀과 DTex의 그룹과의 관계에 관한 정보)를 더한 것.
//3 : 개선된 lightmap..^^

const int N3ME_DTEX_DATA_VERSION = 2;
//
//1 : dTex를 읽을때 각 텍스쳐의 그룹 정보를 각텍스쳐이름.dif파일로 부터 읽는다..
//2 : 각텍스쳐의 그룹정보를 dtexinfo.dtx파일로부터 읽는다..

const int TERRAIN_CELL_SIZE      = 4;
//const int MAX_COLORMAPTEXTURE = 32;		//	최대 #colormaptexture at one side = 4096/128..
//현재 m_iNumColorMap으로 대체해서 쓰고 있당...
const int PATCH_TILE_SIZE        = 8;    //	패치 하나에 타일이 8x8개 들어간다.

const int MAX_COLORMAPVB_SIZE    = 1026; //컬러맵으로 찍는 VB의 최대 사이즈.
const int MAX_TILEVB_SIZE        = 16;   //타일찍는 VB의 최대 사이즈.
const int MAX_LIGHTMAPVB_SIZE    = 16;   //라이트맵 쓰는 VB의 최대 사이즈.

const int DTEX_SIZE              = 1024; // Detail Texture Size
const int NUM_DTEXTILE           = 8;    // Detail Texture 분할 수..

const int LIGHTMAP_TEX_SIZE      = 8;    //32;

const int DISTANCE_TABLE_SIZE    = 64;

const int NOTMOVE_HEIGHT         = 10; // 높이차가 커서 갈 수 없는 타일의 최소값..
									   // 10이하면 움직일 수 있고, 이상이면 갈 수 없다.

const int MAX_BRUSH_SIZE         = 21;
const int BRUSH_CENTER           = (MAX_BRUSH_SIZE / 2);
enum
{
	BRUSHSHAPE_CIRCLE = 1,
	BRUSHSHAPE_SQUARE = 2
};
enum
{
	TEM_NOT         = 0,
	TEM_HEIGHTEN    = 1,
	TEM_SMOOTH      = 2,
	TEM_DTEX        = 3,
	TEM_LIGHTMAP    = 4,
	TEM_MAKENPCPATH = 5,
	TEM_FLAT        = 6,
	TEM_BRUSH_SHOW  = 7
};

enum
{
	DTEX_FULL = 0,
	DTEX_1PER2,
	DTEX_1PER4,
	DTEX_3PER4,
	DTEX_1PER8,
	DTEX_7PER8,
	DTEX_5PER8,
	DTEX_3PER8,
	DTEX_MAX = 8
};

//macro..
#define DISTANCE3D(a, b)                                                        \
	(sqrt(((a).x - (b).x) * ((a).x - (b).x) + ((a).y - (b).y) * ((a).y - (b).y) \
		  + ((a).z - (b).z) * ((a).z - (b).z)))
#define DISTANCE2D(a, b)                                                          \
	(sqrt(((a).x - (b).x) * ((a).x - (b).x) + ((a).z - (b).z) * ((a).z - (b).z)))

// Structure Definitions..

//맵에디터..
typedef struct __DTexAttr
{
	int Group;
	int Attr;

	__DTexAttr()
	{
		Group = 0;
		Attr  = 0;
	}
} DTEXATTR, *LPDTEXATTR;

typedef struct __DTexTileAttr
{
	int TexID;
	int TileX;
	int TileY;

	__DTexTileAttr()
	{
		TexID = -1;
		TileX = 0;
		TileY = 0;
	}
} DTEXTILEATTR, *LPDTEXTILEATTR;

typedef struct __DTexInfo
{
	int Dir;
	DTEXTILEATTR TexIdx;
	DTEXATTR Attr;

	__DTexInfo()
	{
		Dir = 0;
	}
} DTEXINFO, *LPDTEXINFO;

typedef struct __MapData
{
	float fHeight; //지형의 높이값..
	//float	u1,v1;
	//__Vector3 Normal;
	DTEXINFO DTexInfo1;
	DTEXINFO DTexInfo2;

	__MapData()
	{
		fHeight = -FLT_MAX;
	}
} MAPDATA, *LPMAPDATA;

typedef struct __GameMapData
{
	float fHeight;          //지형의 높이값..
	DWORD bIsTileFull : 1;  //타일이 Full 이면 true 아니면 false..
	DWORD Tex1Dir     : 5;  //툴에서 사용하는 인덱스가 아님..
	DWORD Tex2Dir     : 5;
	DWORD Tex1Idx     : 10; //툴에서 사용하는 인덱스가 아님..
	DWORD Tex2Idx     : 10;

	__GameMapData()
	{
		bIsTileFull = true;
		fHeight     = -FLT_MAX;
		Tex1Idx     = 0;
		Tex1Dir     = 0;
		Tex2Idx     = 0;
		Tex2Dir     = 0;
	}

} GAMEMAPDATA, *LPGAMEMAPDATA;

typedef struct __SeedGroup
{
	unsigned char Obj_Id     : 4;
	unsigned char Seed_Count : 3;
	unsigned char sub_flage  : 1;

	__SeedGroup* SeedGroup_Sub;

	__SeedGroup()
	{
		Obj_Id        = 0;
		Seed_Count    = 0;
		SeedGroup_Sub = nullptr;
	}
} SEEDGROUP, *LPSEEDGROUP;

#endif //end of #ifndef __LYTERRAINDEF_H__