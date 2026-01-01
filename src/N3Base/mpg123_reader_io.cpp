#include "StdAfxBase.h"
#include "mpg123_reader_io.h"
#include "AudioHandle.h"

#include <FileIO/FileReader.h>

mpg123_ssize_t mpg123_filereader_read(void* userData, void* dst, size_t bytes)
{
	if (bytes == 0)
		return 0;

	FileReaderHandle* handle = static_cast<FileReaderHandle*>(userData);
	const FileReader* file = handle->File;
	const size_t size = static_cast<size_t>(file->Size());

	if (handle->Offset >= size)
		return 0;

	const size_t remainingBytes = size - handle->Offset;
	size_t bytesToRead = std::min(bytes, remainingBytes);

	std::memcpy(
		dst,
		static_cast<const uint8_t*>(file->Memory()) + handle->Offset,
		bytesToRead);

	handle->Offset += bytesToRead;
	return static_cast<mpg123_ssize_t>(bytesToRead);
}

off_t mpg123_filereader_seek(void* userData, off_t offset, int whence)
{
	FileReaderHandle* handle = static_cast<FileReaderHandle*>(userData);
	const FileReader* file = handle->File;
	const size_t size = static_cast<size_t>(file->Size());

	size_t newPos = 0;

	switch (whence)
	{
		case SEEK_SET:
			newPos = static_cast<size_t>(offset);
			break;

		case SEEK_CUR:
			newPos = handle->Offset + static_cast<size_t>(offset);
			break;

		case SEEK_END:
			newPos = size + static_cast<size_t>(offset);
			break;

		default:
			errno = EINVAL;
			return -1; // unhandled type
	}

	handle->Offset = newPos;
	return static_cast<off_t>(handle->Offset);
}

void mpg123_filereader_cleanup(void* /*userData*/)
{
	/* do nothing, cleanup is managed externally */
}
