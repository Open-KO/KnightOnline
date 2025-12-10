#ifndef SHARED_BYTEBUFFER_H
#define SHARED_BYTEBUFFER_H

#pragma once

#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

class ByteBuffer
{
public:
	constexpr static size_t DEFAULT_SIZE = 32;
	bool _doubleByte;

	ByteBuffer();
	ByteBuffer(size_t res);
	ByteBuffer(const ByteBuffer& buf);
	virtual ~ByteBuffer();
	void clear();

	template <typename T>
	requires std::is_trivial_v<T>
	void append(T value)
	{
		append(&value, sizeof(value));
	}

	template <typename T>
	requires std::is_trivial_v<T>
	void put(size_t pos, T value)
	{
		put(pos, &value, sizeof(value));
	}

	//
	// stream like operators for storing data
	//

	ByteBuffer& operator<<(bool value);

	// unsigned

	ByteBuffer& operator<<(uint8_t value);
	ByteBuffer& operator<<(uint16_t value);
	ByteBuffer& operator<<(uint32_t value);
	ByteBuffer& operator<<(uint64_t value);

	// signed

	ByteBuffer& operator<<(int8_t value);
	ByteBuffer& operator<<(int16_t value);
	ByteBuffer& operator<<(int32_t value);
	ByteBuffer& operator<<(int64_t value);

	ByteBuffer& operator<<(float value);

	ByteBuffer& operator<<(ByteBuffer& value);

	// Hacky KO string flag - either it's a single byte length, or a double byte.
	void SByte();
	void DByte();

	uint8_t operator[](size_t pos);
	size_t rpos() const;
	size_t rpos(size_t rpos);
	size_t wpos() const;
	size_t wpos(size_t wpos);

	template <typename T>
	T read()
	{
		T r;
		if constexpr (std::is_same_v<T, std::string>)
		{
			readString(r);
		}
		else
		{
			r = read<T>(_rpos);
			_rpos += sizeof(T);
		}
		return r;
	}

	template <typename T>
	T read(size_t pos) const
	{
		//ASSERT(pos + sizeof(T) <= size());
		if (pos + sizeof(T) > size())
			return (T) 0;
		return *((T*) &_storage[pos]);
	}

	void read(void* dest, size_t len);
	void readString(std::string& dest);
	void readString(std::string& dest, size_t len);

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

	void readFrom(ByteBuffer& buffer, size_t len);
	void put(size_t pos, const void* src, size_t cnt);

protected:
	// read and write positions
	size_t _rpos, _wpos;
	std::vector<uint8_t> _storage;
};

#endif // SHARED_BYTEBUFFER_H
