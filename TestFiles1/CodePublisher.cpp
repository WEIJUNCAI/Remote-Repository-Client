/////////////////////////////////////////////////////////////////////////////
//  CodePublisher.cpp - publish source code in HTMLs                       //
//  Language:     C++, VS 2015                                             //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////

#include "CodePublisher.h"

void CodePublisher::PublishFile(Element<std::string>& file)
{
	std::ifstream ifs(file.name, std::ifstream::in);
	std::ostringstream oss;
	if (ifs.good())
	{
		Element<std::string> file_ = file;
		file_.data = FileSystem::Path::getFullFileSpec(pubPath);
		std::string htmlFullPath = pubPath + '\\' + FileSystem::Path::getName(file.name) + ".htm";
		configureHTMLgenerator();
		htmlGenerator.generateHTML(ifs, oss, FileSystem::Path::getFullFileSpec(stylePath), FileSystem::Path::getFullFileSpec(jsPath), &file_);
		std::ofstream ofs(htmlFullPath, std::ofstream::out);
		if (ofs.good())
		{
			ofs << oss.str();
			ofs.close();
			ifs.close();
			publishedFiles.push_back(htmlFullPath);
		}
		else
			throw std::exception("/nUnable to open file to write HTML");
	}
	else
		throw std::exception("/nUnable to open source file");
}

void CodePublisher::PublishAll()
{
	for (auto & pair : *srcFileInfo)
	{
		PublishFile(pair.second);
	}
}

void CodePublisher::setFileInfo(NoSqlDb<std::string>* fileDB)
{
	srcFileInfo = fileDB;
}

void CodePublisher::setPublishPath(const std::string & path)
{
	pubPath = path;
}

std::string CodePublisher::getPublishPath()
{
	return pubPath;
}



void CodePublisher::setStylePath(const std::string& path)
{
	stylePath = path;
}

std::string CodePublisher::getStylePath()
{
	return stylePath;
}

void CodePublisher::setJavaScriptPath(const std::string & path)
{
	jsPath = path;
}

std::string CodePublisher::getJavaScriptPath()
{
	return jsPath;
}

void CodePublisher::configureHTMLgenerator()
{
	htmlGenerator.ConfigureGenerator();
}

std::vector<std::string>& CodePublisher::getHTMLfiles()
{
	return publishedFiles;
}


#ifdef TEST_PUBLISHER

void main()
{
	CodePublisher publisher;
	publisher.configureHTMLgenerator();
	publisher.setPublishPath("C:\\Users\\cgarr\\Desktop");
	publisher.setStylePath("C:\\Users\\cgarr\\Desktop\\style.css");

	Element<std::string> fileInfo;

	fileInfo.name = "C:\\Users\\cgarr\\Desktop\\files\\Grandparent.h";
	fileInfo.children.getRef().insert("C:\\Users\\cgarr\\Desktop\\files\\Parent.h");
	publisher.PublishFile(fileInfo);

}

#endif // TEST_PUBLISHER

