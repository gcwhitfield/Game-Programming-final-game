#include "LitColorTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Scene::Drawable::Pipeline lit_color_texture_program_pipeline;

Load< LitColorTextureProgram > lit_color_texture_program(LoadTagEarly, []() -> LitColorTextureProgram const * {
	LitColorTextureProgram *ret = new LitColorTextureProgram();

	//----- build the pipeline template -----
	lit_color_texture_program_pipeline.program = ret->program;

	lit_color_texture_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	lit_color_texture_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	lit_color_texture_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;

	lit_color_texture_program_pipeline.LIGHT_COUNT_uint = ret->LIGHT_COUNT_uint;
	lit_color_texture_program_pipeline.LIGHT_COUNT_float = ret->LIGHT_COUNT_float;
	
	lit_color_texture_program_pipeline.LIGHT_TYPE_int_array = ret->LIGHT_TYPE_int_array;
	lit_color_texture_program_pipeline.LIGHT_LOCATION_vec3_array = ret->LIGHT_LOCATION_vec3_array;
	lit_color_texture_program_pipeline.LIGHT_DIRECTION_vec3_array = ret->LIGHT_DIRECTION_vec3_array;
	lit_color_texture_program_pipeline.LIGHT_ENERGY_vec3_array = ret->LIGHT_ENERGY_vec3_array;
	lit_color_texture_program_pipeline.LIGHT_CUTOFF_float_array = ret->LIGHT_CUTOFF_float_array;

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


	lit_color_texture_program_pipeline.textures[0].texture = tex;
	lit_color_texture_program_pipeline.textures[0].target = GL_TEXTURE_2D;

	return ret;
});

//https://www.lighthouse3d.com/tutorials/glsl-12-tutorial/toon-shader-version-ii/ used as starting point for cel shading code

