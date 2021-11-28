// this file was copied mostly from ColorTextureProgram.cpp

#include "RenderTextProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Load<RenderTextProgram> render_text_program(LoadTagEarly);

RenderTextProgram::RenderTextProgram() {
    program = gl_compile_program(
        // vertex shader
        "#version 330\n"
        "uniform mat4 OBJECT_TO_CLIP;\n"
        "in vec4 Position;\n"
        "in vec4 Color;\n"
        "in vec2 TexCoord;\n"
        "out vec4 color;\n"
        "out vec2 texCoord;\n"
        "void main() {\n"
        "   gl_Position = OBJECT_TO_CLIP * Position;\n"
        "   color = Color;\n"
        "   texCoord = TexCoord;\n"
        "}\n"
        ,
        // fragment shader
        "#version 330\n"
        "uniform sampler2D TEX;\n"
        "in vec4 color;\n"
        "in vec2 texCoord;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = texture(TEX, texCoord) * color;\n"
        "}\n"
    );
    // As you can see above, adjacent stirngs in C/C++ are concatenated. This is very 
    // useful for writing long shader programs inline/

    // look iup the locations of vertex attributes
    Position_vec4 = glGetAttribLocation(program, "Position");
    Color_vec4 = glGetAttribLocation(program, "Color");
    TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

    // look up the locations of the uniforms
    OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
    GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");

    // set TEX to always refer to the texture binding zero
    glUseProgram(program); // bind program -- glUniform calls refer to this program now
    glUniform1i(TEX_sampler2D, 0); // set TEX to sample from GL_TEXTURE0
    glUseProgram(0); // unbind program -- glUnifrom* calls refer to ??? now
}

RenderTextProgram::~RenderTextProgram() {
    glDeleteProgram(program);
    program = 0;
}