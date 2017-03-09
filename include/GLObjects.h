#ifndef GLOBJECTS_H
#define GLOBJECTS_H

#include <GL/glew.h>
#include <string>
#define GL_SHADER_SOURCE(CODE) #CODE

//--------------------

namespace GL
{
class Buffer {
public:
	Buffer  (void);
	Buffer  (const Buffer&);
	Buffer  (Buffer&&);
	~Buffer (void);

	Buffer& operator= (const Buffer&);
	Buffer& operator= (Buffer&&);
	operator GLuint   (void) const {return buffer_;}

private:
	GLuint buffer_;
};

class Texture {
public:
	Texture  (void);
	Texture  (const Texture&) = delete; // TODO: Implement texture copying.
	Texture  (Texture&&);
	~Texture (void);

	Texture& operator= (const Texture&) = delete;
	Texture& operator= (Texture&&);
	operator GLuint    (void) const {return texture_;}

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
	GLuint texture_;
public:
	unsigned int width_, height_; // TODO: Getters and setters.
};

class VAO {
public:
	VAO  (void);
	VAO  (const VAO&) = delete;
	VAO  (VAO&&);
	~VAO (void);

	VAO& operator=  (const VAO&) = delete;
	VAO& operator=  (VAO&&);
	operator GLuint (void) const {return vao_;}
private:
	GLuint vao_;
};

class FBO {
public:
	FBO  (void);
	FBO  (const FBO&) = delete;
	FBO  (FBO&&);
	~FBO (void);

	FBO& operator=  (const FBO&) = delete;
	FBO& operator=  (FBO&&);
	operator GLuint (void) const {return fbo_;}

	static FBO simple_C0       (const Texture& color);
	static FBO simple_C0D      (const Texture& color, const Texture& depth);
	static FBO multisample_C0  (const Texture& color);
	static FBO multisample_C0D (const Texture& color, const Texture& depth);
private:
	GLuint fbo_;
};

class ShaderObject {
public:
	ShaderObject  (GLenum shader_type);
	ShaderObject  (GLenum shader_type, const char* shader_source);
	ShaderObject  (const ShaderObject&) = delete;
	ShaderObject  (ShaderObject&&);
	~ShaderObject (void);

	ShaderObject& operator= (const ShaderObject&) = delete;
	ShaderObject& operator= (ShaderObject&&);
	operator GLuint         (void) const {return shader_object_;}

	static ShaderObject from_file          (GLenum shader_type, const char* filename);
	static ShaderObject vertex_passthrough (void);
private:
	GLuint shader_object_;
};

class ShaderProgram {
public:
	ShaderProgram  (void);
	ShaderProgram  (const ShaderObject& vertex_shader, const ShaderObject& fragment_shader);
	ShaderProgram  (const ShaderObject& vertex_shader, const ShaderObject& geometry_shader,
	                const ShaderObject& fragment_shader);
	ShaderProgram  (const char* vertex_source, const char* fragment_source);
	ShaderProgram  (const ShaderProgram&) = delete;
	ShaderProgram  (ShaderProgram&&);
	~ShaderProgram (void);

	ShaderProgram& operator= (const ShaderProgram&) = delete;
	ShaderProgram& operator= (ShaderProgram&&);
	operator GLuint          (void) const {return shader_program_;}

	GLint link               (void);
	std::string get_info_log (void);

	static ShaderProgram from_files (const char* vertex_file, const char* fragment_file);
	static ShaderProgram from_files (const char* vertex_file, const char* geometry_file,
	                                 const char* fragment_file);
	static ShaderProgram simple     (void);
private:
	GLuint shader_program_;
};
} // namespace GL

#endif // GLOBJECTS_H
