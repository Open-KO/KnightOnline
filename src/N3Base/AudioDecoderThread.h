#ifndef CLIENT_N3BASE_AUDIODECODERTHREAD_H
#define CLIENT_N3BASE_AUDIODECODERTHREAD_H

#pragma once

#include <shared/Thread.h>

#include <memory> // std::shared_ptr<>
#include <mutex>  // std::mutex
#include <vector> // std::vector<>

/// \enum e_AudioDecoderQueueType
/// \brief Describes operations queued for the audio decoder thread.
///
/// Used internally by AudioDecoderThread to manage streamed audio handles
/// that are added to or removed from the active decode set.
enum e_AudioDecoderQueueType : uint8_t
{
	AUDIO_DECODER_QUEUE_ADD = 0,
	AUDIO_DECODER_QUEUE_REMOVE
};

class StreamedAudioHandle;

/// \class AudioDecoderThread
/// \brief Background thread responsible for decoding streamed audio data.
///
/// AudioDecoderThread runs continuously and incrementally decodes audio
/// data for active streamed audio handles. Decoded PCM chunks are queued
/// on each handle and consumed by the audio playback system.
///
/// Supported decoder types are MP3 and raw PCM streams.
class AudioDecoderThread : public Thread
{
public:
	/// Tuple type representing a queued decoder operation.
	///
	/// The tuple consists of a queue operation type and the associated
	/// streamed audio handle.
	using QueueType = std::tuple<e_AudioDecoderQueueType, std::shared_ptr<StreamedAudioHandle>>;

	/// Returns a mutex protecting decoder execution.
	///
	/// This mutex must be held when accessing decoded chunk queues or
	/// performing decode operations outside the decoder thread.
	std::mutex& DecoderMutex()
	{
		return _decoderMutex;
	}

	/// Main thread loop executed by the decoder thread.
	///
	/// Processes queued add/remove requests and performs incremental
	/// decoding for all active streamed audio handles.
	void thread_loop() override;

	/// Queues a streamed audio handle for decoding.
	///
	/// The handle will begin receiving decoded audio chunks on subsequent
	/// decoder thread ticks.
	///
	/// \param handle Streamed audio handle to add.
	void Add(std::shared_ptr<StreamedAudioHandle> handle);

	/// Performs an immediate initial decode on the calling thread.
	///
	/// This function rewinds the stream, clears any existing decoded data,
	/// and decodes an initial chunk synchronously. Intended to be called
	/// from the main audio thread before playback begins.
	///
	/// \param handle Streamed audio handle to decode.
	void InitialDecode(StreamedAudioHandle* handle);

	/// Queues a streamed audio handle for removal.
	///
	/// Any decoded chunks are discarded and the handle is removed from
	/// active decoding.
	///
	/// \param handle Streamed audio handle to remove.
	void Remove(std::shared_ptr<StreamedAudioHandle> handle);

protected:
	/// Dispatches decoding based on the asset's decoder type.
	///
	/// Decoding is skipped if the handle already has the maximum number of
	/// decoded chunks buffered, as defined by \ref MAX_AUDIO_STREAM_BUFFER_COUNT.
	///
	/// \param handle Streamed audio handle to decode.
	void decode_impl(StreamedAudioHandle* handle);

	/// Decodes MP3 audio data into PCM chunks.
	///
	/// Uses mpg123 to incrementally decode frames and enqueue decoded
	/// audio data. Handles looping and end-of-stream signaling.
	///
	/// \param handle Streamed audio handle to decode.
	void decode_impl_mp3(StreamedAudioHandle* handle);

	/// "Decodes" raw PCM audio data into chunks.
	///
	/// PCM data is copied directly from a memory-mapped file into decoded
	/// chunk buffers. Handles looping and end-of-stream signaling.
	///
	/// \param handle Streamed audio handle to decode.
	void decode_impl_pcm(StreamedAudioHandle* handle);

protected:
	/// Mutex protecting decode execution and decoded chunk access.
	std::mutex _decoderMutex;

	/// Pending queue of add/remove operations for streamed audio handles.
	std::vector<QueueType> _pendingQueue;
};

#endif // CLIENT_N3BASE_AUDIODECODERTHREAD_H
