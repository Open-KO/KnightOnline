// N3SndObj.cpp: implementation of the CN3SndObj class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3SndObj.h"
#include "N3Base.h"
#include "AudioAsset.h"
#include "AudioHandle.h"
#include "WaveFormat.h"
#include "al_wrapper.h"

#include <AL/alc.h>
#include <cassert> // assert()

#ifdef _N3GAME
#include "LogWriter.h"
#endif

bool al_check_error_impl(const char* file, int line)
{
	ALenum error = alGetError();
	if (error == AL_NO_ERROR)
		return false;

#ifdef _N3GAME
	CLogWriter::Write("{}({}) - OpenAL error occurred: {:X}", file, line, error);
#endif
	return true;
}

CN3SndObj::CN3SndObj()
{
	_type          = SNDTYPE_UNKNOWN;
	_isStarted     = false;

	_soundSettings = std::make_shared<SoundSettings>();
}

CN3SndObj::~CN3SndObj()
{
	Release();
}

void CN3SndObj::Init()
{
	Release();

	_isStarted                = false;
	_soundSettings->IsLooping = false;
}

void CN3SndObj::Release()
{
	ReleaseHandle();

	if (_audioAsset != nullptr)
	{
		CN3Base::s_SndMgr.RemoveAudioAsset(_audioAsset.get());
		_audioAsset.reset();
	}
}

bool CN3SndObj::Create(const std::string& szFN, e_SndType eType)
{
	Init();

	if (eType == SNDTYPE_2D || eType == SNDTYPE_3D)
	{
		_audioAsset = CN3Base::s_SndMgr.LoadBufferedAudioAsset(szFN);

		// Fallback to streaming if unsupported
		// Buffering is only supported for small WAV (PCM) files.
		// Streaming supports MP3.
		if (_audioAsset == nullptr)
			_audioAsset = CN3Base::s_SndMgr.LoadStreamedAudioAsset(szFN);
	}
	else if (eType == SNDTYPE_STREAM)
	{
		_audioAsset = CN3Base::s_SndMgr.LoadStreamedAudioAsset(szFN);
	}
	else
	{
		return false;
	}

	if (_audioAsset == nullptr)
		return false;

	_type = eType;
	return true;
}

//	SetVolume
//	range : [0.0f, 1.0f]
void CN3SndObj::SetVolume(float currentVolume)
{
	currentVolume = std::clamp(currentVolume, 0.0f, 1.0f);

	if (_handle != nullptr)
	{
		CN3Base::s_SndMgr.QueueCallback(_handle,
			[currentVolume](AudioHandle* handle)
			{
				handle->Settings->CurrentGain = currentVolume;

				alSourcef(handle->SourceId, AL_GAIN, handle->Settings->CurrentGain);
				AL_CHECK_ERROR();
			});
	}
	else
	{
		_soundSettings->CurrentGain = currentVolume;
	}
}

void CN3SndObj::SetMaxVolume(float maxVolume)
{
	maxVolume               = std::clamp(maxVolume, 0.0f, 1.0f);

	_soundSettings->MaxGain = maxVolume;

	if (_handle != nullptr)
	{
		CN3Base::s_SndMgr.QueueCallback(_handle,
			[maxVolume](AudioHandle* handle)
			{
				if (handle->Settings->CurrentGain <= maxVolume)
					return;

				handle->Settings->CurrentGain = maxVolume;

				alSourcef(handle->SourceId, AL_GAIN, handle->Settings->CurrentGain);
				AL_CHECK_ERROR();
			});
	}
}

const std::string& CN3SndObj::FileName() const
{
	if (_audioAsset != nullptr)
		return _audioAsset->Filename;

	static std::string empty;
	return empty;
}

bool CN3SndObj::IsFinished() const
{
	if (_handle == nullptr)
		return false;

	return _handle->FinishedPlaying;
}

void CN3SndObj::Tick()
{
	if (_handle == nullptr)
	{
		_isStarted = false;
		return;
	}

	// Handle is no longer managed by the audio thread.
	// We should consider it stopped and release our handle for future requests.
	if (!_handle->IsManaged.load())
	{
		_isStarted = false;
		_handle.reset();
	}
}

