#include "ErrorReporting.h"

void gl3d::ErrorReporter::callErrorCallback(std::string s)
{
	if (!s.empty() && currentErrorCallback != nullptr)
	{
		currentErrorCallback(s);
	}
}

void gl3d::defaultErrorCallback(std::string err)
{
	std::cout << err << "\n";
}