LitColorTextureProgram::LitColorTextureProgram() {
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
		"out vec3 position;\n"
		"out vec3 normal;\n"
		"out vec4 color;\n"
		"out vec2 texCoord;\n"
		"void main() {\n"
		"	gl_Position = OBJECT_TO_CLIP * Position;\n"
		"	position = OBJECT_TO_LIGHT * Position;\n"
		"	normal = NORMAL_TO_LIGHT * Normal;\n"
		"	color = Color;\n"
		"	texCoord = TexCoord;\n"
		"}\n"
		,
		"#version 330\n"
		"uniform sampler2D TEX;\n"
		"uniform uint LIGHT_COUNT;\n"
		"uniform float LIGHT_COUNT_F;\n"
		"uniform int LIGHT_TYPE[" + std::to_string(maxLights) + "];\n"
		"uniform vec3 COLOR_EXPLOSION_ORIGIN;\n"
		"uniform float COLOR_EXPLOSION_T;\n"
		"uniform vec3 LIGHT_LOCATION[" + std::to_string(maxLights) + "];\n"
		"uniform vec3 LIGHT_DIRECTION[" + std::to_string(maxLights) + "];\n"
		"uniform vec3 LIGHT_ENERGY[" + std::to_string(maxLights) + "];\n"
		"uniform float LIGHT_CUTOFF[" + std::to_string(maxLights) + "];\n"
		"in vec3 position;\n"
		"in vec3 normal;\n"
		"in vec4 color;\n"
		"in vec2 texCoord;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	vec3 n = normalize(normal);\n"
		"	vec4 albedo = texture(TEX, texCoord) * color;\n"
		"   vec3 total = vec3(0.0f);\n"
		"	vec3 lightColor = vec3(0.0f);\n"	
		"	for(uint light = 0u; light < LIGHT_COUNT; ++light){ \n"
		"		int lightType = LIGHT_TYPE[light];\n"
		"		vec3 lightLocation = LIGHT_LOCATION[light];\n"
		"		vec3 lightDirection = LIGHT_DIRECTION[light];\n"
		"		vec3 lightEnergy = LIGHT_ENERGY[light];\n"
		"		float lightCutoff = LIGHT_CUTOFF[light];\n"
		"		lightColor += lightEnergy / vec3(LIGHT_COUNT_F);\n"	
		"		if (lightType == 0) { //point light \n"
		"			vec3 l = (lightLocation - position);\n"
		"			float dis2 = dot(l,l);\n"
		"			l = normalize(l);\n"
		"			float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
		"			if(dis2 > 100.0f) nl = 0.0f;\n"
		"			total += nl * lightEnergy;\n"
		"		} else if (lightType == 1) { //hemi light \n"
		"			total += (dot(n,-lightDirection) * 0.5 + 0.5) * lightEnergy;\n"
		"		} else if (lightType == 2) { //spot light \n"
		"			vec3 l = (lightLocation - position);\n"
		"			float dis2 = dot(l,l);\n"
		"			l = normalize(l);\n"
		"			float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
		"			float c = dot(l,-lightDirection);\n"
		"			nl *= smoothstep(lightCutoff,mix(lightCutoff,1.0,0.1), c);\n"
		"			if(dis2 > 200.0f) nl = 0.0f;\n"
		"			total += nl * lightEnergy;\n"
		"		} else { //(lightType == 3) //directional light \n"
		"			vec3 l = (lightLocation - position);\n"
		"			float dis2 = dot(l,l);\n"
		"			if(dis2 <= 200.0f)\n"	
		"				total += max(0.0, dot(n,-lightDirection)) * lightEnergy;\n"
		"		}\n"
		"	}\n"
		"	float intensity = length(total);\n"
		"	if(intensity > 0.9)\n"
		"		total = vec3(0.75f)*lightColor;\n"
		"	else if(intensity > 0.75)\n"
		"		total = vec3(0.7f)*lightColor;\n"
		"	else if(intensity > 0.6)\n"
		"		total = vec3(0.65f)*lightColor;\n"
		"	else if(intensity > 0.40)\n"
		"		total = vec3(0.42f)*lightColor;\n"
		"	else if(intensity > 0.05)\n"
		"		total = vec3(0.2f)*lightColor;\n"
		"	else\n"
		"		total = vec3(0.05f)*lightColor;\n"	
		"	fragColor = vec4(total*albedo.rgb, albedo.a);\n"
			// apply colorful explosion
			"if (COLOR_EXPLOSION_T < 1 && COLOR_EXPLOSION_T > 0) {\n"
		"		float explosion_thickness = 0.5;\n"
		"		float max_explosion_distance = 50;\n"
		"		float dist_to_explosion_origin = length(position - COLOR_EXPLOSION_ORIGIN);\n"
				// COLOR_EXPLOSION_T is a normalized float that is being sent in from 
				// PlayMode.cpp. The float 'dist' below is a length, which corresponds to 
				// the range of fragments that will be lit up in the color effect. 
				// In other words, if a fragment is within [dist, dist + explosion_thickness] of the 
				// COLOR_EXPLOSION_ORIGIN, then it will be lit up.
		"		float dist = max_explosion_distance * COLOR_EXPLOSION_T;\n" 
		"		if (dist_to_explosion_origin > dist && dist_to_explosion_origin < dist + explosion_thickness) fragColor *= dist_to_explosion_origin;\n"
		"	}\n"
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

	LIGHT_COUNT_uint = glGetUniformLocation(program, "LIGHT_COUNT");
	LIGHT_COUNT_float = glGetUniformLocation(program, "LIGHT_COUNT_F");

	LIGHT_TYPE_int_array = glGetUniformLocation(program, "LIGHT_TYPE");
	LIGHT_LOCATION_vec3_array = glGetUniformLocation(program, "LIGHT_LOCATION");
	LIGHT_DIRECTION_vec3_array = glGetUniformLocation(program, "LIGHT_DIRECTION");
	LIGHT_ENERGY_vec3_array = glGetUniformLocation(program, "LIGHT_ENERGY");
	LIGHT_CUTOFF_float_array = glGetUniformLocation(program, "LIGHT_CUTOFF");

	GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");

	COLOR_EXPLOSION_ORIGIN_vec3 = glGetUniformLocation(program, "COLOR_EXPLOSION_ORIGIN");
	COLOR_EXPLOSION_T_float = glGetUniformLocation(program, "COLOR_EXPLOSION_T");

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

LitColorTextureProgram::~LitColorTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

