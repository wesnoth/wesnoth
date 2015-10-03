/* $Id$ */
/*
  Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

/**
 * Manage the terrain-palette in the editor.
 * Note: this is a near-straight rip from the old editor.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor_common.hpp"
#include "editor_palettes.hpp"

#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../serialization/string_utils.hpp"
#include "../sound.hpp"
#include "../tooltips.hpp"

namespace editor {

static bool is_valid_terrain(t_translation::t_terrain c) {
	return !(c == t_translation::VOID_TERRAIN || c == t_translation::FOGGED);
}

terrain_group::terrain_group(const config& cfg, display& gui):
	id(cfg["id"]), name(cfg["name"]),
	button(gui.video(), "", gui::button::TYPE_CHECK, cfg["icon"]),
	core(utils::string_bool(cfg["core"], false))
{
}

terrain_palette::terrain_palette(display &gui, const size_specs &sizes,
								 const config& cfg,
								 t_translation::t_terrain& fore,
								 t_translation::t_terrain& back)
	: gui::widget(gui.video())
	, size_specs_(sizes)
	, gui_(gui)
	, tstart_(0)
	, terrain_map_()
	, terrains_()
	, terrain_groups_()
	, non_core_terrains_()
	, checked_group_btn_(0)
	, top_button_(gui.video(), "", gui::button::TYPE_PRESS, "uparrow-button")
	, bot_button_(gui.video(), "", gui::button::TYPE_PRESS, "downarrow-button")
	, button_x_()
	, top_button_y_()
	, bot_button_y_()
	, nterrains_()
	, terrain_start_()
	, selected_fg_terrain_(fore)
	, selected_bg_terrain_(back)
{
	// Get the available terrains temporary in terrains_
	terrains_ = map().get_terrain_list();

	//move "invalid" terrains to the end
	std::stable_partition(terrains_.begin(), terrains_.end(), is_valid_terrain);

	// Get the available groups and add them to the structure
	std::set<std::string> group_names;
	BOOST_FOREACH (const config &g, cfg.child_range("editor_group"))
	{
		if (group_names.find(g["id"]) == group_names.end()) {
			terrain_groups_.push_back(terrain_group(g, gui));
			group_names.insert(terrain_groups_.back().id);
			// By default the 'all'-button is pressed
			if(terrain_groups_.back().id == "all") {
				terrain_groups_.back().button.set_check(true);
				checked_group_btn_ = &terrain_groups_.back().button;
			}
		}
	}
	std::map<std::string, terrain_group*> id_to_group;
	BOOST_FOREACH (terrain_group& tg, terrain_groups_) {
		id_to_group.insert(std::make_pair(tg.id, &tg));
	}
	// The rest of the code assumes this is a valid pointer
	assert(checked_group_btn_ != 0);

	// add the groups for all terrains to the map
	BOOST_FOREACH (const t_translation::t_terrain& t, terrains_) {
		const terrain_type& t_info = map().get_terrain_info(t);
		DBG_ED << "Palette: processing terrain " << t_info.name()
			<< "(" << t_info.number() << ")"
			<< ": " << t_info.editor_group() << "\n";

		// don't display terrains that were automatically created from base+overlay
		if (t_info.is_combined()) continue;
		// nor display terrains that have hide_in_editor=true
		if (t_info.hide_in_editor()) continue;

		// add the terrain to the requested groups
		const std::vector<std::string>& keys = utils::split(t_info.editor_group());
		bool core = false;
		BOOST_FOREACH (const std::string& k, keys) {
			terrain_map_[k].push_back(t);
			std::map<std::string, terrain_group*>::iterator i = id_to_group.find(k);
			if (i != id_to_group.end()) {
				if (i->second->core) {
					core = true;
				}
			}
		}
		// A terrain is considered core iff it appears in at least
		// one core terrain group
		if (core) {
			// Add the terrain to the default group
			terrain_map_["all"].push_back(t);
		} else {
			non_core_terrains_.insert(t);
		}

	}
	typedef std::pair<std::string, t_translation::t_list> map_pair;

	// Set the default group
	terrains_ = terrain_map_["all"];

	if(terrains_.empty()) {
		ERR_ED << "No terrain found.\n";
	}
	update_report();
	//adjust_size();
}

void terrain_palette::adjust_size() {

	scroll_top();
	const size_t button_height = 24;
	const size_t button_palette_padding = 8;

	// Values for the group buttons fully hardcoded for now
	/** @todo will be fixed later */
	const size_t group_button_height   = 24;
	const size_t group_button_padding  =  2;
	const size_t group_buttons_per_row =  5;

	// Determine number of theme button rows
	size_t group_rows = terrain_groups_.size() / group_buttons_per_row;
	if(terrain_groups_.size() % group_buttons_per_row != 0) {
		++group_rows;
	}
	const size_t group_height = group_rows * (group_button_height + group_button_padding);

	SDL_Rect rect = { size_specs_.palette_x, size_specs_.palette_y, size_specs_.palette_w, size_specs_.palette_h };
	set_location(rect);
	top_button_y_ = size_specs_.palette_y + group_height ;
	button_x_ = size_specs_.palette_x + size_specs_.palette_w/2 - button_height/2;
	terrain_start_ = top_button_y_ + button_height + button_palette_padding;
	const size_t space_for_terrains = size_specs_.palette_h - (button_height + button_palette_padding) * 2 - group_height;
	rect.y = terrain_start_;
	rect.h = space_for_terrains;
	bg_register(rect);
	const unsigned terrains_fitting =
		static_cast<unsigned> (space_for_terrains / size_specs_.terrain_space) *
		size_specs_.terrain_width;
	const unsigned total_terrains = num_terrains();
	nterrains_ = std::min<int>(terrains_fitting, total_terrains);
	bot_button_y_ = size_specs_.palette_y + (nterrains_ / size_specs_.terrain_width) * size_specs_.terrain_space + \
		button_palette_padding * size_specs_.terrain_width + button_height + group_height;
	top_button_.set_location(button_x_, top_button_y_);
	bot_button_.set_location(button_x_, bot_button_y_);

	size_t top = size_specs_.palette_y;
	size_t left = size_specs_.palette_x - 8;
	for(size_t i = 0; i < terrain_groups_.size(); ++i) {
		terrain_groups_[i].button.set_location(left, top);
		if(i % group_buttons_per_row == (group_buttons_per_row - 1)) {
			left = size_specs_.palette_x - 8;
			top += group_button_height + group_button_padding;
		} else {
			left += group_button_height + group_button_padding;
		}
	}

	set_dirty();
}

