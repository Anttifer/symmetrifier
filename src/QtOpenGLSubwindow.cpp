#include "QtOpenGLSubwindow.h"

#include "QtOpenGLCanvas.h"

QtOpenGLSubwindow::QtOpenGLSubwindow(QtOpenGLCanvas* canvas, QWindow* parent) :
	QOpenGLWindow  (NoPartialUpdate, parent),
	m_pCanvas      (canvas)
{
	QSurfaceFormat fmt;
	fmt.setVersion(3, 3);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setSwapInterval(0);
	setFormat(fmt);
}

void QtOpenGLSubwindow::initializeGL(void)
{
	m_pCanvas->initializeGLEW();
	m_pCanvas->initializeGL();
}

void QtOpenGLSubwindow::paintGL(void)
{
	m_pCanvas->paintGL();
}

void QtOpenGLSubwindow::resizeGL(int width, int height)
{
	m_pCanvas->resizeViewport(width, height);
	m_pCanvas->resizeGL(width, height);
}

void QtOpenGLSubwindow::mouseMoveEvent(QMouseEvent* event)
{
	m_pCanvas->mouseMoveEvent(event);
}

void QtOpenGLSubwindow::mousePressEvent(QMouseEvent* event)
{
	m_pCanvas->mousePressEvent(event);
}

void QtOpenGLSubwindow::mouseReleaseEvent(QMouseEvent* event)
{
	m_pCanvas->mouseReleaseEvent(event);
}

void QtOpenGLSubwindow::wheelEvent(QWheelEvent* event)
{
	m_pCanvas->wheelEvent(event);
}
