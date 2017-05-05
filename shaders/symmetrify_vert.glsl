#version 330

//--------------------

// Declare the input.
layout(location = 0) in vec3 aPosition;

uniform int uNumInstances = 1;

uniform vec2 uPos = vec2(0, 0);
uniform vec2 uT1  = vec2(1, 0);
uniform vec2 uT2  = vec2(0, 1);
uniform vec2 uImagePos = vec2(0, 0);
uniform vec2 uImageT1  = vec2(1, 0);
uniform vec2 uImageT2  = vec2(0, 1);

// Declare the outputs.
out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

void main() {
	int s = int(sqrt(uNumInstances));

	int y = gl_InstanceID / s;
	int x = gl_InstanceID % s;

	vec3 adjustment = vec3(x - s / 2, y - s / 2, 0);
	vec3 instancePos = adjustment + aPosition;

	vTexCoord = uPos + uT1 * instancePos.x + uT2 * instancePos.y;
	vTexCoord = inverse(mat2(uImageT1, uImageT2)) * (vTexCoord - uImagePos);

	const vec4 vertexPos[6] = vec4[6](vec4(-1, -1, 0, 1), vec4(1, -1, 0, 1), vec4(1, 1, 0, 1),
	                                  vec4(1, 1, 0, 1), vec4(-1, 1, 0, 1), vec4(-1, -1, 0, 1));

	gl_Position = vertexPos[gl_VertexID % 6];

	// Suppress OS X errors by writing zeroes.
	vColor    = vec3(0);
	vNormal   = vec3(0);
}
