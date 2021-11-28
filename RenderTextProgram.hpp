// this file was copied mostly from ColorTextureProgram.hpp

#pragma once

#include "GL.hpp"
#include "Load.hpp"

// Shader program that draws text
struct RenderTextProgram {
    RenderTextProgram();
    ~RenderTextProgram();

    GLuint program = 0;
    
    // Attribute (per-vertex variable) loxations:
    GLuint Position_vec4 = -1U;
    GLuint Color_vec4 = -1U;
    GLuint TexCoord_vec2 = -1U;
    
    // Uniform (per-invocation variable) location:
    GLuint OBJECT_TO_CLIP_mat4 = -1U;

    // Textures:
    // Texture0 - texture that is accessed by TexCoord
};

extern Load<RenderTextProgram> render_text_program;