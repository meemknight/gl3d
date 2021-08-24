////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2021-08-24
////////////////////////////////////////////////

#include "gl3d.h"

////////////////////////////////////////////////
//Core.cpp
////////////////////////////////////////////////
#pragma region Core

#include <stdio.h>
#include <Windows.h>
#include <signal.h>
#include <iostream>

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
	//todo probably remove iostream
	void GLAPIENTRY glDebugOutput(GLenum source,
								GLenum type,
								unsigned int id,
								GLenum severity,
								GLsizei length,
								const char *message,
								const void *userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204
			|| id == 131222
			) return;
		if (type == GL_DEBUG_TYPE_PERFORMANCE) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;
	
		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;
	
		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
			case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;
	
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;

	}


};

#pragma endregion


////////////////////////////////////////////////
//Texture.cpp
////////////////////////////////////////////////
#pragma region Texture

#include <stb_image.h>
#include <iostream>
#include <glm\vec3.hpp>

#include <algorithm>

namespace gl3d
{

	void GpuTexture::loadTextureFromFile(const char *file, int quality, int channels)
	{

		int w, h, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load(file, &w, &h, &nrChannels, channels);

		if (!data)
		{
			//todo err messages
			std::cout << "err: " << file << "\n";
			id = 0;
		}
		else
		{
			loadTextureFromMemory(data, w, h, channels, quality);
			stbi_image_free(data);
		}


	}



	void GpuTexture::loadTextureFromMemory(void *data, int w, int h, int chanels,
		int quality)
	{

		gl3dAssertComment(chanels == 1 || chanels == 3 || chanels == 4, "invalid chanel number");

		GLenum format = GL_RGBA;
		GLenum internalFormat = GL_RGBA8;

		if(chanels == 3)
		{
			format = GL_RGB;
			internalFormat = GL_RGB8;

		}else if(chanels == 1)
		{
			format = GL_RED;
			internalFormat = GL_R8;
		}

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		if (quality < 0)
			return;

		setTextureQuality(quality);
		glGenerateMipmap(GL_TEXTURE_2D);

	}

	

	int GpuTexture::loadTextureFromFileAndCheckAlpha(const char* file, int quality, int channels)
	{
		int w, h, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(file, &w, &h, &nrChannels, channels);

		int alphaData = 0;

		if (!data)
		{
			//todo err messages
			std::cout << "err: " << file << "\n";
			id = 0;
		}
		else
		{
			//first look if there is alpha data in the file or if it is wanted at all
			if(nrChannels != 4 && channels != 4) 
			{
				alphaData = 0;
			}
			else
			{
				for (int i = 0; i < w * h; i++)
				{
					if (data[4 * i + 3] != UCHAR_MAX)
					{
						alphaData = 1;
						break;
					}
				}
			}

			// if there is no alpha channel in file clamp channels to max 3
			if (!alphaData && channels == 4)
			{
				int writePos = 0;
				int readPos = 0;

				for (int i = 0; i < w * h; i++)
				{
					data[writePos++] = data[readPos++];
					data[writePos++] = data[readPos++];
					data[writePos++] = data[readPos++];
					readPos++;//skip alpha
				}

				channels = 3;

			}

			loadTextureFromMemory(data, w, h, channels, quality);
			stbi_image_free(data);
		}

		return alphaData;
	}

	void GpuTexture::clear()
	{
		glDeleteTextures(1, &id);
		id = 0;
	}

	void GpuTexture::setTextureQuality(int quality)
	{
		if (!id)
			return;
		if (quality < 0)
			return;

		glBindTexture(GL_TEXTURE_2D, id);

		switch (quality)
		{
			case leastPossible:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.f);
			}
			break;
			case nearestMipmap:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.f);
			}
			break;
			case linearMipmap:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.f);
			}
			break;
			case maxQuality:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.f);
			}
			break;
			default:
			gl3dAssertComment(0, "invalid quality");
			break;
		}
	}

	int GpuTexture::getTextureQuality()
	{
		if(id == leastPossible)
		{
			return 0;
		}

		glBindTexture(GL_TEXTURE_2D, id);

		int param = 0;

		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &param);

		switch (param)
		{
			case GL_NEAREST:
			{
				return leastPossible;
			}
			break;
			case GL_NEAREST_MIPMAP_NEAREST:
			{
				return nearestMipmap;
			}
			break;
			case GL_LINEAR_MIPMAP_NEAREST:
			{
				return linearMipmap;
			}
			break;
			case GL_LINEAR_MIPMAP_LINEAR:
			{
				return maxQuality;
			}
			break;
			
		}

		return leastPossible;

	}

	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel)
	{
		unsigned char *newImage = new unsigned char[w * h * 3];


		//actually compute this on the gpu if really needed

		auto horiz = [&](int kernel)
		{
		//horizontal blur
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					glm::tvec3<int> colors = {};

					int beg = std::max(0, x - kernel);
					int end = std::min(x + kernel + 1, w);

					for (int i = beg; i < end; i++)
					{
						colors.r += data[(i + y * w) * 3 + 0];
						colors.g += data[(i + y * w) * 3 + 1];
						colors.b += data[(i + y * w) * 3 + 2];
					}

					if (x - kernel < 0)
						for (int i = kernel - x - 1; i >= 0; i--)
						{
							colors.r += data[(i + y * w) * 3 + 0];
							colors.g += data[(i + y * w) * 3 + 1];
							colors.b += data[(i + y * w) * 3 + 2];
						}

					if (x + kernel >= w)
						for (int i = w - 1; i >= w - (x + kernel - w + 1); i--)
						{
							colors.r += data[(i + y * w) * 3 + 0];
							colors.g += data[(i + y * w) * 3 + 1];
							colors.b += data[(i + y * w) * 3 + 2];
						}

					colors /= kernel * 2 + 1;


					//colors /= end - beg;

					newImage[(x + y * w) * 3 + 0] = colors.r;
					newImage[(x + y * w) * 3 + 1] = colors.g;
					newImage[(x + y * w) * 3 + 2] = colors.b;

				}

			}
		};

		auto vert = [&](int kernel)
		{
			//vertical blur
			for (int x = 0; x < w; x++)
			{
				for (int y = 0; y < h; y++)
				{
					glm::tvec3<int> colors = {};

					int beg = std::max(0, y - kernel);
					int end = std::min(y + kernel + 1, h);

					for (int j = beg; j < end; j++)
					{
						colors.r += data[(x + j * w) * 3 + 0];
						colors.g += data[(x + j * w) * 3 + 1];
						colors.b += data[(x + j * w) * 3 + 2];
					}

					if (y - kernel < 0)
						for (int j = kernel - y - 1; j >= 0; j--)
						{
							colors.r += data[(x + j * w) * 3 + 0];
							colors.g += data[(x + j * w) * 3 + 1];
							colors.b += data[(x + j * w) * 3 + 2];
						}

					if (y + kernel >= h)
						for (int j = h - 1; j >= h - (y + kernel - h + 1); j--)
						{
							colors.r += data[(x + j * w) * 3 + 0];
							colors.g += data[(x + j * w) * 3 + 1];
							colors.b += data[(x + j * w) * 3 + 2];
						}

					colors /= kernel * 2 + 1;

					//colors /= end - beg;

					newImage[(x + y * w) * 3 + 0] = colors.r;
					newImage[(x + y * w) * 3 + 1] = colors.g;
					newImage[(x + y * w) * 3 + 2] = colors.b;

				}

			}

		};

		int iterations = 2;

		for(int i=0;i<iterations;i++)
		{
			horiz(kernel);
			vert(kernel);
		}
		
		for (int i = 0; i < w * h * 3; i++)
		{
			data[i] = newImage[i];
		}

		delete newImage;
	}

	

};
#pragma endregion


////////////////////////////////////////////////
//Shader.cpp
////////////////////////////////////////////////
#pragma region Shader
#define GL3D_LOAD_SHADERS_FROM_HEADER_ONLY

#include <fstream>
#include <iostream>
#include <unordered_map>

namespace gl3d
{
	
	GLint createShaderFromData(const char* data, GLenum shaderType)
	{
		GLuint shaderId = glCreateShader(shaderType);
		glShaderSource(shaderId, 1, &data, nullptr);
		glCompileShader(shaderId);

		GLint rezult = 0;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &rezult);

		if (!rezult)
		{
			char* message = 0;
			int   l = 0;

			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &l);

			if (l)
			{
				message = new char[l];

				glGetShaderInfoLog(shaderId, l, &l, message);

				message[l - 1] = 0;

				std::cout << data << ":\n" << message << "\n";

				delete[] message;

			}
			else
			{
				std::cout << data << ":\n" << "unknown error" << "\n";
			}

			glDeleteShader(shaderId);

			shaderId = 0;
			return shaderId;
		}

		return shaderId;

	}

