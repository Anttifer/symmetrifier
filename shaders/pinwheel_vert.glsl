#version 330

//--------------------

layout(location = 0) in vec3 aPosition;

uniform ivec2 uScreenSize;
uniform vec2  uScreenCenter;
uniform float uPixelsPerUnit;
uniform float uTime;

out vec3 vInfluence;

void main() {
	vec2 position = (aPosition.xy - uScreenCenter) * vec2(uPixelsPerUnit) / (0.5 * uScreenSize);
	gl_Position   = vec4(position, 0, 1);
	
	switch (gl_VertexID % 3) {
		case 0:
			vInfluence = vec3(1.0, 0.0, 0.0);
			break;
		case 1:
			vInfluence = vec3(0.0, 1.0, 0.0);
			break;
		case 2:
			vInfluence = vec3(0.0, 0.0, 1.0);
	}
}
