#version 330

//--------------------

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform float uAR;
uniform ivec2 uScreenSize;
uniform vec2  uScreenCenter;
uniform float uPixelsPerUnit;

out Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

void main() {
	// Transform ShaderCanvas from (-1, -1) to (0, 0) and scale it.
	vec2 position = 0.5 * aPosition.xy + vec2(0.5);

	// Scale according to aspect ratio.
	position.y /= uAR;

	// Finally the usual view transformation.
	position      = (position - uScreenCenter) * vec2(uPixelsPerUnit) / (0.5 * uScreenSize);
	gl_Position   = vec4(position, 0, 1);

	vNormal   = aNormal;
	vTexCoord = aTexCoord;
}
