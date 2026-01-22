#ifndef FILEIO_FILE_H
#define FILEIO_FILE_H

#pragma once

#include <filesystem> // std::filesystem::path, std::filesystem::file_size()
#include <cstdint>

/// \brief Abstract base class representing a generic file interface.
class File
{
public:
	/// \brief Gets the path of the file.
	/// \returns The filesystem path of the currently opened file.
	const std::filesystem::path& Path() const
	{
		return _path;
	}

	/// \brief Gets the current offset within the file.
	/// \returns The current read/write position.
	uint64_t Offset() const
	{
		return _offset;
	}

	/// \brief Gets the current file size.
	/// \returns The current size of the file, in bytes.
	uint64_t Size() const
	{
		return _size;
	}

	/// \brief Checks whether the file is currently open.
	/// \returns true if the file is open, false otherwise.
	bool IsOpen() const
	{
		return _open;
	}

protected:
	File() = default;

public:
	/// \brief Opens an existing file.
	/// \param path Path to the file to open.
	/// \returns true if the file was successfully opened, false otherwise.
	virtual bool OpenExisting(const std::filesystem::path& path)                                = 0;

	/// \brief Creates a new file.
	/// \param path Path to the file to create.
	/// \returns true if the file was created successfully, false otherwise.
	virtual bool Create(const std::filesystem::path& path)                                      = 0;

	/// \brief Reads data from the file at the current offset.
	/// \param buffer Destination buffer for data.
	/// \param bytesToRead Number of bytes to read.
	/// \param bytesRead Optional output for number of bytes actually read.
	/// \returns true if at least one byte was read successfully, false otherwise.
	virtual bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr)            = 0;

	/// \brief Writes data to the file at the current offset.
	/// \param buffer Data to write.
	/// \param bytesToWrite Number of bytes to write.
	/// \param bytesWritten Optional output for number of bytes actually written.
	/// \returns true if all requested bytes were written successfully, false otherwise.
	virtual bool Write(const void* buffer, size_t bytesToWrite, size_t* bytesWritten = nullptr) = 0;

	/// \brief Changes the current file offset.
	/// \param offset Offset value.
	/// \param origin One of SEEK_SET, SEEK_CUR, SEEK_END.
	/// \returns true if the new offset is valid, false otherwise.
	virtual bool Seek(int64_t offset, int origin)                                               = 0;

	/// \brief Flushes any buffered data to disk.
	virtual void Flush()                                                                        = 0;

	/// \brief Closes the file and release resources.
	/// \returns true if the file was open and closed successfully, false if it was already closed.
	virtual bool Close()                                                                        = 0;

	virtual ~File()
	{
	}

protected:
	uint64_t _offset = 0;        ///< Current offset within the file.
	uint64_t _size   = 0;        ///< Current file size in bytes.
	std::filesystem::path _path; ///< Path to the currently opened file.
	bool _open = false;          ///< Whether the file is currently open or not.
};

#endif                           // FILEIO_FILE_H
