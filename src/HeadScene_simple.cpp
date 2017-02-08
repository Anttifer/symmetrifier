#include "HeadScene.h"

//--------------------

HeadScene HeadScene::simple(void) {
	HeadScene scene;

	// Head mesh, textures & shader.
	scene.head_ = Mesh::from_obj("res/head.obj");
	scene.head_diffuse_ = GL::Texture::from_png("res/lambertian-lores.png");
	scene.head_normal_ = GL::Texture::from_png("res/normal-lores.png");
	scene.head_shader_ = GL::ShaderProgram::from_files("shaders/head_vert.glsl", "shaders/head_frag.glsl");

	// Uniform locations.
	scene.head_uniforms_.model_to_clip			= glGetUniformLocation(scene.head_shader_, "uModelToClip");
	scene.head_uniforms_.model_to_view			= glGetUniformLocation(scene.head_shader_, "uModelToView");
	scene.head_uniforms_.normal_model_to_view	= glGetUniformLocation(scene.head_shader_, "uNormalModelToView");
	scene.head_uniforms_.view_to_world			= glGetUniformLocation(scene.head_shader_, "uViewToWorld");

	scene.head_uniforms_.diffuse_sampler		= glGetUniformLocation(scene.head_shader_, "uDiffuseSampler");
	scene.head_uniforms_.normal_sampler			= glGetUniformLocation(scene.head_shader_, "uNormalSampler");
	scene.head_uniforms_.env_sampler			= glGetUniformLocation(scene.head_shader_, "uEnvironmentSampler");

	scene.head_uniforms_.specular_color			= glGetUniformLocation(scene.head_shader_, "uSpecularColor");
	scene.head_uniforms_.specular_exponent		= glGetUniformLocation(scene.head_shader_, "uSpecularExponent");
	scene.head_uniforms_.normals_from_texture	= glGetUniformLocation(scene.head_shader_, "uNormalsFromTexture");
	scene.head_uniforms_.use_env				= glGetUniformLocation(scene.head_shader_, "uUseEnv");

	scene.head_uniforms_.dir_light_sampler		= glGetUniformLocation(scene.head_shader_, "uDirLightSampler");
	scene.head_uniforms_.point_light_sampler	= glGetUniformLocation(scene.head_shader_, "uPointLightSampler");

	// Static uniforms.
	glUseProgram(scene.head_shader_);
	glUniform1i(scene.head_uniforms_.diffuse_sampler, 0);
	glUniform1i(scene.head_uniforms_.normal_sampler, 1);
	glUniform1i(scene.head_uniforms_.env_sampler, 2);

	glUniform3f(scene.head_uniforms_.specular_color, 0.35f, 0.35f, 0.35f);
	glUniform1f(scene.head_uniforms_.specular_exponent, 40.0f);

	glUniform1i(scene.head_uniforms_.dir_light_sampler, 3);
	glUniform1i(scene.head_uniforms_.point_light_sampler, 4);
	glUseProgram(0);

	// Static texture units.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.head_diffuse_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scene.head_normal_);

	return scene;
}
