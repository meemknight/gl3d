#include <Windows.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../headerOnly/gl3d.h"

#include <chrono>
#include <random>

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


std::vector<gl3d::Entity> balls;

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
	gl3d::Renderer3D renderer;


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
	glDebugMessageCallback(gl3d::glDebugOutput, &renderer);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
#pragma endregion

	renderer.init(w, h, "resources/BRDFintegrationMap.png");

	const char* names[6] =
	{ "resources/skyBoxes/ocean/right.jpg",
		"resources/skyBoxes/ocean/left.jpg",
		"resources/skyBoxes/ocean/top.jpg",
		"resources/skyBoxes/ocean/bottom.jpg",
		"resources/skyBoxes/ocean/front.jpg",
		"resources/skyBoxes/ocean/back.jpg" };

	renderer.skyBox = renderer.loadSkyBox(names);
	//renderer.skyBox = renderer.atmosfericScattering({0,1,0}, {0.2,0.2,0.5}, {0.6,0.2,0.1}, 10);

	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/Newport_Loft_Ref.hdr", 0);
	//renderer.skyBox.color = {0.2,0.3,0.8};

	//auto rockMaterialModel = renderer.loadMaterial("resources/rock/rock.mtl", 0);
	auto model = renderer.loadModel("resources/sphere.obj", gl3d::TextureLoadQuality::maxQuality, 1.f);

	for (int x = -10; x < 10; x++)
		for (int y = -10; y < 10; y++)
			for (int z = 0; z < 20; z++)
			{				
				gl3d::Transform transform{};
				transform.position = glm::vec3(x,y,z)*2.2f;

				auto entity = renderer.createEntity(model, transform, false);
				balls.push_back(entity);
			}

	std::cout << "Balls cound: " << balls.size() << "\n";
	//transform.rotation.x = glm::radians(90.f);


	//renderer.setEntityMeshMaterial(entity, 0, steveMaterial[0]);

	

	//renderer.setEntityMeshMaterial(entity, 0, rockMaterialModel[0]);

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
		int a = 0;
		//change position
		if (1)
		{

			for (int i = 0; i < balls.size(); i++)
			{
				auto t = renderer.getEntityTransform(balls[i]);
			
				renderer.setEntityTransform(balls[i], t);
			}

		}

		//change material
		if (1 && !balls.empty())
		{
			static int counter = 0;

			//static float timer;
			//timer += deltaTime;
			//if (timer >= 1)
			for(int i=0; i<4; i++)
			{
				//timer -= 1;
				//auto m = renderer.getEntityMeshMaterialValues(balls[counter], 0);
				auto m = gl3d::MaterialValues{};

				auto getRandomFloat = []()
				{
					return (rand() % 1000) / 1000.f;

					//std::uniform_real_distribution<float> dist(0, 1.f);
					//std::random_device d;
					//return dist(d);
				};

				m.metallic = getRandomFloat();
				m.roughness = getRandomFloat();
				m.kd.r = getRandomFloat();
				m.kd.g = getRandomFloat();
				m.kd.b = getRandomFloat();

				if (getRandomFloat() > 0.80f)
				{
					m.emmisive = getRandomFloat();
				}
				else
				{
					m.emmisive = 0;
				}

				renderer.setEntityMeshMaterialValues(balls[counter], 0, m);

				counter++;
				if (counter >= balls.size()) { counter = 0; }
			}


		}


		//rotate camera
		if (0)
		{
			static float rotation = 0;
			rotation += 3.141592 * deltaTime * 0.7;
			if (rotation >= 3.141592 * 2) { rotation -= 3.141592 * 2; }

			glm::vec3 cameraPos(0, 0, 5);
			cameraPos = glm::rotate(rotation, glm::vec3{0,1,0}) * glm::vec4(cameraPos,1);
			
			//std::cout << glm::length(cameraPos) << "\n";

			renderer.camera.position = cameraPos;
			renderer.camera.viewDirection = -glm::normalize(cameraPos);



		}


		float speed = 40;


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
		
				float speed = 0.7f;
		
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
		
		//transform = renderer.getEntityTransform(entity);
		//transform.position.x = std::sin(clock() / 1000.f);
		//transform.position.z = -1.f;
		//renderer.setEntityTransform(entity, transform);

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