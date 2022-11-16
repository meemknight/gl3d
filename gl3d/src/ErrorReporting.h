#pragma once

#include <string>
#include <vector>

#include <iostream> //you can remove this if neded to. It is just used for the default errorcallback
#include <fstream> //you can remove this if neded to. It is just used for the default file callback

namespace gl3d
{

	void defaultErrorCallback(std::string err, void *userData);
	std::string defaultReadEntireFile(const char *fileName, bool &couldNotOpen, void *userData);
	std::vector<char> defaultReadEntireFileBinary(const char *fileName, bool &couldNotOpen, void *userData);

	using ErrorCallback_t = decltype(defaultErrorCallback);
	using ReadEntireFile_t = decltype(defaultReadEntireFile);
	using ReadEntireFileBinart_t = decltype(defaultReadEntireFileBinary);
	
	struct ErrorReporter
	{
		ErrorCallback_t *currentErrorCallback = defaultErrorCallback;
		void *userData = nullptr;

		void callErrorCallback(std::string s);
	};



	struct FileOpener
	{
		ReadEntireFile_t *readEntireFileCallback = defaultReadEntireFile;
		ReadEntireFileBinart_t *readEntireFileBinaryCallback = defaultReadEntireFileBinary;
		void *userData = nullptr;

		std::string operator()(const char *fileName, bool &couldNotOpen)
		{
			return readEntireFileCallback(fileName, couldNotOpen, userData);
		}

		std::vector<char> binary(const char *fileName, bool &couldNotOpen)
		{
			return readEntireFileBinaryCallback(fileName, couldNotOpen, userData);
		}

	};

};