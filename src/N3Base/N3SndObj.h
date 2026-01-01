#if !defined(AFX_N3SndObj_H__64BCBFD5_FD77_438D_9BF4_DC9B7C5D5BB9__INCLUDED_)
#define AFX_N3SndObj_H__64BCBFD5_FD77_438D_9BF4_DC9B7C5D5BB9__INCLUDED_

#pragma once

#include "N3SndDef.h"

#include <memory> // std::shared_ptr<>
#include <string> // std::string

class AudioAsset;
class AudioHandle;

/// \brief Represents a single sound object, either buffered or streamed.
///
/// CN3SndObj manages a single sound instance, including playback, looping, 
/// volume, 3D position, and lifetime of the underlying audio asset. It interacts
/// with the sound manager to queue OpenAL operations on a dedicated audio thread.
class CN3SndObj
{
protected:
	/// Type of sound (2D, 3D, Stream, Unknown)
	e_SndType						_type;

	/// Indicates whether the sound has been started
	bool							_isStarted;

	/// Settings associated with this sound object, including current and maximum volume.
	/// These settings persist with the CN3SndObj instance and are shared with any AudioHandle created for playback.
	std::shared_ptr<SoundSettings>	_soundSettings;

	/// Cached audio asset used by this sound
	std::shared_ptr<AudioAsset>		_audioAsset;

	/// Handle managed by the audio thread for playback
	std::shared_ptr<AudioHandle>	_handle;

public:
	/// \brief Returns the type of this sound object.
	e_SndType GetType() const
	{
		return _type;
	}

	/// \brief Returns true if the sound has been started.
	/// \note A started sound may not necessarily be currently playing (e.g., it could be in delay).
	bool IsStarted() const
	{
		return _isStarted;
	}

	/// \brief Returns true if the sound is set to loop.
	bool IsLooping() const
	{
		return _soundSettings->IsLooping;
	}

	/// \brief Returns the current volume of the sound in range [0.0f, 1.0f].
	float GetVolume() const
	{
		return _soundSettings->CurrentGain;
	}

	/// \brief Returns the maximum volume of the sound in range [0.0f, 1.0f].
	float GetMaxVolume() const
	{
		return _soundSettings->MaxGain;
	}

public:
	/// \brief Sets the listener's global position for 3D sound.
	/// \param vPos Listener position in world coordinates.
	static void SetListenerPos(const __Vector3& vPos);

	/// \brief Sets the listener's orientation for 3D sound.
	/// \param vAt Direction the listener is facing.
	/// \param vUp Up vector for the listener.
	static void SetListenerOrientation(const __Vector3& vAt, const __Vector3& vUp);

protected:
	/// \brief Releases the audio handle and removes it from the audio thread.
	void ReleaseHandle();

public:
	/// \brief Returns the filename of the underlying audio asset.
	const std::string& FileName() const;

	/// \brief Returns true if playback has finished for this sound object.
	bool IsFinished() const;

	/// \brief Sets the current playback volume.
	/// \param volume Volume value in range [0.0f, 1.0f].
	void SetVolume(float volume);

	/// \brief Sets the maximum volume allowed for this sound object.
	/// \param maxVolume Maximum volume in range [0.0f, 1.0f].
	void SetMaxVolume(float maxVolume);

	/// \brief Initializes the sound object, releasing any previous audio.
	void Init();

	/// \brief Releases the sound object and its associated asset.
	///
	/// This releases the handle from the audio thread and also decrements
	/// the reference count of the underlying cached audio asset.
	void Release();

	/// \brief Creates a sound object from a file and type.
	/// \param szFN Path to the audio file.
	/// \param eType Type of sound (2D, 3D, Stream).
	/// \return true if creation succeeded, false otherwise.
	bool Create(const std::string& szFN, e_SndType eType);

	/// \brief Starts playback of the sound.
	///
	/// \param pvPos Optional 3D position for 3D sounds.
	/// \param delay Time in seconds to delay the start.
	/// \param fFadeInTime Time in seconds to fade in the sound.
	void Play(const __Vector3* pvPos = nullptr, float delay = 0.0f, float fFadeInTime = 0.0f);

	/// \brief Stops playback of the sound.
	///
	/// \param fFadeOutTime Time in seconds to fade out before stopping. 
	/// If zero, the sound stops immediately.
	void Stop(float fFadeOutTime = 0.0f);

	/// \brief Updates the sound object's state.
	///
	/// This primarily monitors the handle and releases it if it is no
	/// longer managed by the audio thread.
	void Tick();

	/// \brief Sets the 3D position of the sound.
	/// \param vPos Position in world coordinates.
	void SetPos(const __Vector3 vPos);

	/// \brief Sets whether the sound should loop.
	/// \param loop true to loop, false to play once.
	void Looping(bool loop);

	/// \brief Constructor. Initializes members to default values.
	CN3SndObj();

	/// \brief Destructor. Releases its handle and cached asset.
	virtual ~CN3SndObj();
};

#endif // !defined(AFX_N3SndObj_H__64BCBFD5_FD77_438D_9BF4_DC9B7C5D5BB9__INCLUDED_)
