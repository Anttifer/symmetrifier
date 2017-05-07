#ifndef LAYERING_H
#define LAYERING_H

#include "Layer.h"

class Layering
{
public:
	Layering (void);

	size_t size (void) const { return layers_.size(); }

	// The returned reference should _not_ be stored long-term, obviously.
	const Layer& layer         (size_t index) const;
	Layer&       layer         (size_t index);

	size_t       current_layer_index (void) const { return current_index_; }
	const Layer& current_layer       (void) const { return layers_[current_index_]; }
	Layer&       current_layer       (void)       { return layers_[current_index_]; }

	void set_current_layer (const Layer&);
	void set_current_layer (size_t index);

	void next_layer     (void);
	void previous_layer (void);

	void transfer_image    (size_t source_layer, size_t destination_layer,
	                        size_t source_image, size_t destination_image);
	void transfer_image    (size_t layer, size_t source_image, size_t destination_image);

	// Moves from top to bottom or from bottom to top, depending on layer order.
	void transfer_image    (size_t source_layer, size_t destination_layer);

	template <typename... Args>
	void add_layer (Args&&...);

	template <typename... Args>
	void insert_layer (size_t index, Args&&...);

	void remove_layer (Layer&&);
	void remove_layer (size_t index);

	void remove_empty_layers (void);

	auto begin (void) const { return layers_.begin(); }
	auto end   (void) const { return layers_.end(); }

private:
	std::vector<Layer> layers_;
	size_t             current_index_;
};

#include "Layering.tcc"

#endif // LAYERING_H
