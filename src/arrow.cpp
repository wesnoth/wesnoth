
#include "arrow.hpp"


void arrow::set_path(const std::list<map_location> &)
{
	//TODO: implement
}

void arrow::set_color(const SDL_Color &)
{
	//TODO: implement
}

void arrow::set_layer(const display::tdrawing_layer &)
{
	//TODO: implement
}

arrow::arrow() {
	//TODO: implement
}

void arrow::add_observer(const arrow_observer &)
{
	//TODO: implement
}

void arrow::remove_observer(const arrow_observer &)
{
	//TODO: implement
}

std::vector<std::pair<map_location, surface> > arrow::getImages()
{
	//TODO: implement
	std::vector<std::pair<map_location, surface> > images;
	return images;
}

void arrow::notify_arrow_changed() {
	//TODO: implement
}

void arrow::notify_arrow_deleted() {
	//TODO: implement
}
