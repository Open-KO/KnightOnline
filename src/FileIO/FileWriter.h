#ifndef FILEIO_FILEWRITER_H
#define FILEIO_FILEWRITER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

/// \brief FileWriter implements a write-capable file interface.
///
/// Supports writing to a file, seeking, and flushing data to disk.
/// Reading operations are not supported. Files may be extended by seeking past the current size.
class FileWriter : public File
{
public:
	/// \brief Gets the size of the file on disk.
	/// \return Current file size on disk.
	uint64_t SizeOnDisk() const
	{
		return _sizeOnDisk;
	}

	/// \brief Constructs a new FileWriter object.
	FileWriter();

	/// \brief Opens an existing file for writing.
	/// \param path Path to the file to open.
	/// \returns true if the file was successfully opened, false otherwise.
	/// \note The initial write offset starts at 0, not at the end of the file.
	///       The caller must manually seek to the end if they intend to append.
	///       This matches Win32 behaviour, as well as general C file I/O behaviour
	///       as we otherwise have no explicit append flag to rely upon.
	bool OpenExisting(const std::filesystem::path& path) override;

	/// \brief Creates a new file for writing, truncating if it exists.
	/// \param path Path to the file to create.
	/// \returns true if the file was created successfully, false otherwise.
	bool Create(const std::filesystem::path& path) override;

	/// \brief Reading is not supported for FileWriter.
	/// \param buffer Destination buffer for data.
	/// \param bytesToRead Number of bytes to read.
	/// \param bytesRead Optional output for number of bytes actually read.
	/// \returns false always.
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr) override;

	/// \brief Writes data to the file at the current offset.
	/// \param buffer Data to write.
	/// \param bytesToWrite Number of bytes to write.
	/// \param bytesWritten Optional output for number of bytes actually written.
	/// \returns true if all requested bytes were written, false otherwise.
	/// \note Writing beyond the current file size automatically expands the internal size.
	bool Write(const void* buffer, size_t bytesToWrite, size_t* bytesWritten = nullptr) override;

	/// \brief Changes the current write offset.
	/// \param offset Offset value.
	/// \param origin One of SEEK_SET, SEEK_CUR, SEEK_END.
	/// \returns true if the new offset is valid, false otherwise.
	/// \note Seeking past the current file size will expand the internal file size but does not
	///       immediately update the file on-disk until Flush() or Close().
	bool Seek(int64_t offset, int origin) override;

	/// \brief Flushes any buffered writes to disk.
	/// \note This will expand the on-disk file if the offset moved beyond the previous size.
	void Flush() override;

	/// \brief Closes the file and flush any pending writes.
	/// \returns true if the file was open and closed successfully, false if already closed.
	bool Close() override;

	/// \brief Destroys the FileWriter object.
	~FileWriter() override;

protected:
	LLFIO_V2_NAMESPACE::file_handle _fileHandle; ///< File handle used for writing.
	uint64_t _sizeOnDisk = 0;                    ///< Current size of the file on disk.
};

#endif                                           // FILEIO_FILEWRITER_H
