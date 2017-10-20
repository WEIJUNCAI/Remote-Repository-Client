#pragma once
///////////////////////////////////////////////////////////////////////
// CLIShim.h - C++\CLI layer is a proxy for calls into MockChannel   //
//                                                                   //
// Note:                                                             //
//   - build as a dll so C# can load                                 //
//   - link to MockChannel static library                            //
//                                                                   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2017         //
///////////////////////////////////////////////////////////////////////


#include <string>
#include "../CommChannel/IComm.h"

using namespace System;

public ref class myShim
{
public:
	myShim(String^ rootDir, uint32_t serverPort, uint16_t ipVer);

	bool start_server();
	bool connect_to_server(String^ ip, uint32_t port);

	void PostMessage(String^ msg);
	String^ GetMessage();
	String^ stdStrToSysStr(const std::string& str);
	std::string sysStrToStdStr(String^ str);

private:
	ISendr* pISendr;
	IRecvr* pIRecvr;
	IChannel* pChan;

};
