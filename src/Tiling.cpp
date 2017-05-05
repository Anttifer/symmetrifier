#include "Tiling.h"

#include "GLFunctions.h"
#include <cstring>
#include <cstdio>

#define SCALE 0.98f

Tiling::Tiling(void) : Tiling(SCALE) {}

Tiling::Tiling(float symmetry_scale) :
	num_lattice_domains_ (1),

	position_            (0.0f, 0.0f),
	t1_                  (1.0f, 0.0f),

	line_color_          (1.0, 0.6, 0.1),
	mirror_color_        (0.1, 0.6, 1.0),
	rotation_color_      (0.1, 1.0, 0.6),

	symmetry_scale_      (symmetry_scale)
{
	set_symmetry_group("o");
}

Eigen::Vector2f Tiling::center(void) const
{
	if (num_lattice_domains_ % 2)
		return position_ + (t1_ + t2()) / 2.0f;
	else
		return position_;
}

Eigen::Vector2f Tiling::t2(void) const
{
	Eigen::Vector2f orthogonal = { -t1_.y(), t1_.x() };
	return t1_ * t2_relative_.x() + orthogonal * t2_relative_.y();
}

double Tiling::rotation(void) const
{
	return std::atan2(t1_.y(), t1_.x());
}

void Tiling::set_symmetry_group(const char* group)
{
	if (!strncmp(group, "o", 8))
	{
		symmetry_group_ = "o";
		lattice_        = Lattice::Oblique;
		construct_p1();
	}
	else if (!strncmp(group, "**", 8))
	{
		symmetry_group_ = "**";
		lattice_        = Lattice::Rectangular;
		construct_pm();
	}
	else if (!strncmp(group, "*x", 8))
	{
		symmetry_group_ = "*x";
		lattice_        = Lattice::Rhombic;
		construct_cm();
	}
	else if (!strncmp(group, "xx", 8))
	{
		symmetry_group_ = "xx";
		lattice_        = Lattice::Rectangular;
		construct_pg();
	}
	else if (!strncmp(group, "2222", 8))
	{
		symmetry_group_ = "2222";
		lattice_        = Lattice::Oblique;
		construct_p2();
	}
	else if (!strncmp(group, "*2222", 8))
	{
		symmetry_group_ = "*2222";
		lattice_        = Lattice::Rectangular;
		construct_pmm();
	}
	else if (!strncmp(group, "22*", 8))
	{
		symmetry_group_ = "22*";
		lattice_        = Lattice::Rectangular;
		construct_pmg();
	}
	else if (!strncmp(group, "2*22", 8))
	{
		symmetry_group_ = "2*22";
		lattice_        = Lattice::Rhombic;
		construct_cmm();
	}
	else if (!strncmp(group, "22x", 8))
	{
		symmetry_group_ = "22x";
		lattice_        = Lattice::Rectangular;
		construct_pgg();
	}
	else if (!strncmp(group, "333", 8))
	{
		symmetry_group_ = "333";
		lattice_        = Lattice::Hexagonal;
		construct_p3();
	}
	else if (!strncmp(group, "*333", 8))
	{
		symmetry_group_ = "*333";
		lattice_        = Lattice::Hexagonal;
		construct_p3m1();
	}
	else if (!strncmp(group, "3*3", 8))
	{
		symmetry_group_ = "3*3";
		lattice_        = Lattice::Hexagonal;
		construct_p31m();
	}
	else if (!strncmp(group, "442", 8))
	{
		symmetry_group_ = "442";
		lattice_        = Lattice::Square;
		construct_p4();
	}
	else if (!strncmp(group, "*442", 8))
	{
		symmetry_group_ = "*442";
		lattice_        = Lattice::Square;
		construct_p4m();
	}
	else if (!strncmp(group, "4*2", 8))
	{
		symmetry_group_ = "4*2";
		lattice_        = Lattice::Square;
		construct_p4g();
	}
	else if (!strncmp(group, "632", 8))
	{
		symmetry_group_ = "632";
		lattice_        = Lattice::Hexagonal;
		construct_p6();
	}
	else if (!strncmp(group, "*632", 8))
	{
		symmetry_group_ = "*632";
		lattice_        = Lattice::Hexagonal;
		construct_p6m();
	}
	else
	{
		printf("Unsupported group. Falling back to pure translational symmetry.\n");
		symmetry_group_ = "o";
		lattice_        = Lattice::Oblique;
		construct_p1();
	}

	construct_symmetry_mesh();
	construct_mesh_texture();
}

