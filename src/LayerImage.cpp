#include "LayerImage.h"

LayerImage::LayerImage(const std::string& name, GL::Texture&& texture) :
	position_ (0.0f, 0.0f),
	t1_       (1.0f, 0.0f),
	name_     (name)
{
	set_texture(std::move(texture));
}

Eigen::Vector2f LayerImage::center(void) const
{
	return position_ + (t1_ + t2()) / 2.0f;
}

Eigen::Vector2f LayerImage::t2(void) const
{
	Eigen::Vector2f orthogonal = { -t1_.y(), t1_.x() };
	orthogonal *= texture_.height_ / (float)texture_.width_;

	return orthogonal;
}

float LayerImage::rotation(void) const
{
	return std::atan2(t1_.y(), t1_.x());
}

void LayerImage::set_texture(GL::Texture&& texture)
{
	texture_ = std::move(texture);

	// We'll use nearest neighbor filtering.
	GLint old_tex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex);
	glBindTexture(GL_TEXTURE_2D, texture_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Sensible wrapping parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindTexture(GL_TEXTURE_2D, old_tex);
}

void LayerImage::set_center(const Eigen::Vector2f& center)
{
	position_ = center - (t1_ + t2()) / 2.0f;
}

void LayerImage::set_t1(const Eigen::Vector2f& t1)
{
	// Never scale to zero.
	if (t1.x() == 0.0f && t1.y() == 0.0f)
		return;

	auto old_center = center();
	t1_ = t1;

	set_center(old_center);
}

void LayerImage::set_rotation(float r)
{
	auto old_center = center();
	auto old_scale  = scale();
	auto rotation   = Eigen::Rotation2D<float>(r);

	t1_ = rotation * Eigen::Vector2f(old_scale, 0.0f);

	set_center(old_center);
}

void LayerImage::set_scale(float scale)
{
	// Never scale to zero.
	if (scale == 0.0f)
		return;

	auto old_center = center();
	t1_.normalize();
	t1_ *= scale;

	set_center(old_center);
}

void LayerImage::multiply_scale(float factor)
{
	// Never scale to zero.
	if (factor == 0.0f)
		return;

	auto old_center = center();
	t1_ *= factor;

	set_center(old_center);
}
