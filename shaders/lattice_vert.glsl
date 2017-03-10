#version 330

//--------------------

// Declare the inputs.
layout(location = 0) in vec3 aPosition;

// Declare the outputs.
out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

void main() {
	gl_Position = vec4(2 * aPosition - 1, 1);

	// Suppress OS X errors by writing zeroes.
	vColor    = vec3(0);
	vNormal   = vec3(0);
	vTexCoord = vec2(0);
}