void Tiling::set_num_lattice_domains(int n)
{
	num_lattice_domains_ = n;
	construct_mesh_texture();
}

void Tiling::set_center(const Eigen::Vector2f& center)
{
	position_ = center;
	if (num_lattice_domains_ % 2)
		position_ -= (t1_ + t2()) / 2.0f;
}

void Tiling::set_t1(const Eigen::Vector2f& t1)
{
	// Never scale to zero.
	if (t1.x() == 0.0f && t1.y() == 0.0f)
		return;

	auto center = this->center();

	t1_ = t1;

	this->set_center(center);
}

void Tiling::set_t2(const Eigen::Vector2f& t2)
{
	// Early out if lattice is square or hexagonal and thus fixed.
	if (lattice_ == Lattice::Square || lattice_ == Lattice::Hexagonal)
		return;

	// Convert t2 to relative coordinates.
	Eigen::Vector2f orthogonal = { -t1_.y(), t1_.x() };

	Eigen::Matrix2f basis;
	basis << t1_, orthogonal;

	Eigen::Vector2f t2_relative = basis.inverse() * t2;

	// Fulfill lattice constraints as well as possible.
	if (lattice_ == Lattice::Oblique)
		// No constraints.
		t2_relative_ = t2_relative;
	else if (lattice_ == Lattice::Rhombic)
		// Must be of same length as t1.
		t2_relative_ = t2_relative.normalized();
	else if (lattice_ == Lattice::Rectangular)
		// Must be orthogonal to t1.
		t2_relative_ = {0.0f, t2_relative.y()};
}

void Tiling::set_rotation(double r)
{
	auto center   = this->center();
	float norm    = t1_.norm();
	auto rotation = Eigen::Rotation2D<float>(r);

	t1_ = rotation * Eigen::Vector2f(norm, 0);

	this->set_center(center);
}

void Tiling::set_scale(double scale)
{
	// Never scale to zero.
	if (scale == 0.0)
		return;

	auto center = this->center();
	t1_.normalize();
	t1_ *= scale;

	this->set_center(center);
}

void Tiling::multiply_scale(double factor)
{
	// Never scale to zero.
	if (factor == 0.0)
		return;

	auto center = this->center();
	t1_ *= factor;

	this->set_center(center);
}

void Tiling::set_deform_origin(const Eigen::Vector2f& deform_origin)
{
	deform_origin_      = deform_origin;
	deform_original_t1_ = t1_;
	deform_original_t2_ = t2();

	if (lattice_ == Lattice::Rectangular || lattice_ == Lattice::Rhombic)
	{
		Eigen::Matrix2f basis;
		basis << t1_.normalized(), t2().normalized();

		Eigen::Vector2f origin = basis.inverse() * (deform_origin - center());

		deform_quadrant_ = { origin.x() >= 0.0f ? 1.0f : -1.0f,
		                     origin.y() >= 0.0f ? 1.0f : -1.0f };
		deform_corner_   = deform_quadrant_.x() * t1_
		                 + deform_quadrant_.y() * t2();
	}
}

