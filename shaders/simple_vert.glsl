#version 330

// This is a vertex shader. It is executed once for every vertex in the mesh.
// Afterwards, the output values are interpolated for every fragment affected
// by this vertex, i.e. the fragments (pixels, roughly) within the triangle to
// which this vertex belongs.

// We'll color each vertex with one of 6 colours based on its index.
// These colours will then be linearly interpolated within the triangles.
// We'll also transform the vertices into clip space.

//--------------------

// Declare the inputs.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// Declare the uniforms.
uniform mat4 uModelToClip = mat4(1.0);
uniform mat3 uNormalToWorld = mat3(1.0);

// Declare the outputs.
// These will go to the fragment shader, interpolated automagically.
out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

void main() {
	const vec3 distinctColors[6] = vec3[6](vec3(0, 0, 1), vec3(0, 1, 0), vec3(0, 1, 1),
	                                       vec3(1, 0, 0), vec3(1, 0, 1), vec3(1, 1, 0));

	vColor = distinctColors[gl_VertexID % 6];
	vNormal = normalize(uNormalToWorld * aNormal);
	vTexCoord = aTexCoord;

	gl_Position = uModelToClip * vec4(aPosition, 1.0);
}
