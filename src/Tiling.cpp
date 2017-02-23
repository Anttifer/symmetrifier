#include "Tiling.h"

#include "GLFunctions.h"
#include <cstring>
#include <cstdio>

Tiling::Tiling(void)
:	position_                    (0.0, 0.0),
	t1_                          (1.0, 0.0),
	t2_                          (0.0, 1.0),
	symmetrify_shader_           (GL::ShaderProgram::from_files(
		                             "shaders/symmetrify_vert.glsl",
		                             "shaders/symmetrify_frag.glsl")),
	aspect_ratio_uniform_        (glGetUniformLocation(symmetrify_shader_, "uAR")),
	symmetrify_position_uniform_ (glGetUniformLocation(symmetrify_shader_, "uPos")),
	symmetrify_t1_uniform_       (glGetUniformLocation(symmetrify_shader_, "uT1")),
	symmetrify_t2_uniform_       (glGetUniformLocation(symmetrify_shader_, "uT2")),
	symmetrify_sampler_uniform_  (glGetUniformLocation(symmetrify_shader_, "uTextureSampler")),
	render_shader_               (GL::ShaderProgram::from_files(
		                             "shaders/tiling_vert.glsl",
		                             "shaders/tiling_geom.glsl",
		                             "shaders/tiling_frag.glsl")),
	render_position_uniform_     (glGetUniformLocation(render_shader_, "uPos")),
	render_t1_uniform_           (glGetUniformLocation(render_shader_, "uT1")),
	render_t2_uniform_           (glGetUniformLocation(render_shader_, "uT2")),
	screen_size_uniform_         (glGetUniformLocation(render_shader_, "uScreenSize")),
	screen_center_uniform_       (glGetUniformLocation(render_shader_, "uScreenCenter")),
	pixels_per_unit_uniform_     (glGetUniformLocation(render_shader_, "uPixelsPerUnit")),
	render_sampler_uniform_      (glGetUniformLocation(render_shader_, "uTextureSampler"))
{
	set_symmetry_group("o");
}

// TODO: Add the remaining groups.
void Tiling::set_symmetry_group(const char* group)
{
	if (!strncmp(group, "o", 8))
	{
		symmetry_group_ = "o";
		construct_p1();
	}
	else if (!strncmp(group, "2*22", 8))
	{
		symmetry_group_ = "2*22";
		construct_cmm();
	}
	else
	{
		printf("Unsupported group. Falling back to pure translational symmetry.\n");
		symmetry_group_ = "o";
		construct_p1();
	}
}

// TODO: Lattice transformations.
void Tiling::construct_p1(void)
{
	mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0}, {1, 1, 0},
		{0, 0, 0}, {1, 1, 0}, {0, 1, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_cmm(void)
{
	mesh_.positions_ = {
		// Bottom triangle, divided in half.
		{0, 0, 0}, {0.5, 0, 0},   {0.5, 0.5, 0},
		{1, 0, 0}, {0.5, 0.5, 0}, {0.5, 0, 0},
		// Left triangle. This is mirrored, so clockwise.
		{0, 0, 0}, {0, 0.5, 0},   {0.5, 0.5, 0},
		{0, 1, 0}, {0.5, 0.5, 0}, {0, 0.5, 0},
		// Top triangle. Not mirrored - counter-clockwise.
		{1, 1, 0}, {0.5, 1, 0},   {0.5, 0.5, 0},
		{0, 1, 0}, {0.5, 0.5, 0}, {0.5, 1, 0},
		// Right triangle. Mirrored. Clockwise again.
		{1, 1, 0}, {1, 0.5, 0},   {0.5, 0.5, 0},
		{1, 0, 0}, {0.5, 0.5, 0}, {1, 0.5, 0}
	};
	mesh_.update_buffers();
}

void Tiling::symmetrify(const GL::Texture& texture)
{
	auto AR       = texture.width_ / (float)texture.height_;
	auto dimension = std::max(texture.width_, texture.height_);

	// Set up the symmetrified texture.
	symmetrified_ = GL::Texture::empty_2D(dimension, dimension);
	fbo_          = GL::FBO::simple_C0(symmetrified_);
	GL::clear(GL_COLOR_BUFFER_BIT, fbo_);

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

	glViewport(0, 0, dimension, dimension);

	// Set the shader program, uniforms and texture, and draw.
	glUseProgram(symmetrify_shader_);

	glUniform1f  (aspect_ratio_uniform_, AR);
	glUniform3fv (symmetrify_position_uniform_, 1, position_.data());
	glUniform3fv (symmetrify_t1_uniform_, 1, t1_.data());
	glUniform3fv (symmetrify_t2_uniform_, 1, t2_.data());
	glUniform1i  (symmetrify_sampler_uniform_, 1);

	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(mesh_.vao_);
	glDrawArrays(mesh_.primitive_type_, 0, mesh_.num_vertices_);

	// Clean up.
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}
