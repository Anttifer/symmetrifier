#ifndef QTSYMMETRYCANVAS_H
#define QTSYMMETRYCANVAS_H

#include <QOpenGLWidget>

class QtSymmetryCanvas : public QOpenGLWidget
{
	Q_OBJECT

public:
	explicit QtSymmetryCanvas(QWidget* parent = 0);

protected:
	void initializeGL (void)     override;
	void paintGL      (void)     override;
	void resizeGL     (int, int) override;
};

#endif // QTSYMMETRYCANVAS_H
