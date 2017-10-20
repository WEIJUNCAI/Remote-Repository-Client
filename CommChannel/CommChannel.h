#pragma once

/////////////////////////////////////////////////////////////////////////////
//  CommChannel.h - Communication channel for peer messaging               //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Jim Fawcett											   //
//				  Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the implementaion of sender, reciever and client handler
*   class which implements a message based asynchronous communication channel among peers
*
*/
/*
*   Build Process
*   -------------
*   - Required files: FileSystem.h, FileSystem.cpp
*                     HttpMessage.h. HttpMessage.cpp
*					  Sockets.h, Sockets.cpp
*					  CommChannel.h, CommChannel.cpp
*
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 April 2017
*/

#include <thread>
#include <unordered_map>
#include <utility>
#include <atomic>
#include "../Sockets/Sockets.h"
#include "IComm.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../FileSystem/FileSystem.h"
#include "../HttpMessage/HttpMessage.h"


using BQueue = Async::BlockingQueue<Message>;

//////////////////////////////////////////////
// Sendr class
// - accepts messages from client for sending
//
class Sendr : public ISendr
{
public:
	using AddrInfo = std::string;

	Sendr();
	~Sendr();
	// put message to send in the sending queue
	void postMessage(const HttpMessage& msg) override;
	// connect to the server on specified destination
	bool connectServer(const std::string& ip, size_t port);
	// get and set the send queue
	BQueue& queue();
	
private:
	// helper methods for message sending, 
	// required additional processing when dealing with
	// file related messages
	void send_all();
	bool send_message(Message& msgToSend);
	bool handle_file_msg_sending(Socket& socket, Message& msgToSend);
	bool send_file(FileSystem::File& file, size_t fileSize, Socket& socket);

private:
	std::unordered_map<AddrInfo, Socket> sockTable;
	std::thread sendThread_;
	std::atomic<bool> stop_ = false;
	BQueue sendQ_;
};


/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You need to be careful using data members of this class
//   because each client handler thread gets a reference to this 
//   instance so you may get unwanted sharing.
// - that would mean that all ClientHandlers would need either copy or
//   move semantics.

class ClientHandler
{
public:
	ClientHandler(Async::BlockingQueue<HttpMessage>& msgQ, const std::string& path) : msgQ_(msgQ), rootFilePath(path) {}
	void operator()(Socket socket);

private:
	HttpMessage readMessage(Socket& socket);
	void handleFileMsg(Message& msg, const std::string& filname, Socket& socket);
	void readMsgBody(Message& msg, Socket& socket);
	bool readFile(const std::string& filename, const std::string& localSavePath, const std::string& category,
		std::string& errorMsg, size_t fileSize, Socket& socket);
	void readFromSockToFile(Socket& socket, FileSystem::File& file, size_t fileSize, bool dumpPacket);
	bool isLegalCategory(const std::string& category);
private:
	std::string rootFilePath;
	bool connectionClosed_;
	Async::BlockingQueue<Message>& msgQ_; // the blocking queue of reciever, put framed messages in
};



/////////////////////////////////////////////////////////////////////////////
// Recvr class
// - accepts messages from MockChanel for consumption by client
//
class Recvr : public IRecvr
{
public:
	Recvr(const std::string& rootFilePath, size_t port, Socket::IpVer ipv = Socket::IP6);
	~Recvr();
	// retreive message from receive queue
	Message getMessage();
	// get or set receive queue
	BQueue& queue();
	// start socket for listening to incoming messages
	bool startServer();
	// stop listening for communication
	void stopServer();

private:
	SocketSystem sockSys;
	SocketListener sockLisnr;
	// each time a connection is established, 
	// a dedicated child thread is created and a clientHandler
	// is responsible for handling incoming communications
	ClientHandler clHandlr;
	std::string rootFilePath;
	BQueue recvQ_;
};