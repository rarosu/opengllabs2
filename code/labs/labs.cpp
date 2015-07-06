#define GLM_FORCE_RADIANS

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <gli/gli.hpp>
#include <ctime>
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
const std::string PLAIN_VS_FILE = "mesh_plain.vert";
const std::string PLAIN_FS_FILE = "mesh_plain.frag";

const float CRATE_SPREAD = 50;
const int CRATE_COUNT = 10;
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

const int QUADTREE_MAX_PER_NODE = 1;

const int FRUSTUM_MODE_HIDDEN = 0;
const int FRUSTUM_MODE_FIXED = 1;

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

struct PlainPerInstanceUniformBuffer
{
	glm::mat4 modelMatrix;
	glm::vec4 color;
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

class QuadTreeNode
{
public:
	QuadTreeNode(const glm::vec2& origin, float width, float height);
	~QuadTreeNode();
	void AddEntity(Entity* entity);
	bool IntersectFrustum(const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const;
	std::vector<const QuadTreeNode*> GetNodesIntersectingFrustum(const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const;
	std::vector<const QuadTreeNode*> GetNodesNotIntersectingFrustum(const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const;

	glm::vec2 origin;
	float width;
	float height;
	QuadTreeNode* children[4];
	std::vector<Entity*> entities;
private:
	QuadTreeNode(const QuadTreeNode&);
	QuadTreeNode& operator=(const QuadTreeNode&);

	void GetNodesFrustumInternal(std::vector<const QuadTreeNode*>& nodes, bool intersecting, const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const;
};

SDL_Window* window = nullptr;
SDL_GLContext context = nullptr;

int viewportWidth = VIEWPORT_WIDTH;
int viewportHeight = VIEWPORT_HEIGHT;
InputState previousInput;
InputState currentInput;
std::vector<Entity> crates;
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
Frustum cameraFrustum;
Camera camera;
float crateAngle = 0.0f;
float spotLightPositionAngle = 0.0f;
bool spotLightFlashlightMode = true;
QuadTreeNode* quadtree = nullptr;
GLuint vshaderPlain = 0;
GLuint fshaderPlain = 0;
GLuint programPlain = 0;
GLuint cubePositionVBO = 0;
GLuint cubeVAO = 0;
GLuint cubeVertexCount = 0;
glm::vec3 frustumPosition;
glm::vec3 frustumFacing;
GLuint frustumPerInstanceBuffer = 0;
PlainPerInstanceUniformBuffer frustumPerInstanceBufferData;
int frustumMode = FRUSTUM_MODE_HIDDEN;
GLuint quadNodePerInstanceBuffer = 0;
PlainPerInstanceUniformBuffer quadNodePerInstanceBufferData;

void GLAPIENTRY OutputDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param);
void InitializeContext();
void InitializeScene();
bool HandleEvents();
void CleanupScene();
void HandleCamera(float dt);
void UpdateSpotLight(float dt);
void UpdateFrustumMode();
float RandomFloat();

int main(int argc, char* argv[])
{
	std::cout << argv[0] << std::endl;

	int result = 0;
	try
	{
		srand(time(nullptr));

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

			// Update the spot light.
			UpdateSpotLight(dt);

			// Update the frustum visualization.
			UpdateFrustumMode();

			// Get the quadtree nodes intersecting / not intersecting the frustum.
			//std::vector<const QuadTreeNode*> intersectingNodes = quadtree->GetNodesIntersectingFrustum(frustumPosition, frustumFacing, cameraFrustum);
			//std::vector<const QuadTreeNode*> nonIntersectingNodes = quadtree->GetNodesNotIntersectingFrustum(frustumPosition, frustumFacing, cameraFrustum);

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

			// Render the crates' depth (enable only position attributes).
			for (int i = 0; i < crates.size(); ++i)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, crates[i].perInstanceBuffer);
				glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &crates[i].perInstanceBufferData, GL_DYNAMIC_DRAW);

				glBindVertexArray(crates[i].vao);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
				glDrawArrays(GL_TRIANGLES, 0, crates[i].vertexCount);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
			}

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

			// Render the crates.
			for (int i = 0; i < crates.size(); ++i)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, crates[i].perInstanceBuffer);
				glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &crates[i].perInstanceBufferData, GL_DYNAMIC_DRAW);
				glBindTexture(GL_TEXTURE_2D, crates[i].texture);
				glBindVertexArray(crates[i].vao);
				glDrawArrays(GL_TRIANGLES, 0, crates[i].vertexCount);
			}

			// Render the plane.
			glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, plane.perInstanceBuffer);
			glBindTexture(GL_TEXTURE_2D, plane.texture);
			//glBindTexture(GL_TEXTURE_2D, spotShadowDepthTexture);
			glBindVertexArray(plane.vao);
			glDrawArrays(GL_TRIANGLES, 0, plane.vertexCount);

			// Setup for visualizations.
			glUseProgram(programPlain);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glBindVertexArray(cubeVAO);

			if (frustumMode != FRUSTUM_MODE_HIDDEN)
			{
				// Render the frustum visualization.
				glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, frustumPerInstanceBuffer);
				glBufferData(GL_UNIFORM_BUFFER, sizeof(PlainPerInstanceUniformBuffer), &frustumPerInstanceBufferData, GL_DYNAMIC_DRAW);
				glBindVertexArray(cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
			
				// Render the quadtree leaf nodes.
				quadNodePerInstanceBufferData.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
				//for (int i = 0; i < intersectingNodes.size(); ++i)
				for (int i = 0; i < 4; ++i)
				{
					//const QuadTreeNode* node = intersectingNodes[i];
					const QuadTreeNode* node = quadtree->children[i];

					if (node->IntersectFrustum(frustumPosition, frustumFacing, cameraFrustum))
					{
						float width = node->width * 0.5f;
						float height = node->height * 0.5f;
						glm::vec3 position = glm::vec3(node->origin.x, 0.0f, node->origin.y);
						quadNodePerInstanceBufferData.modelMatrix = glm::translate(position) * glm::scale(glm::vec3(width, 1.0f, height));

						glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, quadNodePerInstanceBuffer);
						glBufferData(GL_UNIFORM_BUFFER, sizeof(PlainPerInstanceUniformBuffer), &quadNodePerInstanceBufferData, GL_DYNAMIC_DRAW);

						glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
					}
					
				}
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

	//glEnable(GL_CULL_FACE);
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
	// Load the crate model.
	{
		OBJ model;
		if (!LoadOBJ((MODELS_FILEPATH + CRATE_MODEL_FILE).c_str(), model))
			throw std::runtime_error(std::string("Failed to load model: ") + MODELS_FILEPATH + CRATE_MODEL_FILE);

		MTL material;
		if (!LoadMTL((MODELS_FILEPATH + model.mtllib).c_str(), material))
			throw std::runtime_error(std::string("Failed to load material: ") + model.mtllib);

		gli::storage textureImage = gli::load_dds((TEXTURES_FILEPATH + material.map_Kd).c_str());

		// Create a cube VBO that can be used for visualizations later.
		cubeVertexCount = (GLuint)model.positions.size();

		glGenVertexArrays(1, &cubeVAO);
		glBindVertexArray(cubeVAO);

		glGenBuffers(1, &cubePositionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO);
		glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(glm::vec3), &model.positions[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(0);

		// Create a list of crate entities and randomize their positions.
		crates.resize(CRATE_COUNT);
		for (int i = 0; i < CRATE_COUNT; ++i)
		{
			crates[i].vertexCount = (GLuint)model.positions.size();

			glGenVertexArrays(1, &crates[i].vao);
			glBindVertexArray(crates[i].vao);

			glGenBuffers(1, &crates[i].positionVBO);
			glBindBuffer(GL_ARRAY_BUFFER, crates[i].positionVBO);
			glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(glm::vec3), &model.positions[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glGenBuffers(1, &crates[i].normalVBO);
			glBindBuffer(GL_ARRAY_BUFFER, crates[i].normalVBO);
			glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glGenBuffers(1, &crates[i].texcoordVBO);
			glBindBuffer(GL_ARRAY_BUFFER, crates[i].texcoordVBO);
			glBufferData(GL_ARRAY_BUFFER, model.texcoords.size() * sizeof(glm::vec2), &model.texcoords[0], GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);

			glGenTextures(1, &crates[i].texture);
			glBindTexture(GL_TEXTURE_2D, crates[i].texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, textureImage.dimensions(0).x, textureImage.dimensions(0).y, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage.data());

			glm::vec3 spawnLocation(RandomFloat() * CRATE_SPREAD - CRATE_SPREAD * 0.5f, 1.0f, RandomFloat() * CRATE_SPREAD - CRATE_SPREAD * 0.5f);
			crates[i].perInstanceBufferData.modelMatrix = glm::translate(spawnLocation);
			crates[i].perInstanceBufferData.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(crates[i].perInstanceBufferData.modelMatrix))));
			crates[i].perInstanceBufferData.materialSpecularColor = glm::vec4(material.Ks, material.Ns);

			glGenBuffers(1, &crates[i].perInstanceBuffer);
			glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, crates[i].perInstanceBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerInstanceUniformBuffer), &crates[i].perInstanceBufferData, GL_STATIC_DRAW);
		}
		
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

		plane.perInstanceBufferData.modelMatrix = glm::translate(glm::vec3(0.0f, -4.0f, 0.0f)) * glm::scale(glm::vec3(CRATE_SPREAD));
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
	cameraFrustum = Frustum(PERSPECTIVE_NEAR, PERSPECTIVE_FAR, PERSPECTIVE_FOV, (float)VIEWPORT_WIDTH, (float)VIEWPORT_HEIGHT);
	camera.SetProjection(cameraFrustum.GetPerspectiveProjection());
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
	
	Frustum spotFrustum(SHADOWMAP_NEAR, SHADOWMAP_FAR, 2.0f * constantBufferData.spotLights[0].angle, (float)SHADOWMAP_WIDTH, (float)SHADOWMAP_HEIGHT);
	spotDepthProjectionMatrix = spotFrustum.GetPerspectiveProjection();

	spotDepthBiasMatrix = glm::mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	glGenBuffers(1, &perShadowcasterBuffer);

	// Compile the plain shaders used for visualizations.
	vshaderPlain = CompileShaderFromFile((SHADERS_FILEPATH + PLAIN_VS_FILE).c_str(), GL_VERTEX_SHADER);
	fshaderPlain = CompileShaderFromFile((SHADERS_FILEPATH + PLAIN_FS_FILE).c_str(), GL_FRAGMENT_SHADER);

	programPlain = glCreateProgram();
	glAttachShader(programPlain, vshaderPlain);
	glAttachShader(programPlain, fshaderPlain);
	LinkProgram(programPlain);


	// Setup the quadtree acceleration structure.
	quadtree = new QuadTreeNode(glm::vec2(0.0f, 0.0f), CRATE_SPREAD, CRATE_SPREAD);
	for (int i = 0; i < crates.size(); ++i)
	{
		quadtree->AddEntity(&crates[i]);
	}

	// Setup the frustum visualization.
	{
		frustumPerInstanceBufferData.modelMatrix = glm::inverse(camera.GetProjection() * camera.GetView());
		frustumPerInstanceBufferData.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

		glGenBuffers(1, &frustumPerInstanceBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, frustumPerInstanceBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PlainPerInstanceUniformBuffer), &frustumPerInstanceBufferData, GL_STATIC_DRAW);
	}

	// Setup the quadtree visualization.
	{
		glGenBuffers(1, &quadNodePerInstanceBuffer);
	}
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

	for (int i = 0; i < crates.size(); ++i)
	{
		glDeleteBuffers(1, &crates[i].positionVBO);
		glDeleteBuffers(1, &crates[i].normalVBO);
		glDeleteBuffers(1, &crates[i].texcoordVBO);
		glDeleteBuffers(1, &crates[i].perInstanceBuffer);
		glDeleteVertexArrays(1, &crates[i].vao);
		glDeleteTextures(1, &crates[i].texture);
	}

	glDeleteBuffers(1, &plane.positionVBO);
	glDeleteBuffers(1, &plane.normalVBO);
	glDeleteBuffers(1, &plane.texcoordVBO);
	glDeleteBuffers(1, &plane.perInstanceBuffer);
	glDeleteVertexArrays(1, &plane.vao);
	glDeleteTextures(1, &plane.texture);

	delete quadtree;
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
						cameraFrustum.width = static_cast<float>(viewportWidth);
						cameraFrustum.height = static_cast<float>(viewportHeight);
						camera.SetProjection(cameraFrustum.GetPerspectiveProjection());

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

