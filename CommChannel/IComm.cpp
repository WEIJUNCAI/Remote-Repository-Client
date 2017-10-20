#define IN_DLL

/////////////////////////////////////////////////////////////////////////////
//  IComm.cpp - interface for sender and receiver                          //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Jim Fawcett											   //
//				  Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////

#include "CommChannel.h"
#include "IComm.h"

// channel used by upper level layers to communicate
// send and receive using sender and receiver
class Channel : public IChannel
{
public:
	Channel(ISendr* pSendr, IRecvr* pRecvr);
	bool startServer() override;
	bool connectServer(const std::string& ip, size_t port) override;

private:
	std::thread thread_;
	ISendr* pISendr_;
	IRecvr* pIRecvr_;
	bool stop_ = false;
};


Channel::Channel(ISendr* pSendr, IRecvr* pRecvr) : pISendr_(pSendr), pIRecvr_(pRecvr) {}

bool Channel::startServer()
{
	Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);
	if (pRecvr == nullptr)
	{
		std::cout << "\n\n Failed to start listener." << std::endl;
		return false;
	}
	return pRecvr->startServer();
}

bool Channel::connectServer(const std::string & ip, size_t port)
{
	Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
	if (pSendr == nullptr)
	{
		std::cout << "\n\n Failed to start sender." << std::endl;
		return false;
	}
	return pSendr->connectServer(ip, port);
}

ISendr* ObjectFactory::createSendr() 
{ 
	return new Sendr; 
}

IRecvr* ObjectFactory::createRecvr(const std::string& rootFilePath, size_t port, size_t ipv) 
{
	return new Recvr(rootFilePath, port, (ipv == 4) ? Socket::IpVer::IP4 : Socket::IpVer::IP6);
}

IChannel* ObjectFactory::createChannel(ISendr* pISendr, IRecvr* pIRecvr)
{
	return new Channel(pISendr, pIRecvr);
}