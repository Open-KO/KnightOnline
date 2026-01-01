#if !defined(AFX_N3SNDMGR_H__9CB531B0_4FEB_4360_8141_D0BF61347BD7__INCLUDED_)
#define AFX_N3SNDMGR_H__9CB531B0_4FEB_4360_8141_D0BF61347BD7__INCLUDED_

#pragma once

#include "N3SndDef.h"
#include "N3TableBase.h"

#include <list>				// std::list<>
#include <memory>			// std::unique_ptr<>
#include <mutex>			// std::mutex
#include <string>			// std::string
#include <unordered_map>	// std::unordered_map<>
#include <unordered_set>	// std::unordered_set<>

#include "AudioThread.h"

struct ALCcontext;
struct ALCdevice;
class AudioAsset;
class BufferedAudioAsset;
class CN3SndObj;
class StreamedAudioAsset;

/**
 * @class CN3SndMgr
 * @brief Central sound manager for the engine.
 *
 * Manages all sound playback, including 2D/3D sound objects, streamed audio,
 * and sound effects that play once. Handles source allocation from OpenAL,
 * threaded decoding for streamed sounds, and lifetime of sound assets.
 */
class CN3SndMgr
{
	friend class BufferedAudioHandle;
	friend class StreamedAudioHandle;
	friend class CN3SndObj;

protected:
	/// Table of sound source definitions loaded from "sound.tbl".
	CN3TableBase<__TABLE_SOUND>			m_Tbl_Source;

	/// Indicates whether the sound system is currently enabled.
	bool								m_bSndEnable;

	/// List of streaming sound objects (typically background music as they're long tracks).
	std::list<CN3SndObj*>				m_SndObjStreams;

	/// List of regular sound objects (effects, 3D sounds, etc.).
	std::list<CN3SndObj*>				m_SndObjs;

	/// List of sounds that play once and are automatically released afterward.
	std::list<CN3SndObj*>				m_SndObjs_PlayOnceAndRelease;

	/// OpenAL device handle.
	ALCdevice*	_alcDevice;

	/// OpenAL context handle.
	ALCcontext*	_alcContext;

	/// Pool of unassigned audio source IDs for buffered sounds.
	std::list<uint32_t>				_unassignedSourceIds;

	/// Pool of unassigned audio source IDs for streamed sounds.
	std::list<uint32_t>				_unassignedStreamSourceIds;

	/// Set of assigned source IDs for buffered sounds.
	std::unordered_set<uint32_t>	_assignedSourceIds;

	/// Set of assigned source IDs for streamed sounds.
	std::unordered_set<uint32_t>	_assignedStreamSourceIds;

	/// Mutex protecting access to source ID pools.
	std::mutex						_sourceIdMutex;

	/// Map of loaded buffered audio assets by filename.
	std::unordered_map<std::string, std::shared_ptr<BufferedAudioAsset>> _bufferedAudioAssetByFilenameMap;

	/// Mutex protecting buffered audio asset map.
	std::mutex _bufferedAudioAssetByFilenameMutex;

	/// Map of loaded streamed audio assets by filename.
	std::unordered_map<std::string, std::shared_ptr<StreamedAudioAsset>> _streamedAudioAssetByFilenameMap;

	/// Mutex protecting streamed audio asset map.
	std::mutex _streamedAudioAssetByFilenameMutex;

	/// Audio processing thread handling playback (and indirectly, decoding) of streamed sounds.
	std::unique_ptr<AudioThread> _thread;

public:
	/// \brief Enables or disables the sound system.
	/// \param enable true to enable, false to disable.
	void SetEnable(bool enable)
	{
		m_bSndEnable = enable;
	}

	/// \brief Releases a managed sound object.
	/// 
	/// Frees the sound object and removes it from internal management. 
	/// The provided pointer is set to \c nullptr after the object is freed.
	/// \param ppObj Pointer to the sound object pointer to release.
	void		ReleaseObj(CN3SndObj** ppObj);
	
