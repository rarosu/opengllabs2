#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <common/text.h>
#include <common/shader.h>
#include <common/camera.h>

#undef main

const int VIEWPORT_WIDTH = 800;
const int VIEWPORT_HEIGHT = 600;
const char* const FONT_FILEPATH = "../../../assets/fonts/FreeSans.ttf";
const char* const VS_FILEPATH = "../../../assets/shaders/mesh_textured.vs";
const char* const FS_FILEPATH = "../../../assets/shaders/mesh_textured.fs";
const float CAMERA_RADIUS = 5.0f;
const float CAMERA_ANGULAR_SPEED = (float) M_PI;
const float PERSPECTIVE_NEAR = 1.0f;
const float PERSPECTIVE_FAR = 100.0f;
const float PERSPECTIVE_FOV = glm::radians(75.0f);

FT_Library ft = nullptr;
FT_Face face = nullptr;
SDL_Window* window = nullptr;
SDL_GLContext context = nullptr;
GLuint vbo = 0;
GLuint vao = 0;
GLuint vertexCount = 0;
GLuint vshader = 0;
GLuint fshader = 0;
GLuint program = 0;
GLuint perFrameBuffer = 0;
GLuint perInstanceBuffer = 0;
Camera camera;
float cameraAngle = (float) M_PI * 0.5f;

struct PerFrameUniformBuffer
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
} perFrame;

struct PerInstanceUniformBuffer
{
	glm::mat4 modelMatrix;
	glm::mat4 normalMatrix;
} perInstance;

void InitializeContext();
void InitializeScene();
bool HandleEvents();
void CleanupScene();

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
			cameraAngle += CAMERA_ANGULAR_SPEED * dt;
			camera.SetPosition(CAMERA_RADIUS * glm::vec3(cos(cameraAngle), 0, -sin(cameraAngle)));
			camera.LookAt(glm::vec3(0, 0, 0));
			camera.RecalculateMatrices();
			perFrame.viewMatrix = camera.GetView();
			perFrame.projectionMatrix = camera.GetProjection();

			// Render the scene.
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glUseProgram(program);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, perFrameBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniformBuffer), &perFrame, GL_DYNAMIC_DRAW);

			glBindBufferBase(GL_UNIFORM_BUFFER, 1, perInstanceBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &perInstance, GL_DYNAMIC_DRAW);

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

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

	if (FT_New_Face(ft, FONT_FILEPATH, 0, &face) != 0)
	{
		throw std::runtime_error(std::string("Failed to load face: ") + FONT_FILEPATH);
	}

	if (FT_Set_Pixel_Sizes(face, 0, 48) != 0)
	{
		throw std::runtime_error(std::string("Failed to load face: ") + FONT_FILEPATH);
	}
}

void InitializeScene()
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};

	std::vector<Vertex> vertices(4);
	vertices[0].position = glm::vec3(-1.0f, -1.0f, +0.0f);
	vertices[1].position = glm::vec3(+1.0f, -1.0f, +0.0f);
	vertices[2].position = glm::vec3(-1.0f, +1.0f, +0.0f);
	vertices[3].position = glm::vec3(+1.0f, +1.0f, +0.0f);

	vertices[0].normal = glm::vec3(+0.0f, +0.0f, +1.0f);
	vertices[1].normal = glm::vec3(+0.0f, +0.0f, +1.0f);
	vertices[2].normal = glm::vec3(+0.0f, +0.0f, +1.0f);
	vertices[3].normal = glm::vec3(+0.0f, +0.0f, +1.0f);

	vertices[0].texcoord = glm::vec2(+0.0f, +1.0f);
	vertices[1].texcoord = glm::vec2(+1.0f, +1.0f);
	vertices[2].texcoord = glm::vec2(+0.0f, +0.0f);
	vertices[3].texcoord = glm::vec2(+1.0f, +0.0f);

	vertexCount = (GLuint) vertices.size();

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) sizeof(glm::vec3));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) (sizeof(glm::vec3) + sizeof(glm::vec3)) );
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	vshader = CompileShaderFromFile(VS_FILEPATH, GL_VERTEX_SHADER);
	fshader = CompileShaderFromFile(FS_FILEPATH, GL_FRAGMENT_SHADER);
	
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	LinkProgram(program);

	//perFrameIndex = glGetUniformBlockIndex(program, "PerFrame");
	//perInstanceIndex = glGetUniformBlockIndex(program, "PerInstance");

	glGenBuffers(1, &perFrameBuffer);
	glGenBuffers(1, &perInstanceBuffer);

	perInstance.modelMatrix = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

	camera.SetProjection(Camera::GetPerspectiveProjection(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)VIEWPORT_WIDTH, (float)VIEWPORT_HEIGHT));
	camera.SetPosition(glm::vec3(0, 0, CAMERA_RADIUS));
	camera.SetFacing(glm::vec3(0, 0, -1));
	camera.RecalculateMatrices();

	perFrame.viewMatrix = camera.GetView();
	perFrame.projectionMatrix = camera.GetProjection();
}

void CleanupScene()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);
}

bool HandleEvents()
{
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
						camera.SetProjection(Camera::GetPerspectiveProjection(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, e.window.data1, e.window.data2));

						std::cout << "Window resized to " << e.window.data1 << "x" << e.window.data2 << std::endl;
					} break;
				}
			} break;
				
		}

	}

	return true;
}