void UpdateFrustumMode()
{
	int previousMode = frustumMode;
	if (currentInput.keys[SDL_SCANCODE_Q] && !previousInput.keys[SDL_SCANCODE_Q])
	{
		frustumMode++;
		if (frustumMode > FRUSTUM_MODE_FIXED)
			frustumMode = FRUSTUM_MODE_HIDDEN;
	}

	if (frustumMode != previousMode)
	{
		if (frustumMode == FRUSTUM_MODE_FIXED)
		{
			frustumPosition = camera.GetPosition();
			frustumFacing = camera.GetFacing();
			frustumPerInstanceBufferData.modelMatrix = glm::inverse(camera.GetProjection() * camera.GetView());
		}
	}
}

float RandomFloat()
{
	const int PRECISION = 10000;
	return static_cast<float>(rand() % PRECISION) / PRECISION;
}

QuadTreeNode::QuadTreeNode(const glm::vec2& origin, float width, float height)
	: origin(origin)
	, width(width)
	, height(height)
{
	for (int i = 0; i < 4; ++i)
		children[i] = nullptr;
}

QuadTreeNode::~QuadTreeNode()
{
	for (int i = 0; i < 4; ++i)
		delete children[i];
}

void QuadTreeNode::AddEntity(Entity* entity)
{
	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;
	glm::vec3 entityPosition = glm::vec3(
		entity->perInstanceBufferData.modelMatrix[3][0],
		entity->perInstanceBufferData.modelMatrix[3][1],
		entity->perInstanceBufferData.modelMatrix[3][2]);
	
	if (entityPosition.x > origin.x - halfWidth && entityPosition.x < origin.x + halfWidth &&
		entityPosition.z > origin.y - halfHeight && entityPosition.z < origin.y + halfHeight)
	{
		entities.push_back(entity);
		if (entities.size() >= QUADTREE_MAX_PER_NODE)
		{
			if (children[0] == nullptr)
			{
				float quarterWidth = halfWidth * 0.5f;
				float quarterHeight = halfHeight * 0.5f;
				glm::vec2 childOrigin = origin + glm::vec2(-quarterWidth, -quarterHeight);
				children[0] = new QuadTreeNode(childOrigin, halfWidth, halfHeight);

				childOrigin = origin + glm::vec2(quarterWidth, -quarterHeight);
				children[1] = new QuadTreeNode(childOrigin, halfWidth, halfHeight);

				childOrigin = origin + glm::vec2(-quarterWidth, quarterHeight);
				children[2] = new QuadTreeNode(childOrigin, halfWidth, halfHeight);

				childOrigin = origin + glm::vec2(quarterWidth, quarterHeight);
				children[3] = new QuadTreeNode(childOrigin, halfWidth, halfHeight);
			}

			for (int i = 0; i < 4; ++i)
			{
				children[i]->AddEntity(entity);
			}
		}
	}
}

