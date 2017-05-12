#include "Mesh.h"

#include <iostream>
#include <cassert>
#include <cstdio>

#include "GLUtils.h"
using namespace Eigen;

//--------------------

void Mesh::update_buffers(void) {
	glBindVertexArray(vao_);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	if (!positions_.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer_);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * positions_.size(), positions_[0].data(), GL_STATIC_DRAW);

		if (normals_.size() == positions_.size()) {
			glBindBuffer(GL_ARRAY_BUFFER, normal_buffer_);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * normals_.size(), normals_[0].data(), GL_STATIC_DRAW);
		}
		if (texcoords_.size() == positions_.size()) {
			glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer_);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
			glEnableVertexAttribArray(2);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2f) * texcoords_.size(), texcoords_[0].data(), GL_STATIC_DRAW);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	num_vertices_ = positions_.size();
}

Mesh Mesh::from_obj(const char* filename) {
	assert(filename != nullptr);
	FILE* file = fopen(filename, "r");

	if (file == NULL){
		std::cerr << "Couldn't open file " << filename << std::endl;
		throw std::runtime_error("File reading failed.");
	}

	// Temporary objects for reading.
	char type[32]; type[31] = '\0';
	bool read_successful = true;
	unsigned current_line = 0;

	float data[3];
	unsigned v_indices[3];
	unsigned t_indices[3];
	unsigned n_indices[3];

	std::vector<Vector3f> positions;
	std::vector<Vector3f> normals;
	std::vector<Vector2f> texcoords;

	std::vector<unsigned> position_indices;
	std::vector<unsigned> normal_indices;
	std::vector<unsigned> texcoord_indices;

	while (fscanf(file, "%31s", type) != EOF) {
		++current_line;
		read_successful = true;

		if (!strcmp(type, "v")) {
			memset(data, 0, sizeof(data));

			if (3 == fscanf(file, "%f %f %f", &data[0], &data[1], &data[2]))
				positions.emplace_back(data[0], data[1], data[2]);
			else
				read_successful = false;
		}
		else if (!strcmp(type, "vt")) {
			memset(data, 0, sizeof(data));

			if (2 == fscanf(file, "%f %f", &data[0], &data[1]))
				texcoords.emplace_back(data[0], data[1]);
			else
				read_successful = false;
		}
		else if (!strcmp(type, "vn")) {
			memset(data, 0, sizeof(data));

			if (3 == fscanf(file, "%f %f %f", &data[0], &data[1], &data[2]))
				normals.emplace_back(data[0], data[1], data[2]);
			else
				read_successful = false;
		}
		else if (!strcmp(type, "f")) {
			memset(v_indices, 0, sizeof(v_indices));
			memset(t_indices, 0, sizeof(t_indices));
			memset(n_indices, 0, sizeof(n_indices));

			if (3 == fscanf(file, "%u %u %u", &v_indices[0], &v_indices[1], &v_indices[2]))
			{
				for (int i = 0; i < 3; ++i) {
					position_indices.push_back(v_indices[i] - 1);
				}
			}
			else if (5 == fscanf(file, "/%u %u/%u %u/%u", &t_indices[0],
								      &v_indices[1], &t_indices[1],
								      &v_indices[2], &t_indices[2]))
			{
				for (int i = 0; i < 3; ++i) {
					position_indices.push_back(v_indices[i] - 1);
					texcoord_indices.push_back(t_indices[i] - 1);
				}
			}
			else if (7 == fscanf(file, "/%u %u/%u/%u %u/%u/%u", &n_indices[0],
									    &v_indices[1], &t_indices[1], &n_indices[1],
									    &v_indices[2], &t_indices[2], &n_indices[2]))
			{
				for (int i = 0; i < 3; ++i) {
					position_indices.push_back(v_indices[i] - 1);
					texcoord_indices.push_back(t_indices[i] - 1);
					normal_indices.push_back(n_indices[i] - 1);
				}
			}
			else if (3 == fscanf(file, "/%u %u//%u", &n_indices[1],
								  &v_indices[2], &n_indices[2]))
			{
				for (int i = 0; i < 3; ++i) {
					position_indices.push_back(v_indices[i] - 1);
					normal_indices.push_back(n_indices[i] - 1);
				}
			}
			else
				read_successful = false;
		}

		if (!read_successful)
			break;
	}

	fclose(file);

	if (!read_successful) {
		std::cerr << "File reading failed at line " << current_line << "." << std::endl;
		throw std::runtime_error("File reading failed.");
	}

	// Reading done, generate mesh.
	Mesh mesh;

	mesh.positions_.resize(position_indices.size());
	mesh.normals_.resize(normal_indices.size());
	mesh.texcoords_.resize(texcoord_indices.size());

	for (size_t i = 0; i < position_indices.size(); ++i)
		mesh.positions_[i] = positions[position_indices[i]];
	for (size_t i = 0; i < normal_indices.size(); ++i)
		mesh.normals_[i] = normals[normal_indices[i]];
	for (size_t i = 0; i < texcoord_indices.size(); ++i)
		mesh.texcoords_[i] = texcoords[texcoord_indices[i]];

	mesh.update_buffers();
	mesh.primitive_type_ = GL_TRIANGLES;

	return mesh;
}

