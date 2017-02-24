#version 330

//--------------------

// Declare the inputs.
layout(location = 0) in vec3 aPosition;

uniform int uNumInstances;

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

	int y = gl_InstanceID / 20;
	int x = gl_InstanceID % 20;

	vec3 adjustment = vec3(x - 10, y - 10, 0);

	gl_Position = vec4(adjustment + aPosition, 1);
}
