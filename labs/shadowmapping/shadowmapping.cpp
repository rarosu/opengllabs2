#define GLM_FORCE_RADIANS

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <gli/gli.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
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
const std::string MODELS_FILEPATH = ASSETS_FILEPATH + "models/";
const std::string SHADERS_FILEPATH = ASSETS_FILEPATH + "shaders/";
const std::string TEXTURES_FILEPATH = ASSETS_FILEPATH + "textures/";
const std::string CRATE_MODEL_FILE = "crate.obj";
const std::string PLANE_MODEL_FILE = "plane.obj";
const std::string VS_FILE = "mesh_textured.vert";
const std::string FS_FILE = "mesh_textured.frag";
const std::string DEPTH_VS_FILE = "depth.vert";
const std::string DEPTH_FS_FILE = "depth.frag";

const float CRATE_ANGULAR_VELOCITY = 1.0f;
const int DIRECTIONAL_LIGHT_COUNT = 1;
const int POINT_LIGHT_COUNT = 1;
const int SPOT_LIGHT_COUNT = 1;

const int UNIFORM_BINDING_CONSTANT = 0;
const int UNIFORM_BINDING_FRAME = 1;
const int UNIFORM_BINDING_INSTANCE = 2;
const int UNIFORM_BINDING_SHADOWCASTER = 3;
const int TEXTURE_UNIT_DIFFUSE = 0;
const int TEXTURE_UNIT_SHADOWMAP = 1;

const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;
const float SHADOWMAP_NEAR = 0.5f;
const float SHADOWMAP_FAR = 100.0f;

const float SPOT_LIGHT_ANGULAR_VELOCITY = 0.4f;
const float SPOT_LIGHT_RADIUS = 10.0f;
const float SPOT_LIGHT_HEIGHT = 0.0f;

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
};

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

struct ConstantBuffer
{
	AmbientLight ambientLight;
	DirectionalLight directionalLights[DIRECTIONAL_LIGHT_COUNT];
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight spotLights[SPOT_LIGHT_COUNT];
};

struct PerFrameUniformBuffer
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec4 cameraPositionW;
};

struct PerInstanceUniformBuffer
{
	glm::mat4 modelMatrix;
	glm::mat4 normalMatrix;

	// The last component is the shininess of the material.
	glm::vec4 materialSpecularColor;
};

struct PerShadowcasterUniformBuffer
{
	glm::mat4 lightProjectionViewWorldMatrix;
	glm::mat4 lightBiasMatrix;
};

struct Entity
{
	GLuint positionVBO;
	GLuint normalVBO;
	GLuint texcoordVBO;
	GLuint vao;
	GLuint vertexCount;
	GLuint texture;
	GLuint perInstanceBuffer;
	PerInstanceUniformBuffer perInstanceBufferData;
};

SDL_Window* window = nullptr;
SDL_GLContext context = nullptr;

int viewportWidth = VIEWPORT_WIDTH;
int viewportHeight = VIEWPORT_HEIGHT;
InputState previousInput;
InputState currentInput;
Entity crate;
Entity plane;
GLuint vshader = 0;
GLuint fshader = 0;
GLuint program = 0;
GLuint perFrameBuffer = 0;
GLuint constantBuffer = 0;
GLuint sampler = 0;
GLuint textureUnit = 0;
GLuint vshaderDepth = 0;
GLuint fshaderDepth = 0;
GLuint programDepth = 0;
GLuint spotShadowDepthTexture = 0;
GLuint spotShadowDepthSampler = 0;
GLuint spotShadowDepthFBO = 0;
glm::mat4 spotDepthViewMatrix;
glm::mat4 spotDepthProjectionMatrix;
glm::mat4 spotDepthBiasMatrix;
GLuint perShadowcasterBuffer = 0;
ConstantBuffer constantBufferData;
PerFrameUniformBuffer perFrameBufferData;
PerShadowcasterUniformBuffer perShadowcasterBufferData;
Camera camera;
float crateAngle = 0.0f;
float spotLightPositionAngle = 0.0f;
bool spotLightFlashlightMode = true;

void GLAPIENTRY OutputDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param);
void InitializeContext();
void InitializeScene();
bool HandleEvents();
void CleanupScene();
void HandleCamera(float dt);
void UpdateSpotLight(float dt);

