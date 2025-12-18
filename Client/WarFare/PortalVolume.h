// PortalVolume.h: interface for the CPortalVolume class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORTALVOLUME_H__81823A3C_844B_4385_8D5B_F62606F89593__INCLUDED_)
#define AFX_PORTALVOLUME_H__81823A3C_844B_4385_8D5B_F62606F89593__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3Shape.h>
#include <list>

#define INDOOR_FOLDER "N3Indoor\\"
const float fBaseVolumnSize = 1.0f;

enum e_ExtBool	{ TYPE_UNKNOWN = 0, TYPE_TRUE };

class CPvsMgr;
class CPortalVolume;

//////////////////////////////////////////////////////////////////
typedef struct tagShapeInfo	: public CN3Transform	
{
	int									m_iID;
	std::string						m_strShapeFile;

	int									m_iBelong;				// 소속 - 0:소속 없음 1:엘모라드 2:카루스 3:?? ....
	int									m_iEventID;				 // Event ID
	int									m_iEventType;		  // Event Type
	int									m_iNPC_ID;			   // NPC 로 쓰는 오브젝트일 경우 NPC ID
	int									m_iNPC_Status;	    // NPC 로 쓰는 오브젝트일 경우 Default Status

	CN3Shape*					m_pShape;

	//..
	tagShapeInfo()
	{
		m_iID = -1;
		m_strShapeFile = "";
		m_pShape = nullptr;		

		m_iBelong = 0;
		m_iEventID = 0;	
		m_iEventType = 0;
		m_iNPC_ID = 0;	
		m_iNPC_Status = 0;	
	}
	
	const tagShapeInfo& operator = (const tagShapeInfo& si)
	{
		m_iID = si.m_iID;
		m_strShapeFile = si.m_strShapeFile;
		m_pShape = si.m_pShape;
		m_iBelong = si.m_iBelong;
		m_iEventID = si.m_iEventID;	
		m_iEventType = si.m_iEventType;
		m_iNPC_ID = si.m_iNPC_ID;	
		m_iNPC_Status = si.m_iNPC_Status;
		return *this;
	}

} ShapeInfo;

typedef struct tagVisPartIndex {
	int m_iPartIndex;
	std::vector<int> m_ivVector;
} __VPI;
typedef std::list<__VPI>::iterator	viter;

typedef struct tagShapePart
{
	int									m_iID;
	std::list<__VPI>			 m_viIndex;									// Visible Index List..				

	tagShapePart()
	{
		m_iID = -1;
	}	
	void Clear()
	{
		__VPI vpi;
		viter vit =	m_viIndex.begin();	
		while (vit != m_viIndex.end())
		{
			vpi = *vit++;
			vpi.m_ivVector.clear();
		}
		m_viIndex.clear();
	}
	~tagShapePart()
	{
		Clear();
	}
} ShapePart;

typedef struct tagCollisionIndex
{
	int m_iID;
	std::vector<uint32_t> m_ivVector;
	tagCollisionIndex()
	{
		m_iID = -1;
	}
} __ColIndex;

typedef struct tagIDAndPriority
{
	int						m_iID;
	int						m_iPriority;
}	IDAndPriority;

typedef struct tagVisPortalPriority
{
	CPortalVolume* m_pVol;
	int						m_iPriority;

	tagVisPortalPriority()
	{
		m_pVol = nullptr;
		m_iPriority = 100;
	}
}	VisPortalPriority;

typedef std::list<VisPortalPriority>::iterator vppiter;
typedef std::list<ShapePart*>::iterator spiter;
typedef std::list<ShapeInfo*>::iterator siiter;
typedef std::list<__ColIndex*>::iterator ciiter;
typedef std::list<IDAndPriority>::iterator idapiter;
//////////////////////////////////////////////////////////////////

class CPortalVolume	: public CN3Transform
{
	friend class CPvsMgr;

	int m_iID;
	CPvsMgr* m_pManager;

	e_ExtBool	m_eRenderType;

	const float m_fOffs;				// 일반적인 옵셋..
	const float m_fHeightOffs;		// 높이 옵셋..
	const float m_fVolOffs;			  // Volume 법위 옵셋..
	const float m_fPickIncline;		// 픽킹 기울기 체크..
	const float m_fCameraOffs;		// 카메라 높이..
	
	//////////////////////////////////////////////////////////////////////////////////////
	std::list<ShapeInfo* > m_plShapeInfoList;
	std::list<IDAndPriority>	m_piVisibleIDList;						// 로드에 필요한 중간 데이터..		
	std::list<ShapePart* > m_lpShapePartList;	
	std::list<__ColIndex* >	m_lpShapeColPartList;
	std::list<VisPortalPriority>	m_pVisiblePvsList;		
	//////////////////////////////////////////////////////////////////////////////////////

	__Vector3					  m_pvVertex[8];
	uint16_t			   m_pIndex[36];

	int									m_iPriority;							//.. 100 정도로 초기화..

private:
	void DeleteAllPvsObj();
	bool Load(File& file) override;
	bool IsInVolumn(__Vector3 vec);

	void Render();
	void RenderShape();
	void RenderCollision();

	////////////////////////////////////////////////////////////////
	bool CheckCollisionCameraWithTerrain(__Vector3& vEyeResult, const __Vector3& vAt, float fNP);
	bool CheckCollisionCameraWithShape(__Vector3& vEyeResult, const __Vector3& vAt, float fNP);
	bool GetHeightWithTerrain(float fx, float fz, float& fy);
	float GetHeightNearstPosWithShape(const __Vector3& vPos, float fDist, __Vector3* pvNormal = nullptr); 
	bool IsInTerrainWithTerrain(__Vector3& vec);
	BOOL PickWideWithTerrain(int x, int y, __Vector3& vPick);
	CN3Shape* PickWithShape(int iXScreen, int iYScreen, bool bMustHaveEvent, __Vector3* pvPick = nullptr);
	CN3Shape* ShapeGetByIDWithShape(int iID);
	bool CheckCollisionWithShape(	const __Vector3& vPos,				 // 충돌 위치
														const __Vector3& vDir,				   // 방향 벡터
														float fSpeedPerSec,					    // 초당 움직이는 속도
														__Vector3* pvCol,						 // 충돌 지점
														__Vector3* pvNormal,				  // 충돌한면의 법선벡터
														__Vector3* pVec);						// 충돌한 면 의 폴리곤 __Vector3[3]

public:
	CPortalVolume();
	~CPortalVolume() override;
};

#endif // !defined(AFX_PORTALVOLUME_H__81823A3C_844B_4385_8D5B_F62606F89593__INCLUDED_)
