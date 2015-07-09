#include "project.hpp"
#include <iostream>
#include <ctime>

int main(int argc, char* argv[])
{
	int return_code = 0;
	try
	{
		Project project;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return_code = 1;
		std::cin.get();
	}
	catch (...)
	{
		std::cerr << "Unknown exception caught at outmost level" << std::endl;
		return_code = 1;
		std::cin.get();
	}

	return return_code;
}

void __stdcall OutputDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::cout << message << std::endl;
	}
}

InputState::InputState()
{
	memset(keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
	mouse_left_down = false;
	mouse_right_down = false;
	mouse_x = 0;
	mouse_y = 0;
}

Project::Project()
	: window(nullptr)
	, glcontext(nullptr)
	, viewport_width(VIEWPORT_WIDTH_INITIAL)
	, viewport_height(VIEWPORT_HEIGHT_INITIAL)
	, running(true)
	, fps_camera(true)
{

}

Project::~Project()
{
	srand(time(NULL));

	SetupContext();
	SetupResources();
	Run();
}

void Project::SetupContext()
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, viewport_width, viewport_height);
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

	// Set vsync enabled.
	SDL_GL_SetSwapInterval(1);

	// Hide the mouse cursor.
	SDL_ShowCursor(0);
}

void Project::SetupResources()
{
	// Setup the camera starting attributes.
	camera_frustum = Frustum(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)viewport_width, (float)viewport_height);
	camera.SetProjection(camera_frustum.GetPerspectiveProjection());
	camera.SetPosition(glm::vec3(64.0f, 30.0f, 64.0f));
	camera.SetFacing(glm::vec3(0, 0, -1.0f));
	camera.RecalculateMatrices();

	// Create and initialize the constant and frame buffers.
	glGenBuffers(1, &uniform_buffer_constant);
	glGenBuffers(1, &uniform_buffer_frame);

	uniform_data_constant.ambient_light.intensity = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	uniform_data_constant.directional_lights[0].direction_W = glm::normalize(glm::vec4(1.0f, -1.0f, 0.0f, 0.0f));
	uniform_data_constant.directional_lights[0].intensity = glm::vec4(0.4f, 0.3f, 0.0f, 1.0f);
	uniform_data_constant.directional_lights[1].direction_W = glm::normalize(glm::vec4(0.0f, -1.0f, -1.0f, 0.0f));
	uniform_data_constant.directional_lights[1].intensity = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	uniform_data_constant.point_lights[0].position_W = glm::vec4(40.0f, 15.0f, 20.0f, 1.0f);
	uniform_data_constant.point_lights[0].intensity = glm::vec4(0.6f, 0.0f, 0.0f, 1.0f);
	uniform_data_constant.point_lights[0].cutoff = 20.0f;
	uniform_data_constant.point_lights[1].position_W = glm::vec4(10.0f, 35.0f, 15.0f, 1.0f);
	uniform_data_constant.point_lights[1].intensity = glm::vec4(0.0f, 0.0f, 0.6f, 1.0f);
	uniform_data_constant.point_lights[1].cutoff = 20.0f;
	uniform_data_constant.spot_lights[0].position_W = glm::vec4(5.0f, 5.0f, 0.0f, 1.0f);
	uniform_data_constant.spot_lights[0].direction_W = glm::normalize(glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f));
	uniform_data_constant.spot_lights[0].intensity = glm::vec4(0.0f, 0.6f, 0.4f, 1.0f);
	uniform_data_constant.spot_lights[0].cutoff = 20.0f;
	uniform_data_constant.spot_lights[0].angle = glm::radians(22.5f);
	uniform_data_constant.spot_lights[1].position_W = glm::vec4(45.0f, 15.0f, 30.0f, 1.0f);
	uniform_data_constant.spot_lights[1].direction_W = glm::normalize(glm::vec4(0.5f, -1.0f, 0.5f, 1.0f));
	uniform_data_constant.spot_lights[1].intensity = glm::vec4(0.6f, 0.6f, 0.4f, 1.0f);
	uniform_data_constant.spot_lights[1].cutoff = 20.0f;
	uniform_data_constant.spot_lights[1].angle = glm::radians(22.5f);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_CONSTANT, uniform_buffer_constant);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferConstant), &uniform_data_constant, GL_STATIC_DRAW);
	
	// Setup the scene objects.
	terrain = std::make_unique<Terrain>();
	emitters[0] = std::make_unique<ShaftEmitter>(glm::vec3(64.0f, 2.0f + terrain->GetHeight(64.0f, 80.0f), 80.0f));
	emitters[1] = std::make_unique<SmokeEmitter>(glm::vec3(80.0f, 2.0f + terrain->GetHeight(80.0f, 64.0f), 64.0f));
	emitters[2] = std::make_unique<OrbitEmitter>(glm::vec3(80.0f, 2.0f + terrain->GetHeight(80.0f, 80.0f), 80.0f));
}

