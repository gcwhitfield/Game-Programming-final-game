#pragma once
//internal lib
#include "ColorTextureProgram.hpp"
//freetype
#include <ft2build.h>
#include FT_FREETYPE_H
//harfbuzz
#include <hb.h>
#include <hb-ft.h>
//opengl
#include "GL.hpp"
#include <glm/glm.hpp>

#include <math.h>
//c++ stl
#include <string>
#include <map>
#include <vector>

struct Glyph
{
    GLuint id;
    glm::ivec2 dim;
    glm::ivec2 pos;
};

typedef unsigned char tchar_t;

class Textgenerator
{
public:
    Textgenerator();
    ~Textgenerator();
    void cleanup();
    void draw_glyph(hb_codepoint_t glyphid, hb_position_t x, hb_position_t y);
    void load_font(std::string filename);
    void reshape(std::string text, glm::vec2 pos, glm::vec3 color, double line);
    void println(std::string &line, glm::vec2 pos, double line_num = 0.0f, glm::vec3 color = glm::vec3(1,1,1));
    struct Character
    {
        const int texture;
        const int width;
        const int height;
        const double line;
        const float start_x;
        const float start_y;
        const float bearing_x;
        const float bearing_y;
        const float x_offset;
        const float y_offset;
        const float x_advance;
        const float y_advance;
        const float red;
        const float green;
        const float blue;
    };

    std::vector<Character> characters;
    std::map<tchar_t, Glyph> Glyph_dict;
    int font_size = 10;
    int line_char = 100;
    GLuint VAO, VBO;

private:
    hb_glyph_info_t *glyph_info;
    hb_glyph_position_t *glyph_pos;
    hb_buffer_t *buf;
    hb_font_t *font;
    hb_face_t *face;
    hb_blob_t *blob;

    FT_Library ft_library;
    FT_Face ft_face;
};