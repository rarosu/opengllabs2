#include "raytracing.hpp"
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
	try
	{
		Raytracing raytracing;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "Unknown exception caught at outmost level" << std::endl;
		return 1;
	}

	return 0;
}

void __stdcall OutputDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::cout << message << std::endl;
	}
}

Raytracing::Raytracing()
	: window(nullptr)
	, glcontext(nullptr)
	, sampler(0)
	, overlay_texture(0)
	, overlay_position_vbo(0)
	, overlay_vao(0)
	, overlay_vs(0)
	, overlay_fs(0)
	, overlay_program(0)
	, viewport_width(VIEWPORT_WIDTH_INITIAL)
	, viewport_height(VIEWPORT_HEIGHT_INITIAL)
	, running(true)
{
	SetupContext();
	SetupResources();
	Run();
}

Raytracing::~Raytracing()
{

}

void Raytracing::SetupContext()
{
	// Initialize the basic SDL.
	if (SDL_Init(SDL_INIT_TIMER) != 0)
	{
		throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
	}

	window = SDL_CreateWindow(WINDOW_TITLE.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		viewport_width, viewport_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create SDL window: ") + SDL_GetError());
	}

	// Initialize the OpenGL context.
	SDL_GLcontextFlag flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifndef NDEBUG
	flags = (SDL_GLcontextFlag)(flags | SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

	glcontext = SDL_GL_CreateContext(window);
	if (glcontext == nullptr)
	{
		throw std::runtime_error(std::string("Failed to create OpenGL context: ") + SDL_GetError());
	}

	// Initialize the profile loader.
	if (gl3wInit() != 0)
	{
		throw std::runtime_error(std::string("Failed to initialize gl3w"));
	}

	if (gl3wIsSupported(4, 4) != 1)
	{
		throw std::runtime_error(std::string("OpenGL 4.4 is not supported"));
	}

	// Setup an error callback function.
	glDebugMessageCallback(OutputDebugMessage, nullptr);

	// Setup the initial OpenGL context state.
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glViewport(0, 0, viewport_width, viewport_height);
	glEnable(GL_CULL_FACE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	{
		int major;
		int minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

		std::cout << "OpenGL version: " << major << "." << minor << std::endl;
	}

	// Set vsync enabled.
	SDL_GL_SetSwapInterval(1);
}

void Raytracing::SetupResources()
{
	// Compile the shader program.
	overlay_vs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_OVERLAY_VS).c_str(), GL_VERTEX_SHADER);
	overlay_fs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_OVERLAY_FS).c_str(), GL_FRAGMENT_SHADER);
	overlay_program = glCreateProgram();
	glAttachShader(overlay_program, overlay_vs);
	glAttachShader(overlay_program, overlay_fs);
	LinkProgram(overlay_program);

	// Generate a texture sampler.
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Setup the camera starting attributes.
	camera_frustum = Frustum(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)viewport_width, (float)viewport_height);
	camera.SetProjection(camera_frustum.GetPerspectiveProjection());
	camera.SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
	camera.SetFacing(glm::vec3(0.0f, 0.0f, -1.0f));
	camera.RecalculateMatrices();

	// Setup the buffer.
	glm::vec2 positions[] = { glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, -1.0f), glm::vec2(-1.0f, 1.0f), glm::vec2(1.0f, 1.0f) };
	glm::vec2 texcoords[] = { glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f) };

	glGenVertexArrays(1, &overlay_vao);
	glBindVertexArray(overlay_vao);

	glGenBuffers(1, &overlay_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, overlay_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &overlay_texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, overlay_texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Setup the initial overlay texture.
	glGenTextures(1, &overlay_texture);
	glBindTexture(GL_TEXTURE_2D, overlay_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, viewport_width, viewport_height);

	// Setup the geometry.
	spheres[0].geometry = Sphere(glm::vec3(0.0f, 0.0f, -10.0f), 2.0f);
	spheres[0].color = glm::vec3(1.0f, 0.0f, 0.0f);
	
	spheres[1].geometry = Sphere(glm::vec3(5.0f, 0.0f, -10.0f), 2.0f);
	spheres[1].color = glm::vec3(0.8f, 0.2f, 0.0f);

	boxes[0].geometry = OBB(glm::vec3(-5.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 2.0f, 0.0f), 2.0f);
	boxes[0].color = glm::vec3(0.0f, 1.0f, 0.0f);

	boxes[1].geometry = OBB(glm::vec3(-5.0f, 5.0f, -10.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, -1.0f), 3.0f);
	boxes[1].color = glm::vec3(0.2f, 0.5f, 0.0f);

	triangles[0].geometry = Triangle(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 7.0f, 7.0f), glm::vec3(0.0f, 5.0f, 7.0f));
	triangles[0].color = glm::vec3(0.0f, 0.0f, 1.0f);

	triangles[1].geometry = Triangle(glm::vec3(-10.0f, 5.0f, -13.0f), glm::vec3(-13.0f, 5.0f, -13.0f), glm::vec3(-10.0f, 8.0f, -13.0f));
	triangles[1].color = glm::vec3(0.0f, 0.2f, 0.8f);

	light.position = glm::vec3(0.0f, -3.0f, 0.0f);
	light.color = glm::vec3(0.8f, 0.8f, 0.8f);
	light.cutoff = 15.0f;

	// Render the initial scene.
	RenderScene();
}

