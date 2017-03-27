#ifndef QTSYMMETRYCANVAS_H
#define QTSYMMETRYCANVAS_H

#include <QWidget>
#include <memory>
#include <Eigen/Geometry>
#include "GLInclude.h"

class QtOpenGLSubwindow;
class Tiling;
class ShaderCanvas;
namespace GL { class Texture; }

class QtSymmetryCanvas : public QWidget
{
	Q_OBJECT

public:
	explicit QtSymmetryCanvas (QWidget* parent = 0);
	virtual ~QtSymmetryCanvas (void);

	void initializeGL (void);
	void paintGL      (void);
	void resizeGL     (int, int);

protected:
	void mousePressEvent   (QMouseEvent*) override;
	void mouseReleaseEvent (QMouseEvent*) override;
	void mouseMoveEvent    (QMouseEvent*) override;

private:
	void render_image(const GL::Texture& image, int fb_width, int fb_height, GLuint fbo);

	// TODO: Refactor this away.
	Eigen::Vector2f screen_to_world(const Eigen::Vector2f&);


	// These need an OpenGL context, which exists only after initializeGL.
	// Thus they have to live in the heap.
	std::unique_ptr<Tiling>       tiling_;
	std::unique_ptr<ShaderCanvas> canvas_;
	std::unique_ptr<GL::Texture>  base_image_;

	Eigen::Vector3f clear_color_;
	Eigen::Vector2f screen_center_;
	double          pixels_per_unit_;

	bool mouse_down_;
	Eigen::Vector2f press_pos_;
	Eigen::Vector2f press_screen_center_;

	// OpenGL subwindow handling.
	QtOpenGLSubwindow* subwindow_;
	friend class QtOpenGLSubwindow;
	void   subwindow_setup          (void);
	void   subwindow_update         (void);
	GLuint defaultFramebufferObject (void);
};

#endif // QTSYMMETRYCANVAS_H
