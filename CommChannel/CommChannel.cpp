/////////////////////////////////////////////////////////////////////////////
//  CommChannel.cpp - Communication channel for peer messaging             //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Jim Fawcett											   //
//				  Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////


#include <unordered_set>
#include "CommChannel.h"

using Show = Logging::StaticLogger<1>;
using namespace Utilities;

Sendr::Sendr() : sendQ_()
{
	sendThread_ = std::thread(&Sendr::send_all, this);
}

Sendr::~Sendr()
{
	//stop_.exchange(true);
	HttpMessage quitMsg;
	quitMsg.addAttribute(HttpMessage::Attribute("Command", "quit"));
	sendQ_.enQ(quitMsg);
	sendThread_.join();
}

void Sendr::postMessage(const Message& msg)
{
	sendQ_.enQ(msg);
}

bool Sendr::connectServer(const std::string & ip, size_t port)
{
	SocketConnecter connectr;
	if (!connectr.connect(ip, port))
		return false;
	sockTable[ip + ":" + Converter<size_t>::toString(port)] = std::move(connectr);
	return true;
}

BQueue& Sendr::queue() { return sendQ_; }

void Sendr::send_all()
{
	while (true)
	{
		auto msg = sendQ_.deQ();
		if (msg.findValue("Command") == "quit")
			break;
		send_message(msg);
	}
}

bool Sendr::send_message(Message & msgToSend)
{
	auto dest_addr_str = msgToSend.findValue("Destination");

	if (dest_addr_str == "")
	{
		Show::write("\n\n  Invalid message to send, dropped.");
		return false;
	}

	auto dest_addr = HttpMessage::parseAttribute(dest_addr_str);

	if (sockTable.find(dest_addr_str) == sockTable.end()) // not found socket connected to target destination
													  // create a new socket and connect
		if (!connectServer(dest_addr.first, Converter<size_t>::toValue(dest_addr.second)))
		{
			Show::write("\n\n  Cannot create socket for sending message.");
			return false;
		}

	Socket& sendSock = sockTable[dest_addr_str];

	std::string command;
	if ((command = msgToSend.findValue("Command")) == "File Upload")
		return handle_file_msg_sending(sendSock, msgToSend);
	
	if (!sendSock.sendString(msgToSend.toString()))
		Show::write("\n\n  Failed sending message.");
	return true;
}



bool Sendr::handle_file_msg_sending(Socket & sendSock, Message & msgToSend)
{
	bool isSuccessful = true;
	auto filePath = msgToSend.findValue("File Name");
	if (filePath == "")
	{
		Show::write("\n\n  Invalid message to send, dropped.");
		return false;
	}
	std::string fileName = FileSystem::Path::getName(filePath);
	msgToSend.removeAttribute("File Name");
	msgToSend.addAttribute(HttpMessage::Attribute("File Name", fileName));
	FileSystem::File file(filePath);
	FileSystem::FileInfo fi(filePath);
	size_t fileSize = fi.size();
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())  // if cannot open file locally, send a File ACK instead so peer does not wait for file blocks
	{
		msgToSend.removeAttribute("Command");
		msgToSend.addAttribute(HttpMessage::Attribute("Command", "FileAck"));
		msgToSend.addAttribute(HttpMessage::Attribute("File Send Result", "Failed"));
		msgToSend.addAttribute(HttpMessage::Attribute("Additional Message", "Cannot open file \"" + fileName + "\"."));
		Show::write("\n\n  Failed opening file \"" + filePath + "\" for sending.");
		isSuccessful = false;
	}
	else
	{
		msgToSend.addAttribute(HttpMessage::Attribute("File Send Result", "Successful"));
		msgToSend.addAttribute(HttpMessage::Attribute("Content Length", Converter<size_t>::toString(fileSize)));
	}
	auto msgStr = msgToSend.toString();
	if (!sendSock.sendString(msgStr))
		Show::write("\n\n  Failed sending message.");
	if (isSuccessful)
	{
		if (!send_file(file, fileSize, sendSock))
		{
			Show::write("\n\n  Error sending file " + filePath);
			return false;
		}
	}
	return true;
}



