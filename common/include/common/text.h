#pragma once

#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	/**
		Naive text rendering. One draw call per glyph. For debugging purposes only.
	*/
	void RenderText(FT_Face face, const char* text, float x, float y, float sx = 1.0f, float sy = 1.0f);
private:
	GLuint fshader;
	GLuint vshader;
	GLuint program;
	GLuint glyphVAO;
	GLuint glyphVBO;
	GLuint glyphTexture;
	GLuint glyphSampler;
	GLint textureUniform;
	GLint colorUniform;
};

