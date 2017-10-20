#pragma once

/////////////////////////////////////////////////////////////////////////////
//  HTMLgenerator.h - generate HTML file                                   //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the delaration of HTMLgenerator class, which
*   can generate HTML using the composing HTML parts 
*/
/*
*   Build Process
*   -------------
*   - Required files: IHTML.h
*                     HTMLparts.h, HTMLparts.cpp
*                     HTMLgenerator.h, HTMLgenerator.cpp
*
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 Mar 2017
*/

#include <iostream>
#include <string>
#include <vector>
#include "IHTML.h"
#include "HTMLparts.h"


class HTMLgenerator : public IHTMLgenerator
{
public:
	HTMLgenerator();

	// interface implementation, generate complete html to specified output
	void generateHTML(std::istream& input, std::ostream& output, const std::string& style, const std::string& js, Element<std::string>* info) override;
	
	// attach specified html generator parts to generator
	void ConfigureGenerator();
	void ConfigureGenerator(IHTMLhead* head, IHTMLbody* body);
	~HTMLgenerator();
private:
	IHTMLhead* head;
	IHTMLbody* body;
};