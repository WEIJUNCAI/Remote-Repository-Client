/////////////////////////////////////////////////////////////////////////////
//  HTMLparts.cpp - HTML genetator parts                                   //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////


#include <queue>
#include <set>
#include "HTMLparts.h"

void HTMLhead::generateHead(std::istream & input, std::ostream & output, const std::string & style, const std::string& js)
{
	output << "\n <head>";
	output << "\n     <link rel=\"stylesheet\" href=";
	output << "\"" << style << "\"" << ">";
	output << "<script src=\"" << js <<"\"></script>";
	output << "\n </head>";
}


void DependencyPart::generateBodyPart(std::istream & input, std::ostream & output, Element<std::string>* info)
{
	output << "\n    <h3>" << FileSystem::Path::getName(info->name) << "</h3>";
	output << "\n    <hr />";
	output << "\n    <div class=\"indent\">";
	output << "\n        <h4>Dependencies:</h4>";
	for (auto & childFullPath : info->children.getRef())
	{
		std::string filename = FileSystem::Path::getName(childFullPath);
		output << "\n         <p><a href=\"" << (info->data).getValue() + '\\' + filename << ".htm\">" << filename << "</a></p>"; // publish path is in data part
	}
	output << "\n    </div>";
	output << "\n    <hr />";
}

void CodePart::generateBodyPart(std::istream & input, std::ostream & output, Element<std::string>* info)
{
	const unsigned int buffersize = 256;
	char buffer[buffersize];

	output << "\n  <pre>\n";
	while (input.getline(buffer, buffersize).good())
	{
		std::string currLine(buffer);
		processMarkupChars(currLine);
		output << currLine << '\n';
	}
	output << "\n  </pre>";
	
}



void CodePart::processMarkupChars(std::string & line)
{
	size_t markupCharPos_l;
	size_t markupCharPos_r;
	while ((markupCharPos_l = line.find('<')) != (markupCharPos_r = line.find('>')))
	{
		if (markupCharPos_l != std::string::npos)
		{
			line.erase(markupCharPos_l, 1);
			line.insert(markupCharPos_l, "&lt;");
		}
		if (markupCharPos_r != std::string::npos)
		{
			// found '<', so the length of line changed 
			if (markupCharPos_l != std::string::npos)
			{
				line.erase(markupCharPos_r + 3, 1);
				line.insert(markupCharPos_r + 3, "&gt;");
			}
			else
			{
				line.erase(markupCharPos_r, 1);
				line.insert(markupCharPos_r, "&gt;");
			}
		}
	}
}


void CodePartCollapse::generateBodyPart(std::istream & input, std::ostream & output, Element<std::string>* info)
{
	output << "\n  <pre>\n";
	GenerateCollapse(input, output);
	output << "\n  </pre>\n";
}

void CodePartCollapse::GenerateCollapse(std::istream & input, std::ostream & output)
{
	std::stack<char> quoteStack;
	char c;
	size_t lineCount = 1;
	while (input.get(c).good())
	{
		if (processNormalChar(c, output, lineCount))
			continue;
		if (processQuoteChar(c, output, quoteStack))
			continue;
		if (!processScopeChar(c, input, output, quoteStack, lineCount))
			throw std::exception("\n\nunknown character\n");
	}
}

bool CodePartCollapse::processNormalChar(char c, std::ostream & output, size_t & lineCount)
{
	if (c != '{' && c != '}' && c != '\'' && c != '\"')
	{
		if (c == '\n')
			++lineCount;
		if (c == '<')
			output << "&lt;";
		else if (c == '>')
			output << "&gt;";
		else
			output << c;
		return true;
	}
	return false;
}

bool CodePartCollapse::processQuoteChar(char c, std::ostream & output, std::stack<char>& quoteStack)
{
	if (c == '\'' || c == '\"')
	{
		if (quoteStack.size() > 0 && c == quoteStack.top())
			quoteStack.pop();
		else
			quoteStack.push(c);
		return true;
	}
	return false;
}

bool CodePartCollapse::processScopeChar(char c, std::istream & input, std::ostream & output, std::stack<char>& quoteStack, size_t & lineCount)
{
	if (quoteStack.size() == 0 && c == '{')
	{
		std::string uniqueId, btnId;
		std::ostringstream oss, oss2;
		oss << "body" << lineCount;
		uniqueId = oss.str();
		oss2 << "btn" << lineCount;
		btnId = oss2.str();
		output << "<button id=\"" << btnId << "\" onclick=\"collapse(\'" << uniqueId << "\', \'" << btnId << "\')\">" << "-" << "</button>";
		output << "<span id=\"" << uniqueId << "\">{";
		return true;
	}
	if(quoteStack.size() == 0 && c == '}')
	{
		char c2;
		if (input.get(c2).good())
		{
			if (c2 == ';')
				output << "};</span>";
			else
				output << "}</span>" << c2;
		}
		else
			throw std::exception("\n\nError reading source file\n");;
		return true;
	}
	return false;
}




void HTMLbody::generateBody(std::istream & input, std::ostream & output, Element<std::string>* info)
{
	output << "\n <body>";
	for (auto htmlPart : bodyParts)
	{
		htmlPart->generateBodyPart(input, output, info);
	}
	output << "\n </body>";
}

void HTMLbody::addBodyPart(IHTMLbodypart * body)
{
	if (body != nullptr)
		bodyParts.push_back(body);
}

void HTMLbody::delBodyPart(IHTMLbodypart * body)
{
	bodyParts.pop_back();
}

void HTMLbody::clearBodyparts()
{
	bodyParts.clear();
}

HTMLbody::~HTMLbody()
{
	for (auto part : bodyParts)
	{
		delete part;
	}
}

#ifdef TEST_HTMLPARTS

void main()
{
	HTMLhead head;
	HTMLbody body;
	DependencyPart* depPart = new DependencyPart();
	CodePartCollapse* cdPart = new CodePartCollapse();
	Element<std::string> file;
	std::ofstream ofs("../HTMLtest/generate.htm", std::ofstream::out);
	file.name = "C:\\Users\\cgarr\\OneDrive\\Documents\\OOD\\P3\\HTMLtest\\Cpp11-BlockingQueue.cpp";
	file.children.getRef().insert("C:\\Users\\cgarr\\OneDrive\\Documents\\OOD\\P3\\HTMLtest\\Cpp11-BlockingQueue.h");
	file.data = "C:\\Users\\cgarr\\OneDrive\\Documents\\OOD\\P3\\HTMLtest";
	std::ifstream ifs("../HTMLtest/Cpp11-BlockingQueue.cpp", std::ifstream::in);
	std::cout << FileSystem::Directory::getCurrentDirectory() << std::endl;
	head.generateHead(ifs, ofs/*std::cout*/, "style.css", "collapse.js");
	body.addBodyPart(depPart);
	body.addBodyPart(cdPart);
	body.generateBody(ifs, ofs/*std::cout*/, &file);
	ifs.close();
	system("pause");
}


#endif // TEST_HTMLPARTS