void Raytracing::Run()
{
	while (running)
	{
		HandleEvents();
	}
}

void Raytracing::HandleEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
			{
				running = false;
			} break;

			case SDL_WINDOWEVENT:
			{
				switch (e.window.event)
				{
					case SDL_WINDOWEVENT_RESIZED:
					{
						// Update the camera attributes.
						viewport_width = e.window.data1;
						viewport_height = e.window.data2;
						glViewport(0, 0, viewport_width, viewport_height);
						camera_frustum.width = static_cast<float>(viewport_width);
						camera_frustum.height = static_cast<float>(viewport_height);
						camera.SetProjection(camera_frustum.GetPerspectiveProjection());

						// Recreate the overlay texture.
						glDeleteTextures(1, &overlay_texture);
						glGenTextures(1, &overlay_texture);
						glBindTexture(GL_TEXTURE_2D, overlay_texture);
						glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, viewport_width, viewport_height);

						// Re-render the scene.
						RenderScene();

						std::cout << "Window resized to " << e.window.data1 << "x" << e.window.data2 << std::endl;
					} break;
				}
			} break;
		}
	}
}

void Raytracing::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT);

	// Use raytracing to fill the texture.
	RaytraceTexture();

	// Render the texture on the overlay.
	glUseProgram(overlay_program);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE);
	glBindSampler(TEXTURE_UNIT_DIFFUSE, sampler);
	glBindTexture(GL_TEXTURE_2D, overlay_texture);

	glBindVertexArray(overlay_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	SDL_GL_SwapWindow(window);
}

void Raytracing::RaytraceTexture()
{
	// Perform the raytracing.
	std::vector<glm::u8vec3> texture_data(viewport_width * viewport_height);
	for (int y = 0; y < viewport_height; ++y)
	{
		for (int x = 0; x < viewport_width; ++x)
		{
			Ray ray = GetRayFromScreenCoordinates(x, y);
			glm::vec3 normalized_color = IntersectRayVsScene(ray);
			texture_data[y * viewport_width + x] = glm::u8vec3(normalized_color.r * 255, normalized_color.g * 255, normalized_color.b * 255);
		}
	}

	// Update the texture.
	glBindTexture(GL_TEXTURE_2D, overlay_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, viewport_width, viewport_height, GL_RGB, GL_UNSIGNED_BYTE, &texture_data[0]);
}

Ray Raytracing::GetRayFromScreenCoordinates(int screen_x, int screen_y) const
{
	// Transform the screen coordinates to homogeneous coordinates
	glm::vec4 n(2.0f * (float(screen_x) / viewport_width) - 1.0f,
				2.0f * (float(viewport_height - screen_y) / viewport_height) - 1.0f,
				0.0f,
				1.0f);

	glm::vec4 f(n.x,
				n.y,
				1.0f,
				1.0f);

	// Get the inverse projection and view matrix and transform near and far into world space.
	glm::mat4 inverse_projection = glm::inverse(camera.GetProjection());
	glm::mat4 inverse_view = glm::inverse(camera.GetView());
	glm::mat4 inverse_projection_view = inverse_view * inverse_projection;

	n = inverse_projection_view * n;
	f = inverse_projection_view * f;

	f /= f.w;

	// Construct a ray
	Ray ray;
	ray.direction = glm::normalize(glm::vec3(f - n));
	ray.origin = glm::vec3(n);

	return ray;
}

glm::vec3 Raytracing::IntersectRayVsScene(const Ray& ray) const
{
	glm::vec3 color = glm::u8vec3(0, 0, 0);
	float t = 100000.0f;

	for (int k = 0; k < SPHERE_COUNT; ++k)
	{
		Ray::Intersection i = ray.intersect(spheres[k].geometry);
		if (i.intersected && i.t < t)
		{
			t = i.t;
			color = spheres[k].color;
		}
	}

	for (int k = 0; k < BOX_COUNT; ++k)
	{
		Ray::Intersection i = ray.intersect(boxes[k].geometry);
		if (i.intersected && i.t < t)
		{
			t = i.t;
			color = boxes[k].color;
		}
	}

	for (int k = 0; k < TRIANGLE_COUNT; ++k)
	{
		Ray::Intersection i = ray.intersect(triangles[k].geometry);
		if (i.intersected && i.t < t)
		{
			t = i.t;
			color = triangles[k].color;
		}
	}

	return color;
}