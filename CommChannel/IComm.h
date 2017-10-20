#ifndef CHANNEL_H
#define CHANNEL_H

/////////////////////////////////////////////////////////////////////////////
//  IComm.h - interface for sender and receiver                            //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Jim Fawcett											   //
//				  Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the implementaion of sender, reciever and channel interface
*	as well as channel that uese sender and receiver.
*
*/
/*
*   Build Process
*   -------------
*   - Required files: IComm.h, IComm.cpp
*
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 April 2017
*/



#ifdef IN_DLL
#define DLL_DECL __declspec(dllexport)
#else
#define DLL_DECL __declspec(dllimport)
#endif

#include <string>
#include "../HttpMessage/HttpMessage.h"

using Message = HttpMessage;

// sender interface
struct ISendr
{
	virtual void postMessage(const HttpMessage& msg) = 0;
};

// receiver interface
struct IRecvr
{
	virtual HttpMessage getMessage() = 0;
};

// channel interface
struct IChannel
{
public:

	virtual bool startServer() = 0;
	virtual bool connectServer(const std::string& ip, size_t port) = 0;
	
	//virtual void stop() = 0;
};

// factory for creating channel
extern "C" {
	struct ObjectFactory
	{
		DLL_DECL ISendr* createSendr();
		DLL_DECL IRecvr* createRecvr(const std::string& rootFilePath, size_t port, size_t ipv);
		DLL_DECL IChannel* createChannel(ISendr* pISendr, IRecvr* pIRecvr);
	};
}

#endif


