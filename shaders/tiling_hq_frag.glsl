#version 330

//--------------------

in vec4 vInfluence;

uniform vec2 uFramePos = vec2(0, 0);
uniform vec2 uT1       = vec2(1, 0);
uniform vec2 uT2       = vec2(0, 1);
uniform vec2 uImagePos = vec2(0, 0);
uniform vec2 uImageT1  = vec2(1, 0);
uniform vec2 uImageT2  = vec2(0, 1);

uniform int uNumSymmetryDomains;
uniform samplerBuffer uMeshSampler;

uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 fColor;

void main() {
	fColor = vec4(0);
	for (int i = 0; i < uNumSymmetryDomains; ++i)
	{
		// 6 and 3?
		// Every symmetry domain consists of two triangles, i.e. 6 vertices.
		// vInfluence.w tells us which triangle we're texturing now.
		vec3 v1 = texelFetch(uMeshSampler, 6*i + 3*int(vInfluence.w) + 0).xyz;
		vec3 v2 = texelFetch(uMeshSampler, 6*i + 3*int(vInfluence.w) + 1).xyz;
		vec3 v3 = texelFetch(uMeshSampler, 6*i + 3*int(vInfluence.w) + 2).xyz;

		// Manual barycentric interpolation.
		vec2 coord = (mat3(v1, v2, v3) * vInfluence.xyz).xy;

		coord = uFramePos + uT1 * coord.x + uT2 * coord.y;
		coord = inverse(mat2(uImageT1, uImageT2)) * (coord - uImagePos);

		vec4 sample = texture(uTextureSampler, coord);

		// Manual alpha blending.
		fColor.xyz = sample.w * sample.xyz + (1.0 - sample.w) * fColor.xyz;
		fColor.w = sample.w + (1.0 - sample.w) * fColor.w;
	}
}
