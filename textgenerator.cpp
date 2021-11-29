#include "Textgenerator.hpp"

#include <fstream>
#include <iostream>
#define radi 64.0f
Textgenerator::Textgenerator() = default;
Textgenerator::~Textgenerator()
{
	FT_Done_Face(ft_face);
	cleanup();
}
void Textgenerator::cleanup()
{
	if (buf != nullptr)
	{
		hb_buffer_destroy(buf);
	}
	if (font != nullptr)
	{
		hb_font_destroy(font);
	}
	if (face != nullptr)
	{
		hb_face_destroy(face);
	}
	if (blob != nullptr)
	{
		hb_blob_destroy(blob);
	}
}
void Textgenerator::load_font(std::string filename)
{
	FT_Init_FreeType(&ft_library);
	auto error = FT_New_Face(ft_library, filename.c_str(), 0, &ft_face);
	if (error)
	{
		std::cout << "not able to load new face\n";
	}
	FT_Set_Char_Size(ft_face, font_size * 64, font_size * 64, 0, 0);

	//blob = hb_blob_create_from_file(filename.c_str()); /* or hb_blob_create_from_file_or_fail() */
	//face = hb_face_create(blob, 0);
	//font = hb_font_create(face);
	font = hb_ft_font_create(ft_face, NULL);

	buf = hb_buffer_create();
}
void Textgenerator::draw_glyph(hb_codepoint_t glyphid, hb_position_t x, hb_position_t y)
{
	std::cout << glyphid << ' ' << x << ' ' << y << std::endl;
}

void Textgenerator::reshape(std::string text, glm::vec2 pos, glm::vec3 color, double line) {

	hb_buffer_reset(buf);
	hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);

	hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
	hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language(buf, hb_language_from_string("en", -1));

	hb_shape(font, buf, NULL, 0);

	uint32_t glyph_count;
	glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
	glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
	//std::cout<<glyph_count<<std::endl;
	for (size_t i = 0; i < glyph_count; ++i)
	{

		auto glyphid = glyph_info[i].codepoint;
		double x_offset = glyph_pos[i].x_offset / radi;
		double y_offset = glyph_pos[i].y_offset / radi;
		double x_advance = glyph_pos[i].x_advance / radi;
		double y_advance = glyph_pos[i].y_advance / radi;

		if (FT_Load_Glyph(ft_face, glyphid, FT_LOAD_DEFAULT))
			std::cout << "Load glyph error" << std::endl;
		if (FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL))
			std::cout << "Render error" << std::endl;
		int w = ft_face->glyph->bitmap.width;
		int h = ft_face->glyph->bitmap.rows;

		float l = (float)ft_face->glyph->bitmap_left;
		float t = (float)ft_face->glyph->bitmap_top;

		//glUseProgram(color_texture_program->program);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint texture;
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			ft_face->glyph->bitmap.width,
			ft_face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			ft_face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		characters.push_back({(int)texture, w, h, line, pos.x, pos.y, l, t, (float)x_offset, (float)y_offset, (float)x_advance, (float)y_advance, color.x, color.y, color.z});
	}
}

// reference: https://stackoverflow.com/questions/25022880/c-split-string-every-x-characters
std::vector<std::string> str_split(const std::string& str, int splitLength)
{
   size_t NumSubstrings = str.length() / splitLength;
   std::vector<std::string> ret;

   for (size_t i = 0; i < NumSubstrings; i++)
   {
        ret.push_back(str.substr(i * splitLength, splitLength));
   }

   // If there are leftover characters, create a shorter item at the end.
   if (str.length() % splitLength != 0)
   {
        ret.push_back(str.substr(splitLength * NumSubstrings));
   }


   return ret;
}

void Textgenerator::println(std::string &line, glm::vec2 pos, double line_num, glm::vec3 color) {
	// gets the string and change the line when necessary;
	std::vector<std::string> lines; // vector of substrings
	// std::cout << line << '\n';
	lines = str_split(line, line_char);
	for (std::string &s : lines) {
		this->reshape(s, pos, color, line_num);
		// std::cout << s << "\n";
		line_num += 0.25;
	}
}


/*int main(int argc, char **argv)
{
	std::cout<<"fuck"<<std::endl;
	Textgenerator A;
	std::string text = "databases";
	std::string filename = "../font/times_new_roman.ttf";
	A.load_font(filename);
	A.reshape(text, glm::vec2(100,100), glm::vec3(255,0,0), 25.0);
	return 0;
}*/