Mesh Mesh::cube(void) {
	static const Vector3f vertices[] = {
		Vector3f(-1, -1,  1),   Vector3f( 0,  0,  1),
		Vector3f( 1, -1,  1),   Vector3f( 0,  0,  1),
		Vector3f( 1,  1,  1),   Vector3f( 0,  0,  1),

		Vector3f(-1, -1,  1),   Vector3f( 0,  0,  1),
		Vector3f( 1,  1,  1),   Vector3f( 0,  0,  1),
		Vector3f(-1,  1,  1),   Vector3f( 0,  0,  1),

		Vector3f( 1, -1, -1),   Vector3f( 0,  0, -1),
		Vector3f(-1, -1, -1),   Vector3f( 0,  0, -1),
		Vector3f(-1,  1, -1),   Vector3f( 0,  0, -1),

		Vector3f( 1, -1, -1),   Vector3f( 0,  0, -1),
		Vector3f(-1,  1, -1),   Vector3f( 0,  0, -1),
		Vector3f( 1,  1, -1),   Vector3f( 0,  0, -1),

		Vector3f(-1, -1, -1),   Vector3f(-1,  0,  0),
		Vector3f(-1, -1,  1),   Vector3f(-1,  0,  0),
		Vector3f(-1,  1,  1),   Vector3f(-1,  0,  0),

		Vector3f(-1, -1, -1),   Vector3f(-1,  0,  0),
		Vector3f(-1,  1,  1),   Vector3f(-1,  0,  0),
		Vector3f(-1,  1, -1),   Vector3f(-1,  0,  0),

		Vector3f( 1, -1,  1),   Vector3f( 1,  0,  0),
		Vector3f( 1, -1, -1),   Vector3f( 1,  0,  0),
		Vector3f( 1,  1, -1),   Vector3f( 1,  0,  0),

		Vector3f( 1, -1,  1),   Vector3f( 1,  0,  0),
		Vector3f( 1,  1, -1),   Vector3f( 1,  0,  0),
		Vector3f( 1,  1,  1),   Vector3f( 1,  0,  0),

		Vector3f(-1, -1, -1),   Vector3f( 0, -1,  0),
		Vector3f( 1, -1, -1),   Vector3f( 0, -1,  0),
		Vector3f( 1, -1,  1),   Vector3f( 0, -1,  0),

		Vector3f(-1, -1, -1),   Vector3f( 0, -1,  0),
		Vector3f( 1, -1,  1),   Vector3f( 0, -1,  0),
		Vector3f(-1, -1,  1),   Vector3f( 0, -1,  0),

		Vector3f(-1,  1,  1),   Vector3f( 0,  1,  0),
		Vector3f( 1,  1,  1),   Vector3f( 0,  1,  0),
		Vector3f( 1,  1, -1),   Vector3f( 0,  1,  0),

		Vector3f(-1,  1,  1),   Vector3f( 0,  1,  0),
		Vector3f( 1,  1, -1),   Vector3f( 0,  1,  0),
		Vector3f(-1,  1, -1),   Vector3f( 0,  1,  0)
	};

	static const Vector2f texcoords[] = {
		Vector2f(0, 0), Vector2f(1, 0), Vector2f(1, 1),
		Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1),

		Vector2f(0, 0), Vector2f(1, 0), Vector2f(1, 1),
		Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1),

		Vector2f(0, 0), Vector2f(1, 0), Vector2f(1, 1),
		Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1),

		Vector2f(0, 0), Vector2f(1, 0), Vector2f(1, 1),
		Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1),

		Vector2f(0, 0), Vector2f(1, 0), Vector2f(1, 1),
		Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1),

		Vector2f(0, 0), Vector2f(1, 0), Vector2f(1, 1),
		Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1)
	};

	Mesh mesh;

	std::vector<Vector3f>& position_buffer_data = mesh.positions_;
	std::vector<Vector3f>& normal_buffer_data = mesh.normals_;
	std::vector<Vector2f>& texcoord_buffer_data = mesh.texcoords_;

	size_t size = sizeof(vertices) / sizeof(vertices[0]) / 2;
	position_buffer_data.resize(size);
	normal_buffer_data.resize(size);
	texcoord_buffer_data.resize(size);

	for (size_t i = 0; i < size; ++i) {
		position_buffer_data[i]	 = vertices[2*i];
		normal_buffer_data[i]	   = vertices[2*i + 1];
		texcoord_buffer_data[i]	 = texcoords[i];
	}

	// Store in the VAO all the info necessary for drawing sequential vertices.
	glBindVertexArray(mesh.vao_);

	// Bind the VBO to store the cube's vertices.
	glBindBuffer(GL_ARRAY_BUFFER, mesh.position_buffer_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * position_buffer_data.size(), position_buffer_data[0].data(), GL_STATIC_DRAW);

	// Bind the VBO to store the cube's normals.
	glBindBuffer(GL_ARRAY_BUFFER, mesh.normal_buffer_);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * normal_buffer_data.size(), normal_buffer_data[0].data(), GL_STATIC_DRAW);

	// Bind the VBO to store the cube's texcoords.
	glBindBuffer(GL_ARRAY_BUFFER, mesh.texcoord_buffer_);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords[0].data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Set the rest of the info necessary for drawing the cube.
	mesh.num_vertices_ = size;
	mesh.primitive_type_ = GL_TRIANGLES;

	return mesh;
}

