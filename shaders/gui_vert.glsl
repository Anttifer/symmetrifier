#version 330

//--------------------

// Declare the inputs.
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

// Declare the uniforms.
uniform vec2 uDisplaySize;

// Declare the outputs.
out Data {
	vec4 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

void main() {
	vColor    = aColor;
	vTexCoord = aTexCoord;

	vec2 normalizedPosition = aPosition / uDisplaySize * vec2(2, -2) + vec2(-1, 1);
	gl_Position = vec4(normalizedPosition, 0, 1);
}
