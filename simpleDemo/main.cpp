#include <Windows.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../headerOnly/gl3d.h"

#include <chrono>

int w = 840;
int h = 640;


#define USE_GPU_ENGINE 1
#define DEBUG_OUTPUT 1

#undef min
#undef max

#pragma region gpu
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = USE_GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = USE_GPU_ENGINE;
}
#pragma endregion


int main()
{
#pragma region init

	if (!glfwInit())
	{
		std::cout << "err initializing glfw";
	}

	glfwWindowHint(GLFW_SAMPLES, 1);

#if DEBUG_OUTPUT
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* wind = glfwCreateWindow(w, h, "geam", nullptr, nullptr);
	glfwMakeContextCurrent(wind);
	glfwSwapInterval(0);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "err initializing glew";
	}

#pragma region enable debug output
#if DEBUG_OUTPUT
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gl3d::glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
#pragma endregion

	gl3d::Renderer3D renderer;
	renderer.init(w, h, 0);

	const char* names[6] =
	{ "resources/skyBoxes/ocean/right.jpg",
		"resources/skyBoxes/ocean/left.jpg",
		"resources/skyBoxes/ocean/top.jpg",
		"resources/skyBoxes/ocean/bottom.jpg",
		"resources/skyBoxes/ocean/front.jpg",
		"resources/skyBoxes/ocean/back.jpg" };

	renderer.skyBox = renderer.loadSkyBox(names, 0);

	auto rockMaterialModel = renderer.loadMaterial("resources/rock/rock.mtl", 0);


	gl3d::Model helmetModel = renderer.loadModel("resources/helmet/helmet.obj", 0);
	gl3d::Model steveModel = renderer.loadModel("resources/steve.glb", 0);
	auto steveMaterial = renderer.loadMaterial("resources/adventurer/adventurer.mtl", 0);
	

	gl3d::Transform transform{};

	//transform.rotation.x = glm::radians(90.f);
	gl3d::Entity entity = renderer.createEntity(steveModel, transform);
	//renderer.setEntityMeshMaterial(entity, 0, steveMaterial[0]);

	auto textures = renderer.getEntityMeshMaterialTextures(entity, 0);

	gl3d::GpuTexture{ renderer.getTextureOpenglId(textures.albedoTexture) }.setTextureQuality(gl3d::TextureLoadQuality::leastPossible);
	gl3d::GpuTexture{ renderer.getTextureOpenglId(textures.emissiveTexture) }.setTextureQuality(gl3d::TextureLoadQuality::leastPossible);
	gl3d::GpuTexture{ renderer.getTextureOpenglId(textures.pbrTexture.texture) }.setTextureQuality(gl3d::TextureLoadQuality::leastPossible);


	renderer.setEntityMeshMaterial(entity, 0, rockMaterialModel[0]);

#pragma region deltaTime
	int fpsCount = 0;
	float timeFpsCount = 0;
	int timeBeg = clock();
#pragma endregion

	while (!glfwWindowShouldClose(wind))
	{
	#pragma region window metrics
		glfwGetWindowSize(wind, &w, &h);
		w = std::max(w, 1);
		h = std::max(h, 1);
	#pragma endregion

	#pragma region deltatime
		int timeEnd = clock();
		float deltaTime = (timeEnd - timeBeg) / 1000.f;
		timeBeg = clock();

		timeFpsCount += deltaTime;
		fpsCount++;
		if (timeFpsCount > 1)
		{
			timeFpsCount -= 1;
			std::string name = std::to_string(fpsCount);
			name = "fps: " + name;
			glfwSetWindowTitle(wind, name.c_str());

			fpsCount = 0;
		}

	#pragma endregion


	#pragma region camera

		float speed = 4+0;
		glm::vec3 dir = {};
		if (GetAsyncKeyState('W'))
		{
			dir.z -= speed * deltaTime;
		}
		if (GetAsyncKeyState('S'))
		{
			dir.z += speed * deltaTime;
		}
		
		if (GetAsyncKeyState('A'))
		{
			dir.x -= speed * deltaTime;
		}
		if (GetAsyncKeyState('D'))
		{
			dir.x += speed * deltaTime;
		}
		
		if (GetAsyncKeyState('Q'))
		{
			dir.y -= speed * deltaTime;
		}
		if (GetAsyncKeyState('E'))
		{
			dir.y += speed * deltaTime;
		}
		
		renderer.camera.moveFPS(dir);
		
		{
			static glm::dvec2 lastMousePos = {};
			if (glfwGetMouseButton(wind, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				glm::dvec2 currentMousePos = {};
				glfwGetCursorPos(wind, &currentMousePos.x, &currentMousePos.y);
		
				float speed = 0.8f;
		
				glm::vec2 delta = lastMousePos - currentMousePos;
				delta *= speed * deltaTime;
		
				renderer.camera.rotateCamera(delta);
		
				lastMousePos = currentMousePos;
			}
			else
			{
				glfwGetCursorPos(wind, &lastMousePos.x, &lastMousePos.y);
		
			}
		}
		
		renderer.camera.aspectRatio = (float)w / h;

	#pragma endregion
		
		transform = renderer.getEntityTransform(entity);
		//transform.position.x = std::sin(clock() / 1000.f);
		transform.position.z = -1.f;
		renderer.setEntityTransform(entity, transform);

	#pragma region render and events
		renderer.render(deltaTime);

		glfwSwapBuffers(wind);
		glfwPollEvents();
		glViewport(0, 0, w, h);
		renderer.updateWindowMetrics(w, h);
	#pragma endregion

	}


	return 0;
}