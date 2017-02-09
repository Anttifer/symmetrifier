#ifndef PINWHEELPLANE_H
#define PINWHEELPLANE_H

#include "Mesh.h"
#include <Eigen/Geometry>

//--------------------

class PinwheelPlane {
public:
	PinwheelPlane (void);

	void subdivide       (void);
	void trim            (void);

	size_t                 num_triangles   (void) const { return mesh_.positions_.size() / 3; }
	double                 pixels_per_unit (void) const { return pixels_per_unit_; }
	const Mesh&            mesh            (void) const { return mesh_; }
	const Eigen::Vector2f& screen_center   (void) const { return screen_center_; }
private:
	Mesh mesh_;

	Eigen::Vector2f screen_center_;
	double          pixels_per_unit_;
	double          size_parameter_; // length of short leg
};

#endif // PINWHEELPLANE_H