#ifdef GL3D_LOAD_SHADERS_FROM_HEADER_ONLY

	std::unordered_map<std::string, const char*> headerOnlyShaders =
	{

		//std::pair< std::string, const char*>{"name", "value"}
		//#pragma shaderSources
      std::pair<std::string, const char*>{"ssao.frag", R"(#version 330 core
out float fragCoord;
in vec2 v_texCoords;
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_texNoise;
uniform vec3 samples[64];
uniform mat4 u_projection; // camera projection matrix
uniform mat4 u_view; // camera view matrix
const int kernelSize = 64;
layout(std140) uniform u_SSAODATA
{
float radius;
float bias;
int samplesTestSize; // should be less than kernelSize
}ssaoDATA;
void main()
{
vec2 screenSize = textureSize(u_gPosition, 0);
vec2 noiseScale = vec2(screenSize.x/4.0, screenSize.y/4.0);
vec3 fragPos   = texture(u_gPosition, v_texCoords).xyz;
vec3 normal    = vec3(transpose(inverse(mat3(u_view))) * texture(u_gNormal, v_texCoords).rgb);
vec3 randomVec = texture(u_texNoise, v_texCoords * noiseScale).xyz; 
vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
vec3 bitangent = cross(normal, tangent);
mat3 TBN       = mat3(tangent, bitangent, normal); 
float occlusion = 0.0;
int begin = int((kernelSize - ssaoDATA.samplesTestSize) * abs(randomVec.x));
for(int i = begin; i < begin + ssaoDATA.samplesTestSize; ++i)
{
vec3 samplePos = TBN * samples[i]; // from tangent to view-space
samplePos = fragPos + samplePos * ssaoDATA.radius; 
vec4 offset = vec4(samplePos, 1.0);
offset = u_projection * offset; // from view to clip-space
offset.xyz /= offset.w; // perspective divide
offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
{
float sampleDepth = texture(u_gPosition, offset.xy).z; // get depth value of kernel sample
float rangeCheck = smoothstep(0.0, 1.0, ssaoDATA.radius / abs(fragPos.z - sampleDepth));
occlusion += (sampleDepth >= samplePos.z + ssaoDATA.bias ? 1.0 : 0.0) * rangeCheck;
}
}  
occlusion = 1.0 - (occlusion / kernelSize);
fragCoord = occlusion;
})"},

      std::pair<std::string, const char*>{"blur.frag", R"(#version 150
in vec2 v_texCoords;
uniform sampler2D u_ssaoInput;
out float fragColor;
void main ()
{
float result_1;
vec2 texelSize_2;
texelSize_2 = (1.0/(vec2(textureSize (u_ssaoInput, 0))));
result_1 = texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, -2.0) * texelSize_2))).x;
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-1.0, -2.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(0.0, -2.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(1.0, -2.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, -1.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords - texelSize_2)).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(0.0, -1.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(1.0, -1.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, 0.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-1.0, 0.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, v_texCoords).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(1.0, 0.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-2.0, 1.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(-1.0, 1.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + (vec2(0.0, 1.0) * texelSize_2))).x);
result_1 = (result_1 + texture (u_ssaoInput, (v_texCoords + texelSize_2)).x);
fragColor = (result_1 / 16.0);
})"},

      std::pair<std::string, const char*>{"skyBox.vert", R"(#version 330
#pragma debug(on)
layout (location = 0) in vec3 aPos;
out vec3 v_texCoords;
uniform mat4 u_viewProjection;
void main()
{
v_texCoords = aPos;
vec4 pos = u_viewProjection * vec4(aPos, 1.0);
gl_Position = pos.xyww;
}  )"},

      std::pair<std::string, const char*>{"skyBox.frag", R"(#version 150
out vec4 a_outColor;
in vec3 v_texCoords;
uniform samplerCube u_skybox;
uniform float u_exposure;
uniform vec3 u_ambient;
uniform int u_skyBoxPresent;
void main ()
{
if ((u_skyBoxPresent != 0)) {
vec4 tmpvar_1;
tmpvar_1 = textureLod (u_skybox, v_texCoords, 2.0);
a_outColor.w = tmpvar_1.w;
a_outColor.xyz = (tmpvar_1.xyz * u_ambient);
} else {
a_outColor.xyz = u_ambient;
};
a_outColor.xyz = (vec3(1.0, 1.0, 1.0) - exp((
-(a_outColor.xyz)
* u_exposure)));
a_outColor.xyz = pow (a_outColor.xyz, vec3(0.4545454, 0.4545454, 0.4545454));
})"},

      std::pair<std::string, const char*>{"preFilterSpecular.frag", R"(#version 150
out vec4 FragColor;
in vec3 v_localPos;
uniform samplerCube u_environmentMap;
uniform float u_roughness;
void main ()
{
float totalWeight_2;
vec3 prefilteredColor_3;
vec3 V_4;
vec3 N_5;
vec3 tmpvar_6;
tmpvar_6 = normalize(v_localPos);
N_5 = tmpvar_6;
V_4 = tmpvar_6;
prefilteredColor_3 = vec3(0.0, 0.0, 0.0);
totalWeight_2 = 0.0;
for (uint i_1 = uint(0); i_1 < 1024u; i_1++) {
float tmpvar_7;
uint bits_8;
bits_8 = ((i_1 << 16u) | (i_1 >> 16u));
bits_8 = (((bits_8 & 1431655765u) << 1u) | ((bits_8 & 2863311530u) >> 1u));
bits_8 = (((bits_8 & 858993459u) << 2u) | ((bits_8 & 3435973836u) >> 2u));
bits_8 = (((bits_8 & 252645135u) << 4u) | ((bits_8 & 4042322160u) >> 4u));
bits_8 = (((bits_8 & 16711935u) << 8u) | ((bits_8 & 4278255360u) >> 8u));
tmpvar_7 = (float(bits_8) * 2.328306e-10);
vec2 tmpvar_9;
tmpvar_9.x = (float(i_1) / 1024.0);
tmpvar_9.y = tmpvar_7;
vec3 H_10;
float tmpvar_11;
tmpvar_11 = (u_roughness * u_roughness);
float tmpvar_12;
tmpvar_12 = (6.283185 * tmpvar_9.x);
float tmpvar_13;
tmpvar_13 = sqrt(((1.0 - tmpvar_7) / (1.0 + 
(((tmpvar_11 * tmpvar_11) - 1.0) * tmpvar_7)
)));
float tmpvar_14;
tmpvar_14 = sqrt((1.0 - (tmpvar_13 * tmpvar_13)));
H_10.x = (cos(tmpvar_12) * tmpvar_14);
H_10.y = (sin(tmpvar_12) * tmpvar_14);
H_10.z = tmpvar_13;
float tmpvar_15;
tmpvar_15 = abs(N_5.z);
vec3 tmpvar_16;
if ((tmpvar_15 < 0.999)) {
tmpvar_16 = vec3(0.0, 0.0, 1.0);
} else {
tmpvar_16 = vec3(1.0, 0.0, 0.0);
};
vec3 tmpvar_17;
tmpvar_17 = normalize(((tmpvar_16.yzx * N_5.zxy) - (tmpvar_16.zxy * N_5.yzx)));
vec3 tmpvar_18;
tmpvar_18 = normalize(((
(tmpvar_17 * H_10.x)
+ 
(((N_5.yzx * tmpvar_17.zxy) - (N_5.zxy * tmpvar_17.yzx)) * H_10.y)
) + (N_5 * tmpvar_13)));
vec3 tmpvar_19;
tmpvar_19 = normalize(((
(2.0 * dot (V_4, tmpvar_18))
* tmpvar_18) - V_4));
float tmpvar_20;
tmpvar_20 = max (dot (N_5, tmpvar_19), 0.0);
if ((tmpvar_20 > 0.0)) {
float tmpvar_21;
tmpvar_21 = (u_roughness * u_roughness);
float tmpvar_22;
tmpvar_22 = (tmpvar_21 * tmpvar_21);
float tmpvar_23;
tmpvar_23 = max (dot (N_5, tmpvar_18), 0.0);
float tmpvar_24;
tmpvar_24 = (((tmpvar_23 * tmpvar_23) * (tmpvar_22 - 1.0)) + 1.0);
float tmpvar_25;
tmpvar_25 = (1.0/(((1024.0 * 
((((tmpvar_22 / 
((3.141593 * tmpvar_24) * tmpvar_24)
) * max (
dot (N_5, tmpvar_18)
, 0.0)) / (4.0 * max (
dot (tmpvar_18, V_4)
, 0.0))) + 0.0001)
) + 0.0001)));
float tmpvar_26;
if ((u_roughness == 0.0)) {
tmpvar_26 = 0.0;
} else {
tmpvar_26 = (0.5 * log2((tmpvar_25 / 7.989483e-6)));
};
prefilteredColor_3 = (prefilteredColor_3 + (textureLod (u_environmentMap, tmpvar_19, tmpvar_26).xyz * tmpvar_20));
totalWeight_2 = (totalWeight_2 + tmpvar_20);
};
};
prefilteredColor_3 = (prefilteredColor_3 / totalWeight_2);
vec4 tmpvar_27;
tmpvar_27.w = 1.0;
tmpvar_27.xyz = prefilteredColor_3;
FragColor = tmpvar_27;
})"},

      std::pair<std::string, const char*>{"hdrToCubeMap.vert", R"(#version 330
#pragma debug(on)
layout (location = 0) in vec3 aPos;
out vec3 v_localPos;
uniform mat4 u_viewProjection;
void main()
{
v_localPos = aPos;
gl_Position = u_viewProjection * vec4(aPos, 1.0);
}  )"},

      std::pair<std::string, const char*>{"hdrToCubeMap.frag", R"(#version 150
out vec4 FragColor;
in vec3 v_localPos;
uniform sampler2D u_equirectangularMap;
void main ()
{
vec3 tmpvar_1;
tmpvar_1 = normalize(v_localPos);
vec2 uv_2;
float tmpvar_3;
float tmpvar_4;
tmpvar_4 = (min (abs(
(tmpvar_1.z / tmpvar_1.x)
), 1.0) / max (abs(
(tmpvar_1.z / tmpvar_1.x)
), 1.0));
float tmpvar_5;
tmpvar_5 = (tmpvar_4 * tmpvar_4);
tmpvar_5 = (((
((((
((((-0.01213232 * tmpvar_5) + 0.05368138) * tmpvar_5) - 0.1173503)
* tmpvar_5) + 0.1938925) * tmpvar_5) - 0.3326756)
* tmpvar_5) + 0.9999793) * tmpvar_4);
tmpvar_5 = (tmpvar_5 + (float(
(abs((tmpvar_1.z / tmpvar_1.x)) > 1.0)
) * (
(tmpvar_5 * -2.0)
+ 1.570796)));
tmpvar_3 = (tmpvar_5 * sign((tmpvar_1.z / tmpvar_1.x)));
if ((abs(tmpvar_1.x) > (1e-8 * abs(tmpvar_1.z)))) {
if ((tmpvar_1.x < 0.0)) {
if ((tmpvar_1.z >= 0.0)) {
tmpvar_3 += 3.141593;
} else {
tmpvar_3 = (tmpvar_3 - 3.141593);
};
};
} else {
tmpvar_3 = (sign(tmpvar_1.z) * 1.570796);
};
vec2 tmpvar_6;
tmpvar_6.x = tmpvar_3;
tmpvar_6.y = (sign(tmpvar_1.y) * (1.570796 - (
sqrt((1.0 - abs(tmpvar_1.y)))
* 
(1.570796 + (abs(tmpvar_1.y) * (-0.2146018 + (
abs(tmpvar_1.y)
* 
(0.08656672 + (abs(tmpvar_1.y) * -0.03102955))
))))
)));
uv_2 = (tmpvar_6 * vec2(0.1591, 0.3183));
uv_2 = (uv_2 + 0.5);
vec4 tmpvar_7;
tmpvar_7.w = 1.0;
tmpvar_7.xyz = texture (u_equirectangularMap, uv_2).xyz;
FragColor = tmpvar_7;
})"},

      std::pair<std::string, const char*>{"convolute.frag", R"(#version 150
out vec4 fragColor;
in vec3 v_localPos;
uniform samplerCube u_environmentMap;
void main ()
{
float nrSamples_2;
vec3 right_3;
vec3 up_4;
vec3 irradiance_5;
vec3 normal_6;
vec3 tmpvar_7;
tmpvar_7 = normalize(v_localPos);
normal_6 = tmpvar_7;
irradiance_5 = vec3(0.0, 0.0, 0.0);
vec3 tmpvar_8;
tmpvar_8 = normalize(((vec3(1.0, 0.0, 0.0) * tmpvar_7.zxy) - (vec3(0.0, 0.0, 1.0) * tmpvar_7.yzx)));
right_3 = tmpvar_8;
up_4 = normalize(((tmpvar_7.yzx * tmpvar_8.zxy) - (tmpvar_7.zxy * tmpvar_8.yzx)));
nrSamples_2 = 0.0;
for (float phi_1 = 0.0; phi_1 < 6.283185; phi_1 += 0.025) {
for (float theta_9 = 0.0; theta_9 < 1.570796; theta_9 += 0.025) {
float tmpvar_10;
tmpvar_10 = cos(theta_9);
vec3 tmpvar_11;
tmpvar_11.x = (sin(theta_9) * cos(phi_1));
tmpvar_11.y = (sin(theta_9) * sin(phi_1));
tmpvar_11.z = tmpvar_10;
irradiance_5 = (irradiance_5 + ((texture (u_environmentMap, 
(((tmpvar_11.x * right_3) + (tmpvar_11.y * up_4)) + (tmpvar_10 * normal_6))
).xyz * 
cos(theta_9)
) * sin(theta_9)));
nrSamples_2 += 1.0;
};
};
irradiance_5 = (irradiance_5 * (3.141593 / nrSamples_2));
vec4 tmpvar_12;
tmpvar_12.w = 1.0;
tmpvar_12.xyz = irradiance_5;
fragColor = tmpvar_12;
})"},

      std::pair<std::string, const char*>{"atmosphericScattering.frag", R"(#version 330 core
uniform vec3 u_lightPos;
in vec3 v_localPos;
out vec3 fragColor;
void main (void)
{
vec3 v3CameraPos = vec3(0,0,0);		// The camera's current position
vec3 v3InvWavelength;	// 1 / pow(wavelength, 4) for the red, green, and blue channels
float fCameraHeight = 0;	// The camera's current height
float fCameraHeight2 = fCameraHeight * fCameraHeight;	// fCameraHeight^2
float fOuterRadius;		// The outer (atmosphere) radius
float fOuterRadius2 = fOuterRadius * fOuterRadius;	// fOuterRadius^2
float fInnerRadius;		// The inner (planetary) radius
float fInnerRadius2 = fInnerRadius * fInnerRadius;	// fInnerRadius^2
float fKrESun;			// Kr * ESun
float fKmESun;			// Km * ESun
float fKr4PI;			// Kr * 4 * PI
float fKm4PI;			// Km * 4 * PI
float fScale;			// 1 / (fOuterRadius - fInnerRadius)
float fScaleDepth;		// The scale depth (i.e. the altitude at which the atmosphere's average density is found)
float fScaleOverScaleDepth;	// fScale / fScaleDepth
vec3 firstColor;
vec3 secondColor;
vec3 localPos = normalize(v_localPos);
vec3 lightPos = normalize(u_lightPos);
float u_g = 0.76;
float u_g2 = u_g * u_g;
float fCos = dot(lightPos, localPos);
float fMiePhase = 1.5 * ((1.0 - u_g2) / (2.0 + u_g2)) * (1.0 + fCos*fCos) / pow(1.0 + u_g2 - 2.0*u_g*fCos, 1.5);
fragColor.rgb =  firstColor + fMiePhase * secondColor;
})"},

      std::pair<std::string, const char*>{"varienceShadowMap.frag", R"(#version 150
uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;
out vec2 outColor;
void main ()
{
if ((u_hasTexture != 0)) {
vec4 tmpvar_1;
tmpvar_1 = texture (u_albedoSampler, v_texCoord);
if ((tmpvar_1.w <= 0.1)) {
discard;
};
};
vec2 tmpvar_2;
tmpvar_2.x = gl_FragCoord.z;
tmpvar_2.y = (gl_FragCoord.z * gl_FragCoord.z);
outColor = tmpvar_2;
})"},

      std::pair<std::string, const char*>{"pointShadow.vert", R"(#version 330
#pragma debug(on)
layout(location = 0) in vec3 a_positions;
layout(location = 1) in vec3 a_normals; //todo comment out
layout(location = 2) in vec2 a_texCoord;
uniform mat4 u_transform; //full model view projection or just model for point shadows
out vec2 v_texCoord;
void main()
{
gl_Position = u_transform * vec4(a_positions, 1.f);
v_texCoord = a_texCoord;
} )"},

      std::pair<std::string, const char*>{"pointShadow.geom", R"(#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;
uniform mat4 u_shadowMatrices[6];
uniform int u_lightIndex;
out vec4 v_fragPos; // FragPos from GS (output per emitvertex)
out vec2 v_finalTexCoord;
in vec2 v_texCoord[3];
void main()
{
for(int face = 0; face < 6; ++face)
{
gl_Layer = face + u_lightIndex * 6; // built-in variable that specifies to which face we render.
for(int i = 0; i < 3; ++i) // for each triangle vertex
{
v_fragPos = gl_in[i].gl_Position;
v_finalTexCoord = v_texCoord[i];
gl_Position = u_shadowMatrices[face] * v_fragPos;
EmitVertex();
}    
EndPrimitive();
}
}  )"},

      std::pair<std::string, const char*>{"pointShadow.frag", R"(#version 150
uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
uniform vec3 u_lightPos;
uniform float u_farPlane;
in vec2 v_texCoord;
in vec4 v_fragPos;
void main ()
{
if ((u_hasTexture != 0)) {
vec4 tmpvar_1;
tmpvar_1 = texture (u_albedoSampler, v_texCoord);
if ((tmpvar_1.w <= 0.1)) {
discard;
};
};
vec3 x_2;
x_2 = (v_fragPos.xyz - u_lightPos);
gl_FragDepth = (sqrt(dot (x_2, x_2)) / u_farPlane);
})"},

      std::pair<std::string, const char*>{"postProcess.frag", R"(#version 150
out vec4 a_color;
in vec2 v_texCoords;
uniform sampler2D u_colorTexture;
uniform sampler2D u_bloomTexture;
uniform sampler2D u_bloomNotBluredTexture;
uniform float u_bloomIntensity;
uniform float u_exposure;
uniform int u_useSSAO;
uniform float u_ssaoExponent;
uniform sampler2D u_ssao;
void main ()
{
float ssaof_1;
vec4 tmpvar_2;
tmpvar_2 = texture (u_colorTexture, v_texCoords);
ssaof_1 = 1.0;
if ((u_useSSAO != 0)) {
ssaof_1 = pow (texture (u_ssao, v_texCoords).x, u_ssaoExponent);
} else {
ssaof_1 = 1.0;
};
a_color.xyz = ((texture (u_bloomTexture, v_texCoords).xyz * u_bloomIntensity) + ((texture (u_bloomNotBluredTexture, v_texCoords).xyz + tmpvar_2.xyz) * ssaof_1));
a_color.xyz = (vec3(1.0, 1.0, 1.0) - exp((
-(a_color.xyz)
* u_exposure)));
a_color.xyz = pow (a_color.xyz, vec3(0.4545454, 0.4545454, 0.4545454));
a_color.w = tmpvar_2.w;
})"},

      std::pair<std::string, const char*>{"gausianBlur.frag", R"(#version 150
in vec2 v_texCoords;
uniform sampler2D u_toBlurcolorInput;
out vec3 fragColor;
uniform bool u_horizontal;
void main ()
{
vec3 result_1;
vec2 texOffset_2;
texOffset_2 = (1.0/(vec2(textureSize (u_toBlurcolorInput, 0))));
result_1 = (texture (u_toBlurcolorInput, v_texCoords).xyz * 0.227027);
if (u_horizontal) {
vec2 tmpvar_3;
tmpvar_3.y = 0.0;
tmpvar_3.x = texOffset_2.x;
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_3)).xyz * 0.1945946));
vec2 tmpvar_4;
tmpvar_4.y = 0.0;
tmpvar_4.x = texOffset_2.x;
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_4)).xyz * 0.1945946));
vec2 tmpvar_5;
tmpvar_5.y = 0.0;
tmpvar_5.x = (texOffset_2.x * 2.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_5)).xyz * 0.1216216));
vec2 tmpvar_6;
tmpvar_6.y = 0.0;
tmpvar_6.x = (texOffset_2.x * 2.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_6)).xyz * 0.1216216));
vec2 tmpvar_7;
tmpvar_7.y = 0.0;
tmpvar_7.x = (texOffset_2.x * 3.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_7)).xyz * 0.054054));
vec2 tmpvar_8;
tmpvar_8.y = 0.0;
tmpvar_8.x = (texOffset_2.x * 3.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_8)).xyz * 0.054054));
vec2 tmpvar_9;
tmpvar_9.y = 0.0;
tmpvar_9.x = (texOffset_2.x * 4.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_9)).xyz * 0.016216));
vec2 tmpvar_10;
tmpvar_10.y = 0.0;
tmpvar_10.x = (texOffset_2.x * 4.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_10)).xyz * 0.016216));
} else {
vec2 tmpvar_11;
tmpvar_11.x = 0.0;
tmpvar_11.y = texOffset_2.y;
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_11)).xyz * 0.1945946));
vec2 tmpvar_12;
tmpvar_12.x = 0.0;
tmpvar_12.y = texOffset_2.y;
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_12)).xyz * 0.1945946));
vec2 tmpvar_13;
tmpvar_13.x = 0.0;
tmpvar_13.y = (texOffset_2.y * 2.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_13)).xyz * 0.1216216));
vec2 tmpvar_14;
tmpvar_14.x = 0.0;
tmpvar_14.y = (texOffset_2.y * 2.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_14)).xyz * 0.1216216));
vec2 tmpvar_15;
tmpvar_15.x = 0.0;
tmpvar_15.y = (texOffset_2.y * 3.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_15)).xyz * 0.054054));
vec2 tmpvar_16;
tmpvar_16.x = 0.0;
tmpvar_16.y = (texOffset_2.y * 3.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_16)).xyz * 0.054054));
vec2 tmpvar_17;
tmpvar_17.x = 0.0;
tmpvar_17.y = (texOffset_2.y * 4.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords + tmpvar_17)).xyz * 0.016216));
vec2 tmpvar_18;
tmpvar_18.x = 0.0;
tmpvar_18.y = (texOffset_2.y * 4.0);
result_1 = (result_1 + (texture (u_toBlurcolorInput, (v_texCoords - tmpvar_18)).xyz * 0.016216));
};
fragColor = result_1;
})"},

      std::pair<std::string, const char*>{"mergePBRmat.frag", R"(#version 430 core
in vec2 v_texCoords;
out vec4 fragColor;
layout(binding = 0) uniform sampler2D u_roughness;
layout(binding = 1) uniform sampler2D u_metallic;
layout(binding = 2) uniform sampler2D u_ambient;
void main()
{
float metallic = texture(u_metallic, v_texCoords).r;
float roughness = texture(u_roughness, v_texCoords).r;
float ambient = texture(u_ambient, v_texCoords).r;
fragColor = vec4(roughness, metallic, ambient, 1);
})"},

      std::pair<std::string, const char*>{"zPrePass.frag", R"(#version 150
uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;
void main ()
{
if ((u_hasTexture != 0)) {
vec4 tmpvar_1;
tmpvar_1 = texture (u_albedoSampler, v_texCoord);
if ((tmpvar_1.w <= 0.1)) {
discard;
};
};
})"},

      std::pair<std::string, const char*>{"lightingPass.frag", R"(#version 430
#pragma debug(on)
layout(location = 0) out vec4 a_outColor;
layout(location = 1) out vec4 a_outBloom;
in vec2 v_texCoords;
uniform sampler2D u_albedo;
uniform sampler2D u_normals;
uniform samplerCube u_skyboxFiltered;
uniform samplerCube u_skyboxIradiance;
uniform sampler2D u_positions;
uniform sampler2D u_materials;
uniform sampler2D u_brdfTexture;
uniform sampler2D u_emmisive;
uniform sampler2DArrayShadow u_cascades;
uniform sampler2DArrayShadow u_spotShadows;
uniform samplerCubeArrayShadow u_pointShadows;
uniform vec3 u_eyePosition;
uniform mat4 u_view;
layout (std140) uniform u_lightPassData
{
vec4 ambientColor;
float bloomTresshold;
int lightSubScater;
float exposure;
int skyBoxPresent;
}lightPassData;
struct PointLight
{
vec3 positions; 
float dist;
vec3 color;
float attenuation;
int castShadowsIndex;
float hardness;
int castShadows;
int changedThisFrame;
};
readonly restrict layout(std140) buffer u_pointLights
{
PointLight light[];
};
uniform int u_pointLightCount;
struct DirectionalLight
{
vec3 direction; 
int castShadowsIndex; //this is both the index and the toggle
vec4 color;		//w is a hardness exponent
mat4 firstLightSpaceMatrix;
mat4 secondLightSpaceMatrix;
mat4 thirdLightSpaceMatrix;
};
readonly restrict layout(std140) buffer u_directionalLights
{
DirectionalLight dLight[];
};
uniform int u_directionalLightCount;
struct SpotLight
{
vec4 position; //w = cos(half angle)
vec4 direction; //w dist
vec4 color; //w attenuation
float hardness;
int shadowIndex;
int castShadows;		
int changedThisFrame; //not used in the gpu
float near;
float far;
float notUsed1;
float notUsed2;
mat4 lightSpaceMatrix;
};
readonly restrict layout(std140) buffer u_spotLights
{
SpotLight spotLights[];
};
uniform int u_spotLightCount;
const float PI = 3.14159265359;
const float randomNumbers[100] = float[100](
0.05535,	0.22262,	0.93768,	0.80063,	0.40089,	0.49459,	0.44997,	0.27060,	0.58789,	0.61765,
0.87949,	0.38913,	0.23154,	0.27249,	0.93448,	0.71567,	0.26940,	0.32226,	0.73918,	0.30905,
0.98754,	0.82585,	0.84031,	0.60059,	0.56027,	0.10819,	0.55848,	0.95612,	0.88034,	0.94950,
0.53892,	0.86421,	0.84131,	0.39158,	0.25861,	0.10192,	0.19673,	0.25165,	0.68675,	0.79157,
0.94730,	0.36948,	0.27978,	0.66377,	0.38935,	0.93795,	0.83168,	0.01452,	0.51242,	0.12272,
0.61045,	0.34752,	0.13781,	0.92361,	0.73422,	0.31213,	0.55513,	0.81074,	0.56166,	0.31797,
0.09507,	0.50049,	0.44248,	0.38244,	0.58468,	0.32327,	0.61830,	0.67908,	0.16011,	0.82861,
0.36502,	0.12052,	0.28872,	0.73448,	0.51443,	0.99355,	0.75244,	0.22432,	0.95501,	0.90914,
0.37992,	0.61330,	0.49202,	0.69464,	0.14831,	0.51697,	0.34620,	0.55315,	0.41602,	0.49807,
0.15133,	0.07372,	0.75259,	0.59642,	0.35652,	0.60051,	0.08879,	0.59271,	0.29388,	0.69505
);
float attenuationFunctionNotClamped(float x, float r, float p)
{
float p4 = p*p*p*p;
float power = pow(x/r, p4);
float rez = (1-power);
rez = rez * rez;
return rez;
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
float a      = roughness*roughness;
float a2     = a*a;
float NdotH  = max(dot(N, H), 0.0);
float NdotH2 = NdotH*NdotH;
float denom = (NdotH2 * (a2 - 1.0) + 1.0);
denom = PI * denom * denom;
return  a2 / max(denom, 0.0000001);
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
float r = (roughness + 1.0);
float k = (r*r) / 8.0;
float num   = NdotV;
float denom = NdotV * (1.0 - k) + k;
return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
float NdotV = max(dot(N, V), 0.0);
float NdotL = max(dot(N, L), 0.0);
float ggx2  = GeometrySchlickGGX(NdotV, roughness);
float ggx1  = GeometrySchlickGGX(NdotL, roughness);
return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   
vec3 computePointLightSource(vec3 lightDirection, float metallic, float roughness, in vec3 lightColor, in vec3 worldPosition,
in vec3 viewDir, in vec3 color, in vec3 normal, in vec3 F0)
{
float dotNVclamped = clamp(dot(normal, viewDir), 0.0, 0.99);
vec3 halfwayVec = normalize(lightDirection + viewDir);
float attenuation = 1; //(option) remove light attenuation
vec3 radiance = lightColor * attenuation; //here the first component is the light color
vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);
float NDF = DistributionGGX(normal, halfwayVec, roughness);       
float G   = GeometrySmith(normal, viewDir, lightDirection, roughness);   
float denominator = 4.0 * dotNVclamped  
* max(dot(normal, lightDirection), 0.0);
vec3 specular     = (NDF * G * F) / max(denominator, 0.001);
vec3 kS = F; //this is the specular contribution
vec3 kD = vec3(1.0) - kS; //the difuse is the remaining specular
kD *= 1.0 - metallic;	//metallic surfaces are darker
float NdotL = max(dot(normal, lightDirection), 0.0);        
vec3 Lo = (kD * color.rgb / PI + specular) * radiance * NdotL;
return Lo;
}
float testShadowValue(sampler2DArrayShadow map, vec2 coords, float currentDepth, float bias, int index)
{
return texture(map, vec4(coords, index, currentDepth-bias)).r;
}
float shadowCalculation(vec3 projCoords, float bias, sampler2DArrayShadow shadowMap, int index)
{
if(projCoords.z > 0.99995)
return 1.f;
float currentDepth = projCoords.z;
vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
float shadow = 0.0;
bool fewSamples = false;
int kernelSize = 5;
int kernelSize2 = kernelSize*kernelSize;
int kernelHalf = kernelSize/2;
float shadowValueAtCentre = 0;
if(true)
{
float offsetSize = kernelSize/2;
const int OFFSETS = 4;
vec2 offsets[OFFSETS] = 
{
vec2(offsetSize,offsetSize),
vec2(-offsetSize,offsetSize),
vec2(offsetSize,-offsetSize),
vec2(-offsetSize,-offsetSize),
};
fewSamples = true;
float s1 = testShadowValue(shadowMap, projCoords.xy, 
currentDepth, bias, index); 
shadowValueAtCentre = s1;
for(int i=0;i<OFFSETS; i++)
{
float s2 = testShadowValue(shadowMap, projCoords.xy + offsets[i] * texelSize * 2, 
currentDepth, bias, index); 
if(s1 != s2)
{
fewSamples = false;
break;
}	
s1 = s2;
}
}
if(fewSamples)
{
shadow = shadowValueAtCentre;
}else
{
for(int y = -kernelHalf; y <= kernelHalf; ++y)
{
for(int x = -kernelHalf; x <= kernelHalf; ++x)
{
vec2 offset = vec2(x, y);
if(false)
{
int randomOffset1 = (x*kernelSize) + y;
int randomOffset2 = randomOffset1 + kernelSize2;
offset += vec2(randomNumbers[randomOffset1, randomOffset2]);
}
if(false)
{
float u = (offset.x + kernelHalf+1)/float(kernelSize);
float v = (offset.x + kernelHalf+1)/float(kernelSize);
offset.x = sqrt(v) * cos(2*PI * u)* kernelSize / 2.f;
offset.y = sqrt(v) * sin(2*PI * u)* kernelSize / 2.f;
}
float s = testShadowValue(shadowMap, projCoords.xy + offset * texelSize, 
currentDepth, bias, index); 
shadow += s;
}    
}
shadow /= kernelSize2;
}
return clamp(shadow, 0, 1);
}
float shadowCalculationLinear(vec3 projCoords, vec3 normal, vec3 lightDir, sampler2DArrayShadow shadowMap, int index)
{
float bias = max((10.f/1024.f) * (1.0 - dot(normal, -lightDir)), 3.f/1024.f);
return shadowCalculation(projCoords, bias, shadowMap, index);
}
float linearizeDepth(float depth, float near, float far)
{
float z = depth * 2.0 - 1.0; // Back to NDC 
return (2.0 * near * far) / (far + near - z * (far - near));
}
float nonLinearDepth(float depth, float near, float far)
{
return ((1.f/depth) - (1.f/near)) / ((1.f/far) - (1.f/near));
}
float shadowCalculationLogaritmic(vec3 projCoords, vec3 normal, vec3 lightDir,
sampler2DArrayShadow shadowMap, int index, float near, float far)
{
float bias = max((0.01f) * (1.0 - dot(normal, -lightDir)), 0.001f);
float currentDepth = projCoords.z;
float liniarizedDepth = linearizeDepth(currentDepth, near, far);
liniarizedDepth += bias;
float biasedLogDepth = nonLinearDepth(liniarizedDepth, near, far);
bias = biasedLogDepth - currentDepth;
bias += 0.00003f;
return shadowCalculation(projCoords, bias, shadowMap, index);
}
vec3 getProjCoords(in mat4 matrix, in vec3 pos)
{
vec4 p = matrix * vec4(pos,1);
vec3 r = p .xyz / p .w;
r = r * 0.5 + 0.5;
return r;
}
void generateTangentSpace(in vec3 v, out vec3 outUp, out vec3 outRight)
{
vec3 up = vec3(0.f, 1.f, 0.f);
if (v == up)
{
outRight = vec3(1, 0, 0);
}
else
{
outRight = normalize(cross(v, up));
}
outUp = normalize(cross(outRight, v));
}
float pointShadowCalculation(vec3 pos, vec3 normal, int index)
{	
vec3 fragToLight = pos - light[index].positions; 
vec3 lightDir = normalize(fragToLight);
float bias = max((60.f/512.f) * (1.0 - dot(normal, -lightDir)), 35.f/512.f);
float shadow  = 0.0;
vec3 tangent;
vec3 coTangent;
generateTangentSpace(lightDir, tangent, coTangent);
float texel = 1.f / textureSize(u_pointShadows, 0).x;
int kernel = 5;
int kernelHalf = kernel/2;
for(int y = -kernelHalf; y<=kernelHalf; y++)
{
for(int x = -kernelHalf; x<=kernelHalf; x++)
{
vec3 fragToLight = pos - light[index].positions; 			
fragToLight += 4*x * texel * tangent;
fragToLight += 4*y * texel * coTangent;
float currentDepth = length(fragToLight);  
float value = texture(u_pointShadows, 
vec4(fragToLight, light[index].castShadowsIndex),
(currentDepth-bias)/light[index].dist ).r; 
shadow += value;
}
}
if(shadow <3)
{
shadow = 0;
}
shadow /= (kernel * kernel);
shadow = clamp(shadow, 0, 1);
return shadow;
}
float cascadedShadowCalculation(vec3 pos, vec3 normal, vec3 lightDir, int index)
{
vec4 viewSpacePos = u_view * vec4(pos, 1);
float depth = -viewSpacePos.z; //zfar
vec3 firstProjCoords = getProjCoords(dLight[index].firstLightSpaceMatrix, pos);
vec3 secondProjCoords = getProjCoords(dLight[index].secondLightSpaceMatrix, pos);
vec3 thirdProjCoords = getProjCoords(dLight[index].thirdLightSpaceMatrix, pos);
if(
firstProjCoords.x < 0.98 &&
firstProjCoords.x > 0.01 &&
firstProjCoords.y < 0.98 &&
firstProjCoords.y > 0.01 &&
firstProjCoords.z < 0.98 &&
firstProjCoords.z > 0
)
{
firstProjCoords.y /= 3.f;
return shadowCalculationLinear(firstProjCoords, normal, lightDir, u_cascades, index);
}else 
if(
secondProjCoords.x > 0 &&
secondProjCoords.x < 1 &&
secondProjCoords.y > 0 &&
secondProjCoords.y < 1 &&
secondProjCoords.z < 0.98
)
{
secondProjCoords.y /= 3.f;
secondProjCoords.y += 1.f / 3.f;
return shadowCalculationLinear(secondProjCoords, normal, lightDir, u_cascades, index);
}
else
{
thirdProjCoords.y /= 3.f;
thirdProjCoords.y += 2.f / 3.f;
return shadowCalculationLinear(thirdProjCoords, normal, lightDir, u_cascades, index);
}
}
float linStep(float v, float low, float high)
{
return clamp((v-low) / (high-low), 0.0, 1.0);
};
void main()
{
vec3 pos = texture(u_positions, v_texCoords).xyz;
vec3 normal = texture(u_normals, v_texCoords).xyz;
vec4 albedoAlpha = texture(u_albedo, v_texCoords).rgba;
vec3 emissive = texture(u_emmisive, v_texCoords).xyz;
vec3 albedo = albedoAlpha.rgb;
albedo  = pow(albedo , vec3(2.2,2.2,2.2)).rgb; //gamma corection
emissive  = pow(emissive , vec3(2.2,2.2,2.2)).rgb; //gamma corection
vec3 material = texture(u_materials, v_texCoords).xyz;
vec3 viewDir = normalize(u_eyePosition - pos);
vec3 R = reflect(-viewDir, normal);	//reflected vector
vec3 Lo = vec3(0,0,0); //this is the accumulated light
float roughness = clamp(material.r, 0.09, 0.99);
float metallic = clamp(material.g, 0.0, 0.98);
float ambientOcclution = material.b;
vec3 F0 = vec3(0.04); 
F0 = mix(F0, albedo.rgb, vec3(metallic));
for(int i=0; i<u_pointLightCount;i++)
{
vec3 lightPosition = light[i].positions.xyz;
vec3 lightColor = light[i].color.rgb;
vec3 lightDirection = normalize(lightPosition - pos);
float currentDist = distance(lightPosition, pos);
if(currentDist >= light[i].dist)
{
continue;
}
float attenuation = attenuationFunctionNotClamped(currentDist, light[i].dist, light[i].attenuation);	
float shadow = 1.f;
if(light[i].castShadows != 0)
{
shadow = pointShadowCalculation(pos, normal, i);
shadow = pow(shadow, light[i].hardness);
}
Lo += computePointLightSource(lightDirection, metallic, roughness, lightColor, 
pos, viewDir, albedo, normal, F0) * attenuation * shadow;
}
for(int i=0; i<u_directionalLightCount; i++)
{
vec3 lightDirection = dLight[i].direction.xyz;
vec3 lightColor = dLight[i].color.rgb;
float shadow = 1;
if(dLight[i].castShadowsIndex >= 0)
{	
shadow = cascadedShadowCalculation(pos, normal, lightDirection, dLight[i].castShadowsIndex);
shadow = pow(shadow, dLight[i].color.w);
}
Lo += computePointLightSource(-lightDirection, metallic, roughness, lightColor, 
pos, viewDir, albedo, normal, F0) * shadow;
}
for(int i=0; i<u_spotLightCount; i++)
{
vec3 lightPosition = spotLights[i].position.xyz;
vec3 lightColor = spotLights[i].color.rgb;
vec3 spotLightDirection = spotLights[i].direction.xyz;
vec3 lightDirection = -normalize(lightPosition - pos);
float angle = spotLights[i].position.w;
float dist = spotLights[i].direction.w;
float at = spotLights[i].color.w;
float dotAngle = dot(normalize(vec3(pos - lightPosition)), spotLightDirection);
float currentDist = distance(lightPosition, pos);
if(currentDist >= dist)
{
continue;
}
if(dotAngle > angle && dotAngle > 0)
{
float attenuation = attenuationFunctionNotClamped(currentDist, dist, at);
float smoothingVal = 0.01; //
float innerAngle = angle + smoothingVal;
float smoothing = clamp((dotAngle-angle)/smoothingVal,0.0,1.0);
vec3 shadowProjCoords = getProjCoords(spotLights[i].lightSpaceMatrix, pos);
float shadow = 1;
if(spotLights[i].castShadows != 0)
{
shadow = shadowCalculationLogaritmic(shadowProjCoords, normal, lightDirection, 
u_spotShadows, spotLights[i].shadowIndex, spotLights[i].near, spotLights[i].far);
shadow = pow(shadow, spotLights[i].hardness);
}
smoothing = pow(smoothing, spotLights[i].hardness);
Lo += computePointLightSource(-lightDirection, metallic, roughness, lightColor, 
pos, viewDir, albedo, normal, F0) * smoothing * attenuation * shadow;
}
}
vec3 ambient;
if(lightPassData.skyBoxPresent != 0)
{
vec3 N = normal;
vec3 V = viewDir;
float dotNVClamped = clamp(dot(N, V), 0.0, 0.99);
vec3 F = fresnelSchlickRoughness(dotNVClamped, F0, roughness);
vec3 kS = F;
vec3 irradiance = texture(u_skyboxIradiance, normal).rgb; //this color is coming directly at the object
const float MAX_REFLECTION_LOD = 4.0;
vec3 radiance = textureLod(u_skyboxFiltered, R, roughness * MAX_REFLECTION_LOD).rgb;
vec2 brdfVec = vec2(dotNVClamped, roughness);
vec2 brdf  = texture(u_brdfTexture, brdfVec).rg;
if(lightPassData.lightSubScater == 0)
{
vec3 kD = 1.0 - kS;
kD *= 1.0 - metallic;
vec3 diffuse = irradiance * albedo;
vec3 specular = radiance * (F * brdf.x + brdf.y);
ambient = (kD * diffuse + specular);
}else
{
vec3 FssEss = kS * brdf.x + brdf.y;
float Ess = brdf.x + brdf.y;
float Ems = 1-Ess;
vec3 Favg = F0 + (1-F0)/21;
vec3 Fms = FssEss*Favg/(1-(1-Ess)*Favg);
vec3 Edss = 1 - (FssEss + Fms * Ems);
vec3 kD = albedo * Edss;
ambient = FssEss * radiance + (Fms*Ems+kD) * irradiance;
}
vec3 occlusionData = ambientOcclution * lightPassData.ambientColor.rgb;
ambient *= occlusionData;
}else
{
vec3 N = normal;
vec3 V = viewDir;
float dotNVClamped = clamp(dot(N, V), 0.0, 0.99);
vec3 F = fresnelSchlickRoughness(dotNVClamped, F0, roughness);
vec3 kS = F;
vec3 irradiance = lightPassData.ambientColor.rgb; //this color is coming directly at the object
vec3 radiance = lightPassData.ambientColor.rgb;
vec2 brdfVec = vec2(dotNVClamped, roughness);
vec2 brdf  = texture(u_brdfTexture, brdfVec).rg;
if(lightPassData.lightSubScater == 0)
{
vec3 kD = 1.0 - kS;
kD *= 1.0 - metallic;
vec3 diffuse = irradiance * albedo;
vec3 specular = radiance * (F * brdf.x + brdf.y);
ambient = (kD * diffuse + specular);
}else
{
vec3 FssEss = kS * brdf.x + brdf.y;
float Ess = brdf.x + brdf.y;
float Ems = 1-Ess;
vec3 Favg = F0 + (1-F0)/21;
vec3 Fms = FssEss*Favg/(1-(1-Ess)*Favg);
vec3 Edss = 1 - (FssEss + Fms * Ems);
vec3 kD = albedo * Edss;
ambient = FssEss * radiance + (Fms*Ems+kD) * irradiance;
}
vec3 occlusionData = vec3(ambientOcclution);
ambient *= occlusionData;
}
vec3 color = Lo + ambient; 
vec3 hdrCorrectedColor = color;
hdrCorrectedColor.rgb = vec3(1.0) - exp(-hdrCorrectedColor.rgb  * lightPassData.exposure);
hdrCorrectedColor.rgb = pow(hdrCorrectedColor.rgb, vec3(1.0/2.2));
float lightIntensity = dot(hdrCorrectedColor.rgb, vec3(0.2126, 0.7152, 0.0722));	
if(lightIntensity > lightPassData.bloomTresshold)
{
a_outBloom = vec4(color.rgb, 0) + vec4(emissive.rgb, 0);
a_outColor = vec4(0,0,0, albedoAlpha.a);	
}else
{
a_outBloom = vec4(emissive.rgb, 0);
a_outColor = vec4(color.rgb, albedoAlpha.a);
}
a_outBloom = clamp(a_outBloom, 0, 1000);
})"},

      std::pair<std::string, const char*>{"geometryPass.vert", R"(#version 330
#pragma debug(on)
layout(location = 0) in vec3 a_positions;
layout(location = 1) in vec3 a_normals;
layout(location = 2) in vec2 a_texCoord;
uniform mat4 u_transform; //full model view projection
uniform mat4 u_modelTransform; //just model to world
uniform mat4 u_motelViewTransform; //model to world to view
out vec3 v_normals;
out vec3 v_position;
out vec2 v_texCoord;
out vec3 v_positionViewSpace;
void main()
{
v_positionViewSpace = vec3(u_motelViewTransform * vec4(a_positions, 1.f));
gl_Position = u_transform * vec4(a_positions, 1.f);
v_position = (u_modelTransform * vec4(a_positions,1)).xyz;
v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * a_normals;  //non uniform scale
v_normals = normalize(v_normals);
v_texCoord = a_texCoord;
})"},

      std::pair<std::string, const char*>{"geometryPass.frag", R"(#version 430
#pragma debug(on)
layout(location = 0) out vec3 a_pos;
layout(location = 1) out vec3 a_normal;
layout(location = 2) out vec4 a_outColor;
layout(location = 3) out vec3 a_material;
layout(location = 4) out vec3 a_posViewSpace;
layout(location = 5) out vec3 a_emmisive;
in vec3 v_normals;
in vec3 v_position;	//world space
in vec2 v_texCoord;
in vec3 v_positionViewSpace;
uniform sampler2D u_albedoSampler;
uniform sampler2D u_normalSampler;
uniform sampler2D u_RMASampler;
uniform sampler2D u_emissiveTexture;
uniform int u_materialIndex;
struct MaterialStruct
{
vec4 kd;
vec4 rma; //last component emmisive
};
readonly layout(std140) buffer u_material
{
MaterialStruct mat[];
};
float PI = 3.14159265359;
mat3x3 NormalToRotation(in vec3 normal)
{
vec3 tangent0 = cross(normal, vec3(1, 0, 0));
if (dot(tangent0, tangent0) < 0.001)
tangent0 = cross(normal, vec3(0, 1, 0));
tangent0 = normalize(tangent0);
vec3 tangent1 = normalize(cross(normal, tangent0));
return mat3x3(tangent0,tangent1,normal);
}
subroutine vec3 GetNormalMapFunc(vec3);
subroutine (GetNormalMapFunc) vec3 normalMapped(vec3 v)
{
vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
normal = normalize(2*normal - 1.f);
mat3 rotMat = NormalToRotation(v);
normal = rotMat * normal;
normal = normalize(normal);
return normal;
}
subroutine (GetNormalMapFunc) vec3 noNormalMapped(vec3 v)
{
return v;
}
subroutine uniform GetNormalMapFunc getNormalMapFunc;
subroutine vec3 GetEmmisiveFunc(vec3);
subroutine (GetEmmisiveFunc) vec3 sampledEmmision(vec3 color)
{
return texture2D(u_emissiveTexture, v_texCoord).rgb;
}
subroutine (GetEmmisiveFunc) vec3 notSampledEmmision(vec3 color)
{
return color * mat[u_materialIndex].rma.a;
}
subroutine uniform GetEmmisiveFunc u_getEmmisiveFunc;
subroutine vec4 GetAlbedoFunc();
subroutine (GetAlbedoFunc) vec4 sampledAlbedo()
{
vec4 color = texture2D(u_albedoSampler, v_texCoord).xyzw;
if(color.w <= 0.1)
discard;
color.rgb *= pow( vec3(mat[u_materialIndex].kd.r, mat[u_materialIndex].kd.g, mat[u_materialIndex].kd.b), vec3(1.0/2.2) );
return color;
}
subroutine (GetAlbedoFunc) vec4 notSampledAlbedo()
{
vec4 c = vec4(mat[u_materialIndex].kd.r, mat[u_materialIndex].kd.g, mat[u_materialIndex].kd.b, 1);	
return c;
}
subroutine uniform GetAlbedoFunc u_getAlbedo;
subroutine vec3 GetMaterialMapped();
subroutine (GetMaterialMapped) vec3 materialNone()
{
return vec3(mat[u_materialIndex].rma.x, mat[u_materialIndex].rma.y, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialR()
{
float r = texture2D(u_RMASampler, v_texCoord).r;
return vec3(r, mat[u_materialIndex].rma.y, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialM()
{
float m = texture2D(u_RMASampler, v_texCoord).r;
return vec3(mat[u_materialIndex].rma.x, m, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialA()
{
float a = texture2D(u_RMASampler, v_texCoord).r;
return vec3(mat[u_materialIndex].rma.x, mat[u_materialIndex].rma.y, a);
}
subroutine (GetMaterialMapped) vec3 materialRM()
{
vec2 v = texture2D(u_RMASampler, v_texCoord).rg;
return vec3(v.x, v.y, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialRA()
{
vec2 v = texture2D(u_RMASampler, v_texCoord).rb;
return vec3(v.x, mat[u_materialIndex].rma.y, v.y);
}
subroutine (GetMaterialMapped) vec3 materialMA()
{
vec2 v = texture2D(u_RMASampler, v_texCoord).gb;
return vec3(mat[u_materialIndex].rma.x, v.x, v.y);
}
subroutine (GetMaterialMapped) vec3 materialRMA()
{
return texture2D(u_RMASampler, v_texCoord).rgb;
}
subroutine uniform GetMaterialMapped u_getMaterialMapped;
void main()
{
vec4 color  = u_getAlbedo(); //texture color
if(color.a < 0.1)discard;
vec3 sampledMaterial = u_getMaterialMapped();
float roughnessSampled = sampledMaterial.r;
float metallicSampled = sampledMaterial.g;
float sampledAo = sampledMaterial.b;
vec3 noMappedNorals = normalize(v_normals);
vec3 normal = getNormalMapFunc(noMappedNorals);
a_pos = v_position;
a_normal = normalize(normal);
a_outColor = vec4(clamp(color.rgb, 0, 1), 1);
a_material = vec3(roughnessSampled, metallicSampled, sampledAo);
a_posViewSpace = v_positionViewSpace;
a_emmisive = u_getEmmisiveFunc(a_outColor.rgb);
})"},

      std::pair<std::string, const char*>{"noaa.frag", R"(#version 150
out vec4 a_color;
in vec2 v_texCoords;
uniform sampler2D u_texture;
void main ()
{
vec4 tmpvar_1;
tmpvar_1.w = 1.0;
tmpvar_1.xyz = texture (u_texture, v_texCoords).xyz;
a_color = tmpvar_1;
})"},

      std::pair<std::string, const char*>{"fxaa.frag", R"(#version 150 core
out vec4 a_color;
in vec2 v_texCoords;
uniform sampler2D u_texture;
float luminance(in vec3 color)
{
return dot(color, vec3(0.299, 0.587, 0.114));
}
float lumaSqr(in vec3 color)
{
return sqrt(luminance(color));
}
vec3 getTexture(in vec2 offset)
{
return texture2D(u_texture, v_texCoords + offset).rgb;
}
float quality(int i)
{
const float r[7] = float[7](1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
if(i < 5)
{
return 1;
}else if(i >= 12)
{
return 8;
}else return r[i-5];
}
void main()
{
float edgeMinTreshold = 0.0312;
float edgeDarkTreshold = 0.125;
int ITERATIONS = 12;
float SUBPIXEL_QUALITY = 0.75;
vec3 colorCenter = getTexture(vec2(0,0)).rgb;
float lumaCenter = lumaSqr(colorCenter);
float lumaDown = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(0,-1)).rgb);
float lumaUp = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(0,1)).rgb);
float lumaLeft = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(-1,0)).rgb);
float lumaRight = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(1,0)).rgb);
float lumaMin = min(lumaCenter,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));
float lumaMax = max(lumaCenter,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));
float lumaRange = lumaMax - lumaMin;
if(lumaRange < max(edgeMinTreshold,lumaMax*edgeDarkTreshold))
{
a_color = vec4(colorCenter, 1);
return;
}
float lumaDownLeft = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(-1,-1)).rgb);
float lumaUpRight = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(1,1)).rgb);
float lumaUpLeft = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(-1,1)).rgb);
float lumaDownRight = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(1,-1)).rgb);
float lumaDownUp = lumaDown + lumaUp;
float lumaLeftRight = lumaLeft + lumaRight;
float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
float lumaDownCorners = lumaDownLeft + lumaDownRight;
float lumaRightCorners = lumaDownRight + lumaUpRight;
float lumaUpCorners = lumaUpRight + lumaUpLeft;
float edgeHorizontal =  abs(-2.0 * lumaLeft + lumaLeftCorners)  + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0    + abs(-2.0 * lumaRight + lumaRightCorners);
float edgeVertical =    abs(-2.0 * lumaUp + lumaUpCorners)      + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);
bool isHorizontal = (edgeHorizontal >= edgeVertical);
float luma1 = isHorizontal ? lumaDown : lumaLeft;
float luma2 = isHorizontal ? lumaUp : lumaRight;
float gradient1 = luma1 - lumaCenter;
float gradient2 = luma2 - lumaCenter;
bool is1Steepest = abs(gradient1) >= abs(gradient2);
float gradientScaled = 0.25*max(abs(gradient1),abs(gradient2));
vec2 inverseScreenSize = 1.f/textureSize(u_texture, 0);
float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
float lumaLocalAverage = 0.0;
if(is1Steepest)
{
stepLength = - stepLength;
lumaLocalAverage = 0.5*(luma1 + lumaCenter);
} 
else
{
lumaLocalAverage = 0.5*(luma2 + lumaCenter);
}
vec2 currentUv = v_texCoords;
if(isHorizontal)
{
currentUv.y += stepLength * 0.5;
} else 
{
currentUv.x += stepLength * 0.5;
}
vec2 offset = isHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);
vec2 uv1 = currentUv - offset;
vec2 uv2 = currentUv + offset;
float lumaEnd1 = lumaSqr(texture(u_texture,uv1).rgb);
float lumaEnd2 = lumaSqr(texture(u_texture,uv2).rgb);
lumaEnd1 -= lumaLocalAverage;
lumaEnd2 -= lumaLocalAverage;
bool reached1 = abs(lumaEnd1) >= gradientScaled;
bool reached2 = abs(lumaEnd2) >= gradientScaled;
bool reachedBoth = reached1 && reached2;
if(!reached1){
uv1 -= offset;
}
if(!reached2){
uv2 += offset;
}   
if(!reachedBoth)
{
for(int i = 2; i < ITERATIONS; i++){
if(!reached1){
lumaEnd1 = lumaSqr(texture(u_texture, uv1).rgb);
lumaEnd1 = lumaEnd1 - lumaLocalAverage;
}
if(!reached2){
lumaEnd2 = lumaSqr(texture(u_texture, uv2).rgb);
lumaEnd2 = lumaEnd2 - lumaLocalAverage;
}
reached1 = abs(lumaEnd1) >= gradientScaled;
reached2 = abs(lumaEnd2) >= gradientScaled;
reachedBoth = reached1 && reached2;
if(!reached1)
{
uv1 -= offset * quality(i);
}
if(!reached2)
{
uv2 += offset * quality(i);
}
if(reachedBoth){ break;}
}
}
float distance1 = isHorizontal ? (v_texCoords.x - uv1.x) : (v_texCoords.y - uv1.y);
float distance2 = isHorizontal ? (uv2.x - v_texCoords.x) : (uv2.y - v_texCoords.y);
bool isDirection1 = distance1 < distance2;
float distanceFinal = min(distance1, distance2);
float edgeThickness = (distance1 + distance2);
float pixelOffset = - distanceFinal / edgeThickness + 0.5;
bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
float finalOffset = correctVariation ? pixelOffset : 0.0;
float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);
float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;
finalOffset = max(finalOffset,subPixelOffsetFinal);
vec2 finalUv = v_texCoords;
if(isHorizontal){
finalUv.y += finalOffset * stepLength;
} else {
finalUv.x += finalOffset * stepLength;
}
vec3 finalColor = texture(u_texture, finalUv).rgb;
a_color = vec4(finalColor, 1);
}
/*
void main()
{
float fxaaSpan = 2.0;
float fxaaReduceMin = 0.001;
float fxaaReduceMul = 0.1;
vec2 tSize = 1.f/textureSize(u_texture, 0).xy;
vec3 tL		= getTexture(vec2(-tSize.x,-tSize.y));
vec3 tR		= getTexture(vec2(tSize.x,-tSize.y));
vec3 mid	= getTexture(vec2(0,0));
vec3 bL		= getTexture(vec2(-tSize.x,tSize.y));
vec3 bR		= getTexture(vec2(tSize.x,tSize.y));
float lumaTL = luminance(tL);	
float lumaTR = luminance(tR);	
float lumaMid = luminance(mid);
float lumaBL = luminance(bL);
float lumaBR = luminance(bR);
float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * 0.25f * fxaaReduceMul, fxaaReduceMin);
vec2 dir;
dir.x = -((lumaTL + lumaTR)-(lumaBL + lumaBR));
dir.y = (lumaTL + lumaBL)-(lumaTR + lumaBR);
float minScale = 1.0/min(abs(dir.x), abs(dir.y)+dirReduce);
dir = clamp(vec2(-fxaaSpan), vec2(fxaaSpan), dir * minScale);
if(abs(dir).x < 0.1 && abs(dir).y < 0.1)
{
a_color = vec4(mid,1);
}else
{
dir *= tSize;
vec3 rezult1 = 0.5 * (
getTexture(dir * vec2(1.f/3.f - 0.5f))+
getTexture(dir * vec2(2.f/3.f - 0.5f))
);
vec3 rezult2 = rezult1*0.5 + 0.25 * (
getTexture(dir * vec2(-0.5f))+
getTexture(dir * vec2(0.5f))
);
float lumaRez2 = luminance(rezult2);
float lumaMin = min(min(min(min(lumaTL,lumaTR),lumaMid), lumaBL), lumaBR);
float lumaMax = max(max(max(max(lumaTL,lumaTR),lumaMid), lumaBL), lumaBR);
if(lumaRez2 < lumaMin || lumaRez2 > lumaMax)
{
a_color = vec4(rezult1,1);
}else
{
a_color = vec4(rezult2,1);
}
}
}
*/)"},

      std::pair<std::string, const char*>{"stencil.vert", R"(#version 330
#pragma debug(on)
in layout(location = 0) vec3 a_positions;
in layout(location = 1) vec3 a_normals;
uniform mat4 u_transform;
uniform mat4 u_modelTransform;
out vec3 v_normals;
out vec3 v_position;
void main()
{
gl_Position = u_transform * vec4(a_positions, 1.f);
v_position = (u_modelTransform * vec4(a_positions,1)).xyz;
v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * a_normals;  //non uniform scale
v_normals = normalize(v_normals);
})"},

      std::pair<std::string, const char*>{"stencil.frag", R"(#pragma once)"},

      std::pair<std::string, const char*>{"showNormals.vert", R"(#version 330
#pragma debug(on)
in layout(location = 0) vec3 a_positions;
in layout(location = 1) vec3 a_normals;
uniform mat4 u_modelTransform; //just model view
out vec3 v_normals;
void main()
{
gl_Position = u_modelTransform * vec4(a_positions, 1);
v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * a_normals;  //non uniform scale
v_normals = normalize(v_normals);
})"},

      std::pair<std::string, const char*>{"showNormals.geom", R"(#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;
in vec3 v_normals[];
uniform float u_size = 0.5;
uniform mat4 u_projection; //projection matrix
void emitNormal(int index)
{
gl_Position = u_projection * gl_in[index].gl_Position;
EmitVertex();
gl_Position = u_projection * (gl_in[index].gl_Position + vec4(v_normals[index],0) * u_size);
EmitVertex();
EndPrimitive();
}
void main()
{
emitNormal(0);
emitNormal(1);
emitNormal(2);
})"},

      std::pair<std::string, const char*>{"showNormals.frag", R"(#version 150
out vec4 a_outColor;
uniform vec3 u_color = vec3(0.7, 0.7, 0.1);
void main ()
{
vec4 tmpvar_1;
tmpvar_1.w = 1.0;
tmpvar_1.xyz = u_color;
a_outColor = tmpvar_1;
})"},

      std::pair<std::string, const char*>{"normals.vert", R"(#version 330
#pragma debug(on)
in layout(location = 0) vec3 a_positions;
in layout(location = 1) vec3 a_normals;
in layout(location = 2) vec2 a_texCoord;
uniform mat4 u_transform; //full model view projection
uniform mat4 u_modelTransform; //just model view
out vec3 v_normals;
out vec3 v_position;
out vec2 v_texCoord;
void main()
{
gl_Position = u_transform * vec4(a_positions, 1.f);
v_position = (u_modelTransform * vec4(a_positions,1)).xyz;
v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * a_normals;  //non uniform scale
v_normals = normalize(v_normals);
v_texCoord = a_texCoord;
})"},

      std::pair<std::string, const char*>{"normals.frag", R"(#version 430
#pragma debug(on)
#extension GL_NV_shadow_samplers_cube : enable
out layout(location = 0) vec4 a_outColor;
in vec3 v_normals;
in vec3 v_position;	//world space
in vec2 v_texCoord;
uniform vec3 u_eyePosition;
uniform sampler2D u_albedoSampler;
uniform sampler2D u_normalSampler;
uniform samplerCube u_skybox;
uniform float u_gama;
uniform sampler2D u_RMASampler;
uniform int u_materialIndex;
struct Pointlight
{
vec3 positions; // w component not used
float dist;
vec3 color; // w component not used
float strength;
};
readonly layout(std140) buffer u_pointLights
{
Pointlight light[];
};
uniform int u_pointLightCount;
struct MaterialStruct
{
vec4 kd;
vec4 rma;
};
readonly layout(std140) buffer u_material
{
MaterialStruct mat[];
};
vec3 normal; //the normal of the object (can be normal mapped or not)
vec3 noMappedNorals; //this is the original non normal mapped normal
vec3 viewDir;
float difuseTest;  // used to check if object is in the light
vec4 color; //texture color
float PI = 3.14159265359;
mat3x3 NormalToRotation(in vec3 normal)
{
vec3 tangent0 = cross(normal, vec3(1, 0, 0));
if (dot(tangent0, tangent0) < 0.001)
tangent0 = cross(normal, vec3(0, 1, 0));
tangent0 = normalize(tangent0);
vec3 tangent1 = normalize(cross(normal, tangent0));
return mat3x3(tangent0,tangent1,normal);
}
subroutine vec3 GetNormalMapFunc(vec3);
subroutine (GetNormalMapFunc) vec3 normalMapped(vec3 v)
{
vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
normal = normalize(2*normal - 1.f);
mat3 rotMat = NormalToRotation(v);
normal = rotMat * normal;
normal = normalize(normal);
return normal;
}
subroutine (GetNormalMapFunc) vec3 noNormalMapped(vec3 v)
{
return v;
}
subroutine uniform GetNormalMapFunc getNormalMapFunc;
subroutine vec4 GetAlbedoFunc();
subroutine (GetAlbedoFunc) vec4 sampledAlbedo()
{
color = texture2D(u_albedoSampler, v_texCoord).xyzw;
if(color.w <= 0.1)
discard;
color.rgb = pow(color.rgb, vec3(2.2,2.2,2.2)).rgb; //gamma corection
color *= vec4(mat[u_materialIndex].kd.r, mat[u_materialIndex].kd.g, mat[u_materialIndex].kd.b, 1); //(option) multiply texture by kd
return color;
}
subroutine (GetAlbedoFunc) vec4 notSampledAlbedo()
{
return vec4(mat[u_materialIndex].kd.r, mat[u_materialIndex].kd.g, mat[u_materialIndex].kd.b, 1);	
}
subroutine uniform GetAlbedoFunc u_getAlbedo;
subroutine vec3 GetMaterialMapped();
subroutine (GetMaterialMapped) vec3 materialNone()
{
return vec3(mat[u_materialIndex].rma.x, mat[u_materialIndex].rma.y, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialR()
{
float r = texture2D(u_RMASampler, v_texCoord).r;
return vec3(r, mat[u_materialIndex].rma.y, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialM()
{
float m = texture2D(u_RMASampler, v_texCoord).r;
return vec3(mat[u_materialIndex].rma.x, m, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialA()
{
float a = texture2D(u_RMASampler, v_texCoord).r;
return vec3(mat[u_materialIndex].rma.x, mat[u_materialIndex].rma.y, a);
}
subroutine (GetMaterialMapped) vec3 materialRM()
{
vec2 v = texture2D(u_RMASampler, v_texCoord).rg;
return vec3(v.x, v.y, mat[u_materialIndex].rma.z);
}
subroutine (GetMaterialMapped) vec3 materialRA()
{
vec2 v = texture2D(u_RMASampler, v_texCoord).rb;
return vec3(v.x, mat[u_materialIndex].rma.y, v.y);
}
subroutine (GetMaterialMapped) vec3 materialMA()
{
vec2 v = texture2D(u_RMASampler, v_texCoord).gb;
return vec3(mat[u_materialIndex].rma.x, v.x, v.y);
}
subroutine (GetMaterialMapped) vec3 materialRMA()
{
return texture2D(u_RMASampler, v_texCoord).rgb;
}
subroutine uniform GetMaterialMapped u_getMaterialMapped;
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
float a      = roughness*roughness;
float a2     = a*a;
float NdotH  = max(dot(N, H), 0.0);
float NdotH2 = NdotH*NdotH;
float denom = (NdotH2 * (a2 - 1.0) + 1.0);
denom = PI * denom * denom;
return  a2 / max(denom, 0.0000001);
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
float r = (roughness + 1.0);
float k = (r*r) / 8.0;
float num   = NdotV;
float denom = NdotV * (1.0 - k) + k;
return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
float NdotV = max(dot(N, V), 0.0);
float NdotL = max(dot(N, L), 0.0);
float ggx2  = GeometrySchlickGGX(NdotV, roughness);
float ggx1  = GeometrySchlickGGX(NdotL, roughness);
return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
vec3 computePointLightSource(vec3 lightPosition, float metallic, float roughness, in vec3 lightColor)
{
vec3 lightDirection = normalize(lightPosition - v_position);
vec3 halfwayVec = normalize(lightDirection + viewDir);
float dist = length(lightPosition - v_position);
float attenuation = 1.0 / pow(dist,2);
attenuation = 1; //(option) remove light attenuation
vec3 radiance = lightColor * attenuation; //here the first component is the light color
vec3 F0 = vec3(0.04); 
F0 = mix(F0, color.rgb, metallic);	//here color is albedo, metalic surfaces use albdeo
vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);
float NDF = DistributionGGX(normal, halfwayVec, roughness);       
float G   = GeometrySmith(normal, viewDir, lightDirection, roughness);   
float denominator = 4.0 * max(dot(normal, viewDir), 0.0)  
* max(dot(normal, lightDirection), 0.0);
vec3 specular     = (NDF * G * F) / max(denominator, 0.001);
vec3 kS = F; //this is the specular contribution
vec3 kD = vec3(1.0) - kS; //the difuse is the remaining specular
kD *= 1.0 - metallic;	//metallic surfaces are darker
float NdotL = max(dot(normal, lightDirection), 0.0);        
vec3 Lo = (kD * color.rgb / PI + specular) * radiance * NdotL;
return Lo;
}
void main()
{
vec3 sampledMaterial = u_getMaterialMapped();
float roughnessSampled = sampledMaterial.r;
roughnessSampled = max(0.50,roughnessSampled);
float metallicSampled = sampledMaterial.g;
float sampledAo = sampledMaterial.b;
{	//general data
color = u_getAlbedo();
noMappedNorals = normalize(v_normals);
normal = getNormalMapFunc(noMappedNorals);
viewDir = u_eyePosition - v_position;
viewDir = normalize(viewDir); //v
}
vec3 I = normalize(v_position - u_eyePosition); //looking direction (towards eye)
vec3 R = reflect(I, normal);	//reflected vector
vec3 skyBoxSpecular = textureCube(u_skybox, R).rgb;		//this is the reflected color
vec3 skyBoxDiffuse = textureCube(u_skybox, normal).rgb; //this color is coming directly to the object
vec3 Lo = vec3(0,0,0); //this is the accumulated light
for(int i=0; i<u_pointLightCount;i++)
{
vec3 lightPosition = light[i].positions.xyz;
vec3 lightColor = light[i].color.rgb;
Lo += computePointLightSource(lightPosition, metallicSampled, roughnessSampled, lightColor);
}
vec3 ambient = vec3(0.03) * color.rgb * sampledAo; //this value is made up
vec3 color   = Lo + ambient; 
float exposure = 1;
color = vec3(1.0) - exp(-color  * exposure);
color = pow(color, vec3(1.0/2.2));
a_outColor = clamp(vec4(color.rgb,1), 0, 1);
})"},

      std::pair<std::string, const char*>{"drawQuads.vert", R"(#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoords;
out vec2 v_texCoords;
void main()
{
v_texCoords = a_TexCoords;
gl_Position = vec4(a_Pos, 1.0);
})"},

      std::pair<std::string, const char*>{"drawDepth.frag", R"(#version 150
out vec4 outColor;
in vec2 v_texCoords;
uniform sampler2D u_depth;
void main ()
{
float tmpvar_1;
tmpvar_1 = texture (u_depth, v_texCoords).x;
vec4 tmpvar_2;
tmpvar_2.w = 1.0;
tmpvar_2.x = tmpvar_1;
tmpvar_2.y = tmpvar_1;
tmpvar_2.z = tmpvar_1;
outColor = tmpvar_2;
})"},

      std::pair<std::string, const char*>{"color.vert", R"(#version 330
#pragma debug(on)
in layout(location = 0) vec3 positions;
in layout(location = 1) vec3 colors;
uniform mat4 u_transform;
out vec3 v_colors;
void main()
{
gl_Position = u_transform * vec4(positions,1.f);
v_colors = colors;
} )"},

      std::pair<std::string, const char*>{"color.frag", R"(#version 150
out vec4 outColor;
in vec3 v_colors;
void main ()
{
vec4 tmpvar_1;
tmpvar_1.w = 1.0;
tmpvar_1.xyz = v_colors;
outColor = tmpvar_1;
})"},

        //std::pair test stuff...
	


	};
	
	GLint createShaderFromFile(const char* source, GLenum shaderType)
	{
		std::string newFileName;
		std::string strSource = source;
		newFileName.reserve(30);

		for(int i=strSource.size()-1; i >= 0; i--)
		{
			if (strSource[i] != '\\' && strSource[i] != '/') 
			{
				newFileName.insert(newFileName.begin(), strSource[i]);
			}
			else
			{
				break;
			}

		}

		auto rez = createShaderFromData(headerOnlyShaders[newFileName], shaderType);
		return rez;
	
	}

#else

	GLint createShaderFromFile(const char* source, GLenum shaderType)
	{
		std::ifstream file;
		file.open(source);

		if (!file.is_open())
		{
			std::cout << "Error openning file: " << source << "\n";
			return 0;
		}

		GLint size = 0;
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);

		char* fileContent = new char[size+1] {};

		file.read(fileContent, size);


		file.close();

		auto rez = createShaderFromData(fileContent, shaderType);

		delete[] fileContent;

		return rez;

	}

#endif






	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);


		if (vertexId == 0 || fragmentId == 0)
		{
			return 0;
		}

		id = glCreateProgram();

		glAttachShader(id, vertexId);
		glAttachShader(id, fragmentId);

		glLinkProgram(id);

		glDeleteShader(vertexId);
		glDeleteShader(fragmentId);

		GLint info = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(id, l, &l, message);

			std::cout << "Link error: " << message << "\n";

			delete[] message;

			glDeleteProgram(id);
			id = 0;
			return 0;
		}

		glValidateProgram(id);

		return true;
	}

	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *geometryShader, const char *fragmentShader)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
		auto geometryId = createShaderFromFile(geometryShader, GL_GEOMETRY_SHADER);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);

		if (vertexId == 0 || fragmentId == 0 || geometryId == 0)
		{
			return 0;
		}

		id = glCreateProgram();

		glAttachShader(id, vertexId);
		glAttachShader(id, geometryId);
		glAttachShader(id, fragmentId);

		glLinkProgram(id);

		glDeleteShader(vertexId);
		glDeleteShader(geometryId);
		glDeleteShader(fragmentId);

		GLint info = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(id, l, &l, message);

			std::cout << "Link error: " << message << "\n";

			delete[] message;

			glDeleteProgram(id);
			id = 0;
			return 0;
		}

		glValidateProgram(id);

		return true;
	}

	void Shader::bind()
	{
		glUseProgram(id);
	}

	void Shader::clear()
	{
		glDeleteProgram(id);
		id = 0;
	}

	GLint getUniformSubroutine(GLuint id, GLenum shaderType, const char *name)
	{
		GLint uniform = glGetSubroutineUniformLocation(id, shaderType, name);
		if (uniform == -1)
		{
			std::cout << "uniform subroutine error " << name << "\n";
		}
		return uniform;
	};

	GLint getUniform(GLuint id, const char *name)
	{
		GLint uniform = glGetUniformLocation(id, name);
		if (uniform == -1)
		{
			std::cout << "uniform error " << name << "\n";
		}
		return uniform;
	};

	GLuint getUniformBlock(GLuint id, const char *name)
	{
		GLuint uniform = glGetUniformBlockIndex(id, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "uniform block error " << name << "\n";
		}
		return uniform;
	};

	GLuint getUniformSubroutineIndex(GLuint id, GLenum shaderType, const char *name)
	{
		GLuint uniform = glGetSubroutineIndex(id, shaderType, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "uniform subroutine index error " << name << "\n";
		}
		return uniform;
	};

	GLuint getStorageBlockIndex(GLuint id, const char *name)
	{
		GLuint uniform = glGetProgramResourceIndex(id, GL_SHADER_STORAGE_BLOCK, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "storage block index error " << name << "\n";
		}
		return uniform;
	};


	//todo move
	void LightShader::create()
	{
	#pragma region brdf texture
		brdfTexture.loadTextureFromFile("resources/BRDFintegrationMap.png", TextureLoadQuality::leastPossible, 3);
		glBindTexture(GL_TEXTURE_2D, brdfTexture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#pragma endregion

		prePass.shader.loadShaderProgramFromFile("shaders/deferred/geometryPass.vert", "shaders/deferred/zPrePass.frag");
		prePass.u_transform = getUniform(prePass.shader.id, "u_transform");
		prePass.u_albedoSampler = getUniform(prePass.shader.id, "u_albedoSampler");
		prePass.u_hasTexture = getUniform(prePass.shader.id, "u_hasTexture");

		pointShadowShader.shader.loadShaderProgramFromFile("shaders/shadows/pointShadow.vert",
			"shaders/shadows/pointShadow.geom", "shaders/shadows/pointShadow.frag");
		pointShadowShader.u_albedoSampler = getUniform(pointShadowShader.shader.id, "u_albedoSampler");
		pointShadowShader.u_farPlane = getUniform(pointShadowShader.shader.id, "u_farPlane");
		pointShadowShader.u_hasTexture = getUniform(pointShadowShader.shader.id, "u_hasTexture");
		pointShadowShader.u_lightPos = getUniform(pointShadowShader.shader.id, "u_lightPos");
		pointShadowShader.u_shadowMatrices = getUniform(pointShadowShader.shader.id, "u_shadowMatrices");
		pointShadowShader.u_transform = getUniform(pointShadowShader.shader.id, "u_transform");
		pointShadowShader.u_lightIndex = getUniform(pointShadowShader.shader.id, "u_lightIndex");

		geometryPassShader.loadShaderProgramFromFile("shaders/deferred/geometryPass.vert", "shaders/deferred/geometryPass.frag");
		//geometryPassShader.bind();

		u_transform = getUniform(geometryPassShader.id, "u_transform");
		u_modelTransform = getUniform(geometryPassShader.id, "u_modelTransform");
		u_motelViewTransform = getUniform(geometryPassShader.id, "u_motelViewTransform");
		//normalShaderLightposLocation = getUniform(shader.id, "u_lightPosition");
		textureSamplerLocation = getUniform(geometryPassShader.id, "u_albedoSampler");
		normalMapSamplerLocation = getUniform(geometryPassShader.id, "u_normalSampler");
		//eyePositionLocation = getUniform(shader.id, "u_eyePosition");
		//skyBoxSamplerLocation = getUniform(textureSamplerLocation.id, "u_skybox");
		//gamaLocation = getUniform(shader.id, "u_gama");
		RMASamplerLocation = getUniform(geometryPassShader.id, "u_RMASampler");
		//pointLightCountLocation = getUniform(shader.id, "u_pointLightCount");
		materialIndexLocation = getUniform(geometryPassShader.id, "u_materialIndex");
		u_emissiveTexture = getUniform(geometryPassShader.id, "u_emissiveTexture");
		//pointLightBufferLocation = getUniform(shader.id, "u_pointLights");
		

		materialBlockLocation = getStorageBlockIndex(geometryPassShader.id, "u_material");
		glShaderStorageBlockBinding(geometryPassShader.id, materialBlockLocation, 0);
		glGenBuffers(1, &materialBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);


		lightingPassShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/deferred/lightingPass.frag");
		lightingPassShader.bind();

		light_u_albedo = getUniform(lightingPassShader.id, "u_albedo");
		light_u_normals = getUniform(lightingPassShader.id, "u_normals");
		light_u_skyboxFiltered = getUniform(lightingPassShader.id, "u_skyboxFiltered");
		light_u_positions = getUniform(lightingPassShader.id, "u_positions");
		light_u_materials = getUniform(lightingPassShader.id, "u_materials");
		light_u_eyePosition = getUniform(lightingPassShader.id, "u_eyePosition");
		light_u_pointLightCount = getUniform(lightingPassShader.id, "u_pointLightCount");
		light_u_directionalLightCount = getUniform(lightingPassShader.id, "u_directionalLightCount");
		light_u_spotLightCount = getUniform(lightingPassShader.id, "u_spotLightCount");
		light_u_ssao = getUniform(lightingPassShader.id, "u_ssao");
		light_u_view = getUniform(lightingPassShader.id, "u_view");
		light_u_skyboxIradiance = getUniform(lightingPassShader.id, "u_skyboxIradiance");
		light_u_brdfTexture = getUniform(lightingPassShader.id, "u_brdfTexture");
		light_u_emmisive = getUniform(lightingPassShader.id, "u_emmisive");
		light_u_cascades = getUniform(lightingPassShader.id, "u_cascades");
		light_u_spotShadows = getUniform(lightingPassShader.id, "u_spotShadows");
		light_u_pointShadows = getUniform(lightingPassShader.id, "u_pointShadows");

	#pragma region uniform buffer

		lightPassShaderData.u_lightPassData = glGetUniformBlockIndex(lightingPassShader.id, "u_lightPassData");
		glGenBuffers(1, &lightPassShaderData.lightPassDataBlockBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, lightPassShaderData.lightPassDataBlockBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(LightPassData), &lightPassUniformBlockCpuData, GL_DYNAMIC_DRAW);
		
		glUniformBlockBinding(lightingPassShader.id, lightPassShaderData.u_lightPassData, 1);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightPassShaderData.lightPassDataBlockBuffer);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	#pragma endregion

	#pragma region block buffers
		pointLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_pointLights");
		glShaderStorageBlockBinding(lightingPassShader.id, pointLightsBlockLocation, 1);
		glGenBuffers(1, &pointLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightsBlockBuffer);

		directionalLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_directionalLights");
		glShaderStorageBlockBinding(lightingPassShader.id, directionalLightsBlockLocation, 2);
		glGenBuffers(1, &directionalLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, directionalLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, directionalLightsBlockBuffer);

		spotLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_spotLights");
		glShaderStorageBlockBinding(lightingPassShader.id, spotLightsBlockLocation, 3);
		glGenBuffers(1, &spotLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, spotLightsBlockBuffer);

	#pragma endregion


		glGenVertexArrays(1, &quadDrawer.quadVAO);
		glBindVertexArray(quadDrawer.quadVAO);

		glGenBuffers(1, &quadDrawer.quadBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quadDrawer.quadBuffer);

		float quadVertices[] = {
		   // positions        // texture Coords
		   -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		   -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

		glBindVertexArray(0);

	}

	void LightShader::bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama,
		const MaterialValues &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		geometryPassShader.bind();
		
		this->setData(viewProjMat, transformMat, lightPosition, eyePosition, gama, 
			material, pointLights);
	
	}

	void LightShader::setData(const glm::mat4 &viewProjMat, 
		const glm::mat4 &transformMat, const glm::vec3 &lightPosition, const glm::vec3 &eyePosition,
		float gama, const MaterialValues &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		glUniformMatrix4fv(u_transform, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
		glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(textureSamplerLocation, 0);
		glUniform1i(normalMapSamplerLocation, 1);
		glUniform1i(skyBoxSamplerLocation, 2);
		glUniform1i(RMASamplerLocation, 3);

		if(pointLights.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(internal::GpuPointLight)
				,&pointLights[0], GL_STREAM_DRAW); 

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightsBlockBuffer);

		}

		//glUniform1fv(pointLightBufferLocation, pointLights.size() * 8, (float*)pointLights.data());

		glUniform1i(pointLightCountLocation, pointLights.size());

		setMaterial(material);
	}

	void LightShader::setMaterial(const MaterialValues &material)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(material)
			, &material, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, materialBlockBuffer);
		glUniform1i(materialIndexLocation, 0);
	}



	void LightShader::getSubroutines()
	{


		normalSubroutine_noMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"noNormalMapped");

		normalSubroutine_normalMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"normalMapped");

		
		//
		albedoSubroutine_sampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"sampledAlbedo");

		albedoSubroutine_notSampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"notSampledAlbedo");

		//
		emissiveSubroutine_sampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"sampledEmmision");

		emissiveSubroutine_notSampled= getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"notSampledEmmision");


		//	
		normalSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"getNormalMapFunc");

		materialSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getMaterialMapped");

		getAlbedoSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getAlbedo");

		getEmmisiveSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getEmmisiveFunc");

		const char *materiaSubroutineFunctions[8] = { 
			"materialNone",
			"materialR",
			"materialM",
			"materialA",
			"materialRM",
			"materialRA",
			"materialMA",
			"materialRMA" };

		for(int i=0; i<8; i++)
		{
			materialSubroutine_functions[i] = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				materiaSubroutineFunctions[i]);
		}


	}

};

