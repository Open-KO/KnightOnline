#include "StdAfxBase.h"
#include "AudioDecoderThread.h"
#include "AudioHandle.h"
#include "AudioAsset.h"
#include "al_wrapper.h"

#include <cassert>			// assert()
#include <unordered_map>	// std::unordered_map<>

#include <FileIO/FileReader.h>
#include <mpg123.h>

using namespace std::chrono_literals;

void AudioDecoderThread::thread_loop()
{
	std::vector<QueueType> pendingQueue;
	std::unordered_map<uint32_t, std::shared_ptr<StreamedAudioHandle>> handleMap;

	while (_canTick)
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			std::cv_status status = _cv.wait_for(lock, 20ms);

			if (!_canTick)
				break;

			// Ignore spurious wakeups
			if (status != std::cv_status::timeout)
				continue;

			pendingQueue.swap(_pendingQueue);
		}

		// Process queue
		if (!pendingQueue.empty())
		{
			for (auto& [type, handle] : pendingQueue)
			{
				switch (type)
				{
					case AUDIO_DECODER_QUEUE_ADD:
						handleMap[handle->SourceId] = std::move(handle);
						break;

					case AUDIO_DECODER_QUEUE_REMOVE:
						while (!handle->DecodedChunks.empty())
							handle->DecodedChunks.pop();
						handleMap.erase(handle->SourceId);
						break;
				}
			}

			pendingQueue.clear();
		}

		// Run decode for this tick
		{
			std::lock_guard<std::mutex> lock(_decoderMutex);
			for (auto& [_, handle] : handleMap)
				decode_impl(handle.get());
		}
	}
}

void AudioDecoderThread::Add(std::shared_ptr<StreamedAudioHandle> handle)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_pendingQueue.push_back(std::make_tuple(AUDIO_DECODER_QUEUE_ADD, std::move(handle)));
}

// Force an initial decode *now* from the main audio thread
void AudioDecoderThread::InitialDecode(StreamedAudioHandle* handle)
{
	// Rewind back to the start.
	handle->RewindFrame();

	// Reset any existing decoded chunks.
	while (!handle->DecodedChunks.empty())
		handle->DecodedChunks.pop();

	// Perform a fresh decode.
	decode_impl(handle);
}

void AudioDecoderThread::Remove(std::shared_ptr<StreamedAudioHandle> handle)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_pendingQueue.push_back(std::make_tuple(AUDIO_DECODER_QUEUE_REMOVE, std::move(handle)));
}

void AudioDecoderThread::decode_impl(StreamedAudioHandle* handle)
{
	if (handle->DecodedChunks.size() >= MAX_AUDIO_STREAM_BUFFER_COUNT)
		return;

	StreamedAudioAsset* asset = static_cast<StreamedAudioAsset*>(handle->Asset.get());

	assert(asset->PcmChunkSize != 0);

	if (asset->DecoderType == AUDIO_DECODER_MP3)
		decode_impl_mp3(handle);
	else if (asset->DecoderType == AUDIO_DECODER_PCM)
		decode_impl_pcm(handle);
	else
		assert(!"AudioDecoderThread:decode_impl: Unsupported asset decoder type");
}

void AudioDecoderThread::decode_impl_mp3(StreamedAudioHandle* handle)
{
	assert(handle->Mp3Handle != nullptr);

	if (handle->Mp3Handle == nullptr)
		return;

	StreamedAudioAsset* asset = static_cast<StreamedAudioAsset*>(handle->Asset.get());
	const size_t ChunksToDecode = MAX_AUDIO_STREAM_BUFFER_COUNT - handle->DecodedChunks.size();

	for (size_t i = 0; i < ChunksToDecode; i++)
	{
		AudioDecodedChunk decodedChunk = {};
		decodedChunk.Data.resize(asset->PcmChunkSize);

		size_t done = 0;
		int error = mpg123_read(handle->Mp3Handle, &decodedChunk.Data[0], asset->PcmChunkSize, &done);

		// The first read will invoke MPG123_NEW_FORMAT.
		// This is where we'd fetch the format data, but we don't really care about it.
		// Retry the read so we can decode the chunk.
		while (error == MPG123_NEW_FORMAT)
			error = mpg123_read(handle->Mp3Handle, &decodedChunk.Data[0], asset->PcmChunkSize, &done);

		// Decoded a chunk
		if (error == MPG123_OK)
		{
			decodedChunk.BytesDecoded = static_cast<int32_t>(done);
			decodedChunk.Data.resize(done);

			handle->DecodedChunks.push(std::move(decodedChunk));
			handle->FinishedDecoding = false;
		}
		// Finished decoding chunks. This contains no data.
		else if (error == MPG123_DONE)
		{
			if (handle->FinishedDecoding)
				break;

			decodedChunk.BytesDecoded = 0;
			decodedChunk.Data.clear();

			handle->DecodedChunks.push(std::move(decodedChunk));

			// If we're looping, we should reset back to the first frame
			// and start decoding from there.
			if (handle->Settings->IsLooping)
				handle->RewindFrame();
			// Otherwise, we've finished decoding so we should stop here.
			else
				handle->FinishedDecoding = true;
		}
		else
		{
			assert(error);
		}
	}
}

void AudioDecoderThread::decode_impl_pcm(StreamedAudioHandle* handle)
{
	StreamedAudioAsset* asset = static_cast<StreamedAudioAsset*>(handle->Asset.get());

	assert(asset->PcmDataBuffer != nullptr);
	assert(asset->PcmDataSize != 0);

	if (asset->PcmDataBuffer == nullptr
		|| asset->PcmDataSize == 0)
		return;

	const size_t ChunksToDecode = MAX_AUDIO_STREAM_BUFFER_COUNT - handle->DecodedChunks.size();
	FileReaderHandle& fileReaderHandle = handle->FileReaderHandle;
	FileReader* file = asset->File.get();
	const size_t fileSize = static_cast<size_t>(file->Size());

	for (size_t i = 0; i < ChunksToDecode; i++)
	{
		AudioDecodedChunk decodedChunk = {};

		const size_t bytesRemaining = fileSize > fileReaderHandle.Offset
			? fileSize - fileReaderHandle.Offset
			: 0;

		size_t bytesToRead = asset->PcmChunkSize;
		if (bytesRemaining < bytesToRead)
			bytesToRead = bytesRemaining;

		// Finished reading data.
		if (bytesToRead == 0)
		{
			if (handle->FinishedDecoding)
				break;

			decodedChunk.BytesDecoded = 0;
			decodedChunk.Data.clear();

			handle->DecodedChunks.push(std::move(decodedChunk));

			// If we're looping, we should reset back to the first frame
			// and start decoding from there.
			if (handle->Settings->IsLooping)
				handle->RewindFrame();
			// Otherwise, we've finished decoding so we should stop here.
			else
				handle->FinishedDecoding = true;

			break;
		}

		// Read a chunk.
		decodedChunk.Data.resize(bytesToRead);

		std::memcpy(
			&decodedChunk.Data[0],
			static_cast<const uint8_t*>(file->Memory()) + fileReaderHandle.Offset,
			bytesToRead);

		handle->FileReaderHandle.Offset	+= bytesToRead;

		decodedChunk.BytesDecoded = static_cast<int32_t>(bytesToRead);

		handle->DecodedChunks.push(std::move(decodedChunk));
		handle->FinishedDecoding = false;
	}
}
