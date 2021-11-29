#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
struct DrawOutlineProgram {
	DrawOutlineProgram();
	~DrawOutlineProgram();

	GLuint program = 0;
	
	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
};

extern Load< DrawOutlineProgram > draw_outline_program;

//For convenient scene-graph setup, copy this object:
// NOTE: by default, has texture bound to 1-pixel white texture -- so it's okay to use with vertex-color-only meshes.
extern Scene::Drawable::Pipeline draw_outline_program_pipeline;