#pragma endregion


////////////////////////////////////////////////
//Camera.cpp
////////////////////////////////////////////////
#pragma region Camera



#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>


namespace gl3d
{

	glm::mat4x4 Camera::getProjectionMatrix()
	{
		auto mat = glm::perspective(this->fovRadians, this->aspectRatio, this->closePlane,
		this->farPlane);

		return mat;
	}

	glm::mat4x4 Camera::getWorldToViewMatrix()
	{
		glm::vec3 lookingAt = this->position;
		lookingAt += viewDirection;


		auto mat = glm::lookAt(this->position, lookingAt, this->up);
		return mat;
	}

	void Camera::rotateCamera(const glm::vec2 delta)
	{

		glm::vec3 rotateYaxe = glm::cross(viewDirection, up);

		viewDirection = glm::mat3(glm::rotate(delta.x, up)) * viewDirection;

		if (delta.y < 0)
		{	//down
			if (viewDirection.y < -0.99)
				goto noMove;
		}
		else
		{	//up
			if (viewDirection.y > 0.99)
				goto noMove;
		}

		viewDirection = glm::mat3(glm::rotate(delta.y, rotateYaxe)) * viewDirection;
	noMove:

		viewDirection = glm::normalize(viewDirection);
	}


	void generateTangentSpace(glm::vec3 v, glm::vec3& outUp, glm::vec3& outRight)
	{
		glm::vec3 up(0, 1, 0);

		if (v == up)
		{
			outRight = glm::vec3(1, 0, 0);
		}
		else
		{
			outRight = normalize(glm::cross(v, up));
		}

		outUp = normalize(cross(outRight, v));

	}