void Project::Run()
{
	Uint32 last_clock = SDL_GetTicks();
	while (running)
	{
		Uint32 current_clock = SDL_GetTicks();
		Uint32 delta_clock = static_cast<Uint32>(current_clock - last_clock);
		float dt = delta_clock * 0.001f;
		last_clock = current_clock;

		HandleEvents();
		if (SDL_GetMouseFocus() == window)
		{
			if (fps_camera)
				UpdateCameraFPS(dt);
			else
				UpdateCamera(dt);
		}
		
		UpdateScene(dt);
		RenderScene();
	}
}

void Project::HandleEvents()
{
	input_state_previous = input_state_current;

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
				viewport_width = e.window.data1;
				viewport_height = e.window.data2;
				glViewport(0, 0, viewport_width, viewport_height);
				camera_frustum.width = static_cast<float>(viewport_width);
				camera_frustum.height = static_cast<float>(viewport_height);
				camera.SetProjection(camera_frustum.GetPerspectiveProjection());
				camera.RecalculateMatrices();

				std::cout << "Window resized to " << e.window.data1 << "x" << e.window.data2 << std::endl;
			} break;
			}
		} break;

		case SDL_KEYDOWN:
		{
			input_state_current.keys[e.key.keysym.scancode] = true;
		} break;

		case SDL_KEYUP:
		{
			input_state_current.keys[e.key.keysym.scancode] = false;
		} break;

		case SDL_MOUSEMOTION:
		{
			input_state_current.mouse_x = e.motion.x;
			input_state_current.mouse_y = e.motion.y;
		} break;

		case SDL_MOUSEBUTTONDOWN:
		{
			if (e.button.button == SDL_BUTTON_LEFT)
				input_state_current.mouse_left_down = true;
			if (e.button.button == SDL_BUTTON_RIGHT)
				input_state_current.mouse_right_down = true;
		} break;

		case SDL_MOUSEBUTTONUP:
		{
			if (e.button.button == SDL_BUTTON_LEFT)
				input_state_current.mouse_left_down = false;
			if (e.button.button == SDL_BUTTON_RIGHT)
				input_state_current.mouse_left_down = false;
		} break;
		}
	}
}

