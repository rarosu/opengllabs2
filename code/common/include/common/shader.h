#pragma once

#include <GL/gl3w.h>

GLuint CompileShaderFromFile(const char* filepath, GLenum shaderType);
GLuint CompileShaderFromSource(const char* source, GLenum shaderType);
GLuint LinkProgram(GLuint program);