	//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
	void computeFrustumDimensions(
		glm::vec3 position, glm::vec3 viewDirection,
		float fovRadians, float aspectRatio, float nearPlane, float farPlane, 
		glm::vec2& nearDimensions, glm::vec2& farDimensions, glm::vec3& centerNear, glm::vec3& centerFar)
	{
		float tanFov2 = tan(fovRadians) * 2;

		nearDimensions.y = tanFov2 * nearPlane;			//hNear
		nearDimensions.x = nearDimensions.y * aspectRatio;	//wNear

		farDimensions.y = tanFov2 * farPlane;			//hNear
		farDimensions.x = farDimensions.y * aspectRatio;	//wNear

		centerNear = position + viewDirection * farPlane;
		centerFar  = position + viewDirection * farPlane;

	}

	//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
	void computeFrustumSplitCorners(glm::vec3 directionVector, 
		glm::vec2 nearDimensions, glm::vec2 farDimensions, glm::vec3 centerNear, glm::vec3 centerFar, 
		glm::vec3& nearTopLeft, glm::vec3& nearTopRight, glm::vec3& nearBottomLeft, glm::vec3& nearBottomRight, 
		glm::vec3& farTopLeft, glm::vec3& farTopRight, glm::vec3& farBottomLeft, glm::vec3& farBottomRight)
	{

		glm::vec3 rVector = {};
		glm::vec3 upVectpr = {};

		generateTangentSpace(directionVector, upVectpr, rVector);

		nearTopLeft = centerNear + upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
		nearTopRight = centerNear + upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;
		nearBottomLeft = centerNear - upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
		nearBottomRight = centerNear - upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;

		farTopLeft = centerNear + upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
		farTopRight = centerNear + upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;
		farBottomLeft = centerNear - upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
		farBottomRight = centerNear - upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;

	}

	glm::vec3 fromAnglesToDirection(float zenith, float azimuth)
	{
		glm::vec4 vec(0, 1, 0, 0);

		auto zenithRotate = glm::rotate(-zenith, glm::vec3( 1.f,0.f,0.f ));
		vec = zenithRotate * vec;

		auto azimuthRotate = glm::rotate(-azimuth, glm::vec3(0.f, 1.f, 0.f));
		vec = azimuthRotate * vec;

		return glm::normalize(glm::vec3(vec));
	}

	glm::vec2 fromDirectionToAngles(glm::vec3 direction)
	{
		if (direction == glm::vec3(0, 1, 0))
		{
			return glm::vec2(0, 0);
		}
		else
		{
			glm::vec3 zenith(0, 1, 0);
			float zenithCos = glm::dot(zenith, direction);
			float zenithAngle = std::acos(zenithCos);
			
			glm::vec3 north(0, 0, -1);
			glm::vec3 projectedVector(direction.x, 0, direction.z);
			projectedVector = glm::normalize(projectedVector);

			float azmuthCos = glm::dot(north, projectedVector);
			float azmuthAngle = std::acos(azmuthCos);

			return glm::vec2(zenithAngle, azmuthAngle);
		}

	}

	

	void Camera::moveFPS(glm::vec3 direction)
	{
		viewDirection = glm::normalize(viewDirection);

		//forward
		float forward = -direction.z;
		float leftRight = direction.x;
		float upDown = direction.y;

		glm::vec3 move = {};

		move += up * upDown;
		move += glm::normalize(glm::cross(viewDirection, up)) * leftRight;
		move += viewDirection * forward;

		this->position += move;
	
	}


	
};
#pragma endregion


////////////////////////////////////////////////
//GraphicModel.cpp
////////////////////////////////////////////////
#pragma region GraphicModel



#include <stb_image.h>

#include <algorithm>


namespace gl3d 
{

	void DebugGraphicModel::loadFromComputedData(size_t vertexSize, const float * vercies, size_t indexSize,
		const unsigned int * indexes, bool noTexture)
	{

		gl3dAssertComment(indexSize % 3 == 0, "Index count must be multiple of 3");
		if (indexSize % 3 != 0)return;


		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexSize, vercies, GL_STATIC_DRAW);

