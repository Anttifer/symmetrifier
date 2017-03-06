#version 330

//--------------------

in vec3 vInfluence;

uniform ivec2 uScreenSize;
uniform float uTime;

layout(location = 0) out vec4 fColor;

void main() {
	vec2 rPos = 2.0 * (gl_FragCoord.xy / uScreenSize - 0.5);

	float theta = uTime;
	mat2 rot = mat2(
		cos(theta), sin(theta),
		-sin(theta), cos(theta)
	);

	rPos = vec2(0.5) + rot * rPos;

	mat3 mColor = mat3(
		0.1, 0.5, 0.1,
		0.1, 0.2, 0.5,
		0.6, 0.4, 0.25
	);

	vec3 bColor = mColor * vec3(rPos.x, rPos.y, (1 - rPos.y));


	if (any(lessThan(vInfluence, vec3(0.01))))
		fColor = vec4(0, 0, 0, 1);
	else
		// fColor = vec4(vInfluence.y * bColor, 1.0);
		fColor = vec4(vInfluence.x * vec3(1.0, 0.6, 0.1), 1.0);
}
