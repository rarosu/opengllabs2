#define GLM_FORCE_RADIANS

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <gli/gli.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <common/text.h>
#include <common/shader.h>
#include <common/camera.h>
#include <common/model.h>

#undef main

const int VIEWPORT_WIDTH = 800;
const int VIEWPORT_HEIGHT = 600;
const float PERSPECTIVE_NEAR = 1.0f;
const float PERSPECTIVE_FAR = 100.0f;
const float PERSPECTIVE_FOV = glm::radians(75.0f);

const std::string ASSETS_FILEPATH = "../../../assets/";
const std::string FONTS_FILEPATH = ASSETS_FILEPATH + "fonts/";
const std::string MODELS_FILEPATH = ASSETS_FILEPATH + "models/";
const std::string SHADERS_FILEPATH = ASSETS_FILEPATH + "shaders/";
const std::string TEXTURES_FILEPATH = ASSETS_FILEPATH + "textures/";
const std::string FONT_FILE = "FreeSans.ttf";
const std::string MODEL_FILE = "crate.obj";
const std::string VS_FILE = "mesh_textured.vert";
const std::string FS_FILE = "mesh_textured.frag";

const float CRATE_ANGULAR_VELOCITY = 1.0f;
const int DIRECTIONAL_LIGHT_COUNT = 1;
const int POINT_LIGHT_COUNT = 1;
const int SPOT_LIGHT_COUNT = 1;

FT_Library ft = nullptr;
FT_Face face = nullptr;
SDL_Window* window = nullptr;
SDL_GLContext context = nullptr;
GLuint position_vbo = 0;
GLuint normal_vbo = 0;
GLuint texcoord_vbo = 0;
GLuint vao = 0;
GLuint vertexCount = 0;
GLuint vshader = 0;
GLuint fshader = 0;
GLuint program = 0;
GLuint perFrameBuffer = 0;
GLuint perInstanceBuffer = 0;
GLuint constantBuffer = 0;
GLuint texture = 0;
GLuint sampler = 0;
GLuint textureUnit = 0;
Camera camera;
float crateAngle = 0.0f;

struct InputState
{
	InputState() 
	{
		memset(keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
		mouseLeftDown = false;
		mouseRightDown = false;
		mouseX = 0;
		mouseY = 0;
	}

	bool keys[SDL_NUM_SCANCODES];
	bool mouseLeftDown;
	bool mouseRightDown;
	int mouseX;
	int mouseY;
} currentInput, previousInput;

struct AmbientLight
{
	glm::vec4 intensity;
};

struct DirectionalLight
{
	glm::vec4 directionW;
	glm::vec4 intensity;
};

struct PointLight
{
	glm::vec4 positionW;
	glm::vec4 intensity;
	float cutoff;
	float padding[3];
};

struct SpotLight
{
	glm::vec4 positionW;
	glm::vec4 directionW;
	glm::vec4 intensity;
	float cutoff;
	float angle;
	float padding[2];
};

struct PerFrameUniformBuffer
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec4 cameraPositionW;
} perFrame;

struct PerInstanceUniformBuffer
{
	glm::mat4 modelMatrix;
	glm::mat4 normalMatrix;

	// The last component is the shininess of the material.
	glm::vec4 materialSpecularColor;
} perInstance;

struct ConstantBuffer
{
	AmbientLight ambientLight;
	DirectionalLight directionalLights[DIRECTIONAL_LIGHT_COUNT];
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight spotLights[SPOT_LIGHT_COUNT];
} constant;

void InitializeContext();
void InitializeScene();
bool HandleEvents();
void CleanupScene();
void HandleCamera(float dt);

int main()
{
	int result = 0;
	try
	{
		InitializeContext();
		InitializeScene();
		
		Uint32 lastTick = SDL_GetTicks();
		float dt = 0.0f;

		bool running = true;
		while (running)
		{
			dt = (SDL_GetTicks() - lastTick) / 1000.0f;
			lastTick = SDL_GetTicks();

			running = HandleEvents();

			// Update the camera.
			HandleCamera(dt);
			camera.RecalculateMatrices();

			perFrame.viewMatrix = camera.GetView();
			perFrame.projectionMatrix = camera.GetProjection();
			perFrame.cameraPositionW = glm::vec4(camera.GetPosition(), 1.0f);

			// Update the crate.
			crateAngle += dt * CRATE_ANGULAR_VELOCITY;
			perInstance.modelMatrix = glm::rotate(crateAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			perInstance.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(perInstance.modelMatrix))));

			// Render the scene.
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glUseProgram(program);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, perFrameBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniformBuffer), &perFrame, GL_DYNAMIC_DRAW);

			glBindBufferBase(GL_UNIFORM_BUFFER, 1, perInstanceBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &perInstance, GL_DYNAMIC_DRAW);

			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindSampler(textureUnit, sampler);

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, vertexCount);

			SDL_GL_SwapWindow(window);
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		std::cin.get();

		result = 1;
	}
	catch (...)
	{
		std::cout << "Unrecognized exception caught" << std::endl;
		std::cin.get();

		result = 1;
	}

	CleanupScene();
	if (context != nullptr)
		SDL_GL_DeleteContext(context);
	if (window != nullptr)
		SDL_DestroyWindow(window);

	return result;
}

