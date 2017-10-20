#pragma once


/////////////////////////////////////////////////////////////////////////////
//  HTMLparts.h - HTML genetator parts                                     //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the delaration of the implementors of
*   multiple IHTML part interfaces, which can be attached to HTMLgenerator
*/
/*
*   Build Process
*   -------------
*   - Required files: IHTML.h
*                     HTMLparts.h, HTMLparts.cpp
*                     FileSystem.h, FileSystem.cpp
*
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 Mar 2017
*/

#include <unordered_map>
#include <stack>
#include "IHTML.h"
#include "../FileSystem/FileSystem.h"
#include "../Parser/Parser.h"


class HTMLhead : public IHTMLhead
{
public:
	// application specific, generate html header
	void generateHead(std::istream& input, std::ostream& output, const std::string& style, const std::string& js) override;
};


class DependencyPart : public IHTMLbodypart
{
public:
	// application specific, generate dependency part
	void generateBodyPart(std::istream& input, std::ostream& output, Element<std::string>* info) override;
};


// no scope collapse function
class CodePart : public IHTMLbodypart
{
public:
	// application specific, generate source code part
	void generateBodyPart(std::istream& input, std::ostream& output, Element<std::string>* info) override;

private:
	// replace all markups with the escaped form
	void processMarkupChars(std::string& line);

};


// scope collapse functionality supported
class CodePartCollapse : public IHTMLbodypart
{
public:
	// application specific, generate source code part
	void generateBodyPart(std::istream& input, std::ostream& output, Element<std::string>* info) override;

private:
	void GenerateCollapse(std::istream & input, std::ostream & output);
	bool processNormalChar(char c, std::ostream & output, size_t& lineCount);
	bool processQuoteChar(char c, std::ostream & output, std::stack<char>& quoteStack);
	bool processScopeChar(char c, std::istream & input, std::ostream & output, std::stack<char>& quoteStack, size_t & lineCount);
};

class HTMLbody : public IHTMLbody
{
public:
	// generate the html body using its body generator parts
	void generateBody(std::istream& input, std::ostream& output, Element<std::string>* info) override;

	// add/remove body generator parts 
	void addBodyPart(IHTMLbodypart* body) override;
	void delBodyPart(IHTMLbodypart* body) override;
	void clearBodyparts();
	~HTMLbody();
private:
	std::vector<IHTMLbodypart*> bodyParts;
};