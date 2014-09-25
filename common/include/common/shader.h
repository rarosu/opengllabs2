#pragma once

#include <GL/glew.h>

GLuint CompileShaderFromFile(const char* filepath, GLenum shaderType);
GLuint CompileShaderFromSource(const char* source, GLenum shaderType);
GLuint LinkProgram(GLuint program);