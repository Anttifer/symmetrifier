#version 330

//--------------------

// Declare the inputs.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform int uNumInstances = 1;

uniform vec2 uPos = vec2(0, 0);
uniform vec2 uT1  = vec2(1, 0);
uniform vec2 uT2  = vec2(0, 1);

uniform ivec2 uScreenSize;
uniform vec2  uScreenCenter;
uniform float uPixelsPerUnit;

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
	vColor = aColor;

	int s = int(sqrt(uNumInstances));

	int y = gl_InstanceID / s;
	int x = gl_InstanceID % s;

	vec3 adjustment = vec3(x - s / 2, y - s / 2, 0);

	gl_Position = vec4(NDCPosition(adjustment + aPosition), 0, 1);

	// Suppress OS X errors by writing zeroes.
	vNormal   = vec3(0);
	vTexCoord = vec2(0);
}
