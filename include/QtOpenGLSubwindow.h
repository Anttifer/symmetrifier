#ifndef QTOPENGLSUBWINDOW_H
#define QTOPENGLSUBWINDOW_H

#include <QOpenGLWindow>
class QtOpenGLCanvas;

class QtOpenGLSubwindow : public QOpenGLWindow
{
	Q_OBJECT

public:
	QtOpenGLSubwindow (QOpenGLContext* context, QtOpenGLCanvas* canvas, QWindow* parent = 0);

protected:
	void initializeGL (void)     override;
	void paintGL      (void)     override;
	void resizeGL     (int, int) override;

	void mouseMoveEvent    (QMouseEvent*) override;
	void mousePressEvent   (QMouseEvent*) override;
	void mouseReleaseEvent (QMouseEvent*) override;
	void wheelEvent        (QWheelEvent*) override;

private:
	QtOpenGLCanvas* m_pCanvas;
};

#endif // QTOPENGLSUBWINDOW_H