	/// \brief Releases a managed streamed sound object.
	/// 
	/// Frees the streamed sound object and removes it from internal management. 
	/// The provided pointer is set to \c nullptr after the object is freed.
	/// \param ppObj Pointer to the streamed sound object pointer to release.
	void		ReleaseStreamObj(CN3SndObj** ppObj);
	
	/// \brief Plays a sound once and automatically release it afterward.
	/// \param iSndID Sound table ID.
	/// \param pPos Optional 3D position.
	/// \returns true if playback was started successfully, false otherwise.
	bool		PlayOnceAndRelease(int iSndID, const __Vector3* pPos = nullptr);

	/// \brief Initializes the sound system (loads sound.tbl and sets up OpenAL).
	void		Init();

	/// \brief Initializes the OpenAL device, context, and source pools.
	/// \returns true if initialization succeeded, false otherwise.
	bool		InitOpenAL();

	/// \brief Releases all managed sound objects and stops audio processing.
	void		Release();

	/// \brief Releases the OpenAL context and all generated sources.
	void		ReleaseOpenAL();

	/// \brief Processes all managed sound objects and cleans up one-shot ("play once and release") sounds.
	///
	/// This method iterates over all managed sound objects and performs minimal per-frame
	/// updates via \c CN3SndObj::Tick(), which primarily checks whether the underlying
	/// audio handle is still managed by the audio thread and releases it if not.
	///
	/// For sounds played via \c PlayOnceAndRelease, this method will also check if the
	/// sound has finished playing, delete the sound object, and remove it from the internal list.
	///
	/// Note that most playback and audio state updates are now handled by \c AudioThread;
	/// \c CN3SndObj::Tick() no longer advances playback itself.
	void		Tick();	

	/// \brief Creates a regular (buffered) sound object by filename.
	/// \param szFN Path to the sound file.
	/// \param eType Sound type (2D, 3D, or stream). Defaults to SNDTYPE_3D.
	/// \return Pointer to the created CN3SndObj, or nullptr if creation failed.
	CN3SndObj*	CreateObj(const std::string& szFN, e_SndType eType = SNDTYPE_3D);

	/// \brief Create a regular (buffered) sound object by sound ID (in sound.tbl).
	/// \param iID Sound ID in sound.tbl.
	/// \param eType Sound type (2D, 3D, or stream). Defaults to SNDTYPE_3D.
	/// \return Pointer to the created CN3SndObj, or nullptr if creation failed.
	CN3SndObj*	CreateObj(int iID, e_SndType eType = SNDTYPE_3D);

	/// \brief Creates a streamed sound object by filename.
	/// \param szFN Path to the sound file.
	/// \return Pointer to the created streamed CN3SndObj, or nullptr if creation failed.
	CN3SndObj*	CreateStreamObj(const std::string& szFN);

	/// \brief Create a streamed sound object by sound ID (in sound.tbl).
	/// \param iID Sound ID in sound.tbl.
	/// \return Pointer to the created streamed CN3SndObj, or nullptr if creation failed.
	CN3SndObj*	CreateStreamObj(int iID);

	/// \brief Constructs a new CN3SndMgr instance.
	///
	/// Initializes internal state flags and OpenAL device/context pointers to null.
	/// The sound system is not enabled until \c Init() or \c InitOpenAL() is called.
	CN3SndMgr();

	/// \brief Destroys the CN3SndMgr instance.
	///
	/// Releases all managed sound objects, stops the audio thread, and cleans up
	/// the OpenAL context and any generated sources via \c Release().
	virtual ~CN3SndMgr();

protected:
	/// \brief Acquires a buffered sound source ID from the pool.
	/// 
	/// If a source ID is available, it is removed from the unassigned pool,
	/// added to the assigned set, and written to \p sourceId. If no sources are
	/// available, \p sourceId is set to \c INVALID_AUDIO_SOURCE_ID.
	/// 
	/// \param sourceId Pointer to store the acquired source ID.
	/// \returns true if a source ID was successfully acquired; false if none were available.
	bool PullBufferedSourceIdFromPool(uint32_t* sourceId);

