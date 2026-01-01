////////////////////////////////////////////////////////////////////////////////////////
//
//	N3SndDef.h
//	- 이것저것 Sound에 관련된 자료형정의, 상수정의...
//
//	By Donghoon..
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __N3SNDDEF_H__
#define __N3SNDDEF_H__

#pragma once

#include <string> // std::string

struct __TABLE_SOUND // Sound 리소스 레코드...
{
	uint32_t	dwID;		// 고유 ID
	std::string	szFN;		// wave file name
	int			iType;		// 사운드 타입...
	int			iNumInst;	// 최대 사용할 수 있는 인스턴스의 갯수..
};

/// \struct SoundSettings
/// \brief Represents per-sound playback settings, such as volume.
///
/// It is shared between the sound instance and the audio handle, allowing,
/// for example, persistent volume settings across subsequent plays from the
/// same sound instance across different handles.
struct SoundSettings
{
	/// Indicates whether playback should loop.
	bool IsLooping		= false;

	/// Current gain applied to the sound, in the range [0.0, 1.0].
	float CurrentGain	= 0.0f;

	/// Maximum gain that can be applied to the sound, in the range [0.0, 1.0].
	/// Used as the target gain for fades and playback.
	float MaxGain		= 1.0f;
};

/// \enum e_SndType
/// \brief Type of sound, used to determine playback behavior.
enum e_SndType
{
	/// Standard 2D sound, unaffected by 3D spatialization.
	SNDTYPE_2D = 0,

	/// 3D sound that is spatialized based on position and listener.
	SNDTYPE_3D,

	/// Streamed sound, decoded on the fly.
	SNDTYPE_STREAM,

	/// Unknown or uninitialized sound type.
	SNDTYPE_UNKNOWN
};

/// \enum e_SndState
/// \brief Current playback state of a sound.
///
/// These states are used by the audio system to manage the lifecycle
/// of a sound handle, including fades, delays, and stopping.
enum e_SndState
{
	/// Initial state, before playback starts.
	SNDSTATE_INITIAL = 0,

	/// Playback has stopped. Once in this state, the handle is no longer
	/// considered active and will be removed from the \ref AudioThread.
	SNDSTATE_STOP,

	/// Playback is delayed, waiting for the start time to elapse.
	SNDSTATE_DELAY,

	/// Sound is fading in.
	SNDSTATE_FADEIN,

	/// Sound is actively playing at full volume.
	SNDSTATE_PLAY,

	/// Sound is fading out. Once the fade-out completes, playback
	/// will automatically stop and the handle state will transition to SNDSTATE_STOP.
	SNDSTATE_FADEOUT
};

#endif	//end of #ifndef __N3SNDDEF_H__
