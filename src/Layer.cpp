#include "Layer.h"

#include "GLFunctions.h"

#define SCALE 0.98f

Layer::Layer(void) :
	current_index_  (0),
	position_       (0.0f, 0.0f),
	t1_             (1.0f, 0.0f),
	visible_        (true),
	consistent_     (false),
	symmetry_scale_ (SCALE),
	error_image_    ("ERROR", GL::Texture::from_png("NONEXISTENT_FILE_ERROR"))
{
	const Eigen::Vector2f bottom_centroid = {2.0f / 3.0f, 1.0f / 3.0f};
	const Eigen::Vector2f top_centroid    = {1.0f / 3.0f, 2.0f / 3.0f};
	domain_coordinates_ = {
		bottom_centroid - symmetry_scale_ * (bottom_centroid - Eigen::Vector2f(0.0f, 0.0f)),
		bottom_centroid - symmetry_scale_ * (bottom_centroid - Eigen::Vector2f(1.0f, 0.0f)),
		bottom_centroid - symmetry_scale_ * (bottom_centroid - Eigen::Vector2f(1.0f, 1.0f)),

		top_centroid - symmetry_scale_ * (top_centroid - Eigen::Vector2f(1.0f, 1.0f)),
		top_centroid - symmetry_scale_ * (top_centroid - Eigen::Vector2f(0.0f, 1.0f)),
		top_centroid - symmetry_scale_ * (top_centroid - Eigen::Vector2f(0.0f, 0.0f))
	};
}

Layer::Layer(const std::string& image_name, GL::Texture&& texture) :
	Layer()
{
	add_image(image_name, std::move(texture));
}

Tiling& Layer::tiling(void)
{
	consistent_ = false;
	return tiling_;
}

const LayerImage& Layer::image(size_t index) const
{
	if (index >= size())
		return error_image_;
	else
		return images_[index];
}

// Reuse the const version.
LayerImage& Layer::image(size_t index)
{
	consistent_ = false;
	return const_cast<LayerImage&>(static_cast<const Layer&>(*this).image(index));
}

const GL::Texture& Layer::domain_texture(void) const
{
	if (!consistent_)
		symmetrify();

	return domain_texture_;
}

Eigen::Vector2f Layer::to_world(const Eigen::Vector2f& v) const
{
	Eigen::Matrix2f basis;
	basis << t1_, t2();

	return basis * v + position_;
}

Eigen::Vector2f Layer::from_world(const Eigen::Vector2f& v) const
{
	Eigen::Matrix2f basis;
	basis << t1_, t2();

	return basis.inverse() * (v - position_);
}

Eigen::Vector2f Layer::to_world_direction(const Eigen::Vector2f& v) const
{
	Eigen::Matrix2f basis;
	basis << t1_, t2();

	return basis * v;
}

Eigen::Vector2f Layer::from_world_direction(const Eigen::Vector2f& v) const
{
	Eigen::Matrix2f basis;
	basis << t1_, t2();

	return basis.inverse() * v;
}

void Layer::set_current_image(const LayerImage& image)
{
	auto predicate = [&image](const LayerImage& i){ return &image == &i; };
	auto it = std::find_if(std::begin(images_), std::end(images_), predicate);

	current_index_ = std::distance(std::begin(images_), it);
}

void Layer::set_current_image(size_t index)
{
	set_current_image(image(index));
}

void Layer::remove_image(LayerImage&& image)
{
	auto predicate = [&image](const LayerImage& i){ return &image == &i; };
	auto it        = std::find_if(std::begin(images_), std::end(images_), predicate);

	if (it == std::end(images_))
		return;

	consistent_ = false;

	size_t index = std::distance(std::begin(images_), it);
	if (current_index_ != 0 && current_index_ >= index)
		--current_index_;

	images_.erase(it);
}

void Layer::remove_image(size_t index)
{
	remove_image(std::move(image(index)));
}

