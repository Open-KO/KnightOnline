#include "StdAfxBase.h"
#include "AudioThread.h"
#include "AudioDecoderThread.h"
#include "AudioAsset.h"
#include "AudioHandle.h"
#include "al_wrapper.h"

#include <cassert>       // assert()
#include <unordered_map> // std::unordered_map<>

#include "N3Base.h"      // CN3Base::TimeGet()

using namespace std::chrono_literals;

AudioThread::AudioThread()
{
}

AudioThread::~AudioThread()
{
}

void AudioThread::thread_loop()
{
	std::vector<QueueType> pendingQueue;
	std::unordered_map<uint32_t, std::shared_ptr<AudioHandle>> handleMap;
	AudioDecodedChunk tmpDecodedChunk; // keep a copy here so we aren't constantly reallocating
	float previousTime = CN3Base::TimeGet();

	_decoderThread     = std::make_unique<AudioDecoderThread>();
	_decoderThread->start();

	while (CanTick())
	{
		{
			std::unique_lock<std::mutex> lock(ThreadMutex());
			std::cv_status status = ThreadCondition().wait_for(lock, 20ms);

			if (!CanTick())
				break;

			// Ignore spurious wakeups
			if (status != std::cv_status::timeout)
				continue;

			pendingQueue.swap(_pendingQueue);
		}

		// Process queue
		if (!pendingQueue.empty())
		{
			for (auto& [type, handle, callback] : pendingQueue)
			{
				switch (type)
				{
					case AUDIO_QUEUE_ADD:
						reset(handle, handleMap.contains(handle->SourceId));
						handleMap[handle->SourceId] = std::move(handle);
						break;

					case AUDIO_QUEUE_CALLBACK:
						if (callback != nullptr)
							callback(handle.get());
						break;

					case AUDIO_QUEUE_REMOVE:
						remove_impl(handle);
						handleMap.erase(handle->SourceId);

						handle->IsManaged.store(false);
						break;
				}
			}

			pendingQueue.clear();
		}

		float currentTime = CN3Base::TimeGet();
		float elapsedTime = currentTime - previousTime;

		for (auto& [_, handle] : handleMap)
		{
			if (handle->State != SNDSTATE_STOP)
			{
				tick_decoder(handle, tmpDecodedChunk);
				tick_sound(handle, elapsedTime);
			}

			// Sound is now stopped (either prior or as a result of the tick),
			// queue it for removal.
			if (handle->State == SNDSTATE_STOP)
				Remove(handle);
		}

		previousTime = currentTime;
	}

	_decoderThread->shutdown();
	_decoderThread.reset();
}

void AudioThread::Add(std::shared_ptr<AudioHandle> handle)
{
	assert(handle != nullptr);

	if (handle == nullptr)
		return;

	assert(handle->SourceId != INVALID_AUDIO_SOURCE_ID);

	if (handle->SourceId == INVALID_AUDIO_SOURCE_ID)
		return;

	// Consider the handle managed from the moment we try to add it to the thread.
	// This avoids the gap between processing where it might not quite be in the map yet.
	handle->IsManaged.store(true);

	std::lock_guard<std::mutex> lock(ThreadMutex());
	_pendingQueue.push_back(std::make_tuple(AUDIO_QUEUE_ADD, std::move(handle), nullptr));
}

void AudioThread::QueueCallback(std::shared_ptr<AudioHandle> handle, AudioCallback callback)
{
	assert(handle != nullptr);

	if (handle == nullptr)
		return;

	assert(handle->SourceId != INVALID_AUDIO_SOURCE_ID);

	if (handle->SourceId == INVALID_AUDIO_SOURCE_ID)
		return;

	std::lock_guard<std::mutex> lock(ThreadMutex());
	_pendingQueue.push_back(
		std::make_tuple(AUDIO_QUEUE_CALLBACK, std::move(handle), std::move(callback)));
}

void AudioThread::Remove(std::shared_ptr<AudioHandle> handle)
{
	assert(handle != nullptr);

	if (handle == nullptr)
		return;

	assert(handle->SourceId != INVALID_AUDIO_SOURCE_ID);

	if (handle->SourceId == INVALID_AUDIO_SOURCE_ID)
		return;

	std::lock_guard<std::mutex> lock(ThreadMutex());
	_pendingQueue.push_back(std::make_tuple(AUDIO_QUEUE_REMOVE, std::move(handle), nullptr));
}

