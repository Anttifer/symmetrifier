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
	fColor = texture2D(uTextureSampler, vTexCoord);
	// if (any(lessThan(vColor, vec3(0.01))))
	// 	fColor = vec4(0, 0, 0, 1);
	// else
	// 	fColor = vec4(vColor.x * vec3(1.0, 0.6, 0.1), 1.0);
}
