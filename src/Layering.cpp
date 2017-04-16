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

void Layering::remove_layer(Layer&& layer)
{
	// Never remove the last layer.
	// TODO: Maybe log the illegal operation?
	if (size() == 1)
		return;

	auto predicate = [&layer](const Layer& l){ return &layer == &l; };
	std::remove_if(std::begin(layers_), std::end(layers_), predicate);

	if (current_index_ >= size())
		current_index_ = size() - 1;
}

void Layering::remove_layer(size_t index)
{
	remove_layer(std::move(layer(index)));
}
