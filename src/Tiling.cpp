#include "Tiling.h"

#include "GLFunctions.h"
#include <cstring>
#include <cstdio>

Tiling::Tiling(void)
:	position_             (0.0, 0.0),
	t1_                   (1.0, 0.0),
	t2_                   (0.0, 1.0),
	symmetrify_shader_    (GL::ShaderProgram::from_files(
		                      "shaders/symmetrify_vert.glsl",
		                      "shaders/symmetrify_geom.glsl",
		                      "shaders/symmetrify_frag.glsl")),
	instance_num_uniform_ (glGetUniformLocation(symmetrify_shader_, "uNumInstances")),
	aspect_ratio_uniform_ (glGetUniformLocation(symmetrify_shader_, "uAR")),
	position_uniform_     (glGetUniformLocation(symmetrify_shader_, "uPos")),
	t1_uniform_           (glGetUniformLocation(symmetrify_shader_, "uT1")),
	t2_uniform_           (glGetUniformLocation(symmetrify_shader_, "uT2")),
	sampler_uniform_      (glGetUniformLocation(symmetrify_shader_, "uTextureSampler")),
	consistent_           (false)
{
	set_symmetry_group("o");
}

// TODO: Add the remaining groups.
void Tiling::set_symmetry_group(const char* group)
{
	consistent_ = false;

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

void Tiling::symmetrify(const GL::Texture& texture, int num_domains)
{
	auto AR        = texture.width_ / (float)texture.height_;
	auto dimension = std::max({texture.width_, texture.height_, 512u});

	// Set up the symmetrified texture.
	domain_texture_ = GL::Texture::empty_2D(dimension, dimension);
	auto fbo        = GL::FBO::simple_C0(domain_texture_);
	glClearColor(0, 0, 0, 0);
	GL::clear(GL_COLOR_BUFFER_BIT, fbo);

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);

	// Set texture wrapping parameters.
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glViewport(0, 0, dimension, dimension);

	// Set the shader program, uniforms and texture parameters, and draw.
	glUseProgram(symmetrify_shader_);

	glUniform1i  (instance_num_uniform_, num_domains);
	glUniform1f  (aspect_ratio_uniform_, AR);
	glUniform2fv (position_uniform_, 1, position_.data());
	glUniform2fv (t1_uniform_, 1, t1_.data());
	glUniform2fv (t2_uniform_, 1, t2_.data());
	glUniform1i  (sampler_uniform_, 1);

	glBindVertexArray(mesh_.vao_);
	glDrawArraysInstanced(mesh_.primitive_type_, 0, mesh_.num_vertices_, num_domains);

	// Clean up.
	glBindVertexArray(0);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	consistent_ = true;
}
