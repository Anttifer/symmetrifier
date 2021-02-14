#include "GLObjects.h"

#include "GLFunctions.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cassert>
#include "stb_image.h"
#include <climits>

//--------------------

namespace GL
{
// Buffer
Buffer::Buffer(const Buffer& other)
{
	glBindBuffer(GL_COPY_READ_BUFFER, other.buffer_);
	glBindBuffer(GL_COPY_WRITE_BUFFER, buffer_);

	GLint size, usage;
	glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);
	glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_USAGE, &usage);

	glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, usage);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

Buffer& Buffer::operator=(const Buffer& other)
{
	if (this != &other)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, other.buffer_);
		glBindBuffer(GL_COPY_WRITE_BUFFER, buffer_);

		GLint read_size, write_size, read_usage, write_usage;
		glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &read_size);
		glGetBufferParameteriv(GL_COPY_WRITE_BUFFER, GL_BUFFER_SIZE, &write_size);
		glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_USAGE, &read_usage);
		glGetBufferParameteriv(GL_COPY_WRITE_BUFFER, GL_BUFFER_USAGE, &write_usage);

		if (read_size == write_size && read_usage == write_usage)
		{
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, read_size);
		}
		else
		{
			glBufferData(GL_COPY_WRITE_BUFFER, read_size, nullptr, read_usage);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, read_size);
		}

		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	}

	return *this;
}

// Texture
void Texture::refresh_dimensions_2D(void)
{
	GLint width, height;

	glBindTexture(GL_TEXTURE_2D, texture_);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	width_ = width;
	height_ = height;
}

Texture Texture::from_png(const char* filename, bool& successful)
{
	assert(filename != nullptr);

	[[maybe_unused]] static const bool init = []{
		stbi_set_flip_vertically_on_load(1);
		return true;
	}();
	static const unsigned char error_image[] = {
		255,   0, 255, 255,    0, 255,   0, 255,  255,   0, 255, 255,    0, 255,   0, 255,
		  0, 255,   0, 255,  255,   0, 255, 255,    0, 255,   0, 255,  255,   0, 255, 255,
		255,   0, 255, 255,    0, 255,   0, 255,  255,   0, 255, 255,    0, 255,   0, 255,
		  0, 255,   0, 255,  255,   0, 255, 255,    0, 255,   0, 255,  255,   0, 255, 255
	};

	const unsigned char* image;

	int width, height, channels;
	unsigned char* loaded_image = stbi_load(filename, &width, &height, &channels, 4);

	if (loaded_image != NULL)
	{
		successful = true;
		image = loaded_image;
	}
	else
	{
		successful = false;
		width = height = 4;
		image = error_image;
		std::cerr << "PNG loading failed for " << filename << std::endl
		          << "Error: " << stbi_failure_reason() << std::endl;
	}

	Texture texture;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	stbi_image_free(loaded_image);

	if (successful)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, old_tex);

	texture.width_ = width;
	texture.height_ = height;

	return texture;
}

Texture Texture::from_png(const char* filename)
{
	bool unused_status;
	return from_png(filename, unused_status);
}

Texture Texture::empty_2D(int width, int height)
{
	Texture texture;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, old_tex);

	texture.width_ = width;
	texture.height_ = height;

	return texture;
}

Texture Texture::empty_2D_multisample(int width, int height, int num_samples)
{
	Texture texture;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &old_tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGBA, width, height, false);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, old_tex);

	texture.width_ = width;
	texture.height_ = height;

	return texture;
}

Texture Texture::empty_2D_multisample_depth(int width, int height, int num_samples)
{
	Texture depth;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &old_tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_DEPTH_COMPONENT, width, height, false);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, old_tex);

	depth.width_ = width;
	depth.height_ = height;

	return depth;
}

Texture Texture::empty_2D_depth(int width, int height)
{
	Texture depth;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, depth);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, old_tex);

	depth.width_ = width;
	depth.height_ = height;

	return depth;
}

