#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include "GL.hpp"
#include "gl_errors.hpp"
//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include "RenderTextProgram.hpp"

struct DrawText {
    DrawText(std::string font_file_name);
    ~DrawText();

    void draw_text(glm::vec2 drawable_size, std::string text, glm::vec2 text_pos, glm::u8vec4 color);

    RenderTextProgram render_text_program;
    GLuint vertex_buffer = 0;
    GLuint vertex_buffer_for_render_text_program = 0;

    // the Character struct and the characters map were inspired by the 
	// OpenGL documentation about text rendering: 
	// https://learnopengl.com/In-Practice/Text-Rendering
	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	};

	std::map<char, Character> characters;

	FT_Face face;
	hb_buffer_t *hb_buffer;
	hb_font_t *hb_font;
	hb_glyph_position_t *pos;
	hb_glyph_info_t *info;
};