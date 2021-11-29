#include "DrawOutlineProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Scene::Drawable::Pipeline draw_outline_program_pipeline;

Load< DrawOutlineProgram > draw_outline_program(LoadTagEarly, []() -> DrawOutlineProgram const * {
	DrawOutlineProgram* ret = new DrawOutlineProgram();

	//----- build the pipeline template -----
	draw_outline_program_pipeline.program = ret->program;

	//make a 1-pixel white texture to bind by default:
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	std::vector< glm::u8vec4 > tex_data(1, glm::u8vec4(0xff));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	draw_outline_program_pipeline.textures[0].texture = tex;
	draw_outline_program_pipeline.textures[0].target = GL_TEXTURE_2D;

	return ret;
});


DrawOutlineProgram::DrawOutlineProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"layout (location = 0) in vec3 aPosition;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"out vec2 texCoord;\n"
		"void main() {\n"
		"	texCoord = aTexCoord;\n"
		"	gl_Position = vec4(aPosition,1.0);\n"
		"}\n"
		,
		"#version 330\n"
		"uniform sampler2D TEX;\n"
		"in vec2 texCoord;\n"
		"out float fragColor;\n"
		"void main() {\n"
		"	fragColor = 1.0;\n"
		"	bool nearLine = false;"
		"	for(int i = -2; i <= 2 && !nearLine; i++){\n"
		"		for(int j = -2; j <= 2 && !nearLine; j++){\n"
		"			if(sqrt(i*i + j*j) <= 2){\n"
		"				if(texelFetch(TEX, ivec2(gl_FragCoord.x + i,gl_FragCoord.y + j),0).r == 0.0) nearLine = true;\n"
		"			}\n"
		"		}\n"
		"	}\n"
		"	if(nearLine) fragColor = 0.0;\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

DrawOutlineProgram::~DrawOutlineProgram() {
	glDeleteProgram(program);
	program = 0;
}

