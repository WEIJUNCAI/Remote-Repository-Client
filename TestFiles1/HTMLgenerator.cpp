/////////////////////////////////////////////////////////////////////////////
//  HTMLgenerator.cpp - generate HTML file                                 //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////

#include "HTMLgenerator.h"

HTMLgenerator::HTMLgenerator() : head(nullptr), body(nullptr)
{}

void HTMLgenerator::generateHTML(std::istream & input, std::ostream & output, const std::string & style, const std::string& js, Element<std::string>* info)
{
	output << "\n<html>";
	if (head != nullptr)
		head->generateHead(input, output, style, js);
	if (body != nullptr)
		body->generateBody(input, output, info);
	output << "\n</html>";
}

void HTMLgenerator::ConfigureGenerator()
{
	if (head == nullptr)
		head = new HTMLhead();
	if (body == nullptr)
	{
		body = new HTMLbody();
		body->addBodyPart(new DependencyPart());
		body->addBodyPart(new CodePartCollapse());
	}
}

void HTMLgenerator::ConfigureGenerator(IHTMLhead * head_, IHTMLbody * body_)
{
	head = head_;
	body = body_;
}

HTMLgenerator::~HTMLgenerator()
{
	if (head != nullptr)
		delete head;
	if (body != nullptr)
		delete body;
}


#ifdef TESTGENERATOR

void main()
{
	HTMLgenerator generator;
	generator.ConfigureGenerator();
	Element<std::string> file;
	std::ofstream ofs("../HTMLtest/generate.htm", std::ofstream::out);
	file.name = "C:\\Users\\cgarr\\OneDrive\\Documents\\OOD\\P3\\HTMLtest\\Cpp11-BlockingQueue.cpp";
	file.children.getRef().insert("C:\\Users\\cgarr\\OneDrive\\Documents\\OOD\\P3\\HTMLtest\\Cpp11-BlockingQueue.h");
	file.data = "C:\\Users\\cgarr\\OneDrive\\Documents\\OOD\\P3\\HTMLtest";
	std::ifstream ifs("../HTMLtest/Cpp11-BlockingQueue.cpp", std::ifstream::in);
	std::cout << FileSystem::Directory::getCurrentDirectory() << std::endl;
	generator.generateHTML(ifs, ofs, "style.css", &file);

	system("pause");
}

#endif // TESTGENERATOR

