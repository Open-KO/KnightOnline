// N3SndObj.cpp: implementation of the CN3SndObj class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3SndObj.h"
#include "N3Base.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
LPDIRECTSOUND				CN3SndObj::s_lpDS = NULL;
LPDIRECTSOUND3DLISTENER		CN3SndObj::s_lpDSListener = NULL;		// 3D listener object
DS3DLISTENER				CN3SndObj::s_dsListenerParams;			// Listener properties
bool						CN3SndObj::s_bNeedDeferredTick = false;	// 3D Listener CommitDeferredSetting

CN3SndObj::CN3SndObj()
{
	m_lpDSBuff = NULL;
	m_lpDS3DBuff = NULL;
	m_bIsLoop = false;
	m_iVol = -1;
	m_szFileName = "";

	m_fFadeInTime = 0;
	m_fFadeOutTime = 0;
	m_fStartDelayTime = 0;
	m_fTmpSecPerFrm = 0;
	m_ePlayState = SNDSTATE_STOP;

	m_iMaxVolume = 100;
}

CN3SndObj::~CN3SndObj()
{
	Release();
}


//
//	Initialize....
//
void CN3SndObj::Init()
{
	Release();

	m_bIsLoop = false;
	m_iVol = -1;
	m_szFileName = "";

	m_fStartDelayTime = 0;
	m_fTmpSecPerFrm = 0;
	m_ePlayState = SNDSTATE_STOP;
	m_fFadeInTime = 0;
	m_fFadeOutTime = 0;

	m_iMaxVolume = 100;
}


//
//	Release...
//
void CN3SndObj::Release()
{
	if(m_lpDS3DBuff)
	{
		m_lpDS3DBuff->Release();
		m_lpDS3DBuff = NULL;
	}	

	if(m_lpDSBuff)
	{
		m_lpDSBuff->Stop();
		m_lpDSBuff->Release();
		m_lpDSBuff = NULL;
	}
}

bool CN3SndObj::StaticInit( HWND  hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate )
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER lpDSBPrimary = NULL;

	if(s_lpDS)
	{
		s_lpDS->Release();
		s_lpDS = NULL;
	}

    // Create IDirectSound using the primary sound device
    if( FAILED( hr = DirectSoundCreate( NULL, &s_lpDS, NULL ) ) )
        return false;

    // Set DirectSound coop level 
    if( FAILED( hr = s_lpDS->SetCooperativeLevel( hWnd, dwCoopLevel ) ) )
        return false;
    
    // Set primary buffer format
    // Get the primary buffer 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = NULL;
    if( FAILED( hr = s_lpDS->CreateSoundBuffer( &dsbd, &lpDSBPrimary, NULL ) ) ) return false;

    WAVEFORMATEX wfx;
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag      = WAVE_FORMAT_PCM; 
    wfx.nChannels       = (uint16_t) dwPrimaryChannels; 
    wfx.nSamplesPerSec  = dwPrimaryFreq; 
    wfx.wBitsPerSample  = (uint16_t) dwPrimaryBitRate; 
    wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    if( FAILED( hr = lpDSBPrimary->SetFormat(&wfx) ) ) return false;

	if(lpDSBPrimary)
	{
		lpDSBPrimary->Release();
		lpDSBPrimary = NULL;
	}


	// Create listener
    // Obtain primary buffer, asking it for 3D control
    ZeroMemory( &dsbd, sizeof(dsbd) );
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_MUTE3DATMAXDISTANCE | DSBCAPS_CTRL3D;
    if( FAILED( hr = s_lpDS->CreateSoundBuffer( &dsbd, &lpDSBPrimary, NULL ) ) )
        return false;

    if( FAILED( hr = lpDSBPrimary->QueryInterface( IID_IDirectSound3DListener, (VOID**)(&s_lpDSListener) ) ) )
    {
       if(lpDSBPrimary)
	   {
		   lpDSBPrimary->Release();
		   lpDSBPrimary = NULL;
	   }		   
	   return false;
    }

    if(lpDSBPrimary)
	{
		lpDSBPrimary->Release();
		lpDSBPrimary = NULL;
	}

	// Set listener 
	s_dsListenerParams.dwSize = sizeof(DS3DLISTENER);
	s_lpDSListener->GetAllParameters( &s_dsListenerParams );
	s_lpDSListener->SetRolloffFactor( DS3D_DEFAULTROLLOFFFACTOR/2, DS3D_IMMEDIATE);
	s_lpDSListener->SetDopplerFactor(0, DS3D_DEFERRED);
	
	s_bNeedDeferredTick = true;	// 3D Listener CommitDeferredSetting
    return true;
}

