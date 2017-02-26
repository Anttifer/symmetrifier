#include "PinwheelPlane.h"
#include <cmath>

PinwheelPlane::PinwheelPlane(void)
:	position_(0.0, 0.0),
	size_parameter_(1.0)
{
	mesh_.positions_.emplace_back(-size_parameter_, size_parameter_ / 2, 0.0);
	mesh_.positions_.emplace_back(-size_parameter_, -size_parameter_ / 2, 0.0);
	mesh_.positions_.emplace_back(size_parameter_, -size_parameter_ / 2, 0.0);

	mesh_.update_buffers();
}

void PinwheelPlane::subdivide(void)
{
	std::vector<Eigen::Vector3f> new_vertices;
	for (size_t i = 0; i < num_triangles(); ++i)
	{
		const auto& v1 = mesh_.positions_[3*i];
		const auto& v2 = mesh_.positions_[3*i + 1];
		const auto& v3 = mesh_.positions_[3*i + 2];

		bool first_leg_short = (v2 - v1).squaredNorm() < (v3 - v2).squaredNorm();

		if (first_leg_short)
		{
			const auto va = v1 + (v3 - v1) * (1.0 / 5.0);
			const auto vb = v3 - (v3 - v1) * (2.0 / 5.0);
			const auto vc = (v2 + va) / 2.0;
			const auto vd = (v2 + v3) / 2.0;

			new_vertices.push_back(v2);
			new_vertices.push_back(va);
			new_vertices.push_back(v1);

			new_vertices.push_back(vd);
			new_vertices.push_back(vc);
			new_vertices.push_back(v2);

			new_vertices.push_back(va);
			new_vertices.push_back(vc);
			new_vertices.push_back(vd);

			new_vertices.push_back(vd);
			new_vertices.push_back(vb);
			new_vertices.push_back(va);

			new_vertices.push_back(v3);
			new_vertices.push_back(vb);
			new_vertices.push_back(vd);
		}
		else
		{
			const auto va = v1 + (v3 - v1) * (2.0 / 5.0);
			const auto vb = v3 - (v3 - v1) * (1.0 / 5.0);
			const auto vc = (v2 + vb) / 2.0;
			const auto vd = (v1 + v2) / 2.0;

			new_vertices.push_back(v3);
			new_vertices.push_back(vb);
			new_vertices.push_back(v2);

			new_vertices.push_back(v2);
			new_vertices.push_back(vc);
			new_vertices.push_back(vd);

			new_vertices.push_back(vd);
			new_vertices.push_back(vc);
			new_vertices.push_back(vb);

			new_vertices.push_back(vb);
			new_vertices.push_back(va);
			new_vertices.push_back(vd);

			new_vertices.push_back(vd);
			new_vertices.push_back(va);
			new_vertices.push_back(v1);
		}
	}

	size_parameter_ /= std::sqrt(5.0);
	mesh_.positions_ = std::move(new_vertices);
	mesh_.update_buffers();
}