void AudioThread::reset(std::shared_ptr<AudioHandle>& handle, bool alreadyManaged)
{
	alSourceStop(handle->SourceId);
	AL_CHECK_ERROR();

	alSourceRewind(handle->SourceId);
	AL_CHECK_ERROR();

	alSourcei(handle->SourceId, AL_BUFFER, 0);
	AL_CHECK_ERROR();

	handle->StartedPlaying  = false;
	handle->FinishedPlaying = false;

	if (handle->HandleType == AUDIO_HANDLE_STREAMED)
	{
		auto streamedAudioHandle = std::static_pointer_cast<StreamedAudioHandle>(handle);
		if (streamedAudioHandle != nullptr)
		{
			// Initialize the buffers
			if (!streamedAudioHandle->BuffersAllocated)
			{
				for (size_t i = 0; i < MAX_AUDIO_STREAM_BUFFER_COUNT; i++)
				{
					ALuint bufferId = INVALID_AUDIO_BUFFER_ID;

					alGenBuffers(1, &bufferId);
					if (!AL_CHECK_ERROR() && bufferId != INVALID_AUDIO_BUFFER_ID)
					{
						streamedAudioHandle->BufferIds.push_back(bufferId);
						streamedAudioHandle->AvailableBufferIds.push(bufferId);
					}
				}

				streamedAudioHandle->BuffersAllocated = true;
			}
			else
			{
				while (!streamedAudioHandle->AvailableBufferIds.empty())
					streamedAudioHandle->AvailableBufferIds.pop();

				for (uint32_t bufferId : streamedAudioHandle->BufferIds)
					streamedAudioHandle->AvailableBufferIds.push(bufferId);
			}

			// Force initial decode now, so we can safely start playing.
			// If we're already managed by this thread, we're also in the decoder thread already.
			// As such, we should be safe and guard access.
			if (alreadyManaged)
			{
				std::lock_guard<std::mutex> lock(_decoderThread->DecoderMutex());
				_decoderThread->InitialDecode(streamedAudioHandle.get());
			}
			else
			{
				_decoderThread->InitialDecode(streamedAudioHandle.get());
			}

			// Add to decoder for future decodes.
			_decoderThread->Add(std::move(streamedAudioHandle));

			// Force an initial tick to push our decoded data before play is triggered.
			AudioDecodedChunk tmpDecodedChunk {};
			tick_decoder(handle, tmpDecodedChunk);
		}
	}
}

void AudioThread::tick_decoder(
	std::shared_ptr<AudioHandle>& handle, AudioDecodedChunk& tmpDecodedChunk)
{
	// Process streamed audio
	if (handle->HandleType == AUDIO_HANDLE_STREAMED)
	{
		StreamedAudioHandle* streamedAudioHandle = static_cast<StreamedAudioHandle*>(handle.get());

		// If any buffers have been processed, we should remove them and attempt to replace them
		// with any existing decoded chunks.
		ALint buffersProcessed                   = 0;
		alGetSourcei(handle->SourceId, AL_BUFFERS_PROCESSED, &buffersProcessed);

		if (!AL_CHECK_ERROR() && buffersProcessed > 0)
		{
			// Unqueue the processed buffers so we can reuse their IDs for our new chunks
			for (int i = 0; i < buffersProcessed; i++)
			{
				ALuint bufferId = 0;
				alSourceUnqueueBuffers(handle->SourceId, 1, &bufferId);

				if (AL_CHECK_ERROR())
					continue;

				streamedAudioHandle->AvailableBufferIds.push(bufferId);
			}
		}

		while (!streamedAudioHandle->AvailableBufferIds.empty())
		{
			// Make sure we have a chunk to replace with first, before unqueueing.
			// This way we don't need additional tracking.
			{
				std::lock_guard<std::mutex> lock(_decoderThread->DecoderMutex());
				if (streamedAudioHandle->DecodedChunks.empty())
					break;

				tmpDecodedChunk = std::move(streamedAudioHandle->DecodedChunks.front());
				streamedAudioHandle->DecodedChunks.pop();
			}

			// < 0 indicates error
			// 0 indicates EOF
			if (tmpDecodedChunk.BytesDecoded <= 0)
				continue;

			// Buffer the new decoded data.
			ALuint bufferId = streamedAudioHandle->AvailableBufferIds.front();
			alBufferData(bufferId, streamedAudioHandle->Asset->AlFormat, &tmpDecodedChunk.Data[0],
				tmpDecodedChunk.BytesDecoded, streamedAudioHandle->Asset->SampleRate);
			if (AL_CHECK_ERROR())
				continue;

			alSourceQueueBuffers(handle->SourceId, 1, &bufferId);
			if (AL_CHECK_ERROR())
				continue;

			streamedAudioHandle->AvailableBufferIds.pop();
		}

		ALint buffersQueued = 0;
		alGetSourcei(handle->SourceId, AL_BUFFERS_QUEUED, &buffersQueued);
	}

	if (handle->StartedPlaying && !handle->FinishedPlaying)
	{
		ALint state = AL_INITIAL;
		alGetSourcei(handle->SourceId, AL_SOURCE_STATE, &state);
		if (!AL_CHECK_ERROR() && state == AL_STOPPED)
			handle->FinishedPlaying = true;
	}
}

