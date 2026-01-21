#ifndef SHARED_BYTEBUFFER_H
#define SHARED_BYTEBUFFER_H

#pragma once

#include <cstdint>
#include <string>
#include <vector>

class ByteBuffer
{
public:
	static constexpr size_t DEFAULT_SIZE = 32;

	ByteBuffer();
	ByteBuffer(size_t res);
	ByteBuffer(const ByteBuffer& buf);
	virtual ~ByteBuffer();
	void clear();

	//
	// stream like operators for storing data
	//
	ByteBuffer& operator<<(ByteBuffer& value);

	template <typename T>
	ByteBuffer& operator<<(T value);

	// Hacky KO string flag - either it's a single byte length, or a double byte.
	void SByte();
	void DByte();

	uint8_t operator[](size_t pos);
	size_t rpos() const;
	size_t rpos(size_t rpos);
	size_t wpos() const;
	size_t wpos(size_t wpos);

	bool read(size_t pos, void* dest, size_t len) const;
	bool read(void* dest, size_t len);

	template <typename T>
	T read(size_t pos) const;

	template <typename T>
	T read();

	bool readString(size_t pos, std::string& dest) const;
	bool readString(size_t pos, std::string& dest, size_t len) const;

	bool readString(std::string& dest);
	bool readString(std::string& dest, size_t len);

	const std::vector<uint8_t>& storage() const;
	std::vector<uint8_t>& storage();
	const uint8_t* contents() const;
	size_t size() const;

	// one should never use resize
	void resize(size_t newsize);
	void sync_for_read();
	void reserve(size_t ressize);

	// append to the end of buffer
	void append(const void* src, size_t cnt);
	void append(const ByteBuffer& buffer);
	void append(const ByteBuffer& buffer, size_t len);

	template <typename T>
	void append(T value);

	void readFrom(ByteBuffer& buffer, size_t len);

	void put(size_t pos, const void* src, size_t cnt);

	template <typename T>
	void put(size_t pos, T value);

private:
	bool _doubleByte;

	// read and write positions
	size_t _rpos, _wpos;
	std::vector<uint8_t> _storage;
};

#endif // SHARED_BYTEBUFFER_H