		if(noTexture)
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
		}else
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
		
		}


		if (indexSize && indexes)
		{
			glGenBuffers(1, &indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexes, GL_STATIC_DRAW);

			primitiveCount = indexSize / sizeof(*indexes);

		}
		else
		{
			primitiveCount = vertexSize / sizeof(float);
		}
	
		glBindVertexArray(0);

	}

	
	void DebugGraphicModel::clear()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &vertexArray);

		vertexBuffer = 0;
		indexBuffer = 0;
		primitiveCount = 0;
		vertexArray = 0;
	}

	void DebugGraphicModel::draw()
	{
		glBindVertexArray(vertexArray);

		//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)(3 * sizeof(float)));

		if (indexBuffer)
		{
			glDrawElements(GL_TRIANGLES, primitiveCount, GL_UNSIGNED_INT, 0);

		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, primitiveCount);
		}

		glBindVertexArray(0);

	}

	glm::mat4 DebugGraphicModel::getTransformMatrix()
	{
		return gl3d::getTransformMatrix(position, rotation, scale);

	}


	void LoadedModelData::load(const char *file, float scale)
	{
		loader.LoadFile(file);

		//parse path
		path = file;
		while (!path.empty() &&
			*(path.end() - 1) != '\\' &&
			*(path.end() - 1) != '/'
			)
		{
			path.pop_back();
		}

		for (auto &i : loader.LoadedMeshes)
		{
			for(auto &j : i.Vertices)
			{
				j.Position.X *= scale;
				j.Position.Y *= scale;
				j.Position.Z *= scale;
			}
		}

		for (auto &j : loader.LoadedVertices)
		{
			j.Position.X *= scale;
			j.Position.Y *= scale;
			j.Position.Z *= scale;
		}

		std::cout << "Loaded: " << loader.LoadedMeshes.size() << " meshes\n";
	}

	float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
	};

	
	static glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	static glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
	


	//todo optimize
	glm::mat4 getTransformMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		auto s = glm::scale(scale);
		auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(rotation.z, glm::vec3(0, 0, 1));
		auto t = glm::translate(position);

		return t * r * s;
	}

	glm::mat4 getTransformMatrix(const Transform& t)
	{
		return getTransformMatrix(t.position, t.rotation, t.scale);
	}

	glm::mat4 Transform::getTransformMatrix()
	{
		return gl3d::getTransformMatrix(*this);
	}


	void ModelData::clear(Renderer3D& renderer)
	{
		for (auto &i : models)
		{
			i.clear();
		}

		for (auto &i : subModelsNames)
		{
			delete[] i;
		}
		subModelsNames.clear();
		
		for (auto& i : createdMaterials)
		{
			renderer.deleteMaterial(i);
		}
		createdMaterials.clear();

		models.clear();

	}

	

	void GraphicModel::loadFromComputedData(size_t vertexSize, const float *vercies, size_t indexSize, const unsigned int *indexes, bool noTexture)
	{

		gl3dAssertComment(indexSize % 3 == 0, "Index count must be multiple of 3");
		if (indexSize % 3 != 0)return;

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexSize, vercies, GL_STATIC_DRAW);

		//todo this is only for rendering gizmos stuff
		if (noTexture)
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
		}
		else
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

		}


		if (indexSize && indexes)
		{
			glGenBuffers(1, &indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexes, GL_STATIC_DRAW);

			primitiveCount = indexSize / sizeof(*indexes);

		}
		else
		{
			primitiveCount = vertexSize / sizeof(float);
		}

		glBindVertexArray(0);


	}

	void GraphicModel::clear()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &vertexArray);

		//albedoTexture.clear();
		//normalMapTexture.clear();
		//RMA_Texture.clear();

		vertexBuffer = 0;
		indexBuffer = 0;
		primitiveCount = 0;
		vertexArray = 0;
	}

	void SkyBoxLoaderAndDrawer::createGpuData()
	{
		normalSkyBox.shader.loadShaderProgramFromFile("shaders/skyBox/skyBox.vert", "shaders/skyBox/skyBox.frag");
		normalSkyBox.samplerUniformLocation = getUniform(normalSkyBox.shader.id, "u_skybox");
		normalSkyBox.modelViewUniformLocation = getUniform(normalSkyBox.shader.id, "u_viewProjection");
		normalSkyBox.u_exposure = getUniform(normalSkyBox.shader.id, "u_exposure");
		normalSkyBox.u_ambient = getUniform(normalSkyBox.shader.id, "u_ambient");
		normalSkyBox.u_skyBoxPresent = getUniform(normalSkyBox.shader.id, "u_skyBoxPresent");
		
		hdrtoCubeMap.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/hdrToCubeMap.frag");
		hdrtoCubeMap.u_equirectangularMap = getUniform(hdrtoCubeMap.shader.id, "u_equirectangularMap");
		hdrtoCubeMap.modelViewUniformLocation = getUniform(hdrtoCubeMap.shader.id, "u_viewProjection");

		convolute.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/convolute.frag");
		convolute.u_environmentMap = getUniform(convolute.shader.id, "u_environmentMap");
		convolute.modelViewUniformLocation = getUniform(convolute.shader.id, "u_viewProjection");

		preFilterSpecular.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/preFilterSpecular.frag");
		preFilterSpecular.modelViewUniformLocation = getUniform(preFilterSpecular.shader.id, "u_viewProjection");
		preFilterSpecular.u_environmentMap = getUniform(preFilterSpecular.shader.id, "u_environmentMap");
		preFilterSpecular.u_roughness = getUniform(preFilterSpecular.shader.id, "u_roughness");

		atmosphericScatteringShader.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert",
			"shaders/skyBox/atmosphericScattering.frag");
		//atmosphericScatteringShader.u_lightPos = getUniform(atmosphericScatteringShader.shader.id, "u_lightPos");
		//atmosphericScatteringShader.u_g = getUniform(atmosphericScatteringShader.shader.id, "u_g");
		//atmosphericScatteringShader.u_g2 = getUniform(atmosphericScatteringShader.shader.id, "u_g2");
		atmosphericScatteringShader.modelViewUniformLocation 
			= getUniform(atmosphericScatteringShader.shader.id, "u_viewProjection");


		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);


		glBindVertexArray(0);

		glGenFramebuffers(1, &captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void SkyBoxLoaderAndDrawer::loadTexture(const char *names[6], SkyBox &skyBox)
	{
		skyBox = {};

		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		for (unsigned int i = 0; i < 6; i++)
		{
			int w, h, nrChannels;
			unsigned char *data;

			stbi_set_flip_vertically_on_load(false);
			data = stbi_load(names[i], &w, &h, &nrChannels, 3);

			if (data)
			{

				glTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
							0, GL_SRGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);


				stbi_image_free(data);
			}
			else
			{
				std::cout << "err loading " << names[i] << "\n";
				glDeleteTextures(1, &skyBox.texture);
				return;
			}


		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		createConvolutedAndPrefilteredTextureData(skyBox);
	}

	void SkyBoxLoaderAndDrawer::loadTexture(const char *name, SkyBox &skyBox, int format)
	{
		skyBox = {};

		int width, height, nrChannels;
		unsigned char *data;


		stbi_set_flip_vertically_on_load(false);
		data = stbi_load(name, &width, &height, &nrChannels, 3);

		if (!data) { std::cout << "err loading " << name << "\n"; return; }

		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		//right
		//left
		//top
		//bottom
		//front//?
		//back //?

		auto getPixel = [&](int x, int y, unsigned char *data)
		{
			return data + 3 * (x + y * width);
		};

		glm::ivec2 paddings[6];
		glm::ivec2 immageRatio = {};
		int flipX[6] = {};
		int flipY[6] = {};

		if (format == 0)
		{
			immageRatio = { 4, 3 };
			glm::ivec2 paddingscopy[6] =
			{
				{ (width / 4) * 2, (height / 3) * 1, },
				{ (width / 4) * 0, (height / 3) * 1, },
				{ (width / 4) * 1, (height / 3) * 0, },
				{ (width / 4) * 1, (height / 3) * 2, },
				{ (width / 4) * 1, (height / 3) * 1, },
				{ (width / 4) * 3, (height / 3) * 1, },
			};

			memcpy(paddings, paddingscopy, sizeof(paddings));


		}
		else if (format == 1)
		{
			immageRatio = { 3, 4 };
			glm::ivec2 paddingscopy[6] =
			{
				{ (width / 3) * 2, (height / 4) * 1, },
				{ (width / 3) * 0, (height / 4) * 1, },
				{ (width / 3) * 1, (height / 4) * 0, },
				{ (width / 3) * 1, (height / 4) * 2, },
				{ (width / 3) * 1, (height / 4) * 1, },
				{ (width / 3) * 1, (height / 4) * 3, },
			};

			memcpy(paddings, paddingscopy, sizeof(paddings));
			flipX[5] = 1;
			flipY[5] = 1;

		}else if(format == 2)
		{
			immageRatio = { 4, 3 };
			glm::ivec2 paddingscopy[6] =
			{
				{ (width / 4) * 3, (height / 3) * 1, },
				{ (width / 4) * 1, (height / 3) * 1, },
				{ (width / 4) * 2, (height / 3) * 0, },
				{ (width / 4) * 2, (height / 3) * 2, },
				{ (width / 4) * 2, (height / 3) * 1, },
				{ (width / 4) * 0, (height / 3) * 1, },

			};

			memcpy(paddings, paddingscopy, sizeof(paddings));

		}else
		{
			gl3dAssertComment(0, "invalid format for texture");
		}


		if (data)
		{
			for (unsigned int i = 0; i < 6; i++)
			{
				unsigned char *extractedData = new unsigned char[3 *
					(width / immageRatio.x) * (height / immageRatio.y)];

				int index = 0;

				int paddingX = paddings[i].x;
				int paddingY = paddings[i].y;

			#pragma region flip

				int jBeg = 0;
				int jAdvance = 1;
				if(flipY[i])
				{
					jBeg = height / immageRatio.y - 1;
					jAdvance = -1;
				}

				int xBeg = 0;
				int xAdvance = 1;
				if (flipX[i])
				{
					xBeg = width / immageRatio.x - 1;
					xAdvance = -1;
				}
			#pragma endregion


				for (int j = jBeg; j < height / immageRatio.y && j >= 0; j+= jAdvance)
					for (int i = xBeg; i < width / immageRatio.x && i >= 0; i+= xAdvance)
					{
						extractedData[index] = *getPixel(i + paddingX, j + paddingY, data);
						extractedData[index + 1] = *(getPixel(i + paddingX, j + paddingY, data) + 1);
						extractedData[index + 2] = *(getPixel(i + paddingX, j + paddingY, data) + 2);
						//extractedData[index] = 100;
						//extractedData[index + 1] = 100;
						//extractedData[index + 2] = 100;
						index += 3;
					}

				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_SRGB, width / immageRatio.x, height / immageRatio.y, 0,
					GL_RGB, GL_UNSIGNED_BYTE, extractedData
				);



				delete[] extractedData;
			}

			stbi_image_free(data);

		}
		else
		{
			std::cout << "err loading " << name << "\n";
		}


		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	
		createConvolutedAndPrefilteredTextureData(skyBox);

	}

	void SkyBoxLoaderAndDrawer::loadHDRtexture(const char *name, SkyBox &skyBox)
	{
		skyBox = {};

		int width, height, nrChannels;
		float *data;

		stbi_set_flip_vertically_on_load(true);
		data = stbi_loadf(name, &width, &height, &nrChannels, 0);
		if (!data) { std::cout << "err loading " << name << "\n"; return; }


		GLuint hdrTexture;

		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		//render into the cubemap
		//https://learnopengl.com/PBR/IBL/Diffuse-irradiance
		{
			GLuint captureFBO; //todo cache this
			glGenFramebuffers(1, &captureFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);


			glGenTextures(1, &skyBox.texture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
			for (unsigned int i = 0; i < 6; ++i)
			{
				// note that we store each face with 16 bit floating point values
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
							 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//rendering
			{

				hdrtoCubeMap.shader.bind();
				glUniform1i(hdrtoCubeMap.u_equirectangularMap, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, hdrTexture);

				glViewport(0, 0, 512, 512);

				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
				glBindVertexArray(vertexArray);

				for (unsigned int i = 0; i < 6; ++i)
				{
					glm::mat4 viewProjMat = captureProjection * captureViews[i];
					glUniformMatrix4fv(hdrtoCubeMap.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
										   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.texture, 0);
					glClear(GL_COLOR_BUFFER_BIT);

					glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
				}

				glBindVertexArray(0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

			}

			glDeleteFramebuffers(1, &captureFBO);

		}

		glDeleteTextures(1, &hdrTexture);

		createConvolutedAndPrefilteredTextureData(skyBox);
	}

	void SkyBoxLoaderAndDrawer::atmosphericScattering(glm::vec3 sun, float g, float g2, SkyBox& skyBox)
	{
		skyBox = {};

		//render into the cubemap
		{
			GLuint captureFBO; 
			glGenFramebuffers(1, &captureFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);


			glGenTextures(1, &skyBox.texture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
			for (unsigned int i = 0; i < 6; ++i)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, //todo 16 is probably too much
					512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//rendering
			{

				atmosphericScatteringShader.shader.bind();
				//glUniform3fv(atmosphericScatteringShader.u_lightPos, 1, &sun[0]);
				//glUniform1f(atmosphericScatteringShader.u_g, g);
				//glUniform1f(atmosphericScatteringShader.u_g2, g2);

				glViewport(0, 0, 512, 512);

				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
				glBindVertexArray(vertexArray);

				for (unsigned int i = 0; i < 6; ++i)
				{
					glm::mat4 viewProjMat = captureProjection * captureViews[i];
					glUniformMatrix4fv(atmosphericScatteringShader.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.texture, 0);
					glClear(GL_COLOR_BUFFER_BIT);

					glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
				}

				glBindVertexArray(0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

			}

			glDeleteFramebuffers(1, &captureFBO);

		}


		createConvolutedAndPrefilteredTextureData(skyBox);

	}

	void SkyBoxLoaderAndDrawer::createConvolutedAndPrefilteredTextureData(SkyBox &skyBox)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		GLint viewPort[4] = {};
		glGetIntegerv(GL_VIEWPORT, viewPort); //todo remove because slow


	#pragma region convoluted texture


		glGenTextures(1, &skyBox.convolutedTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.convolutedTexture);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
						 GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		convolute.shader.bind();
		glUniform1i(convolute.u_environmentMap, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glViewport(0, 0, 32, 32);

		glBindVertexArray(vertexArray);

		for (unsigned int i = 0; i < 6; ++i)
		{

			glm::mat4 viewProjMat = captureProjection * captureViews[i];
			glUniformMatrix4fv(convolute.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.convolutedTexture, 0);

			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube

		}
	#pragma endregion

	
	#pragma region prefiltered map

		constexpr int maxMipMap = 5;

		glGenTextures(1, &skyBox.preFilteredMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.preFilteredMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			//todo mabe be able to tweak rezolution
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipMap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		preFilterSpecular.shader.bind();
		glUniform1i(preFilterSpecular.u_environmentMap, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);


		for (int mip = 0; mip < maxMipMap; mip++)
		{
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipMap - 1);
			roughness *= roughness;
			glUniform1f(preFilterSpecular.u_roughness, roughness);

			for (int i = 0; i < 6; i++)
			{
				glm::mat4 viewProjMat = captureProjection * captureViews[i];
				glUniformMatrix4fv(preFilterSpecular.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
									   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.preFilteredMap, mip);
				glClear(GL_COLOR_BUFFER_BIT);

				glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
			}
		}



	#pragma endregion

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

		//texture = convolutedTexture; //visualize convolutex texture
		//texture = preFilteredMap;

		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//todo telete mipmaps
		//GLuint newTexture = 0;
		//glGenTextures(1, &newTexture);
		//int w, h;
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		//glCopyTexImage2D(GL_TEXTURE_CUBE_MAP, 0, GL_SRGB, 0, 0, w, h, 0);

	}

	void SkyBoxLoaderAndDrawer::draw(const glm::mat4 &viewProjMat, SkyBox &skyBox, float exposure,
		glm::vec3 ambient)
	{
		glBindVertexArray(vertexArray);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		normalSkyBox.shader.bind();

		glUniformMatrix4fv(normalSkyBox.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(normalSkyBox.samplerUniformLocation, 0);
		glUniform1f(normalSkyBox.u_exposure, exposure);
		glUniform3f(normalSkyBox.u_ambient, ambient.r, ambient.g, ambient.b);

		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
		glDepthFunc(GL_LESS);

		glBindVertexArray(0);
	}

	void SkyBoxLoaderAndDrawer::drawBefore(const glm::mat4& viewProjMat, SkyBox& skyBox, float exposure,
		glm::vec3 ambient)
	{
		glBindVertexArray(vertexArray);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		normalSkyBox.shader.bind();

		bool skyBoxPresent = 1;

		if (skyBox.texture == 0 || skyBox.convolutedTexture == 0 || skyBox.preFilteredMap == 0)
		{
			skyBoxPresent = 0;
		}

		glUniformMatrix4fv(normalSkyBox.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(normalSkyBox.samplerUniformLocation, 0);
		glUniform1i(normalSkyBox.u_skyBoxPresent, skyBoxPresent);
		glUniform1f(normalSkyBox.u_exposure, exposure);
		glUniform3f(normalSkyBox.u_ambient, ambient.r, ambient.g, ambient.b);


		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(0);
	}

	void SkyBox::clearTextures()
	{
		glDeleteTextures(3, (GLuint*)this);
		texture = 0;
		convolutedTexture = 0;
		preFilteredMap = 0;
	}

};

#pragma endregion


////////////////////////////////////////////////
//gl3d.cpp
////////////////////////////////////////////////
#pragma region gl3d


#include <algorithm>
#include <stb_image.h>
#include <random>
#include <string>

#ifdef _MSC_VER
//#pragma warning( disable : 4244 4305 4267 4996 4018)
#pragma warning( disable : 26812)
#endif

namespace gl3d
{
	

	void Renderer3D::init(int x, int y)
	{
		internal.w = x; internal.h = y;
		internal.adaptiveW = x;
		internal.adaptiveH = y;

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		internal.lightShader.create();
		vao.createVAOs();
		internal.skyBoxLoaderAndDrawer.createGpuData();


		showNormalsProgram.shader.loadShaderProgramFromFile("shaders/showNormals.vert",
		"shaders/showNormals.geom", "shaders/showNormals.frag");


		showNormalsProgram.modelTransformLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_modelTransform");
		showNormalsProgram.projectionLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_projection");
		showNormalsProgram.sizeLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_size");
		showNormalsProgram.colorLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_color");
		
		//unsigned char textureData[] =
		//{
		//	20, 20, 20, 255,
		//	212, 0, 219, 255,
		//	212, 0, 219, 255,
		//	20, 20, 20, 255,
		//};
		//defaultTexture.loadTextureFromMemory(textureData, 2, 2, 4, TextureLoadQuality::leastPossible);


		gBuffer.create(x, y);	
		ssao.create(x, y);
		postProcess.create(x, y);
		directionalShadows.create();
		spotShadows.create();
		pointShadows.create();
		renderDepthMap.create();
		antiAlias.create(x, y);
		adaptiveResolution.create(x, y);

		internal.pBRtextureMaker.init();
	}

	void Renderer3D::VAO::createVAOs()
	{
		glGenVertexArrays(1, &posNormalTexture);
		glBindVertexArray(posNormalTexture);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

		glBindVertexArray(0);

	}
	
	Material Renderer3D::createMaterial(glm::vec3 kd, float roughness, float metallic, float ao
	,std::string name)
	{

		int id = internal::generateNewIndex(internal.materialIndexes);

		MaterialValues gpuMaterial;
		gpuMaterial.kd = glm::vec4(kd, 0);
		gpuMaterial.roughness = roughness;
		gpuMaterial.metallic = metallic;
		gpuMaterial.ao = ao;

		internal.materialIndexes.push_back(id);
		internal.materials.push_back(gpuMaterial);
		internal.materialNames.push_back(name);
		internal.materialTexturesData.push_back({});

		Material m;
		m.id_ = id;
		return m;

	}

	Material Renderer3D::createMaterial(Material m)
	{
		auto newM = createMaterial();
		copyMaterialData(newM, m);

		return newM;
	}

	Material Renderer3D::loadMaterial(std::string file)
	{

		objl::Loader loader;
		loader.LoadFile(file);



		return Material();
	}

	bool Renderer3D::deleteMaterial(Material m)
	{
		auto pos = std::find(internal.materialIndexes.begin(), internal.materialIndexes.end(), m.id_);

		if (pos == internal.materialIndexes.end())
		{
			gl3dAssertComment(pos != internal.materialIndexes.end(), "invalid delete material");
			return 0;
		}

		int index = pos - internal.materialIndexes.begin();

		internal.materialIndexes.erase(pos);
		internal.materials.erase(internal.materials.begin() + index);
		internal.materialNames.erase(internal.materialNames.begin() + index);
		internal.materialTexturesData.erase(internal.materialTexturesData.begin() + index);
		m.id_ = 0;
		return 1;
	}

	bool Renderer3D::copyMaterialData(Material dest, Material source)
	{
		int destId = internal.getMaterialIndex(dest);
		int sourceId = internal.getMaterialIndex(source);

		if(destId == -1 || sourceId == -1)
		{
			gl3dAssertComment(destId != -1, "invaled dest material index");
			gl3dAssertComment(sourceId != -1, "invaled source material index");

			return 0;
		}

		internal.materials[destId] = internal.materials[sourceId];
		internal.materialNames[destId] = internal.materialNames[sourceId];
		internal.materialTexturesData[destId] = internal.materialTexturesData[destId];

		return 1;
	}

	MaterialValues Renderer3D::getMaterialValues(Material m)
	{
		int id = internal.getMaterialIndex(m);

		if(id == -1)
		{
			return {};
		}
		
		return internal.materials[id];
	}

	void Renderer3D::setMaterialValues(Material m, MaterialValues values)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return;
		}

		internal.materials[id] = values;

	}

	TextureDataForMaterial Renderer3D::getMaterialTextures(Material m)
	{
		int id = internal.getMaterialIndex(m);
		if (id == -1)
		{return {};}

		return internal.materialTexturesData[id];
	}

	void Renderer3D::setMaterialTextures(Material m, TextureDataForMaterial textures)
	{
		int id = internal.getMaterialIndex(m);
		if (id == -1)
		{return;}
		
		internal.materialTexturesData[id] = textures;
	}

	std::string Renderer3D::getMaterialName(Material m)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return "";
		}

		return internal.materialNames[id];

	}

	void Renderer3D::setMaterialName(Material m, const std::string& name)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{return;}

		internal.materialNames[id] = name;
	}

	bool Renderer3D::isMaterial(Material& m)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	//bool Renderer3D::setMaterialData(Material m, const MaterialValues &data, std::string *s)
	//{
	//	int id = internal.getMaterialIndex(m);
	//
	//	if (id == -1)
	//	{
	//		return 0;
	//	}
	//
	//	internal.materials[id] = data;
	//	
	//	if (s)
	//	{
	//		internal.materialNames[id] = *s;
	//	}
	//
	//	return 1;
	//}

	Texture Renderer3D::loadTexture(std::string path)
	{

		if(path == "")
		{
			return Texture{ 0 };
		}

		int pos = 0;
		for (auto &i : internal.loadedTexturesNames)
		{
			if (i == path)
			{
				Texture t;
				t.id_ = internal.loadedTexturesIndexes[pos];
				return t;
			}
			pos++;
		}

		GpuTexture t;
		internal::GpuTextureWithFlags text;
		int alphaExists = t.loadTextureFromFileAndCheckAlpha(path.c_str());

		text.texture = t;
		text.flags = alphaExists;

		//if texture is not loaded, return an invalid texture
		if(t.id == 0)
		{
			return Texture{ 0 };
		}

		int id = internal::generateNewIndex(internal.loadedTexturesIndexes);


		internal.loadedTexturesIndexes.push_back(id);
		internal.loadedTextures.push_back(text);
		internal.loadedTexturesNames.push_back(path);

		return Texture{ id };
	}

	GLuint Renderer3D::getTextureOpenglId(Texture& t)
	{
		auto p = getTextureData(t);

		if(p == nullptr)
		{
			return 0;
		}else
		{
			return p->id;
		}
	}

	bool Renderer3D::isTexture(Texture& t)
	{
		int i = internal.getTextureIndex(t);

		if (i > -1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void Renderer3D::deleteTexture(Texture& t)
	{
		int index = internal.getTextureIndex(t);

		if(index < 0)
		{
			return;
		}

		auto gpuTexture = internal.loadedTextures[index];

		internal.loadedTexturesIndexes.erase(internal.loadedTexturesIndexes.begin() + index);
		internal.loadedTextures.erase(internal.loadedTextures.begin() + index);
		internal.loadedTexturesNames.erase(internal.loadedTexturesNames.begin() + index);
		
		t.id_ = 0;

	}

	GpuTexture *Renderer3D::getTextureData(Texture& t)
	{
		int id = internal.getTextureIndex(t);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &internal.loadedTextures[id];

		return &data->texture;
	}

	Texture Renderer3D::createIntenralTexture(GpuTexture t, int alphaData)
	{
		//if t is null return an empty texture
		if (t.id == 0)
		{
			Texture{ 0 };
		}

		int id = internal::generateNewIndex(internal.loadedTexturesIndexes);

		internal::GpuTextureWithFlags text;
		text.texture = t;
		text.flags= alphaData;

		internal.loadedTexturesIndexes.push_back(id);
		internal.loadedTextures.push_back(text);
		internal.loadedTexturesNames.push_back("");

		return Texture{ id };
	}

	Texture Renderer3D::createIntenralTexture(GLuint id_, int alphaData)
	{
		GpuTexture t;
		t.id = id_;
		return createIntenralTexture(t, alphaData);

	}

	PBRTexture Renderer3D::createPBRTexture(Texture& roughness, Texture& metallic,
		Texture& ambientOcclusion)
	{
		bool roughnessLoaded = 0;
		bool metallicLoaded = 0;
		bool ambientLoaded = 0;

		PBRTexture ret = {};

		if (roughnessLoaded && metallicLoaded && ambientLoaded) { ret.RMA_loadedTextures = 7; }
		else
		if (metallicLoaded && ambientLoaded) { ret.RMA_loadedTextures = 6; }
		else
		if (roughnessLoaded && ambientLoaded) { ret.RMA_loadedTextures = 5; }
		else
		if (roughnessLoaded && metallicLoaded) { ret.RMA_loadedTextures = 4; }
		else
		if (ambientLoaded) { ret.RMA_loadedTextures = 3; }
		else
		if (metallicLoaded) { ret.RMA_loadedTextures = 2; }
		else
		if (roughnessLoaded) { ret.RMA_loadedTextures = 1; }
		else { ret.RMA_loadedTextures = 0; }

		auto t = internal.pBRtextureMaker.createRMAtexture(1024, 1024,
			{getTextureOpenglId(roughness)},
			{ getTextureOpenglId(metallic) },
			{ getTextureOpenglId(ambientOcclusion) }, internal.lightShader.quadDrawer.quadVAO);

		ret.texture = this->createIntenralTexture(t, 0);

		return ret;
	}

	void Renderer3D::deletePBRTexture(PBRTexture& t)
	{
		deleteTexture(t.texture);
		t.RMA_loadedTextures = 0;
	}

	static int max(int x, int y, int z)
	{
		return std::max(std::max(x, y), z);
	}

	Model Renderer3D::loadModel(std::string path, float scale)
	{

		gl3d::LoadedModelData model(path.c_str(), scale);
		if(model.loader.LoadedMeshes.empty())
		{
			std::cout << "err loading " + path + "\n";
			return { 0 };
		
		}

		int id = internal::generateNewIndex(internal.graphicModelsIndexes);
	
		ModelData returnModel;
		{

			int s = model.loader.LoadedMeshes.size();
			returnModel.models.reserve(s);


			returnModel.createdMaterials.reserve(model.loader.LoadedMaterials.size());
			for(int i=0;i<model.loader.LoadedMaterials.size(); i++)
			{
				auto &mat = model.loader.LoadedMaterials[i];
				auto m = this->createMaterial(mat.Kd, mat.roughness,
				mat.metallic, mat.ao, mat.name);
				

				{
					//load textures for materials
					TextureDataForMaterial textureData;

					//auto &mat = model.loader.LoadedMeshes[index].MeshMaterial;
					//gm.material = loadedMaterials[model.loader.LoadedMeshes[index].materialIndex];

					//gm.albedoTexture.clear();
					//gm.normalMapTexture.clear();
					//gm.RMA_Texture.clear();

					if (!mat.map_Kd.empty())
					{
						textureData.albedoTexture = this->loadTexture(std::string(model.path + mat.map_Kd));
					}

					if (!mat.map_Kn.empty())
					{
						textureData.normalMapTexture = this->loadTexture(std::string(model.path + mat.map_Kn));
						//	TextureLoadQuality::linearMipmap);
					}

					if (!mat.map_emissive.empty())
					{
						textureData.emissiveTexture = this->loadTexture(std::string(model.path + mat.map_emissive));
					}

					textureData.pbrTexture.RMA_loadedTextures = 0;

					auto rmaQuality = TextureLoadQuality::linearMipmap;

					if (!mat.map_RMA.empty()) 
					{
						//todo not tested
						//rmaQuality);

						textureData.pbrTexture.texture = this->loadTexture(mat.map_RMA.c_str());

						if (textureData.pbrTexture.texture.id_ != 0)
						{
							textureData.pbrTexture.RMA_loadedTextures = 7; //all textures loaded
						}

						//if (gm.RMA_Texture.id)
						//{
						//	gm.RMA_loadedTextures = 7; //all textures loaded
						//}

					}

					if (!mat.map_ORM.empty() && textureData.pbrTexture.RMA_loadedTextures == 0)
					{
						stbi_set_flip_vertically_on_load(true);

						int w = 0, h = 0;
						unsigned char *data = 0;


						{
							data = stbi_load(std::string(model.path + mat.map_ORM).c_str(),
							&w, &h, 0, 4);
							if (!data)
							{
								std::cout << "err loading " << std::string(model.path + mat.map_ORM) << "\n";
							}
							else
							{
								//convert from ORM ro RMA

								for (int j = 0; j < h; j++)
									for (int i = 0; i < w; i++)
									{
										unsigned char R = data[(i + j * w) * 4 + 1];
										unsigned char M = data[(i + j * w) * 4 + 2];
										unsigned char A = data[(i + j * w) * 4 + 0];

										data[(i + j * w) * 4 + 0] = R;
										data[(i + j * w) * 4 + 1] = M;
										data[(i + j * w) * 4 + 2] = A;
									}

								//gm.RMA_Texture.loadTextureFromMemory(data, w, h, 4, rmaQuality);
								GpuTexture t;
								t.loadTextureFromMemory(data, w, h, 4, rmaQuality); //todo 3 channels
								textureData.pbrTexture.texture = this->createIntenralTexture(t, 0);

								textureData.pbrTexture.RMA_loadedTextures = 7; //all textures loaded

								stbi_image_free(data);
							}
						}


					}

					//RMA trexture
					if (textureData.pbrTexture.RMA_loadedTextures == 0)
					{
						constexpr int MERGE_TEXTURES_ON_GPU = 1;

						if constexpr (MERGE_TEXTURES_ON_GPU)
						{

							GpuTexture roughness;
							int emptyData[1] = {};
							int roughnessLoaded = 0, metallicLoaded = 0, ambientLoaded = 0;

							if (!mat.map_Pr.empty())
							{
								roughness.loadTextureFromFile(std::string(model.path + mat.map_Pr).c_str(), dontSet, 1);
								glBindTexture(GL_TEXTURE_2D, roughness.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								roughnessLoaded = 1;
							}
							else
							{
								roughness.loadTextureFromMemory(emptyData, 1, 1, 1, dontSet);
								glBindTexture(GL_TEXTURE_2D, roughness.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							}

							GpuTexture metallic;
							if (!mat.map_Pm.empty())
							{
								metallic.loadTextureFromFile(std::string(model.path + mat.map_Pm).c_str(), dontSet, 1);
								glBindTexture(GL_TEXTURE_2D, metallic.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								metallicLoaded = 1;
							}
							else
							{
								metallic.loadTextureFromMemory(emptyData, 1, 1, 1, dontSet);
								glBindTexture(GL_TEXTURE_2D, metallic.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							}

							GpuTexture ambientOcclusion;
							if (!mat.map_Ka.empty())
							{
								ambientOcclusion.loadTextureFromFile(std::string(model.path + mat.map_Ka).c_str(), dontSet, 1);
								glBindTexture(GL_TEXTURE_2D, ambientOcclusion.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								ambientLoaded = 1;
							}
							else
							{
								ambientOcclusion.loadTextureFromMemory(emptyData, 1, 1, 1, dontSet);
								glBindTexture(GL_TEXTURE_2D, ambientOcclusion.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							}

							//calculate which function to use
							if (roughnessLoaded && metallicLoaded && ambientLoaded) { textureData.pbrTexture.RMA_loadedTextures = 7; }
							else
							if (metallicLoaded && ambientLoaded) { textureData.pbrTexture.RMA_loadedTextures = 6; }
							else
							if (roughnessLoaded && ambientLoaded) { textureData.pbrTexture.RMA_loadedTextures = 5; }
							else
							if (roughnessLoaded && metallicLoaded) { textureData.pbrTexture.RMA_loadedTextures = 4; }
							else
							if (ambientLoaded) { textureData.pbrTexture.RMA_loadedTextures = 3; }
							else
							if (metallicLoaded) { textureData.pbrTexture.RMA_loadedTextures = 2; }
							else
							if (roughnessLoaded) { textureData.pbrTexture.RMA_loadedTextures = 1; }
							else { textureData.pbrTexture.RMA_loadedTextures = 0; }

							auto t = internal.pBRtextureMaker.createRMAtexture(1024, 1024,
								roughness, metallic, ambientOcclusion, internal.lightShader.quadDrawer.quadVAO);

							textureData.pbrTexture.texture = this->createIntenralTexture(t, 0);

							roughness.clear();
							metallic.clear();
							ambientOcclusion.clear();
						}
						else 
						{
							stbi_set_flip_vertically_on_load(true);

							int w1 = 0, h1 = 0;
							unsigned char* data1 = 0;
							unsigned char* data2 = 0;
							unsigned char* data3 = 0;

							if (!mat.map_Pr.empty())
							{
								data1 = stbi_load(std::string(model.path + mat.map_Pr).c_str(),
									&w1, &h1, 0, 1);
								if (!data1) { std::cout << "err loading " << std::string(model.path + mat.map_Pr) << "\n"; }
							}

							int w2 = 0, h2 = 0;
							if (!mat.map_Pm.empty())
							{
								data2 = stbi_load(std::string(model.path + mat.map_Pm).c_str(),
									&w2, &h2, 0, 1);
								if (!data2) { std::cout << "err loading " << std::string(model.path + mat.map_Pm) << "\n"; }
							}


							int w3 = 0, h3 = 0;
							if (!mat.map_Ka.empty())
							{
								data3 = stbi_load(std::string(model.path + mat.map_Ka).c_str(),
									&w3, &h3, 0, 1);
								if (!data3) { std::cout << "err loading " << std::string(model.path + mat.map_Ka) << "\n"; }
							}

							int w = max(w1, w2, w3);
							int h = max(h1, h2, h3);

							//calculate which function to use
							if (data1 && data2 && data3) { textureData.pbrTexture.RMA_loadedTextures = 7; }
							else
							if (data2 && data3) { textureData.pbrTexture.RMA_loadedTextures = 6; }
							else
							if (data1 && data3) { textureData.pbrTexture.RMA_loadedTextures = 5; }
							else
							if (data1 && data2) { textureData.pbrTexture.RMA_loadedTextures = 4; }
							else
							if (data3) { textureData.pbrTexture.RMA_loadedTextures = 3; }
							else
							if (data2) { textureData.pbrTexture.RMA_loadedTextures = 2; }
							else
							if (data1) { textureData.pbrTexture.RMA_loadedTextures = 1; }
							else { textureData.pbrTexture.RMA_loadedTextures = 0; }

							if (textureData.pbrTexture.RMA_loadedTextures)
							{

								unsigned char* finalData = new unsigned char[w * h * 4];

								//todo mabe add bilinear filtering
								//todo load less chanels if necessary
								for (int j = 0; j < h; j++)
								{
									for (int i = 0; i < w; i++)
									{

										if (data1)	//rough
										{
											int texelI = (i / (float)w) * w1;
											int texelJ = (j / float(h)) * h1;

											finalData[((j * w) + i) * 4 + 0] =
												data1[(texelJ * w1) + texelI];

										}
										else
										{
											finalData[((j * w) + i) * 4 + 0] = 0;
										}

										if (data2)	//metalic
										{

											int texelI = (i / (float)w) * w2;
											int texelJ = (j / float(h)) * h2;

											finalData[((j * w) + i) * 4 + 1] =
												data2[(texelJ * w2) + texelI];
										}
										else
										{
											finalData[((j * w) + i) * 4 + 1] = 0;
										}

										if (data3)	//ambient
										{
											int texelI = (i / (float)w) * w3;
											int texelJ = (j / float(h)) * h3;

											finalData[((j * w) + i) * 4 + 2] =
												data3[(texelJ * w3) + texelI];
										}
										else
										{
											finalData[((j * w) + i) * 4 + 2] = 0;
										}

										finalData[((j * w) + i) * 4 + 3] = 255; //used only for imgui, remove later
									}
								}

								//gm.RMA_Texture.loadTextureFromMemory(finalData, w, h, 4,
								//	rmaQuality);

								GpuTexture t;
								t.loadTextureFromMemory(finalData, w, h, 4, rmaQuality);
								textureData.pbrTexture.texture = this->createIntenralTexture(t, 0);

								stbi_image_free(data1);
								stbi_image_free(data2);
								stbi_image_free(data3);
								delete[] finalData;

							}
						}
					

						/*
						
						*/

					}

					this->setMaterialTextures(m, textureData);
					
				}

				returnModel.createdMaterials.push_back(m);
			}


			for (int i = 0; i < s; i++)
			{
				GraphicModel gm;
				int index = i;
				//TextureDataForModel textureData = {};

				
				auto &mesh = model.loader.LoadedMeshes[index];
				gm.loadFromComputedData(mesh.Vertices.size() * 8 * 4,
					 (float *)&mesh.Vertices[0],
					mesh.Indices.size() * 4, &mesh.Indices[0]);


				if(model.loader.LoadedMeshes[index].materialIndex > -1)
				{
					gm.material = returnModel.createdMaterials[model.loader.LoadedMeshes[index].materialIndex];
				}else
				{
					//if no material loaded for this object create a new default one
					gm.material = createMaterial(glm::vec3{ 0.8 }, 0.5, 0, 1.f, "default material");
				}
				
				gm.ownMaterial = true;

				gm.name = model.loader.LoadedMeshes[i].MeshName;
				char *c = new char[gm.name.size() + 1];
				strcpy(c, gm.name.c_str());

				returnModel.subModelsNames.push_back(c);
				returnModel.models.push_back(gm);

			}


		}

		
		internal.graphicModelsIndexes.push_back(id);
		internal.graphicModels.push_back(returnModel);


		Model o;
		o.id_ = id;
		return o;

	}

	bool Renderer3D::isModel(Model& m)
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	void Renderer3D::deleteModel(Model &m)
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete model");
			return;
		}

		internal.graphicModelsIndexes.erase(internal.graphicModelsIndexes.begin() + pos);
		internal.graphicModels[pos].clear(*this);
		internal.graphicModels.erase(internal.graphicModels.begin() + pos);

		m.id_ = 0;
	}

	void Renderer3D::clearModelData(Model& m)
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			return;
		}

		internal.graphicModels[pos].clear(*this);
	}

	int Renderer3D::getModelMeshesCount(Model& m)
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			return 0;
		}
		else
		{
			return internal.graphicModels[pos].models.size();
		}
	}

	std::string Renderer3D::getModelMeshesName(Model& m, int index)
	{
		auto pos = internal.getModelIndex(m);
		if (pos >= 0)
		{
			if(index < internal.graphicModels[pos].models.size())
			{
				return internal.graphicModels[pos].models[pos].name;
			}
		}
		else
		{
			return "";
		}
	}

	std::vector<char*>* Renderer3D::getModelMeshesNames(Model& m)
	{
		auto i = internal.getModelIndex(m);
		if (i < 0) { return nullptr; } //warn or sthing

		return &internal.graphicModels[i].subModelsNames;
	}


#pragma region point light

	PointLight Renderer3D::createPointLight(glm::vec3 position, glm::vec3 color,
		float dist, float attenuation)
	{
		int id = internal::generateNewIndex(internal.pointLightIndexes);
		internal::GpuPointLight light;
		light.position = position;
		light.color = color;
		light.dist = glm::max(0.f, dist);
		light.attenuation = glm::max(0.f, attenuation);

		internal.pointLightIndexes.push_back(id);
		internal.pointLights.push_back(light);

		internal.perFrameFlags.shouldUpdatePointShadows = true;

		return { id };
	}

	void Renderer3D::detletePointLight(PointLight& l)
	{
		auto pos = internal.getPointLightIndex(l);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete point light");
			return;
		}

		//if (internal.spotLights[pos].castShadows)
		//{
		//	internal.perFrameFlags.shouldUpdateSpotShadows = true;
		//}

		internal.pointLightIndexes.erase(internal.pointLightIndexes.begin() + pos);
		internal.pointLights.erase(internal.pointLights.begin() + pos);

		l.id_ = 0;

		internal.perFrameFlags.shouldUpdatePointShadows = true;
	}

	glm::vec3 Renderer3D::getPointLightPosition(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].position;
	}

	void Renderer3D::setPointLightPosition(PointLight& l, glm::vec3 position)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (internal.pointLights[i].position != position)
		{
			internal.pointLights[i].position = position;
			internal.pointLights[i].changedThisFrame = true;
		}
	
	}

	bool Renderer3D::isPointLight(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	glm::vec3 Renderer3D::getPointLightColor(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].color;
	}

	void Renderer3D::setPointLightColor(PointLight& l, glm::vec3 color)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.pointLights[i].color = color;
	}

	float Renderer3D::getPointLightDistance(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		return internal.pointLights[i].dist;
	}

	void Renderer3D::setPointLightDistance(PointLight& l, float distance)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		distance = glm::max(0.f, distance);

		if (internal.pointLights[i].dist != distance)
		{
			internal.pointLights[i].dist = distance;
			internal.pointLights[i].changedThisFrame = true;
		}
	}

	float Renderer3D::getPointLightAttenuation(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].attenuation;
	}

	void Renderer3D::setPointLightAttenuation(PointLight& l, float attenuation)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.pointLights[i].attenuation = glm::max(attenuation, 0.f);
	}

	bool Renderer3D::getPointLightShadows(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].castShadows;
	}

	void Renderer3D::setPointLightShadows(PointLight& l, bool castShadows)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		
		if (internal.pointLights[i].castShadows != castShadows)
		{
			internal.pointLights[i].castShadows = castShadows;
			internal.pointLights[i].changedThisFrame = true;
			internal.perFrameFlags.shouldUpdatePointShadows = true;
		}
	}

	float Renderer3D::getPointLightHardness(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].hardness;
	}

	void Renderer3D::setPointLightHardness(PointLight& l, float hardness)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.pointLights[i].hardness = glm::max(hardness, 0.001f);
	}

	int Renderer3D::getPointLightShadowSize()
	{
		return pointShadows.shadowSize;
	}

	void Renderer3D::setPointLightShadowSize(int size)
	{
		size = std::min(std::max(256, size), 2048);

		if (size != pointShadows.shadowSize)
		{
			pointShadows.shadowSize = size;
			internal.perFrameFlags.shouldUpdatePointShadows = true;
		}
	}

