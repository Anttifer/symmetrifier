#ifndef PINWHEELPLANE_H
#define PINWHEELPLANE_H

#include "Mesh.h"
#include <Eigen/Geometry>

//--------------------

class PinwheelPlane {
public:
	PinwheelPlane (void);

	void subdivide       (void);
	void trim            (void); // TODO: Implement this.

	size_t                 num_triangles   (void) const { return mesh_.positions_.size() / 3; }
	const Mesh&            mesh            (void) const { return mesh_; }
	const Eigen::Vector2f& position        (void) const { return position_; }

	void set_position (const Eigen::Vector2f& p) { position_ = p; }

private:
	Mesh mesh_;

	Eigen::Vector2f position_;
	double          size_parameter_; // length of short leg
};

#endif // PINWHEELPLANE_H
