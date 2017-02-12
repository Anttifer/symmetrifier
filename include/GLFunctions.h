#ifndef GLFUNCTIONS_H
#define GLFUNCTIONS_H

#include <GL/glew.h>
#include "GLObjects.h"

//--------------------

namespace GL
{
inline void clear(GLbitfield mask, GLuint framebuffer = 0)
{
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClear(mask);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

size_t internal_format_size(GLenum format);

void tex_to_png(const Texture& texture, const char* filename);
} // namespace GL

#endif // GLFUNCTIONS_H