Texture Texture::empty_cube(int resolution)
{
	Texture texture;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &old_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
		             resolution, resolution, 0,
		             GL_RGBA, GL_FLOAT, 0);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, old_tex);

	texture.width_ = resolution;
	texture.height_ = resolution;

	return texture;
}

Texture Texture::empty_cube_depth(int resolution)
{
	Texture depth;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &old_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depth);

	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
		             resolution, resolution, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, old_tex);

	depth.width_ = resolution;
	depth.height_ = resolution;

	return depth;
}

Texture Texture::buffer_texture(const Buffer& buffer, GLenum format)
{
	Texture texture;

	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &old_tex);
	glBindTexture(GL_TEXTURE_BUFFER, texture);
	glTexBuffer(GL_TEXTURE_BUFFER, format, buffer);
	glBindTexture(GL_TEXTURE_BUFFER, old_tex);

	GLint old_arr; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_arr);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLint size;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glBindBuffer(GL_ARRAY_BUFFER, old_arr);

	texture.width_  = size / GL::internal_format_size(format);
	texture.height_ = 1;

	return texture;
}

FBO FBO::simple_C0(const Texture& color)
{
	FBO framebuffer;

	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glClearColor(0, 0, 0, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0);

	const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &draw_buffer);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
		std::cerr << "Framebuffer incomplete." << std::endl;
		throw std::runtime_error("Framebuffer incomplete.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	return framebuffer;
}

FBO FBO::simple_C0D(const Texture& color, const Texture& depth)
{
	FBO framebuffer;

	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth, 0);

	const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &draw_buffer);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
		std::cerr << "Framebuffer incomplete." << std::endl;
		throw std::runtime_error("Framebuffer incomplete.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	return framebuffer;
}

FBO FBO::multisample_C0(const Texture& color)
{
	FBO framebuffer;

	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glClearColor(0, 0, 0, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, color, 0);

	// const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;
	// glDrawBuffers(1, &draw_buffer);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
		std::cerr << "Framebuffer incomplete." << std::endl;
		throw std::runtime_error("Framebuffer incomplete.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	return framebuffer;
}

FBO FBO::multisample_C0D(const Texture& color, const Texture& depth)
{
	FBO framebuffer;

	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth, 0);

	const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &draw_buffer);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
		std::cerr << "Framebuffer incomplete." << std::endl;
		throw std::runtime_error("Framebuffer incomplete.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	return framebuffer;
}

// ShaderObject
ShaderObject::ShaderObject(GLenum shader_type) :
	shader_object_ (glCreateShader(shader_type))
{}

