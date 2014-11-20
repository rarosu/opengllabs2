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


#undef main

const int VIEWPORT_WIDTH = 800;
const int VIEWPORT_HEIGHT = 600;
const char* const FONT_FILEPATH = "../../assets/fonts/FreeSans.ttf";

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
GLuint perFrameIndex = 0;
GLuint perInstanceIndex = 0;
GLuint perFrameBuffer = 0;
GLuint perInstanceBuffer = 0;

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

		

		bool running = true;
		while (running)
		{
			running = HandleEvents();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glUseProgram(program);
			glBindBufferBase(GL_UNIFORM_BUFFER, perFrameIndex, perFrameBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniformBuffer), &perFrame, GL_STATIC_READ);

			glBindBufferBase(GL_UNIFORM_BUFFER, perInstanceIndex, perInstanceBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &perInstance, GL_STATIC_READ);

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
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
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

	vertexCount = vertices.size();

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

	vshader = CompileShaderFromFile("../../assets/shaders/mesh_textured.vs", GL_VERTEX_SHADER);
	fshader = CompileShaderFromFile("../../assets/shaders/mesh_textured.fs", GL_FRAGMENT_SHADER);
	
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	LinkProgram(program);

	perFrameIndex = glGetUniformBlockIndex(program, "PerFrame");
	perInstanceIndex = glGetUniformBlockIndex(program, "PerInstance");

	glGenBuffers(1, &perFrameBuffer);
	glGenBuffers(1, &perInstanceBuffer);

	perInstance.modelMatrix = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
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
		}

	}

	return true;
}

