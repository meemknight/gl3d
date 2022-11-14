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
