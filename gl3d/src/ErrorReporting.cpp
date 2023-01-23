#include "ErrorReporting.h"

void gl3d::ErrorReporter::callErrorCallback(std::string s)
{
	if (!s.empty() && currentErrorCallback != nullptr)
	{
		currentErrorCallback(s, userData);
	}
}


void gl3d::defaultErrorCallback(std::string err, void *userData)
{
#if GL3D_REMOVE_IOSTREAM == 0
	std::cout << err << "\n";
#endif
}

std::string gl3d::defaultReadEntireFile(const char* fileName, bool &couldNotOpen, void *userData)
{
#if GL3D_REMOVE_FSTREAM == 0

	std::ifstream file;
	file.open(fileName);

	if (!file.is_open())
	{
		couldNotOpen = true;
		return "";
}

	couldNotOpen = false;

	size_t size = 0;
	file.seekg(0, file.end);
	size = file.tellg();
	file.seekg(0, file.beg);

	std::string ret;
	ret.reserve(size + 1);

	ret.assign((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	file.close();

	return ret;

#else

	return "";

#endif
}

std::vector<char> gl3d::defaultReadEntireFileBinary(const char *fileName, bool &couldNotOpen, void *userData)
{
#if GL3D_REMOVE_FSTREAM == 0
	std::ifstream file;
	file.open(fileName, std::ios::binary);

	if (!file.is_open())
	{
		couldNotOpen = true;
		return {};
	}

	couldNotOpen = false;

	size_t size = 0;
	file.seekg(0, file.end);
	size = file.tellg();
	file.seekg(0, file.beg);

	std::vector<char> ret;
	ret.reserve(size + 1);

	ret.assign((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	file.close();

	return ret;
#else
	return {};
#endif
}

bool gl3d::defaultFileExists(const char *fileName, void *userData)
{

#if GL3D_REMOVE_FSTREAM == 0
	std::ifstream file;
	file.open(fileName);

	if (!file.is_open())
	{
		return false;
	}
	else
	{
		return true;
	}
#else
	return false;
#endif
	
}
