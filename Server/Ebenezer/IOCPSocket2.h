// IOCPSocket2.h: interface for the CIOCPSocket2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOCPSOCKET2_H__36499609_63DD_459C_B4D0_1686FEEC67C2__INCLUDED_)
#define AFX_IOCPSOCKET2_H__36499609_63DD_459C_B4D0_1686FEEC67C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOCPort.h"
#include "Define.h"

#include <shared/CircularBuffer.h>
#include <shared/JvCryption.h>

class CCompressMng;
class CIOCPSocket2
{
	friend class CIOCPort;

	using RawSocket_t = asio::ip::tcp::socket;

public:
	int GetSocketID() const
	{
		return _socketId;
	}

	void SetSocketID(int sid)
	{
		_socketId = sid;
	}

	uint8_t GetState() const
	{
		return _state;
	}

	uint8_t GetSockType() const
	{
		return _type;
	}

	CIOCPSocket2(CIOCPort* iocPort);
	virtual ~CIOCPSocket2();

	bool Create();
	bool Connect(const char* hostAddress, uint16_t hostPort);
	int  Send(char* pBuf, int length);

private:
	bool DoSend(bool fromAsyncChain);

public:
	void Receive();
	void ReceivedData(int length);
	bool PullOutCore(char*& data, int& length);
	void Close();
	virtual void CloseProcess();
	void InitSocket();
	virtual void Parsing(int length, char* pData);
	virtual void Initialize();
	void SendCompressingPacket(const char* pData, int len);
	void RegionPacketAdd(char* pBuf, int len);
	void RegionPacketClear(char* GetBuf, int& len);
	const std::string& GetRemoteIP();

public:
	_REGION_BUFFER*			_regionBuffer;

protected:
	CIOCPort*				_iocPort;
	RawSocket_t				_socket;

	// Data is written here directly from the socket. It shouldn't be used directly.
	char					_recvBuffer[MAX_PACKET_SIZE];

	// Received data is output to the circular buffer from _recvBuffer.
	// This should be parsed to handle packets.
	CCircularBuffer			_recvCircularBuffer;

	// Sends are queued for consistency.
	// These are typically submitted as spans of the circular buffer, so we usually just send {portion 1},{len 1}.
	// Upon wraparound, this splits the write into 2, so we submit {portion 1},{len 1} (end of the circular buffer)
	// and {portion 2},{len 2} (start of the buffer).
	// These are not considered owned.
	// In the event there's too much data in the circular buffer to send, we allocate our own contiguous buffer here for it,
	// and submit that instead.
	// This buffer is considered owned (by the send queue), so the buffer will be freed once the send is complete.
	struct QueuedSend
	{
		CircularBufferSpan	BufferSpan = {};
		bool				IsOwned = false;

		~QueuedSend()
		{
			if (IsOwned)
				delete[] BufferSpan.Buffer1;
		}
	};

	std::queue<std::unique_ptr<QueuedSend>>	_sendQueue;
	std::recursive_mutex	_sendMutex;

	CCircularBuffer			_sendCircularBuffer;
	bool					_sendInProgress;

	bool					_remoteIpCached;
	std::string				_remoteIp;

	uint8_t					_type;
	uint8_t					_state;
	int16_t					_socketErrorCount;

	int						_socketId;

	// Cryption
	CJvCryption				_jvCryption;
	bool					_jvCryptionEnabled;

	uint32_t				_sendValue;
	uint32_t				_recvValue;
	///~
};

#endif // !defined(AFX_IOCPSOCKET2_H__36499609_63DD_459C_B4D0_1686FEEC67C2__INCLUDED_)
