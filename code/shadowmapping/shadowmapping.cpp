#include "shadowmapping.hpp"
#include <iostream>
#include <gli/gli.hpp>
#include <glm/gtx/transform.hpp>

int main(int argc, char* argv[])
{
	int return_code = 0;
	try
	{
		Shadowmapping lighting;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return_code = 1;
	}
	catch (...)
	{
		std::cerr << "Unknown exception caught at outmost level" << std::endl;
		return_code = 1;
	}

	std::cin.get();
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

Entity::Entity()
	: vertex_count(0)
	, vbo_positions(0)
	, vbo_normals(0)
	, vbo_texcoords(0)
	, vao(0)
	, texture(0)
	, uniform_buffer(0)
{}

Shadowmapping::Shadowmapping()
	: window(nullptr)
	, glcontext(nullptr)
	, cube_angle(0.0f)
	, mesh_vs(0)
	, mesh_fs(0)
	, mesh_program(0)
	, uniform_buffer_constant(0)
	, uniform_buffer_frame(0)
	, diffuse_sampler(0)
	, depth_vs(0)
	, depth_fs(0)
	, depth_program(0)
	, shadowmap_texture_array(0)
	, shadowmap_sampler(0)
	, shadowmap_fbo(0)
	, shadowmap_width(SHADOWMAP_WIDTHS[SHADOWMAP_RESOLUTION_DEFAULT])
	, shadowmap_height(SHADOWMAP_HEIGHTS[SHADOWMAP_RESOLUTION_DEFAULT])
	, shadowmap_resolution_index(SHADOWMAP_RESOLUTION_DEFAULT)
	, viewport_width(VIEWPORT_WIDTH_INITIAL)
	, viewport_height(VIEWPORT_HEIGHT_INITIAL)
	, running(true)
{
	SetupContext();
	SetupResources();
	Run();
}

Shadowmapping::~Shadowmapping()
{
	glDetachShader(mesh_program, mesh_vs);
	glDetachShader(mesh_program, mesh_fs);
	glDeleteProgram(mesh_program);
	glDeleteShader(mesh_vs);
	glDeleteShader(mesh_fs);

	glDetachShader(depth_program, depth_vs);
	glDetachShader(depth_program, depth_fs);
	glDeleteProgram(depth_program);
	glDeleteShader(depth_vs);
	glDeleteShader(depth_fs);

	glDeleteBuffers(1, &uniform_buffer_constant);
	glDeleteBuffers(1, &uniform_buffer_frame);

	glDeleteBuffers(1, &cube.uniform_buffer);
	glDeleteBuffers(1, &cube.vbo_positions);
	glDeleteBuffers(1, &cube.vbo_normals);
	glDeleteBuffers(1, &cube.vbo_texcoords);
	glDeleteVertexArrays(1, &cube.vao);
	glDeleteTextures(1, &cube.texture);

	glDeleteBuffers(1, &plane.uniform_buffer);
	glDeleteBuffers(1, &plane.vbo_positions);
	glDeleteBuffers(1, &plane.vbo_normals);
	glDeleteBuffers(1, &plane.vbo_texcoords);
	glDeleteVertexArrays(1, &plane.vao);
	glDeleteTextures(1, &plane.texture);

	glDeleteSamplers(1, &diffuse_sampler);
	glDeleteSamplers(1, &shadowmap_sampler);

	glDeleteTextures(1, &shadowmap_texture_array);
	glDeleteFramebuffers(1, &shadowmap_fbo);
}

void Shadowmapping::SetupContext()
{
	// Initialize the basic SDL.
	if (SDL_Init(SDL_INIT_TIMER) != 0)
	{
		throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
	}

	window = SDL_CreateWindow("Shadowmapping",
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
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
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

	if (gl3wIsSupported(4, 3) != 1)
	{
		throw std::runtime_error(std::string("OpenGL 4.4 is not supported"));
	}


	// Initialize the extension wrangler.
	//glewExperimental = GL_TRUE;
	//GLenum glewResult = glewInit();
	//if (glewResult != GLEW_OK)
	//{
	//	throw std::runtime_error(std::string("Failed to initialize GLEW: ") + (const char*)glewGetErrorString(glewResult));
	//}
	//glGetError(); // Clear the error buffer caused by GLEW.

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

void Shadowmapping::SetupResources()
{
	// Compile the shader programs.
	mesh_vs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_MESH_VS).c_str(), GL_VERTEX_SHADER);
	mesh_fs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_MESH_FS).c_str(), GL_FRAGMENT_SHADER);
	mesh_program = glCreateProgram();
	glAttachShader(mesh_program, mesh_vs);
	glAttachShader(mesh_program, mesh_fs);
	LinkProgram(mesh_program);

	depth_vs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_DEPTH_VS).c_str(), GL_VERTEX_SHADER);
	depth_fs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_DEPTH_FS).c_str(), GL_FRAGMENT_SHADER);
	depth_program = glCreateProgram();
	glAttachShader(depth_program, depth_vs);
	glAttachShader(depth_program, depth_fs);
	LinkProgram(depth_program);

	// Generate the diffuse sampler.
	glGenSamplers(1, &diffuse_sampler);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(diffuse_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Generate the shadow map sampler.
	glGenSamplers(1, &shadowmap_sampler);
	glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Setup the camera starting attributes.
	camera_frustum = Frustum(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)viewport_width, (float)viewport_height);
	camera.SetProjection(camera_frustum.GetPerspectiveProjection());
	camera.SetPosition(glm::vec3(0, 5.0f, 5.0f));
	camera.SetFacing(glm::vec3(0, 0, -1.0f));
	camera.RecalculateMatrices();

	// Create and initialize the frame and constant uniform buffers.
	glGenBuffers(1, &uniform_buffer_frame);
	glGenBuffers(1, &uniform_buffer_constant);

	uniform_data_constant.ambient_light.intensity = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);

	// Calculate the bias matrix for transforming depth from the interval [-1, 1] to [0, 1].
	uniform_data_constant.bias_matrix = glm::mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	
	// Update the shadowmap resources (texture array and constant buffer).
	UpdateShadowmapResources(SHADOWMAP_RESOLUTION_DEFAULT, SPOT_LIGHT_COUNT_DEFAULT);

	// Update the constant uniform buffer.
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_CONSTANT, uniform_buffer_constant);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferConstant), &uniform_data_constant, GL_STATIC_DRAW);

	// Generate the shadowmap framebuffer object.
	glGenFramebuffers(1, &shadowmap_fbo);

	// Load the models and setup the entities.
	LoadModel(FILE_CUBE_MODEL.c_str(), cube);
	LoadModel(FILE_PLANE_MODEL.c_str(), plane);

	plane.uniform_data.model_matrix = glm::scale(glm::vec3(15.0f, 1.0f, 15.0f)) * glm::translate(glm::vec3(0.0f, -3.0f, 0.0f));
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, plane.uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &plane.uniform_data, GL_DYNAMIC_DRAW);
}

