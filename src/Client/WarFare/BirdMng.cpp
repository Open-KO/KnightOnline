// BirdMng.cpp: implementation of the CBirdMng class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BirdMng.h"

CBirdMng::CBirdMng()
{
}

CBirdMng::~CBirdMng()
{
}

void CBirdMng::Release()
{
	_birds.clear();
}

void CBirdMng::LoadFromFile(const std::string& szFN)
{
	Release();

	if (szFN.empty())
		return;

	FILE* stream = fopen(szFN.c_str(), "r"); //text파일로 만든다
	if (stream == nullptr)
		return;

	char szRrcName[_MAX_PATH + 1] {};
	int iBirdCount = 0;

	_birds.clear();

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	int result = fscanf(stream, "count = %d\n", &iBirdCount);
	__ASSERT(result != EOF, "잘못된 Machine 세팅 파일");

	if (result == EOF)
	{
		fclose(stream);
		return;
	}

	if (iBirdCount > 0)
		_birds.resize(iBirdCount);

	for (CBird& bird : _birds)
	{
		if (result != EOF)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
			result = fscanf(stream, "%s\n", szRrcName);
			__ASSERT(result != EOF, "잘못된 bird list 세팅 파일");
		}

		bird.LoadBird(szRrcName);
	}

	fclose(stream);
}

void CBirdMng::Tick()
{
	for (CBird& bird : _birds)
		bird.Tick();
}

void CBirdMng::Render()
{
	for (CBird& bird : _birds)
		bird.Render();
}
