/* $Id$ */
/*
   Copyright (C)
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "filesystem.hpp"
#include "marked-up_text.hpp"
#include "minimap.hpp"
#include "multiplayer_lobby.hpp"
#include "replay.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "show_dialog.hpp"

namespace {

std::string games_menu_heading()
{
	std::ostringstream str;
	str << HEADING_PREFIX << _("Map") << COLUMN_SEPARATOR << _("Name")
		<< COLUMN_SEPARATOR << _("Status") << COLUMN_SEPARATOR << _("Settings");
	return str.str();
}

}

namespace mp {
gamebrowser::gamebrowser(CVideo& video) : scrollarea(video),
	gold_icon_locator_("misc/gold.png"),
	xp_icon_locator_("misc/units.png"),
	vision_icon_locator_("misc/invisible.png"),
	observer_icon_locator_("misc/eye.png"), header_height_(20),
	item_height_(100), margin_(5), h_padding_(5),
	v_padding_(5), selected_(0), visible_range_(std::pair<size_t,size_t>(0,0)),
	double_clicked_(false), ignore_next_doubleclick_(false), last_was_doubleclick_(false)
{
}

void gamebrowser::set_inner_location(const SDL_Rect& rect)
{
	set_full_size(games_.size());
	set_shown_size(rect.h / item_height_);
	bg_register(rect);
	scroll(get_position());
}

void gamebrowser::scroll(int pos)
{
	if(pos >= 0 && pos < games_.size()) {
		visible_range_.first = pos;
		visible_range_.second = minimum<size_t>(pos + inner_location().h / item_height_, games_.size());
		set_dirty();
	}
}

SDL_Rect gamebrowser::get_item_rect(size_t index) const {
	if(index < visible_range_.first || index > visible_range_.second) {
		const SDL_Rect res = { 0, 0, 0, 0 };
		return res;
	}
	const SDL_Rect& loc = inner_location();
	const SDL_Rect res = { loc.x, loc.y + (index - visible_range_.first) * item_height_, loc.w, item_height_ };
	return res;
}

void gamebrowser::draw()
{
	if(hidden())
		return;
	if(dirty()) {
		bg_restore();
		util::scoped_ptr<clip_rect_setter> clipper(NULL);
		if(clip_rect())
			clipper.assign(new clip_rect_setter(video().getSurface(), *clip_rect()));
		draw_contents();
		update_rect(location());
		set_dirty(false);
	}
}

void gamebrowser::draw_contents() const
{
	if(!games_.empty()) {
		for(size_t i = visible_range_.first; i != visible_range_.second; ++i) {
			draw_item(i);
		}
	} else {
		const SDL_Rect rect = inner_location();
		font::draw_text(&video(), rect, font::SIZE_NORMAL, font::NORMAL_COLOUR, _("<no games open>"), rect.x + margin_, rect.y + margin_);
	}
}

void gamebrowser::draw_item(size_t index) const {
	const game_item& game = games_[index];
	SDL_Rect item_rect = get_item_rect(index);
	int xpos = item_rect.x + margin_;
	int ypos = item_rect.y + margin_;

	bg_restore(item_rect);
	draw_solid_tinted_rectangle(item_rect.x, item_rect.y, item_rect.w, item_rect.h, 0, 0, 0, 0.2, video().getSurface());

	// draw mini map
	video().blit_surface(xpos, ypos, game.mini_map);

	xpos += item_height_ + margin_;

	const surface name_surf(font::get_rendered_text(font::make_text_ellipsis(game.name, font::SIZE_PLUS, (item_rect.x + item_rect.w) - xpos - margin_), font::SIZE_PLUS, game.vacant_slots > 0 ? font::GOOD_COLOUR : game.observers ? font::NORMAL_COLOUR : font::BAD_COLOUR));
	video().blit_surface(xpos, ypos, name_surf);

	ypos += v_padding_;

	// draw map info
	const surface map_info_surf(font::get_rendered_text(font::make_text_ellipsis(game.map_info, font::SIZE_PLUS, (item_rect.x + item_rect.w) - xpos - margin_), font::SIZE_PLUS, font::GOOD_COLOUR));
	video().blit_surface(xpos, ypos, map_info_surf);

	// draw dimensions text
	const surface dimensions_text(font::get_rendered_text(game.dimensions, font::SIZE_NORMAL, font::NORMAL_COLOUR));
	ypos = item_rect.y + item_rect.h  - margin_ - dimensions_text->h;
	video().blit_surface(xpos, ypos, dimensions_text);

	xpos += dimensions_text->w + 2 * h_padding_;

	// draw gold icon
	const surface gold_icon(image::get_image(gold_icon_locator_, image::UNSCALED));
	ypos = item_rect.y + item_rect.h  - margin_ - gold_icon->h;

	video().blit_surface(xpos, ypos, gold_icon);

	xpos += gold_icon->w + h_padding_;

	// draw gold text
	const surface gold_text(font::get_rendered_text(game.gold, font::SIZE_NORMAL, font::NORMAL_COLOUR));
	ypos -= abs(gold_icon->h - gold_text->h) / 2;
	video().blit_surface(xpos, ypos, gold_text);

	xpos += gold_text->w + 2 * h_padding_;

	// draw xp icon
	const surface xp_icon(image::get_image(xp_icon_locator_, image::UNSCALED));
	ypos = item_rect.y + item_rect.h  - margin_ - xp_icon->h;
	video().blit_surface(xpos, ypos, xp_icon);

	xpos += xp_icon->w + h_padding_;

	// draw xp text
	const surface xp_text(font::get_rendered_text(game.xp, font::SIZE_NORMAL, font::NORMAL_COLOUR));
	ypos -= abs(xp_icon->h - xp_text->h) / 2;
	video().blit_surface(xpos, ypos, xp_text);

	xpos += xp_text->w + 2 * h_padding_;

	// draw vision icon
	const surface vision_icon(image::get_image(vision_icon_locator_, image::UNSCALED));
	video().blit_surface(xpos, ypos, vision_icon);

	xpos += vision_icon->w + h_padding_;

	const surface status_text(font::get_rendered_text(game.status, font::SIZE_NORMAL, game.vacant_slots > 0 ? font::GOOD_COLOUR : font::NORMAL_COLOUR));
	const surface vision_text(font::get_rendered_text(font::make_text_ellipsis(game.vision, font::SIZE_NORMAL, maximum<int>((item_rect.x + item_rect.w - margin_ - status_text->w - 2 * h_padding_) - xpos, 0)),font::SIZE_NORMAL, font::NORMAL_COLOUR));
	// draw vision text
	video().blit_surface(xpos, ypos, vision_text);

	// draw status text
	xpos = item_rect.x + item_rect.w - margin_ - status_text->w;
	video().blit_surface(xpos, ypos, status_text);

	if(selected_ == index)
		draw_solid_tinted_rectangle(item_rect.x, item_rect.y, item_rect.w, item_rect.h, 153, 0, 0, 0.3, video().getSurface());
}

void gamebrowser::handle_event(const SDL_Event& event)
{
	scrollarea::handle_event(event);
	if(event.type == SDL_KEYDOWN) {
		if(focus() && !games_.empty()) {
			switch(event.key.keysym.sym) {
				case SDLK_UP:
					if(selected_ > 0) {
						--selected_;
						adjust_position(selected_);
						set_dirty();
					}
					break;
				case SDLK_DOWN:
					if(selected_ < games_.size() - 1) {
						++selected_;
						adjust_position(selected_);
						set_dirty();
					}
					break;
				case SDLK_PAGEUP:
				{
					const long items_on_screen = visible_range_.second - visible_range_.first;
					selected_ = static_cast<size_t>(maximum<long>(static_cast<long>(selected_) - items_on_screen, 0));
					adjust_position(selected_);
					set_dirty();
				}
					break;
				case SDLK_PAGEDOWN:
				{
					const size_t items_on_screen = visible_range_.second - visible_range_.first;
					selected_ = minimum<size_t>(selected_ + items_on_screen, games_.size() - 1);
					adjust_position(selected_);
					set_dirty();
				}
					break;
				case SDLK_HOME:
					selected_ = 0;
					adjust_position(selected_);
					set_dirty();
					break;
				case SDLK_END:
					selected_ = games_.size() - 1;
					adjust_position(selected_);
					set_dirty();
					break;
				default:
					break;
			}
		}
	} else if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT || event.type == DOUBLE_CLICK_EVENT) {
		int x = 0;
		int y = 0;
		if(event.type == SDL_MOUSEBUTTONDOWN) {
			x = event.button.x;
			y = event.button.y;
		} else {
			x = (long)event.user.data1;
			y = (long)event.user.data2;
		}
		const SDL_Rect& loc = inner_location();

		if(!games_.empty() && point_in_rect(x, y, loc)) {
			for(size_t i = visible_range_.first; i != visible_range_.second; ++i) {
				const SDL_Rect& item_rect = get_item_rect(i);

				if(point_in_rect(x, y, item_rect)) {
					set_focus(true);
					set_dirty();
					selected_ = i;
					break;
				}
			}
			if(event.type == DOUBLE_CLICK_EVENT) {
				if (ignore_next_doubleclick_) {
					ignore_next_doubleclick_ = false;
				} else if(selection_is_joinable() || selection_is_observable()) {
					double_clicked_ = true;
					last_was_doubleclick_ = true;
				}
			} else if (last_was_doubleclick_) {
				// If we have a double click as the next event, it means
				// this double click was generated from a click that
				// already has helped in generating a double click.
				SDL_Event ev;
				SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT,
							   SDL_EVENTMASK(DOUBLE_CLICK_EVENT));
				if (ev.type == DOUBLE_CLICK_EVENT) {
					ignore_next_doubleclick_ = true;
				}
				last_was_doubleclick_ = false;
			}
		}
	}
}
void gamebrowser::set_game_items(const config& cfg, const config& game_config)
{
	games_.clear();
	config::child_list games = cfg.get_children("game");
	config::child_iterator game;

	for(game = games.begin(); game != games.end(); ++game) {
		games_.push_back(game_item());

		games_.back().map_data = (**game)["map_data"];
		if(games_.back().map_data == "")
			games_.back().map_data = read_map((**game)["map"]);

		if(games_.back().map_data != "") {
			try {
				gamemap map(game_config, games_.back().map_data);
				games_.back().mini_map = image::getMinimap(item_height_ - margin_, item_height_ - 2 * margin_, map, 0);
				char dimensions[10];
				snprintf(dimensions,sizeof(dimensions),"%dx%d", map.x(), map.y());
				games_.back().dimensions=dimensions;
			} catch(gamemap::incorrect_format_exception &e) {
				std::cerr << "illegal map: " << e.msg_ << "\n";
			}
		} else {
			games_.back().dimensions="??x??";
		}
		// now dimensions is non-empty
		games_.back().name = (**game)["name"];
		const std::string& turn = (**game)["turn"];
		const std::string& slots = (**game)["slots"];
		games_.back().vacant_slots = lexical_cast_default<size_t>(slots, 0);
		if(turn != "")
			games_.back().status = _("Turn") + (" " + turn);
		else if(slots != "")
			games_.back().status = slots + " " + ngettext(_("Vacant Slot"), _("Vacant Slots"), games_.back().vacant_slots);

		if((**game)["mp_use_map_settings"] == "yes") {
			games_.back().gold = _("Use map settings");
			games_.back().vision = _("Use map settings");
			games_.back().use_map_settings = true;
		} else {
			games_.back().use_map_settings = false;
			games_.back().gold = (**game)["mp_village_gold"];
			if((**game)["mp_fog"] == "yes") {
				games_.back().vision = _("Fog");
				games_.back().fog = true;
				if((**game)["mp_shroud"] == "yes") {
					games_.back().vision += "/";
					games_.back().vision += _("Shroud");
					games_.back().shroud = true;
				} else {
					games_.back().shroud = false;
				}
			} else if((**game)["mp_shroud"] == "yes") {
				games_.back().vision = _("Shroud");
				games_.back().fog = false;
				games_.back().shroud = true;
			} else {
				games_.back().vision = _("none");
				games_.back().fog = false;
				games_.back().shroud = false;
			}
		}
		games_.back().xp = (**game)["experience_modifier"] + "%";
		games_.back().observers = (**game)["observer"] != "no" ? true : false;
	}
	set_full_size(games_.size());
	set_shown_size(inner_location().h / item_height_);
	scroll(get_position());
	if(selected_ >= games_.size())
		selected_ = maximum<long>(static_cast<long>(games_.size()) - 1, 0);
	set_dirty();
}

lobby::lobby_sorter::lobby_sorter(const config& cfg) : cfg_(cfg)
{
	set_alpha_sort(1);
}

bool lobby::lobby_sorter::column_sortable(int column) const
{
	switch(column)
	{
	case MAP_COLUMN:
	case STATUS_COLUMN:
		return true;
	default:
		return basic_sorter::column_sortable(column);
	}
}

bool lobby::lobby_sorter::less(int column, const gui::menu::item& row1, const gui::menu::item& row2) const
{
	const config* const list = cfg_.child("gamelist");
	if(list == NULL) {
		return false;
	}

	const config::child_list& games = list->get_children("game");
	if(row1.id >= games.size() || row2.id >= games.size()) {
		return false;
	}

	const config& game1 = *games[row1.id];
	const config& game2 = *games[row2.id];

	if(column == MAP_COLUMN) {
		size_t mapsize1 = game1["map_data"].size();
		if(mapsize1 == 0) {
			mapsize1 = game1["map"].size();
		}

		size_t mapsize2 = game2["map_data"].size();
		if(mapsize2 == 0) {
			mapsize2 = game2["map"].size();
		}

		return mapsize1 < mapsize2;

	} else if(column == STATUS_COLUMN) {
		const int nslots1 = atoi(game1["slots"].c_str());
		const int nslots2 = atoi(game2["slots"].c_str());

		const int turn1 = atoi(game1["turn"].c_str());
		const int turn2 = atoi(game2["turn"].c_str());

		if(nslots1 > nslots2) {
			return true;
		} else if(nslots1 < nslots2) {
			return false;
		} else {
			return turn1 < turn2;
		}
	} else {
		return basic_sorter::less(column,row1,row2);
	}

	return false;
}

lobby::lobby(display& disp, const config& cfg, chat& c, config& gamelist) :
	mp::ui(disp, _("Game Lobby"), cfg, c, gamelist),

	observe_game_(disp.video(), _("Observe Game")),
	join_game_(disp.video(), _("Join Game")),
	create_game_(disp.video(), _("Create Game")),
	quit_game_(disp.video(), _("Quit")),
	sorter_(gamelist),
	games_menu_(disp.video()),
	current_game_(0)
{
	game_config::debug = false;
	gamelist_updated();
}

void lobby::hide_children(bool hide)
{
	ui::hide_children(hide);

	games_menu_.hide(hide);
	observe_game_.hide(hide);
	join_game_.hide(hide);
	create_game_.hide(hide);
	quit_game_.hide(hide);
}

void lobby::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);

	join_game_.set_location(xscale(12),yscale(7));
	observe_game_.set_location(join_game_.location().x + join_game_.location().w + 5,yscale(7));
	create_game_.set_location(observe_game_.location().x + observe_game_.location().w + 5,yscale(7));
	quit_game_.set_location(create_game_.location().x + create_game_.location().w + 5,yscale(7));

	games_menu_.set_location(client_area().x, client_area().y + title().height());
	games_menu_.set_measurements(client_area().w, client_area().h
			- title().height() - gui::ButtonVPadding);
}

void lobby::gamelist_updated(bool silent)
{
	ui::gamelist_updated(silent);

	std::vector<std::string> game_strings;
	const config* list = gamelist().child("gamelist");
	if(list == NULL) {
		// No gamelist yet. Do not update anything.
		return;
	}
	games_menu_.set_game_items(*list, game_config());
	join_game_.hide(!games_menu_.selection_is_joinable());
	observe_game_.hide(!games_menu_.selection_is_observable());
}

void lobby::process_event()
{
	join_game_.hide(!games_menu_.selection_is_joinable());
	observe_game_.hide(!games_menu_.selection_is_observable());

	const bool observe = (observe_game_.pressed() || (games_menu_.selected() && !games_menu_.selection_is_joinable())) && games_menu_.selection_is_observable();
	const bool join = (join_game_.pressed() || games_menu_.selected()) && games_menu_.selection_is_joinable();

	if(join || observe) {
		const config* game = gamelist().child("gamelist");

		const int selected = games_menu_.selection();
		if(game != NULL && selected >= 0) {
			const config::const_child_itors i = game->child_range("game");
			wassert(i.first + selected < i.second);

			const config& game_cfg = **(i.first + selected);
			const std::string& id = game_cfg["id"];

			config response;
			config& join = response.add_child("join");
			join["id"] = id;
			network::send_data(response);

			if(observe) {
				set_result(OBSERVE);
			} else {
				set_result(JOIN);
			}
		}
		return;
	}

	if(create_game_.pressed()) {
		set_result(CREATE);
		return;
	}

	if(quit_game_.pressed()) {
		recorder.set_skip(false);
		set_result(QUIT);
		return;
	}
}


void lobby::process_network_data(const config& data, const network::connection sock)
{
	ui::process_network_data(data, sock);
}

}