void Shadowmapping::LoadModel(const char* filepath, Entity& entity)
{
	// Load the model.
	OBJ model;
	if (!LoadOBJ((DIRECTORY_MODELS + filepath).c_str(), model))
	{
		throw std::runtime_error("Failed to load OBJ model: " + DIRECTORY_MODELS + filepath);
	}

	// Setup the buffers.
	glGenVertexArrays(1, &entity.vao);
	glBindVertexArray(entity.vao);

	glGenBuffers(1, &entity.vbo_positions);
	glBindBuffer(GL_ARRAY_BUFFER, entity.vbo_positions);
	glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(glm::vec3), &model.positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &entity.vbo_normals);
	glBindBuffer(GL_ARRAY_BUFFER, entity.vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &entity.vbo_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, entity.vbo_texcoords);
	glBufferData(GL_ARRAY_BUFFER, model.texcoords.size() * sizeof(glm::vec2), &model.texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	entity.vertex_count = model.positions.size();

	// Load the material.
	MTL material;
	if (!LoadMTL((DIRECTORY_MODELS + model.mtllib).c_str(), material))
	{
		throw std::runtime_error("Failed to load material library: " + DIRECTORY_MODELS + model.mtllib);
	}

	gli::storage cube_texture_image = gli::load_dds((DIRECTORY_TEXTURES + material.map_Kd).c_str());
	glGenTextures(1, &entity.texture);
	glBindTexture(GL_TEXTURE_2D, entity.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, cube_texture_image.dimensions(0).x, cube_texture_image.dimensions(0).y, 0, GL_RGB, GL_UNSIGNED_BYTE, cube_texture_image.data());

	// Setup the instance buffer.
	entity.uniform_data.material_specular_color = glm::vec4(material.Ks, material.Ns);

	glGenBuffers(1, &entity.uniform_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, entity.uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &entity.uniform_data, GL_DYNAMIC_DRAW);
}

void Shadowmapping::Run()
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

void Shadowmapping::HandleEvents()
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

void Shadowmapping::UpdateCamera(float dt)
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

void Shadowmapping::UpdateScene(float dt)
{
	cube_angle += CUBE_ROTATION_SPEED * dt;
	cube.uniform_data.model_matrix = glm::rotate(cube_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	cube.uniform_data.normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(cube.uniform_data.model_matrix))));
}

void Shadowmapping::RenderScene()
{
	// Render the scene depth to the shadow maps.
	RenderDepth();

	// Clear the back buffer and start rendering the actual scene.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update the frame uniform buffer.
	uniform_data_frame.view_matrix = camera.GetView();
	uniform_data_frame.projection_matrix = camera.GetProjection();
	uniform_data_frame.camera_position_W = glm::vec4(camera.GetPosition(), 1.0f);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_FRAME, uniform_buffer_frame);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerFrame), &uniform_data_frame, GL_DYNAMIC_DRAW);

	// Draw the entities.
	glUseProgram(mesh_program);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE);
	glBindSampler(TEXTURE_UNIT_DIFFUSE, diffuse_sampler);

	// Draw the cube.
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, cube.uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &cube.uniform_data, GL_DYNAMIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, cube.texture);
	glBindVertexArray(cube.vao);
	glDrawArrays(GL_TRIANGLES, 0, cube.vertex_count);

	// Draw the plane.
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, plane.uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &plane.uniform_data, GL_DYNAMIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, plane.texture);
	glBindVertexArray(plane.vao);
	glDrawArrays(GL_TRIANGLES, 0, plane.vertex_count);

	// Swap the back and front buffers.
	SDL_GL_SwapWindow(window);
}

