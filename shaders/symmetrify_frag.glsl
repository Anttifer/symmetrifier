#version 330

//--------------------

// Declare the inputs.
in Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

// Declare the uniform.
uniform sampler2D uTextureSampler;

// Declare the output.
layout(location = 0) out vec4 fColor;

void main() {
	fColor = texture(uTextureSampler, vTexCoord);
}
