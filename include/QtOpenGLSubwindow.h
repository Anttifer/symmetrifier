#ifndef QTOPENGLSUBWINDOW_H
#define QTOPENGLSUBWINDOW_H

#include <QOpenGLWindow>
class QtSymmetryCanvas;

class QtOpenGLSubwindow : public QOpenGLWindow
{
	Q_OBJECT

public:
	explicit QtOpenGLSubwindow (QtSymmetryCanvas* enclosing_widget, QWindow* parent = 0);

protected:
	void initializeGL (void)     override;
	void paintGL      (void)     override;
	void resizeGL     (int, int) override;

	void mouseMoveEvent    (QMouseEvent*) override;
	void mousePressEvent   (QMouseEvent*) override;
	void mouseReleaseEvent (QMouseEvent*) override;

private:
	// TODO: Write a QtOpenGLCanvas widget to generalize this.
	QtSymmetryCanvas* enclosing_widget_;
};

#endif // QTOPENGLSUBWINDOW_H
