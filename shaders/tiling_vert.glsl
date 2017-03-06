#version 330

//--------------------

// Declare the inputs.
layout(location = 0) in vec3 aPosition;

uniform int uNumInstances = 1;

// Declare the outputs.
out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

void main() {
	switch (gl_VertexID % 3) {
		case 0:
			vColor = vec3(1.0, 0.0, 0.0);
			break;
		case 1:
			vColor = vec3(0.0, 1.0, 0.0);
			break;
		case 2:
			vColor = vec3(0.0, 0.0, 1.0);
	}

	int s = int(sqrt(uNumInstances));

	int y = gl_InstanceID / s;
	int x = gl_InstanceID % s;

	vec3 adjustment = vec3(x - s / 2, y - s / 2, 0);

	gl_Position = vec4(adjustment + aPosition, 1);

	// Suppress OS X errors by writing zeroes.
	vNormal   = vec3(0);
	vTexCoord = vec2(0);
}