	/// \brief Restores a buffered source ID back to the pool.
	/// 
	/// Removes the source ID from the assigned set and returns it to the unassigned
	/// pool. After restoring, \p sourceId is set to \c INVALID_AUDIO_SOURCE_ID.
	/// 
	/// \param sourceId Pointer to the source ID to restore.
	void RestoreBufferedSourceIdToPool(uint32_t* sourceId);

	/// \brief Acquires a streamed sound source ID from the pool.
	/// 
	/// If a streamed source ID is available, it is removed from the unassigned pool,
	/// added to the assigned set, and written to \p sourceId. If no sources are
	/// available, \p sourceId is set to \c INVALID_AUDIO_SOURCE_ID.
	/// 
	/// \param sourceId Pointer to store the acquired source ID.
	/// \returns true if a source ID was successfully acquired; false if none were available.
	bool PullStreamedSourceIdFromPool(uint32_t* sourceId);

	/// \brief Restores a streamed source ID back to the pool.
	/// 
	/// Removes the source ID from the assigned set and returns it to the unassigned
	/// pool. After restoring, \p sourceId is set to \c INVALID_AUDIO_SOURCE_ID.
	/// 
	/// \param sourceId Pointer to the source ID to restore.
	void RestoreStreamedSourceIdToPool(uint32_t* sourceId);

	/// \brief Loads a buffered audio asset by filename, or returns a cached asset if already loaded.
	/// 
	/// The asset is cached internally by the sound manager and is reference-counted. It will remain
	/// cached for as long as the manager considers it in use. For consistent lifetime management,
	/// a corresponding call to \c RemoveAudioAsset should be made when the asset is no longer needed.
	/// 
	/// \param filename Path to the audio file.
	/// \return A shared pointer to the loaded or cached \c BufferedAudioAsset, or \c nullptr if loading failed.
	std::shared_ptr<BufferedAudioAsset> LoadBufferedAudioAsset(const std::string& filename);

	/// \brief Loads a streamed audio asset by filename, or returns a cached asset if already loaded.
	/// 
	/// The asset is cached internally by the sound manager and is reference-counted. It will remain
	/// cached for as long as the manager considers it in use. For consistent lifetime management,
	/// a corresponding call to \c RemoveAudioAsset should be made when the asset is no longer needed.
	/// 
	/// \param filename Path to the audio file.
	/// \returns A shared pointer to the loaded or cached \c StreamedAudioAsset, or \c nullptr if loading failed.
	std::shared_ptr<StreamedAudioAsset> LoadStreamedAudioAsset(const std::string& filename);

	/// \brief Releases the manager's reference to an audio asset.
	/// 
	/// This decreases the manager's internal reference to the asset. The asset will remain cached
	/// and in memory as long as there are any outstanding references (e.g., via \c std::shared_ptr).
	/// Once the asset is no longer referenced anywhere, it will be removed from the internal cache.
	/// This should be paired with a previous call to \c LoadBufferedAudioAsset or
	/// \c LoadStreamedAudioAsset to ensure consistent lifetime management.
	/// 
	/// \param audioAsset Pointer to the audio asset to release.
	void RemoveAudioAsset(AudioAsset* audioAsset);

	/// \brief Queues an audio handle to be managed by the audio thread.
	/// \param handle Audio handle to add.
	void Add(std::shared_ptr<AudioHandle> handle);

	/// \brief Queues a callback for a given audio handle to run on the audio thread.
	/// \param handle Audio handle to associate with the callback.
	/// \param callback Function to call during audio thread processing.
	void QueueCallback(std::shared_ptr<AudioHandle> handle, AudioThread::AudioCallback callback);

	/// \brief Removes an audio handle from the audio thread's management.
	/// \param handle Audio handle to remove.
	void Remove(std::shared_ptr<AudioHandle> handle);
};

#endif // !defined(AFX_N3SNDMGR_H__9CB531B0_4FEB_4360_8141_D0BF61347BD7__INCLUDED_)
