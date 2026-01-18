#ifndef SHARED_PACKET_H
#define SHARED_PACKET_H

#pragma once

#include "ByteBuffer.h"

class Packet : public ByteBuffer
{
public:
	Packet() : ByteBuffer()
	{
	}

	Packet(uint8_t opcode) : ByteBuffer(4096)
	{
		append(&opcode, 1);
	}

	Packet(uint8_t opcode, size_t res) : ByteBuffer(res)
	{
		append(&opcode, 1);
	}

	Packet(const Packet& packet) : ByteBuffer(packet)
	{
	}

	uint8_t GetOpcode() const
	{
		const auto& buffer = storage();
		return buffer.empty() ? 0 : buffer[0];
	}

	//! Clear packet and set opcode all in one mighty blow
	void Initialize(uint8_t opcode)
	{
		clear();
		append(&opcode, 1);
	}
};

#endif // SHARED_PACKET_H
