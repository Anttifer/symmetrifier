#version 330

//--------------------

in Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 fColor;

void main() {
	fColor = texture(uTextureSampler, vTexCoord);
}