void CN3SndObj::Play(const __Vector3* pvPos, float delay, float fFadeInTime)
{
	if (_audioAsset == nullptr)
		return;

	if (_handle == nullptr)
	{
		if (_audioAsset->Type == AUDIO_ASSET_BUFFERED)
			_handle = BufferedAudioHandle::Create(_audioAsset);
		else if (_audioAsset->Type == AUDIO_ASSET_STREAMED)
			_handle = StreamedAudioHandle::Create(_audioAsset);

		if (_handle == nullptr)
			return;
	}

	CN3Base::s_SndMgr.Add(_handle);

	_handle->Settings = _soundSettings;

	// OpenAL-side looping needs to be disabled on streamed handles.
	// They implement their own looping.
	ALint isLooping   = 0;
	if (_handle->HandleType != AUDIO_HANDLE_STREAMED)
		isLooping = static_cast<ALint>(_soundSettings->IsLooping);

	bool playImmediately = false;
	if (delay == 0.0f && fFadeInTime == 0.0f)
		playImmediately = true;

	// Perform setup -- this is triggered after insertion, so the handle is added to the thread at this point
	// and otherwise setup, ready to play.
	if (GetType() == SNDTYPE_2D)
	{
		assert(_audioAsset->Type == AUDIO_ASSET_BUFFERED);

		if (_audioAsset->Type != AUDIO_ASSET_BUFFERED)
			return;

		auto audioAsset = static_pointer_cast<BufferedAudioAsset>(_audioAsset);
		if (audioAsset == nullptr)
			return;

		CN3Base::s_SndMgr.QueueCallback(_handle,
			[fFadeInTime, delay, playImmediately, audioAsset, isLooping](AudioHandle* handle)
			{
				handle->FadeInTime     = fFadeInTime;
				handle->FadeOutTime    = 0.0f;
				handle->StartDelayTime = delay;
				handle->Timer          = 0.0f;
				handle->State          = SNDSTATE_DELAY;

				if (playImmediately)
					handle->Settings->CurrentGain = handle->Settings->MaxGain;

				if (handle->Asset->Type == AUDIO_ASSET_BUFFERED)
				{
					alSourcei(handle->SourceId, AL_BUFFER, audioAsset->BufferId);
					if (AL_CHECK_ERROR())
						return;
				}

				alSourcei(handle->SourceId, AL_SOURCE_RELATIVE, AL_TRUE);
				AL_CHECK_ERROR();

				alSource3f(handle->SourceId, AL_POSITION, 0.0f, 0.0f, 0.0f);
				AL_CHECK_ERROR();

				alSourcef(handle->SourceId, AL_ROLLOFF_FACTOR, 0.0f);
				AL_CHECK_ERROR();

				alSourcei(handle->SourceId, AL_LOOPING, isLooping);
				AL_CHECK_ERROR();

				if (playImmediately)
				{
					handle->State = SNDSTATE_PLAY;

					alSourcef(handle->SourceId, AL_GAIN, handle->Settings->CurrentGain);
					AL_CHECK_ERROR();

					alSourcePlay(handle->SourceId);
					AL_CHECK_ERROR();
				}
			});
	}
	else if (GetType() == SNDTYPE_3D)
	{
		assert(_audioAsset->Type == AUDIO_ASSET_BUFFERED);

		if (_audioAsset->Type != AUDIO_ASSET_BUFFERED)
			return;

		auto audioAsset = static_pointer_cast<BufferedAudioAsset>(_audioAsset);
		if (audioAsset == nullptr)
			return;

		bool hasPosition = false;

		__Vector3 position;
		if (pvPos != nullptr)
		{
			position    = *pvPos;
			hasPosition = true;
		}

		CN3Base::s_SndMgr.QueueCallback(_handle,
			[fFadeInTime, delay, playImmediately, audioAsset, hasPosition, position, isLooping](
				AudioHandle* handle)
			{
				handle->FadeInTime     = fFadeInTime;
				handle->FadeOutTime    = 0.0f;
				handle->StartDelayTime = delay;
				handle->Timer          = 0.0f;
				handle->State          = SNDSTATE_DELAY;

				if (playImmediately)
					handle->Settings->CurrentGain = handle->Settings->MaxGain;

				if (handle->Asset->Type == AUDIO_ASSET_BUFFERED)
				{
					alSourcei(handle->SourceId, AL_BUFFER, audioAsset->BufferId);
					if (AL_CHECK_ERROR())
						return;
				}

				alSourcei(handle->SourceId, AL_SOURCE_RELATIVE, AL_FALSE);
				AL_CHECK_ERROR();

				if (hasPosition)
					alSource3f(handle->SourceId, AL_POSITION, position.x, position.y, position.z);
				else
					alSource3f(handle->SourceId, AL_POSITION, 0.0f, 0.0f, 0.0f);
				AL_CHECK_ERROR();

				alSourcef(handle->SourceId, AL_ROLLOFF_FACTOR, 0.5f);
				AL_CHECK_ERROR();

				alSourcei(handle->SourceId, AL_LOOPING, isLooping);
				AL_CHECK_ERROR();

				if (playImmediately)
				{
					handle->State = SNDSTATE_PLAY;

					alSourcef(handle->SourceId, AL_GAIN, handle->Settings->CurrentGain);
					AL_CHECK_ERROR();

					alSourcePlay(handle->SourceId);
					AL_CHECK_ERROR();
				}
			});
	}
	else if (GetType() == SNDTYPE_STREAM)
	{
		CN3Base::s_SndMgr.QueueCallback(_handle,
			[fFadeInTime, delay, isLooping, playImmediately](AudioHandle* handle)
			{
				handle->FadeInTime     = fFadeInTime;
				handle->FadeOutTime    = 0.0f;
				handle->StartDelayTime = delay;
				handle->Timer          = 0.0f;
				handle->State          = SNDSTATE_DELAY;

				alSourcei(handle->SourceId, AL_SOURCE_RELATIVE, AL_TRUE);
				AL_CHECK_ERROR();

				alSource3f(handle->SourceId, AL_POSITION, 0.0f, 0.0f, 0.0f);
				AL_CHECK_ERROR();

				alSourcef(handle->SourceId, AL_ROLLOFF_FACTOR, 0.0f);
				AL_CHECK_ERROR();

				alSourcei(handle->SourceId, AL_LOOPING, isLooping);
				AL_CHECK_ERROR();

				if (playImmediately)
				{
					handle->State                 = SNDSTATE_PLAY;
					handle->Settings->CurrentGain = handle->Settings->MaxGain;

					alSourcef(handle->SourceId, AL_GAIN, handle->Settings->CurrentGain);
					AL_CHECK_ERROR();

					alSourcePlay(handle->SourceId);
					AL_CHECK_ERROR();
				}
			});
	}
	else
	{
		return;
	}

	_isStarted = true;
}

