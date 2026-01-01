// N3AlphaPrimitiveManager.h: interface for the CN3AlphaPrimitiveManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3ALPHAPRIMITIVEMANAGER_H__616E318C_49E4_4468_9719_E62E384FC48D__INCLUDED_)
#define AFX_N3ALPHAPRIMITIVEMANAGER_H__616E318C_49E4_4468_9719_E62E384FC48D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "My_3DStruct.h"

//const uint32_t RF_NOTHING = 0;
//const uint32_t RF_ALPHABLENDING = 1;
//const uint32_t RF_NOTUSEFOG = 2;
//const uint32_t RF_DOUBLESIDED = 4;

struct __AlphaPrimitive
{
	float				fCameraDistance;	// 카메라와의 거리..
	uint32_t				dwBlendSrc;			
	uint32_t				dwBlendDest;
	int 				nRenderFlags;		// 렌더링 플래그.. RF_... 참조..
	LPDIRECT3DTEXTURE9	lpTex;				// Texture pointer
	uint32_t				dwFVF;				// flexible vertex format
	D3DPRIMITIVETYPE	ePrimitiveType;	// Primitive Type 
	int					nPrimitiveCount;	// PrimitiveCount
	uint32_t				dwPrimitiveSize;	// Primitive Size .. stream 0 stride
	BOOL				bUseVB;				// 버텍스 버퍼, 인덱스 버퍼를 사용할 것인가 아닌가
	const void*			pwIndices;			// 만약 Index 기반이면... nullptr 이 아닌것을 넣으면 된다.
	int					nVertexCount;
	const void*			pVertices;			// 삼각형.. 벡터 형이지만.. 강제 형변환을 통해 다양한 점형식이 들어오도록 써야 한다..
	__Matrix44			MtxWorld;			// Matrix
};

const int MAX_ALPHAPRIMITIVE_BUFFER = 1024;

class CN3AlphaPrimitiveManager
{
protected:
	int					m_nToDrawCount; // 그려야 할 버퍼 갯수 
	__AlphaPrimitive	m_Buffers[MAX_ALPHAPRIMITIVE_BUFFER]; // 프리미티브 버퍼..

public:
	int					ToDrawCount() { return m_nToDrawCount; }
	__AlphaPrimitive*	Add();

	void		Render();

	static int SortByCameraDistance(const void *pArg1, const void *pArg2); // 정렬 함수..
	
	CN3AlphaPrimitiveManager();
	virtual ~CN3AlphaPrimitiveManager();
};

#endif // !defined(AFX_N3ALPHAPRIMITIVEMANAGER_H__616E318C_49E4_4468_9719_E62E384FC48D__INCLUDED_)
