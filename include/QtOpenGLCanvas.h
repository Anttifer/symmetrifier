#ifndef QTOPENGLCANVAS_H
#define QTOPENGLCANVAS_H

#include <QWidget>
#include "GLInclude.h"

class QtOpenGLSubwindow;
class QOpenGLContext;

class QtOpenGLCanvas : public QWidget
{
	Q_OBJECT

public:
	explicit QtOpenGLCanvas (QOpenGLContext* context, QWidget* parent = 0);

protected:
	virtual void initializeGL (void)     {}
	virtual void paintGL      (void)     {}
	virtual void resizeGL     (int, int) {}

	GLuint defaultFramebufferObject (void) const;
	void   updateSubwindow          (void);

private:
	// These functions are here and not in QtOpenGLSubwindow because
	// we need to have the subwindow handling and GLEW-using things
	// in separate translation units to avoid the infamous GLEW/Qt
	// inclusion order trouble.
	//
	// A cleaner solution might be to just create separate
	// free-standing wrappers for these functions and call them, but
	// seeing that this class and QtOpenGLSubwindow are hopelessly
	// interdependent anyway, it doesn't really matter.
	void initializeGLEW (void);
	void resizeViewport (int, int);

	QtOpenGLSubwindow* m_pSubwindow;
	friend class QtOpenGLSubwindow;
};

#endif // QTOPENGLCANVAS_H