void CN3SndObj::Stop(float fFadeOutTime)
{
	_isStarted = false;

	if (_handle == nullptr)
		return;

	// If we intend on stopping it immediately, we can just remove the handle directly,
	// removing it from the audio thread.
	if (fFadeOutTime == 0.0f)
	{
		ReleaseHandle();
		return;
	}

	// It won't be stopped immediately, so queue the stop request so it can process with the delay.
	CN3Base::s_SndMgr.QueueCallback(_handle,
		[fFadeOutTime](AudioHandle* handle)
		{
			if (handle->State == SNDSTATE_FADEOUT || handle->State == SNDSTATE_STOP)
				return;

			handle->Timer       = 0.0f;
			handle->FadeOutTime = fFadeOutTime;
			handle->State       = SNDSTATE_FADEOUT;
		});
}

void CN3SndObj::ReleaseHandle()
{
	_isStarted = false;

	if (_handle == nullptr)
		return;

	CN3Base::s_SndMgr.Remove(_handle);
	_handle.reset();
}

void CN3SndObj::SetPos(const __Vector3& vPos)
{
	if (_handle == nullptr || GetType() != SNDTYPE_3D)
		return;

	// NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
	__Vector3 posCopy = vPos;

	CN3Base::s_SndMgr.QueueCallback(_handle,
		[posCopy](AudioHandle* handle)
		{
			alSource3f(handle->SourceId, AL_POSITION, posCopy.x, posCopy.y, posCopy.z);
			AL_CLEAR_ERROR_STATE();
		});
}

void CN3SndObj::Looping(bool loop)
{
	_soundSettings->IsLooping = loop;

	if (_handle == nullptr)
		return;

	// Streams need to manually handle their looping.
	if (_handle->HandleType == AUDIO_HANDLE_STREAMED)
		return;

	CN3Base::s_SndMgr.QueueCallback(_handle,
		[](AudioHandle* handle)
		{
			alSourcei(
				handle->SourceId, AL_LOOPING, static_cast<ALint>(handle->Settings->IsLooping));
			AL_CLEAR_ERROR_STATE();
		});
}

void CN3SndObj::SetListenerPos(const __Vector3& vPos)
{
	alListener3f(AL_POSITION, vPos.x, vPos.y, vPos.z);
	AL_CHECK_ERROR();
}

void CN3SndObj::SetListenerOrientation(const __Vector3& vAt, const __Vector3& vUp)
{
	ALfloat fv[6] = { vAt.x, vAt.y, vAt.z, vUp.x, vUp.y, vUp.z };

	alListenerfv(AL_ORIENTATION, fv);
	AL_CHECK_ERROR();
}
