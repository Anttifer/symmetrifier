#version 330

//--------------------

in Data {
	vec3 vNormal;
	vec2 vTexCoord;
};

uniform ivec2 uScreenSize;
uniform float uStripeSize;
uniform bool  uVerticalCrop;

layout(location = 0) out vec4 fColor;

void main() {
	vec2 centerDoubleDist = abs(2.0 * gl_FragCoord.xy - uScreenSize);
	int axis = uVerticalCrop ? 1 : 0;

	fColor = vec4(0, 0, 0, 0.7);
	if (centerDoubleDist[axis] < uStripeSize)
		fColor.a = 0.0;
}
