#version 330

//--------------------

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
} vIn[];

out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
} vOut;

void main() {
	// The texture coordinates depend on whether we're dealing with triangle
	// type 1 or 2. These triangles alternate by convention.
	mat2 coordLookup;
	if (gl_PrimitiveIDIn % 2 = 0)
		coordLookup = mat2(vec2(1, 0), vec2(1, 1));
	else
		coordLookup = mat2(vec2(1, 1), vec2(0, 1));

	// First vertex is always unchanged.
	gl_Position    = gl_in[0].gl_Position;
	vOut.vColor    = vIn[0].vColor;
	vOut.vNormal   = vIn[0].vNormal;
	vOut.vTexCoord = vec2(0, 0);
	EmitVertex();

	// Check whether the triangle is flipped or not.
	vec3 v1 = gl_in[1].gl_Position - gl_in[0].gl_Position;
	vec3 v2 = gl_in[2].gl_Position - gl_in[0].gl_Position;

	if(v1.cross(v2).z < 0) {
		gl_Position    = gl_in[2].gl_Position;
		vOut.vColor    = vIn[2].vColor;
		vOut.vNormal   = vIn[2].vNormal;
		vOut.vTexCoord = coordLookup[1];
		EmitVertex();

		gl_Position    = gl_in[1].gl_Position;
		vOut.vColor    = vIn[1].vColor;
		vOut.vNormal   = vIn[1].vNormal;
		vOut.vTexCoord = coordLookup[0];
		EmitVertex();
	}
	else {
		gl_Position    = gl_in[1].gl_Position;
		vOut.vColor    = vIn[1].vColor;
		vOut.vNormal   = vIn[1].vNormal;
		vOut.vTexCoord = coordLookup[0];
		EmitVertex();

		gl_Position    = gl_in[2].gl_Position;
		vOut.vColor    = vIn[2].vColor;
		vOut.vNormal   = vIn[2].vNormal;
		vOut.vTexCoord = coordLookup[1];
		EmitVertex();
	}

	EndPrimitive();
}