void terrain_palette::set_dirty(bool dirty) {
	widget::set_dirty(dirty);
	if (dirty) {
		top_button_.set_dirty();
		bot_button_.set_dirty();
		for(size_t i = 0; i < terrain_groups_.size(); ++i) {
			terrain_groups_[i].button.set_dirty();
		}
	}
}

void terrain_palette::scroll_down() {
	if(tstart_ + nterrains_ + size_specs_.terrain_width <= num_terrains()) {
		tstart_ += size_specs_.terrain_width;
		bg_restore();
		set_dirty();
	}
	else if (tstart_ + nterrains_ + (num_terrains() % size_specs_.terrain_width) <= num_terrains()) {
		tstart_ += num_terrains() % size_specs_.terrain_width;
		bg_restore();
		set_dirty();
	}
}

void terrain_palette::scroll_up() {
	unsigned int decrement = size_specs_.terrain_width;
	if (tstart_ + nterrains_ == num_terrains() && num_terrains() % size_specs_.terrain_width != 0) {
		decrement = num_terrains() % size_specs_.terrain_width;
	}
	if(tstart_ >= decrement) {
		bg_restore();
		set_dirty();
		tstart_ -= decrement;
	}
}

void terrain_palette::scroll_top() {
	tstart_ = 0;
	bg_restore();
	set_dirty();
}

void terrain_palette::scroll_bottom() {
	unsigned int old_start = num_terrains();
	while (old_start != tstart_) {
		old_start = tstart_;
		scroll_down();
	}
}

void terrain_palette::set_group(const std::string& id)
{
	terrains_ = terrain_map_[id];
	if(terrains_.empty()) {
		ERR_ED << "No terrain found.\n";
	}
	scroll_top();
}

t_translation::t_terrain terrain_palette::selected_fg_terrain() const
{
	return selected_fg_terrain_;
}

t_translation::t_terrain terrain_palette::selected_bg_terrain() const
{
	return selected_bg_terrain_;
}

void terrain_palette::select_fg_terrain(t_translation::t_terrain terrain)
{
	if (selected_fg_terrain_ != terrain) {
		set_dirty();
		selected_fg_terrain_ = terrain;
		update_report();
	}
}