#pragma endregion


#pragma region directional light

	DirectionalLight Renderer3D::createDirectionalLight(glm::vec3 direction,
		glm::vec3 color, float hardness, bool castShadows)
	{
		int id = internal::generateNewIndex(internal.directionalLightIndexes);
		internal::GpuDirectionalLight light;
		light.color = color;
		light.hardness = glm::max(hardness, 0.001f);
		if (castShadows)
		{
			light.castShadowsIndex = 1;
		}
		else
		{
			light.castShadowsIndex = -1;
		}

		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{ 0, -1, 0 };
		}
		else
		{
			direction = glm::normalize(direction);
		}
		light.direction = glm::vec4(direction, 0);

		internal.directionalLightIndexes.push_back(id);
		internal.directionalLights.push_back(light);

		return { id };
	}

	void Renderer3D::deleteDirectionalLight(DirectionalLight& l)
	{
		auto pos = internal.getDirectionalLightIndex(l);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete directional light");
			return;
		}


		//if (internal.directionalLights[pos].castShadows)
		//{
		//	internal.perFrameFlags.shouldUpdateDirectionalShadows = true;
		//}

		internal.directionalLightIndexes.erase(internal.directionalLightIndexes.begin() + pos);
		internal.directionalLights.erase(internal.directionalLights.begin() + pos);

		l.id_ = 0;
	}

	bool Renderer3D::isDirectionalLight(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	glm::vec3 Renderer3D::getDirectionalLightDirection(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.directionalLights[i].direction;
	}

	void Renderer3D::setDirectionalLightDirection(DirectionalLight& l, glm::vec3 direction)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{ 0, -1, 0 };
		}
		else
		{
			direction = glm::normalize(direction);
		}

		if (glm::vec3(internal.directionalLights[i].direction) != direction)
		{
			internal.directionalLights[i].direction = glm::vec4(direction, 0);
			//internal.directionalLights[i].changedThisFrame = true;
		}
	}

	glm::vec3 Renderer3D::getDirectionalLightColor(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.directionalLights[i].color;
	}

	void Renderer3D::setDirectionalLightColor(DirectionalLight& l, glm::vec3 color)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.directionalLights[i].color = color;
	}

	float Renderer3D::getDirectionalLightHardness(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.directionalLights[i].hardness;
	}

	void Renderer3D::setDirectionalLightHardness(DirectionalLight& l, float hardness)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.directionalLights[i].hardness = glm::max(hardness, 0.001f);
	}

	bool Renderer3D::getDirectionalLightShadows(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		if (internal.directionalLights[i].castShadowsIndex >= 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	void Renderer3D::setDirectionalLightShadows(DirectionalLight& l, bool castShadows)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		if (castShadows)
		{
			internal.directionalLights[i].castShadowsIndex = 1;
		}
		else
		{
			internal.directionalLights[i].castShadowsIndex = -1;
		}
	}

	int Renderer3D::getDirectionalLightShadowSize()
	{
		return directionalShadows.shadowSize;
	}

	void Renderer3D::setDirectionalLightShadowSize(int size)
	{
		size = std::min(std::max(256, size), 2048);

		if (size != directionalShadows.shadowSize)
		{
			directionalShadows.shadowSize = size;
			//internal.perFrameFlags.shouldupdatedi
		}
	}

#pragma endregion


#pragma region spot light

	SpotLight Renderer3D::createSpotLight(glm::vec3 position, float fov, glm::vec3 direction,
		float dist, float attenuation, glm::vec3 color, float hardness, int castShadows)
	{
		int id = internal::generateNewIndex(internal.spotLightIndexes);

		internal::GpuSpotLight light = {};
		light.position = position;
		
		fov = glm::clamp(fov, glm::radians(0.f), glm::radians(160.f));
		fov /= 2.f;
		fov = std::cos(fov);
		light.cosHalfAngle = fov;
		
		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{0, -1, 0};
		}
		else
		{
			direction = glm::normalize(direction);
		}
		light.direction = direction;
		
		light.dist = glm::max(0.f, dist);
		light.attenuation = glm::max(attenuation, 0.f);
		light.color = color;
		light.hardness = glm::max(hardness, 0.001f);
		light.castShadows = castShadows;
		light.changedThisFrame = true;

		if (castShadows)
		{
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}

		internal.spotLightIndexes.push_back(id);
		internal.spotLights.push_back(light);

		return { id };
	}

	SpotLight Renderer3D::createSpotLight(glm::vec3 position, float fov, glm::vec2 angles, float dist, float attenuation, glm::vec3 color, float hardness, int castShadows)
	{
		glm::vec3 direction = fromAnglesToDirection(angles.x, angles.y);

		return createSpotLight(position, fov, direction, dist, attenuation,
			color, hardness, castShadows);

	}

	void Renderer3D::deleteSpotLight(SpotLight& l)
	{
		auto pos = internal.getSpotLightIndex(l);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete spot light");
			return;
		}


		if (internal.spotLights[pos].castShadows)
		{ 
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}

		internal.spotLightIndexes.erase(internal.spotLightIndexes.begin() + pos);
		internal.spotLights.erase(internal.spotLights.begin() + pos);

		l.id_ = 0;
	}

	glm::vec3 Renderer3D::getSpotLightPosition(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].position;

	}

	void Renderer3D::setSpotLightPosition(SpotLight& l, glm::vec3 position)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return ; } //warn or sthing

		if (internal.spotLights[i].position != position)
		{
			internal.spotLights[i].position = position;
			internal.spotLights[i].changedThisFrame = true;
		}
	}

	bool Renderer3D::isSpotLight(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	glm::vec3 Renderer3D::getSpotLightColor(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].color;
	}

	void Renderer3D::setSpotLightColor(SpotLight& l, glm::vec3 color)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.spotLights[i].color = color;
	}

	float Renderer3D::getSpotLightFov(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		float angle =  internal.spotLights[i].cosHalfAngle;
		angle = std::acos(angle);
		angle *= 2;

		return angle;
	}

	void Renderer3D::setSpotLightFov(SpotLight& l, float fov)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		fov = glm::clamp(fov, glm::radians(0.f), glm::radians(160.f)); //todo magic number
		fov /= 2.f;
		fov = std::cos(fov);

		if(internal.spotLights[i].cosHalfAngle != fov)
		{
			internal.spotLights[i].cosHalfAngle = fov;
			internal.spotLights[i].changedThisFrame= true;
		}
	}

	glm::vec3 Renderer3D::getSpotLightDirection(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		return internal.spotLights[i].direction;
	}

	void Renderer3D::setSpotLightDirection(SpotLight& l, glm::vec3 direction)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{ 0, -1, 0 };
		}
		else
		{
			direction = glm::normalize(direction);
		}

		if (internal.spotLights[i].direction != direction)
		{
			internal.spotLights[i].direction = direction;
			internal.spotLights[i].changedThisFrame = true;
		}
	}

	float Renderer3D::getSpotLightDistance(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		return internal.spotLights[i].dist;
	}

	void Renderer3D::setSpotLightDistance(SpotLight& l, float distance)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		distance = glm::max(0.f, distance);

		if (internal.spotLights[i].dist != distance)
		{
			internal.spotLights[i].dist = distance;
			internal.spotLights[i].changedThisFrame = true;
		}
	}

	float Renderer3D::getSpotLightAttenuation(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].attenuation;
	}

	void Renderer3D::setSpotLightAttenuation(SpotLight& l, float attenuation)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.spotLights[i].attenuation = glm::max(attenuation, 0.f);
	}

	float Renderer3D::getSpotLightHardness(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].hardness;
	}

	void Renderer3D::setSpotLightHardness(SpotLight& l, float hardness)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.spotLights[i].hardness = glm::max(hardness, 0.001f);
	}

	void Renderer3D::setSpotLightShadows(SpotLight& l, bool castShadows)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (internal.spotLights[i].castShadows != castShadows)
		{
			
			internal.spotLights[i].castShadows = castShadows;
			internal.spotLights[i].changedThisFrame = true;
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}
	}

	bool Renderer3D::getSpotLightShadows(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].castShadows;
	}

	int Renderer3D::getSpotLightShadowSize()
	{
		return spotShadows.shadowSize;
	}

	void Renderer3D::setSpotLightShadowSize(int size)
	{
		size = std::min(std::max(256, size), 2048);

		if (spotShadows.shadowSize != size)
		{
			spotShadows.shadowSize = size;
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}
	}