void Shadowmapping::RenderDepth()
{
	// Render the scene depth for each shadow map.
	for (int i = 0; i < uniform_data_constant.spot_light_count; ++i)
	{
		// Bind the framebuffer.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowmap_fbo);
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmap_texture_array, 0, i);

		if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw std::runtime_error("Failed to bind framebuffer object to shadow map texture");

		glViewport(0, 0, shadowmap_width, shadowmap_height);
		glDrawBuffer(GL_NONE);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(depth_program);

		// Render the cube. Hijack the normal matrix uniform location for the light projection view matrix.
		UniformBufferPerInstance depth_uniform_data = cube.uniform_data;
		depth_uniform_data.normal_matrix = uniform_data_constant.spot_lights[i].light_projection_view_matrix;
		glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, cube.uniform_buffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &depth_uniform_data, GL_DYNAMIC_DRAW);

		glBindVertexArray(cube.vao);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDrawArrays(GL_TRIANGLES, 0, cube.vertex_count);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// Render the plane.
		depth_uniform_data = plane.uniform_data;
		depth_uniform_data.normal_matrix = uniform_data_constant.spot_lights[i].light_projection_view_matrix;
		glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, plane.uniform_buffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &depth_uniform_data, GL_DYNAMIC_DRAW);

		glBindVertexArray(plane.vao);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDrawArrays(GL_TRIANGLES, 0, plane.vertex_count);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// Restore the viewport and render to the backbuffer again.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, viewport_width, viewport_height);
		glDrawBuffer(GL_BACK);
	}
}

void Shadowmapping::UpdateShadowmapResources(int resolution_index, int spot_light_count)
{
	// Update the new attributes.
	shadowmap_width = SHADOWMAP_WIDTHS[resolution_index];
	shadowmap_height = SHADOWMAP_HEIGHTS[resolution_index];
	uniform_data_constant.spot_light_count = spot_light_count;
	shadowmap_resolution_index = resolution_index;

	// Calculate the spot lights' projection matrix.
	Frustum frustum = Frustum(SHADOWMAP_NEAR, SHADOWMAP_FAR, 2.0f * SPOT_LIGHT_ANGLE, (float)shadowmap_width, (float)shadowmap_height);
	glm::mat4 projection = frustum.GetPerspectiveProjection();

	// Put the spot lights in a circle, facing the origin.
	float radius = 10.0f;
	float delta_angle = glm::radians(360.0f / spot_light_count);
	float current_angle = 0.0f;
	for (int i = 0; i < spot_light_count; ++i)
	{
		glm::vec3 position = glm::vec3(radius * std::cos(current_angle), 1.0f, radius * std::sin(current_angle));
		glm::vec3 facing = glm::normalize(-position);

		// Calculate the spot light's view matrix.
		// TODO: Check whether x has to be 0.1f.
		glm::mat4 view = glm::lookAt(glm::vec3(position),
			glm::vec3(position + facing),
			glm::vec3(0.1f, 1.0f, 0.0f));

		uniform_data_constant.spot_lights[i].position_W = glm::vec4(position, 1.0f);
		uniform_data_constant.spot_lights[i].direction_W = glm::vec4(facing, 0.0f);
		uniform_data_constant.spot_lights[i].intensity = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
		uniform_data_constant.spot_lights[i].cutoff = 25.0f;
		uniform_data_constant.spot_lights[i].angle = SPOT_LIGHT_ANGLE;
		uniform_data_constant.spot_lights[i].light_projection_view_matrix = projection * view;

		current_angle += delta_angle;
	}

	// Generate the shadowmap texture array.
	glDeleteTextures(1, &shadowmap_texture_array);
	glGenTextures(1, &shadowmap_texture_array);
	glBindTexture(GL_TEXTURE_2D_ARRAY, shadowmap_texture_array);

	//glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, shadowmap_width, shadowmap_height, uniform_data_constant.spot_light_count, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	
	//float* data = new float[shadowmap_width * shadowmap_height * uniform_data_constant.spot_light_count];
	//memset(data, 0, shadowmap_width * shadowmap_height * uniform_data_constant.spot_light_count);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, shadowmap_width, shadowmap_height, uniform_data_constant.spot_light_count);
	//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, shadowmap_width, shadowmap_height, uniform_data_constant.spot_light_count, GL_DEPTH_COMPONENT, GL_FLOAT, data);
	//delete[] data;
}