bool Sendr::send_file(FileSystem::File& file, size_t fileSize, Socket & socket)
{
	std::string sizeString = Converter<size_t>::toString(fileSize);
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	while (true)
	{
		//std::cout << "\n\n  Sending file....";
		FileSystem::Block blk = file.getBlock(BlockSize);
		if (blk.size() == 0)
			break;
		for (size_t i = 0; i < blk.size(); ++i)
			buffer[i] = blk[i];
		socket.send(blk.size(), buffer);
		if (!file.isGood())
			break;
	}
	file.close();
	return true;
}



Recvr::Recvr(const std::string& rootFilePath, size_t port, Socket::IpVer ipv) 
	: sockSys(), sockLisnr(port, ipv), recvQ_(), clHandlr(recvQ_, rootFilePath) {}

Recvr::~Recvr()
{
	stopServer();
}


Message Recvr::getMessage()
{
	return recvQ_.deQ();
}

BQueue& Recvr::queue()
{
	return recvQ_;
}




// <--------------------------------- reciever side methods ------------------------------->

bool Recvr::startServer()
{
	return sockLisnr.start(clHandlr);
}

void Recvr::stopServer()
{
	sockLisnr.stop();
}

HttpMessage ClientHandler::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;
	// read message attributes
	while (true)
	{
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1)
		{
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
			msg.addAttribute(attrib);
		}
		/////////////////////////////////////////////////////////////
		// Socket.sendString() by default will send an extra '\0' at the end of each string
		// read one more charactor to properly align incoming messages
		else 
		{ 
			socket.recvString('\0');
			break;
		}
	}
	// ************ an empty message will cause client handling thread to shut down**********//
	//  need to be determined if this is the desired behavior
	/////////////////////////////////////////////////////////////////////////////
	// If client is done, connection breaks and recvString returns empty string//
	if (msg.attributes().size() == 0)
	{
		connectionClosed_ = true;
		return msg;
	}
	////////////////////////////////////////////////////////////////////////////
	// read body if POST - all messages in this demo are POSTs
	if (msg.attributes()[0].first == "POST")
	{
		// is this a file message?
		if (msg.findValue("Command") == "File Upload")
		{
			std::string filename = msg.findValue("File Name");
			if (filename != "")
				handleFileMsg(msg, filename, socket);
			else
				readMsgBody(msg, socket);
		}
	}
	return msg;
}


void ClientHandler::handleFileMsg(Message & msg, const std::string& filename, Socket & socket)
{
	size_t contentSize;
	std::string sizeString = msg.findValue("Content Length");
	std::string category = msg.findValue("Category");
	std::string localSavePath = msg.findValue("Local Save Path");
	std::string errorMsg;
	if (sizeString != "")
		contentSize = Converter<size_t>::toValue(sizeString);
	else
		return;

	if (!readFile(filename, localSavePath, category, errorMsg, contentSize, socket))
	{
		msg.addAttribute(HttpMessage::Attribute("File Receive Result", "Failed"));
		msg.addAttribute(HttpMessage::Attribute("Additional Message", errorMsg));
	}
	else
		msg.addAttribute(HttpMessage::Attribute("File Receive Result", "Successful"));
	// construct message body
	msg.removeAttribute("Content Length");
	std::string bodyString = "<file>" + filename + "</file>";
	sizeString = Converter<size_t>::toString(bodyString.size());
	msg.addAttribute(HttpMessage::Attribute("Content Length", sizeString));
	msg.addBody(bodyString);
}



void ClientHandler::readMsgBody(Message & msg, Socket & socket)
{
	// read message body
	size_t numBytes = 0;
	size_t pos = msg.findAttribute("Content Length");
	if (pos < msg.attributes().size())
	{
		numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
		Socket::byte* buffer = new Socket::byte[numBytes + 1];
		socket.recv(numBytes, buffer);
		buffer[numBytes] = '\0';
		std::string msgBody(buffer);
		msg.addBody(msgBody);
		delete[] buffer;
	}
}


