#include "QtOpenGLCanvas.h"

#include "QtOpenGLSubwindow.h"
#include <QBoxLayout>

QtOpenGLCanvas::QtOpenGLCanvas(QWidget* parent) :
	QWidget (parent)
{
	auto layout = new QHBoxLayout;
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);

	m_pSubwindow = new QtOpenGLSubwindow(this);
	layout->addWidget(createWindowContainer(m_pSubwindow, this));
}

void QtOpenGLCanvas::updateSubwindow(void)
{
	m_pSubwindow->update();
}

GLuint QtOpenGLCanvas::defaultFramebufferObject(void) const
{
	return m_pSubwindow->defaultFramebufferObject();
}