void Project::UpdateCamera(float dt)
{
	if (input_state_current.mouse_left_down)
	{
		int dx = input_state_current.mouse_x - input_state_previous.mouse_x;
		int dy = input_state_current.mouse_y - input_state_previous.mouse_y;

		glm::vec3 facing = camera.GetFacing();
		float yaw = std::atan2(-facing.z, facing.x);
		float pitch = std::acos(facing.y);

		yaw -= dx * CAMERA_SENSITIVITY;
		pitch += dy * CAMERA_SENSITIVITY;
		pitch = glm::clamp(pitch, 0.01f, 3.13f);

		float h = std::sin(pitch);
		facing.x = h * std::cos(yaw);
		facing.y = std::cos(pitch);
		facing.z = -h * std::sin(yaw);

		camera.SetFacing(facing);
	}

	glm::vec3 displacement = camera.GetPosition();
	if (input_state_current.keys[SDL_SCANCODE_W] || input_state_current.keys[SDL_SCANCODE_UP])
		displacement += camera.GetFacing() * CAMERA_MOVE_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_S] || input_state_current.keys[SDL_SCANCODE_DOWN])
		displacement -= camera.GetFacing() * CAMERA_MOVE_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_A] || input_state_current.keys[SDL_SCANCODE_LEFT])
		displacement -= camera.GetRight() * CAMERA_MOVE_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_D] || input_state_current.keys[SDL_SCANCODE_RIGHT])
		displacement += camera.GetRight() * CAMERA_MOVE_SPEED * dt;

	camera.SetPosition(displacement);
	camera.RecalculateMatrices();
}

void Project::UpdateCameraFPS(float dt)
{
	// Update the facing.
	int dx;
	int dy;
	SDL_GetMouseState(&dx, &dy);
	dx = dx - (viewport_width / 2);
	dy = dy - (viewport_height / 2);

	glm::vec3 facing = camera.GetFacing();
	float yaw = std::atan2(-facing.z, facing.x);
	float pitch = std::acos(facing.y);

	yaw -= dx * CAMERA_SENSITIVITY;
	pitch += dy * CAMERA_SENSITIVITY;
	pitch = glm::clamp(pitch, 0.01f, 3.13f);

	float h = std::sin(pitch);
	facing.x = h * std::cos(yaw);
	facing.y = std::cos(pitch);
	facing.z = -h * std::sin(yaw);

	camera.SetFacing(facing);

	SDL_WarpMouseInWindow(window, viewport_width / 2, viewport_height / 2);

	// Update the movement.
	glm::vec3 displacement = camera.GetPosition();
	glm::vec3 direction = glm::normalize(glm::vec3(camera.GetFacing().x, 0.0f, camera.GetFacing().z));
	glm::vec3 right = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f));
	if (input_state_current.keys[SDL_SCANCODE_W] || input_state_current.keys[SDL_SCANCODE_UP])
		displacement += direction * CAMERA_MOVE_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_S] || input_state_current.keys[SDL_SCANCODE_DOWN])
		displacement -= direction * CAMERA_MOVE_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_A] || input_state_current.keys[SDL_SCANCODE_LEFT])
		displacement -= right * CAMERA_MOVE_SPEED * dt;
	if (input_state_current.keys[SDL_SCANCODE_D] || input_state_current.keys[SDL_SCANCODE_RIGHT])
		displacement += right * CAMERA_MOVE_SPEED * dt;
	
	displacement.y = 4.0f + terrain->GetHeight(displacement.x, displacement.z);
	
	camera.SetPosition(displacement);
	camera.RecalculateMatrices();
}

void Project::UpdateScene(float dt)
{
	if (input_state_current.keys[SDL_SCANCODE_ESCAPE] && !input_state_previous.keys[SDL_SCANCODE_ESCAPE])
		running = false;
	if (input_state_current.keys[SDL_SCANCODE_C] && !input_state_previous.keys[SDL_SCANCODE_C])
	{
		fps_camera = !fps_camera;
		SDL_ShowCursor(fps_camera ? 0 : 1);
	}

	for (int i = 0; i < 3; ++i)
		emitters[i]->Update(dt);
}

void Project::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update the per frame buffer.
	uniform_data_frame.camera_position_W = glm::vec4(camera.GetPosition(), 1.0f);
	uniform_data_frame.view_matrix = camera.GetView();
	uniform_data_frame.projection_matrix = camera.GetProjection();
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_FRAME, uniform_buffer_frame);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerFrame), &uniform_data_frame, GL_DYNAMIC_DRAW);

	// Render the scene objects.
	terrain->Render();
	for (int i = 0; i < 3; ++i)
		emitters[i]->Render();

	// Swap the back and front buffers.
	SDL_GL_SwapWindow(window);
}
