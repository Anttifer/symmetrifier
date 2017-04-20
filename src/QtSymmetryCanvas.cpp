#include <GL/glew.h>
#include "QtSymmetryCanvas.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <iostream>
#include <cmath>
#include "GLObjects.h"
#include "Tiling.h"
#include "ShaderCanvas.h"

QtSymmetryCanvas::QtSymmetryCanvas(QWidget* parent) :
	QtOpenGLCanvas   (parent),

	clear_color_     (0.1f, 0.1f, 0.1f),
	screen_center_   (0.5f, 0.5f),
	pixels_per_unit_ (500),
	zoom_factor_     (1.2),

	mouse_down_      (false)
{}

QtSymmetryCanvas::~QtSymmetryCanvas(void) {}

void QtSymmetryCanvas::initializeGL(void)
{
	// "Enable" depth testing and alpha blending.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// This probably doesn't work, but worth asking anyway. :)
	glEnable(GL_LINE_SMOOTH);

	tiling_ = std::make_unique<Tiling>();
	canvas_ = std::make_unique<ShaderCanvas>();
	base_image_ = std::make_unique<GL::Texture>(GL::Texture::from_png("res/kissa"));

	// Nearest neighbor filtering for the base image.
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, *base_image_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, old_tex);

	tiling_->set_symmetry_group("333");
	tiling_->set_center({0.5f, 0.5f});
}

void QtSymmetryCanvas::paintGL(void)
{
	auto fbo = defaultFramebufferObject();

	glClearColor(clear_color_.x(), clear_color_.y(), clear_color_.z(), 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_image(*base_image_, width(), height(), fbo);

	// render_scene(width(), height(), fbo);
}

void QtSymmetryCanvas::mousePressEvent(QMouseEvent* event)
{
	press_pos_ = { event->x() / (float)width() * 2.0f - 1.0f,
	               1.0f - event->y() / (float)height() * 2.0f };

	press_screen_center_ = screen_center_;
	mouse_down_ = true;
}

void QtSymmetryCanvas::mouseReleaseEvent(QMouseEvent* event)
{
	mouse_down_ = false;
}

void QtSymmetryCanvas::mouseMoveEvent(QMouseEvent* event)
{
	if (mouse_down_)
	{
		Eigen::Vector2f pos  = { event->x() / (float)width() * 2.0f - 1.0f,
		                         1.0f - event->y() / (float)height() * 2.0f };
		Eigen::Vector2f diff = pos - press_pos_;

		screen_center_ = press_screen_center_ - screen_to_world(diff);

		updateSubwindow();
	}
}

void QtSymmetryCanvas::wheelEvent(QWheelEvent* event)
{
	float delta = event->angleDelta().y() / 256.0f;
	pixels_per_unit_ *= std::pow(zoom_factor_, delta);

	updateSubwindow();
}

void QtSymmetryCanvas::render_image(const GL::Texture& image, int fb_width, int fb_height, GLuint fbo)
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/image_vert.glsl",
		"shaders/image_frag.glsl");

	// Find uniform locations once.
	static GLuint screen_size_uniform;
	static GLuint screen_center_uniform;
	static GLuint image_position_uniform;
	static GLuint image_t1_uniform;
	static GLuint image_t2_uniform;
	static GLuint pixels_per_unit_uniform;
	static GLuint texture_sampler_uniform;
	static bool init = [&](){
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		screen_center_uniform   = glGetUniformLocation(shader, "uScreenCenter");
		image_position_uniform  = glGetUniformLocation(shader, "uImagePos");
		image_t1_uniform        = glGetUniformLocation(shader, "uImageT1");
		image_t2_uniform        = glGetUniformLocation(shader, "uImageT2");
		pixels_per_unit_uniform = glGetUniformLocation(shader, "uPixelsPerUnit");
		texture_sampler_uniform = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	const Eigen::Vector2f image_position = {0.0f, 0.0f};
	const Eigen::Vector2f image_t1       = {1.0f, 0.0f};
	const Eigen::Vector2f image_t2       = {0.0f, image.height_ / (float)image.width_};

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, image);

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform2i  (screen_size_uniform, fb_width, fb_height);
	glUniform2fv (screen_center_uniform, 1, screen_center_.data());
	glUniform2fv (image_position_uniform, 1, image_position.data());
	glUniform2fv (image_t1_uniform, 1, image_t1.data());
	glUniform2fv (image_t2_uniform, 1, image_t2.data());
	glUniform1f  (pixels_per_unit_uniform, pixels_per_unit_);
	glUniform1i  (texture_sampler_uniform, 1);

	glBindVertexArray(canvas_->vao_);
	glDrawArrays(canvas_->primitive_type_, 0, canvas_->num_vertices_);

	// Clean up.
	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

Eigen::Vector2f QtSymmetryCanvas::screen_to_world(const Eigen::Vector2f& v)
{
	return { v.x() * (0.5f * width()) / pixels_per_unit_,
	         v.y() * (0.5f * height()) / pixels_per_unit_ };
}
