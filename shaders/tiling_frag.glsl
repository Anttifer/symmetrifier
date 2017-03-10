#version 330

//--------------------

in Data {
	vec3 vColor;
	vec3 vNormal;
	vec2 vTexCoord;
};

uniform sampler2D uTextureSampler;

uniform vec2 uPos = vec2(0, 0);
uniform vec2 uT1  = vec2(1, 0);
uniform vec2 uT2  = vec2(0, 1);

uniform ivec2 uScreenSize;
uniform vec2  uScreenCenter;
uniform float uPixelsPerUnit;

layout(location = 0) out vec4 fColor;

void main() {
	vec2 worldScreenHalfSize = uScreenSize / (2 * uPixelsPerUnit);
	vec2 worldPos   = uScreenCenter
	                + worldScreenHalfSize * (2 * gl_FragCoord.xy / uScreenSize - 1);
	vec2 latticePos = inverse(mat2(uT1, uT2)) * (worldPos - uPos);

	fColor = texture(uTextureSampler, latticePos);
}
