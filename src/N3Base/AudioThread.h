#ifndef CLIENT_N3BASE_AUDIOTHREAD_H
#define CLIENT_N3BASE_AUDIOTHREAD_H

#pragma once

#include <shared/Thread.h>

#include <functional> // std::function<>
#include <memory>     // std::shared_ptr<>, std::unique_Ptr<>
#include <vector>     // std::vector<>

/// \enum e_AudioQueueType
/// \brief Type of operation queued for the audio thread.
///
/// Used internally by \ref AudioThread to manage audio handles and callbacks.
enum e_AudioQueueType : uint8_t
{
	/// Add a handle to the audio thread for playback and decoding.
	AUDIO_QUEUE_ADD = 0,

	/// Remove a handle from the audio thread and stop playback.
	AUDIO_QUEUE_REMOVE,

	/// Queue a callback to be executed on the audio thread.
	AUDIO_QUEUE_CALLBACK
};

struct AudioDecodedChunk;
class AudioDecoderThread;
class AudioHandle;

/// \class AudioThread
/// \brief Manages active audio playback and streamed decoding.
///
/// AudioThread runs a dedicated background thread that processes
/// queued audio handles, updates playback state, handles fades,
/// manages the decoder thread for streamed assets, and invokes
/// callbacks. This allows safe asynchronous audio playback
/// while maintaining real-time update of all sounds.
class AudioThread : public Thread
{
public:
	/// Callback type executed on the audio thread with an AudioHandle.
	using AudioCallback = std::function<void(AudioHandle*)>;

	/// Tuple representing a queued operation in the audio thread.
	///
	/// Consists of:
	/// - e_AudioQueueType: the type of operation.
	/// - shared_ptr<AudioHandle>: the handle affected by the operation.
	/// - AudioCallback: optional callback function.
	using QueueType     = std::tuple<e_AudioQueueType, std::shared_ptr<AudioHandle>, AudioCallback>;

	/// Constructs the audio thread object.
	AudioThread();

	/// Stops the thread and cleans up resources.
	~AudioThread() override;

	/// Main loop executed by the audio thread.
	///
	/// Processes queued handle operations, updates playback states,
	/// applies fades, and schedules decoding of streamed audio.
	void thread_loop() override;

	/// Queues an audio handle for playback and decoding.
	///
	/// Marks the handle as managed and ensures it is tracked by
	/// the audio thread until removed.
	///
	/// \param handle Audio handle to add.
	void Add(std::shared_ptr<AudioHandle> handle);

	/// Queues a callback to be executed on the audio thread.
	///
	/// \param handle Audio handle context for the callback.
	/// \param callback Function to invoke on the audio thread.
	void QueueCallback(std::shared_ptr<AudioHandle> handle, AudioCallback callback);

	/// Queues an audio handle for removal from the audio thread.
	///
	/// Stops playback, releases resources, and marks the handle as
	/// no longer managed.
	///
	/// \param handle Audio handle to remove.
	void Remove(std::shared_ptr<AudioHandle> handle);

private:
	/// Resets a handle for playback, stopping any current source.
	///
	/// For streamed handles, ensures buffers are allocated and
	/// triggers an initial decode if needed.
	///
	/// \param handle Audio handle to reset.
	/// \param alreadyManaged true if the handle is already active in the thread.
	void reset(std::shared_ptr<AudioHandle>& handle, bool alreadyManaged);

	/// Updates the decoder state for a streamed handle.
	///
	/// Processes any available decoded chunks and queues them to
	/// available OpenAL buffers for playback.
	///
	/// \param handle Audio handle to process.
	/// \param tmpDecodedChunk Temporary buffer used for decoded data to reduce allocations.
	void tick_decoder(std::shared_ptr<AudioHandle>& handle, AudioDecodedChunk& tmpDecodedChunk);

	/// Updates playback state for an audio handle.
	///
	/// This method handles start delays, fade-in, fade-out, looping, and
	/// stopping, updating the handle's internal timer and applying gain
	/// changes as needed.
	///
	/// The logic was originally part of \ref CN3SndObj::Tick(), but has been
	/// moved here into the audio thread to allow asynchronous, thread-safe
	/// playback updates. It no longer resides in CN3SndObj::Tick().
	///
	/// \param handle Audio handle to update.
	/// \param elapsedTime Time in seconds since the last tick.
	void tick_sound(std::shared_ptr<AudioHandle>& handle, float elapsedTime);

	/// Sets the OpenAL gain for a handle and updates its locally stored
	/// CurrentGain.
	///
	/// \param handle Audio handle to update.
	/// \param gain Gain value in range [0.0, 1.0].
	void set_gain(std::shared_ptr<AudioHandle>& handle, float gain);

	/// Starts playback of an audio handle.
	///
	/// Updates state, marks StartedPlaying, and triggers OpenAL playback.
	///
	/// \param handle Audio handle to play.
	void play_impl(std::shared_ptr<AudioHandle>& handle);

	/// Stops playback and releases OpenAL buffers for a handle.
	///
	/// For streamed handles, also removes them from the decoder thread.
	///
	/// \param handle Audio handle to remove.
	void remove_impl(std::shared_ptr<AudioHandle>& handle);

protected:
	/// Queue of pending operations for the audio thread.
	std::vector<QueueType> _pendingQueue;

	/// Background decoder thread for streamed audio assets.
	std::unique_ptr<AudioDecoderThread> _decoderThread;
};

#endif // CLIENT_N3BASE_AUDIOTHREAD_H
