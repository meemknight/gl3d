#pragma once

#include <string>
#include <iostream> //you can remove this if neded to. It is just used for the default errorcallback

namespace gl3d
{

	void defaultErrorCallback(std::string err, void *userData);

	using ErrorCallback_t = decltype(defaultErrorCallback);
	
	struct ErrorReporter
	{
		ErrorCallback_t *currentErrorCallback = defaultErrorCallback;
		void *userData = nullptr;

		void callErrorCallback(std::string s);
	};

};