void terrain_palette::select_bg_terrain(t_translation::t_terrain terrain)
{
	if (selected_bg_terrain_ != terrain) {
		set_dirty();
		selected_bg_terrain_ = terrain;
		update_report();
	}
}

void terrain_palette::swap()
{
	std::swap(selected_fg_terrain_, selected_bg_terrain_);
	set_dirty();
	update_report();
}


/** After the language is changed, the selected terrains needs an update. */
void terrain_palette::update_selected_terrains()
{
	set_dirty();
	update_report();
}

std::string terrain_palette::get_terrain_string(const t_translation::t_terrain t)
{
	std::stringstream str;
	const std::string& name = map().get_terrain_info(t).name();
	const t_translation::t_list& underlying = map().underlying_union_terrain(t);
	str << name;
	if(underlying.size() != 1 || underlying[0] != t) {
		str << " (";
		for(t_translation::t_list::const_iterator i = underlying.begin();
			i != underlying.end(); ++i) {

			str << map().get_terrain_info(*i).name();
			if(i+1 != underlying.end()) {
				str << ",";
			}
		}
		str << ")";
	}
	return str.str();
}

void terrain_palette::left_mouse_click(const int mousex, const int mousey) {
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0 && (tstart_+tselect) < terrains_.size()) {
		select_fg_terrain(terrains_[tstart_+tselect]);
		gui_.invalidate_game_status();
	}
}

void terrain_palette::right_mouse_click(const int mousex, const int mousey) {
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0 && (tstart_+tselect) < terrains_.size()) {
		select_bg_terrain(terrains_[tstart_+tselect]);
		gui_.invalidate_game_status();
	}
}

size_t terrain_palette::num_terrains() const {
	return terrains_.size();
}

void terrain_palette::draw() {
	draw(false);
}

void terrain_palette::handle_event(const SDL_Event& event) {
	if (event.type == SDL_MOUSEMOTION) {
		// If the mouse is inside the palette, give it focus.
		if (point_in_rect(event.button.x, event.button.y, location())) {
			if (!focus(&event)) {
				set_focus(true);
			}
		}
		// If the mouse is outside, remove focus.
		else {
			if (focus(&event)) {
				set_focus(false);
			}
		}
	}
	if (!focus(&event)) {
		return;
	}
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const SDL_MouseButtonEvent mouse_button_event = event.button;
	if (mouse_button_event.type == SDL_MOUSEBUTTONDOWN) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
			left_mouse_click(mousex, mousey);
		}
		if (mouse_button_event.button == SDL_BUTTON_RIGHT) {
			right_mouse_click(mousex, mousey);
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELUP) {
			scroll_up();
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELDOWN) {
			scroll_down();
		}
	}
	if (mouse_button_event.type == SDL_MOUSEBUTTONUP) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
		}
	}
}