void AudioThread::tick_sound(std::shared_ptr<AudioHandle>& handle, float elapsedTime)
{
	handle->Timer += elapsedTime;

	if (handle->State == SNDSTATE_DELAY && handle->Timer >= handle->StartDelayTime)
	{
		play_impl(handle);

		handle->Timer = 0.0f;
		handle->State = SNDSTATE_FADEIN;
	}

	if (handle->State == SNDSTATE_FADEIN)
	{
		if (handle->Timer >= handle->FadeInTime)
		{
			handle->Timer = 0.0f;
			handle->State = SNDSTATE_PLAY;

			set_gain(handle, handle->Settings->MaxGain);
		}
		else
		{
			float gain = 0;
			if (handle->FadeInTime > 0.0f)
				gain = ((handle->Timer / handle->FadeInTime) * handle->Settings->MaxGain);

			set_gain(handle, gain);
		}
	}

	if (handle->State == SNDSTATE_PLAY)
	{
		if (!handle->Settings->IsLooping && handle->FinishedPlaying)
		{
			handle->Timer = 0.0f;
			handle->State = SNDSTATE_FADEOUT;
		}
	}

	if (handle->State == SNDSTATE_FADEOUT)
	{
		if (handle->Timer >= handle->FadeOutTime)
		{
			// Invoke a stop.
			handle->Timer = 0.0f;
			handle->State = SNDSTATE_STOP;

			set_gain(handle, 0.0f);
		}
		else
		{
			float gain = 0.0f;
			if (handle->FadeOutTime > 0.0f)
				gain = (((handle->FadeOutTime - handle->Timer) / handle->FadeOutTime)
						* handle->Settings->MaxGain);

			set_gain(handle, gain);
		}
	}
}

void AudioThread::set_gain(std::shared_ptr<AudioHandle>& handle, float gain)
{
	gain                          = std::clamp(gain, 0.0f, 1.0f);
	handle->Settings->CurrentGain = gain;

	alSourcef(handle->SourceId, AL_GAIN, gain);
	AL_CHECK_ERROR();
}

void AudioThread::play_impl(std::shared_ptr<AudioHandle>& handle)
{
	handle->State           = SNDSTATE_PLAY;
	handle->StartedPlaying  = true;
	handle->FinishedPlaying = false;

	ALint state             = AL_INITIAL;
	alGetSourcei(handle->SourceId, AL_SOURCE_STATE, &state);
	AL_CLEAR_ERROR_STATE();

	if (state != AL_PLAYING)
	{
		alSourcePlay(handle->SourceId);
		AL_CHECK_ERROR();
	}
}

void AudioThread::remove_impl(std::shared_ptr<AudioHandle>& handle)
{
	alSourceStop(handle->SourceId);
	AL_CHECK_ERROR();

	alSourceRewind(handle->SourceId);
	AL_CHECK_ERROR();

	alSourcei(handle->SourceId, AL_BUFFER, 0);
	AL_CHECK_ERROR();

	if (handle->HandleType == AUDIO_HANDLE_STREAMED)
	{
		auto streamedAudioHandle = std::static_pointer_cast<StreamedAudioHandle>(handle);
		if (streamedAudioHandle != nullptr)
		{
			_decoderThread->Remove(streamedAudioHandle);

			while (!streamedAudioHandle->AvailableBufferIds.empty())
				streamedAudioHandle->AvailableBufferIds.pop();
		}
	}
}
