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

	gl_Position = vec4(aPosition, 1);
}
