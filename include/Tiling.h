#ifndef TILING_H
#define TILING_H

#include "GLObjects.h"
#include "Mesh.h"
#include <Eigen/Geometry>

class Tiling {
public:
	Tiling (void);

	// Lattice types.
	enum class Lattice {
		Oblique,
		Rhombic,
		Rectangular,
		Square,
		Hexagonal
	};

	const char*            symmetry_group       (void) const { return symmetry_group_; }
	Lattice                lattice              (void) const { return lattice_; }
	const Mesh&            mesh                 (void) const { return mesh_; }
	const Mesh&            frame                (void) const { return frame_mesh_; }
	const std::vector<Eigen::Vector2f>&
	                       domain_coordinates   (void) const { return domain_coordinates_; }
	const GL::Texture&     domain_texture       (void) const { return domain_texture_; }
	const GL::Texture&     mesh_texture         (void) const { return mesh_texture_; }
	const Eigen::Vector2f& position             (void) const { return position_; }
	Eigen::Vector2f        center               (void) const;
	double                 rotation             (void) const;
	double                 scale                (void) const { return t1_.norm(); }
	const Eigen::Vector2f& t1                   (void) const { return t1_; }
	Eigen::Vector2f        t2                   (void) const;
	int                    num_lattice_domains  (void) const { return num_lattice_domains_; }
	int                    num_symmetry_domains (void) const { return mesh_.positions_.size() / 6 * num_lattice_domains_; }
	bool                   consistent           (void) const { return consistent_; }

	// TODO: Symmetry group parameters such as lattice angle etc.
	void set_symmetry_group      (const char*);
	void set_position            (const Eigen::Vector2f& p) { consistent_ = false; position_ = p; }
	void set_center              (const Eigen::Vector2f&);
	void set_rotation            (double);
	void set_scale               (double);
	void multiply_scale          (double factor);
	void set_num_lattice_domains (int n);
	void set_inconsistent        (void)                     { consistent_ = false; }

	// This function constructs the symmetrified texture according to
	// the current symmetry group.
	void symmetrify (const GL::Texture&);

private:
	// Mesh construction functions for the different symmetry groups.
	void construct_p1   (void);
	void construct_pm   (void);
	void construct_cm   (void);
	void construct_pg   (void);

	void construct_p2   (void);
	void construct_pmm  (void);
	void construct_pmg  (void);
	void construct_cmm  (void);
	void construct_pgg  (void);

	void construct_p3   (void);
	void construct_p3m1 (void);
	void construct_p31m (void);

	void construct_p4   (void);
	void construct_p4m  (void);
	void construct_p4g  (void);

	void construct_p6   (void);
	void construct_p6m  (void);

	void construct_symmetry_mesh  (void);
	void construct_mesh_texture (void);

	// The vertices of the mesh are defined relative to the translation vectors.
	Eigen::Vector2f position_;
	Eigen::Vector2f t1_;

	// The second translation vector is defined relative to the first.
	// This allows us to deform the fundamental domain in a controlled fashion
	// when it is possible, i.e. when we have a non-hexagonal and non-square lattice.
	Eigen::Vector2f t2_relative_;

	int             num_lattice_domains_;
	const char*     symmetry_group_;
	Lattice         lattice_;

	// This shader is used for building the symmetrified fundamental domain.
	// It practically superimposes samples of the user-supplied texture in
	// such a way that the resulting fundamental domain conforms to the chosen
	// symmetry group.
	GL::ShaderProgram symmetrify_shader_;
	GLuint num_instances_uniform_;
	GLuint aspect_ratio_uniform_;
	GLuint position_uniform_;
	GLuint t1_uniform_;
	GLuint t2_uniform_;
	GLuint sampler_uniform_;

	// This texture will contain the symmetrified fundamental domain.
	GL::Texture domain_texture_;

	// This is a buffer texture and will contain the sampling mesh.
	GL::Texture mesh_texture_;
	GL::Buffer  mesh_buffer_;

	// The mesh representation of one lattice domain. Triangles are defined clockwise
	// or counterclockwise depending on whether they should be mirrored or not.
	Mesh mesh_;

	// We only need texture coordinates for two triangles, so no need to define them
	// in the mesh for every vertex separately.
	std::vector<Eigen::Vector2f> domain_coordinates_;

	// This is used internally for symmetrifying. Each triangle is rescaled
	// with respect to its centroid.
	Mesh symmetry_mesh_;

	// Not really. We need two.
	Mesh frame_mesh_;
	Eigen::Vector3f line_color_;
	Eigen::Vector3f mirror_color_;
	Eigen::Vector3f rotation_color_;

	// Is the domain texture consistent with current settings?
	bool consistent_;
};

#endif // TILING_H
