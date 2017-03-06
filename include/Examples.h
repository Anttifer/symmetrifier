#ifndef EXAMPLES_H
#define EXAMPLES_H

#include "GLObjects.h"
#include <Eigen/Geometry>
class Mesh;

//--------------------

namespace Examples
{
void render_wave     (int width, int height, GLuint framebuffer = 0);
void render_pinwheel (const Eigen::Vector2f& screen_center, double pixels_per_unit,
                      int width, int height, GLuint framebuffer);
void render_texture  (const GL::Texture& texture, int width, int height, GLuint framebuffer = 0);
void render_mesh     (const Mesh& mesh, int width, int height, GLuint framebuffer = 0);
void render_on_mesh  (const GL::Texture& texture, const Mesh& mesh, int width, int height, GLuint framebuffer = 0);
} // namespace Examples

#endif // EXAMPLES_H
