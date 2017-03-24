#ifndef QTSYMMETRYCANVAS_H
#define QTSYMMETRYCANVAS_H

#include <QOpenGLWidget>
#include <memory>
#include <Eigen/Geometry>

class Tiling;
class ShaderCanvas;
namespace GL { class Texture; }

class QtSymmetryCanvas : public QOpenGLWidget
{
	Q_OBJECT

public:
	explicit QtSymmetryCanvas (QWidget* parent = 0);
	virtual ~QtSymmetryCanvas (void);

protected:
	void initializeGL (void)     override;
	void paintGL      (void)     override;
	void resizeGL     (int, int) override;

	void mousePressEvent (QMouseEvent*) override;
	void mouseMoveEvent  (QMouseEvent*) override;

private:
	void render_image(const GL::Texture& image, int fb_width, int fb_height, GLuint fbo);

	// TODO: Refactor this away.
	Eigen::Vector2f screen_to_world(const Eigen::Vector2f&);

	std::unique_ptr<Tiling>       tiling_;
	std::unique_ptr<ShaderCanvas> canvas_;
	std::unique_ptr<GL::Texture>  base_image_;

	Eigen::Vector3f clear_color_;
	Eigen::Vector2f screen_center_;
	double          pixels_per_unit_;

	Eigen::Vector2f press_pos_;
	Eigen::Vector2f press_screen_center_;
};

#endif // QTSYMMETRYCANVAS_H
