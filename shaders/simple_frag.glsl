#version 330

// Check the wave shader for basic information on
// fragment shaders.

//--------------------

// Declare the inputs.
in Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

// Declare the uniforms.
uniform bool uTextureFlag = false;
uniform sampler2D uTextureSampler;

// Declare the output.
layout(location = 0) out vec4 fColor;

void main() {
	// Just take the interpolated colour and use it.
	fColor = vec4(vColor, 1.0);

	// If we have a texture, sample it instead
	// with the interpolated texture coordinate.
	if (uTextureFlag)
		fColor = texture(uTextureSampler, vTexCoord);
}