void InitializeContext()
{
	if (SDL_Init(SDL_INIT_TIMER) != 0)
	{
		throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
	}

	SDL_GLcontextFlag flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifndef NDEBUG
	flags = (SDL_GLcontextFlag) (flags | SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	window = SDL_CreateWindow("Texturing & Lighting", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create SDL window: ") + SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

	context = SDL_GL_CreateContext(window);
	if (context == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create OpenGL context: ") + SDL_GetError());
	}

	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();
	if (glewResult != GLEW_OK)
	{
		throw std::runtime_error(std::string("Failed to initialize GLEW: ") + (const char*) glewGetErrorString(glewResult));
	}

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);

	{
		int major;
		int minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

		std::cout << "OpenGL version: " << major << "." << minor << std::endl;
	}

	SDL_GL_SetSwapInterval(1);

	if (FT_Init_FreeType(&ft) != 0)
	{
		throw std::runtime_error("Failed to initialize FreeType");
	}

	if (FT_New_Face(ft, (FONTS_FILEPATH + FONT_FILE).c_str(), 0, &face) != 0)
	{
		throw std::runtime_error(std::string("Failed to load face: ") + FONT_FILE);
	}

	if (FT_Set_Pixel_Sizes(face, 0, 48) != 0)
	{
		throw std::runtime_error(std::string("Failed to load face: ") + FONT_FILE);
	}
}

void InitializeScene()
{
	OBJ model;
	if (!LoadOBJ((MODELS_FILEPATH + MODEL_FILE).c_str(), model))
		throw std::runtime_error(std::string("Failed to load model: ") + MODEL_FILE);

	vertexCount = (GLuint) model.positions.size();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(glm::vec3), &model.positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, model.texcoords.size() * sizeof(glm::vec2), &model.texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	vshader = CompileShaderFromFile((SHADERS_FILEPATH + VS_FILE).c_str(), GL_VERTEX_SHADER);
	fshader = CompileShaderFromFile((SHADERS_FILEPATH + FS_FILE).c_str(), GL_FRAGMENT_SHADER);
	
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	LinkProgram(program);

	glGenBuffers(1, &perFrameBuffer);
	glGenBuffers(1, &perInstanceBuffer);
	glGenBuffers(1, &constantBuffer);

	MTL material;
	if (!LoadMTL((MODELS_FILEPATH + model.mtllib).c_str(), material))
		throw std::runtime_error(std::string("Failed to load material: ") + model.mtllib);

	gli::storage crateImage = gli::load_dds((TEXTURES_FILEPATH + material.map_Kd).c_str());
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, crateImage.dimensions(0).x, crateImage.dimensions(0).y, 0, GL_RGB, GL_UNSIGNED_BYTE, crateImage.data());

	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	perInstance.modelMatrix = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	perInstance.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(perInstance.modelMatrix))));
	perInstance.materialSpecularColor = glm::vec4(material.Ks, material.Ns);

	camera.SetProjection(Camera::GetPerspectiveProjection(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)VIEWPORT_WIDTH, (float)VIEWPORT_HEIGHT));
	camera.SetPosition(glm::vec3(0, 0, 5));
	camera.SetFacing(glm::vec3(0, 0, -1));
	camera.RecalculateMatrices();

	perFrame.viewMatrix = camera.GetView();
	perFrame.projectionMatrix = camera.GetProjection();
	perFrame.cameraPositionW = glm::vec4(camera.GetPosition(), 1.0f);

	constant.ambientLight.intensity = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	constant.directionalLights[0].directionW = glm::normalize(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
	constant.directionalLights[0].intensity = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
	constant.pointLights[0].positionW = glm::vec4(0.0f, 5.0f, 5.0f, 1.0f);
	constant.pointLights[0].intensity = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
	constant.pointLights[0].cutoff = 15.0f;
	constant.spotLights[0].positionW = glm::vec4(0.0f, -5.0f, 5.0f, 1.0f);
	constant.spotLights[0].directionW = glm::normalize(glm::vec4(0.0f, 1.0f, -1.0f, 1.0f));
	constant.spotLights[0].intensity = glm::vec4(0.7f, 0.0f, 0.0f, 1.0f);
	constant.spotLights[0].cutoff = 15.0f;
	constant.spotLights[0].angle = glm::radians(45.0f);

	glBindBufferBase(GL_UNIFORM_BUFFER, 2, constantBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), &constant, GL_STATIC_DRAW);
}

