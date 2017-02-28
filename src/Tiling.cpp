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
	else if (!strncmp(group, "2222", 8))
	{
		symmetry_group_ = "2222";
		construct_p2();
	}
	else if (!strncmp(group, "333", 8))
	{
		symmetry_group_ = "333";
		construct_p3();
	}
	else if (!strncmp(group, "*333", 8))
	{
		symmetry_group_ = "*333";
		construct_p3m1();
	}
	else if (!strncmp(group, "3*3", 8))
	{
		symmetry_group_ = "3*3";
		construct_p31m();
	}
	else if (!strncmp(group, "442", 8))
	{
		symmetry_group_ = "442";
		construct_p4();
	}
	else if (!strncmp(group, "*442", 8))
	{
		symmetry_group_ = "*442";
		construct_p4m();
	}
	else if (!strncmp(group, "4*2", 8))
	{
		symmetry_group_ = "4*2";
		construct_p4g();
	}
	else if (!strncmp(group, "632", 8))
	{
		symmetry_group_ = "632";
		construct_p6();
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

// TODO: Add separate frame rendering mesh.
// TODO: Custom lattice transformations.
void Tiling::construct_p1(void)
{
	// Square lattice.
	t2_ = { -t1_.y(), t1_.x() };

	mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0}, {1, 1, 0},
		{1, 1, 0}, {0, 1, 0}, {0, 0, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_p2(void)
{
	// Square lattice.
	t2_ = { -t1_.y(), t1_.x() };

	mesh_.positions_ = {
		// Lower right-hand triangle, divided in half.
		{1, 0, 0}, {0.5, 0.5, 0}, {0, 0, 0},
		{1, 1, 0}, {0.5, 0.5, 0}, {1, 0, 0},
		// Upper left-hand triangle.
		{0, 1, 0}, {0.5, 0.5, 0}, {1, 1, 0},
		{0, 0, 0}, {0.5, 0.5, 0}, {0, 1, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_p3(void)
{
	// Hexagonal lattice.
	t2_ = { -t1_.y(), t1_.x() };
	t2_ = (std::sqrt(3) / 2.0) * t2_ + 0.5 * t1_;

	mesh_.positions_ = {
		// Left and right side.
		{0, 0, 0}, {1 / 3., 1 / 3., 0}, {0, 1, 0},
		{1, 1, 0}, {2 / 3., 2 / 3., 0}, {1, 0, 0},
		// Bottom and top.
		{1, 0, 0}, {1 / 3., 1 / 3., 0}, {0, 0, 0},
		{0, 1, 0}, {2 / 3., 2 / 3., 0}, {1, 1, 0},
		// Center.
		{0, 1, 0}, {1 / 3., 1 / 3., 0}, {1, 0, 0},
		{1, 0, 0}, {2 / 3., 2 / 3., 0}, {0, 1, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_p3m1(void)
{
	// Hexagonal lattice.
	t2_ = { -t1_.y(), t1_.x() };
	t2_ = (std::sqrt(3) / 2.0) * t2_ + 0.5 * t1_;

	mesh_.positions_ = {
		// Bottom and top left.
		{0, 0, 0}, {0.5, 0, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0}, {0, 1, 0},
		// Bottom and top right, mirrored.
		{1, 0, 0}, {0.5, 0, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0}, {1, 1, 0},
		// Left and right top.
		{0, 1, 0}, {0, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0}, {1, 1, 0},
		// Left and right bottom, mirrored.
		{0, 0, 0}, {0, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0}, {1, 0, 0},
		// Bottom center.
		{1, 0, 0}, {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {1, 0, 0},
		// Top center, mirrored.
		{0, 1, 0}, {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_p31m(void)
{
	// Hexagonal lattice.
	t2_ = { -t1_.y(), t1_.x() };
	t2_ = (std::sqrt(3) / 2.0) * t2_ + 0.5 * t1_;

	mesh_.positions_ = {
		// Bottom.
		{0, 0, 0},           {0.5, 0, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0, 0},   {1, 0, 0},
		// Left.
		{0, 1, 0},           {0, 0.5, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0, 0.5, 0},   {0, 0, 0},
		// Top, mirrored.
		{0, 1, 0},           {0.5, 1, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0},   {1, 1, 0},
		// Right, mirrored.
		{1, 1, 0},           {1, 0.5, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0},   {1, 0, 0},
		// Center left.
		{1, 0, 0},           {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0},
		// Kepu, mirrored.
		{1, 0, 0},           {0.5, 0.5, 0}, {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_p4(void)
{
	// Square lattice.
	t2_ = { -t1_.y(), t1_.x() };

	mesh_.positions_ = {
		// Bottom left.
		{0, 0, 0},     {0.5, 0, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 0, 0},
		// Top left.
		{0, 1, 0},     {0, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 1, 0}, {0, 1, 0},
		// Top right.
		{1, 1, 0},     {0.5, 1, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {1, 0.5, 0}, {1, 1, 0},
		// Bottom right.
		{1, 0, 0},     {1, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 0, 0}, {1, 0, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_p4m(void)
{
	// Square lattice.
	t2_ = { -t1_.y(), t1_.x() };

	mesh_.positions_ = {
		// Bottom left.
		{0.5, 0.5, 0}, {0.25, 0.25, 0}, {0.5, 0, 0},
		{0.5, 0, 0},   {0.25, 0.25, 0}, {0, 0, 0},
		// Left bottom, mirrored.
		{0.5, 0.5, 0}, {0.25, 0.25, 0}, {0, 0.5, 0},
		{0, 0.5, 0},   {0.25, 0.25, 0}, {0, 0, 0},
		// Left top.
		{0.5, 0.5, 0}, {0.25, 0.75, 0}, {0, 0.5, 0},
		{0, 0.5, 0},   {0.25, 0.75, 0}, {0, 1, 0},
		// Top left, mirrored.
		{0.5, 0.5, 0}, {0.25, 0.75, 0}, {0.5, 1, 0},
		{0.5, 1, 0},   {0.25, 0.75, 0}, {0, 1, 0},
		// Top right.
		{0.5, 0.5, 0}, {0.75, 0.75, 0}, {0.5, 1, 0},
		{0.5, 1, 0},   {0.75, 0.75, 0}, {1, 1, 0},
		// Right top, mirrored.
		{0.5, 0.5, 0}, {0.75, 0.75, 0}, {1, 0.5, 0},
		{1, 0.5, 0},   {0.75, 0.75, 0}, {1, 1, 0},
		// Right bottom.
		{0.5, 0.5, 0}, {0.75, 0.25, 0}, {1, 0.5, 0},
		{1, 0.5, 0},   {0.75, 0.25, 0}, {1, 0, 0},
		// Bottom right, mirrored.
		{0.5, 0.5, 0}, {0.75, 0.25, 0}, {0.5, 0, 0},
		{0.5, 0, 0},   {0.75, 0.25, 0}, {1, 0, 0},
	};
	mesh_.update_buffers();
}

void Tiling::construct_p4g(void)
{
	// Square lattice.
	t2_ = { -t1_.y(), t1_.x() };

	mesh_.positions_ = {
		// Inner bottom left.
		{0, 0.5, 0},   {0.25, 0.25, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.25, 0.25, 0}, {0.5, 0, 0},
		// Outer bottom left, mirrored.
		{0, 0.5, 0},   {0.25, 0.25, 0}, {0, 0, 0},
		{0, 0, 0},     {0.25, 0.25, 0}, {0.5, 0, 0},
		// Inner top left.
		{0.5, 1, 0},   {0.25, 0.75, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.25, 0.75, 0}, {0, 0.5, 0},
		// Outer top left, mirrored.
		{0.5, 1, 0},   {0.25, 0.75, 0}, {0, 1, 0},
		{0, 1, 0},     {0.25, 0.75, 0}, {0, 0.5, 0},
		// Inner top right.
		{1, 0.5, 0},   {0.75, 0.75, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.75, 0.75, 0}, {0.5, 1, 0},
		// Outer top right, mirrored.
		{1, 0.5, 0},   {0.75, 0.75, 0}, {1, 1, 0},
		{1, 1, 0},     {0.75, 0.75, 0}, {0.5, 1, 0},
		// Inner bottom right.
		{0.5, 0, 0},   {0.75, 0.25, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.75, 0.25, 0}, {1, 0.5, 0},
		// Outer bottom right, mirrored.
		{0.5, 0, 0},   {0.75, 0.25, 0}, {1, 0, 0},
		{1, 0, 0},     {0.75, 0.25, 0}, {1, 0.5, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_p6(void)
{
	// Hexagonal lattice.
	t2_ = { -t1_.y(), t1_.x() };
	t2_ = (std::sqrt(3) / 2.0) * t2_ + 0.5 * t1_;

	mesh_.positions_ = {
		// Bottom.
		{0, 0, 0},           {0.5, 0, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0, 0},   {1, 0, 0},
		// Left.
		{0, 1, 0},           {0, 0.5, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0, 0.5, 0},   {0, 0, 0},
		// Top.
		{1, 1, 0},           {0.5, 1, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0},   {0, 1, 0},
		// Right.
		{1, 0, 0},           {1, 0.5, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0},   {1, 1, 0},
		// Center left.
		{1, 0, 0},           {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0},
		// Kepu.
		{0, 1, 0},           {0.5, 0.5, 0}, {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {1, 0, 0}
	};
	mesh_.update_buffers();
}

void Tiling::construct_cmm(void)
{
	// Square lattice.
	t2_ = { -t1_.y(), t1_.x() };

	mesh_.positions_ = {
		// Bottom triangle, divided in half.
		{0, 0, 0},     {0.5, 0, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 0, 0}, {1, 0, 0},
		// Left triangle. This is mirrored, so clockwise.
		{0, 0, 0},     {0, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 1, 0},
		// Top triangle. Not mirrored - counter-clockwise.
		{1, 1, 0},     {0.5, 1, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 1, 0}, {0, 1, 0},
		// Right triangle. Mirrored. Clockwise again.
		{1, 1, 0},     {1, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {1, 0.5, 0}, {1, 0, 0}
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