void CN3SndObj::StaticRelease()
{
	if(s_lpDSListener)
	{
		s_lpDSListener->Release();
		s_lpDSListener = NULL;
	}
	if(s_lpDS)
	{
		s_lpDS->Release();
		s_lpDS = NULL;
	}

	s_bNeedDeferredTick = false;	// 3D Listener CommitDeferredSetting
}

void CN3SndObj::StaticTick()
{
	if(false == s_bNeedDeferredTick) return;
	// 3D Listener CommitDeferredSetting

	s_lpDSListener->CommitDeferredSettings();
	s_bNeedDeferredTick = false;
}

bool CN3SndObj::Create(const std::string& szFN, e_SndType eType)
{
	if(NULL == s_lpDS) return false;
	if(SNDTYPE_2D != eType && SNDTYPE_3D != eType) return false;
	if(m_lpDSBuff) this->Init();

	CWaveFile	WaveFile;
	HRESULT hr = WaveFile.Open(szFN.c_str(), NULL, 1);	//#define WAVEFILE_READ   1
	if(FAILED(hr))
	{
#ifdef _N3GAME
		if (!szFN.empty())
			CLogWriter::Write("CN3SndObj::Create - WaveFile Open Failed.. ({})", szFN);
#endif
		return false;
	}

	DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
	dsbd.dwBufferBytes   = WaveFile.GetSize();
	dsbd.lpwfxFormat     = WaveFile.m_pwfx;

	if(SNDTYPE_2D == eType) // 2D 음원
	{
		dsbd.dwFlags		 = DSBCAPS_CTRLVOLUME; // | DSBCAPS_STATIC;
	}
	else if(SNDTYPE_3D == eType)	//3D 음원..
	{
		dsbd.dwFlags         = DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE; // | DSBCAPS_STATIC;
		dsbd.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
	}

	hr = s_lpDS->CreateSoundBuffer( &dsbd, &m_lpDSBuff, NULL );
	if(FAILED(hr))
	{
#ifdef _N3GAME
		CLogWriter::Write("CN3SndObj::Create - CreateSoundBuffer Failed.. ({})", szFN);
#endif
		return false;
	}

    if(!FillBufferWithSound(&WaveFile))
	{
#ifdef _N3GAME
		CLogWriter::Write("CN3SndObj::Create - FillBufferWithSound Failed.. ({})", szFN);
#endif
		return false;
	}

	m_lpDSBuff->SetCurrentPosition(0);
	if(SNDTYPE_3D == eType)	//3D 음원..
		if(S_OK != m_lpDSBuff->QueryInterface(IID_IDirectSound3DBuffer, (VOID**)(&m_lpDS3DBuff)))
			return false;

	m_szFileName = szFN; // 파일 이름을 기록한다..

	s_bNeedDeferredTick = true;	// 3D Listener CommitDeferredSetting
	return true;
}

bool CN3SndObj::Duplicate(CN3SndObj* pSrc, e_SndType eType, D3DVECTOR* pPos)
{
	if(NULL == s_lpDS || NULL == pSrc || NULL == pSrc->m_lpDSBuff)
		return false;
	if(m_lpDSBuff) this->Init();

	HRESULT hr = 0;
	if(DS_OK != s_lpDS->DuplicateSoundBuffer(pSrc->m_lpDSBuff, &(m_lpDSBuff)))
		return false;
	
	m_lpDSBuff->SetCurrentPosition(0);
	if(SNDTYPE_3D == eType)
	{
		if(S_OK != m_lpDSBuff->QueryInterface( IID_IDirectSound3DBuffer, (VOID**)(&m_lpDS3DBuff)))
			return false;
		
		/*
		if(pSrc->m_lpDS3DBuff)
		{
			DS3DBUFFER BuffParam;
			BuffParam.dwSize = sizeof(DS3DBUFFER);
			hr = pSrc->m_lpDS3DBuff->GetAllParameters(&BuffParam);
		  	hr = m_lpDS3DBuff->SetAllParameters(&BuffParam, DS3D_DEFERRED);			
		}
		*/

		if(pPos) this->SetPos(pPos);
		this->SetMaxDistance(100);
		
		D3DVECTOR vDir;
		vDir.x = 0.0f;	vDir.z = 0.0f;	vDir.y = 1.0f;
		this->SetConeOrientation(&vDir);
	}

	s_bNeedDeferredTick = true;	// 3D Listener CommitDeferredSetting
	m_szFileName = pSrc->m_szFileName;
	return true;
}

