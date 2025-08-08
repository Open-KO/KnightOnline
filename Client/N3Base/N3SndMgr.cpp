// N3SndMgr.cpp: implementation of the CN3SndMgr class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3SndMgr.h"
#include "N3SndObj.h"
#include "N3SndObjStream.h"
#include "N3Base.h"

#include <mpg123.h>
#include <filesystem>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3SndMgr::CN3SndMgr()
{
	m_bSndEnable = false;	
	m_bSndDuplicated = false;
}

CN3SndMgr::~CN3SndMgr()
{
	Release();
}

//
//	엔진 초기화..
//
void CN3SndMgr::Init(HWND hWnd)
{
	Release();
	m_bSndEnable = CN3SndObj::StaticInit(hWnd);
	m_Tbl_Source.LoadFromFile("Data\\sound.tbl");
}

CN3SndObj* CN3SndMgr::CreateObj(int iID, e_SndType eType)
{
	TABLE_SOUND* pTbl = m_Tbl_Source.Find(iID);
	if(pTbl==NULL) return NULL;

	return this->CreateObj(pTbl->szFN, eType);
}

CN3SndObj* CN3SndMgr::CreateObj(std::string szFN, e_SndType eType)
{
	if(!m_bSndEnable) return NULL;

	if (!PreprocessFilename(szFN))
		return nullptr;

	CN3SndObj* pObjSrc = NULL;
	itm_Snd it = m_SndObjSrcs.find(szFN);
	if(it == m_SndObjSrcs.end()) // 못 찾았다... 새로 만들자..
	{
		pObjSrc = new CN3SndObj();
		if(false == pObjSrc->Create(szFN, eType)) // 새로 로딩..
		{
			delete pObjSrc; pObjSrc = NULL;
			return NULL;
		}
		m_SndObjSrcs.insert(val_Snd(szFN, pObjSrc)); // 맵에 추가한다..
	}
	else pObjSrc = it->second;

	if(!m_bSndDuplicated) return pObjSrc;//this_Snd

	if(NULL == pObjSrc) return NULL;

	CN3SndObj* pObjNew = new CN3SndObj();
	if(false == pObjNew->Duplicate(pObjSrc, eType)) // Duplicate 처리..
	{
		delete pObjNew; pObjNew = NULL;
		return NULL;
	}
	
	if(pObjNew) m_SndObjs_Duplicated.push_back(pObjNew); // 리스트에 넣는다...
	return pObjNew;
}

CN3SndObjStream* CN3SndMgr::CreateStreamObj(std::string szFN)
{
	if (!PreprocessFilename(szFN))
		return nullptr;

	CN3SndObjStream* pObj = new CN3SndObjStream();
	if(false == pObj->Create(szFN))
	{
		delete pObj; pObj = NULL;
		return NULL;
	}

	m_SndObjStreams.push_back(pObj); // 리스트에 넣기..

	return pObj;
}

CN3SndObjStream* CN3SndMgr::CreateStreamObj(int iID)
{
	TABLE_SOUND* pTbl = m_Tbl_Source.Find(iID);
	if(pTbl==NULL) return NULL;

	return this->CreateStreamObj(pTbl->szFN);
}

void CN3SndMgr::ReleaseStreamObj(CN3SndObjStream** ppObj)
{
	if(NULL == ppObj || NULL == *ppObj) return;

	itl_SndStream it = m_SndObjStreams.begin(), itEnd = m_SndObjStreams.end();
	for(; it != itEnd; it++)
	{
		if(*ppObj == *it) 
		{
			delete *ppObj; *ppObj = NULL;
			m_SndObjStreams.erase(it);
			break;
		}
	}
}


