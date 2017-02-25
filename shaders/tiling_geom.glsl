#version 330

//--------------------

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec2 uPos = vec2(0, 0);
uniform vec2 uT1  = vec2(1, 0);
uniform vec2 uT2  = vec2(0, 1);

uniform ivec2 uScreenSize;
uniform vec2  uScreenCenter;
uniform float uPixelsPerUnit;

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

vec2 NDCPosition(in vec4 pos) {
	vec2 worldPos = uPos + uT1 * pos.x + uT2 * pos.y;
	return (worldPos - uScreenCenter) * vec2(uPixelsPerUnit) / (0.5 * uScreenSize);
}

mat3x2 coordLookupScaled(in float scale) {
	vec2 ccentroid; // centroid is a reserved word.
	mat3x2 matrix;

	// The texture coordinates depend on whether we're dealing with triangle
	// type 1 or 2. These triangles alternate by convention.
	if (gl_PrimitiveIDIn % 2 == 0)
	{
		ccentroid = vec2(2.0 / 3.0, 1.0 / 3.0);
		matrix[0] = scale * (vec2(0, 0) - ccentroid) + ccentroid;
		matrix[1] = scale * (vec2(1, 0) - ccentroid) + ccentroid;
		matrix[2] = scale * (vec2(1, 1) - ccentroid) + ccentroid;
	}
	else {
		ccentroid = vec2(1.0 / 3.0, 2.0 / 3.0);
		matrix[0] = scale * (vec2(0, 0) - ccentroid) + ccentroid;
		matrix[1] = scale * (vec2(1, 1) - ccentroid) + ccentroid;
		matrix[2] = scale * (vec2(0, 1) - ccentroid) + ccentroid;
	}

	return matrix;
}

void main() {
	const float scaling_factor = 0.98;
	mat3x2 coordLookup = coordLookupScaled(scaling_factor);

	// First vertex is always unchanged.
	gl_Position    = vec4(NDCPosition(gl_in[0].gl_Position), 0, 1);
	vOut.vColor    = vIn[0].vColor;
	vOut.vNormal   = vIn[0].vNormal;
	vOut.vTexCoord = coordLookup[0];
	EmitVertex();

	// Check whether the triangle is flipped or not.
	vec3 v1 = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
	vec3 v2 = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;

	if(cross(v1, v2).z < 0) {
		gl_Position    = vec4(NDCPosition(gl_in[2].gl_Position), 0, 1);
		vOut.vColor    = vIn[2].vColor;
		vOut.vNormal   = vIn[2].vNormal;
		vOut.vTexCoord = coordLookup[2];
		EmitVertex();

		gl_Position    = vec4(NDCPosition(gl_in[1].gl_Position), 0, 1);
		vOut.vColor    = vIn[1].vColor;
		vOut.vNormal   = vIn[1].vNormal;
		vOut.vTexCoord = coordLookup[1];
		EmitVertex();
	}
	else {
		gl_Position    = vec4(NDCPosition(gl_in[1].gl_Position), 0, 1);
		vOut.vColor    = vIn[1].vColor;
		vOut.vNormal   = vIn[1].vNormal;
		vOut.vTexCoord = coordLookup[1];
		EmitVertex();

		gl_Position    = vec4(NDCPosition(gl_in[2].gl_Position), 0, 1);
		vOut.vColor    = vIn[2].vColor;
		vOut.vNormal   = vIn[2].vNormal;
		vOut.vTexCoord = coordLookup[2];
		EmitVertex();
	}

	EndPrimitive();
}
