#pragma once


/////////////////////////////////////////////////////////////////////////////
//  CodePublisher.h - publish source code in HTMLs                         //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the delaration of the CodePublisher class,
*   which can publish source code into the form of html files
*/
/*
*   Build Process
*   -------------
*   - Required files: HTMLgenerator.h, HTMLgenerator.cpp
*                     NoSqlDb.h
*				      CodePublisher.h, CodePublisher.cpp
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 Mar 2017
*/


#include "../NoSqlDb/NoSqlDb.h"
#include "../HTMLgenerator/HTMLgenerator.h"

class CodePublisher
{
public:
	// publish source file(s)
	void PublishFile(Element<std::string>& file);
	void PublishAll();
	
	// get/set various info to publish
	void setFileInfo(NoSqlDb<std::string>* fileDB);
	void setPublishPath(const std::string & path);
	std::string getPublishPath();
	void setStylePath(const std::string& path);
	std::string getStylePath();
	void setJavaScriptPath(const std::string& path);
	std::string getJavaScriptPath();
	// assemble HTML generator units
	void configureHTMLgenerator();
	// retrieve the name of all published HTML files
	std::vector<std::string>& getHTMLfiles();

private:
	NoSqlDb<std::string>* srcFileInfo;
	std::vector<std::string> publishedFiles;
	HTMLgenerator htmlGenerator;
	std::string pubPath;
	std::string stylePath;
	std::string jsPath;
};
