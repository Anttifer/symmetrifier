#include "QtOpenGLSubwindow.h"

#include "QtSymmetryCanvas.h"

QtOpenGLSubwindow::QtOpenGLSubwindow(QtSymmetryCanvas* enclosing_widget, QWindow* parent) :
	QOpenGLWindow           (NoPartialUpdate, parent),
	enclosing_widget_       (enclosing_widget)
{
	QSurfaceFormat fmt;
	fmt.setVersion(3, 3);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setSwapInterval(0);
	setFormat(fmt);
}

void QtOpenGLSubwindow::initializeGL(void)
{
	enclosing_widget_->initializeGL();
}

void QtOpenGLSubwindow::paintGL(void)
{
	enclosing_widget_->paintGL();
}

void QtOpenGLSubwindow::resizeGL(int width, int height)
{
	enclosing_widget_->resizeGL(width, height);
}

void QtOpenGLSubwindow::mouseMoveEvent(QMouseEvent* event)
{
	enclosing_widget_->mouseMoveEvent(event);
}

void QtOpenGLSubwindow::mousePressEvent(QMouseEvent* event)
{
	enclosing_widget_->mousePressEvent(event);
}

void QtOpenGLSubwindow::mouseReleaseEvent(QMouseEvent* event)
{
	enclosing_widget_->mouseReleaseEvent(event);
}