void terrain_palette::draw(bool force) {
	if (top_button_.pressed()) {
		scroll_up();
	}
	if (bot_button_.pressed()) {
		scroll_down();
	}

	BOOST_FOREACH (terrain_group& g, terrain_groups_) {
		if (g.button.pressed()) {
			checked_group_btn_ = &g.button;
			set_group(g.id);
			break;
		}
	}

	BOOST_FOREACH (terrain_group& g, terrain_groups_) {
		if (&g.button == checked_group_btn_) {
			g.button.set_check(true);
		} else {
			g.button.set_check(false);
		}
	}

	if (!dirty() && !force) {
		return;
	}
	unsigned int starting = tstart_;
	unsigned int ending = starting + nterrains_;
	SDL_Surface* const screen = gui_.video().getSurface();
	if(ending > num_terrains()){
		ending = num_terrains();
	}
	const SDL_Rect &loc = location();
	int y = terrain_start_;
	SDL_Rect palrect;
	palrect.x = loc.x;
	palrect.y = terrain_start_;
	palrect.w = size_specs_.palette_w;
	palrect.h = size_specs_.palette_h;
	tooltips::clear_tooltips(palrect);
	for(unsigned int counter = starting; counter < ending; counter++){
		const t_translation::t_terrain terrain = terrains_[counter];
		const t_translation::t_terrain base_terrain = map().get_terrain_info(terrain).default_base();

		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = loc.x + (counter_from_zero % size_specs_.terrain_width) * size_specs_.terrain_space;
		dstrect.y = y;
		dstrect.w = size_specs_.terrain_size;
		dstrect.h = size_specs_.terrain_size;
		std::stringstream tooltip_text;

		//Draw default base for overlay terrains
		if(base_terrain != t_translation::NONE_TERRAIN) {
			const std::string base_filename = "terrain/" + map().get_terrain_info(base_terrain).editor_image() + ".png";
			surface base_image(image::get_image(base_filename));

			if(base_image == NULL) {
				tooltip_text << "BASE IMAGE NOT FOUND\n";
				ERR_ED << "image for terrain " << counter << ": '" << base_filename << "' not found\n";
				base_image = image::get_image("misc/missing-image.png");
				if (base_image == NULL) {
					ERR_ED << "Placeholder image not found\n";
					return;
				}
			}

			if(static_cast<unsigned>(base_image->w) != size_specs_.terrain_size ||
			   static_cast<unsigned>(base_image->h) != size_specs_.terrain_size) {

				base_image.assign(scale_surface(base_image,
				   size_specs_.terrain_size, size_specs_.terrain_size));
			}

			SDL_BlitSurface(base_image, NULL, screen, &dstrect);
		}

		const std::string filename = "terrain/" + map().get_terrain_info(terrain).editor_image() + ".png";
		surface image(image::get_image(filename));
		if(image == NULL) {
			tooltip_text << "IMAGE NOT FOUND\n";
			ERR_ED << "image for terrain " << counter << ": '" << filename << "' not found\n";
			image = image::get_image("misc/missing-image.png");
			if (image == NULL) {
				ERR_ED << "Placeholder image not found\n";
				return;
			}
		}

		if(static_cast<unsigned>(image->w) != size_specs_.terrain_size ||
			static_cast<unsigned>(image->h) != size_specs_.terrain_size) {

			image.assign(scale_surface(image,
				size_specs_.terrain_size, size_specs_.terrain_size));
		}

		SDL_BlitSurface(image, NULL, screen, &dstrect);

		SDL_Surface* const screen = gui_.video().getSurface();
		Uint32 color;
		if (terrain == selected_bg_terrain() && terrain == selected_fg_terrain()) {
			color = SDL_MapRGB(screen->format,0xFF,0x00,0xFF);
		}
		else if (terrain == selected_bg_terrain()) {
			color = SDL_MapRGB(screen->format,0x00,0x00,0xFF);
		}
		else if (terrain == selected_fg_terrain()) {
			color = SDL_MapRGB(screen->format,0xFF,0x00,0x00);
		}
		else {
			color = SDL_MapRGB(screen->format,0x00,0x00,0x00);
		}
		draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, color, screen);

		bool is_core = non_core_terrains_.find(terrain) == non_core_terrains_.end();
		SDL_Color tip_color = font::NORMAL_COLOUR;
		if (!is_core) {
			SDL_Color red = {0xff, 0, 0, 0};
			tip_color = red;
		}
		tooltip_text << map().get_terrain_string(terrain);
		if (gui_.get_draw_terrain_codes()) {
			tooltip_text << " - " << terrain;
		}
		if (!is_core) {
			tooltip_text << " " << _("(non-core)") << "\n";
			tooltip_text << _("Will not work in game without extra care.");
		}
		tooltips::add_tooltip(dstrect, tooltip_text.str(), tip_color);
		if (counter_from_zero % size_specs_.terrain_width == size_specs_.terrain_width - 1)
			y += size_specs_.terrain_space;
	}
	update_rect(loc);
	set_dirty(false);
}

int terrain_palette::tile_selected(const int x, const int y) const {
	for(unsigned int i = 0; i != nterrains_; i++) {
		const int px = size_specs_.palette_x + (i % size_specs_.terrain_width) * size_specs_.terrain_space;
		const int py = terrain_start_ + (i / size_specs_.terrain_width) * size_specs_.terrain_space;
		const int pw = size_specs_.terrain_space;
		const int ph = size_specs_.terrain_space;

		if(x > px && x < px + pw && y > py && y < py + ph) {
			return i;
		}
	}
	return -1;
}

void terrain_palette::update_report() {
	const std::string msg = std::string(_("FG")) + ": "
		+ get_terrain_string(selected_fg_terrain()) + "\n"
		+ std::string(_("BG")) +
		": " + get_terrain_string(selected_bg_terrain());
	gui_.set_report_content(reports::SELECTED_TERRAIN, msg);
}

