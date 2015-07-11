#include "objviewer.hpp"
#include <iostream>
#include <gli/gli.hpp>
#include <glm/gtx/transform.hpp>

int main(int argc, char* argv[])
{
	try
	{
		OBJViewer objviewer;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		std::cin.get();
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

InputState::InputState()
{
	memset(keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
	mouse_left_down = false;
	mouse_right_down = false;
	mouse_x = 0;
	mouse_y = 0;
}

OBJViewer::OBJViewer()
	: window(nullptr)
	, glcontext(nullptr)
	, model_angle(0.0f)
	, model_vertex_count(0)
	, model_vbo_positions(0)
	, model_vbo_normals(0)
	, model_vbo_texcoords(0)
	, model_vao(0)
	, model_texture(0)
	, mesh_vs(0)
	, mesh_fs(0)
	, mesh_program(0)
	, uniform_buffer_constant(0)
	, uniform_buffer_frame(0)
	, uniform_buffer_model(0)
	, diffuse_sampler(0)
	, viewport_width(VIEWPORT_WIDTH_INITIAL)
	, viewport_height(VIEWPORT_HEIGHT_INITIAL)
	, running(true)
{
	SetupContext();
	SetupResources();
	Run();
}

OBJViewer::~OBJViewer()
{
	glDetachShader(mesh_program, mesh_vs);
	glDetachShader(mesh_program, mesh_fs);
	glDeleteProgram(mesh_program);
	glDeleteShader(mesh_vs);
	glDeleteShader(mesh_fs);

	glDeleteBuffers(1, &uniform_buffer_constant);
	glDeleteBuffers(1, &uniform_buffer_frame);

	glDeleteBuffers(1, &uniform_buffer_model);
	glDeleteBuffers(1, &model_vbo_positions);
	glDeleteBuffers(1, &model_vbo_normals);
	glDeleteBuffers(1, &model_vbo_texcoords);
	glDeleteVertexArrays(1, &model_vao);
	glDeleteTextures(1, &model_texture);

	glDeleteSamplers(1, &diffuse_sampler);

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
}

void OBJViewer::SetupContext()
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
}

void OBJViewer::SetupResources()
{
	// Compile the shader program.
	mesh_vs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_MESH_VS).c_str(), GL_VERTEX_SHADER);
	mesh_fs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_MESH_FS).c_str(), GL_FRAGMENT_SHADER);
	mesh_program = glCreateProgram();
	glAttachShader(mesh_program, mesh_vs);
	glAttachShader(mesh_program, mesh_fs);
	LinkProgram(mesh_program);

	// Generate a texture sampler.
	glGenSamplers(1, &diffuse_sampler);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Setup the camera starting attributes.
	camera_frustum = Frustum(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)viewport_width, (float)viewport_height);
	camera.SetProjection(camera_frustum.GetPerspectiveProjection());
	camera.SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
	camera.SetFacing(glm::vec3(0, 0, -1.0f));
	camera.RecalculateMatrices();

	// Create and initialize the frame and constant uniform buffers.
	glGenBuffers(1, &uniform_buffer_frame);
	glGenBuffers(1, &uniform_buffer_constant);

	uniform_data_constant.ambient_light.intensity = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	uniform_data_constant.point_light.position_W = glm::vec4(0.0f, 5.0f, 5.0f, 1.0f);
	uniform_data_constant.point_light.intensity = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
	uniform_data_constant.point_light.cutoff = 15.0f;

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_CONSTANT, uniform_buffer_constant);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferConstant), &uniform_data_constant, GL_STATIC_DRAW);

	// Load the model.
	OBJ model;
	if (!LoadOBJ((DIRECTORY_MODELS + FILE_MODEL).c_str(), model))
	{
		throw std::runtime_error("Failed to load OBJ model: " + DIRECTORY_MODELS + FILE_MODEL);
	}

	// Setup the model buffers.
	glGenVertexArrays(1, &model_vao);
	glBindVertexArray(model_vao);

	glGenBuffers(1, &model_vbo_positions);
	glBindBuffer(GL_ARRAY_BUFFER, model_vbo_positions);
	glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(glm::vec3), &model.positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &model_vbo_normals);
	glBindBuffer(GL_ARRAY_BUFFER, model_vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &model_vbo_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, model_vbo_texcoords);
	glBufferData(GL_ARRAY_BUFFER, model.texcoords.size() * sizeof(glm::vec2), &model.texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	model_vertex_count = model.positions.size();

	// Load the model material.
	MTL material;
	if (!LoadMTL((DIRECTORY_MODELS + model.mtllib).c_str(), material))
	{
		throw std::runtime_error("Failed to load material library: " + DIRECTORY_MODELS + model.mtllib);
	}

	gli::storage model_texture_image = gli::load_dds((DIRECTORY_TEXTURES + material.map_Kd).c_str());
	glGenTextures(1, &model_texture);
	glBindTexture(GL_TEXTURE_2D, model_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, model_texture_image.dimensions(0).x, model_texture_image.dimensions(0).y, 0, GL_BGR, GL_UNSIGNED_BYTE, model_texture_image.data());

	// Setup the instance buffer.
	uniform_data_model.material_specular_color = glm::vec4(material.Ks, material.Ns);
	uniform_data_model.model_matrix = glm::scale(glm::vec3(0.01f, 0.01f, 0.01f));
	uniform_data_model.normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(uniform_data_model.model_matrix))));

	glGenBuffers(1, &uniform_buffer_model);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer_model);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &uniform_data_model, GL_DYNAMIC_DRAW);


}

