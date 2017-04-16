#ifndef TILING_H
#define TILING_H

#include "GLObjects.h"
#include "Mesh.h"
#include <Eigen/Geometry>

class Tiling
{
public:
	Tiling          (void);
	explicit Tiling (float symmetry_scale);

	// Lattice types.
	enum class Lattice
	{
		Oblique,
		Rhombic,
		Rectangular,
		Square,
		Hexagonal
	};

	// Properties.
	const char* symmetry_group       (void) const { return symmetry_group_; }
	Lattice     lattice              (void) const { return lattice_; }
	int         num_lattice_domains  (void) const { return num_lattice_domains_; }
	int         num_symmetry_domains (void) const { return mesh_.positions_.size() / 6 * num_lattice_domains_; }
	bool        consistent           (void) const { return consistent_; }

	// Transformations.
	const Eigen::Vector2f& position (void) const { return position_; }
	Eigen::Vector2f        center   (void) const;

	const Eigen::Vector2f& t1       (void) const { return t1_; }
	Eigen::Vector2f        t2       (void) const;

	double                 rotation (void) const;
	double                 scale    (void) const { return t1_.norm(); }

	// Base image transformations.
	const Eigen::Vector2f& image_position (void) const { return image_position_; }
	Eigen::Vector2f        image_center   (void) const;

	const Eigen::Vector2f& image_t1       (void) const { return image_t1_; }
	Eigen::Vector2f        image_t2       (void) const;

	double                 image_rotation (void) const;
	double                 image_scale    (void) const { return image_t1_.norm(); }

	// Meshes.
	const Mesh& mesh          (void) const { return mesh_; }
	const Mesh& frame         (void) const { return frame_mesh_; }
	const Mesh& symmetry_mesh (void) const { return symmetry_mesh_; }

	// Textures & coordinates.
	const GL::Texture& base_image         (void) const { return base_image_; }
	const GL::Texture& domain_texture     (void) const { return domain_texture_; }
	const GL::Texture& mesh_texture       (void) const { return mesh_texture_; }
	const std::vector<Eigen::Vector2f>&
	                   domain_coordinates (void) const { return domain_coordinates_; }

	// Set properties.
	void set_symmetry_group      (const char*);
	void set_num_lattice_domains (int n);
	void set_inconsistent        (void) { consistent_ = false; }

	// Set transformations.
	void set_position   (const Eigen::Vector2f& p) { consistent_ = false; position_ = p; }
	void set_center     (const Eigen::Vector2f&);

	void set_t1         (const Eigen::Vector2f&);
	void set_t2         (const Eigen::Vector2f&);

	void set_rotation   (double);
	void set_scale      (double);
	void multiply_scale (double factor);

	// Set base image transformations.
	void set_image_position   (const Eigen::Vector2f& p) { consistent_ = false; image_position_ = p; }
	void set_image_center     (const Eigen::Vector2f&);

	void set_image_t1         (const Eigen::Vector2f&);

	void set_image_rotation   (double);
	void set_image_scale      (double);
	void multiply_image_scale (double factor);

	void set_base_image (GL::Texture&&);

	// Intuitive lattice domain deformations.
	void set_deform_origin       (const Eigen::Vector2f&);
	void deform                  (const Eigen::Vector2f&);

	// This function constructs the symmetrified texture according to
	// the current symmetry group.
	void symmetrify (void);

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


	// Auxiliary mesh construction functions.
	void construct_symmetry_mesh  (void);
	void construct_mesh_texture   (void);


	// Properties.
	const char* symmetry_group_;
	Lattice     lattice_;

	// How many lattice domains to take into account when building
	// the domain texture?
	int         num_lattice_domains_;

	// Is the domain texture consistent with current settings?
	bool        consistent_;


	// Transformations.
	Eigen::Vector2f position_;
	Eigen::Vector2f t1_;

	// The second translation vector is defined relative to the first.
	Eigen::Vector2f t2_relative_;


	// Image transformations.
	Eigen::Vector2f image_position_;
	Eigen::Vector2f image_t1_;


	// Meshes.

	// The mesh representation of one lattice domain. Triangles are defined clockwise
	// or counterclockwise depending on whether they should be mirrored or not.
	// The vertices are defined relative to the translation vectors.
	Mesh mesh_;

	// Line mesh for visualizing the lattice domain.
	// Lines that are mirrors or 180-degree flips are coloured separately.
	Mesh frame_mesh_;
	Eigen::Vector3f line_color_;
	Eigen::Vector3f mirror_color_;
	Eigen::Vector3f rotation_color_;

	// This mesh is used internally for building the domain texture.
	// In this mesh, each triangle is rescaled with respect to its centroid.
	// This is reflected in the domain_coordinates_ for proper sampling.
	// This is necessary in order to avoid ugly seams when rendering.
	float symmetry_scale_;
	Mesh  symmetry_mesh_;


	// Textures & coordinates.

	GL::Texture                  base_image_;

	// This texture will contain the symmetrified fundamental domain.
	GL::Texture                  domain_texture_;

	// This is a buffer texture and will contain the sampling mesh.
	GL::Texture                  mesh_texture_;
	GL::Buffer                   mesh_buffer_;

	// We only need texture coordinates for two triangles, so no need to define them
	// in the mesh for every vertex separately.
	// TODO: We don't need a duplicate of these in every tiling - same as the shader below.
	std::vector<Eigen::Vector2f> domain_coordinates_;


	// This shader is used for building the symmetrified fundamental domain.
	// It practically superimposes samples of the user-supplied texture in
	// such a way that the resulting fundamental domain conforms to the chosen
	// symmetry group.
	// TODO: We really don't need a copy of this shader for every tiling - do something.
	GL::ShaderProgram symmetrify_shader_;
	struct Uniforms
	{
		GLint num_instances;
		GLint position;
		GLint t1;
		GLint t2;
		GLint image_position;
		GLint image_t1;
		GLint image_t2;
		GLint sampler;
	}
	uniforms_;


	// Variables used for intuitive deformations.
	Eigen::Vector2f deform_origin_;
	Eigen::Vector2f deform_original_t1_;
	Eigen::Vector2f deform_original_t2_;

	Eigen::Vector2f deform_quadrant_;
	Eigen::Vector2f deform_corner_;
};

#endif // TILING_H
