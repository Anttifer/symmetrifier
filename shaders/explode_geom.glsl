#version 330

//--------------------

layout(triangles) in;
layout(triangle_strip, max_vertices = 11) out;

uniform mat4 uModelToClip   = mat4(1.0);
uniform mat3 uNormalToWorld = mat3(1.0);
uniform float uTime;

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
	float m = (1.0 + sin(3*uTime)) / 2.0;

	int max = (gl_PrimitiveIDIn % 2 == 1) ? 4 : 3;
	for (int ii = 0; ii < max; ++ii) {
		int i = ii % 3;

		vOut.vColor    = vIn[i].vColor;
		vOut.vNormal   = vIn[i].vNormal;
		vOut.vTexCoord = vIn[i].vTexCoord;

		// We transformed the normal to world space in the vertex shader.
		// Now we need it back in model space.
		vec3 modelSpaceNormal = inverse(uNormalToWorld) * vIn[i].vNormal;

		// We transformed the vertices to clip space in the vertex shader.
		// Now we need to transform our translation to clip space too.
		vec4 clipSpaceTrans = uModelToClip * vec4(m * modelSpaceNormal, 0.0);

		gl_Position = gl_in[i].gl_Position + clipSpaceTrans;
		EmitVertex();

		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}

	EndPrimitive();

	for (int i = 0; i < 3; ++i) {
		vOut.vColor    = vIn[i].vColor;
		vOut.vNormal   = vIn[i].vNormal;
		vOut.vTexCoord = vIn[i].vTexCoord;

		vec3 modelSpaceNormal = inverse(uNormalToWorld) * vIn[i].vNormal;
		vec4 clipSpaceTrans = uModelToClip * vec4(m * modelSpaceNormal, 0.0);

		gl_Position = gl_in[i].gl_Position + clipSpaceTrans;
		EmitVertex();
	}

	EndPrimitive();
}
