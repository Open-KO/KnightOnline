// N3AlphaPrimitiveManager.h: interface for the CN3AlphaPrimitiveManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3ALPHAPRIMITIVEMANAGER_H__616E318C_49E4_4468_9719_E62E384FC48D__INCLUDED_)
#define AFX_N3ALPHAPRIMITIVEMANAGER_H__616E318C_49E4_4468_9719_E62E384FC48D__INCLUDED_

#pragma once

#include "My_3DStruct.h"

struct __AlphaPrimitive
{
	float fCameraDistance           = 0.0f;
	uint32_t dwBlendSrc             = 0;
	uint32_t dwBlendDest            = 0;
	int nRenderFlags                = RF_NOTHING;
	LPDIRECT3DTEXTURE9 lpTex        = nullptr;
	uint32_t dwFVF                  = 0;
	D3DPRIMITIVETYPE ePrimitiveType = D3DPT_TRIANGLELIST;
	int nPrimitiveCount             = 0;
	uint32_t dwPrimitiveSize        = 0;
	BOOL bUseVB                     = FALSE;
	const void* pwIndices           = nullptr;
	int nVertexCount                = 0;
	const void* pVertices           = nullptr;
	__Matrix44 MtxWorld             = __Matrix44::GetIdentity();
};

inline constexpr int MAX_ALPHAPRIMITIVE_BUFFER = 1024;

class CN3AlphaPrimitiveManager
{
protected:
	int m_nToDrawCount;                                    // 그려야 할 버퍼 갯수
	__AlphaPrimitive m_Buffers[MAX_ALPHAPRIMITIVE_BUFFER]; // 프리미티브 버퍼..

public:
	int ToDrawCount()
	{
		return m_nToDrawCount;
	}
	__AlphaPrimitive* Add();

	void Render();

	static int SortByCameraDistance(const void* pArg1, const void* pArg2); // 정렬 함수..

	CN3AlphaPrimitiveManager();
	virtual ~CN3AlphaPrimitiveManager();
};

#endif // !defined(AFX_N3ALPHAPRIMITIVEMANAGER_H__616E318C_49E4_4468_9719_E62E384FC48D__INCLUDED_)
