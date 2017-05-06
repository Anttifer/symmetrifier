#include "Layering.h"

#include <algorithm>

Layering::Layering(void) :
	current_index_ (0)
{
	layers_.reserve(5);
	add_layer();
}

const Layer& Layering::layer(size_t index) const
{
	if (index >= size())
		return layers_.back();
	else
		return layers_[index];
}

// Reuse the const version.
Layer& Layering::layer(size_t index)
{
	return const_cast<Layer&>(static_cast<const Layering&>(*this).layer(index));
}

void Layering::set_current_layer(const Layer& layer)
{
	auto predicate = [&layer](const Layer& l){ return &layer == &l; };
	auto it = std::find_if(std::begin(layers_), std::end(layers_), predicate);

	current_index_ = std::distance(std::begin(layers_), it);
	if (current_index_ >= size())
		current_index_ = size() - 1;
}

void Layering::set_current_layer(size_t index)
{
	set_current_layer(layer(index));
}

void Layering::next_layer(void)
{
	auto next_index = current_layer_index() + 1;

	if (next_index < size())
		set_current_layer(next_index);
	else
		set_current_layer(0);

	current_layer().unset_current_image();
}

void Layering::previous_layer(void)
{
	auto index = current_layer_index();

	if (index > 0)
		set_current_layer(index - 1);
	else
		set_current_layer(size() - 1);

	current_layer().unset_current_image();
}

void Layering::transfer_image(size_t src, size_t dst, size_t src_img, size_t dst_img)
{
	if (src >= size() || dst >= size() || src_img >= layer(src).size())
		return;

	dst_img = std::min(dst_img, layer(dst).size());

	if (src == dst)
	{
		auto& source      = layer(src).image(src_img);
		auto& destination = layer(src).image(dst_img);

		std::swap(source, destination);
	}
	else
	{
		auto& image = *(std::begin(layer(src)) + src_img);

		layer(dst).insert_image(dst_img, std::move(image));
		layer(src).remove_image(std::move(image));
	}

	set_current_layer(dst);
	layer(dst).set_current_image(dst_img);
}

void Layering::transfer_image(size_t layer, size_t src_img, size_t dst_img)
{
	transfer_image(layer, layer, src_img, dst_img);
}

void Layering::transfer_image(size_t src, size_t dst)
{
	if (src < dst)
		transfer_image(src, dst, layer(src).size() - 1, 0);
	else if (src > dst)
		transfer_image(src, dst, 0, layer(dst).size());
}

void Layering::remove_layer(Layer&& layer)
{
	// Never remove the last layer.
	// TODO: Maybe log the illegal operation?
	if (size() == 1)
		return;

	auto predicate = [&layer](const Layer& l){ return &layer == &l; };
	auto it        = std::find_if(std::begin(layers_), std::end(layers_), predicate);

	if (it == std::end(layers_))
		return;

	size_t index = std::distance(std::begin(layers_), it);
	if (current_index_ != 0 && current_index_ >= index)
		--current_index_;

	layers_.erase(it);

	this->layer(current_index_).unset_current_image();
}

void Layering::remove_layer(size_t index)
{
	remove_layer(std::move(layer(index)));
}

void Layering::remove_empty_layers(void)
{
	// Never remove the last layer.
	// TODO: Maybe log the illegal operation?
	if (size() == 1)
		return;

	auto is_empty = [](const Layer& l) { return l.size() == 0; };
	auto it = std::remove_if(std::begin(layers_), std::end(layers_), is_empty);

	layers_.erase(it, std::end(layers_));

	current_index_ = size() - 1;
}
