#include "QtSymmetryCanvas.h"

#include "QtOpenGLSubwindow.h"
#include <QBoxLayout>

void QtSymmetryCanvas::subwindow_setup(void)
{
	subwindow_  = new QtOpenGLSubwindow(this);
	auto layout = new QHBoxLayout;
	setLayout(layout);

	layout->addWidget(createWindowContainer(subwindow_, this));
	layout->setContentsMargins(0, 0, 0, 0);
}

void QtSymmetryCanvas::subwindow_update(void)
{
	subwindow_->update();
}

GLuint QtSymmetryCanvas::defaultFramebufferObject(void)
{
	return subwindow_->defaultFramebufferObject();

}
