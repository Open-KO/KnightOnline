// NPCPath.h: interface for the CNPCPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPCPATH_H__FA4815FA_8602_45C2_BEF2_5154AB5A4770__INCLUDED_)
#define AFX_NPCPATH_H__FA4815FA_8602_45C2_BEF2_5154AB5A4770__INCLUDED_

#pragma once

#include <N3Base/N3Base.h>
#include <list>

class CLyTerrain;

class CNPCPath : public CN3Base
{
public:
	char m_strPathName[256];   // 길이름..
	char m_strNPCName[80];     // NPC 이름.
	int m_iNPCID;              // NPC 종류..
	int m_iNumNPC;             // 몇마리?..

	__Vector3 m_LTStartVertex; // 시작영역의 모서리 두점..
	__Vector3 m_RBStartVertex;

	//CRect	m_rcStartRect;		// NPC생기는 영역.
	int m_iRegenTime; // NPC가 죽었다가 다시 살아나는데 걸리는 시간.
	int m_iActType;
	int m_iZoneID;

	std::list<__Vector3> m_Path; //path지정하는 점들....시작점~끝점..

	CLyTerrain* m_pRefTerrain;

	DWORD m_dwColor;
	int m_iVersion;

	__Vector3 m_LTActVertex; // 시작영역의 모서리 두점..
	__Vector3 m_RBActVertex;

	unsigned char m_cAttr_Create;
	unsigned char m_cAttr_Regen;
	unsigned char m_cAttr_Group;
	unsigned char m_cAttr_Option;

public:
	int GetSize() const
	{
		return static_cast<int>(m_Path.size());
	}

	bool CheckValid();
	void TransPos(float x, float z);
	void Load(File& file);
	void Save(File& file);
	bool GetPath(int idx, __Vector3* pPos); // idx째 점 가져오기..
	void AddPos(__Vector3 Path);            //path더하기.
	void DelPrevPos();
	CNPCPath();
	~CNPCPath() override;
};

#endif // !defined(AFX_NPCPATH_H__FA4815FA_8602_45C2_BEF2_5154AB5A4770__INCLUDED_)
