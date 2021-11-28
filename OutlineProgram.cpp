#include "OutlineProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Scene::Drawable::Pipeline outline_program_pipeline;

Load< OutlineProgram > outline_program(LoadTagEarly, []() -> OutlineProgram const * {
	OutlineProgram* ret = new OutlineProgram();

	//----- build the pipeline template -----
	outline_program_pipeline.program = ret->program;

	outline_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	outline_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	outline_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;

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


	outline_program_pipeline.textures[0].texture = tex;
	outline_program_pipeline.textures[0].target = GL_TEXTURE_2D;

	return ret;
});


OutlineProgram::OutlineProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"uniform mat4 OBJECT_TO_CLIP;\n"
		"uniform mat4x3 OBJECT_TO_LIGHT;\n"
		"uniform mat3 NORMAL_TO_LIGHT;\n"
		"in vec4 Position;\n"
		"in vec3 Normal;\n"
		"in vec4 Color;\n"
		"in vec2 TexCoord;\n"
		"void main() {\n"
		"	gl_Position = OBJECT_TO_CLIP * Position;\n"
		"}\n"
		,
		"#version 330\n"
		"uniform sampler2D TEX;\n"
		"in vec3 position;\n"
		"in vec3 normal;\n"
		"in vec4 color;\n"
		"in vec2 texCoord;\n"
		"out float fragColor;\n"
		"void main() {\n"
		"	float depthVal = texelFetch(TEX, ivec2(gl_FragCoord.xy),0).r;\n"
		"	float depthValL = texelFetch(TEX, ivec2(gl_FragCoord.x+1,gl_FragCoord.y),0).r;\n"
		"	float depthValR = texelFetch(TEX, ivec2(gl_FragCoord.x-1,gl_FragCoord.y),0).r;\n"
		"	float depthValD = texelFetch(TEX, ivec2(gl_FragCoord.x,gl_FragCoord.y+1),0).r;\n"
		"	float depthValU = texelFetch(TEX, ivec2(gl_FragCoord.x,gl_FragCoord.y-1),0).r;\n"
		"	float maxDelta = abs(depthVal - depthValL)\n;"
		"	maxDelta = max(abs(depthVal - depthValR), maxDelta)\n;"
		"	maxDelta = max(abs(depthVal - depthValU), maxDelta)\n;"
		"	maxDelta = max(abs(depthVal - depthValD), maxDelta)\n;"
		"	fragColor = 1.0;\n"
		"	if(maxDelta > 0.00002) fragColor = 0.0;\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	//look up the locations of vertex attributes:
	Position_vec4 = glGetAttribLocation(program, "Position");
	Normal_vec3 = glGetAttribLocation(program, "Normal");
	Color_vec4 = glGetAttribLocation(program, "Color");
	TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

	//look up the locations of uniforms:
	OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
	OBJECT_TO_LIGHT_mat4x3 = glGetUniformLocation(program, "OBJECT_TO_LIGHT");
	NORMAL_TO_LIGHT_mat3 = glGetUniformLocation(program, "NORMAL_TO_LIGHT");


	GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

OutlineProgram::~OutlineProgram() {
	glDeleteProgram(program);
	program = 0;
}