//
//	TickTick...^^
//
void CN3SndMgr::Tick()
{
	if(!m_bSndEnable) return;

//	m_Eng.SetListenerPos(&(CN3Base::s_CameraData.vEye));
//	__Vector3 vUP(0.0f, 1.0f, 0.0f);
//	__Vector3 vDir = CN3Base::s_CameraData.vAt - CN3Base::s_CameraData.vEye;
//
//	if(vDir.Magnitude() <= FLT_MIN) vDir.Set(0.0f, 0.0f, 1.0f);
//
//	m_Eng.SetListenerOrientation(&vDir, &vUP);
//

/*
	CN3SndObj* pObj = NULL;
	itl_Snd it = m_SndObjs_Duplicated.begin(), itEnd = m_SndObjs_Duplicated.end();
	for(; it != itEnd; it++)
	{
		pObj = *it;
		pObj->Tick();
	}
*/
	itl_Snd it, itEnd;//this_Snd
	CN3SndObj* pObj = NULL;
	if(!m_bSndDuplicated)
	{
		itm_Snd it_m = m_SndObjSrcs.begin(), itEnd_m = m_SndObjSrcs.end();
		for(; it_m != itEnd_m; it_m++)
		{
			pObj = it_m->second;
			pObj->Tick();
		}
	}
	else
	{
		it = m_SndObjs_Duplicated.begin();
		itEnd = m_SndObjs_Duplicated.end();
		for(; it != itEnd; it++)
		{
			pObj = *it;
			pObj->Tick();
		}
	}


	it = m_SndObjs_PlayOnceAndRelease.begin();
	itEnd = m_SndObjs_PlayOnceAndRelease.end();
	for(; it != itEnd; )
	{
		pObj = *it;
		pObj->Tick();
		if(false == pObj->IsPlaying())
		{
			it = m_SndObjs_PlayOnceAndRelease.erase(it);
			delete pObj; pObj = NULL;
		}
		else it++;
	}

	CN3SndObjStream* pObj2 = NULL;
	itl_SndStream it2 = m_SndObjStreams.begin(), itEnd2 = m_SndObjStreams.end();
	for(; it2 != itEnd2; it2++)
	{
		pObj2 = *it2;
		if (pObj2) pObj2->Tick();
	}

//	itm_Snd it2 = m_SndObjSrcs.begin();
//	for(; it2 != m_SndObjSrcs.end(); it2++)
//	{
//		pObj = it2->second.pSndObj;
//		if(pObj) pObj->Tick();
//	}

	CN3SndObj::StaticTick(); // CommitDeferredSetting...
}


//
//	Obj하나 무효화..
void CN3SndMgr::ReleaseObj(CN3SndObj** ppObj)
{
	if(NULL == ppObj || NULL == *ppObj) return;
	std::string szFN = (*ppObj)->m_szFileName; // 파일 이름을 기억하고..

	itl_Snd it = m_SndObjs_Duplicated.begin(), itEnd = m_SndObjs_Duplicated.end();
	for(; it != itEnd; it++)
	{
		if(*ppObj == *it)
		{
			m_SndObjs_Duplicated.erase(it);
			delete *ppObj; *ppObj = NULL; // 객체 지우기..
			return;
		}
	}

	it = m_SndObjs_PlayOnceAndRelease.begin();
	itEnd = m_SndObjs_PlayOnceAndRelease.end();
	for(; it != itEnd; it++)
	{
		if(*ppObj == *it)
		{
			m_SndObjs_PlayOnceAndRelease.erase(it);
			delete *ppObj; *ppObj = NULL; // 객체 지우기..
			return;
		}
	}

	*ppObj = NULL; // 포인터만 널로 만들어 준다..

/*	itm_Snd it = m_SndObjSrcs.find(szFN);
	if(it != m_SndObjSrcs.end()) // 찾았다..
	{
		CN3SndObj* pObj = it->second;
		delete pObj;
		m_SndObjSrcs.erase(it);
	}
	else
	{
		itl_Snd it2 = m_SndObjs_PlayOnceAndRelease.begin();
		for(; it2 != m_SndObjs_PlayOnceAndRelease.end(); it2++)
		{
			CN3SndObj* pObj = *it2;
			if(pObj == *ppObj)
			{
				delete pObj;
				m_SndObjs_PlayOnceAndRelease.erase(it2);
				break;
			}		
		}
	}
*/
}


