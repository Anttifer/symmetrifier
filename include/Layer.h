#ifndef LAYER_H
#define LAYER_H

#include "LayerImage.h"
#include "Tiling.h"

class Layer
{
public:
	Layer (void);
	Layer (const std::string& image_name, GL::Texture&&);

	const Layer&           as_const            (void)         const { return *this; }

	const Eigen::Vector2f& position            (void)         const { return position_; }
	float                  rotation            (void)         const;
	const Eigen::Vector2f& t1                  (void)         const { return t1_; }
	Eigen::Vector2f        t2                  (void)         const { return {-t1_.y(), t1_.x()}; }

	size_t                 size                (void)         const { return images_.size(); }
	bool                   visible             (void)         const { return visible_; }
	bool                   consistent          (void)         const { return consistent_; }

	const Tiling&          tiling              (void)         const { return tiling_; }
	Tiling&                tiling              (void);

	// Returns an error texture if index is invalid. Never fails.
	const LayerImage&      image               (size_t index) const;
	LayerImage&            image               (size_t index);

	bool                   has_current_image   (void)         const { return current_index_ < size(); }
	size_t                 current_image_index (void)         const { return current_index_; }
	const LayerImage&      current_image       (void)         const { return image(current_index_); }
	LayerImage&            current_image       (void)               { return image(current_index_); }

	const GL::Texture&     domain_texture      (void)         const;
	const std::vector<Eigen::Vector2f>&
	                       domain_coordinates  (void)         const { return domain_coordinates_; }

	Eigen::Vector2f to_world             (const Eigen::Vector2f&) const;
	Eigen::Vector2f from_world           (const Eigen::Vector2f&) const;
	Eigen::Vector2f to_world_direction   (const Eigen::Vector2f&) const;
	Eigen::Vector2f from_world_direction (const Eigen::Vector2f&) const;

	void set_position (const Eigen::Vector2f& p) { consistent_ = false; position_ = p; }
	void set_rotation (float);
	void set_t1       (const Eigen::Vector2f&);

	void set_visibility   (bool b) { visible_ = b; }
	void set_visible      (void)   { set_visibility(true); }
	void set_invisible    (void)   { set_visibility(false); }
	void set_inconsistent (void)   { consistent_ = false; }

	void set_current_image   (const LayerImage&);
	void set_current_image   (size_t index);
	void unset_current_image (void) { current_index_ = size(); }

	template <typename... Args>
	void add_image (Args&&...);

	template <typename... Args>
	void insert_image (size_t index, Args&&...);

	void remove_image (LayerImage&&);
	void remove_image (size_t index);

	auto begin (void) { consistent_ = false; return images_.begin(); }
	auto end   (void) { consistent_ = false; return images_.end(); }

	auto begin (void) const { return images_.begin(); }
	auto end   (void) const { return images_.end(); }

private:
	void symmetrify (void) const;

	std::vector<LayerImage>      images_;
	size_t                       current_index_;

	Tiling                       tiling_;

	Eigen::Vector2f              position_;
	Eigen::Vector2f              t1_;

	bool                         visible_;
	mutable bool                 consistent_;

	LayerImage                   error_image_;
	mutable GL::Texture          domain_texture_;
	std::vector<Eigen::Vector2f> domain_coordinates_;
};

#include "Layer.tcc"

#endif // LAYER_H
