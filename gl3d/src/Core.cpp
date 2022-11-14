#include "Core.h"
#include <stdio.h>
#include <Windows.h>
#include <signal.h>
#include <sstream>
#include "ErrorReporting.h"

#undef min
#undef max

namespace gl3d 
{

	void assertFunc(const char *expression,
		const char *file_name,
		unsigned const line_number,
		const char *comment)
	{
	
		char c[1024] = {};
	
		sprintf(c,
			"Assertion failed\n\n"
			"File:\n"
			"%s\n\n"
			"Line:\n"
			"%u\n\n"
			"Expresion:\n"
			"%s\n\n"
			"Comment:\n"
			"%s",
			file_name,
			line_number,
			expression,
			comment
		);
	
		int const action = MessageBox(0, c, "Platform Layer", MB_TASKMODAL
			| MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);
	
		switch (action)
		{
			case IDABORT: // Abort the program:
			{
				raise(SIGABRT);
	
				// We won't usually get here, but it's possible that a user-registered
				// abort handler returns, so exit the program immediately.  Note that
				// even though we are "aborting," we do not call abort() because we do
				// not want to invoke Watson (the user has already had an opportunity
				// to debug the error and chose not to).
				_exit(3);
			}
			case IDRETRY: // Break into the debugger then return control to caller
			{
				__debugbreak();
				return;
			}
			case IDIGNORE: // Return control to caller
			{
				return;
			}
			default: // This should not happen; treat as fatal error:
			{
				abort();
			}
		}
		
	
	}

	//https://learnopengl.com/In-Practice/Debugging
	//todo probably remove iostream when error api is ready
	void GLAPIENTRY glDebugOutput(GLenum source,
								GLenum type,
								unsigned int id,
								GLenum severity,
								GLsizei length,
								const char *message,
								const void *userParam)
	{
		ErrorReporter *errorReporter = (ErrorReporter*)userParam;

		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204
			|| id == 131222
			|| id == 131140 //that dittering thing
			) return;
		if (type == GL_DEBUG_TYPE_PERFORMANCE) return;

		std::stringstream error;

		error << "---------------" << std::endl;
		error << "Debug message (" << id << "): " << message << std::endl;
	
		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             error << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   error << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: error << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     error << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     error << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           error << "Source: Other"; break;
		} error << std::endl;
	
		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               error << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: error << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  error << "Type: Undefined Behaviour"; break;
			case GL_DEBUG_TYPE_PORTABILITY:         error << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         error << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              error << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          error << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           error << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               error << "Type: Other"; break;
		} error << std::endl;
	
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         error << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       error << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          error << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: error << "Severity: notification"; break;
		};

		errorReporter->callErrorCallback(error.str());
	}


};

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE  
#define JSON_NOEXCEPTION
#include "json.h"
#include "stb_image_write.h"
#include "tiny_gltf.h"
