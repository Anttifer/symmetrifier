#ifndef QTOPENGLCANVAS_H
#define QTOPENGLCANVAS_H

#include <QWidget>
#include "GLInclude.h"

class QtOpenGLSubwindow;

class QtOpenGLCanvas : public QWidget
{
	Q_OBJECT

public:
	explicit QtOpenGLCanvas (QWidget* parent = 0);

protected:
	virtual void initializeGL (void)     {}
	virtual void paintGL      (void)     {}
	virtual void resizeGL     (int, int) {}

	GLuint defaultFramebufferObject (void) const;
	void   updateSubwindow          (void);

private:
	void initializeGLEW (void);
	void resizeViewport (int, int);

	QtOpenGLSubwindow* m_pSubwindow;
	friend class QtOpenGLSubwindow;
};

#endif // QTOPENGLCANVAS_H
