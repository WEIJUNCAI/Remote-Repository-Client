#include "CLIshim.h"



myShim::myShim(String^ rootDir, uint32_t serverPort, uint16_t ipVer)
{
	ObjectFactory factory;
	pISendr = factory.createSendr();
	pIRecvr = factory.createRecvr(sysStrToStdStr(rootDir), serverPort, ipVer);
	pChan = factory.createChannel(pISendr, pIRecvr);
}

bool myShim::start_server()
{
	
	return pChan->startServer();
}

bool myShim::connect_to_server(String ^ ip, uint32_t port)
{
	
	return pChan->connectServer(sysStrToStdStr(ip), port);
}

// --<------------------ GUI --------------------> <---------------- shim -------------------> <-receiver->
// (Message handling -> HttpMessageCS -> StringCS) -> StringCS -> stringSTD -> HttpMesaageSTD -------> POST 
void myShim::PostMessage(String ^ msg)
{
	std::string msgStr_std = sysStrToStdStr(msg);
	HttpMessage msg_std = HttpMessage::parseMessage(msgStr_std);
	pISendr->postMessage(msg_std);
}

// <-receiver-> <---------------- shim -------------------> <------------------- GUI --------------------->
// GET -------> HttpeMessageSTD -> stringSTD -> StringCS -> (StringCS -> HttpMessageCS -> Message handling)
String ^ myShim::GetMessage()
{
	HttpMessage& msg_std = pIRecvr->getMessage();
	std::string msgStr_std = msg_std.toString();
	return stdStrToSysStr(msgStr_std);
}

String^ myShim::stdStrToSysStr(const std::string& str)
{
	return gcnew String(str.c_str());
}

std::string myShim::sysStrToStdStr(String^ str)
{
	std::string temp;
	for (int i = 0; i < str->Length; ++i)
		temp += char(str[i]);
	return temp;
}