//
//	Release Whole Objects & Sound Sources & Sound Engine..
//
void CN3SndMgr::Release()
{
	if(!m_bSndEnable) return;

	CN3SndObj* pObj = NULL;
	itm_Snd it = m_SndObjSrcs.begin(), itEnd = m_SndObjSrcs.end();
	for(; it != itEnd; it++)
	{
		pObj = it->second;
		if(pObj) delete pObj;
	}
	m_SndObjSrcs.clear();

	itl_Snd it2 = m_SndObjs_Duplicated.begin(), itEnd2 = m_SndObjs_Duplicated.end();
	for(; it2 != itEnd2; it2++)
	{
		pObj = *it2;
		if(pObj) delete pObj;
	}
	m_SndObjs_Duplicated.clear();

	it2 = m_SndObjs_PlayOnceAndRelease.begin();
	itEnd2 = m_SndObjs_PlayOnceAndRelease.end();
	for(; it2 != itEnd2; it2++)
	{
		pObj = *it2;
		if(pObj) delete pObj;
	}
	m_SndObjs_PlayOnceAndRelease.clear();

	CN3SndObjStream* pObj2 = NULL;
	itl_SndStream it3 = m_SndObjStreams.begin(), itEnd3 = m_SndObjStreams.end();
	for(; it3 != itEnd3; it3++)
	{
		pObj2 = *it3;
		if(pObj2) delete pObj2;
	}
	m_SndObjStreams.clear();

	CN3SndObj::StaticRelease();
}


// 이 함수는 한번 플레이 하고 그 포인터를 다시 쓸수있게 ReleaseObj를 호출한다.
// 대신 위치는 처음 한번밖에 지정할 수 없다.
bool CN3SndMgr::PlayOnceAndRelease(int iSndID, const _D3DVECTOR* pPos)
{
	if(!m_bSndEnable) return false;

	TABLE_SOUND* pTbl = m_Tbl_Source.Find(iSndID);
	if(pTbl==NULL || pTbl->szFN.empty()) return false;
	
	CN3SndObj* pObjSrc = NULL;
	itm_Snd it = m_SndObjSrcs.find(pTbl->szFN);
	if(it == m_SndObjSrcs.end()) // 못 찾았다... 새로 만들자..
	{
		pObjSrc = new CN3SndObj();
		if(false == pObjSrc->Create(pTbl->szFN, (e_SndType)pTbl->iType)) // 새로 로딩..
		{
			delete pObjSrc; pObjSrc = NULL;
			return NULL;
		}
		m_SndObjSrcs.insert(val_Snd(pTbl->szFN, pObjSrc)); // 맵에 추가한다..
		if(!m_bSndDuplicated) pObjSrc->Play(pPos);//this_Snd
	}
	else pObjSrc = it->second;

	if(NULL == pObjSrc) return false;

	if(!m_bSndDuplicated)
	{
		pObjSrc->Play(pPos); //this_Snd
		return true;
	}

	CN3SndObj* pObj = new CN3SndObj();
	if(false == pObj->Duplicate(pObjSrc, (e_SndType)pTbl->iType)) // Duplicate 처리..
	{
		delete pObj; pObj = NULL;
		return NULL;
	}
	
	if(pObj) // 리스트에 넣는다...noah
	{
		m_SndObjs_PlayOnceAndRelease.push_back(pObj);
		pObj->Play(pPos);
		return true;
	}
	return false;
/*
	CN3SndObj* pObj = new CN3SndObj();
	if(false == pObj->Create(pTbl->szFN, (e_SndType)pTbl->iType))
	{
		delete pObj; pObj = NULL;
		return false;
	}
	pObj->Play(pPos);
	m_SndObjs_PlayOnceAndRelease.push_back(pObj);
	return true;
*/
}

bool CN3SndMgr::PreprocessFilename(std::string& szFN)
{
	// Expect an extension (.mp3, .wav)
	if (szFN.length() < 4)
		return false;

	// Officially it has native support for MP3 decoding.
	// We decode back to WAV (one-time) and use the new filename instead.
	// Ideally this would just load it into memory directly, but that would require restructuring
	// a little.
	// This approach is fairly unintrusive and only incurs the performance hit once.
	// There's also very few mp3 files to actually convert, so space should not be an issue.
	// Finally, the game requires admin access, so it should have write access.
	if (_stricmp(&szFN[szFN.length() - 4], ".mp3") == 0)
	{
		if (!DecodeMp3ToWav(szFN))
			return false;
	}

	return true;
}

