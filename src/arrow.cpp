
#include "arrow.hpp"

#include "foreach.hpp"

arrow::arrow(): layer_(display::LAYER_ARROWS)
{
	color_.b = 0;
	color_.g = 0;
	color_.r = 0;
}

void arrow::set_path(const std::list<map_location> path)
{
	path_ = path;
	notify_arrow_changed();
}

void arrow::set_color(const SDL_Color color)
{
	color_ = color;
	notify_arrow_changed();
}

void arrow::set_layer(const display::tdrawing_layer & layer)
{
	layer_ = layer;
	notify_arrow_changed();
}

void arrow::add_observer(arrow_observer & observer)
{
	observers_.push_back(&observer);
}

void arrow::remove_observer(arrow_observer & observer)
{
	observers_.remove(&observer);
}

std::vector<arrow_image> arrow::getImages() const
{
	//TODO: return the proper images instead of this placeholder
	std::vector<arrow_image> images;
	surface test_picture = image::get_image("footsteps/teleport-in.png", image::SCALED_TO_HEX);

	foreach(map_location loc, path_)
	{
		images.push_back(arrow_image(loc, test_picture));
	}

	return images;
}

void arrow::notify_arrow_changed() {
	foreach(arrow_observer* observer, observers_)
	{
		observer->arrow_changed(*this);
	}
}

void arrow::notify_arrow_deleted() {
	foreach(arrow_observer* observer, observers_)
	{
		observer->arrow_deleted(*this);
	}
}