void CleanupScene()
{
	glDeleteSamplers(1, &sampler);
	glDeleteTextures(1, &texture);
	glDeleteBuffers(1, &perFrameBuffer);
	glDeleteBuffers(1, &perInstanceBuffer);
	glDeleteBuffers(1, &constantBuffer);
	glDeleteBuffers(1, &position_vbo);
	glDeleteBuffers(1, &normal_vbo);
	glDeleteBuffers(1, &texcoord_vbo);
	glDeleteVertexArrays(1, &vao);
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);
}

bool HandleEvents()
{
	previousInput = currentInput;

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				return false;
			case SDL_WINDOWEVENT:
			{
				switch (e.window.event)
				{
					case SDL_WINDOWEVENT_RESIZED:
					{
						glViewport(0, 0, e.window.data1, e.window.data2);
						camera.SetProjection(Camera::GetPerspectiveProjection(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float) e.window.data1, (float) e.window.data2));

						std::cout << "Window resized to " << e.window.data1 << "x" << e.window.data2 << std::endl;
					} break;
				}
			} break;

			case SDL_KEYDOWN:
			{
				currentInput.keys[e.key.keysym.scancode] = true;
			} break;

			case SDL_KEYUP:
			{
				currentInput.keys[e.key.keysym.scancode] = false;
			} break;
			
			case SDL_MOUSEMOTION:
			{
				currentInput.mouseX = e.motion.x;
				currentInput.mouseY = e.motion.y;
			} break;

			case SDL_MOUSEBUTTONDOWN:
			{
				if (e.button.button == SDL_BUTTON_LEFT)
					currentInput.mouseLeftDown = true;
				if (e.button.button == SDL_BUTTON_RIGHT)
					currentInput.mouseRightDown = true;
			} break;

			case SDL_MOUSEBUTTONUP:
			{
				if (e.button.button == SDL_BUTTON_LEFT)
					currentInput.mouseLeftDown = false;
				if (e.button.button == SDL_BUTTON_RIGHT)
					currentInput.mouseRightDown = false;
			} break;
		}

	}

	return true;
}

void HandleCamera(float dt)
{
	float sensitivity = 0.005f;
	float speed = 0.1f;

	if (currentInput.mouseLeftDown)
	{
		int dx = currentInput.mouseX - previousInput.mouseX;
		int dy = currentInput.mouseY - previousInput.mouseY;

		glm::vec3 facing = camera.GetFacing();
		float yaw = std::atan2(-facing.z, facing.x);
		float pitch = std::acos(facing.y);

		//std::cout << "(" << dx << ", " << dy << ") (" << yaw << ", " << pitch << ")" << std::endl;

		yaw -= dx * sensitivity;
		pitch += dy * sensitivity;

		float h = std::sin(pitch);
		facing.x = h * std::cos(yaw);
		facing.y = std::cos(pitch);
		facing.z = -h * std::sin(yaw);

		camera.SetFacing(facing);
	}

	glm::vec3 displacement = camera.GetPosition();
	if (currentInput.keys[SDL_SCANCODE_W] || currentInput.keys[SDL_SCANCODE_UP])
		displacement += camera.GetFacing() * speed;
	if (currentInput.keys[SDL_SCANCODE_S] || currentInput.keys[SDL_SCANCODE_DOWN])
		displacement -= camera.GetFacing() * speed;
	if (currentInput.keys[SDL_SCANCODE_A] || currentInput.keys[SDL_SCANCODE_LEFT])
		displacement -= camera.GetRight() * speed;
	if (currentInput.keys[SDL_SCANCODE_D] || currentInput.keys[SDL_SCANCODE_RIGHT])
		displacement += camera.GetRight() * speed;

	camera.SetPosition(displacement);
}