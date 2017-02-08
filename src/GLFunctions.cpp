#include "GLFunctions.h"

#include <cassert>
#include <vector>
#include <lodepng.h>

//--------------------

void GL::tex_to_png(const GL::Texture& texture, const char* filename) {
	assert(filename != nullptr);

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width = 0, height = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	auto image_data = std::vector<unsigned char>(width * height * 4);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image_data[0]);

	// Lode's system is upside down.
	auto lode_data = std::vector<unsigned char>();
	lode_data.reserve(width * height * 4);
	for (auto it = image_data.end() - (width*4); it != image_data.begin(); it -= (width*4)) {
		lode_data.insert(lode_data.end(), it, it + (width*4));
	}
	lode_data.insert(lode_data.end(), image_data.begin(), image_data.begin() + (width*4));

	auto image = std::vector<unsigned char>();
	lodepng::encode(image, lode_data, width, height);
	lodepng::save_file(image, filename);

	glBindTexture(GL_TEXTURE_2D, old_tex);
}