void terrain_palette::load_tooltips()
{
	for(size_t i = 0; i < terrain_groups_.size(); ++i) {
		const std::string& text = terrain_groups_[i].name;
		if(text !="") {
			const SDL_Rect tooltip_rect = terrain_groups_[i].button.location();
			tooltips::add_tooltip(tooltip_rect, text);
		}
	}
}

// void terrain_palette::bg_backup() {
//	restorer_ = surface_restorer(&gui_.video(), get_rect());
// }

// void terrain_palette::bg_restore() {
//	restorer_.restore();
// }

brush_bar::brush_bar(display &gui, const size_specs &sizes,
	std::vector<brush>& brushes, brush** the_brush)
: gui::widget(gui.video()), size_specs_(sizes), gui_(gui),
selected_(0), brushes_(brushes), the_brush_(the_brush),
size_(30) {
	adjust_size();
}

void brush_bar::adjust_size() {// TODO
	set_location(size_specs_.brush_x, size_specs_.brush_y);
	set_measurements(size_ * brushes_.size() + (brushes_.size() - 1) * size_specs_.brush_padding, size_);
	set_dirty();
}

unsigned int brush_bar::selected_brush_size() {
	return selected_;
}

void brush_bar::left_mouse_click(const int mousex, const int mousey) {
	sound::play_UI_sound(game_config::sounds::button_press);
	int index = selected_index(mousex, mousey);
	if(index >= 0) {
		if (static_cast<size_t>(index) != selected_) {
			set_dirty();
			selected_ = index;
			*the_brush_ = &brushes_[index];
		}
	}
}

void brush_bar::handle_event(const SDL_Event& event) {
	if (event.type == SDL_MOUSEMOTION) {
		// If the mouse is inside the palette, give it focus.
		if (point_in_rect(event.button.x, event.button.y, location())) {
			if (!focus(&event)) {
				set_focus(true);
			}
		}
		// If the mouse is outside, remove focus.
		else {
			if (focus(&event)) {
				set_focus(false);
			}
		}
	}
	if (!focus(&event)) {
		return;
	}
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const SDL_MouseButtonEvent mouse_button_event = event.button;
	if (mouse_button_event.type == SDL_MOUSEBUTTONDOWN) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
			left_mouse_click(mousex, mousey);
		}
	}
}

void brush_bar::draw() {
	draw(false);
}

void brush_bar::draw(bool force) {
	if (!dirty() && !force) {
		return;
	}
	const SDL_Rect loc = location();
	int x = loc.x;
	// Everything will be redrawn even though only one little part may
	// have changed, but that happens so seldom so we'll settle with this.
	SDL_Surface* const screen = gui_.video().getSurface();
	for (size_t i = 0; i < brushes_.size(); i++) {
		std::string filename = brushes_[i].image();
		surface image(image::get_image(filename));
		if (image == NULL) {
			ERR_ED << "Image " << filename << " not found." << std::endl;
			continue;
		}
		if (static_cast<unsigned>(image->w) != size_
		|| static_cast<unsigned>(image->h) != size_) {
			image.assign(scale_surface(image, size_, size_));
		}
		SDL_Rect dstrect = {x, size_specs_.brush_y, image->w, image->h};
		SDL_BlitSurface(image, NULL, screen, &dstrect);
		const Uint32 color = i == selected_brush_size() ?
			SDL_MapRGB(screen->format,0xFF,0x00,0x00) :
			SDL_MapRGB(screen->format,0x00,0x00,0x00);
		draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, color, screen);
		x += image->w + size_specs_.brush_padding;
	}
	update_rect(loc);
	set_dirty(false);
}

int brush_bar::selected_index(int x, int y) const {
	const int bar_x = size_specs_.brush_x;
	const int bar_y = size_specs_.brush_y;

	if ((x < bar_x || static_cast<size_t>(x) > bar_x + size_ * brushes_.size() +
	                  brushes_.size() * size_specs_.brush_padding) ||
	    (y < bar_y || static_cast<size_t>(y) > bar_y + size_)) {

		return -1;
	}

	for(size_t i = 0; i <  brushes_.size(); i++) {
		int px = bar_x + size_ * i + i * size_specs_.brush_padding;
		if (x >= px && x <= px + static_cast<int>(size_) && y >= bar_y && y <= bar_y + static_cast<int>(size_)) {
			return i;
		}
	}
	return -1;
}

} // end namespace editor

