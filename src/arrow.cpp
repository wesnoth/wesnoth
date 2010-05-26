
#include "arrow.hpp"

using std::list;
using std::vector;
using std::pair;


void arrow::set_path(const list<map_location> & path) {
	//TODO: implement
}

void arrow::set_color(const SDL_Color & color) {
	//TODO: implement
}

void arrow::set_layer(const display::tdrawing_layer & layer) {
	//TODO: implement
}

arrow::arrow() {
	//TODO: implement
}

void arrow::add_observer(const arrow_observer & observer) {
	//TODO: implement
}

void arrow::remove_observer(const arrow_observer & observer) {
	//TODO: implement
}

vector<pair<map_location, surface> > arrow::getImages() {
	//TODO: implement
	vector<pair<map_location, surface> > images;
	return images;
}

void arrow::notify_arrow_changed() {
	//TODO: implement
}

void arrow::notify_arrow_deleted() {
	//TODO: implement
}

