#include "DepthTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Scene::Drawable::Pipeline depth_texture_program_pipeline;

Load< DepthTextureProgram > depth_texture_program(LoadTagEarly, []() -> DepthTextureProgram const * {
	DepthTextureProgram* ret = new LitColorTextureProgram();

	//----- build the pipeline template -----
	depth_texture_program_pipeline.program = ret->program;

	depth_texture_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	depth_texture_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	depth_texture_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;
	
	std::cout << "depth mats " << depth_texture_program_pipeline.OBJECT_TO_CLIP_mat4 << " " << depth_texture_program_pipeline.OBJECT_TO_LIGHT_mat4x3 << " "
		<< depth_texture_program_pipeline.NORMAL_TO_LIGHT_mat3 << std::endl;


	return ret;
});


DepthTextureProgram::DepthTextureProgram(uint32_t width = 1920, uint32_t height = 1080) {


	GL_ERRORS();
	

	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"uniform mat4 OBJECT_TO_CLIP;\n"
		"in vec4 Position;\n"
		"in vec3 Normal;\n"
		"in vec4 Color;\n"
		"void main() {\n"
		"	gl_Position = OBJECT_TO_CLIP * Position;\n"
		"}\n"
		,
		"#version 330\n"
		"uniform sampler2D TEX;\n"
		"void main() {\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.
	GL_ERRORS();

	//look up the locations of vertex attributes:
	Position_vec4 = glGetAttribLocation(program, "Position");
	Normal_vec3 = glGetAttribLocation(program, "Normal");
	Color_vec4 = glGetAttribLocation(program, "Color");
	GL_ERRORS();

	//look up the locations of uniforms:
	OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
	GL_ERRORS();

	GL_ERRORS();

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now
	GL_ERRORS();

	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	GL_ERRORS();

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
	GL_ERRORS();
}

DepthTextureProgram::~DepthTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

