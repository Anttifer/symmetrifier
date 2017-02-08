#ifndef SHADERCANVAS_H
#define SHADERCANVAS_H

#include "GLObjects.h"

//--------------------

class ShaderCanvas {
public:
	ShaderCanvas(void);
	ShaderCanvas(const ShaderCanvas&) = delete;

	ShaderCanvas& operator=(const ShaderCanvas&) = delete;
public:
	GL::Buffer position_buffer_;
	GL::Buffer texcoord_buffer_;
	GL::VAO vao_;
	size_t num_vertices_;
	GLenum primitive_type_;
};

#endif // SHADERCANVAS_H
