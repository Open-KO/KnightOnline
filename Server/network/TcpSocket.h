#pragma once

#include <shared/CircularBuffer.h>
#include <asio.hpp>

#include <mutex>
#include <queue>

enum e_ConnectionState : uint8_t
{
	CONNECTION_STATE_CONNECTED = 1,
	CONNECTION_STATE_DISCONNECTED,
	CONNECTION_STATE_GAMESTART
};

class SocketManager;
class TcpSocket
{
	friend class SocketManager;

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

	e_ConnectionState GetState() const
	{
		return _state;
	}

	TcpSocket(SocketManager* socketManager);
	virtual ~TcpSocket() {}

	virtual int Send(char* pBuf, int length) = 0;

protected:
	int QueueAndSend(char* buffer, int length);
	virtual bool PullOutCore(char*& data, int& length) = 0;
	virtual void ReleaseToManager() = 0;

private:
	bool AsyncSend(bool fromAsyncChain);

public:
	void AsyncReceive();
	void ReceivedData(int length);
	virtual void Close() = 0;
	virtual void CloseProcess();
	void InitSocket();
	virtual void Parsing(int length, char* pData) = 0;
	virtual void Initialize() {}
	const std::string& GetRemoteIP();

protected:
	SocketManager*			_socketManager;
	RawSocket_t				_socket;

	int						_recvBufferSize;
	int						_sendBufferSize;

	// Data is written here directly from the socket. It shouldn't be used directly.
	std::vector<char>		_recvBuffer;

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

	e_ConnectionState		_state;
	bool					_pendingDisconnect;
	int16_t					_socketErrorCount;

	int						_socketId;
};
