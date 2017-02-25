#version 330

//--------------------

// Declare the input.
layout(location = 0) in vec3 aPosition;

uniform int uNumInstances = 1;

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
	gl_Position = vec4(adjustment + aPosition, 1);
}
