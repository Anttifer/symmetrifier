#include "ShaderCanvas.h"

#include <Eigen/Geometry>
using namespace Eigen;

//--------------------

ShaderCanvas::ShaderCanvas(void) {
	const Vector3f vertices[] = {
		Vector3f(-1, -1, 0), Vector3f(1, -1, 0), Vector3f( 1, 1, 0),
		Vector3f(-1, -1, 0), Vector3f(1,  1, 0), Vector3f(-1, 1, 0)
	};

	const Vector2f texcoords[] = {
		Vector2f(0, 0), Vector2f(1, 0), Vector2f(1, 1),
		Vector2f(0, 0), Vector2f(1, 1), Vector2f(0, 1)
	};

	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, position_buffer_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer_);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	num_vertices_ = sizeof(vertices) / sizeof(vertices[0]);
	primitive_type_ = GL_TRIANGLES;
}
