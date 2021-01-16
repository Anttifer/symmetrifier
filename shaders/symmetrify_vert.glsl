#version 330

//--------------------

// Declare the input.
layout(location = 0) in vec3 aPosition;

uniform vec2 uLatticePos   = vec2(0, 0);
uniform mat2 uLatticeBasis = mat2(1.0);

uniform vec2 uImagePos      = vec2(0, 0);
uniform mat2 uImageBasisInv = mat2(1.0);

// Declare the outputs.
out Data {
	vec2 vTexCoord;
};

const vec4 vertexPos[6] = vec4[6](vec4(-1, -1, 0, 1), vec4(1, -1, 0, 1), vec4(1, 1, 0, 1),
                                  vec4(1, 1, 0, 1), vec4(-1, 1, 0, 1), vec4(-1, -1, 0, 1));

void main() {
	vTexCoord = uLatticePos + uLatticeBasis * aPosition.xy;
	vTexCoord = uImageBasisInv * (vTexCoord - uImagePos);

	gl_Position = vertexPos[gl_VertexID % 6];
}
