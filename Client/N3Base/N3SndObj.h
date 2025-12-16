// N3SndObj.h: interface for the CN3SndObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3SndObj_H__64BCBFD5_FD77_438D_9BF4_DC9B7C5D5BB9__INCLUDED_)
#define AFX_N3SndObj_H__64BCBFD5_FD77_438D_9BF4_DC9B7C5D5BB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "N3SndDef.h"
#include <stdio.h>
#include <string>

class CN3SndObj
{
protected:
	static LPDIRECTSOUND			s_lpDS;
	static LPDIRECTSOUND3DLISTENER	s_lpDSListener;			// 3D listener object
	static DS3DLISTENER				s_dsListenerParams;		// Listener properties
	static bool						s_bNeedDeferredTick;	// 3D Listener CommitDeferredSetting

	LPDIRECTSOUNDBUFFER		m_lpDSBuff;
	LPDIRECTSOUND3DBUFFER	m_lpDS3DBuff;			// 3D sound buffer
	bool					m_bIsLoop;
	int						m_iVol;
	int						m_iMaxVolume;
	e_SndState				m_ePlayState;
	
	float					m_fFadeInTime;
	float					m_fFadeOutTime;
	float					m_fStartDelayTime;
	float					m_fTmpSecPerFrm;

public:
	std::string				m_szFileName;	//파일이름...(ID 대용).
	

protected:
	virtual void RealPlay();
	bool	FillBufferWithSound(class CWaveFile* pWaveFile);
	bool	RestoreBuffer();

		
public:
	static bool StaticInit( HWND hWnd, DWORD dwCoopLevel = DSSCL_PRIORITY, DWORD dwPrimaryChannels = 2, DWORD dwPrimaryFreq = 22050, DWORD dwPrimaryBitRate = 16 );
	static void StaticRelease();
	static void StaticTick();
	static void SetDopplerFactor(D3DVALUE factor);
	static void SetListenerPos(const __Vector3* pVPos, bool IsDeferred = true);
	static void SetListenerOrientation(const __Vector3* pVAt, const __Vector3* pVUp, bool IsDeferred = true);

	LPDIRECTSOUNDBUFFER		GetDSBuffer() { return m_lpDSBuff; }
	LPDIRECTSOUND3DBUFFER	GetDS3DBuffer() { return m_lpDS3DBuff; }

	bool		IsPlaying();

	void	Looping(bool loop) { m_bIsLoop = loop; }
	bool	IsLoop() { return m_bIsLoop; }
	
	int		GetVolume() { return m_iVol; }
	void	SetVolume(int Vol);	// range : [0,100]

	void	SetMaxVolume(int vol) { m_iMaxVolume = vol; }
	int		GetMaxVolume() { return m_iMaxVolume; }

	void	Init();
	void	Release(); // 참조 카운트를 리턴 해준다.. 사운드 매니저에서는 이 참조 카운트를 보고 맵에서 지운다..
	virtual bool Create(const std::string& szFN, e_SndType eType);
	bool	Duplicate(CN3SndObj* pSrc, e_SndType eType, __Vector3* pPos = nullptr);

	void	Play(const __Vector3* pvPos = nullptr, float delay = 0.0f, float fFadeInTime = 0.0f, bool bImmediately = true);
	void	Stop(float fFadeOutTime = 0.0f);
	void	Tick();

	void	SetConeOrientation(__Vector3* pDir);
	void	SetConeOutSizeVolume(int32_t vol);
	void	SetMaxDistance(D3DVALUE max);
	void	SetMinDistance(D3DVALUE min);	
	void	SetPos(const __Vector3* pvPos);

	CN3SndObj();
	virtual ~CN3SndObj();
};

#endif // !defined(AFX_N3SndObj_H__64BCBFD5_FD77_438D_9BF4_DC9B7C5D5BB9__INCLUDED_)