bool CN3SndObj::FillBufferWithSound(CWaveFile* pWaveFile)
{
    if( NULL == m_lpDSBuff || NULL == pWaveFile )
		return false; // 포인터들 점검..
	
    HRESULT hr; 
    VOID*   pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
    DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
    DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file 

	DSBCAPS dsbc; dsbc.dwSize = sizeof(dsbc);
	m_lpDSBuff->GetCaps(&dsbc);
	if(dsbc.dwBufferBytes != pWaveFile->GetSize())
		return false; // 사이즈 점검..

	if (!RestoreBuffer())
		return false;

    // Lock the buffer down
	if( FAILED( hr = m_lpDSBuff->Lock( 0, dsbc.dwBufferBytes, &pDSLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0L ) ) )
        return false;

	pWaveFile->ResetFile();

    if( FAILED( hr = pWaveFile->Read( (uint8_t*) pDSLockedBuffer, dwDSLockedBufferSize, &dwWavDataRead ) ) )
		return false;

    if( dwWavDataRead == 0 )
    {
        // Wav is blank, so just fill with silence
        FillMemory( (uint8_t*) pDSLockedBuffer, dwDSLockedBufferSize, (uint8_t)(pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else if( dwWavDataRead < dwDSLockedBufferSize )
    {
       // just fill in silence 
        FillMemory( (uint8_t*) pDSLockedBuffer + dwWavDataRead, 
                    dwDSLockedBufferSize - dwWavDataRead, 
                    (uint8_t)(pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
    }

    // Unlock the buffer, we don't need it anymore.
    m_lpDSBuff->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return true;
}

bool CN3SndObj::RestoreBuffer()
{
    if( NULL == m_lpDSBuff ) return false;

    HRESULT hr;
    DWORD dwStatus;
    if( FAILED( hr = m_lpDSBuff->GetStatus( &dwStatus ) ) )
        return false;

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
		hr = m_lpDSBuff->Restore();
		while (FAILED(hr))
        {
			if (hr == DSERR_BUFFERLOST)
				Sleep(10);

			hr = m_lpDSBuff->Restore();
        }
    }

	return true;
}

//
//	SetVolume...
//	range : [0,100]
//
void CN3SndObj::SetVolume(int Vol)
{
	if(NULL == m_lpDSBuff) return;
	if(m_lpDS3DBuff) return; // 3D Sound 일때는 소리 조절이 안된다..!!!

	m_iVol = Vol;
	if(Vol==0)
	{
		m_lpDSBuff->SetVolume(-10000);
		return;
	}

	float fVol = (float)(Vol) / 100.0f;
	long dwVol = (long)(log10(fVol) * 3000);	//데시벨 관련 소리조절식..
	m_lpDSBuff->SetVolume(dwVol);
}


//
//
//
bool CN3SndObj::IsPlaying()
{
	if(NULL == m_lpDSBuff) return false;
	
	DWORD dwStatus = 0;
	m_lpDSBuff->GetStatus( &dwStatus );
	if(dwStatus & DSBSTATUS_PLAYING) return true;

	if(m_ePlayState != SNDSTATE_STOP) return true;
	return false;
}


//
//
//
void CN3SndObj::Tick()
{
	if(NULL == m_lpDSBuff || m_ePlayState == SNDSTATE_STOP) return;

	m_fTmpSecPerFrm += CN3Base::s_fSecPerFrm;
	
	if(m_ePlayState==SNDSTATE_DELAY && m_fTmpSecPerFrm >= m_fStartDelayTime)
	{
		m_fTmpSecPerFrm = 0;
		m_ePlayState = SNDSTATE_FADEIN;
		RealPlay();
	}

	if(m_ePlayState==SNDSTATE_FADEIN) 
	{
		if(m_fTmpSecPerFrm >= m_fFadeInTime)
		{
			m_fTmpSecPerFrm = 0;
			m_ePlayState = SNDSTATE_PLAY;
			this->SetVolume(m_iMaxVolume);
		}
		else
		{
			int vol = 0;
			if(m_fFadeInTime>0.0f) vol = (int)((m_fTmpSecPerFrm/m_fFadeInTime)*(float)m_iMaxVolume);
			SetVolume(vol);
		}
	}

	if(m_ePlayState==SNDSTATE_PLAY)
	{
		if(!m_bIsLoop) m_ePlayState = SNDSTATE_STOP;		
	}
	if(m_ePlayState==SNDSTATE_FADEOUT)
	{
		if(m_fTmpSecPerFrm >= m_fFadeOutTime)
		{
			m_fTmpSecPerFrm = 0;
			m_ePlayState = SNDSTATE_STOP;
			this->SetVolume(0);
			HRESULT hr = m_lpDSBuff->Stop();
		}
		else
		{
			//볼륨 점점 작게....
			int vol = 0;
			if(m_fFadeOutTime>0.0f)  vol = (int)(((m_fFadeOutTime - m_fTmpSecPerFrm)/m_fFadeOutTime)*(float)m_iMaxVolume);
			SetVolume(vol);
		}
	}
}


//
//
//
void CN3SndObj::Play(const D3DVECTOR* pvPos, float delay, float fFadeInTime, bool bImmediately)
{
	this->SetPos(pvPos);
	if(bImmediately) this->Stop();

	m_fFadeInTime = fFadeInTime;
	m_fFadeOutTime = 0;
	m_fStartDelayTime = delay;
	m_fTmpSecPerFrm = 0;
	m_ePlayState = SNDSTATE_DELAY;

	if(m_lpDS3DBuff) // 3D 사운드일때에는 FadeIn 등이 필요 없구.. 볼륨이 먹지 않기 때문에 리턴..
	{
		m_ePlayState = SNDSTATE_PLAY;
		if(m_lpDSBuff)
		{
			if(m_bIsLoop) m_lpDSBuff->Play(0,0,DSBPLAY_LOOPING);
			else m_lpDSBuff->Play( 0, 0, 0 );
		}
	}
}


//
//
//
void CN3SndObj::RealPlay()
{
    if(NULL == m_lpDSBuff) return;

	DWORD dwStatus = 0;
	m_lpDSBuff->GetStatus( &dwStatus );
	if((dwStatus & DSBSTATUS_PLAYING)==DSBSTATUS_PLAYING) return;

	HRESULT hr;
	hr = m_lpDSBuff->SetCurrentPosition(0);

	if(m_bIsLoop) m_lpDSBuff->Play(0,0,DSBPLAY_LOOPING);
	else hr |= m_lpDSBuff->Play( 0, 0, 0 );

	return; 
}


//
//
//
void CN3SndObj::Stop(float fFadeOutTime)
{
	if( NULL == m_lpDSBuff ) return;
	if( m_ePlayState == SNDSTATE_FADEOUT || m_ePlayState == SNDSTATE_STOP) return; 

	if(fFadeOutTime==0.0f)
	{
		m_fTmpSecPerFrm = 0;
		m_fFadeOutTime = fFadeOutTime;
		m_ePlayState = SNDSTATE_STOP;
		HRESULT hr = m_lpDSBuff->Stop();
		return;
	}

	m_fTmpSecPerFrm = 0;
	m_fFadeOutTime = fFadeOutTime;
	m_ePlayState = SNDSTATE_FADEOUT;
	return;
}


void CN3SndObj::SetPos(const D3DVECTOR* pvPos)
{
    if( m_lpDS3DBuff && pvPos ) 
		HRESULT hr = m_lpDS3DBuff->SetPosition(pvPos->x, pvPos->y, pvPos->z, DS3D_IMMEDIATE );
}


//
//
//
void CN3SndObj::SetMaxDistance(D3DVALUE max)
{
	if( m_lpDS3DBuff )	m_lpDS3DBuff->SetMaxDistance(max, DS3D_IMMEDIATE);
}


//
//
//
void CN3SndObj::SetMinDistance(D3DVALUE min)
{
	if( m_lpDS3DBuff )	m_lpDS3DBuff->SetMinDistance(min, DS3D_IMMEDIATE);
}


//
//
//
void CN3SndObj::SetConeOutSizeVolume(int32_t vol)
{
	if( m_lpDS3DBuff )	m_lpDS3DBuff->SetConeOutsideVolume(vol, DS3D_IMMEDIATE);
}


//
//
//
void CN3SndObj::SetConeOrientation(D3DVECTOR* pDir)
{
	if( m_lpDS3DBuff )	m_lpDS3DBuff->SetConeOrientation(pDir->x, pDir->y, pDir->z, DS3D_IMMEDIATE);
}



//
// static functions ....
//

void CN3SndObj::SetDopplerFactor(D3DVALUE factor)
{
	if(NULL == s_lpDSListener) return;
	s_lpDSListener->SetDopplerFactor(factor, DS3D_DEFERRED);
	s_bNeedDeferredTick = true;	// 3D Listener CommitDeferredSetting
}

void CN3SndObj::SetListenerPos(const D3DVECTOR* pVPos, bool IsDeferred)
{
	if(NULL == s_lpDSListener || NULL == pVPos) return;
	DWORD dwParam = (IsDeferred) ? DS3D_DEFERRED : DS3D_IMMEDIATE;
	s_lpDSListener->SetPosition(pVPos->x, pVPos->y, pVPos->z, dwParam);
	s_bNeedDeferredTick = true;	// 3D Listener CommitDeferredSetting
}


//
//
//
void CN3SndObj::SetListenerOrientation(const D3DVECTOR* pVAt, const D3DVECTOR* pVUp, bool IsDeferred)
{
	if(NULL == s_lpDSListener || NULL == pVAt || NULL == pVUp) return;
	DWORD dwParam = (IsDeferred) ? DS3D_DEFERRED : DS3D_IMMEDIATE;
	s_lpDSListener->SetOrientation(pVAt->x, pVAt->y, pVAt->z, pVUp->x, pVUp->y, pVUp->z, dwParam);
	s_bNeedDeferredTick = true;	// 3D Listener CommitDeferredSetting
}
