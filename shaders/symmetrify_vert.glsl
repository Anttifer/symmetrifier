#version 330

//--------------------

// Declare the input.
layout(location = 0) in vec3 aPosition;

// Declare the uniforms.
uniform float uAR;
uniform vec2 uPos = vec2(0, 0);
uniform vec2 uT1  = vec2(1, 0);
uniform vec2 uT2  = vec2(0, 1);

// Declare the outputs.
out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

void main() {
	// We need to take into account the possibility of non-square images.
	vTexCoord = uPos + uT1 * aPosition.x + uT2 * aPosition.y;
	vTexCoord.y *= uAR;

	if ( (gl_VertexID / 3) % 2 == 0 ) {
		switch (gl_VertexID % 3) {
			case 0:
				gl_Position = vec4(-1, -1, 0, 1);
				break;
			case 1:
				gl_Position = vec4(1, -1, 0, 1);
				break;
			case 2:
				gl_Position = vec4(1, 1, 0, 1);
		}
	}
	else {
		switch (gl_VertexID % 3) {
			case 0:
				gl_Position = vec4(-1, -1, 0, 1);
				break;
			case 1:
				gl_Position = vec4(1, 1, 0, 1);
				break;
			case 2:
				gl_Position = vec4(-1, 1, 0, 1);
		}
	}
}
