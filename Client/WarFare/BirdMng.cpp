// BirdMng.cpp: implementation of the CBirdMng class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BirdMng.h"
#include "Bird.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBirdMng::CBirdMng()
{
	m_pBird = nullptr;
	Release();
}

CBirdMng::~CBirdMng()
{
	Release();
}

void CBirdMng::Release()
{
	if(m_pBird)
	{
		delete[] m_pBird;
		m_pBird = nullptr;
	}
	m_iBirdCount = 0;
}

void CBirdMng::LoadFromFile(const std::string& szFN)
{
	Release();

	if(szFN.empty()) return;
	FILE* stream = fopen(szFN.c_str(), "r"); //text파일로 만든다 
	if (stream == nullptr)
		return;

	int i;
	char szRrcName[_MAX_PATH];
	int result = fscanf(stream, "count = %d\n", &m_iBirdCount);			__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	if(m_iBirdCount>0) m_pBird = new CBird[m_iBirdCount];

	for (i=0; i<m_iBirdCount; i++)
	{
		result = fscanf(stream, "%s\n", szRrcName);	__ASSERT(result != EOF, "잘못된 bird list 세팅 파일");
		m_pBird[i].LoadBird(szRrcName);		
	}
	fclose(stream);
}

void CBirdMng::Tick()
{
	for(int i=0; i<m_iBirdCount; i++)
	{
		m_pBird[i].Tick();
	}
}

void CBirdMng::Render()
{
	for(int i=0; i<m_iBirdCount; i++)
	{
		m_pBird[i].Render();
	}
}
