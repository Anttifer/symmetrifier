#ifndef GLOBJECTS_H
#define GLOBJECTS_H

#include <GL/glew.h>
#include <string>
#include "GLHandle.h"
#define GL_SHADER_SOURCE(CODE) #CODE

//--------------------

namespace GL
{
class Buffer {
public:
	Buffer (void)          = default;
	Buffer (Buffer&&)      = default;
	Buffer (const Buffer&);

	Buffer&  operator= (Buffer&&)      = default;
	Buffer&  operator= (const Buffer&);
	operator GLuint    (void) const    { return buffer_; }
private:
	Handle<&glGenBuffers, &glDeleteBuffers> buffer_;
};

class Texture {
public:
	operator GLuint (void) const { return texture_; }
	unsigned width  (void) const { return width_; }
	unsigned height (void) const { return height_; }

	// This should be called if any operations changing the dimensions
	// of a 2D texture have been made.
	void refresh_dimensions_2D (void);

	static Texture from_png                   (const char* filename, bool& successful);
	static Texture from_png                   (const char* filename);
	static Texture empty_2D                   (int width, int height);
	static Texture empty_2D_multisample       (int width, int height, int samples = 4);
	static Texture empty_2D_depth             (int width, int height);
	static Texture empty_2D_multisample_depth (int width, int height, int samples = 4);
	static Texture empty_cube                 (int resolution);
	static Texture empty_cube_depth           (int resolution);
	static Texture buffer_texture             (const Buffer& buffer, GLenum format);
private:
	inline static const GenFuncP glGenTexturesP    = &glGenTextures;
	inline static const DelFuncP glDeleteTexturesP = &glDeleteTextures;

	Handle<&glGenTexturesP, &glDeleteTexturesP> texture_;

	// Not really handles, but convenient to use.
	Handle<> width_, height_;
};

using VAO = Handle<&glGenVertexArrays, &glDeleteVertexArrays>;

class FBO {
public:
	operator GLuint (void) const { return fbo_; }

	static FBO simple_C0       (const Texture& color);
	static FBO simple_C0D      (const Texture& color, const Texture& depth);
	static FBO multisample_C0  (const Texture& color);
	static FBO multisample_C0D (const Texture& color, const Texture& depth);
private:
	Handle<&glGenFramebuffers, &glDeleteFramebuffers> fbo_;
};

class ShaderObject {
public:
	ShaderObject (GLenum shader_type);
	ShaderObject (GLenum shader_type, const char* shader_source);

	operator GLuint (void) const { return shader_object_; }

	static ShaderObject from_file          (GLenum shader_type, const char* filename);
	static ShaderObject vertex_passthrough (void);
private:
	static void glDeleteShaderAdapted (GLsizei, const GLuint* s) { glDeleteShader(*s); }
	inline static const DelFuncP glDeleteShaderP = &glDeleteShaderAdapted;

	Handle<nullptr, &glDeleteShaderP> shader_object_;
};

class ShaderProgram {
public:
	ShaderProgram (void) = default;
	ShaderProgram (const ShaderObject& vertex_shader, const ShaderObject& fragment_shader);
	ShaderProgram (const ShaderObject& vertex_shader, const ShaderObject& geometry_shader,
	               const ShaderObject& fragment_shader);
	ShaderProgram (const char* vertex_source, const char* fragment_source);

	operator    GLuint       (void) const { return shader_program_; }
	GLint       link         (void);
	std::string get_info_log (void);

	static ShaderProgram from_files (const char* vertex_file, const char* fragment_file);
	static ShaderProgram from_files (const char* vertex_file, const char* geometry_file,
	                                 const char* fragment_file);
	static ShaderProgram simple     (void);
private:
	static void glCreateProgramAdapted (GLsizei, GLuint* p)       { *p = glCreateProgram(); }
	static void glDeleteProgramAdapted (GLsizei, const GLuint* p) { glDeleteProgram(*p); }
	inline static const GenFuncP glCreateProgramP = &glCreateProgramAdapted;
	inline static const DelFuncP glDeleteProgramP = &glDeleteProgramAdapted;

	Handle<&glCreateProgramP, &glDeleteProgramP> shader_program_;
};
} // namespace GL

#endif // GLOBJECTS_H
