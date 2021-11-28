#include "DrawText.hpp"

// The vertex class was copied from the NEST framework
// draw functions will work on vectors of vertices, defined as follows:
struct Vertex {
    Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
        Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
    glm::vec3 Position;
    glm::u8vec4 Color;
    glm::vec2 TexCoord;
};

// inline helper functions for drawing shapes. The triangles are being counter clockwise.
// draw_rectangle copied from NEST framework
inline void draw_rectangle (std::vector<Vertex> &verts, glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color) {
    verts.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));
    verts.emplace_back(glm::vec3(center.x+radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 1.0f));
    verts.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));

    verts.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));
    verts.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));
    verts.emplace_back(glm::vec3(center.x-radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 0.0f));
};

DrawText::DrawText(std::string font_file_name) {
    // set up the vertex buffer, vertex attribute array
    {
        glGenBuffers(1, &vertex_buffer);
        GL_ERRORS();

        // ask OpenGl to fill vertex_buffer_for_render_text_program with the name of
        // an unused vertex arrya object
        glGenVertexArrays(1, &vertex_buffer_for_render_text_program);

        // set vertex_buffer_for_render_text_program as the current vertex array object:
        glBindVertexArray(vertex_buffer_for_render_text_program);

        // set vertex_buffer as the source of glVertexAttribPointer commands:
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        // set up the vertex array object to describe arrays of Vertices
        glVertexAttribPointer(
            render_text_program.Position_vec4, // attribute
            3, // size
            GL_FLOAT, // type 
            GL_FALSE, // normalized
            sizeof(Vertex), // stride
            (GLbyte *)0 + 0 // offset
        );
        glEnableVertexAttribArray(render_text_program.Position_vec4);
        glVertexAttribPointer(
            render_text_program.Color_vec4, // attribute
            4,  // size
            GL_UNSIGNED_BYTE, // type
            GL_TRUE, // normalized
            sizeof(Vertex), // stride
            (GLbyte *)0 + 4*3 // offset
        );
        glEnableVertexAttribArray(render_text_program.Color_vec4);
        glVertexAttribPointer(
            render_text_program.TexCoord_vec2, // attribute
            2, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized
            sizeof(Vertex), // stride
            (GLbyte *)0 + 4*3 + 4*1 // offset
        );
        glEnableVertexAttribArray(render_text_program.TexCoord_vec2);
        // done referring to vertex_attributes_for_render_text_program, so 
        // unbind it
        glBindVertexArray(0);
        GL_ERRORS();
    }

    // make map of glyph textures using FreeType
    {
        // 1) Load font with freetype
        // copied from https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "Could not init FreeType Library" << std::endl;
            throw;
        }
        if(FT_New_Face(ft, font_file_name.c_str(), 0, &face)) {
            std::cerr << "Failed to load font" << std::endl;
            throw;
        }
        // disable aligned since what we want read from the face (font) is grey-scale
        // the next line line was copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
        FT_Set_Pixel_Sizes(face, 0, 48);
        // 2) characters with FreeType
        char LETTER_MIN = 32;
        char LETTER_MAX  = 127;
        for (char c = LETTER_MIN; c < LETTER_MAX; c++) {
            if (FT_Load_Glyph(face, c, FT_LOAD_RENDER)) {
                std::cerr << "Failed to load glyph" << std::endl;
                throw;
            }
            // 3) create a texture from the glyph
            GLuint newTex = 0;
            glGenTextures(1, &newTex);
            glBindTexture(GL_TEXTURE_2D, newTex);
            glm::uvec2 size = glm::uvec2(face->glyph->bitmap.rows, face->glyph->bitmap.width);
            std::vector<glm::u8vec4> data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
            for (size_t y = 0; y < size.y; y++) {
                for (size_t x = 0; x < size.x; x++) {
                    size_t index = y * size.y + x;
                    uint8_t val = face->glyph->bitmap.buffer[x * std::abs(face->glyph->bitmap.pitch) + y]; // copied from professor mccan's example code for printing bitmap buffer
                    data[index].x = val;
                    data[index].y = val;
                    data[index].z = val;
                    data[index].w = val;
                }
            }
            glTexImage2D(
                GL_TEXTURE_2D, 
                0, 
                GL_RGBA, 
                size.y, 
                size.x, 
                0, 
                GL_RGBA, 
                GL_UNSIGNED_BYTE, 
                data.data()
            );
            Character newChar = {
                newTex, 
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), // line copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), // line copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
            };
            characters.insert(std::make_pair(c, newChar));
            //since texture uses a mipmap and we haven't uploaded one, instruct
            // OpenGl to make one for us:
            glGenerateMipmap(GL_TEXTURE_2D);
            // set filtering and wrapping paramters:
            // (its a bit silly to mipmap a 1x1 texture, but I'm going it because you 
            // may want to use this code to load different sizes of texture)
            // parameters copied form https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, 0);

            GL_ERRORS();
        }
    }
}

DrawText::~DrawText() {
    // ----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_render_text_program);
	vertex_buffer_for_render_text_program = 0;

}

void DrawText::draw_text(glm::vec2 drawable_size, std::string text, glm::vec3 text_pos) {
    // shape the text using Harfbuzz
    {
        hb_font = hb_ft_font_create(face, NULL);
        hb_buffer = hb_buffer_create();
        if (!hb_buffer_allocation_successful(hb_buffer)) {
            std::cerr << "hb_buffer allocation unsuccessful" << std::endl;
            throw;
        }
        hb_buffer_add_utf8(hb_buffer, text.c_str(), -1, 0, -1);
        hb_buffer_guess_segment_properties(hb_buffer);
        hb_shape(hb_font, hb_buffer, NULL, 0);
        info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
        pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);
    }
    // draw the text with OpenGl
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(render_text_program.program);
        glm::mat4 projection = glm::ortho(0, 500, 0, 500);
        glUniformMatrix4fv(render_text_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(projection));
        // use the mapping vertex_buffer_for_render_text_program to fetch vertex data
        glBindVertexArray(vertex_buffer_for_render_text_program);

        std::vector<Vertex> vertices;
        vertices.clear();
        { // copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
            uint16_t i = 0;
            float x = drawable_size.x / 2.0f;
            float y = drawable_size.y / 2.0f;
            // loop over all of the characters in the text and draw them with 
            // RenderTextProgram
            for (char c : text) {
                // get the Harfbuzz shaping information
                float x_offset = pos[i].x_offset / 64.0f;
                float y_offset = pos[i].y_offset / 64.0f;
                float x_advance = pos[i].x_advance / 64.0f;
                float y_advance = pos[i].y_advance / 64.0f;

                // draw the character
                Character ch = characters[c];
                glm::vec2 scale(10, 10);
                draw_rectangle(vertices, glm::ivec2(x + x_offset + text_pos.x, y + y_offset + text_pos.y), scale, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
                // render the glyoh texture over a quad
                glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_DYNAMIC_DRAW);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
                // advance to next graph, using the harfbuzz shaping info
                x += x_advance * scale.x;
                y += y_advance * scale.y;
                i++;
            }
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        GL_ERRORS();
    }
}