bool QuadTreeNode::IntersectFrustum(const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const
{
	// All quadtree nodes are assumed to span the entire y-axis. An orthogonal projection of the node and the frustum
	// on the xz-plane simplifies the problem. The resulting intersection test is done using the separating axis theorem
	// as both the quad and the orthogonal projection of a frustum are convex.

	// Setup the frustum along a local z-axis.
	glm::vec3 vertices[8];
	float offsetY = std::tan(frustum.fovY * 0.5f);
	float offsetX = offsetY * (frustum.width / frustum.height);

	vertices[0] = frustum.near * glm::vec3(-offsetX, -offsetY, 1.0f);
	vertices[1] = frustum.near * glm::vec3(offsetX, -offsetY, 1.0f);
	vertices[2] = frustum.near * glm::vec3(-offsetX, offsetY, 1.0f);
	vertices[3] = frustum.near * glm::vec3(offsetX, offsetY, 1.0f);

	vertices[4] = frustum.far * glm::vec3(-offsetX, -offsetY, 1.0f);
	vertices[5] = frustum.far * glm::vec3(offsetX, -offsetY, 1.0f);
	vertices[6] = frustum.far * glm::vec3(-offsetX, offsetY, 1.0f);
	vertices[7] = frustum.far * glm::vec3(offsetX, offsetY, 1.0f);

	// Transform the vertices to the proper orientation and position.
	glm::vec3 right = glm::normalize(glm::cross(facing, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::normalize(glm::cross(right, facing));
	glm::mat3 rotation = glm::mat3(
		right.x, right.y, right.z,
		up.x, up.y, up.z,
		facing.x, facing.y, facing.z
	);

	for (int i = 0; i < 8; ++i)
	{
		vertices[i] = rotation * vertices[i] + position;
	}

	// Project the vertices orthonally on the xz-plane.
	glm::vec2 projectedVertices[8];
	for (int i = 0; i < 8; ++i)
	{
		projectedVertices[i] = glm::vec2(vertices[i].x, vertices[i].z);
	}

	// Calculate the quad vertices.
	float halfWidth = this->width * 0.5f;
	float halfHeight = this->height * 0.5f;
	glm::vec2 quadVertices[4];
	quadVertices[0] = this->origin + glm::vec2(-halfWidth, -halfHeight);
	quadVertices[1] = this->origin + glm::vec2(halfWidth, -halfHeight);
	quadVertices[2] = this->origin + glm::vec2(-halfWidth, halfHeight);
	quadVertices[3] = this->origin + glm::vec2(halfWidth, halfHeight);

	// Since the quad is axis-aligned, its normals will be used for the SAT tests.
	glm::vec2 normals[4];
	normals[0] = glm::vec2(1, 0);
	normals[1] = glm::vec2(0, 1);
	normals[2] = glm::vec2(-1, 0);
	normals[3] = glm::vec2(0, -1);

	for (int i = 0; i < 4; ++i)
	{
		float frustumMinBound = 10e10f;
		float frustumMaxBound = -10e10f;
		for (int k = 0; k < 8; ++k)
		{
			float bound = glm::dot(projectedVertices[k], normals[i]);
			frustumMinBound = std::min(frustumMinBound, bound);
			frustumMaxBound = std::max(frustumMaxBound, bound);
		}

		float quadMinBound = 10e10f;
		float quadMaxBound = -10e10f;
		for (int k = 0; k < 4; ++k)
		{
			float bound = glm::dot(quadVertices[k], normals[i]);
			quadMinBound = std::min(quadMinBound, bound);
			quadMaxBound = std::max(quadMaxBound, bound);
		}

		// Check if the projected shapes do not intersect. If a separating axis is found, return no intersection.
		if (quadMinBound > frustumMaxBound)
			return false;
		if (frustumMinBound > quadMaxBound)
			return false;
	}
	
	return true;
}

std::vector<const QuadTreeNode*> QuadTreeNode::GetNodesIntersectingFrustum(const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const
{
	std::vector<const QuadTreeNode*> nodes;
	GetNodesFrustumInternal(nodes, true, position, facing, frustum);
	return nodes;
}

std::vector<const QuadTreeNode*> QuadTreeNode::GetNodesNotIntersectingFrustum(const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const
{
	std::vector<const QuadTreeNode*> nodes;
	GetNodesFrustumInternal(nodes, false, position, facing, frustum);
	return nodes;
}

void QuadTreeNode::GetNodesFrustumInternal(std::vector<const QuadTreeNode*>& nodes, bool intersecting, const glm::vec3& position, const glm::vec3& facing, const Frustum& frustum) const
{
	if (IntersectFrustum(position, facing, frustum))
	{
		if (intersecting)
		{
			// Only add intersecting leaf nodes.
			if (this->children[0] == nullptr)
			{
				nodes.push_back(this);
			}
			else
			{
				for (int i = 0; i < 4; ++i)
				{
					this->children[i]->GetNodesFrustumInternal(nodes, intersecting, position, facing, frustum);
				}
			}
		}
		else
		{
			// Check if the children are not intersecting.
			for (int i = 0; i < 4; ++i)
			{
				if (this->children[i] != nullptr)
					this->children[i]->GetNodesFrustumInternal(nodes, intersecting, position, facing, frustum);
			}
		}
	}
	else
	{
		if (!intersecting)
		{
			nodes.push_back(this);
		}
	}

	/*
	if (IntersectFrustum(position, facing, frustum))
	{
		if (intersecting)
			nodes.push_back(this);

		for (int i = 0; i < 4; ++i)
		{
			if (this->children[i] != nullptr)
				this->children[i]->GetNodesFrustumInternal(nodes, intersecting, position, facing, frustum);
		}
		
	}
	else
	{
		if (!intersecting)
			nodes.push_back(this);
	}
	*/
}