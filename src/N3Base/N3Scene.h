// N3Scene.h: interface for the CN3Scene class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3Scene_h__INCLUDED_)
#define AFX_N3Scene_h__INCLUDED_

#pragma once

#include "N3Camera.h"
#include "N3Light.h"
#include "N3Shape.h"
#include "N3Chr.h"
#include "N3Mesh.h"

const int MAX_SCENE_CAMERA    = 32;
const int MAX_SCENE_LIGHT     = 32;
const int MAX_SCENE_SHAPE     = 10240;
const int MAX_SCENE_CHARACTER = 4096;

#include <vector>

typedef std::vector<class CN3Shape*>::iterator it_Shape;
typedef std::vector<class CN3Chr*>::iterator it_Chr;

class CN3Scene : public CN3BaseFileAccess
{
public:
	float m_fFrmCur, m_fFrmStart, m_fFrmEnd; // 현재, 시작, 끝 프레임..
	bool m_bDisableDefaultLight;             // 참이면 기본라이트를 끈다..
	D3DCOLOR m_AmbientLightColor;

protected:
	int m_nCameraActive; // 현재 선택된 카메라..
	int m_nCameraCount;
	int m_nLightCount;

	class CN3Camera* m_pCameras[MAX_SCENE_CAMERA];
	class CN3Light* m_pLights[MAX_SCENE_LIGHT];
	std::vector<class CN3Shape*> m_Shapes;
	std::vector<class CN3Chr*> m_Chrs;

public:
	void DefaultCameraAdd();
	void DefaultLightAdd();

	bool LoadDataAndResourcesFromFile(const std::string& szFileName);
	bool SaveDataAndResourcesToFile(const std::string& szFileName);

	//	bool CheckOverlappedShapesAndReport();
	//	void DeleteOverlappedShapes();

	void Tick(float fFrm = FRAME_SELFPLAY);
	void TickCameras();
	void TickLights();
	void TickShapes();
	void TickChrs();
	void Render();

	int CameraCount() const
	{
		return m_nCameraCount;
	}

	int CameraAdd(CN3Camera* pCamera);
	void CameraDelete(CN3Camera* pCamera);
	void CameraDelete(int iIndex);
	CN3Camera* CameraGet(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_nCameraCount)
			return nullptr;
		return m_pCameras[iIndex];
	}

	void CameraSetActive(int iIndex);
	int CameraGetActiveNumber()
	{
		return m_nCameraActive;
	};
	CN3Camera* CameraGetActive()
	{
		if (m_nCameraActive < 0 || m_nCameraActive >= m_nCameraCount)
			return nullptr;
		return m_pCameras[m_nCameraActive];
	}

	int LightCount() const
	{
		return m_nLightCount;
	}

	int LightAdd(CN3Light* pLight);
	void LightDelete(CN3Light* pLight);
	void LightDelete(int iIndex);
	CN3Light* LightGet(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_nLightCount)
			return nullptr;
		return m_pLights[iIndex];
	}

	int ShapeCount() const
	{
		return static_cast<int>(m_Shapes.size());
	}

	int ShapeAdd(CN3Shape* pShape);
	void ShapeDelete(CN3Shape* pShape);
	void ShapeDelete(int iIndex);
	CN3Shape* ShapeGet(int iIndex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_Shapes.size()))
			return nullptr;

		return m_Shapes[iIndex];
	}

	CN3Shape* ShapeGetByFileName(const std::string& str)
	{
		for (CN3Shape* shape : m_Shapes)
		{
			if (str == shape->FileName())
				return shape;
		}

		return nullptr;
	}

	void ShapeRelease();

	int ChrCount() const
	{
		return static_cast<int>(m_Chrs.size());
	}

	int ChrAdd(CN3Chr* pChr);
	void ChrDelete(int iIndex);
	void ChrDelete(CN3Chr* pChr);
	CN3Chr* ChrGet(int iIndex)
	{
		if (iIndex < 0 || iIndex >= static_cast<int>(m_Chrs.size()))
			return nullptr;

		return m_Chrs[iIndex];
	}

	void ChrRelease();

	bool Load(File& file) override;
	bool Save(File& file) override;

	void Release() override;

	CN3Scene();
	~CN3Scene() override;
};

#endif // !defined(AFX_N3Scene_h__INCLUDED_)
