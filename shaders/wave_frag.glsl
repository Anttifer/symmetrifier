#version 330

// This is a fragment shader. It is executed once for every pixel* on the screen.
// The idea is to determine the colour of the current pixel just by its coordinates,
// which are retrieved from the vector gl_FragCoord.xy.

// We'll draw a centered set of axes and a waveform by checking if the current pixel
// lies close enough to the axes or the waveform.


// * actually for every fragment, but in this case there is no difference.

//--------------------

// Declare the uniforms.
uniform ivec2 uScreenSize;
uniform float uTime;

uniform bool uTextureFlag = false;
uniform sampler2D uTextureSampler;

// Declare the inputs.
in Data {
	vec3 vNormal;
	vec2 vTexCoord;
};

// Declare the output.
layout(location = 0) out vec4 fColor;

void main() {
	float pi = 3.14159265359;
	vec4 bgColor = uTextureFlag ?
		texture(uTextureSampler, vTexCoord) : vec4(0.15, 0.1, 0.1, 1);

	// Calculate the aspect ratio.
	float AR = uScreenSize.x / float(uScreenSize.y);

	// The axes will be in the middle of the screen.
	ivec2 axes = uScreenSize / 2;

	// Normalized position. The center of the screen
	// will be at (0, 0).
	vec2 nPos = gl_FragCoord.xy / uScreenSize - 0.5;
	nPos.x *= AR;

	// The waveform in normalized coordinates
	// and in floating point.
	float envelope = sin(2 * pi * nPos.x - 2 * uTime);
	float carrier  = sin(20 * pi * nPos.x - 10 * uTime);
	float amplitude = 0.4;

	// The wavefrom, discretized by truncation and in screen
	// coordinates.
	int wave = int(axes.y * (amplitude * envelope * carrier + 1.0));

	// Check if this pixel hit either one of the axes...
	if (any(equal(ivec2(gl_FragCoord.xy), axes)))
		fColor = vec4(0, 1, 0, 1);

	// or if it hit the wave...
	else if (int(gl_FragCoord.y) == wave)
		fColor = vec4(1, 0.6, 0.2, 1);

	// or if it didn't hit anything.
	else
		fColor = bgColor;
}