#pragma endregion

	Entity Renderer3D::createEntity(Model m, Transform transform,
		bool staticGeometry, bool visible, bool castShadows)
	{
		int id = internal::generateNewIndex(internal.entitiesIndexes);

		CpuEntity entity;
		
		entity.transform = transform;
		entity.setStatic(staticGeometry);
		entity.setVisible(visible);
		entity.setCastShadows(castShadows);

		internal.entitiesIndexes.push_back(id);
		internal.cpuEntities.push_back(std::move(entity));

		if (staticGeometry && visible && castShadows)
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		Entity e;
		e.id_ = id;
		
		setEntityModel(e, m);
		
		return e;
	}

	Entity Renderer3D::duplicateEntity(Entity& e)
	{
		int oldIndex = internal.getEntityIndex(e);

		if (oldIndex < 0)
		{
			return {};
		}
		
		int id = internal::generateNewIndex(internal.entitiesIndexes);

		CpuEntity entity;

		entity.transform = internal.cpuEntities[oldIndex].transform;
		entity.flags = internal.cpuEntities[oldIndex].flags;
		entity.subModelsNames.reserve(internal.cpuEntities[oldIndex].subModelsNames.size());

		for (auto i : internal.cpuEntities[oldIndex].subModelsNames)
		{
			int size = strlen(i);
			char* c = new char[size] {};
			strcpy(c, i);
			entity.subModelsNames.push_back(c);
		}

		entity.models.reserve(internal.cpuEntities[oldIndex].models.size());
		for (auto i : internal.cpuEntities[oldIndex].models)
		{
			GraphicModel model = i;

			if (model.ownMaterial)
			{
				model.material = createMaterial();
				this->copyMaterialData(model.material, i.material);
			}

			entity.models.push_back(std::move(model));
		}

		internal.entitiesIndexes.push_back(id);
		internal.cpuEntities.push_back(std::move(entity));

		Entity ret;
		ret.id_ = id;
		return ret;
	}

	void Renderer3D::setEntityModel(Entity& e, Model m)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn

		clearEntityModel(e);

		auto& entity = internal.cpuEntities[i];

		//clear if needed

		int modelindex = internal.getModelIndex(m);
		if (modelindex >= 0)
		{
			int size = internal.graphicModels[modelindex].models.size();
			entity.models.reserve(size);

			for (int i = 0; i < size; i++)
			{
				entity.models.push_back(internal.graphicModels[modelindex].models[i]);
				entity.models.back().ownMaterial = false;

				int charSize = strlen(internal.graphicModels[modelindex].subModelsNames[i]);
				char* name = new char[charSize + 1]{};
				strcpy(name, internal.graphicModels[modelindex].subModelsNames[i]);

				entity.subModelsNames.push_back(name);
			}

		}
	}

	void Renderer3D::clearEntityModel(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return ; } //warn

		auto& entity = internal.cpuEntities[i];

		for (auto& i : entity.subModelsNames)
		{
			delete[] i;
		}

		for (auto& i : entity.models)
		{
			if (i.ownMaterial)
			{
				this->deleteMaterial(i.material);
			}
		}

		entity.models.clear();
		entity.subModelsNames.clear();

	}

	CpuEntity* Renderer3D::getEntityData(Entity &e)
	{
		auto i = internal.getEntityIndex(e);

		if (i < 0) { return nullptr; }

		return &internal.cpuEntities[i];

	}

	Transform Renderer3D::getEntityTransform(Entity &e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return Transform{}; } //warn or sthing
	
		return internal.cpuEntities[i].transform;
	}

	void Renderer3D::setEntityTransform(Entity &e, Transform transform)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing
		
		if (internal.cpuEntities[i].isStatic())
		{
			if (internal.cpuEntities[i].transform != transform)
			{
				internal.perFrameFlags.staticGeometryChanged = true;
			};
		}

		internal.cpuEntities[i].transform = transform;

	}

	bool Renderer3D::isEntityStatic(Entity &e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing

		return internal.cpuEntities[i].isStatic();
	}

	void Renderer3D::setEntityStatic(Entity &e, bool s)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if ((internal.cpuEntities[i].isStatic() != s)
			&& internal.cpuEntities[i].isVisible()
			&& internal.cpuEntities[i].castShadows()
			)
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		internal.cpuEntities[i].setStatic(s);
	}

	void Renderer3D::deleteEntity(Entity &e)
	{
		auto pos = internal.getEntityIndex(e);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete entity");
			return;
		}

		clearEntityModel(e);

		internal.entitiesIndexes.erase(internal.entitiesIndexes.begin() + pos);
		internal.cpuEntities.erase(internal.cpuEntities.begin() + pos);
		
		e.id_ = 0;

	}

	int Renderer3D::getEntityMeshesCount(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing

		return internal.cpuEntities[i].models.size();

	}

	MaterialValues Renderer3D::getEntityMeshMaterialValues(Entity& e, int meshIndex)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return {}; } //warn or sthing
	
		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto mat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			bool succeeded = internal.getMaterialData(mat, &data, nullptr, nullptr);

			if (succeeded)
			{
				return data;
			}
			else
			{
				return {}; //warn
			}
		}
		else
		{
			return {}; //warn
		}

	}

	void Renderer3D::setEntityMeshMaterialValues(Entity& e, int meshIndex, MaterialValues mat)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return ; } //warn or sthing

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			std::string name = {};
			TextureDataForMaterial textures;
			bool succeeded = internal.getMaterialData(currentMat, &data, &name, &textures);

			if (succeeded)
			{
				if (internal.cpuEntities[i].models[meshIndex].ownMaterial == 1)
				{
					setMaterialValues(currentMat, mat);
				}else
				if (mat != data)
				{
					Material newMat = this->createMaterial(mat.kd, mat.roughness,
						mat.metallic, mat.ao, name);
					int newMatIndex = internal.getMaterialIndex(newMat); //this should not fail

					internal.materialTexturesData[newMatIndex] = textures;

					internal.cpuEntities[i].models[meshIndex].material = newMat;
					internal.cpuEntities[i].models[meshIndex].ownMaterial = 1;
				}
			}
			else
			{
				return ; //warn
			}
		}
		else
		{
			return ; //warn
		}

	}

	std::string Renderer3D::getEntityMeshMaterialName(Entity& e, int meshIndex)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return {}; } //no valid entity

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			std::string name = {};
			bool succeeded = internal.getMaterialData(currentMat, nullptr, &name, nullptr);
			if (succeeded)
			{
				return name;
			}
			else
			{
				return{};//no valid material
			}
		}
		else
		{
			return {};//wrong index
		}

	}

	void Renderer3D::setEntityMeshMaterialName(Entity& e, int meshIndex, const std::string& name)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			std::string oldName = {};
			TextureDataForMaterial textures;
			bool succeeded = internal.getMaterialData(currentMat, &data, &oldName, &textures);

			if (succeeded)
			{
				if (internal.cpuEntities[i].models[meshIndex].ownMaterial == 1)
				{
					setMaterialName(currentMat, name);
				}
				else
				if (name != oldName) //copy to new material
				{
					Material newMat = this->createMaterial(data.kd, data.roughness,
						data.metallic, data.ao, name);
					int newMatIndex = internal.getMaterialIndex(newMat); //this should not fail
					internal.materialTexturesData[newMatIndex] = textures;

					internal.cpuEntities[i].models[meshIndex].material = newMat;
					internal.cpuEntities[i].models[meshIndex].ownMaterial = 1;
				}
			}
			else
			{
				return; //warn
			}
		}
		else
		{
			return; //warn
		}
	}

	void Renderer3D::setEntityMeshMaterial(Entity& e, int meshIndex, Material mat)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //invalid entity;

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			if (internal.cpuEntities[i].models[meshIndex].ownMaterial)
			{
				deleteMaterial(internal.cpuEntities[i].models[meshIndex].material);
			}

			internal.cpuEntities[i].models[meshIndex].material = mat;
			internal.cpuEntities[i].models[meshIndex].ownMaterial = 0;

			//todo look into textures and see if they have alpha data
			if (
				internal.cpuEntities[i].isStatic()
				&& internal.cpuEntities[i].castShadows()
				&&internal.cpuEntities[i].isVisible()
				)
			{
				internal.perFrameFlags.staticGeometryChanged = true;
			}
		}
	}

	TextureDataForMaterial Renderer3D::getEntityMeshMaterialTextures(Entity& e, int meshIndex)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return {}; } //no valid entity

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			TextureDataForMaterial t= {};
			bool succeeded = internal.getMaterialData(currentMat, nullptr, nullptr, &t);
			if (succeeded)
			{
				return t;
			}
			else
			{
				return{};//invalid material
			}

		}
		else
		{
			return {};//invalid index
		}
	}

	void Renderer3D::setEntityMeshMaterialTextures(Entity& e, int meshIndex, TextureDataForMaterial texture)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			std::string oldName = {};
			TextureDataForMaterial oldTextures;
			bool succeeded = internal.getMaterialData(currentMat, &data, &oldName, &oldTextures);

			if (succeeded)
			{
				if (internal.cpuEntities[i].models[meshIndex].ownMaterial == 1)
				{
					setMaterialTextures(currentMat, texture);
				}
				else
				if (texture != oldTextures) //copy to new material
				{
					Material newMat = this->createMaterial(data.kd, data.roughness,
						data.metallic, data.ao, oldName);
					int newMatIndex = internal.getMaterialIndex(newMat); //this should not fail
					internal.materialTexturesData[newMatIndex] = texture; //new textures

					internal.cpuEntities[i].models[meshIndex].material = newMat;
					internal.cpuEntities[i].models[meshIndex].ownMaterial = 1;
				}
			}
			else
			{
				return; //warn
			}
		}
		else
		{
			return; //warn
		}
	}

	bool Renderer3D::isEntity(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) 
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool Renderer3D::isEntityVisible(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing
		return internal.cpuEntities[i].isVisible();
	}

	void Renderer3D::setEntityVisible(Entity& e, bool v)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (internal.cpuEntities[i].isVisible() != v)
		{
			internal.cpuEntities[i].setVisible(v);
			if (internal.cpuEntities[i].isStatic()
				&& internal.cpuEntities[i].castShadows()
				)
			{
				internal.perFrameFlags.staticGeometryChanged = true;
			}
		}
	}

	void Renderer3D::setEntityCastShadows(Entity& e, bool s)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if(
			internal.cpuEntities[i].isVisible()
			&& internal.cpuEntities[i].isStatic()
			&& (s != internal.cpuEntities[i].castShadows())
			)
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		internal.cpuEntities[i].setCastShadows(s);
	}

	bool Renderer3D::getEntityCastShadows(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing
		return internal.cpuEntities[i].castShadows();
	}

	std::vector<char*> *Renderer3D::getEntityMeshesNames(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return nullptr; } //warn or sthing
		
		return &internal.cpuEntities[i].subModelsNames;
	}

	void Renderer3D::setExposure(float exposure)
	{
		internal.lightShader.lightPassUniformBlockCpuData.exposure =
			std::max(exposure, 0.001f);
	}

	float Renderer3D::getExposure()
	{
		return internal.lightShader.lightPassUniformBlockCpuData.exposure;
	}

	void Renderer3D::enableNormalMapping(bool normalMapping)
	{
		internal.lightShader.normalMap = normalMapping;
	}

	bool Renderer3D::isNormalMappingEnabeled()
	{
		return internal.lightShader.normalMap;
	}

	void Renderer3D::enableLightSubScattering(bool lightSubScatter)
	{
		internal.lightShader.lightPassUniformBlockCpuData.lightSubScater = lightSubScatter;
	}

	bool Renderer3D::isLightSubScatteringEnabeled()
	{
		return internal.lightShader.lightPassUniformBlockCpuData.lightSubScater;
	}

	void Renderer3D::enableSSAO(bool ssao)
	{
		internal.lightShader.useSSAO = ssao;
	}

	bool Renderer3D::isSSAOenabeled()
	{
		return internal.lightShader.useSSAO;
	}

	float Renderer3D::getSSAOBias()
	{
		return ssao.ssaoShaderUniformBlockData.bias;
	}

	void Renderer3D::setSSAOBias(float bias)
	{
		ssao.ssaoShaderUniformBlockData.bias = std::max(bias, 0.f);
	}

	float Renderer3D::getSSAORadius()
	{
		return ssao.ssaoShaderUniformBlockData.radius;
	}

	void Renderer3D::setSSAORadius(float radius)
	{
		ssao.ssaoShaderUniformBlockData.radius = std::max(radius, 0.01f);
	}

	int Renderer3D::getSSAOSampleCount()
	{
		return ssao.ssaoShaderUniformBlockData.samplesTestSize;

	}

	void Renderer3D::setSSAOSampleCount(int samples)
	{
		ssao.ssaoShaderUniformBlockData.samplesTestSize = std::min(std::max(samples, 5), 64);
	}

	float Renderer3D::getSSAOExponent()
	{
		return ssao.ssao_finalColor_exponent;
	}

	void Renderer3D::setSSAOExponent(float exponent)
	{
		ssao.ssao_finalColor_exponent = std::min(std::max(1.f, exponent), 32.f);
	}

	void Renderer3D::enableFXAA(bool fxaa)
	{
		this->antiAlias.usingFXAA = fxaa;
	}

	bool Renderer3D::isFXAAenabeled()
	{
		return antiAlias.usingFXAA;
	}

	//todo look into  glProgramUniform
	//in order to send less stuff tu uniforms

	//todo look into
	//ATI/AMD created GL_ATI_meminfo. This extension is very easy to use. 
	//You basically need to call glGetIntegerv with the appropriate token values.
	//https://www.khronos.org/registry/OpenGL/extensions/ATI/ATI_meminfo.txt
	//http://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt
	
	void Renderer3D::renderModelNormals(Model o, glm::vec3 position, glm::vec3 rotation,
		glm::vec3 scale, float normalSize, glm::vec3 normalColor)
	{
		auto obj = internal.getModelData(o);

		if(!obj)
		{
			return;
		}

		for(int i=0; i<obj->models.size(); i++)
		{
			renderSubModelNormals(o, i, position, rotation, scale, normalSize, normalColor);
		}
		
	}

	void Renderer3D::renderSubModelNormals(Model o, int index, glm::vec3 position, glm::vec3 rotation,
		glm::vec3 scale, float normalSize, glm::vec3 normalColor)
	{
			
		showNormalsProgram.shader.bind();
		
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = gl3d::getTransformMatrix(position, rotation, scale);
		
		auto viewTransformMat = viewMat * transformMat;
		
		glUniformMatrix4fv(showNormalsProgram.modelTransformLocation,
			1, GL_FALSE, &viewTransformMat[0][0]);
		
		glUniformMatrix4fv(showNormalsProgram.projectionLocation,
			1, GL_FALSE, &projMat[0][0]);
		
		glUniform1f(showNormalsProgram.sizeLocation, normalSize);

		glUniform3fv(showNormalsProgram.colorLocation, 1, &(normalColor[0]));

		auto modelIndex = this->internal.getModelIndex(o);

		auto obj = internal.getModelData(o);
		if(obj == nullptr)
		{
			return;
		}

		{
			if(index >= obj->models.size())
			{
				return;
			}

			auto &i = obj->models[index];
			
			glBindVertexArray(i.vertexArray);

			if (i.indexBuffer)
			{
				glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
			}
			glBindVertexArray(0);
		}

	}

	void Renderer3D::renderSubModelBorder(Model o, int index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, float borderSize, glm::vec3 borderColor)
	{
		//auto modelIndex = this->getObjectIndex(o);
		//
		//auto obj = getObjectData(o);
		//if (obj == nullptr)
		//{
		//	return;
		//}
		//
		//if (index >= obj->models.size())
		//{
		//	return;
		//}
		//
		//	
		//glEnable(GL_STENCIL_TEST);
		//glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		//glStencilFunc(GL_ALWAYS, 1, 0xFF);
		//glStencilMask(0xFF);	
		//
		//auto projMat = renderer.camera.getProjectionMatrix();
		//auto viewMat = renderer.camera.getWorldToViewMatrix();
		//auto transformMat = models[0].getTransformMatrix();
		//
		//auto viewProjMat = projMat * viewMat * transformMat;
		//
		////lightShader.bind(viewProjMat, transformMat,
		////	lightCubeModel.position, renderer.camera.position, gamaCorection,
		////	models[itemCurrent].models[subItemCurent].material, renderer.pointLights);
		//
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//models[itemCurrent].models[subItemCurent].draw();
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//
		//glDisable(GL_STENCIL_TEST);
		//
		//glEnable(GL_STENCIL_TEST);
		//glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		//glDepthFunc(GL_ALWAYS);
		//glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		//glStencilMask(0x00);
		//
		//auto &m = models[itemCurrent].models[subItemCurent];
		//projMat = renderer.camera.getProjectionMatrix();
		//viewMat = renderer.camera.getWorldToViewMatrix();
		//
		//auto rotation = models[itemCurrent].rotation;
		//auto scale = models[itemCurrent].scale;
		//scale *= 1.05;
		//auto position = models[itemCurrent].position;
		//
		//
		//auto s = glm::scale(scale);
		//auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
		//	glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
		//	glm::rotate(rotation.z, glm::vec3(0, 0, 1));
		//auto t = glm::translate(position);
		//
		//transformMat = t * r * s;
		//
		//viewProjMat = projMat * viewMat * transformMat;
		//
		//shader.bind();
		//glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);
		//
		//glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);
		//
		//glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		//glVertexAttrib3f(1, 98 / 255.f, 24 / 255.f, 201 / 255.f);
		//
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indexBuffer);
		//glDrawElements(GL_TRIANGLES, m.primitiveCount, GL_UNSIGNED_INT, 0);
		//
		//glDisable(GL_STENCIL_TEST);
		//glDepthFunc(GL_LESS);


	}

	int Renderer3D::InternalStruct::getMaterialIndex(Material m)
	{
		int id = m.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(materialIndexes.begin(), materialIndexes.end(), id);
		if (found == materialIndexes.end())
		{
			gl3dAssertComment(found != materialIndexes.end(), "invalid material");
			return -1;
		}
		id = found - materialIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getModelIndex(Model o)
	{
		int id = o.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), id);
		if (found == graphicModelsIndexes.end())
		{
			gl3dAssertComment(found != graphicModelsIndexes.end(), "invalid object");
			return -1;
		}
		id = found - graphicModelsIndexes.begin();
	
		return id;
	}

	int Renderer3D::InternalStruct::getTextureIndex(Texture t)
	{
		int id = t.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(loadedTexturesIndexes.begin(), loadedTexturesIndexes.end(), id);
		if (found == loadedTexturesIndexes.end())
		{
			gl3dAssertComment(found != loadedTexturesIndexes.end(), "invalid texture");
			return -1;
		}
		id = found - loadedTexturesIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getEntityIndex(Entity t)
	{
		int id = t.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(entitiesIndexes.begin(), entitiesIndexes.end(), id);
		if (found == entitiesIndexes.end())
		{
			gl3dAssertComment(found != entitiesIndexes.end(), "invalid entity");
			return -1;
		}
		id = found - entitiesIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getSpotLightIndex(SpotLight l)
	{
		int id = l.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(spotLightIndexes.begin(), spotLightIndexes.end(), id);
		if (found == spotLightIndexes.end())
		{
			gl3dAssertComment(found != spotLightIndexes.end(), "invalid spot light");
			return -1;
		}
		id = found - spotLightIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getPointLightIndex(PointLight l)
	{
		int id = l.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(pointLightIndexes.begin(), pointLightIndexes.end(), id);
		if (found == pointLightIndexes.end())
		{
			gl3dAssertComment(found != pointLightIndexes.end(), "invalid point light");
			return -1;
		}
		id = found - pointLightIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getDirectionalLightIndex(DirectionalLight l)
	{
		int id = l.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(directionalLightIndexes.begin(), directionalLightIndexes.end(), id);
		if (found == directionalLightIndexes.end())
		{
			gl3dAssertComment(found != directionalLightIndexes.end(), "invalid directional light");
			return -1;
		}
		id = found - directionalLightIndexes.begin();

		return id;
	}

	bool Renderer3D::InternalStruct::getMaterialData(Material m, MaterialValues* gpuMaterial, std::string* name, TextureDataForMaterial* textureData)
	{
		int id = getMaterialIndex(m);

		if (id == -1)
		{
			return false;
		}

		if (gpuMaterial)
		{
			*gpuMaterial = materials[id];
		}

		if (name)
		{
			*name = materialNames[id];
		}

		if (textureData)
		{
			*textureData = materialTexturesData[id];
		}

		return true;
	}

	ModelData* Renderer3D::InternalStruct::getModelData(Model o)
	{
		int id = getModelIndex(o);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &graphicModels[id];
		return data;
	}


	//todo add to other projects and places
	glm::mat4 lookAtSafe(glm::vec3 const& eye, glm::vec3 const& center, glm::vec3 const& upVec)
	{
		glm::vec3 up = glm::normalize(upVec);

		glm::vec3 f;
		glm::vec3 s;
		glm::vec3 u;

		f = (normalize(center - eye));
		if (f == up || f == -up)
		{
			s = glm::vec3(up.z, up.x, up.y);
			u = (cross(s, f));

		}
		else
		{
			s = (normalize(cross(f, up)));
			u = (cross(s, f));
		}
		
		glm::mat4 Result(1);
		Result[0][0] = s.x;
		Result[1][0] = s.y;
		Result[2][0] = s.z;
		Result[0][1] = u.x;
		Result[1][1] = u.y;
		Result[2][1] = u.z;
		Result[0][2] = -f.x;
		Result[1][2] = -f.y;
		Result[2][2] = -f.z;
		Result[3][0] = -dot(s, eye);
		Result[3][1] = -dot(u, eye);
		Result[3][2] = dot(f, eye);
		return Result;
	}

	void Renderer3D::render(float deltaTime)
	{
		if (adaptiveResolution.timeSample >= adaptiveResolution.timeSamplesCount)
		{
			float ms = 0;
			for (int i = 0; i < adaptiveResolution.timeSamplesCount; i++)
			{
				ms += adaptiveResolution.msSampled[i];
			}
			ms /= adaptiveResolution.timeSamplesCount;
			float seconds = ms * 1000;

			if (seconds < adaptiveResolution.stepUpSecTarget)
			{
				adaptiveResolution.rezRatio += 0.1f;

				if (adaptiveResolution.rezRatio >= 1.f)
				{
					adaptiveResolution.rezRatio = 1.f;
					adaptiveResolution.shouldUseAdaptiveResolution = false;
				}
			}
			else if(seconds > adaptiveResolution.stepDownSecTarget)
			{
				adaptiveResolution.rezRatio -= 0.1f;
				if (adaptiveResolution.rezRatio <= adaptiveResolution.maxScaleDown)
				{
					adaptiveResolution.rezRatio = adaptiveResolution.maxScaleDown;
				}

				adaptiveResolution.shouldUseAdaptiveResolution = true;
			}
			adaptiveResolution.timeSample = 0;
		}
		else
		{
			adaptiveResolution.msSampled[adaptiveResolution.timeSample] = deltaTime;
			adaptiveResolution.timeSample++;
		}

		updateWindowMetrics(internal.w, internal.h);
		
		glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);//todo check remove

		glBindFramebuffer(GL_FRAMEBUFFER, 0);//todo check remove

		glStencilMask(0xFF);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDepthFunc(GL_LESS);

		if (antiAlias.usingFXAA || adaptiveResolution.useAdaptiveResolution)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, adaptiveResolution.fbo);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		renderSkyBoxBefore();


		#pragma region render shadow maps
		
		//filter true and onlyStatic true renders only static geometry
		auto renderModelsShadows = [&](glm::mat4& lightSpaceMatrix, bool filter =0, bool onlyStatic=0)
		{
			//render shadow of the models
			for (auto& i : internal.cpuEntities)
			{

				if (!i.isVisible() || !i.castShadows())
				{
					continue;
				}

				if (filter)
				{
					if (onlyStatic != i.isStatic())
					{
						continue;
					}
				}

				auto transformMat = i.transform.getTransformMatrix();
				auto modelViewProjMat = lightSpaceMatrix * transformMat;

				glUniformMatrix4fv(internal.lightShader.prePass.u_transform, 1, GL_FALSE,
					&modelViewProjMat[0][0]);

				for (auto& i : i.models)
				{

					auto m = internal.getMaterialIndex(i.material);

					if (m < 0)
					{
						glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);
					}
					else
					{

						auto t = internal.materialTexturesData[m];
						auto tId = internal.getTextureIndex(t.albedoTexture);

						if (tId < 0)
						{
							glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);
						}
						else
						{
							auto texture = internal.loadedTextures[tId];

							glUniform1i(internal.lightShader.prePass.u_hasTexture, 1);
							glUniform1i(internal.lightShader.prePass.u_albedoSampler, 0);

							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, texture.texture.id);
						}
					}

					glBindVertexArray(i.vertexArray);

					if (i.indexBuffer)
					{
						glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
					}
					else
					{
						glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
					}
				}

			}
		};
		
		auto renderModelsPointShadows = [&](int lightIndex, int shadowCastIndex, bool filter = 0, bool onlyStatic = 0)
		{
			glUniform1i(internal.lightShader.pointShadowShader.u_lightIndex, shadowCastIndex);

			glm::mat4 shadowProj = glm::perspective(glm::radians(90.f), 1.f, 0.1f,
				internal.pointLights[lightIndex].dist);
			glm::vec3 lightPos = internal.pointLights[lightIndex].position;

			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.reserve(6);
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

			glUniformMatrix4fv(internal.lightShader.pointShadowShader.u_shadowMatrices, 6, GL_FALSE,
				&(*shadowTransforms.data())[0][0]);

			glUniform3fv(internal.lightShader.pointShadowShader.u_lightPos, 1,
				&lightPos[0]);

			glUniform1f(internal.lightShader.pointShadowShader.u_farPlane,
				internal.pointLights[lightIndex].dist);

			//render shadow of the models
			for (auto& i : internal.cpuEntities)
			{

				if (!i.isVisible() || !i.castShadows())
				{
					continue;
				}

				if (filter)
				{
					if (onlyStatic != i.isStatic())
					{
						continue;
					}
				}

				auto transformMat = i.transform.getTransformMatrix();

				glUniformMatrix4fv(internal.lightShader.pointShadowShader.u_transform, 1, GL_FALSE,
					&transformMat[0][0]);

				for (auto& i : i.models)
				{

					auto m = internal.getMaterialIndex(i.material);

					if (m < 0)
					{
						glUniform1i(internal.lightShader.pointShadowShader.u_hasTexture, 0);
					}
					else
					{

						auto t = internal.materialTexturesData[m];
						auto tId = internal.getTextureIndex(t.albedoTexture);

						if (tId < 0)
						{
							glUniform1i(internal.lightShader.pointShadowShader.u_hasTexture, 0);
						}
						else
						{
							auto texture = internal.loadedTextures[tId];

							glUniform1i(internal.lightShader.pointShadowShader.u_hasTexture, 1);
							glUniform1i(internal.lightShader.pointShadowShader.u_albedoSampler, 0);

							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, texture.texture.id);
						}
					}

					glBindVertexArray(i.vertexArray);

					if (i.indexBuffer)
					{
						glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
					}
					else
					{
						glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
					}
				}

			}
		};

		if (internal.pointLights.size())
		{
			int shouldUpdateAllPointShadows = internal.perFrameFlags.staticGeometryChanged
				|| internal.perFrameFlags.shouldUpdatePointShadows;

			int pointLightsShadowsCount = 0;
			for (auto& i :internal.pointLights)
			{
				if (i.castShadows != 0)
				{
					i.castShadowsIndex = pointLightsShadowsCount;
					pointLightsShadowsCount++;
				}

			}
			
			if (pointLightsShadowsCount)
			{
				if (pointLightsShadowsCount != pointShadows.textureCount
					|| pointShadows.currentShadowSize != pointShadows.shadowSize
					)
				{
					pointShadows.allocateTextures(pointLightsShadowsCount);
					shouldUpdateAllPointShadows = true;
				}

				internal.lightShader.pointShadowShader.shader.bind();
				glViewport(0, 0, pointShadows.shadowSize, pointShadows.shadowSize);


				//static geometry
				glBindFramebuffer(GL_FRAMEBUFFER, pointShadows.staticGeometryFbo);
				for (int lightIndex = 0; lightIndex < internal.pointLights.size(); lightIndex++)
				{
					if (internal.pointLights[lightIndex].castShadows == 0)
					{
						continue;
					}

					if (internal.pointLights[lightIndex].changedThisFrame
						|| shouldUpdateAllPointShadows
						)
					{
						internal.pointLights[lightIndex].changedThisFrame = false;


						for (int i = 0; i < 6; i++)
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								pointShadows.staticGeometryTextures, 0,
								internal.pointLights[lightIndex].castShadowsIndex * 6 + i);
							glClear(GL_DEPTH_BUFFER_BIT);
						}

						glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							pointShadows.staticGeometryTextures, 0);


						renderModelsPointShadows(lightIndex,
							internal.pointLights[lightIndex].castShadowsIndex, true, true);
					}

				}

				//copy static geometry
				glCopyImageSubData(pointShadows.staticGeometryTextures, GL_TEXTURE_CUBE_MAP_ARRAY, 0,
					0, 0, 0,
					pointShadows.shadowTextures, GL_TEXTURE_CUBE_MAP_ARRAY, 0,
					0, 0, 0,
					pointShadows.shadowSize, pointShadows.shadowSize, pointShadows.textureCount * 6
				);

				//dynamic geometry
				glBindFramebuffer(GL_FRAMEBUFFER, pointShadows.fbo);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointShadows.shadowTextures, 0);
				//glClear(GL_DEPTH_BUFFER_BIT);

				for (int lightIndex = 0; lightIndex < internal.pointLights.size(); lightIndex++)
				{
					if (internal.pointLights[lightIndex].castShadows == 0)
					{
						continue;
					}


					renderModelsPointShadows(lightIndex,
						internal.pointLights[lightIndex].castShadowsIndex, true, false);

				}

			}

			
		}

		internal.lightShader.prePass.shader.bind();

		if (internal.directionalLights.size())
		{

			int directionalLightsShadows = 0;
			for (const auto& i : internal.directionalLights)
			{
				if (i.castShadowsIndex >= 0) { directionalLightsShadows++; }
			}

			if (directionalLightsShadows != directionalShadows.textureCount
				|| directionalShadows.shadowSize != directionalShadows.currentShadowSize
				)
			{
				directionalShadows.allocateTextures(directionalLightsShadows);
			}
			


			auto calculateLightProjectionMatrix = [&](glm::vec3 lightDir, glm::mat4 lightView, 
				float nearPlane, float farPlane,
				float zOffset)
			{
				glm::vec3 rVector = {};
				glm::vec3 upVectpr = {};
				generateTangentSpace(lightDir, upVectpr, rVector);

				glm::vec2 nearDimensions{};
				glm::vec2 farDimensions{};
				glm::vec3 centerNear{};
				glm::vec3 centerFar{};

				computeFrustumDimensions(camera.position, camera.viewDirection, camera.fovRadians, camera.aspectRatio,
					nearPlane, farPlane, nearDimensions, farDimensions, centerNear, centerFar);

				glm::vec3 nearTopLeft{};
				glm::vec3 nearTopRight{};
				glm::vec3 nearBottomLeft{};
				glm::vec3 nearBottomRight{};
				glm::vec3 farTopLeft{};
				glm::vec3 farTopRight{};
				glm::vec3 farBottomLeft{};
				glm::vec3 farBottomRight{};

				computeFrustumSplitCorners(camera.viewDirection, nearDimensions, farDimensions, centerNear, centerFar,
					nearTopLeft,
					nearTopRight,
					nearBottomLeft,
					nearBottomRight,
					farTopLeft,
					farTopRight,
					farBottomLeft,
					farBottomRight
				);


				glm::vec3 corners[] =
				{
					nearTopLeft,
					nearTopRight,
					nearBottomLeft,
					nearBottomRight,
					farTopLeft,
					farTopRight,
					farBottomLeft,
					farBottomRight,
				};

				float longestDiagonal = glm::distance(nearTopLeft, farBottomRight);

				glm::vec3 minVal{};
				glm::vec3 maxVal{};

				for (int i = 0; i < 8; i++)
				{
					glm::vec4 corner(corners[i], 1);

					glm::vec4 lightViewCorner = lightView * corner;

					if (i == 0)
					{
						minVal = lightViewCorner;
						maxVal = lightViewCorner;
					}
					else
					{
						if (lightViewCorner.x < minVal.x) { minVal.x = lightViewCorner.x; }
						if (lightViewCorner.y < minVal.y) { minVal.y = lightViewCorner.y; }
						if (lightViewCorner.z < minVal.z) { minVal.z = lightViewCorner.z; }

						if (lightViewCorner.x > maxVal.x) { maxVal.x = lightViewCorner.x; }
						if (lightViewCorner.y > maxVal.y) { maxVal.y = lightViewCorner.y; }
						if (lightViewCorner.z > maxVal.z) { maxVal.z = lightViewCorner.z; }

					}

				}

				//keep them square and the same size:
				//https://www.youtube.com/watch?v=u0pk1LyLKYQ&t=99s&ab_channel=WesleyLaFerriere
				if (1)
				{
					float firstSize = maxVal.x - minVal.x;
					float secondSize = maxVal.y - minVal.y;
					float thirdSize = maxVal.z - minVal.z;

					{
						float ratio = longestDiagonal / firstSize;

						glm::vec2 newVecValues = { minVal.x, maxVal.x };
						float dimension = firstSize;
						float dimensionOver2 = dimension / 2.f;

						newVecValues -= glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);
						newVecValues *= ratio;
						newVecValues += glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);

						minVal.x = newVecValues.x;
						maxVal.x = newVecValues.y;
					}

					{
						float ratio = longestDiagonal / secondSize;

						glm::vec2 newVecValues = { minVal.y, maxVal.y };
						float dimension = secondSize;
						float dimensionOver2 = dimension / 2.f;

						newVecValues -= glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);
						newVecValues *= ratio;
						newVecValues += glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);

						minVal.y = newVecValues.x;
						maxVal.y = newVecValues.y;
					}

					{//todo this size probably can be far-close
						float ratio = longestDiagonal / thirdSize;

						glm::vec2 newVecValues = { minVal.z, maxVal.z };
						float dimension = thirdSize;
						float dimensionOver2 = dimension / 2.f;

						newVecValues -= glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);
						newVecValues *= ratio;
						newVecValues += glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);

						minVal.z = newVecValues.x;
						maxVal.z = newVecValues.y;
					}

				}

				float near_plane = minVal.z - zOffset;
				float far_plane = maxVal.z;


				glm::vec2 ortoMin = { minVal.x, minVal.y };
				glm::vec2 ortoMax = { maxVal.x, maxVal.y };

				//remove shadow flicker
				if (1)
				{
					glm::vec2 shadowMapSize(directionalShadows.shadowSize, directionalShadows.shadowSize);
					glm::vec2 worldUnitsPerTexel = (ortoMax - ortoMin) / shadowMapSize;

					ortoMin /= worldUnitsPerTexel;
					ortoMin = glm::floor(ortoMin);
					ortoMin *= worldUnitsPerTexel;

					ortoMax /= worldUnitsPerTexel;
					ortoMax = glm::floor(ortoMax);
					ortoMax *= worldUnitsPerTexel;

					float zWorldUnitsPerTexel = (far_plane - near_plane) / directionalShadows.shadowSize;

					near_plane /= zWorldUnitsPerTexel;
					far_plane /= zWorldUnitsPerTexel;
					near_plane = glm::floor(near_plane);
					far_plane = glm::floor(far_plane);
					near_plane *= zWorldUnitsPerTexel;
					far_plane *= zWorldUnitsPerTexel;

				}

				glm::mat4 lightProjection = glm::ortho(ortoMin.x, ortoMax.x, ortoMin.y, ortoMax.y, near_plane, far_plane);

				return lightProjection;

			};

			if (directionalLightsShadows)
			{
				int shadowCastCount = 0;
				for (int lightIndex = 0; lightIndex < internal.directionalLights.size(); lightIndex++)
				{
					if (internal.directionalLights[lightIndex].castShadowsIndex >= 0)
					{
						internal.directionalLights[lightIndex].castShadowsIndex = shadowCastCount;

						glm::vec3 lightDir = internal.directionalLights[lightIndex].direction;
						//glm::mat4 lightView = lookAtSafe(-lightDir, {}, { 0.f,1.f,0.f });

						glm::mat4 lightView = lookAtSafe(camera.position - (lightDir), camera.position, { 0.f,1.f,0.f });
						//glm::mat4 lightView = lookAtSafe(camera.position, camera.position + lightDir, { 0.f,1.f,0.f });

						//zoffset is used to move the light further


						float zOffsets[] = { 15 / 200.f,0,0 };

						glBindFramebuffer(GL_FRAMEBUFFER, directionalShadows.cascadesFbo);
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							directionalShadows.cascadesTexture, 0, shadowCastCount); //last is layer

						glClear(GL_DEPTH_BUFFER_BIT);
						float lastNearPlane = 0.0001;

						for (int i = 0; i < DirectionalShadows::CASCADES; i++)
						{
							glViewport(0, directionalShadows.shadowSize * i,
								directionalShadows.shadowSize, directionalShadows.shadowSize);

							auto projection = calculateLightProjectionMatrix(lightDir, lightView,
								directionalShadows.frustumSplits[i] * camera.farPlane,
								lastNearPlane,
								zOffsets[i] * camera.farPlane);

							//this will add some precision but add artefacts todo?
							//lastNearPlane = zOffsets[i] * camera.farPlane;

							internal.directionalLights[lightIndex].lightSpaceMatrix[i] = projection * lightView;

							renderModelsShadows(internal.directionalLights[lightIndex].lightSpaceMatrix[i]);


						}

						shadowCastCount++;
					}
				}
			}
			
		}

		if (internal.spotLights.size())
		{
			bool shouldRenderStaticGeometryAllLights = internal.perFrameFlags.staticGeometryChanged
				|| internal.perFrameFlags.shouldUpdateSpotShadows;

			int spotLightsShadowsCount = 0;
			for (const auto& i : internal.spotLights)
			{
				if (i.castShadows) { spotLightsShadowsCount++; }
			}

			if (spotLightsShadowsCount != spotShadows.textureCount
				|| spotShadows.shadowSize != spotShadows.currentShadowSize
				)
			{
				spotShadows.allocateTextures(spotLightsShadowsCount);
				shouldRenderStaticGeometryAllLights = true; 
			}

			if (spotLightsShadowsCount)
			{
				glViewport(0, 0, spotShadows.shadowSize, spotShadows.shadowSize);

				glBindFramebuffer(GL_FRAMEBUFFER, spotShadows.staticGeometryfbo);

				int shadowCastCount = 0;
				for (int lightIndex = 0; lightIndex < internal.spotLights.size(); lightIndex++)
				{
					if (internal.spotLights[lightIndex].castShadows)
					{
						if (shouldRenderStaticGeometryAllLights || internal.spotLights[lightIndex].changedThisFrame)
						{
							glm::vec3 lightDir = internal.spotLights[lightIndex].direction;
							glm::vec3 lightPos = internal.spotLights[lightIndex].position;
							glm::mat4 lightView = lookAtSafe(lightPos, lightPos + lightDir, { 0.f,1.f,0.f });
							float fov = internal.spotLights[lightIndex].cosHalfAngle;
							fov = std::acos(fov);
							fov *= 2;

							float nearPlane = 0.01f;
							float farPlane = internal.spotLights[lightIndex].dist;

							auto projection = glm::perspective(fov, 1.f, nearPlane, farPlane);
							internal.spotLights[lightIndex].lightSpaceMatrix = projection * lightView;
							internal.spotLights[lightIndex].shadowIndex = shadowCastCount;

							internal.spotLights[lightIndex].nearPlane = nearPlane;
							internal.spotLights[lightIndex].farPlane = farPlane;

							internal.spotLights[lightIndex].changedThisFrame = false;

							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								spotShadows.staticGeometryTextures, 0, shadowCastCount);
							glClear(GL_DEPTH_BUFFER_BIT);
							//render only static geometry first
							renderModelsShadows(internal.spotLights[lightIndex].lightSpaceMatrix, true, true);

						}

						shadowCastCount++;
					}

				}

				//copy static geometry
				glCopyImageSubData(spotShadows.staticGeometryTextures, GL_TEXTURE_2D_ARRAY, 0,
					0, 0, 0,
					spotShadows.shadowTextures, GL_TEXTURE_2D_ARRAY, 0,
					0, 0, 0,
					spotShadows.shadowSize, spotShadows.shadowSize, spotLightsShadowsCount
				);

				//render dynamic geometry on top
				glBindFramebuffer(GL_FRAMEBUFFER, spotShadows.fbo);
				shadowCastCount = 0;
				for (int lightIndex = 0; lightIndex < internal.spotLights.size(); lightIndex++)
				{
					if (internal.spotLights[lightIndex].castShadows)
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							spotShadows.shadowTextures, 0, shadowCastCount);

						renderModelsShadows(internal.spotLights[lightIndex].lightSpaceMatrix, true, false);
						shadowCastCount++;
					}

				}
			}


		}
	
		#pragma endregion


		#pragma region stuff to be bound for rendering the pre pass geometry

		glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);

		internal.lightShader.prePass.shader.bind();

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);


		#pragma endregion


		#pragma region z pre pass
		for (auto& i : internal.cpuEntities)
		{
			if (!i.isVisible())
			{
				continue;
			}

			
			auto projMat = camera.getProjectionMatrix();
			auto viewMat = camera.getWorldToViewMatrix();
			auto transformMat = i.transform.getTransformMatrix();
			auto modelViewProjMat = projMat * viewMat * transformMat;
			glUniformMatrix4fv(internal.lightShader.prePass.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);

			for (auto &i : i.models)
			{
				auto m = internal.getMaterialIndex(i.material);

				if (m < 0)
				{
					glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);
				}
				else
				{

					auto t = internal.materialTexturesData[m];
					auto tId = internal.getTextureIndex(t.albedoTexture);

					if (tId < 0)
					{
						glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);

					}
					else
					{
						auto texture = internal.loadedTextures[tId];

						glUniform1i(internal.lightShader.prePass.u_hasTexture, 1);
						glUniform1i(internal.lightShader.prePass.u_albedoSampler, 0);
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, texture.texture.id);
					}

				}

				glBindVertexArray(i.vertexArray);
			
				if (i.indexBuffer)
				{
					glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
				}
				else
				{
					glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
				}
			}

			

		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		#pragma endregion 


		#pragma region stuff to be bound for rendering the geometry


		internal.lightShader.geometryPassShader.bind();
		internal.lightShader.getSubroutines();

		//glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		//glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(internal.lightShader.textureSamplerLocation, 0);
		glUniform1i(internal.lightShader.normalMapSamplerLocation, 1);
		//glUniform1i(lightShader.skyBoxSamplerLocation, 2);
		glUniform1i(internal.lightShader.RMASamplerLocation, 3);
		glUniform1i(internal.lightShader.u_emissiveTexture, 4);


		//material buffer
		if (internal.materials.size())
		{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MaterialValues) * internal.materials.size()
			, &internal.materials[0], GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, internal.lightShader.materialBlockBuffer);
		}

		GLsizei n;
		glGetProgramStageiv(internal.lightShader.geometryPassShader.id,
			GL_FRAGMENT_SHADER,
			GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
			&n);

		GLuint* indices = new GLuint[n]{ 0 };


		glDepthFunc(GL_EQUAL);

		#pragma endregion


		#pragma region g buffer render

		//first we render the entities in the gbuffer
		for (auto& entity : internal.cpuEntities)
		{
			if (!entity.isVisible())
			{
				continue;
			}

			if (entity.models.empty())
			{
				continue;
			}

			auto projMat = camera.getProjectionMatrix();
			auto viewMat = camera.getWorldToViewMatrix();
			auto transformMat = entity.transform.getTransformMatrix();
			auto modelViewProjMat = projMat * viewMat * transformMat;

			glUniformMatrix4fv(internal.lightShader.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);
			glUniformMatrix4fv(internal.lightShader.u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
			glUniformMatrix4fv(internal.lightShader.u_motelViewTransform, 1, GL_FALSE, &(viewMat * transformMat)[0][0]);
			
			
			bool changed = 1;

			for (auto& i : entity.models)
			{

				int materialId = internal.getMaterialIndex(i.material);

				if (materialId == -1)
				{
					continue;
				}

				glUniform1i(internal.lightShader.materialIndexLocation, materialId);


				TextureDataForMaterial textureData = internal.materialTexturesData[materialId];

				int rmaLoaded = 0;
				int albedoLoaded = 0;
				int normalLoaded = 0;

				GpuTexture* albedoTextureData = this->getTextureData(textureData.albedoTexture);
				if (albedoTextureData != nullptr)
				{
					albedoLoaded = 1;
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, albedoTextureData->id);
				}

				GpuTexture* normalMapTextureData = this->getTextureData(textureData.normalMapTexture);
				if (normalMapTextureData != nullptr && normalMapTextureData->id != 0)
				{
					normalLoaded = 1;
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, normalMapTextureData->id);
				}

				GpuTexture* rmaTextureData = this->getTextureData(textureData.pbrTexture.texture);
				if (rmaTextureData != nullptr && rmaTextureData->id != 0)
				{
					rmaLoaded = 1;
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, rmaTextureData->id);
				}

				int emissiveTextureLoaded = 0;
				GpuTexture* emissiveTextureData = this->getTextureData(textureData.emissiveTexture);
				if (emissiveTextureData != nullptr && emissiveTextureData->id != 0)
				{
					emissiveTextureLoaded = 1;
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, emissiveTextureData->id);
				}


				if (emissiveTextureLoaded)
				{
					if (indices[internal.lightShader.getEmmisiveSubroutineLocation] !=
						internal.lightShader.emissiveSubroutine_sampled)
					{
						indices[internal.lightShader.getEmmisiveSubroutineLocation] = 
							internal.lightShader.emissiveSubroutine_sampled;
						changed = 1;
					}
				}
				else
				{
					if (indices[internal.lightShader.getEmmisiveSubroutineLocation] != 
						internal.lightShader.emissiveSubroutine_notSampled)
					{
						indices[internal.lightShader.getEmmisiveSubroutineLocation] = 
							internal.lightShader.emissiveSubroutine_notSampled;
						changed = 1;
					}
				}

				if (normalLoaded && internal.lightShader.normalMap)
				{
					if (indices[internal.lightShader.normalSubroutineLocation] !=
						internal.lightShader.normalSubroutine_normalMap)
					{
						indices[internal.lightShader.normalSubroutineLocation] = 
							internal.lightShader.normalSubroutine_normalMap;
						changed = 1;
					}
				}
				else
				{
					if (indices[internal.lightShader.normalSubroutineLocation] != 
						internal.lightShader.normalSubroutine_noMap)
					{
						indices[internal.lightShader.normalSubroutineLocation] = 
							internal.lightShader.normalSubroutine_noMap;
						changed = 1;
					}
				}

				if (rmaLoaded)
				{

					if (indices[internal.lightShader.materialSubroutineLocation] != 
						internal.lightShader.materialSubroutine_functions[textureData.pbrTexture.RMA_loadedTextures])
					{
						indices[internal.lightShader.materialSubroutineLocation] = 
							internal.lightShader.materialSubroutine_functions[textureData.pbrTexture.RMA_loadedTextures];
						changed = 1;
					}

				}
				else
				{
					if (indices[internal.lightShader.materialSubroutineLocation] != 
						internal.lightShader.materialSubroutine_functions[0])
					{
						indices[internal.lightShader.materialSubroutineLocation] = 
							internal.lightShader.materialSubroutine_functions[0];
						changed = 1;
					}

				}


				if (albedoLoaded != 0)
				{
					if (indices[internal.lightShader.getAlbedoSubroutineLocation] != 
						internal.lightShader.albedoSubroutine_sampled)
					{
						indices[internal.lightShader.getAlbedoSubroutineLocation] = 
							internal.lightShader.albedoSubroutine_sampled;
						changed = 1;
					}
				}
				else
					if (indices[internal.lightShader.getAlbedoSubroutineLocation] != 
						internal.lightShader.albedoSubroutine_notSampled)
					{
						indices[internal.lightShader.getAlbedoSubroutineLocation] = 
							internal.lightShader.albedoSubroutine_notSampled;
						changed = 1;
					}


				if (changed)
				{
					glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, n, indices);
				}
				changed = 0;

				{
					glBindVertexArray(i.vertexArray);

					if (i.indexBuffer)
					{
						glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
					}
					else
					{
						glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
					}
				}

			}



		}

		delete[] indices;

		#pragma endregion


		glBindVertexArray(0);
		glDepthFunc(GL_LESS);


		//we draw a rect several times so we keep this vao binded
		glBindVertexArray(internal.lightShader.quadDrawer.quadVAO);
		
		#pragma region ssao

		if(internal.lightShader.useSSAO)
		{
			glViewport(0, 0, internal.adaptiveW / 2, internal.adaptiveH / 2);

			glUseProgram(ssao.shader.id);

			glBindBuffer(GL_UNIFORM_BUFFER, ssao.ssaoUniformBlockBuffer);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SSAO::SsaoShaderUniformBlockData),
				&ssao.ssaoShaderUniformBlockData);

			glUniformMatrix4fv(ssao.u_projection, 1, GL_FALSE,
				&(camera.getProjectionMatrix())[0][0]);

			glUniformMatrix4fv(ssao.u_view, 1, GL_FALSE,
				&(camera.getWorldToViewMatrix())[0][0]);

			glUniform3fv(ssao.u_samples, 64, &(ssao.ssaoKernel[0][0]));

			glBindFramebuffer(GL_FRAMEBUFFER, ssao.ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.positionViewSpace]);
			glUniform1i(ssao.u_gPosition, 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);
			glUniform1i(ssao.u_gNormal, 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, ssao.noiseTexture);
			glUniform1i(ssao.u_texNoise, 2);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);

		#pragma region ssao "blur" (more like average blur)
			glViewport(0, 0, internal.adaptiveW / 4, internal.adaptiveH / 4);

			glBindFramebuffer(GL_FRAMEBUFFER, ssao.blurBuffer);
			ssao.blurShader.bind();
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ssao.ssaoColorBuffer);
			glUniform1i(ssao.u_ssaoInput, 0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);
		#pragma endregion
		}
		#pragma endregion


		#pragma region do the lighting pass

		glBindFramebuffer(GL_FRAMEBUFFER, postProcess.fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(internal.lightShader.lightingPassShader.id);

		glUniform1i(internal.lightShader.light_u_positions, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position]);

		glUniform1i(internal.lightShader.light_u_normals, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);

		glUniform1i(internal.lightShader.light_u_albedo, 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo]);

		glUniform1i(internal.lightShader.light_u_materials, 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);


		glUniform1i(internal.lightShader.light_u_skyboxFiltered, 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.preFilteredMap);

		glUniform1i(internal.lightShader.light_u_skyboxIradiance, 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.convolutedTexture);

		glUniform1i(internal.lightShader.light_u_brdfTexture, 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, internal.lightShader.brdfTexture.id);

		glUniform1i(internal.lightShader.light_u_emmisive, 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.emissive]);

		glUniform1i(internal.lightShader.light_u_cascades, 8);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D_ARRAY, directionalShadows.cascadesTexture);

		glUniform1i(internal.lightShader.light_u_spotShadows, 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D_ARRAY, spotShadows.shadowTextures);

		glUniform1i(internal.lightShader.light_u_pointShadows, 10);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, pointShadows.shadowTextures);


		glUniform3f(internal.lightShader.light_u_eyePosition, camera.position.x, camera.position.y, camera.position.z);

		glUniformMatrix4fv(internal.lightShader.light_u_view, 1, GL_FALSE, &(camera.getWorldToViewMatrix()[0][0]) );

		if (internal.pointLights.size())
		{//todo laziness if lights don't change and stuff
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.pointLightsBlockBuffer);
		
			glBufferData(GL_SHADER_STORAGE_BUFFER, internal.pointLights.size() * sizeof(internal::GpuPointLight)
				, &internal.pointLights[0], GL_STREAM_DRAW);
		
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, internal.lightShader.pointLightsBlockBuffer);
		
		}
		glUniform1i(internal.lightShader.light_u_pointLightCount, internal.pointLights.size());

		if (internal.directionalLights.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.directionalLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, internal.directionalLights.size() * sizeof(internal::GpuDirectionalLight)
				, &internal.directionalLights[0], GL_STREAM_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, internal.lightShader.directionalLightsBlockBuffer);

		}
		glUniform1i(internal.lightShader.light_u_directionalLightCount, internal.directionalLights.size());

		if (internal.spotLights.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.spotLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, internal.spotLights.size() * sizeof(internal::GpuSpotLight),
				internal.spotLights.data(), GL_STREAM_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, internal.lightShader.spotLightsBlockBuffer);
		}
		glUniform1i(internal.lightShader.light_u_spotLightCount, internal.spotLights.size());


		//update the uniform block with data for the light shader
		internal.lightShader.lightPassUniformBlockCpuData.ambientLight = glm::vec4(skyBox.color, 0.f);

		if (skyBox.texture != 0
			&& skyBox.convolutedTexture != 0
			&& skyBox.preFilteredMap != 0
			)
		{
			internal.lightShader.lightPassUniformBlockCpuData.skyBoxPresent = true;
		}
		else
		{
			internal.lightShader.lightPassUniformBlockCpuData.skyBoxPresent = false;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, internal.lightShader.lightPassShaderData.lightPassDataBlockBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightShader::LightPassData),
			&internal.lightShader.lightPassUniformBlockCpuData);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	#pragma endregion

		#pragma region bloom blur
		
		if(internal.lightShader.bloom)
		{
			
			bool horizontal = 1; bool firstTime = 1;
			postProcess.gausianBLurShader.bind();
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(postProcess.u_toBlurcolorInput, 0);
			glViewport(0, 0, internal.adaptiveW/2, internal.adaptiveH/2);


			for (int i = 0; i < internal.lightShader.bloomBlurPasses*2; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[horizontal]);
				glClear(GL_COLOR_BUFFER_BIT);
				glUniform1i(postProcess.u_horizontal, horizontal);

				glBindTexture(GL_TEXTURE_2D,
					firstTime ? postProcess.colorBuffers[1] : postProcess.bluredColorBuffer[!horizontal]);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				horizontal = !horizontal;
				firstTime = false;

			}
			glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);

		}

	#pragma endregion

		#pragma region do the post process stuff and draw to the screen


		if (antiAlias.usingFXAA || adaptiveResolution.useAdaptiveResolution)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, adaptiveResolution.fbo);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		glUseProgram(postProcess.postProcessShader.id);

		//color data
		glUniform1i(postProcess.u_colorTexture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[0]);

		//bloom data
		glUniform1i(postProcess.u_bloomTexture, 1);
		glActiveTexture(GL_TEXTURE1);

		if(internal.lightShader.bloom)
		{

			if (internal.lightShader.bloomBlurPasses <= 0)
			{
				glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[1]);
			}

			glUniform1f(postProcess.u_bloomIntensity, postProcess.bloomIntensty);

		}else
		{
			glUniform1f(postProcess.u_bloomIntensity, 0);

			//todo uniform block for this and also probably boolean to check if using bloom or not
			glBindTexture(GL_TEXTURE_2D, 0);

		}

		if (internal.lightShader.useSSAO)
		{
			glUniform1i(postProcess.u_useSSAO, 1);
			//todo change ssao_finalColor_exponent
			glUniform1f(postProcess.u_ssaoExponent, ssao.ssao_finalColor_exponent);
			
			
			glUniform1i(postProcess.u_ssao, 3);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ssao.blurColorBuffer);
			

		}else
		{
			glUniform1i(postProcess.u_useSSAO, 0);

		}

		glUniform1i(postProcess.u_bloomNotBluredTexture, 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);

		
		glUniform1f(postProcess.u_exposure, internal.lightShader.lightPassUniformBlockCpuData.exposure);

		//blend with skybox
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisable(GL_BLEND);


	#pragma endregion

	#pragma region draw to screen and fxaa

		glViewport(0, 0, internal.w, internal.h);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (antiAlias.usingFXAA || adaptiveResolution.useAdaptiveResolution)
		{

			if (antiAlias.usingFXAA)
			{
				antiAlias.shader.bind();
				glUniform1i(antiAlias.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, adaptiveResolution.texture);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
			else
			{
				antiAlias.noAAshader.bind();
				glUniform1i(antiAlias.noAAu_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, adaptiveResolution.texture);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			
		}

	#pragma endregion



	#pragma region copy depth buffer for later forward rendering
		glBindVertexArray(0);

		//glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		//glBlitFramebuffer(
		//  0, 0, adaptiveW, adaptiveH, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		//);

		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	#pragma endregion


		//reset per frame flags
		internal.perFrameFlags = {};

	}

	void Renderer3D::updateWindowMetrics(int x, int y)
	{

		internal.w = x; internal.h = y;

		if (adaptiveResolution.useAdaptiveResolution && 
			adaptiveResolution.shouldUseAdaptiveResolution)
		{
			internal.adaptiveW = internal.w * adaptiveResolution.rezRatio;
			internal.adaptiveH = internal.h * adaptiveResolution.rezRatio;
		}
		else
		{
			internal.adaptiveW = internal.w;
			internal.adaptiveH = internal.h;
		}
		

		//gbuffer
		gBuffer.resize(internal.adaptiveW, internal.adaptiveH);

		//ssao
		ssao.resize(internal.adaptiveW, internal.adaptiveH);
	
		//bloom buffer and color buffer
		postProcess.resize(internal.adaptiveW, internal.adaptiveH);

		adaptiveResolution.resize(internal.adaptiveW, internal.adaptiveH);

	}

	//todo remove
	void Renderer3D::renderADepthMap(GLuint texture)
	{
		glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, renderDepthMap.fbo);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


		renderDepthMap.shader.bind();
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, 1024, 1024);

		glBindVertexArray(internal.lightShader.quadDrawer.quadVAO);
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(renderDepthMap.u_depth, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glEnable(GL_DEPTH_TEST);

	}

	void Renderer3D::renderSkyBox()
	{
		//todo move into render
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		viewMat = glm::mat4(glm::mat3(viewMat));

		auto viewProjMat = projMat * viewMat;

		internal.skyBoxLoaderAndDrawer.draw(viewProjMat, skyBox, 
			internal.lightShader.lightPassUniformBlockCpuData.exposure,
			internal.lightShader.lightPassUniformBlockCpuData.ambientLight);
	}

	void Renderer3D::renderSkyBoxBefore()
	{
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		viewMat = glm::mat4(glm::mat3(viewMat));

		auto viewProjMat = projMat * viewMat;

		internal.skyBoxLoaderAndDrawer.drawBefore(viewProjMat, skyBox,
			internal.lightShader.lightPassUniformBlockCpuData.exposure,
			internal.lightShader.lightPassUniformBlockCpuData.ambientLight);
	}

	SkyBox Renderer3D::loadSkyBox(const char *names[6])
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.loadTexture(names, skyBox);
		return skyBox;
	}

	SkyBox Renderer3D::loadSkyBox(const char *name, int format)
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.loadTexture(name, skyBox, format);
		return skyBox;
	}

	SkyBox Renderer3D::loadHDRSkyBox(const char *name)
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.loadHDRtexture(name, skyBox);
		return skyBox;
	}

	void Renderer3D::deleteSkyBoxTextures(SkyBox& skyBox)
	{
		skyBox.clearTextures();
	}

	SkyBox Renderer3D::atmosfericScattering(glm::vec3 sun, float g, float g2)
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.atmosphericScattering(sun, g, g2, skyBox);
		return skyBox;
	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer3D::SSAO::create(int w, int h)
	{
		std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
		std::uniform_real_distribution<float> randomFloatsSmaller(0.1f, 0.9f); //avoid ssao artefacts
		std::default_random_engine generator;

		ssaoKernel.reserve(64);

		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)	// z component is always positive
			);
			sample = glm::normalize(sample);

			float scale = (float)i / 64.0;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);

		}
		//std::shuffle(ssaoKernel.begin(), ssaoKernel.end(), generator);


		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);


		shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/ssao/ssao.frag");


		u_projection = getUniform(shader.id, "u_projection");
		u_view = getUniform(shader.id, "u_view");
		u_gPosition = getUniform(shader.id, "u_gPosition");
		u_gNormal = getUniform(shader.id, "u_gNormal");
		u_texNoise = getUniform(shader.id, "u_texNoise");
		u_samples = getUniform(shader.id, "samples[0]");
		

		glGenBuffers(1, &ssaoUniformBlockBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, ssaoUniformBlockBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SsaoShaderUniformBlockData),
			&ssaoShaderUniformBlockData, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, ssaoUniformBlockBuffer);
		
		u_SSAODATA = glGetUniformBlockIndex(shader.id, "u_SSAODATA");
		glUniformBlockBinding(shader.id, u_SSAODATA, 2);

		//blur
		blurShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/ssao/blur.frag");
		
		glGenFramebuffers(1, &blurBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer);
		glGenTextures(1, &blurColorBuffer);
		glBindTexture(GL_TEXTURE_2D, blurColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffer, 0);
		u_ssaoInput = getUniform(blurShader.id, "u_ssaoInput");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		resize(w, h);
	}

	void Renderer3D::SSAO::resize(int w, int h)
	{
		if (currentDimensions.x != w || currentDimensions.y != h)
		{

			glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w / 2, h / 2, 0, GL_RED, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, blurColorBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w / 4, h / 4, 0, GL_RED, GL_FLOAT, NULL);
			
			currentDimensions = glm::ivec2(w, h);
		}
	
	}

	void Renderer3D::PostProcess::create(int w, int h)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		//one for colors the other for things to be bloomed
		glGenTextures(2, colorBuffers);
		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach texture to framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
		}

		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		
		postProcessShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/postProcess.frag");
		u_colorTexture = getUniform(postProcessShader.id, "u_colorTexture");
		u_bloomTexture = getUniform(postProcessShader.id, "u_bloomTexture");
		u_bloomNotBluredTexture = getUniform(postProcessShader.id, "u_bloomNotBluredTexture");
		u_bloomIntensity = getUniform(postProcessShader.id, "u_bloomIntensity");
		u_exposure = getUniform(postProcessShader.id, "u_exposure");

		u_useSSAO = getUniform(postProcessShader.id, "u_useSSAO");
		u_ssaoExponent = getUniform(postProcessShader.id, "u_ssaoExponent");
		u_ssao = getUniform(postProcessShader.id, "u_ssao");


		gausianBLurShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/gausianBlur.frag");
		u_toBlurcolorInput = getUniform(gausianBLurShader.id, "u_toBlurcolorInput");
		u_horizontal = getUniform(gausianBLurShader.id, "u_horizontal");


		glGenFramebuffers(2, blurFbo);
		glGenTextures(2, bluredColorBuffer);

		for(int i=0;i <2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[i]);

			glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bluredColorBuffer[i], 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
		resize(w, h);
	}

	void Renderer3D::PostProcess::resize(int w, int h)
	{
		if (currentDimensions.x != w || currentDimensions.y != h)
		{
			currentDimensions = glm::ivec2(w, h);

			for (int i = 0; i < 2; i++)
			{
				glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
			}
			
			for (int i = 0; i < 2; i++)
			{
				glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w / 2, h / 2, 0, GL_RGBA, GL_FLOAT, NULL);
			}
		}
	}

	void Renderer3D::InternalStruct::PBRtextureMaker::init()
	{
		shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/modelLoader/mergePBRmat.frag");
		glGenFramebuffers(1, &fbo);


	}

	GLuint Renderer3D::InternalStruct::PBRtextureMaker::createRMAtexture(int w, int h, GpuTexture roughness, 
		GpuTexture metallic, GpuTexture ambientOcclusion, GLuint quadVAO)
	{

		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//todo set the quality of this texture in a function parameter.
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	

		glBindVertexArray(quadVAO);

		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, roughness.id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, metallic.id);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ambientOcclusion.id);


		glViewport(0, 0, w, h);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindTexture(GL_TEXTURE_2D, texture);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return texture;
	}

	void Renderer3D::DirectionalShadows::allocateTextures(int count)
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, cascadesTexture);

		textureCount = count;
		currentShadowSize = shadowSize;

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, shadowSize, shadowSize * CASCADES,
			textureCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	}

	void Renderer3D::DirectionalShadows::create()
	{
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glGenTextures(1, &cascadesTexture);
		glGenFramebuffers(1, &cascadesFbo);

		glBindTexture(GL_TEXTURE_2D_ARRAY, cascadesTexture);
		

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, cascadesFbo);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_ARRAY, cascadesTexture, 0);
		//glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cascadesTexture, 0, 0); //last is layer

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void Renderer3D::AdaptiveResolution::create(int w, int h)
	{

		if (useAdaptiveResolution)
		{
			currentDimensions = glm::ivec2(w*rezRatio, h*rezRatio);
		}
		else
		{
			currentDimensions = glm::ivec2(w, h);
		}

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, currentDimensions.x, currentDimensions.y
			, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	}

	void Renderer3D::AdaptiveResolution::resize(int w, int h)
	{
		if (currentDimensions.x != w || currentDimensions.y != h)
		{

			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			currentDimensions = glm::ivec2(w, h);
		}
	}


	void Renderer3D::AntiAlias::create(int w, int h)
	{

		shader.loadShaderProgramFromFile("shaders/drawQuads.vert",
			"shaders/aa/fxaa.frag");

		u_texture = getUniform(shader.id, "u_texture");

		noAAshader.loadShaderProgramFromFile("shaders/drawQuads.vert",
			"shaders/aa/noaa.frag");

		noAAu_texture = getUniform(noAAshader.id, "u_texture");


	}

	void Renderer3D::RenderDepthMap::create()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "renderdepth map frame buffer failed\n";
		}

		shader.loadShaderProgramFromFile
			("shaders/drawQuads.vert", "shaders/drawDepth.frag");
		u_depth = getUniform(shader.id, "u_depth");
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void Renderer3D::PointShadows::create()
	{

		glGenTextures(1, &shadowTextures);
		glGenTextures(1, &staticGeometryTextures);

		GLuint textures[2] = { shadowTextures , staticGeometryTextures };

		for (int i = 0; i < 2; i++)
		{

			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textures[i]);

			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		}

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glGenFramebuffers(1, &staticGeometryFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, staticGeometryFbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer3D::PointShadows::allocateTextures(int count)
	{
		textureCount = count;
		currentShadowSize = shadowSize;

		GLuint textures[2] = { shadowTextures , staticGeometryTextures };

		for (int i = 0; i < 2; i++)
		{

			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textures[i]);
			glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0,
				GL_DEPTH_COMPONENT32, shadowSize, shadowSize,
				textureCount*6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	
		}

	}


	void Renderer3D::SpotShadows::create()
	{
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glGenTextures(1, &shadowTextures);
		glGenTextures(1, &staticGeometryTextures);

		glGenFramebuffers(1, &fbo);
		glGenFramebuffers(1, &staticGeometryfbo);

		glBindTexture(GL_TEXTURE_2D_ARRAY, shadowTextures);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		glBindTexture(GL_TEXTURE_2D_ARRAY, staticGeometryTextures);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, staticGeometryfbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer3D::SpotShadows::allocateTextures(int count)
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, shadowTextures);
		textureCount = count;
		currentShadowSize = shadowSize;

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, shadowSize, shadowSize,
			textureCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	
		glBindTexture(GL_TEXTURE_2D_ARRAY, staticGeometryTextures);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, shadowSize, shadowSize,
			textureCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	}

	void Renderer3D::GBuffer::create(int w, int h)
	{

		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		glGenTextures(bufferCount, buffers);


		//todo refactor
		//todo glGetInternalFormativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, 1, &preferred_format).
		//https://www.khronos.org/opengl/wiki/Common_Mistakes#Extensions_and_OpenGL_Versions

		glBindTexture(GL_TEXTURE_2D, buffers[position]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers[position], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[normal]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffers[normal], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[albedo]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffers[albedo], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, buffers[material], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[positionViewSpace]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, buffers[positionViewSpace], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[emissive]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, buffers[emissive], 0);


		unsigned int attachments[bufferCount] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		glDrawBuffers(bufferCount, attachments);

		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1, 1);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Gbuffer failed\n";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		resize(w, h);
	}

	void Renderer3D::GBuffer::resize(int w, int h)
	{
		if (currentDimensions.x != w || currentDimensions.y != h)
		{
			glBindTexture(GL_TEXTURE_2D, buffers[position]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[normal]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[albedo]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[material]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[positionViewSpace]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[emissive]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

			glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
			
			currentDimensions = glm::ivec2(w, h);
		}
	}

};
#pragma endregion


