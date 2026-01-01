#ifndef CLIENT_N3BASE_AUDIOASSET_H
#define CLIENT_N3BASE_AUDIOASSET_H

#pragma once

#include <memory> // std::unique_ptr<>
#include <string> // std::string

/// \enum e_AudioAssetType
/// \brief Describes how an audio asset is stored and consumed by the audio engine.
///
/// This enum is used to distinguish between fully-loaded audio buffers and
/// streamed audio assets, which may be decoded incrementally at runtime.
enum e_AudioAssetType : uint8_t
{
	/// Audio data is fully loaded into memory and played from a shared OpenAL buffer.
	AUDIO_ASSET_BUFFERED = 0,

	/// Audio data is memory mapped from file and streamed incremenetally (being decoded in chunks) during playback.
	AUDIO_ASSET_STREAMED,

	/// Placeholder type for undetermined audio asset types.
	AUDIO_ASSET_UNKNOWN
};

/// \enum e_AudioDecoderType
/// \brief Identifies the decoder used to interpret an audio asset's data format.
///
/// This enum indicates the decoding backend required to convert encoded audio
/// data into raw PCM samples for playback.
enum e_AudioDecoderType : uint8_t
{
	/// MPEG-1 / MPEG-2 Layer III compressed audio (.mp3).
	AUDIO_DECODER_MP3 = 0,

	/// Uncompressed PCM audio data (.wav, PCM streams only).
	AUDIO_DECODER_PCM,

	/// Placeholder type for undetermined audio decoder types.
	AUDIO_DECODER_UNKNOWN
};

/// \class AudioAsset
/// \brief Base class representing a loadable audio asset.
///
/// AudioAsset provides common metadata and lifetime management for audio
/// resources used by the sound engine. Concrete implementations define how
/// the audio data is loaded, decoded, and supplied to the playback backend
/// (e.g. buffered or streamed assets).
class AudioAsset
{
public:
	/// Specifies how the audio asset is stored and consumed.
	/// This is used for runtime identification of class types,
	/// in lieu of dynamic_cast() (which would require the use of RTTI).
	e_AudioAssetType	Type		= AUDIO_ASSET_UNKNOWN;

	/// Decoder used to convert encoded audio data into PCM samples.
	e_AudioDecoderType	DecoderType	= AUDIO_DECODER_UNKNOWN;

	/// Source filename of the audio asset.
	std::string			Filename;

	/// Reference count used to manage shared ownership of the asset.
	/// Once this hits 0 (i.e. nothing references it), the asset will be
	/// immediately removed from its pool and unloaded.
	uint32_t			RefCount	= 0;

	/// OpenAL audio format (e.g. AL_FORMAT_MONO16), or -1 if uninitialized.
	int32_t				AlFormat	= -1;

	/// Sample rate of the decoded audio data, in Hz.
	int32_t				SampleRate	= 0;

public:
	/// Loads the audio asset from a file.
	///
	/// Implementations are responsible for parsing the file and preparing
	/// the asset for playback, including populating format and sample rate
	/// information.
	///
	/// \param filename Path to the audio file to load.
	/// \return True if the asset was successfully loaded; false otherwise.
	virtual bool LoadFromFile(const std::string& filename) = 0;

	/// Virtual destructor to allow safe deletion via base-class pointers.
	virtual ~AudioAsset() {}
};

/// \class BufferedAudioAsset
/// \brief Audio asset fully loaded into memory for immediate playback.
///
/// BufferedAudioAsset loads the entire decoded audio stream into a shared
/// OpenAL buffer. This is suitable for short sound effects that are reused
/// frequently and require low-latency playback.
class BufferedAudioAsset : public AudioAsset
{
public:
	/// Constructs an empty buffered audio asset.
	BufferedAudioAsset();

	/// Loads and decodes an audio file into a shared OpenAL buffer.
	///
	/// Supports WAV files containing raw PCM data only. Other formats are not
	/// supported and will fail to load.
	///
	/// \param filename Path to the audio file to load.
	/// \returns true if the buffer was successfully created; false otherwise.
	bool LoadFromFile(const std::string& filename) override;

	/// Releases the associated OpenAL buffer.
	~BufferedAudioAsset() override;

public:
	uint32_t BufferId;
};

class FileReader;

/// \class StreamedAudioAsset
/// \brief Audio asset streamed incrementally during playback.
///
/// StreamedAudioAsset is designed for large audio files (e.g. music tracks)
/// that are decoded and supplied to the audio engine in chunks rather than
/// fully loaded into memory at once.
class StreamedAudioAsset : public AudioAsset
{
public:
	/// File reader used to stream encoded audio data from disk.
	///
	/// The underlying file remains open and memory-mapped for the entire lifetime
	/// of the streamed audio asset.
	std::unique_ptr<FileReader>	File;

	/// Total size of the decoded PCM data, in bytes.
	size_t						PcmDataSize;

	/// Size of each decoded PCM chunk, in bytes.
	size_t						PcmChunkSize;

	/// Pointer to the start of the decoded PCM data buffer.
	///
	/// This member is only used when \ref DecoderType is set to
	/// AUDIO_DECODER_PCM. The pointer references the beginning of the PCM data
	/// within the memory-mapped file and remains valid for the lifetime of the
	/// asset.
	const uint8_t*				PcmDataBuffer;

public:
	/// Constructs an empty streamed audio asset.
	StreamedAudioAsset();

	/// Opens the audio file and prepares it for streamed decoding.
	///
	/// Supported formats are MP3 (.mp3) and WAV files containing raw PCM data.
	/// Other formats are not supported and will fail to load.
	///
	/// \param filename Path to the audio file to stream.
	/// \returns true if the stream was successfully initialized; false otherwise.
	bool LoadFromFile(const std::string& filename) override;

	/// Releases streaming resources and closes the file.
	~StreamedAudioAsset() override;

protected:
	/// Loads and initializes an MP3 audio stream.
	///
	/// \param filename Path to the MP3 file.
	/// \returns true if the stream was successfully initialized; false otherwise.
	bool load_from_file_mp3(const std::string& filename);

	/// Loads and initializes a WAV (PCM) audio stream.
	///
	/// \param filename Path to the WAV file.
	/// \returns true if the stream was successfully initialized; false otherwise.
	bool load_from_file_wav(const std::string& filename);
};

#endif // CLIENT_N3BASE_AUDIOASSET_H