bool CN3SndMgr::DecodeMp3ToWav(std::string& filename)
{
	std::filesystem::path newPath(filename);

	// Differentiate the converted files from any that already exist.
	// We want to be able to identify them for ignoring (so they don't make the repo 'dirty'),
	// and we generally don't want to assume any old, existing wav files match.
	newPath.replace_extension(".mp3.wav");

	// If we've already converted this file, we can just use it immediately.
	std::error_code fsErrorCode = {};
	if (std::filesystem::exists(newPath, fsErrorCode))
	{
		filename = newPath.string();
		return true;
	}

	// We have yet to convert it, so we need to load it up and decode it.
	int error = MPG123_ERR;

	mpg123_handle* mpgHandle = mpg123_new(nullptr, &error);
	if (mpgHandle == nullptr)
		return false;

	// Force output as 16-bit PCM - preserve sample rate and channel count.
	mpg123_format(mpgHandle, 0, 0, MPG123_ENC_SIGNED_16);

	error = mpg123_open(mpgHandle, filename.c_str());
	if (error != MPG123_OK)
	{
#ifdef _N3GAME
		CLogWriter::Write("Failed to open MP3: {} ({})",
			filename, error);
#endif
		mpg123_delete(mpgHandle);
		return false;
	}

	long rate = 0;
	int channels = 0, encoding = 0;

	error = mpg123_getformat(mpgHandle, &rate, &channels, &encoding);
	if (error != MPG123_OK)
	{
#ifdef _N3GAME
		CLogWriter::Write("Failed to get MP3 format: {} ({}: {})",
			filename, error, mpg123_strerror(mpgHandle));
#endif

		mpg123_delete(mpgHandle);
		return false;
	}

	off_t sampleCount = mpg123_length(mpgHandle);
	if (sampleCount < 0)
	{
#ifdef _N3GAME
		CLogWriter::Write("Failed to get total MP3 samples per channel: {}",
			filename);
#endif

		mpg123_delete(mpgHandle);
		return false;
	}

	std::string newFilename = newPath.string();

	// Open the new WAV file for writing.
	FILE* fp = fopen(newFilename.c_str(), "wb");
	if (fp == nullptr)
	{
#ifdef _N3GAME
		CLogWriter::Write("Failed to open file for writing decoded MP3 to: {}",
			newFilename);
#endif
		return false;
	}

	const int sampleSizeBytes = mpg123_encsize(encoding);

	// Initialise header to defaults
	WavFileHeader wavFileHeader = {};

	// Setup the file header.
	wavFileHeader.Format.AudioFormat	= 1; // PCM
	wavFileHeader.Format.NumChannels	= static_cast<uint16_t>(channels);
	wavFileHeader.Format.SampleRate		= static_cast<uint16_t>(rate);
	wavFileHeader.Format.BitsPerSample	= static_cast<uint16_t>(sampleSizeBytes * 8);
	wavFileHeader.Format.BytesPerBlock	= wavFileHeader.Format.NumChannels * wavFileHeader.Format.BitsPerSample / 8;
	wavFileHeader.Format.BytesPerSec	= wavFileHeader.Format.SampleRate * wavFileHeader.Format.BytesPerBlock;

	size_t done = 0, decodedBytes = 0;
	const size_t frameSize = mpg123_outblock(mpgHandle);
	std::vector<uint8_t> frameBlock(frameSize);

	// Skip the header - we'll write that at the end.
	fseek(fp, sizeof(WavFileHeader), SEEK_SET);

	error = mpg123_read(mpgHandle, &frameBlock[0], frameSize, &done);
	while (error == MPG123_OK)
	{
		decodedBytes += done;

		fwrite(&frameBlock[0], done, 1, fp);
		error = mpg123_read(mpgHandle, &frameBlock[0], frameSize, &done);
	}

	mpg123_delete(mpgHandle);

	if (error != MPG123_DONE)
	{
		fclose(fp);
		std::remove(newFilename.c_str());

#ifdef _N3GAME
		CLogWriter::Write("Failed to decode MP3: {} ({} - decoded {} bytes)",
			filename, error, decodedBytes);
#endif
		return false;
	}

	wavFileHeader.FileSize += static_cast<uint32_t>(decodedBytes);
	wavFileHeader.Data.Size = static_cast<uint32_t>(decodedBytes);

	// Write out the header.
	long endOfFileOffset = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fwrite(&wavFileHeader, sizeof(WavFileHeader), 1, fp);

	// Seek back to the known end, before the file is closed and flushed.
	fseek(fp, endOfFileOffset, SEEK_SET);
	fclose(fp);

	filename = std::move(newFilename);
	return true;
}
