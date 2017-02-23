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
	const Eigen::Vector2f& position           (void) const { return position_; }
	const Eigen::Vector2f& translation_1      (void) const { return t1_; }
	const Eigen::Vector2f& translation_2      (void) const { return t1_; }

	// TODO: Symmetry group parameters such as lattice angle etc.
	void set_symmetry_group (const char*);
	void set_position       (const Eigen::Vector2f&);
	void set_rotation       (double);

	// This function constructs the symmetrified texture according to
	// the current symmetry group.
	void symmetrify (const GL::Texture&);

private:
	// Mesh construction functions for different symmetry groups.
	void construct_p1(void);
	void construct_cmm(void);

	// The vertices of the mesh are defined relative to the translation vectors.
	Eigen::Vector2f position_;
	Eigen::Vector2f t1_, t2_;
	const char*     symmetry_group_;

	// This shader is used for building the symmetrified fundamental domain.
	// It practically superimposes samples of the user-supplied texture in
	// such a way that the resulting fundamental domain conforms to the chosen
	// symmetry group.
	GL::ShaderProgram symmetrify_shader_;
	GLuint aspect_ratio_uniform_;
	GLuint position_uniform_;
	GLuint t1_uniform_;
	GLuint t2_uniform_;

	// This texture will contain the symmetrified fundamental domain.
	GL::Texture symmetrified_;
	GL::FBO     fbo_;

	// We only need one mesh. It will be unrenderable in its base state
	// because of mirrored triangles, but we can write a geometry shader to correct
	// this when rendering. When symmetrifying, the mirroring will be essential.
	Mesh mesh_;
};

#endif // TILING_H
