#version 330

//--------------------

in vec3 vInfluence;

layout(location = 0) out vec4 fColor;

void main() {
	if (any(lessThan(vInfluence, vec3(0.02))))
		fColor = vec4(0, 0, 0, 1);
	else
		fColor = vec4(vInfluence.y * vec3(1.0, 0.6, 0.1), 1.0);
}