int main(int argc, char* argv[])
{
	std::cout << argv[0] << std::endl;

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

			perFrameBufferData.viewMatrix = camera.GetView();
			perFrameBufferData.projectionMatrix = camera.GetProjection();
			perFrameBufferData.cameraPositionW = glm::vec4(camera.GetPosition(), 1.0f);

			glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_FRAME, perFrameBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniformBuffer), &perFrameBufferData, GL_DYNAMIC_DRAW);

			// Update the crate.
			//crateAngle += dt * CRATE_ANGULAR_VELOCITY;
			crate.perInstanceBufferData.modelMatrix = glm::rotate(crateAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			crate.perInstanceBufferData.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(crate.perInstanceBufferData.modelMatrix))));

			// Update the spot light.
			UpdateSpotLight(dt);

			// Render to shadowmapping depth textures.
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, spotShadowDepthFBO);
			glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
			glDrawBuffer(GL_NONE);
			glClear(GL_DEPTH_BUFFER_BIT);
			
			glUseProgram(programDepth);
			
			perShadowcasterBufferData.lightProjectionViewWorldMatrix = spotDepthProjectionMatrix * spotDepthViewMatrix;
			perShadowcasterBufferData.lightBiasMatrix = spotDepthBiasMatrix;
			glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_SHADOWCASTER, perShadowcasterBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerShadowcasterUniformBuffer), &perShadowcasterBufferData, GL_DYNAMIC_DRAW);

			glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, crate.perInstanceBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &crate.perInstanceBufferData, GL_DYNAMIC_DRAW);

			// Render the crate depth (enable only position attributes).
			glBindVertexArray(crate.vao);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDrawArrays(GL_TRIANGLES, 0, crate.vertexCount);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			


			// Render the scene.
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glViewport(0, 0, viewportWidth, viewportHeight);
			glDrawBuffer(GL_BACK);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			
			glUseProgram(program);
			
			glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_SHADOWMAP);
			glBindSampler(TEXTURE_UNIT_SHADOWMAP, spotShadowDepthSampler);
			glBindTexture(GL_TEXTURE_2D, spotShadowDepthTexture);

			glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE);
			glBindSampler(TEXTURE_UNIT_DIFFUSE, sampler);

			// Render the crate.
			glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, crate.perInstanceBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &crate.perInstanceBufferData, GL_DYNAMIC_DRAW);
			glBindTexture(GL_TEXTURE_2D, crate.texture);
			glBindVertexArray(crate.vao);
			glDrawArrays(GL_TRIANGLES, 0, crate.vertexCount);

			// Render the plane.
			glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, plane.perInstanceBuffer);
			glBindTexture(GL_TEXTURE_2D, plane.texture);
			//glBindTexture(GL_TEXTURE_2D, spotShadowDepthTexture);
			glBindVertexArray(plane.vao);
			glDrawArrays(GL_TRIANGLES, 0, plane.vertexCount);

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

void GLAPIENTRY OutputDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	//std::cout << message << std::endl;
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
		viewportWidth, viewportHeight,
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
	glGetError(); // Clear the error buffer caused by GLEW.


	glDebugMessageCallback(OutputDebugMessage, nullptr);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, viewportWidth, viewportHeight);

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
}

