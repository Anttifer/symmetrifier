// Nonstandard inclusion order because of Qt and GLEW.
#include <GL/glew.h>
#include "QtSymmetryCanvas.h"
#include "ShaderCanvas.h"
#include <iostream>

QtSymmetryCanvas::QtSymmetryCanvas(QWidget* parent) :
	QOpenGLWidget(parent)
{
	QSurfaceFormat fmt;
	fmt.setVersion(3, 3);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	setFormat(fmt);
	QSurfaceFormat::setDefaultFormat(fmt);
}

void QtSymmetryCanvas::initializeGL(void)
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Error " << glewGetErrorString(err) << std::endl;
		throw std::runtime_error("Failed to initialize GLEW for QOpenGLWidget.");
	}
}

void QtSymmetryCanvas::paintGL(void)
{
	double time = 0.0;
	static ShaderCanvas canvas;
	static auto shader = GL::ShaderProgram(
		GL::ShaderObject::vertex_passthrough(),
		GL::ShaderObject::from_file(GL_FRAGMENT_SHADER, "shaders/wave_frag.glsl"));

	// Find uniform locations once.
	static GLuint screen_size_uniform;
	static GLuint time_uniform;
	static bool init = [&](){
		screen_size_uniform     = glGetUniformLocation(shader, "uScreenSize");
		time_uniform            = glGetUniformLocation(shader, "uTime");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Set the shader program and uniforms, and draw.
	glUseProgram(shader);

	glUniform2i(screen_size_uniform, width(), height());
	glUniform1f(time_uniform, time);

	glBindVertexArray(canvas.vao_);
	glDrawArrays(canvas.primitive_type_, 0, canvas.num_vertices_);
}

void QtSymmetryCanvas::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}
