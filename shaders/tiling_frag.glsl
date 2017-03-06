#version 330

//--------------------

in Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

uniform sampler2D uTextureSampler;
uniform bool uTextureFlag = true;

layout(location = 0) out vec4 fColor;

void main() {
	if (uTextureFlag)
		fColor = texture2D(uTextureSampler, vTexCoord);
	else if (any(lessThan(vColor, vec3(0.005))))
		fColor = vec4(0.2, 0.7, 0.2, 1);
	else
		fColor = vec4(0.5, 1, 0.5, 0.05);
}