void Layer::symmetrify(void) const
{
	static auto shader = GL::ShaderProgram::from_files(
		"shaders/symmetrify_vert.glsl",
		"shaders/symmetrify_frag.glsl");

	// Find uniform locations once.
	static GLuint lattice_position_uniform;
	static GLuint lattice_basis_uniform;
	static GLuint image_position_uniform;
	static GLuint image_basis_inv_uniform;
	static GLuint sampler_uniform;
	static bool init = [&](){
		lattice_position_uniform = glGetUniformLocation(shader, "uLatticePos");
		lattice_basis_uniform    = glGetUniformLocation(shader, "uLatticeBasis");
		image_position_uniform   = glGetUniformLocation(shader, "uImagePos");
		image_basis_inv_uniform  = glGetUniformLocation(shader, "uImageBasisInv");
		sampler_uniform          = glGetUniformLocation(shader, "uTextureSampler");
		return true;
	}();
	(void)init; // Suppress unused variable warning.

	// Find the maximum image dimension.
	auto dimension = 512u;
	for (const auto& image : images_)
	{
		const auto& t = image.texture();
		dimension = std::max({dimension, t.width_, t.height_});
	}

	// Set up the symmetrified texture.
	if (domain_texture_.width_ != dimension)
		domain_texture_ = GL::Texture::empty_2D(dimension, dimension);
	auto fbo = GL::FBO::simple_C0(domain_texture_);

	// Save previous state.
	GLint old_fbo; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLint old_active; glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active);
	glActiveTexture(GL_TEXTURE1);
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, dimension, dimension);

	const auto& mesh = symmetry_mesh();
	Eigen::Matrix2f lattice_basis;
	lattice_basis << tiling_.t1(), tiling_.t2();

	// Set the shader program, uniforms and texture parameters, and draw.
	glUseProgram(shader);
	glBindVertexArray(mesh.vao_);

	glUniform2fv       (lattice_position_uniform, 1, tiling_.position().data());
	glUniformMatrix2fv (lattice_basis_uniform, 1, GL_FALSE, lattice_basis.data());
	glUniform1i        (sampler_uniform, 1);

	for (const auto& image : images_)
	{
		Eigen::Matrix2f image_basis_inv = (Eigen::Matrix2f() << image.t1(), image.t2())
		                                  .finished().inverse();

		glBindTexture      (GL_TEXTURE_2D, image.texture());
		glUniform2fv       (image_position_uniform, 1, image.position().data());
		glUniformMatrix2fv (image_basis_inv_uniform, 1, GL_FALSE, image_basis_inv.data());

		glDrawArrays(mesh.primitive_type_, 0, mesh.num_vertices_);
	}

	// Clean up.
	glBindVertexArray(0);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, old_tex);
	glActiveTexture(old_active);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	consistent_ = true;
}

const Mesh& Layer::symmetry_mesh(void) const
{
	static const char* prev_symmetry_group = "";
	const char*        curr_symmetry_group = tiling_.symmetry_group();

	static int prev_lattice_domains = -1;
	int        curr_lattice_domains = tiling_.num_lattice_domains();

	// If the symmetry group or the number of lattice domains has changed,
	// the dilated symmetrification mesh must be updated.
	if (strncmp(prev_symmetry_group, curr_symmetry_group, 8) ||
	    prev_lattice_domains != curr_lattice_domains)
	{
		symmetry_mesh_   = Mesh();
		const auto& mesh = tiling_.total_mesh();

		for (size_t i = 0; i < mesh.positions_.size(); i += 3)
		{
			const Eigen::Vector3f& a        = mesh.positions_[i];
			const Eigen::Vector3f& b        = mesh.positions_[i+1];
			const Eigen::Vector3f& c        = mesh.positions_[i+2];
			const Eigen::Vector3f  centroid = (a + b + c) / 3.0f;

			symmetry_mesh_.positions_.push_back(centroid + 1.0f / symmetry_scale_ * (a - centroid));
			symmetry_mesh_.positions_.push_back(centroid + 1.0f / symmetry_scale_ * (b - centroid));
			symmetry_mesh_.positions_.push_back(centroid + 1.0f / symmetry_scale_ * (c - centroid));
		}

		symmetry_mesh_.update_buffers();
		prev_symmetry_group = curr_symmetry_group;
		prev_lattice_domains = curr_lattice_domains;
	}

	return symmetry_mesh_;
}