//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string& filename, const std::string& localSavePath, const std::string& category, std::string& errorMsg, size_t fileSize, Socket& socket)
{
	bool isSuccessful = true;
	std::string fqname;
	std::string parentDir;

	// if a lcoal save path is specified by user, use this path
	if (localSavePath != "")
	{
		parentDir = localSavePath;
		fqname = localSavePath + "\\" + filename;
	}
	else
	{
		if (!isLegalCategory(category))
		{
			errorMsg = "Illegal category \"" + category + "\".";
			isSuccessful = false;
		}

		parentDir = rootFilePath + "\\" + category;
		fqname = parentDir + "\\" + filename;
	}


	FileSystem::File file(fqname);

	if(!FileSystem::Directory::exists(parentDir))
		if (FileSystem::Directory::create(parentDir)) // specified category folder does not exist and cannot be created
		{
			errorMsg = "Cannot create category directory \"" + category + "\".";
			Show::write("\n\n  " + errorMsg);
			isSuccessful = false;
		}
	if (isSuccessful)
	{
		file.open(FileSystem::File::out, FileSystem::File::binary);
		if (!file.isGood())
		{
			/*
			 * The client will continue to send bytes,
			 * but if the file can't be opened, then the server
			 * collect and dump them as it should.
			 */
			errorMsg = "Cannot open file \"" + filename + "\".";
			Show::write("\n\n  can't open file " + fqname);
			isSuccessful = false;
		}
	}
	readFromSockToFile(socket, file, fileSize, !isSuccessful);
	file.close();
	return isSuccessful;
}



void ClientHandler::readFromSockToFile(Socket & socket, FileSystem::File & file, size_t fileSize, bool dumpPacket)
{
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];

	size_t bytesToRead;
	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		if (!dumpPacket)
		{
			FileSystem::Block blk;
			for (size_t i = 0; i < bytesToRead; ++i)
				blk.push_back(buffer[i]);

			file.putBlock(blk);
		}
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
}


bool ClientHandler::isLegalCategory(const std::string & category)
{
	std::unordered_set<char> reservedChars({ '<', '>', ':', '\"', '/', '\\', '|', '?', '*' });
	for (auto c : category)
		if (reservedChars.find(c) != reservedChars.end())
			return false;
	return true;
}


//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
	/*
	* There is a potential race condition due to the use of connectionClosed_.
	* If two clients are sending files at the same time they may make changes
	* to this member in ways that are incompatible with one another.  This
	* race is relatively benign in that it simply causes the readMessage to
	* be called one extra time.
	*
	* The race is easy to fix by changing the socket listener to pass in a
	* copy of the clienthandler to the clienthandling thread it created.
	* I've briefly tested this and it seems to work.  However, I did not want
	* to change the socket classes this late in your project cycle so I didn't
	* attempt to fix this.
	*/
	while (true)
	{
		HttpMessage msg = readMessage(socket);
		if (connectionClosed_ || msg.bodyString() == "quit")
		{
			Show::write("\n\n  clienthandler thread is terminating");
			break;
		}
		msgQ_.enQ(msg);
	}
}

#ifdef TEST_COMMCHANNEL

// mock server 
void main()
{

	Recvr serverRecvr("../testChannel", 8080);
	Sendr serverSender;
	if (!serverRecvr.startServer())
		Show::write("\n\n Failed starting reciever.");
	//std::this_thread::sleep_for(std::chrono::seconds(10));
	serverSender.connectServer("localhost", 9090);

	HttpMessage msg;
	std::string dest = "localhost:9090";
	std::string src = "localhost:8080";
	msg.addAttribute(HttpMessage::attribute("POST", "Message"));
	msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
	msg.addAttribute(HttpMessage::Attribute("Command", "File Upload"));
	msg.addAttribute(HttpMessage::Attribute("Category", "Headers"));
	msg.addAttribute(HttpMessage::Attribute("File Name", "C:\\Users\\cgarr\\Desktop\\CodePublisher.h"));
	msg.addAttribute(HttpMessage::Attribute("Content Length", "1925"));
	msg.addAttribute(HttpMessage::parseAttribute("Destination:" + dest));
	msg.addAttribute(HttpMessage::parseAttribute("Source:" + src));
	serverSender.postMessage(msg);
	system("pause");
}

#endif // TEST_COMMCHANNEL