void OBJViewer::Run()
{
	Uint32 last_clock = SDL_GetTicks();
	while (running)
	{
		Uint32 current_clock = SDL_GetTicks();
		Uint32 delta_clock = static_cast<Uint32>(current_clock - last_clock);
		float dt = delta_clock * 0.001f;
		last_clock = current_clock;

		HandleEvents();
		UpdateCamera(dt);
		UpdateScene(dt);
		RenderScene();
	}
}

void OBJViewer::HandleEvents()
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

void OBJViewer::UpdateCamera(float dt)
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
		displacement += camera.GetFacing() * CAMERA_MOVE_SPEED;
	if (input_state_current.keys[SDL_SCANCODE_S] || input_state_current.keys[SDL_SCANCODE_DOWN])
		displacement -= camera.GetFacing() * CAMERA_MOVE_SPEED;
	if (input_state_current.keys[SDL_SCANCODE_A] || input_state_current.keys[SDL_SCANCODE_LEFT])
		displacement -= camera.GetRight() * CAMERA_MOVE_SPEED;
	if (input_state_current.keys[SDL_SCANCODE_D] || input_state_current.keys[SDL_SCANCODE_RIGHT])
		displacement += camera.GetRight() * CAMERA_MOVE_SPEED;

	camera.SetPosition(displacement);
	camera.RecalculateMatrices();
}

void OBJViewer::UpdateScene(float dt)
{
	model_angle += MODEL_ROTATION_SPEED * dt;
	uniform_data_model.model_matrix = glm::scale(glm::vec3(0.05f, 0.05f, 0.05f)) * glm::rotate(model_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	uniform_data_model.normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(uniform_data_model.model_matrix))));
}

void OBJViewer::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uniform_data_frame.view_matrix = camera.GetView();
	uniform_data_frame.projection_matrix = camera.GetProjection();
	uniform_data_frame.camera_position_W = glm::vec4(camera.GetPosition(), 1.0f);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_FRAME, uniform_buffer_frame);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerFrame), &uniform_data_frame, GL_DYNAMIC_DRAW);

	glUseProgram(mesh_program);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE);
	glBindSampler(TEXTURE_UNIT_DIFFUSE, diffuse_sampler);
	glBindTexture(GL_TEXTURE_2D, model_texture);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer_model);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &uniform_data_model, GL_DYNAMIC_DRAW);

	glBindVertexArray(model_vao);
	glDrawArrays(GL_TRIANGLES, 0, model_vertex_count);

	SDL_GL_SwapWindow(window);
}