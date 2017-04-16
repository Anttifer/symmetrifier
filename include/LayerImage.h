#ifndef LAYERIMAGE_H
#define LAYERIMAGE_H

#include "GLObjects.h"
#include <Eigen/Geometry>

class LayerImage
{
public:
	explicit LayerImage (GL::Texture&&);

	const GL::Texture&     texture  (void) const { return texture_; }

	const Eigen::Vector2f& position (void) const { return position_; }
	Eigen::Vector2f        center   (void) const;

	const Eigen::Vector2f& t1       (void) const { return t1_; }
	Eigen::Vector2f        t2       (void) const;

	float                  rotation (void) const;
	float                  scale    (void) const { return t1_.norm(); }

	void set_texture (GL::Texture&&);

	void set_position   (const Eigen::Vector2f& p) { position_ = p; }
	void set_center     (const Eigen::Vector2f&);

	void set_t1         (const Eigen::Vector2f&);

	void set_rotation   (float);
	void set_scale      (float);
	void multiply_scale (float factor);

private:
	GL::Texture     texture_;
	Eigen::Vector2f position_;
	Eigen::Vector2f t1_;
};

#endif // LAYERIMAGE_H
