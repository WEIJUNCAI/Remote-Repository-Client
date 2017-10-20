#pragma once

/////////////////////////////////////////////////////////////////////////////
//  CLIshim.h - Transition layer between C# GUI and C++                    //
//  Language:     C++/CLI, VS 2015                                         //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the implementaion of CLI shim class which enables 
*	the intercommunication between native C++ code and managed C++ code
*
*/
/*
*   Build Process
*   -------------
*   - Required files: CLIshim.h, CLIshim.cpp
*                     CommChannel.h. CommChannel.cpp
*
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 April 2017
*
*   Note:                                 
*	 - build as a dll so C# can load     
*	 - link to MockChannel static library
*/



#include <string>
#include "../CommChannel/IComm.h"

using namespace System;

public ref class myShim
{
public:
	myShim(String^ rootDir, uint32_t serverPort, uint16_t ipVer);

	// start listing server
	bool start_server();
	// connect to a server
	bool connect_to_server(String^ ip, uint32_t port);
	// get message from communication channel
	void PostMessage(String^ msg);
	// put message to communication channel to send
	String^ GetMessage();
	// convert between native C++ string and managed String
	String^ stdStrToSysStr(const std::string& str);
	std::string sysStrToStdStr(String^ str);

private:
	ISendr* pISendr;
	IRecvr* pIRecvr;
	IChannel* pChan;

};
