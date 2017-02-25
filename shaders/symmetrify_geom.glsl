#version 330

//--------------------

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform float uAR;
uniform vec2 uPos = vec2(0, 0);
uniform vec2 uT1  = vec2(1, 0);
uniform vec2 uT2  = vec2(0, 1);

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

vec2 getTexCoord(in vec2 pos) {
	vec2 ccentroid = (gl_in[0].gl_Position.xy +
	                  gl_in[1].gl_Position.xy +
	                  gl_in[2].gl_Position.xy) / 3;

	// TODO: Make the scaling factor (0.98) an uniform.
	// It is also used in the rendering shader.
	const float scale = 1 / 0.98;
	vec2 scaledPos = scale * (pos - ccentroid) + ccentroid;
	vec2 returnPos = uPos + uT1 * scaledPos.x + uT2 * scaledPos.y;

	// Take into account non-square textures.
	returnPos.y *= uAR;

	return returnPos;
}

void main() {
	vOut.vColor    = vIn[0].vColor;
	vOut.vNormal   = vIn[0].vNormal;
	vOut.vTexCoord = getTexCoord(gl_in[0].gl_Position.xy);
	gl_Position    = vec4(-1, -1, 0, 1);
	EmitVertex();

	if (gl_PrimitiveIDIn % 2 == 0) {
		vOut.vColor    = vIn[1].vColor;
		vOut.vNormal   = vIn[1].vNormal;
		vOut.vTexCoord = getTexCoord(gl_in[1].gl_Position.xy);
		gl_Position    = vec4(1, -1, 0, 1);
		EmitVertex();

		vOut.vColor    = vIn[2].vColor;
		vOut.vNormal   = vIn[2].vNormal;
		vOut.vTexCoord = getTexCoord(gl_in[2].gl_Position.xy);
		gl_Position    = vec4(1, 1, 0, 1);
		EmitVertex();
	}
	else {
		vOut.vColor    = vIn[1].vColor;
		vOut.vNormal   = vIn[1].vNormal;
		vOut.vTexCoord = getTexCoord(gl_in[1].gl_Position.xy);
		gl_Position    = vec4(1, 1, 0, 1);
		EmitVertex();

		vOut.vColor    = vIn[2].vColor;
		vOut.vNormal   = vIn[2].vNormal;
		vOut.vTexCoord = getTexCoord(gl_in[2].gl_Position.xy);
		gl_Position    = vec4(-1, 1, 0, 1);
		EmitVertex();
	}

	EndPrimitive();
}
