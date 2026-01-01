#ifndef __N3TERRAINDEF_H_
#define __N3TERRAINDEF_H_

#include <float.h>
#include <list>

#include "N3Base.h"

//-----------------------------------------------------------------------------
const int PATCH_TILE_SIZE = 8;
const int TILE_SIZE = 4;
const int MAX_LOD_LEVEL = 10;
const int MIN_LOD_LEVEL = 0;
const int MAX_PATCH_LEVEL = 3;
const int PATCH_PIXEL_SIZE = 32;
const int UNITUV = 32;
const int LIGHTMAP_TEX_SIZE = 16;
const int TILE_PIXEL_SIZE = 128;
const int COLORMAPTEX_SIZE = 128;
const int DISTANCE_TABLE_SIZE = 64;
const int MAX_GRASS = 8;
const int MAX_TERRAIN_SOUND_EFFECT = 4;

enum
{
    DIR_LT = 0,
    DIR_CT = 1,
    DIR_RT = 2,
    DIR_LM = 3,
    DIR_CM = 4,
    DIR_RM = 5,
    DIR_LB = 6,
    DIR_CB = 7,
    DIR_RB = 8,
    DIR_WARP = 9
};

//-----------------------------------------------------------------------------
typedef struct __MapData
{
    float fHeight;

    uint32_t bIsTileFull : 1;
    uint32_t Tex1Dir : 5;
    uint32_t Tex2Dir : 5;
    uint32_t Tex1Idx : 10;
    uint32_t Tex2Idx : 10;

    __MapData(void)
    {
        fHeight = FLT_MIN;

        bIsTileFull = true;
        Tex1Dir = 0;
        Tex2Dir = 0;
        Tex1Idx = 1023;
        Tex2Idx = 1023;
    }
} MAPDATA, *LPMAPDATA;

//-----------------------------------------------------------------------------
typedef struct __FanInfo
{
    int NumFace;
    int NumVertex;
} FANINFO, *LPFANINFO;

//-----------------------------------------------------------------------------
typedef std::list<FANINFO> FanInfoList;
typedef FanInfoList::iterator FIIt;

#endif