void Tiling::deform(const Eigen::Vector2f& deformation)
{
	auto center = this->center();

	float adj_factor = 2.0f / std::sqrt(num_lattice_domains_);
	Eigen::Vector2f adj_deformation = adj_factor * deformation;

	if (lattice_ == Lattice::Rhombic)
	{
		Eigen::Vector2f orthogonal = { -deform_corner_.y(), deform_corner_.x() };
		Eigen::Matrix2f basis;
		basis << deform_corner_.normalized(),
		         orthogonal.normalized() * (-deform_quadrant_.prod());

		Eigen::Vector2f c_relative = basis.inverse() * (deform_corner_ + adj_deformation);

		float h = deform_original_t1_.norm();
		float x = std::abs(c_relative.x() / 2.0f);
		while (x > h) // We don't want NaNs from the square root.
			x = std::abs(2.0f * h - x);
		float y = std::sqrt(h*h - x*x);

		Eigen::Vector2f t1_relative = {x, y};
		Eigen::Vector2f t2_relative = {x, -y};

		t1_        = basis * t1_relative * deform_quadrant_.x();
		this->set_t2(basis * t2_relative * deform_quadrant_.y());
	}
	else if (lattice_ == Lattice::Rectangular)
	{
		Eigen::Matrix2f basis;
		basis << deform_original_t1_.normalized(),
		         deform_original_t2_.normalized();
		basis *= deform_quadrant_.asDiagonal();

		Eigen::Vector2f c_relative = basis.inverse() * (deform_corner_ + adj_deformation);

		t1_          = c_relative.cwiseAbs().x() * deform_original_t1_.normalized();
		t2_relative_ = {0.0f, c_relative.cwiseAbs().y() / t1_.norm()};
	}

	this->set_center(center);
}

// TODO: Preserve custom lattice transformations?
void Tiling::construct_p1(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0}, {1, 1, 0},
		{1, 1, 0}, {0, 1, 0}, {0, 0, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},
	};

	// We'll use normals as colors.
	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(8, line_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_pm(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Left half.
		{0, 0, 0},   {0.5, 0, 0}, {0.5, 1, 0},
		{0.5, 1, 0}, {0, 1, 0},   {0, 0, 0},
		// Right half, mirrored.
		{1, 0, 0},   {0.5, 0, 0}, {0.5, 1, 0},
		{0.5, 1, 0}, {1, 1, 0},   {1, 0, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0.5, 0, 0}, {0.5, 1, 0},
	};

	frame_mesh_.normals_ = {
		line_color_, line_color_,
		line_color_, line_color_,
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,

		mirror_color_, mirror_color_
	};

	frame_mesh_.update_buffers();
}

void Tiling::construct_cm(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Bottom right.
		{1, 1, 0}, {0.5, 0.5, 0}, {1, 0, 0},
		{1, 0, 0}, {0.5, 0.5, 0}, {0, 0, 0},
		// Top left, mirrored.
		{1, 1, 0}, {0.5, 0.5, 0}, {0, 1, 0},
		{0, 1, 0}, {0.5, 0.5, 0}, {0, 0, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0, 0, 0}, {1, 1, 0}
	};

	frame_mesh_.normals_ = {
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,

		mirror_color_, mirror_color_
	};

	frame_mesh_.update_buffers();
}

