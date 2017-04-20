#include <GL/glew.h>
#include "QtOpenGLCanvas.h"

#include <iostream>

void QtOpenGLCanvas::initializeGLEW(void)
{
	glewExperimental = GL_TRUE;
	GLenum status    = glewInit();

	if (status != GLEW_OK)
	{
		std::cerr << "Error " << glewGetErrorString(status) << std::endl;
		throw std::runtime_error("Failed to initialize GLEW for QtOpenGLCanvas.");
	}
}

void QtOpenGLCanvas::resizeViewport(int width, int height)
{
	glViewport(0, 0, width, height);
}
