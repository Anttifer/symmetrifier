#include "GLFunctions.h"

#include <cassert>
#include <vector>
#include "stb_image_write.h"

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

	// Images are stored top-to-bottom.
	auto file_data = std::vector<unsigned char>();
	file_data.reserve(4 * width * height);
	for (auto it = image_data.end() - (4*width); it != image_data.begin(); it -= (4*width)) {
		file_data.insert(file_data.end(), it, it + (4*width));
	}
	file_data.insert(file_data.end(), image_data.begin(), image_data.begin() + (4*width));

	// TODO: Failure reporting and output without alpha.
	stbi_write_png(filename, width, height, 4, &file_data[0], 0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
}

size_t GL::internal_format_size(GLenum format)
{
	size_t size = 0;
	switch (format)
	{
		case GL_R8:
		case GL_R8UI:

		case GL_RG8:
		case GL_RG8UI:

		case GL_RGBA8:
		case GL_RGBA8UI:
			size = sizeof(GLubyte);
			break;
		case GL_R8I:
		case GL_RG8I:
		case GL_RGBA8I:
			size = sizeof(GLbyte);
			break;
		case GL_R16:
		case GL_R16UI:

		case GL_RG16:
		case GL_RG16UI:

		case GL_RGBA16:
		case GL_RGBA16UI:
			size = sizeof(GLushort);
			break;
		case GL_R16I:
		case GL_RG16I:
		case GL_RGBA16I:
			size = sizeof(GLshort);
			break;
		case GL_R32UI:
		case GL_RG32UI:
		case GL_RGB32UI:
		case GL_RGBA32UI:
			size = sizeof(GLuint);
			break;
		case GL_R32I:
		case GL_RG32I:
		case GL_RGB32I:
		case GL_RGBA32I:
			size = sizeof(GLint);
			break;
		case GL_R16F:
		case GL_RG16F:
		case GL_RGBA16F:
			size = sizeof(GLhalf);
			break;
		case GL_R32F:
		case GL_RG32F:
		case GL_RGB32F:
		case GL_RGBA32F:
			size = sizeof(GLfloat);
			break;
	}

	switch (format)
	{
		case GL_RG8:
		case GL_RG8I:
		case GL_RG8UI:

		case GL_RG16:
		case GL_RG16I:
		case GL_RG16UI:

		case GL_RG32I:
		case GL_RG32UI:

		case GL_RG16F:
		case GL_RG32F:
			size *= 2;
			break;

		case GL_RGB32I:
		case GL_RGB32UI:
		case GL_RGB32F:
			size *= 3;
			break;

		case GL_RGBA8:
		case GL_RGBA8I:
		case GL_RGBA8UI:

		case GL_RGBA16:
		case GL_RGBA16I:
		case GL_RGBA16UI:

		case GL_RGBA32I:
		case GL_RGBA32UI:

		case GL_RGBA16F:
		case GL_RGBA32F:
			size *= 4;
			break;
	}

	return size;
}