void Tiling::construct_pg(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Left half.
		{0, 0, 0},   {0.5, 0, 0}, {0.5, 1, 0},
		{0.5, 1, 0}, {0, 1, 0},   {0, 0, 0},
		// Right half, mirrored.
		{0.5, 1, 0}, {1, 1, 0},   {1, 0, 0},
		{1, 0, 0},   {0.5, 0, 0}, {0.5, 1, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0.5, 0, 0}, {0.5, 1, 0},
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(10, line_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_p2(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Lower right-hand triangle, divided in half.
		{1, 0, 0}, {0.5, 0.5, 0}, {0, 0, 0},
		{1, 1, 0}, {0.5, 0.5, 0}, {1, 0, 0},
		// Upper left-hand triangle.
		{0, 1, 0}, {0.5, 0.5, 0}, {1, 1, 0},
		{0, 0, 0}, {0.5, 0.5, 0}, {0, 1, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_  = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0, 0, 0}, {1, 1, 0}
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(10, rotation_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_pmm(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Bottom left.
		{0, 0, 0},     {0.5, 0, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 0, 0},
		// Top left, mirrored.
		{0, 1, 0},     {0.5, 1, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 1, 0},
		// Top right.
		{1, 1, 0},     {0.5, 1, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {1, 0.5, 0}, {1, 1, 0},
		// Bottom right, mirrored.
		{1, 0, 0},     {0.5, 0, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {1, 0.5, 0}, {1, 0, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0, 0.5, 0}, {1, 0.5, 0},
		{0.5, 0, 0}, {0.5, 1, 0}
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(12, mirror_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_pmg(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Bottom left.
		{0, 0, 0},     {0.5, 0, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 0, 0},
		// Top left, mirrored.
		{0, 1, 0},     {0.5, 1, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 1, 0},
		// Top right, mirrored.
		{1, 0.5, 0},   {0.5, 0.5, 0}, {0.5, 1, 0},
		{0.5, 1, 0},   {1, 1, 0},     {1, 0.5, 0},
		// Bottom right.
		{1, 0.5, 0},   {0.5, 0.5, 0}, {0.5, 0, 0},
		{0.5, 0, 0},   {1, 0, 0},     {1, 0.5, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0, 0.5, 0}, {1, 0.5, 0},
		{0.5, 0, 0}, {0.5, 1, 0}
	};

	frame_mesh_.normals_ = {
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,

		mirror_color_, mirror_color_,
		rotation_color_, rotation_color_
	};
	frame_mesh_.update_buffers();
}

void Tiling::construct_cmm(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Bottom triangle, divided in half.
		{0, 0, 0},     {0.5, 0, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 0, 0}, {1, 0, 0},
		// Left triangle. This is mirrored, so clockwise.
		{0, 0, 0},     {0, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 1, 0},
		// Top triangle. Not mirrored - counter-clockwise.
		{1, 1, 0},     {0.5, 1, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 1, 0}, {0, 1, 0},
		// Right triangle. Mirrored. Clockwise again.
		{1, 1, 0},     {1, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {1, 0.5, 0}, {1, 0, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0, 0, 0}, {1, 1, 0},
		{0, 1, 0}, {1, 0, 0}
	};

	frame_mesh_.normals_ = {
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,

		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_
	};
	frame_mesh_.update_buffers();
}

void Tiling::construct_pgg(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Bottom left and right.
		{0, 0.5, 0}, {0, 0, 0},     {0.5, 0, 0},
		{0.5, 0, 0}, {1, 0, 0},     {1, 0.5, 0},
		// Top right and left.
		{1, 0.5, 0}, {1, 1, 0},     {0.5, 1, 0},
		{0.5, 1, 0}, {0, 1, 0},     {0, 0.5, 0},
		// Bottom center, mirrored.
		{0.5, 0, 0}, {0.5, 0.5, 0}, {1, 0.5, 0},
		{0, 0.5, 0}, {0.5, 0.5, 0}, {0.5, 0, 0},
		// Top center, mirrored.
		{0.5, 1, 0}, {0.5, 0.5, 0}, {0, 0.5, 0},
		{1, 0.5, 0}, {0.5, 0.5, 0}, {0.5, 1, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 0.5, 0}, {1, 0.5, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0.5, 0}, {0.5, 0, 0},
		{0.5, 0, 0}, {1, 0.5, 0},
		{1, 0.5, 0}, {0.5, 1, 0},
		{0.5, 1, 0}, {0, 0.5, 0},
	};

	frame_mesh_.normals_ = {
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_
	};
	frame_mesh_.update_buffers();
}

void Tiling::construct_p3(void)
{
	// Hexagonal lattice.
	t2_relative_ = {0.5, std::sqrt(3) / 2.0};

	mesh_.positions_ = {
		// Left and right side.
		{0, 0, 0}, {1 / 3., 1 / 3., 0}, {0, 1, 0},
		{1, 1, 0}, {2 / 3., 2 / 3., 0}, {1, 0, 0},
		// Bottom and top.
		{1, 0, 0}, {1 / 3., 1 / 3., 0}, {0, 0, 0},
		{0, 1, 0}, {2 / 3., 2 / 3., 0}, {1, 1, 0},
		// Center.
		{0, 1, 0}, {1 / 3., 1 / 3., 0}, {1, 0, 0},
		{1, 0, 0}, {2 / 3., 2 / 3., 0}, {0, 1, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1 / 3., 1 / 3., 0},
		{1, 0, 0}, {1 / 3., 1 / 3., 0},
		{0, 1, 0}, {1 / 3., 1 / 3., 0},

		{1, 1, 0}, {2 / 3., 2 / 3., 0},
		{0, 1, 0}, {2 / 3., 2 / 3., 0},
		{1, 0, 0}, {2 / 3., 2 / 3., 0},
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(12, line_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_p3m1(void)
{
	// Hexagonal lattice.
	t2_relative_ = {0.5, std::sqrt(3) / 2.0};

	mesh_.positions_ = {
		// Bottom and top left.
		{0, 0, 0}, {0.5, 0, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0}, {0, 1, 0},
		// Bottom and top right, mirrored.
		{1, 0, 0}, {0.5, 0, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0}, {1, 1, 0},
		// Left and right top.
		{0, 1, 0}, {0, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0}, {1, 1, 0},
		// Left and right bottom, mirrored.
		{0, 0, 0}, {0, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0}, {1, 0, 0},
		// Bottom center.
		{1, 0, 0}, {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {1, 0, 0},
		// Top center, mirrored.
		{0, 1, 0}, {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_  = {
		{0, 0, 0}, {1, 1, 0},
		{0.5, 0, 0}, {0, 1, 0},
		{0.5, 1, 0}, {1, 0, 0},
		{0, 0.5, 0}, {1, 0, 0},
		{1, 0.5, 0}, {0, 1, 0}
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(10, mirror_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_p31m(void)
{
	// Hexagonal lattice.
	t2_relative_ = {0.5, std::sqrt(3) / 2.0};

	mesh_.positions_ = {
		// Bottom.
		{0, 0, 0},           {0.5, 0, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0, 0},   {1, 0, 0},
		// Left.
		{0, 1, 0},           {0, 0.5, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0, 0.5, 0},   {0, 0, 0},
		// Top, mirrored.
		{0, 1, 0},           {0.5, 1, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0},   {1, 1, 0},
		// Right, mirrored.
		{1, 1, 0},           {1, 0.5, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0},   {1, 0, 0},
		// Center left.
		{1, 0, 0},           {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0},
		// Kepu, mirrored.
		{1, 0, 0},           {0.5, 0.5, 0}, {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},
		{1, 0, 0}, {0, 1, 0},

		{0, 0, 0}, {1 / 3., 1 / 3., 0},
		{1, 0, 0}, {1 / 3., 1 / 3., 0},
		{0, 1, 0}, {1 / 3., 1 / 3., 0},

		{1, 1, 0}, {2 / 3., 2 / 3., 0},
		{0, 1, 0}, {2 / 3., 2 / 3., 0},
		{1, 0, 0}, {2 / 3., 2 / 3., 0}
	};

	frame_mesh_.normals_ = {
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,

		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,

		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_
	};
	frame_mesh_.update_buffers();
}

void Tiling::construct_p4(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Bottom left.
		{0, 0, 0},     {0.5, 0, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0, 0.5, 0}, {0, 0, 0},
		// Top left.
		{0, 1, 0},     {0, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 1, 0}, {0, 1, 0},
		// Top right.
		{1, 1, 0},     {0.5, 1, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {1, 0.5, 0}, {1, 1, 0},
		// Bottom right.
		{1, 0, 0},     {1, 0.5, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.5, 0, 0}, {1, 0, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 0.5, 0}, {1, 0.5, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{0.5, 0, 0}, {0.5, 1, 0},
		{1, 0, 0}, {1, 1, 0}
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(12, line_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_p4m(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Bottom left.
		{0.5, 0.5, 0}, {0.25, 0.25, 0}, {0.5, 0, 0},
		{0.5, 0, 0},   {0.25, 0.25, 0}, {0, 0, 0},
		// Left bottom, mirrored.
		{0.5, 0.5, 0}, {0.25, 0.25, 0}, {0, 0.5, 0},
		{0, 0.5, 0},   {0.25, 0.25, 0}, {0, 0, 0},
		// Left top.
		{0.5, 0.5, 0}, {0.25, 0.75, 0}, {0, 0.5, 0},
		{0, 0.5, 0},   {0.25, 0.75, 0}, {0, 1, 0},
		// Top left, mirrored.
		{0.5, 0.5, 0}, {0.25, 0.75, 0}, {0.5, 1, 0},
		{0.5, 1, 0},   {0.25, 0.75, 0}, {0, 1, 0},
		// Top right.
		{0.5, 0.5, 0}, {0.75, 0.75, 0}, {0.5, 1, 0},
		{0.5, 1, 0},   {0.75, 0.75, 0}, {1, 1, 0},
		// Right top, mirrored.
		{0.5, 0.5, 0}, {0.75, 0.75, 0}, {1, 0.5, 0},
		{1, 0.5, 0},   {0.75, 0.75, 0}, {1, 1, 0},
		// Right bottom.
		{0.5, 0.5, 0}, {0.75, 0.25, 0}, {1, 0.5, 0},
		{1, 0.5, 0},   {0.75, 0.25, 0}, {1, 0, 0},
		// Bottom right, mirrored.
		{0.5, 0.5, 0}, {0.75, 0.25, 0}, {0.5, 0, 0},
		{0.5, 0, 0},   {0.75, 0.25, 0}, {1, 0, 0},
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 0.5, 0}, {1, 0.5, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{0.5, 0, 0}, {0.5, 1, 0},
		{1, 0, 0}, {1, 1, 0},
		{0, 0, 0}, {1, 1, 0},
		{0, 1, 0}, {1, 0, 0}
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(16, mirror_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_p4g(void)
{
	// Square lattice.
	t2_relative_ = {0, 1};

	mesh_.positions_ = {
		// Inner bottom left.
		{0, 0.5, 0},   {0.25, 0.25, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.25, 0.25, 0}, {0.5, 0, 0},
		// Outer bottom left, mirrored.
		{0, 0.5, 0},   {0.25, 0.25, 0}, {0, 0, 0},
		{0, 0, 0},     {0.25, 0.25, 0}, {0.5, 0, 0},
		// Inner top left.
		{0.5, 1, 0},   {0.25, 0.75, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.25, 0.75, 0}, {0, 0.5, 0},
		// Outer top left, mirrored.
		{0.5, 1, 0},   {0.25, 0.75, 0}, {0, 1, 0},
		{0, 1, 0},     {0.25, 0.75, 0}, {0, 0.5, 0},
		// Inner top right.
		{1, 0.5, 0},   {0.75, 0.75, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.75, 0.75, 0}, {0.5, 1, 0},
		// Outer top right, mirrored.
		{1, 0.5, 0},   {0.75, 0.75, 0}, {1, 1, 0},
		{1, 1, 0},     {0.75, 0.75, 0}, {0.5, 1, 0},
		// Inner bottom right.
		{0.5, 0, 0},   {0.75, 0.25, 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0}, {0.75, 0.25, 0}, {1, 0.5, 0},
		// Outer bottom right, mirrored.
		{0.5, 0, 0},   {0.75, 0.25, 0}, {1, 0, 0},
		{1, 0, 0},     {0.75, 0.25, 0}, {1, 0.5, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 0.5, 0}, {1, 0.5, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{0.5, 0, 0}, {0.5, 1, 0},
		{1, 0, 0}, {1, 1, 0},

		{0, 0.5, 0}, {0.5, 0, 0},
		{0.5, 0, 0}, {1, 0.5, 0},
		{1, 0.5, 0}, {0.5, 1, 0},
		{0.5, 1, 0}, {0, 0.5, 0}
	};

	frame_mesh_.normals_ = {
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,

		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_,
		mirror_color_, mirror_color_
	};
	frame_mesh_.update_buffers();
}

void Tiling::construct_p6(void)
{
	// Hexagonal lattice.
	t2_relative_ = {0.5, std::sqrt(3) / 2.0};

	mesh_.positions_ = {
		// Bottom.
		{0, 0, 0},           {0.5, 0, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0, 0},   {1, 0, 0},
		// Left.
		{0, 1, 0},           {0, 0.5, 0},   {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0, 0.5, 0},   {0, 0, 0},
		// Top.
		{1, 1, 0},           {0.5, 1, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 1, 0},   {0, 1, 0},
		// Right.
		{1, 0, 0},           {1, 0.5, 0},   {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {1, 0.5, 0},   {1, 1, 0},
		// Center left.
		{1, 0, 0},           {0.5, 0.5, 0}, {1 / 3., 1 / 3., 0},
		{1 / 3., 1 / 3., 0}, {0.5, 0.5, 0}, {0, 1, 0},
		// Kepu.
		{0, 1, 0},           {0.5, 0.5, 0}, {2 / 3., 2 / 3., 0},
		{2 / 3., 2 / 3., 0}, {0.5, 0.5, 0}, {1, 0, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_ = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},
		{1, 0, 0}, {0, 1, 0},

		{0, 0, 0}, {1 / 3., 1 / 3., 0},
		{1, 0, 0}, {1 / 3., 1 / 3., 0},
		{0, 1, 0}, {1 / 3., 1 / 3., 0},

		{1, 1, 0}, {2 / 3., 2 / 3., 0},
		{0, 1, 0}, {2 / 3., 2 / 3., 0},
		{1, 0, 0}, {2 / 3., 2 / 3., 0}
	};

	frame_mesh_.normals_ = {
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,
		rotation_color_, rotation_color_,

		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_,

		line_color_, line_color_,
		line_color_, line_color_,
		line_color_, line_color_
	};
	frame_mesh_.update_buffers();
}

void Tiling::construct_p6m(void)
{
	// Hexagonal lattice.
	t2_relative_ = {0.5, std::sqrt(3) / 2.0};

	mesh_.positions_ = {
		// Bottom left.
		{1 / 3., 1 / 3., 0}, {1 / 6., 1 / 6., 0}, {0.5, 0, 0},
		{0.5, 0, 0},         {1 / 6., 1 / 6., 0}, {0, 0, 0},
		// Bottom right, mirrored.
		{1 / 3., 1 / 3., 0}, {2 / 3., 1 / 6., 0}, {0.5, 0, 0},
		{0.5, 0, 0},         {2 / 3., 1 / 6., 0}, {1, 0, 0},
		// Left top.
		{1 / 3., 1 / 3., 0}, {1 / 6., 2 / 3., 0}, {0, 0.5, 0},
		{0, 0.5, 0},         {1 / 6., 2 / 3., 0}, {0, 1, 0},
		// Left bottom, mirrored.
		{1 / 3., 1 / 3., 0}, {1 / 6., 1 / 6., 0}, {0, 0.5, 0},
		{0, 0.5, 0},         {1 / 6., 1 / 6., 0}, {0, 0, 0},
		// Top right.
		{2 / 3., 2 / 3., 0}, {5 / 6., 5 / 6., 0}, {0.5, 1, 0},
		{0.5, 1, 0},         {5 / 6., 5 / 6., 0}, {1, 1, 0},
		// Top left, mirrored.
		{2 / 3., 2 / 3., 0}, {1 / 3., 5 / 6., 0}, {0.5, 1, 0},
		{0.5, 1, 0},         {1 / 3., 5 / 6., 0}, {0, 1, 0},
		// Right bottom.
		{2 / 3., 2 / 3., 0}, {5 / 6., 1 / 3., 0}, {1, 0.5, 0},
		{1, 0.5, 0},         {5 / 6., 1 / 3., 0}, {1, 0, 0},
		// Right top, mirrored.
		{2 / 3., 2 / 3., 0}, {5 / 6., 5 / 6., 0}, {1, 0.5, 0},
		{1, 0.5, 0},         {5 / 6., 5 / 6., 0}, {1, 1, 0},
		// Central bottom left.
		{1 / 3., 1 / 3., 0}, {2 / 3., 1 / 6., 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0},       {2 / 3., 1 / 6., 0}, {1, 0, 0},
		// Central bottom right, mirrored.
		{2 / 3., 2 / 3., 0}, {5 / 6., 1 / 3., 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0},       {5 / 6., 1 / 3., 0}, {1, 0, 0},
		// Central top right.
		{2 / 3., 2 / 3., 0}, {1 / 3., 5 / 6., 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0},       {1 / 3., 5 / 6., 0}, {0, 1, 0},
		// Central top left, mirrored.
		{1 / 3., 1 / 3., 0}, {1 / 6., 2 / 3., 0}, {0.5, 0.5, 0},
		{0.5, 0.5, 0},       {1 / 6., 2 / 3., 0}, {0, 1, 0}
	};
	mesh_.update_buffers();

	frame_mesh_.positions_  = {
		{0, 0, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 1, 0},
		{0, 0, 0}, {0, 1, 0},
		{1, 0, 0}, {1, 1, 0},
		{1, 0, 0}, {0, 1, 0},

		{0, 0, 0}, {1, 1, 0},
		{0.5, 0, 0}, {0, 1, 0},
		{0.5, 1, 0}, {1, 0, 0},
		{0, 0.5, 0}, {1, 0, 0},
		{1, 0.5, 0}, {0, 1, 0}
	};

	frame_mesh_.normals_ = std::vector<Eigen::Vector3f>(20, mirror_color_);
	frame_mesh_.update_buffers();
}

void Tiling::construct_symmetry_mesh(void)
{
	symmetry_mesh_ = Mesh();
	for (size_t i = 0; i < mesh_.positions_.size(); i += 3)
	{
		const Eigen::Vector3f& a        = mesh_.positions_[i];
		const Eigen::Vector3f& b        = mesh_.positions_[i+1];
		const Eigen::Vector3f& c        = mesh_.positions_[i+2];
		const Eigen::Vector3f  centroid = (a + b + c) / 3.0f;

		symmetry_mesh_.positions_.push_back(centroid + 1.0f / symmetry_scale_ * (a - centroid));
		symmetry_mesh_.positions_.push_back(centroid + 1.0f / symmetry_scale_ * (b - centroid));
		symmetry_mesh_.positions_.push_back(centroid + 1.0f / symmetry_scale_ * (c - centroid));
	}

	symmetry_mesh_.update_buffers();
}

void Tiling::construct_mesh_texture(void)
{
	std::vector<Eigen::Vector3f> vertices;
	vertices.reserve(num_lattice_domains_ * mesh_.positions_.size());

	int s = std::sqrt(num_lattice_domains_);
	for (int y = 0; y < s; ++y)
	{
		for (int x = 0; x < s; ++x)
		{
			Eigen::Vector3f adjustment = {(float)(x - s/2), (float)(y - s/2), 0};

			for (const auto& vertex : mesh_.positions_)
				vertices.push_back(vertex + adjustment);
		}
	}

	GLint old_arr; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_arr);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices[0].data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, old_arr);

	mesh_texture_ = GL::Texture::buffer_texture(mesh_buffer_, GL_RGB32F);
}