void InitializeScene()
{
	// Load the crate.
	{
		OBJ model;
		if (!LoadOBJ((MODELS_FILEPATH + CRATE_MODEL_FILE).c_str(), model))
			throw std::runtime_error(std::string("Failed to load model: ") + MODELS_FILEPATH + CRATE_MODEL_FILE);

		crate.vertexCount = (GLuint)model.positions.size();

		glGenVertexArrays(1, &crate.vao);
		glBindVertexArray(crate.vao);

		glGenBuffers(1, &crate.positionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, crate.positionVBO);
		glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(glm::vec3), &model.positions[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &crate.normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, crate.normalVBO);
		glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &crate.texcoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, crate.texcoordVBO);
		glBufferData(GL_ARRAY_BUFFER, model.texcoords.size() * sizeof(glm::vec2), &model.texcoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		MTL material;
		if (!LoadMTL((MODELS_FILEPATH + model.mtllib).c_str(), material))
			throw std::runtime_error(std::string("Failed to load material: ") + model.mtllib);

		gli::storage textureImage = gli::load_dds((TEXTURES_FILEPATH + material.map_Kd).c_str());
		glGenTextures(1, &crate.texture);
		glBindTexture(GL_TEXTURE_2D, crate.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, textureImage.dimensions(0).x, textureImage.dimensions(0).y, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage.data());
	
		crate.perInstanceBufferData.modelMatrix = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
		crate.perInstanceBufferData.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(crate.perInstanceBufferData.modelMatrix))));
		crate.perInstanceBufferData.materialSpecularColor = glm::vec4(material.Ks, material.Ns);
	
		glGenBuffers(1, &crate.perInstanceBuffer);
	}

	// Load the plane.
	{
		OBJ model;
		if (!LoadOBJ((MODELS_FILEPATH + PLANE_MODEL_FILE).c_str(), model))
			throw std::runtime_error(std::string("Failed to load model: ") + MODELS_FILEPATH + PLANE_MODEL_FILE);

		plane.vertexCount = (GLuint)model.positions.size();

		glGenVertexArrays(1, &plane.vao);
		glBindVertexArray(plane.vao);

		glGenBuffers(1, &plane.positionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, plane.positionVBO);
		glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(glm::vec3), &model.positions[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &plane.normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, plane.normalVBO);
		glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &plane.texcoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, plane.texcoordVBO);
		glBufferData(GL_ARRAY_BUFFER, model.texcoords.size() * sizeof(glm::vec2), &model.texcoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		MTL material;
		if (!LoadMTL((MODELS_FILEPATH + model.mtllib).c_str(), material))
			throw std::runtime_error(std::string("Failed to load material: ") + model.mtllib);

		gli::storage textureImage = gli::load_dds((TEXTURES_FILEPATH + material.map_Kd).c_str());
		glGenTextures(1, &plane.texture);
		glBindTexture(GL_TEXTURE_2D, plane.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, textureImage.dimensions(0).x, textureImage.dimensions(0).y, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage.data());

		plane.perInstanceBufferData.modelMatrix = glm::translate(glm::vec3(0.0f, -4.0f, 0.0f)) * glm::scale(glm::vec3(5.0f));
		plane.perInstanceBufferData.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(plane.perInstanceBufferData.modelMatrix))));
		plane.perInstanceBufferData.materialSpecularColor = glm::vec4(material.Ks, material.Ns);

		glGenBuffers(1, &plane.perInstanceBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, plane.perInstanceBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &plane.perInstanceBufferData, GL_STATIC_DRAW);
	}
	

	// Compile the program.
	vshader = CompileShaderFromFile((SHADERS_FILEPATH + VS_FILE).c_str(), GL_VERTEX_SHADER);
	fshader = CompileShaderFromFile((SHADERS_FILEPATH + FS_FILE).c_str(), GL_FRAGMENT_SHADER);
	
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	LinkProgram(program);

	// Generate a texture sampler object.
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Setup the camera starting attributes.
	camera.SetProjection(Camera::GetPerspectiveProjection(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)VIEWPORT_WIDTH, (float)VIEWPORT_HEIGHT));
	camera.SetPosition(glm::vec3(0, 5.0f, 5.0f));
	camera.SetFacing(glm::vec3(0, 0, -1.0f));
	camera.RecalculateMatrices();

	// Create and initialize the perFrame and constant uniform buffers.
	glGenBuffers(1, &perFrameBuffer);
	glGenBuffers(1, &constantBuffer);

	perFrameBufferData.viewMatrix = camera.GetView();
	perFrameBufferData.projectionMatrix = camera.GetProjection();
	perFrameBufferData.cameraPositionW = glm::vec4(camera.GetPosition(), 1.0f);

	constantBufferData.ambientLight.intensity = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	constantBufferData.directionalLights[0].directionW = glm::normalize(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
	constantBufferData.directionalLights[0].intensity = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
	constantBufferData.pointLights[0].positionW = glm::vec4(0.0f, 5.0f, 5.0f, 1.0f);
	constantBufferData.pointLights[0].intensity = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
	constantBufferData.pointLights[0].cutoff = 15.0f;
	constantBufferData.spotLights[0].angle = glm::radians(40.0f);
	constantBufferData.spotLights[0].intensity = glm::vec4(0.7f, 0.0f, 0.0f, 1.0f);
	constantBufferData.spotLights[0].cutoff = 50.0f;
	

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_CONSTANT, constantBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), &constantBufferData, GL_DYNAMIC_DRAW);



	// Setup shadowmapping resources.
	vshaderDepth = CompileShaderFromFile((SHADERS_FILEPATH + DEPTH_VS_FILE).c_str(), GL_VERTEX_SHADER);
	fshaderDepth = CompileShaderFromFile((SHADERS_FILEPATH + DEPTH_FS_FILE).c_str(), GL_FRAGMENT_SHADER);

	programDepth = glCreateProgram();
	glAttachShader(programDepth, vshaderDepth);
	glAttachShader(programDepth, fshaderDepth);
	LinkProgram(programDepth);

	glGenTextures(1, &spotShadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, spotShadowDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glGenSamplers(1, &spotShadowDepthSampler);
	glSamplerParameteri(spotShadowDepthSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(spotShadowDepthSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(spotShadowDepthSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(spotShadowDepthSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &spotShadowDepthFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, spotShadowDepthFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, spotShadowDepthTexture, 0);

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Failed to create spot light shadow depth FBO");

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Setup starting spot light values.
	UpdateSpotLight(0.0f);
	
	spotDepthProjectionMatrix = Camera::GetPerspectiveProjection(SHADOWMAP_NEAR, SHADOWMAP_FAR, 2.0f * constantBufferData.spotLights[0].angle, (float)SHADOWMAP_WIDTH, (float)SHADOWMAP_HEIGHT);

	spotDepthBiasMatrix = glm::mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	glGenBuffers(1, &perShadowcasterBuffer);


}

void CleanupScene()
{
	glDeleteFramebuffers(1, &spotShadowDepthFBO);
	glDeleteSamplers(1, &spotShadowDepthSampler);
	glDeleteTextures(1, &spotShadowDepthTexture);
	glDeleteSamplers(1, &sampler);
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);
	glDeleteBuffers(1, &constantBuffer);
	glDeleteBuffers(1, &perFrameBuffer);

	glDeleteBuffers(1, &crate.positionVBO);
	glDeleteBuffers(1, &crate.normalVBO);
	glDeleteBuffers(1, &crate.texcoordVBO);
	glDeleteBuffers(1, &crate.perInstanceBuffer);
	glDeleteVertexArrays(1, &crate.vao);
	glDeleteTextures(1, &crate.texture);

	glDeleteBuffers(1, &plane.positionVBO);
	glDeleteBuffers(1, &plane.normalVBO);
	glDeleteBuffers(1, &plane.texcoordVBO);
	glDeleteBuffers(1, &plane.perInstanceBuffer);
	glDeleteVertexArrays(1, &plane.vao);
	glDeleteTextures(1, &plane.texture);
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
						viewportWidth = e.window.data1;
						viewportHeight = e.window.data2;
						glViewport(0, 0, viewportWidth, viewportHeight);
						camera.SetProjection(Camera::GetPerspectiveProjection(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)viewportWidth, (float)viewportHeight));

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

		yaw -= dx * sensitivity;
		pitch += dy * sensitivity;
		pitch = glm::clamp(pitch, 0.01f,  3.13f);

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

void UpdateSpotLight(float dt)
{
	if (currentInput.keys[SDL_SCANCODE_F] && !previousInput.keys[SDL_SCANCODE_F])
		spotLightFlashlightMode = !spotLightFlashlightMode;

	if (spotLightFlashlightMode)
	{
		constantBufferData.spotLights[0].positionW = glm::vec4(camera.GetPosition(), 1.0f);
		constantBufferData.spotLights[0].directionW = glm::vec4(camera.GetFacing(), 1.0f);

		spotDepthViewMatrix = glm::lookAt(glm::vec3(constantBufferData.spotLights[0].positionW),
										  glm::vec3(constantBufferData.spotLights[0].positionW + constantBufferData.spotLights[0].directionW),
										  glm::vec3(0.1f, 1.0f, 0.0f));
	
		// Not so constant anymore...
		glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_CONSTANT, constantBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ConstantBuffer), &constantBufferData, GL_DYNAMIC_DRAW);
	}
}