ShaderObject::ShaderObject(GLenum shader_type, const char* shader_source) :
	shader_object_ (glCreateShader(shader_type))
{
	glShaderSource(shader_object_, 1, &shader_source, NULL);

	glCompileShader(shader_object_);

	// Check errors.
	GLint compile_status;
	glGetShaderiv(shader_object_, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		GLint info_log_length;
		glGetShaderiv(shader_object_, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<GLchar> info_log_vector(info_log_length + 1);
		glGetShaderInfoLog(shader_object_, info_log_length, NULL, &info_log_vector[0]);

		std::string type_string;
		switch (shader_type)
		{
		case GL_COMPUTE_SHADER:
			type_string = "Compute shader "; break;
		case GL_VERTEX_SHADER:
			type_string = "Vertex shader "; break;
		case GL_TESS_CONTROL_SHADER:
			type_string = "Tessellation control shader "; break;
		case GL_TESS_EVALUATION_SHADER:
			type_string = "Tessellation evaluation shader "; break;
		case GL_GEOMETRY_SHADER:
			type_string = "Geometry shader "; break;
		case GL_FRAGMENT_SHADER:
			type_string = "Fragment shader "; break;
		default:
			type_string = "Unknown shader ";
		}

		std::cerr << type_string << "compilation failed. Info log:" << std::endl << &info_log_vector[0];
		throw std::runtime_error(type_string + "compilation failed.");
	}
}

ShaderObject ShaderObject::from_file(GLenum shader_type, const char* source_file)
{
	assert(source_file != nullptr);

	std::ifstream source_stream(source_file);
	std::stringstream source_buffer;
	source_buffer << source_stream.rdbuf();

	return ShaderObject(shader_type, source_buffer.str().c_str());
}

ShaderObject ShaderObject::vertex_passthrough(void)
{
	return ShaderObject(GL_VERTEX_SHADER,
		"#version 330 core\n"
		GL_SHADER_SOURCE(
			layout(location = 0) in vec4 aPosition;
			layout(location = 1) in vec3 aNormal;
			layout(location = 2) in vec2 aTexCoord;

			out Data {
				vec3 vNormal;
				vec2 vTexCoord;
			};

			void main() {
				gl_Position = aPosition;
				vNormal = aNormal;
				vTexCoord = aTexCoord;
			}
		)
	);
}

// ShaderProgram
ShaderProgram::ShaderProgram(const ShaderObject& vertex_shader, const ShaderObject& fragment_shader)
{
	glAttachShader(shader_program_, vertex_shader);
	glAttachShader(shader_program_, fragment_shader);

	if (!link())
	{
		std::cerr << "Shader program linking failed. Info log:" << std::endl << get_info_log();
		throw std::runtime_error("Shader program linking failed.");
	}
}

ShaderProgram::ShaderProgram(const char* vertex_source, const char* fragment_source) :
	ShaderProgram(ShaderObject(GL_VERTEX_SHADER, vertex_source),
	              ShaderObject(GL_FRAGMENT_SHADER, fragment_source))
{}

ShaderProgram::ShaderProgram(const ShaderObject& vertex_shader, const ShaderObject& geometry_shader, const ShaderObject& fragment_shader)
{
	glAttachShader(shader_program_, vertex_shader);
	glAttachShader(shader_program_, geometry_shader);
	glAttachShader(shader_program_, fragment_shader);

	if (!link())
	{
		std::cerr << "Shader program linking failed. Info log:" << std::endl << get_info_log();
		throw std::runtime_error("Shader program linking failed.");
	}
}

GLint ShaderProgram::link(void)
{
	glLinkProgram(shader_program_);

	GLint link_status;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, &link_status);

	return link_status;
}

std::string ShaderProgram::get_info_log(void)
{
	GLint info_log_length;

	glGetProgramiv(shader_program_, GL_INFO_LOG_LENGTH, &info_log_length);
	std::vector<GLchar> info_log_vector(info_log_length + 1);
	glGetProgramInfoLog(shader_program_, info_log_length, NULL, &info_log_vector[0]);

	return &info_log_vector[0];
}

ShaderProgram ShaderProgram::from_files(const char* vertex_file, const char* fragment_file)
{
	ShaderObject vertex_shader   = ShaderObject::from_file(GL_VERTEX_SHADER, vertex_file);
	ShaderObject fragment_shader = ShaderObject::from_file(GL_FRAGMENT_SHADER, fragment_file);

	return ShaderProgram(vertex_shader, fragment_shader);
}

ShaderProgram ShaderProgram::from_files(const char* vertex_file, const char* geometry_file, const char* fragment_file)
{
	ShaderObject vertex_shader   = ShaderObject::from_file(GL_VERTEX_SHADER, vertex_file);
	ShaderObject geometry_shader = ShaderObject::from_file(GL_GEOMETRY_SHADER, geometry_file);
	ShaderObject fragment_shader = ShaderObject::from_file(GL_FRAGMENT_SHADER, fragment_file);

	return ShaderProgram(vertex_shader, geometry_shader, fragment_shader);
}

ShaderProgram ShaderProgram::simple(void)
{
	return ShaderProgram::from_files("shaders/simple_vert.glsl", "shaders/simple_frag.glsl");
}
} // namespace GL
