#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <stdexcept>
#include <string>
#include <common/text.h>

#undef main

const int VIEWPORT_WIDTH = 800;
const int VIEWPORT_HEIGHT = 600;
const char* const FONT_FILEPATH = "../../assets/fonts/FreeSans.ttf";

FT_Library ft = nullptr;
FT_Face face = nullptr;
SDL_Window* window = nullptr;
SDL_GLContext context = nullptr;

void InitializeContext();
void InitializeScene();
bool HandleEvents();

int main()
{
	int result = 0;
	try
	{
		InitializeContext();
		InitializeScene();

		TextRenderer textRenderer;

		bool running = true;
		while (running)
		{
			running = HandleEvents();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			textRenderer.RenderText(face, "Hello world", 400, 300);

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

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);

	window = SDL_CreateWindow("Texturing & Lighting", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create SDL window: ") + SDL_GetError());
	}

	context = SDL_GL_CreateContext(window);
	if (context == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create OpenGL context: ") + SDL_GetError());
	}

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