#ifndef TILING_H
#define TILING_H

#include "GLObjects.h"
#include "Mesh.h"
#include <Eigen/Geometry>

class Tiling {
public:
	Tiling (void);

	const char*            symmetry_group     (void) const { return symmetry_group_; }
	const Mesh&            mesh               (void) const { return mesh_; }
	const GL::Texture&     domain_texture     (void) const { return domain_texture_; }
	const Eigen::Vector2f& position           (void) const { return position_; }
	const Eigen::Vector2f& t1                 (void) const { return t1_; }
	const Eigen::Vector2f& t2                 (void) const { return t2_; }
	bool                   consistent         (void) const { return consistent_; }

	// TODO: Symmetry group parameters such as lattice angle etc.
	void set_symmetry_group (const char*);
	void set_position       (const Eigen::Vector2f& position) { consistent_ = false; position_ = position; }
	void set_rotation       (double);
	void set_scale          (double factor);
	void set_inconsistent   (void)                            { consistent_ = false; }

	// This function constructs the symmetrified texture according to
	// the current symmetry group.
	void symmetrify (const GL::Texture&, int num_domains = 1);

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

	// The vertices of the mesh are defined relative to the translation vectors.
	Eigen::Vector2f position_;
	Eigen::Vector2f t1_, t2_;
	const char*     symmetry_group_;

	// This shader is used for building the symmetrified fundamental domain.
	// It practically superimposes samples of the user-supplied texture in
	// such a way that the resulting fundamental domain conforms to the chosen
	// symmetry group.
	GL::ShaderProgram symmetrify_shader_;
	GLuint instance_num_uniform_;
	GLuint aspect_ratio_uniform_;
	GLuint position_uniform_;
	GLuint t1_uniform_;
	GLuint t2_uniform_;
	GLuint sampler_uniform_;

	// This texture will contain the symmetrified fundamental domain.
	GL::Texture domain_texture_;

	// We only need one mesh. It will be unrenderable in its base state
	// because of mirrored triangles, but we can write a geometry shader to correct
	// this when rendering. When symmetrifying, the mirroring will be essential.
	Mesh mesh_;

	// Is the domain texture consistent with current settings?
	bool consistent_;
};

#endif // TILING_H
