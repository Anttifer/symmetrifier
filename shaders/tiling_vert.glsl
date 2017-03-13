#version 330

//--------------------

// Declare the inputs.
layout(location = 0) in vec3 aPosition;

uniform int uNumInstances = 1;

uniform vec2 uPos = vec2(0, 0);
uniform vec2 uT1  = vec2(1, 0);
uniform vec2 uT2  = vec2(0, 1);

uniform ivec2 uScreenSize;
uniform vec2  uScreenCenter;
uniform float uPixelsPerUnit;

uniform vec2 uTexCoords[6];

// Declare the outputs.
out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

vec2 NDCPosition(in vec3 pos) {
	vec2 worldPos = uPos + uT1 * pos.x + uT2 * pos.y;
	return (worldPos - uScreenCenter) * vec2(uPixelsPerUnit) / (0.5 * uScreenSize);
}

void main() {
	int s = int(sqrt(uNumInstances));

	int y = gl_InstanceID / s;
	int x = gl_InstanceID % s;

	vec3 adjustment = vec3(x - s / 2, y - s / 2, 0);
	vec3 instancePos = adjustment + aPosition;

	vTexCoord = uTexCoords[gl_VertexID % 6];

	gl_Position = vec4(NDCPosition(instancePos), 0, 1);

	// Suppress OS X errors by writing zeroes.
	vColor    = vec3(0);
	vNormal   = vec3(0);
}
