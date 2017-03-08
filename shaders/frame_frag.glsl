#version 330

//--------------------

in Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

uniform bool uRenderOverlay = false;

layout(location = 0) out vec4 fColor;

void main() {
	if (uRenderOverlay)
		fColor = vec4(0.5, 0.7, 1.0, 0.05);
	else
		fColor = vec4(vColor, 1);
}
