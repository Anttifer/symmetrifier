#include "QtOpenGLSubwindow.h"

#include <QOpenGLContext>
#include "QtOpenGLCanvas.h"

QtOpenGLSubwindow::QtOpenGLSubwindow(QOpenGLContext* context,
                                     QtOpenGLCanvas* canvas,
                                     QWindow*        parent) :
	QOpenGLWindow  (context, NoPartialUpdate, parent),
	m_pCanvas      (canvas)
{
	create();
	context->makeCurrent(this);
	m_pCanvas->initializeGLEW();
}

void QtOpenGLSubwindow::initializeGL(void)
{
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
