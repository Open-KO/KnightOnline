#ifndef CLIENT_N3BASE_AUDIOHANDLE_H
#define CLIENT_N3BASE_AUDIOHANDLE_H

#pragma once

#include "N3SndDef.h"

#include <atomic>	// std::atomic<>
#include <memory>	// std::shared_ptr<>
#include <vector>	// std::vector<>
#include <queue>	// std::queue<>

/// \enum e_AudioHandleType
/// \brief Identifies the playback audio handle type.
///
/// Used to distinguish between buffered and streamed audio playback
/// mechanisms.
enum e_AudioHandleType : uint8_t
{
	/// Audio handle backed by a fully buffered audio asset accessible via a shared OpenAL buffer.
	AUDIO_HANDLE_BUFFERED = 0,

	/// Audio handle backed by a streamed audio asset accessible via a memory mapped file.
	AUDIO_HANDLE_STREAMED,

	/// Audio handle type is unknown or uninitialized.
	AUDIO_HANDLE_UNKNOWN
};

class AudioAsset;

/// \class AudioHandle
/// \brief Base class representing an active audio playback instance.
///
/// AudioHandle encapsulates playback state, timing information, and
/// configuration shared between buffered and streamed audio playback.
/// Concrete subclasses manage backend-specific resources.
class AudioHandle
{
public:
	/// Type of audio handle (buffered or streamed).
	e_AudioHandleType				HandleType;

	/// Indicates whether this handle is currently managed by the audio thread.
	///
	/// The flag is set when the handle is queued for decoding/playback in
	/// the AudioThread (i.e. via AudioThread::Add) and cleared when the
	/// handle is removed from the AudioThread.
	/// 
	/// \ref CN3SndObj::Tick() checks this flag to detect when the handle is
	/// no longer active; if false, the object releases its reference to the
	/// handle, restoring it to its associated pool for reuse.
	std::atomic<bool>				IsManaged;

	/// Indicates whether playback has started.
	bool							StartedPlaying;

	/// Indicates whether playback has finished.
	///
	/// This flag can only be true if \ref StartedPlaying was previously true.
	/// It is set when the audio playback completes or reaches the end of the
	/// sound/stream.
	bool							FinishedPlaying;

	/// OpenAL source identifier associated with this handle.
	uint32_t						SourceId;

	/// Audio asset supplying data for playback.
	std::shared_ptr<AudioAsset>		Asset;

	/// Current sound playback state.
	///
	/// Indicates the lifecycle state of this audio handle, as defined by
	/// \ref e_SndState. Typical states include initial, playing, paused, or
	/// stopped. This is used by the audio engine to manage playback
	/// progression, transitions, and cleanup.
	e_SndState						State;

	/// Shared sound settings for this audio handle.
	///
	/// Contains volume settings. While the handle
	/// can access and modify these settings, they are not exclusively owned
	/// by the handle. For example, if the volume is changed, the new value
	/// persists in \ref CN3SndObj for subsequent playback of the same sound,
	/// ensuring consistency across uses. This allows both the audio engine
	/// and the handle instance to reference the same configuration.
	std::shared_ptr<SoundSettings>	Settings;

	/// Duration of fade-in effect, in seconds.
	float							FadeInTime;

	/// Duration of fade-out effect, in seconds.
	float							FadeOutTime;

	/// Delay before playback begins, in seconds.
	float							StartDelayTime;

	/// Internal playback timer used for state tracking, in seconds.
	float							Timer;

	/// Constructs an audio handle with default state.
	AudioHandle();

	/// Virtual destructor to allow safe deletion via base-class pointers.
	virtual ~AudioHandle();
};

/// \class BufferedAudioHandle
/// \brief Audio handle for fully buffered audio playback.
///
/// BufferedAudioHandle plays audio assets that are fully loaded into a
/// shared OpenAL buffer. Suitable for short sound effects with low-latency
/// playback requirements.
class BufferedAudioHandle : public AudioHandle
{
public:
	/// Creates a buffered audio handle for the given asset.
	///
	/// Allocates an OpenAL source ID from the buffered source pool and
	/// associates it with the provided audio asset.
	///
	/// \param asset Buffered audio asset to play.
	/// \return A shared pointer to the created handle, or nullptr on failure.
	static std::shared_ptr<BufferedAudioHandle> Create(std::shared_ptr<AudioAsset> asset);

public:
	/// Constructs a buffered audio handle.
	BufferedAudioHandle();

	/// Stops playback and releases the OpenAL source.
	~BufferedAudioHandle() override;
};

class FileReader;

/// \struct FileReaderHandle
/// \brief Tracks file reader state for streamed audio decoding.
///
/// Stores the current read position for the associated memory-mapped file.
struct FileReaderHandle
{
	/// Memory-mapped file reader instance used for streaming.
	FileReader* File	= nullptr;

	/// Current read offset, in bytes.
	size_t		Offset	= 0;
};

/// \struct AudioDecodedChunk
/// \brief Represents a chunk of decoded PCM audio data.
///
/// Decoded chunks are queued by the audio decoder thread and consumed
/// by the audio playback system.
struct AudioDecodedChunk
{
	/// Buffer containing decoded PCM audio data.
	std::vector<uint8_t>	Data;

	/// Number of valid bytes decoded into the buffer.
	///
	/// A value of zero indicates end-of-stream.
	int32_t					BytesDecoded = -1;
};

struct mpg123_handle_struct;

/// \class StreamedAudioHandle
/// \brief Audio handle for streamed audio playback.
///
/// StreamedAudioHandle manages incremental decoding and buffering of
/// streamed audio assets. Decoded PCM chunks are produced by the
/// AudioDecoderThread and queued for playback.
class StreamedAudioHandle : public AudioHandle
{
	friend class AudioThread;
	friend class AudioDecoderThread;

protected:
	/// File reader state used for streamed decoding.
	FileReaderHandle				FileReaderHandle;

	/// mpg123 decoder handle (used for MP3 streams).
	mpg123_handle_struct*			Mp3Handle;

	/// Queue of available OpenAL buffer IDs.
	std::queue<uint32_t>			AvailableBufferIds;

	/// All OpenAL buffer IDs allocated for this handle.
	std::vector<uint32_t>			BufferIds;

	/// Queue of decoded PCM audio chunks awaiting playback.
	std::queue<AudioDecodedChunk>	DecodedChunks;

	/// Indicates whether OpenAL buffers have been allocated.
	bool							BuffersAllocated;

	/// Indicates whether decoding has reached end-of-stream.
	bool							FinishedDecoding;

public:
	/// Creates a streamed audio handle for the given asset.
	///
	/// Initializes decoder state, allocates an OpenAL source ID, and
	/// prepares file or decoder handles based on the asset's decoder type.
	///
	/// \param asset Streamed audio asset to play.
	/// \return A shared pointer to the created handle, or nullptr on failure.
	static std::shared_ptr<StreamedAudioHandle> Create(std::shared_ptr<AudioAsset> asset);

public:
	/// Constructs a streamed audio handle.
	StreamedAudioHandle();

	/// Stops playback, releases OpenAL buffers, and destroys decoder state.
	~StreamedAudioHandle() override;

protected:
	/// Rewinds the decoder or file reader to the start of the audio stream.
	///
	/// Used when restarting playback or handling looping behavior.
	void RewindFrame();
};

#endif // CLIENT_N3BASE_AUDIOHANDLE_H
