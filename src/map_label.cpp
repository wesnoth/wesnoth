#include "display.hpp"
#include "font.hpp"
#include "map_label.hpp"

map_labels::map_labels(const display& disp, const gamemap& map) : disp_(disp), map_(map)
{}

map_labels::map_labels(const display& disp, const config& cfg, const gamemap& map) : disp_(disp), map_(map)
{
	read(cfg);
}

map_labels::~map_labels()
{
	clear();
}

void map_labels::write(config& res) const
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		config item;
		i->first.write(item);
		item.values["text"] = get_label(i->first);
		res.add_child("label",item);
	}
}

void map_labels::read(const config& cfg)
{
	clear();

	const config::child_list& items = cfg.get_children("label");
	for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
		const gamemap::location loc(**i);
		const std::string& text = (**i)["text"];
		set_label(loc,text);
	}
}

const std::string& map_labels::get_label(int index) const {
	return font::get_floating_label_text(index);
}

const std::string& map_labels::get_label(const gamemap::location& loc) const
{
	const label_map::const_iterator itor = labels_.find(loc);
	if(itor != labels_.end()) {
		return font::get_floating_label_text(itor->second);
	} else {
		static const std::string empty_str;
		return empty_str;
	}
}

void map_labels::set_label(const gamemap::location& loc, const std::string& text)
{
	const label_map::iterator current_label = labels_.find(loc);
	if(current_label != labels_.end()) {
		font::remove_floating_label(current_label->second);
		labels_.erase(current_label);
	}

	if(text == "") {
		return;
	}

	SDL_Color colour = font::NORMAL_COLOUR;

	if(map_.get_terrain_info(map_.get_terrain(loc)).is_light()) {
		colour.r = 0;
		colour.g = 0;
		colour.b = 0;
	}

	const gamemap::location loc_nextx(loc.x+1,loc.y);
	const gamemap::location loc_nexty(loc.x,loc.y+1);
	const int xloc = (disp_.get_location_x(loc) + disp_.get_location_x(loc_nextx)*2)/3;
	const int yloc = disp_.get_location_y(loc_nexty) - 14;
	const int handle = font::add_floating_label(text,14,colour,xloc,yloc,0,0,-1,disp_.map_area());

	labels_.insert(std::pair<gamemap::location,int>(loc,handle));

	if(disp_.shrouded(loc.x,loc.y)) {
		font::show_floating_label(handle,false);
	}
}

void map_labels::clear()
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::remove_floating_label(i->second);
	}

	labels_.clear();
}

void map_labels::scroll(double xmove, double ymove)
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::move_floating_label(i->second,xmove,ymove);
	}
}

void map_labels::recalculate_labels()
{
	const label_map labels = labels_;
	labels_.clear();
	for(label_map::const_iterator i = labels.begin(); i != labels.end(); ++i) {
		const std::string text = font::get_floating_label_text(i->second);
		font::remove_floating_label(i->second);
		set_label(i->first,text);
	}
}

void map_labels::recalculate_shroud()
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::show_floating_label(i->second,!disp_.shrouded(i->first.x,i->first.y));
	}
}
