#pragma once


/////////////////////////////////////////////////////////////////////////////
//  IHTML.h - HTML genetator interfaces                                    //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the delaration of the HTML generator interfaces
*   should be implemented to provide application side solutions
*   all html generator parts are incrementally assembled using base pointers
*/
/*
*   Build Process
*   -------------
*   - Required files: IHTML.h
*                     Element.h
*                     
*
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 Mar 2017
*/

#include <iostream>
#include <string>
#include <vector>
#include "../Element/Element.h"

struct IHTMLhead
{
	virtual void generateHead(std::istream& input, std::ostream& output,  const std::string& style, const std::string& js) = 0;
};


struct IHTMLbodypart
{
	// publish path is included in info
	virtual void generateBodyPart(std::istream& input, std::ostream& output, Element<std::string>* info) = 0;
};

struct IHTMLbody
{
	virtual void generateBody(std::istream& input, std::ostream& output, Element<std::string>* info) = 0;
	virtual void addBodyPart(IHTMLbodypart* body) = 0;
	virtual void delBodyPart(IHTMLbodypart* body) = 0;
};

struct IHTMLgenerator
{
	virtual void generateHTML(std::istream& input, std::ostream& output, const std::string& style, const std::string& js, Element<std::string>* info) = 0;
};