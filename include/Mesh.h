#ifndef MESH_H
#define MESH_H

#include "GLObjects.h"
#include <Eigen/Geometry>
#include <vector>

//--------------------

class Mesh {
public:
	Mesh (void) : num_vertices_(0), primitive_type_(GL_TRIANGLES) {}

	void update_buffers (void);

	static Mesh from_obj (const char* filename);
	static Mesh cube     (void);
	static Mesh torus    (float major_radius, float minor_radius,
	                      size_t major_steps, size_t minor_steps);
public:
	std::vector<Eigen::Vector3f> positions_;
	std::vector<Eigen::Vector3f> normals_;
	std::vector<Eigen::Vector2f> texcoords_;

	GL::Buffer position_buffer_;
	GL::Buffer normal_buffer_;
	GL::Buffer texcoord_buffer_;

	GL::VAO vao_;

	size_t num_vertices_;
	GLenum primitive_type_;
};

#endif // MESH_H