Mesh Mesh::torus(float major_radius, float minor_radius, size_t major_steps, size_t minor_steps) {
	Mesh mesh;

	float major_angle_incr = 2 * PI / major_steps;
	float minor_angle_incr = 2 * PI / minor_steps;

	std::vector<Vector3f> vertices;
	for (size_t i = 0; i < major_steps; ++i) {
		float major_angle  = i * major_angle_incr;

		// Stetson-Harrison
		Vector3f major_dir = Vector3f(cosf(PI/2 + major_angle + 0.89 * major_angle_incr),
		                              0.0f,
		                              sinf(PI/2 + major_angle + 0.89 * major_angle_incr));

		Vector3f center    = major_radius * major_dir;

		for (size_t j = 0; j < minor_steps; ++j) {
			float minor_angle  = j * minor_angle_incr;
			Vector3f minor_dir = cosf(PI / 2 + minor_angle) * major_dir
			                   + sinf(PI / 2 + minor_angle) * Vector3f(0.0f, 1.0f, 0.0f);
			vertices.push_back(center + minor_radius * minor_dir);
		}
	}

	for (size_t i = 0; i < major_steps - 1; ++i) {
		for (size_t j = 0; j < minor_steps - 1; ++j) {
			Vector3f& a = vertices[j + minor_steps * i];
			Vector3f& b = vertices[(j + 1) + minor_steps * i];
			Vector3f& c = vertices[(j + 1) + minor_steps * (i + 1)];

			Vector3f& d = a;
			Vector3f& e = vertices[(j + 1) + minor_steps * (i + 1)];
			Vector3f& f = vertices[j + minor_steps * (i + 1)];

			Vector3f n = (b - a).cross(c - a);

			mesh.positions_.push_back(a);
			mesh.texcoords_.emplace_back(1.0f, 0.0f);

			mesh.positions_.push_back(b);
			mesh.texcoords_.emplace_back(1.0f, 1.0f);

			mesh.positions_.push_back(c);
			mesh.texcoords_.emplace_back(0.0f, 1.0f);


			mesh.positions_.push_back(d);
			mesh.texcoords_.emplace_back(1.0f, 0.0f);

			mesh.positions_.push_back(e);
			mesh.texcoords_.emplace_back(0.0f, 1.0f);

			mesh.positions_.push_back(f);
			mesh.texcoords_.emplace_back(0.0f, 0.0f);

			for (int k = 0; k < 6; ++k)
				mesh.normals_.push_back(n);
		}

		// Last two triangles in every ring.
		Vector3f& a = vertices[-1 + minor_steps * (i + 1)];
		Vector3f& b = vertices[minor_steps * i];
		Vector3f& c = vertices[minor_steps * (i + 1)];

		Vector3f& d = a;
		Vector3f& e = vertices[minor_steps * (i + 1)];
		Vector3f& f = vertices[-1 + minor_steps * (i + 2)];

		Vector3f n = (b - a).cross(c - a);

		mesh.positions_.push_back(a);
		mesh.texcoords_.emplace_back(1.0f, 0.0f);

		mesh.positions_.push_back(b);
		mesh.texcoords_.emplace_back(1.0f, 1.0f);

		mesh.positions_.push_back(c);
		mesh.texcoords_.emplace_back(0.0f, 1.0f);


		mesh.positions_.push_back(d);
		mesh.texcoords_.emplace_back(1.0f, 0.0f);

		mesh.positions_.push_back(e);
		mesh.texcoords_.emplace_back(0.0f, 1.0f);

		mesh.positions_.push_back(f);
		mesh.texcoords_.emplace_back(0.0f, 0.0f);

		for (int k = 0; k < 6; ++k)
			mesh.normals_.push_back(n);
	}

	// Last ring.
	size_t last_ring = minor_steps * (major_steps - 1);
	for (size_t j = 0; j < minor_steps - 1; ++j) {
		Vector3f& a = vertices[j + last_ring];
		Vector3f& b = vertices[(j + 1) + last_ring];
		Vector3f& c = vertices[(j + 1)];

		Vector3f& d = a;
		Vector3f& e = vertices[(j + 1)];
		Vector3f& f = vertices[j];

		const Vector3f& n = (b - a).cross(c - a);

		mesh.positions_.push_back(a);
		mesh.texcoords_.emplace_back(1.0f, 0.0f);

		mesh.positions_.push_back(b);
		mesh.texcoords_.emplace_back(1.0f, 1.0f);

		mesh.positions_.push_back(c);
		mesh.texcoords_.emplace_back(0.0f, 1.0f);


		mesh.positions_.push_back(d);
		mesh.texcoords_.emplace_back(1.0f, 0.0f);

		mesh.positions_.push_back(e);
		mesh.texcoords_.emplace_back(0.0f, 1.0f);

		mesh.positions_.push_back(f);
		mesh.texcoords_.emplace_back(0.0f, 0.0f);

		for (int i = 0; i < 6; ++i)
			mesh.normals_.push_back(n);
	}

	// And the last two triangles in the last ring.
	Vector3f& a = vertices[-1 + last_ring + minor_steps];
	Vector3f& b = vertices[last_ring];
	Vector3f& c = vertices[0];

	Vector3f& d = a;
	Vector3f& e = vertices[0];
	Vector3f& f = vertices[-1 + minor_steps];

	const Vector3f& n = (b - a).cross(c - a);

	mesh.positions_.push_back(a);
	mesh.texcoords_.emplace_back(1.0f, 0.0f);

	mesh.positions_.push_back(b);
	mesh.texcoords_.emplace_back(1.0f, 1.0f);

	mesh.positions_.push_back(c);
	mesh.texcoords_.emplace_back(0.0f, 1.0f);


	mesh.positions_.push_back(d);
	mesh.texcoords_.emplace_back(1.0f, 0.0f);

	mesh.positions_.push_back(e);
	mesh.texcoords_.emplace_back(0.0f, 1.0f);

	mesh.positions_.push_back(f);
	mesh.texcoords_.emplace_back(0.0f, 0.0f);

	for (int i = 0; i < 6; ++i)
		mesh.normals_.push_back(n);

	mesh.update_buffers();
	mesh.primitive_type_ = GL_TRIANGLES;

	return mesh;
}
