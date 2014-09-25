#include "../include/common/text.h"
#include "../include/common/shader.h"
#include <glm/glm.hpp>

TextRenderer::TextRenderer()
{
	glGenTextures(1, &glyphTexture);
	glGenSamplers(1, &glyphSampler);
	glSamplerParameteri(glyphSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(glyphSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(glyphSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(glyphSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	vshader = CompileShaderFromFile("../../assets/shaders/text.vs", GL_VERTEX_SHADER);
	fshader = CompileShaderFromFile("../../assets/shaders/text.fs", GL_FRAGMENT_SHADER);
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	LinkProgram(program);
	textureUniform = glGetUniformLocation(program, "texture");
	colorUniform = glGetUniformLocation(program, "color");

	glGenBuffers(1, &glyphVBO);
	glGenVertexArrays(1, &glyphVAO);
	glBindBuffer(GL_ARRAY_BUFFER, glyphVBO);
	glBindVertexArray(glyphVAO);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

TextRenderer::~TextRenderer()
{
	if (program != 0)
	{
		glDetachShader(program, vshader);
		glDetachShader(program, fshader);
		glDeleteProgram(program);
	}

	if (vshader != 0)
		glDeleteShader(vshader);
	if (fshader != 0)
		glDeleteShader(fshader);
}

void TextRenderer::RenderText(FT_Face face, const char* text, float x, float y, float sx, float sy)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(1, glyphTexture);
	glBindSampler(1, glyphSampler);
	glUniform1i(textureUniform, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glUniform4fv(colorUniform, 1, (const GLfloat*) &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	glBindBuffer(GL_ARRAY_BUFFER, glyphVBO);
	glBindVertexArray(glyphVAO);

	// The following code is taken from: http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
	for(const char* p = text; *p; p++) 
	{
		FT_Error fterror = FT_Load_Char(face, *p, FT_LOAD_RENDER);
		if (fterror != 0)
			continue;
 
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_ALPHA,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_ALPHA,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
 
		float x2 = x + face->glyph->bitmap_left * sx;
		float y2 = -y - face->glyph->bitmap_top * sy;
		float w = face->glyph->bitmap.width * sx;
		float h = face->glyph->bitmap.rows * sy;
 
		GLfloat box[4][4] = {
			{x2,     -y2    , 0, 0},
			{x2 + w, -y2    , 1, 0},
			{x2,     -y2 - h, 0, 1},
			{x2 + w, -y2 - h, 1, 1},
		};
 
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
 
		x += (face->glyph->advance.x >> 6) * sx;
		y += (face->glyph->advance.y >> 6) * sy;
	}
}