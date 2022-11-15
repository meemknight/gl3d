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
	std::cout << err << "\n";
}

std::string gl3d::defaultReadEntireFile(const char* fileName, bool &couldNotOpen, void *userData)
{
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
}
