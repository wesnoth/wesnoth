/*
   Copyright (C) 2007 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file multiplayer_lobby.cpp
 * A section on the server where players can chat, create and join games.
 */

#include "global.hpp"

#include "addon/manager_ui.hpp"
#include "construct_dialog.hpp"
#include "filesystem.hpp"
#include "game_preferences.hpp"
#include "lobby_preferences.hpp"
#include "map_exception.hpp"
#include "marked-up_text.hpp"
#include "minimap.hpp"
#include "multiplayer_lobby.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/old_markup.hpp"
#include "gui/dialogs/message.hpp" // for gui2::show_message
#include "gui/dialogs/mp_join_game_password_prompt.hpp"
#include "gui/widgets/window.hpp" // for gui2::twindow::OK
#include "lobby_reload_request_exception.hpp"
#include "log.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "terrain_type_data.hpp"
#include "version.hpp"
#include "game_display.hpp"


#include <cassert>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_lobby("mp/lobby");
#define ERR_MP LOG_STREAM(err, log_lobby)
#define WRN_MP LOG_STREAM(warn, log_lobby)
#define LOG_MP LOG_STREAM(info, log_lobby)
#define DBG_MP LOG_STREAM(debug, log_lobby)

namespace {
std::vector<std::string> empty_string_vector;
}

namespace mp {
gamebrowser::gamebrowser(CVideo& video, const config &map_hashes) :
	menu(video, empty_string_vector, false, -1, -1, NULL, &menu::bluebg_style),
	gold_icon_locator_("themes/gold.png"),
	xp_icon_locator_("themes/units.png"),
	map_size_icon_locator_("misc/map.png"),
	vision_icon_locator_("misc/visibility.png"),
	time_limit_icon_locator_("themes/sand-clock.png"),
	observer_icon_locator_("misc/eye.png"),
	no_observer_icon_locator_("misc/no_observer.png"),
	shuffle_sides_icon_locator_("misc/shuffle-sides.png"),
	map_hashes_(map_hashes),
	item_height_(100),
	margin_(5),
	minimap_size_(item_height_ - 2*margin_),
	h_padding_(5),
	h_padding_image_to_text_(4),
	header_height_(20),
	selected_(0),
	visible_range_(std::pair<size_t,size_t>(0,0)),
	games_(),
	redraw_items_(),
	widths_(),
	double_clicked_(false),
	ignore_next_doubleclick_(false),
	last_was_doubleclick_(false)
{
	set_numeric_keypress_selection(false);
}

void gamebrowser::set_inner_location(const SDL_Rect& rect)
{
	set_full_size(games_.size());
	set_shown_size(rect.h / row_height());
	bg_register(rect);
	scroll(get_position());
}

void gamebrowser::scroll(unsigned int pos)
{
	if(pos < games_.size()) {
		visible_range_.first = pos;
		visible_range_.second = std::min<size_t>(pos + inner_location().h / row_height(), games_.size());
		set_dirty();
	}
}

SDL_Rect gamebrowser::get_item_rect(size_t index) const {
	if(index < visible_range_.first || index > visible_range_.second) {
		const SDL_Rect res = { 0, 0, 0, 0 };
		return res;
	}
	const SDL_Rect& loc = inner_location();
	return sdl::create_rect(
			  loc.x
			, loc.y + (index - visible_range_.first) * row_height()
			, loc.w
			, row_height());
}

void gamebrowser::draw()
{
	if(hidden())
		return;
	if(dirty()) {
		bg_restore();
		util::scoped_ptr<clip_rect_setter> clipper(NULL);
		if(clip_rect())
			clipper.assign(new clip_rect_setter(video().getSurface(), clip_rect()));
		draw_contents();
		update_rect(location());
		set_dirty(false);
	}
}

void gamebrowser::draw_contents()
{
	if(!games_.empty()) {
		for(size_t i = visible_range_.first; i != visible_range_.second; ++i) {
			style_->draw_row(*this,i,get_item_rect(i),(i==selected_)? SELECTED_ROW : NORMAL_ROW);
		}
	} else {
		const SDL_Rect rect = inner_location();
		font::draw_text(&video(), rect, font::SIZE_NORMAL, font::NORMAL_COLOR, _("--no games open--"), rect.x + margin_, rect.y + margin_);
	}
}

void gamebrowser::draw_row(const size_t index, const SDL_Rect& item_rect, ROW_TYPE /*type*/) {
	const game_item& game = games_[index];
	int xpos = item_rect.x + margin_;
	int ypos = item_rect.y + margin_;
	std::string no_era_string = "";
	// Draw minimaps
	if (game.mini_map != NULL) {
		int minimap_x = xpos + (minimap_size_ - game.mini_map->w)/2;
		int minimap_y = ypos + (minimap_size_ - game.mini_map->h)/2;
		video().blit_surface(minimap_x, minimap_y, game.mini_map);
	}
	xpos += minimap_size_ + margin_;

	// Set font color
	SDL_Color font_color;
	if (game.vacant_slots > 0) {
		if (game.reloaded || game.started) {
			font_color = font::YELLOW_COLOR;
		} else {
			font_color = font::GOOD_COLOR;
		}
	} else {
		if (game.observers) {
			font_color = font::NORMAL_COLOR;
		} else {
			font_color = font::BAD_COLOR;
		}
	}
	if(!game.have_scenario && font_color != font::BAD_COLOR) {
		font_color = font::DISABLED_COLOR;
	}
	if(!game.have_era && font_color != font::BAD_COLOR) {
		font_color = font::DISABLED_COLOR;
		no_era_string = _(" (Unknown Era)");
	}
	if(!game.have_all_mods && font_color != font::BAD_COLOR) {
		font_color = font::DISABLED_COLOR;
	}
	if(game.addons_outcome != SATISFIED) {
		font_color = font::DISABLED_COLOR;
		if (game.addons_outcome == NEED_DOWNLOAD) {
			no_era_string += _(" (Need to download addons)");
		} else {
			no_era_string += _(" (Outdated addons)");
		}
	} /*else {
		no_era_string += _(" (You have this addon)");
	}*/

	const surface status_text(font::get_rendered_text(game.status,
	    font::SIZE_NORMAL, font_color, TTF_STYLE_BOLD));
	const int status_text_width = status_text ? status_text->w : 0;

	// First line: draw game name
	const surface name_surf(font::get_rendered_text(
	    font::make_text_ellipsis(game.name + no_era_string, font::SIZE_PLUS,
	        (item_rect.x + item_rect.w) - xpos - margin_ - status_text_width - h_padding_),
	    font::SIZE_PLUS, font_color, TTF_STYLE_BOLD));
	video().blit_surface(xpos, ypos, name_surf);

	// Draw status text
	if(status_text) {
		// Align the bottom of the text with the game name
		video().blit_surface(item_rect.x + item_rect.w - margin_ - status_text_width,
		    ypos + name_surf->h - status_text->h, status_text);
	}

	// Second line
	ypos = item_rect.y + item_rect.h/3 + margin_;

	// Draw map info
	const surface map_info_surf(font::get_rendered_text(
	    font::make_text_ellipsis(game.map_info, font::SIZE_NORMAL,
	        (item_rect.x + item_rect.w) - xpos - margin_),
		font::SIZE_NORMAL, font::NORMAL_COLOR));
	if(map_info_surf) {
		video().blit_surface(xpos, ypos - map_info_surf->h/2, map_info_surf);
	}

	// Third line
	ypos = item_rect.y + 2*item_rect.h/3 - margin_;

	// Draw modifications info
	const surface era_and_mod_info_surf(font::get_rendered_text(
	    font::make_text_ellipsis(game.era_and_mod_info, font::SIZE_NORMAL,
	        (item_rect.x + item_rect.w) - xpos - margin_),
		font::SIZE_NORMAL, font::NORMAL_COLOR));
	if(era_and_mod_info_surf) {
		video().blit_surface(xpos, ypos - era_and_mod_info_surf->h/2, era_and_mod_info_surf);
	}

	// Fourth line
	ypos = item_rect.y + item_rect.h  - margin_;

	// Draw observer icon
	const surface observer_icon(image::get_image(game.observers
	    ? observer_icon_locator_ : no_observer_icon_locator_));
	if(observer_icon) {
		video().blit_surface(xpos, ypos - observer_icon->h, observer_icon);

		// Set ypos to the middle of the line, so that
		// all text and icons can be aligned symmetrical to it
		ypos -= observer_icon->h/2;
		xpos += observer_icon->w + 2 * h_padding_;
	}

	// Draw shuffle icon
	if (game.shuffle_sides)
	{
		const surface shuffle_icon(image::get_image(shuffle_sides_icon_locator_));
		if(shuffle_icon) {
			video().blit_surface(xpos, ypos - shuffle_icon->h/2, shuffle_icon);

			xpos += shuffle_icon->w + 2 * h_padding_;
		}
	}

	// Draw gold icon
	const surface gold_icon(image::get_image(gold_icon_locator_));
	if(gold_icon) {
		video().blit_surface(xpos, ypos - gold_icon->h/2, gold_icon);

		xpos += gold_icon->w + h_padding_image_to_text_;
	}

	// Draw gold text
	const surface gold_text(font::get_rendered_text(game.gold, font::SIZE_NORMAL,
		game.use_map_settings ? font::GRAY_COLOR : font::NORMAL_COLOR));
	if(gold_text) {
		video().blit_surface(xpos, ypos - gold_text->h/2, gold_text);

		xpos += gold_text->w + 2 * h_padding_;
	}

	// Draw xp icon
	const surface xp_icon(image::get_image(xp_icon_locator_));
	if(xp_icon) {
		video().blit_surface(xpos, ypos - xp_icon->h/2, xp_icon);

		xpos += xp_icon->w + h_padding_image_to_text_;
	}

	// Draw xp text
	const surface xp_text(font::get_rendered_text(game.xp, font::SIZE_NORMAL, font::NORMAL_COLOR));
	if(xp_text) {
		video().blit_surface(xpos, ypos - xp_text->h/2, xp_text);

		xpos += xp_text->w + 2 * h_padding_;
	}

	if(!game.map_data.empty()) {
		// Draw map size icon
		const surface map_size_icon(image::get_image(map_size_icon_locator_));
		if(map_size_icon) {
			video().blit_surface(xpos, ypos - map_size_icon->h/2, map_size_icon);

			xpos += map_size_icon->w + h_padding_image_to_text_;
		}

		// Draw map size text
		const surface map_size_text(font::get_rendered_text(game.map_info_size,
			font::SIZE_NORMAL, font::NORMAL_COLOR));
		if(map_size_text) {
			video().blit_surface(xpos, ypos - map_size_text->h/2, map_size_text);

			xpos += map_size_text->w + 2 * h_padding_;
		}
	}

	if(!game.time_limit.empty()) {
		// Draw time icon
		const surface time_icon(image::get_image(time_limit_icon_locator_));
		video().blit_surface(xpos, ypos - time_icon->h/2, time_icon);

		xpos += time_icon->w + h_padding_image_to_text_;

		// Draw time text
		const surface time_text(font::get_rendered_text(game.time_limit,
			font::SIZE_NORMAL, font::NORMAL_COLOR));
		video().blit_surface(xpos, ypos - time_text->h/2, time_text);

		xpos += time_text->w + 2 * h_padding_;
	}

	// Draw vision icon
	const surface vision_icon(image::get_image(vision_icon_locator_));
	if(vision_icon) {
		video().blit_surface(xpos, ypos - vision_icon->h/2, vision_icon);

		xpos += vision_icon->w + h_padding_image_to_text_;
	}

	// Draw vision text
	const surface vision_text(font::get_rendered_text(
	    font::make_text_ellipsis(game.vision, font::SIZE_NORMAL,
	        (item_rect.x + item_rect.w) - xpos - margin_),
	    font::SIZE_NORMAL,
		game.use_map_settings ? font::GRAY_COLOR : font::NORMAL_COLOR));
	if(vision_text) {
		video().blit_surface(xpos, ypos - vision_text->h/2, vision_text);
	}

	// Draw map settings text
	if (game.use_map_settings) {
		xpos += vision_text->w + 3 * h_padding_;
		const surface map_settings_text(font::get_rendered_text(
		    font::make_text_ellipsis(_("Use map settings"), font::SIZE_NORMAL,
		        (item_rect.x + item_rect.w) - xpos - margin_),
		    font::SIZE_NORMAL,
		    (game.verified && game.vacant_slots > 0)
				? font::GOOD_COLOR : font::NORMAL_COLOR));
		video().blit_surface(xpos, ypos - map_settings_text->h/2, map_settings_text);
	}
}

void gamebrowser::handle_event(const SDL_Event& event)
{
	scrollarea::handle_event(event);
	if(event.type == SDL_KEYDOWN) {
		if(focus(&event) && !games_.empty()) {
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
					selected_ = static_cast<size_t>(std::max<long>(static_cast<long>(selected_) - items_on_screen, 0));
					adjust_position(selected_);
					set_dirty();
				}
					break;
				case SDLK_PAGEDOWN:
				{
					const size_t items_on_screen = visible_range_.second - visible_range_.first;
					selected_ = std::min<size_t>(selected_ + items_on_screen, games_.size() - 1);
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
	} else if((event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) || event.type == DOUBLE_CLICK_EVENT) {
		int x = 0;
		int y = 0;
		if(event.type == SDL_MOUSEBUTTONDOWN) {
			x = event.button.x;
			y = event.button.y;
		} else {
			x = reinterpret_cast<size_t>(event.user.data1);
			y = reinterpret_cast<size_t>(event.user.data2);
		}
		const SDL_Rect& loc = inner_location();

		if(!games_.empty() && sdl::point_in_rect(x, y, loc)) {
			for(size_t i = visible_range_.first; i != visible_range_.second; ++i) {
				const SDL_Rect& item_rect = get_item_rect(i);

				if(sdl::point_in_rect(x, y, item_rect)) {
					set_focus(true);
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
				// ??
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

static mp::ADDON_REQ check_addon_version_compatibility (const config & local_item, const config & game, std::vector<required_addon> & req_list);

// Determine if this is a campaign or scenario and add info string to this game item.
void gamebrowser::populate_game_item_campaign_or_scenario_info(gamebrowser::game_item & item, const config & game, const config & game_config, bool & verified)
{
	if (game["mp_campaign"].empty()) {
		if (!game["mp_scenario"].empty()) {
			// Check if it's a multiplayer scenario.
			const config* level_cfg = &game_config.find_child("multiplayer",
				"id", game["mp_scenario"]);
			if (!*level_cfg) {
				// Check if it's a user map.
				level_cfg = &game_config.find_child("generic_multiplayer",
					"id", game["mp_scenario"]);
			}
			if (*level_cfg) {
				item.map_info = _("Scenario:");
				item.map_info += " ";
				item.map_info += (*level_cfg)["name"].str();
				// Reloaded games do not match the original scenario hash,
				// so it makes no sense to test them,
				// they always would appear as remote scenarios.
				if (map_hashes_ && !item.reloaded) {
					std::string hash = game["hash"];
					bool hash_found = false;
					BOOST_FOREACH(const config::attribute& i,
						map_hashes_.attribute_range()) {

						if (i.first == game["mp_scenario"] &&
							i.second == hash) {

							hash_found = true;
							break;
						}
					}
					if (!hash_found) {
						item.map_info += " — ";
						item.map_info += _("Remote scenario");
						verified = false;
					}
				}

				if ((*level_cfg)["require_scenario"].to_bool(false)) {
					mp::ADDON_REQ result = check_addon_version_compatibility((*level_cfg), game, item.addons);
					item.addons_outcome = std::max(item.addons_outcome, result); //elevate to most severe error level encountered so far
				}

			} else {
				utils::string_map symbols;
				symbols["scenario_id"] = game["mp_scenario"];
				item.map_info =
					vgettext("Unknown scenario: $scenario_id", symbols);
				verified = false;

				if (game["require_scenario"].to_bool(false)) {
					item.have_scenario = false;
				}
			}
		} else {
			item.map_info = _("Unknown scenario");
			verified = false;

			if (game["require_scenario"].to_bool(false)) {
				item.have_scenario = false;
			}
		}
	} else { // Is a campaign
		const config* level_cfg = &game_config.find_child("campaign", "id",
			game["mp_campaign"]);
		if (*level_cfg) {
			item.map_info = _("Campaign:");
			item.map_info += " ";
			item.map_info += (*level_cfg)["name"].str();
			item.map_info += " — ";
			item.map_info += game["mp_scenario_name"].str();

			// Difficulty.
			const std::vector<std::string> difficulties =
				utils::split((*level_cfg)["difficulties"]);
			const std::string difficulty_descriptions =
				(*level_cfg)["difficulty_descriptions"];
			std::vector<std::string> difficulty_options =
				utils::split(difficulty_descriptions, ';');
			int index = 0;
			//TODO: use difficulties instead of difficulty_descriptions if
			//difficulty_descriptions is not available
			assert(difficulties.size() == difficulty_options.size());
			BOOST_FOREACH(const std::string& difficulty, difficulties) {
				if (difficulty == game["difficulty_define"]) {
					gui2::tlegacy_menu_item menu_item(difficulty_options[index]);
					item.map_info += " — ";
					item.map_info += menu_item.label();
					item.map_info += " ";
					item.map_info += menu_item.description();

					break;
				}
				index++;
			}

		} else {
			utils::string_map symbols;
			symbols["campaign_id"] = game["mp_campaign"];
			item.map_info =
				vgettext("Unknown campaign: $campaign_id", symbols);
			verified = false;

			if (game["require_scenario"].to_bool(false)) {
				item.have_scenario = false;
			}
		}
	}
}

struct minimap_cache_item {

	minimap_cache_item() :
		map_data(),
		mini_map(),
		map_info_size()
	{
	}

	std::string map_data;
	surface mini_map;
	std::string map_info_size;
};

// Handle the minimap data. Some caching is taking place to avoid repeatedly rendering the minimaps.
void gamebrowser::populate_game_item_map_info(gamebrowser::game_item & item, const config & game, const config & game_config, bool & verified)
{
	// Don't throw the rendered minimaps away
	std::vector<minimap_cache_item> minimap_cache;
	for(std::vector<game_item>::iterator oldgame = games_.begin(); oldgame != games_.end(); ++oldgame) {
		minimap_cache_item item;
		item.map_data = oldgame->map_data;
		item.mini_map = oldgame->mini_map;
		item.map_info_size = oldgame->map_info_size;
		minimap_cache.push_back(item);
	}

	item.map_data = game["map_data"].str();
	if(item.map_data.empty()) {
		item.map_data = filesystem::read_map(game["map"]);
	}
	if(! item.map_data.empty()) {
		try {
			std::vector<minimap_cache_item>::iterator i;
			bool found = false;
			for(i = minimap_cache.begin(); i != minimap_cache.end() && !found; ++i) {
				if (i->map_data == item.map_data) {
					found = true;
					item.map_info_size = i->map_info_size;
					item.mini_map = i->mini_map;
				}
			}
			if (!found) {
				// Parsing the map and generating the minimap are both cpu expensive
				gamemap map(boost::make_shared<terrain_type_data>(game_config), item.map_data);
				item.mini_map = image::getMinimap(minimap_size_, minimap_size_, map, 0);
				item.map_info_size = str_cast(map.w()) + utils::unicode_multiplication_sign
					+ str_cast(map.h());
			}
		} catch (incorrect_map_format_error &e) {
			ERR_CF << "illegal map: " << e.message << '\n';
			verified = false;
		} catch(twml_exception& e) {
			ERR_CF <<  "map could not be loaded: " << e.dev_message << '\n';
			verified = false;
		}
	}
}

// local_item is either an [era] or [modification] tag, something with addon_version and addon_id.
// (These are currently added at add-on loading time in the game_config_manager.)
// It is checked whether the local item's add-on version is required for this game, and if the
// versions are compatible. If it's not, a record is made in the req_list passed as argument.
static mp::ADDON_REQ check_addon_version_compatibility (const config & local_item, const config & game, std::vector<required_addon> & req_list)
{
	if (local_item.has_attribute("addon_id") && local_item.has_attribute("addon_version")) {
		if (const config & game_req = game.find_child("addon", "id", local_item["addon_id"])) {
			// Record object which we will potentially store for this check
			required_addon r;
			r.addon_id = local_item["addon_id"].str();

			const version_info local_ver(local_item["addon_version"].str());
			version_info local_min_ver(local_item.has_attribute("addon_min_version") ? local_item["addon_min_version"] : local_item["addon_version"]);
			// If UMC didn't specify last compatible version, assume no backwards compatibility.
			if (local_min_ver > local_ver) {
				// Some sanity checking regarding min version. If the min ver doens't make sense, ignore it.
				local_min_ver = local_ver;
			}

			const version_info remote_ver(game_req["version"].str());
			version_info remote_min_ver(game_req.has_attribute("min_version") ? game_req["min_version"] : game_req["version"]);
			if (remote_min_ver > remote_ver) {
				remote_min_ver = remote_ver;
			}

			// Check if the host is too out of date to play.
			if (local_min_ver > remote_ver) {
				r.outcome = CANNOT_SATISFY;

				utils::string_map symbols;
				symbols["addon"] = r.addon_id; // TODO: Figure out how to ask the add-on manager for the user-friendly name of this add-on.
				symbols["host_ver"] = remote_ver.str();
				symbols["local_ver"] = local_ver.str();
				r.message = vgettext("Host's version of $addon is too old: host's version $host_ver < your version $local_ver.", symbols);
				req_list.push_back(r);
				return r.outcome;
			}

			// Check if our version is too out of date to play.
			if (remote_min_ver > local_ver) {
				r.outcome = NEED_DOWNLOAD;

				utils::string_map symbols;
				symbols["addon"] = r.addon_id; // TODO: Figure out how to ask the add-on manager for the user-friendly name of this add-on.
				symbols["host_ver"] = remote_ver.str();
				symbols["local_ver"] = local_ver.str();
				r.message = vgettext("Your version of $addon is out of date: host's version $host_ver > your version $local_ver.", symbols);
				req_list.push_back(r);
				return r.outcome;
			}
		}
	}

	return SATISFIED;
}

// Check the era of the game, whether it is available, and compatible with installed content, and add info strings to this game_item.
void gamebrowser::populate_game_item_era_info(gamebrowser::game_item & item, const config & game, const config & game_config, bool & verified) {
	if (!game["mp_era"].empty())
	{
		const config &era_cfg = game_config.find_child("era", "id", game["mp_era"]);
		utils::string_map symbols;
		symbols["era_id"] = game["mp_era"];
		if (era_cfg) {
			item.era_and_mod_info = _("Era:");
			item.era_and_mod_info += " ";
			item.era_and_mod_info += era_cfg["name"].str();

			mp::ADDON_REQ result = check_addon_version_compatibility(era_cfg, game, item.addons);
			item.addons_outcome = std::max(item.addons_outcome, result); //elevate to most severe error level encountered so far
		} else {
			if (!game["require_era"].to_bool(true)) {
				item.have_era = true;
			} else {
				item.have_era = false;
			}
			item.era_and_mod_info = vgettext("Unknown era: $era_id", symbols);
			verified = false;
		}
	} else {
		item.era_and_mod_info = _("Unknown era");
		verified = false;
	}
}

// Check the mods applied to the game, whether they are available / required, and compatible with installed content, and add info strings to this game_item.
void gamebrowser::populate_game_item_mod_info(gamebrowser::game_item & item, const config & game, const config & game_config, bool & verified) {
	(void) verified;
	if (!game.child_or_empty("modification").empty()) {
		item.have_all_mods = true;
		item.era_and_mod_info += " — ";
		item.era_and_mod_info += _("Modifications:");
		item.era_and_mod_info += " ";

		BOOST_FOREACH (const config& m, game.child_range("modification")) {
			const config& mod_cfg = game_config.find_child("modification", "id", m["id"]);
			if (mod_cfg) {
				item.era_and_mod_info += mod_cfg["name"].str();
				item.era_and_mod_info += ", ";

				mp::ADDON_REQ result = check_addon_version_compatibility(mod_cfg, game, item.addons);
				item.addons_outcome = std::max(item.addons_outcome, result); //elevate to most severe error level encountered so far
			} else {
				item.era_and_mod_info += m["id"].str();
				if (m["require_modification"].to_bool(false)) {
					item.have_all_mods = false;
					item.era_and_mod_info += _(" (missing)");
				}
				item.era_and_mod_info += ", ";
			}
		}
		item.era_and_mod_info.erase(item.era_and_mod_info.size()-2, 2);
	} else {
		item.have_all_mods = true;
	}
}

// Do this before populating eras and mods, to get reports of missing add-ons in the most sensible order.
void gamebrowser::populate_game_item_addons_installed(gamebrowser::game_item & item, const config & game, const std::vector<std::string> & installed_addons)
{
	BOOST_FOREACH(const config & addon, game.child_range("addon")) {
		if (addon.has_attribute("id")) {
			if (std::find(installed_addons.begin(), installed_addons.end(), addon["id"].str()) == installed_addons.end()) {
				required_addon r;
				r.addon_id = addon["id"].str();
				r.outcome = NEED_DOWNLOAD;

				utils::string_map symbols;
				symbols["id"] = addon["id"].str();
				r.message = vgettext("Missing addon: $id", symbols);
				item.addons.push_back(r);
				if (item.addons_outcome == SATISFIED) {
					item.addons_outcome = NEED_DOWNLOAD;
				}
			}
		}
	}
}

// Build an appropriate game_item corresponding to this game (config) which we retrieved from [gamelist] from the server.
void gamebrowser::populate_game_item(gamebrowser::game_item & item, const config & game, const config & game_config, const std::vector<std::string> & installed_addons)
{
	bool verified = true;
	item.password_required = game["password"].to_bool();
	item.reloaded = game["savegame"].to_bool();
	item.have_era = true;
	item.have_scenario = true;

	populate_game_item_addons_installed(item, game, installed_addons);
	populate_game_item_campaign_or_scenario_info(item, game, game_config, verified);
	populate_game_item_map_info(item, game, game_config, verified);
	populate_game_item_era_info(item, game, game_config, verified);
	populate_game_item_mod_info(item, game, game_config, verified);

	if (item.reloaded) {
		item.map_info += " — ";
		item.map_info += _("Reloaded game");
		verified = false;
	}
	item.id = game["id"].str();
	item.name = game["name"].str();
	std::string turn = game["turn"];
	std::string slots = game["slots"];
	item.vacant_slots = lexical_cast_default<size_t>(slots, 0);
	item.current_turn = 0;
	if (!turn.empty()) {
		item.started = true;
		int index = turn.find_first_of('/');
		if (index > -1){
			const std::string current_turn = turn.substr(0, index);
			item.current_turn = lexical_cast<unsigned int>(current_turn);
		}
		item.status = _("Turn ") + turn;
	} else {
		item.started = false;
		if (item.vacant_slots > 0) {
			item.status = std::string(_n("Vacant Slot:", "Vacant Slots:",
					item.vacant_slots)) + " " + slots;
			if (item.password_required) {
				item.status += std::string(" (") + std::string(_("Password Required")) + ")";
			}
		}
	}

	item.use_map_settings = game["mp_use_map_settings"].to_bool();
	item.gold = game["mp_village_gold"].str();
	if (game["mp_fog"].to_bool()) {
		item.vision = _("Fog");
		item.fog = true;
		if (game["mp_shroud"].to_bool()) {
			item.vision += "/";
			item.vision += _("Shroud");
			item.shroud = true;
		} else {
			item.shroud = false;
		}
	} else if (game["mp_shroud"].to_bool()) {
		item.vision = _("Shroud");
		item.fog = false;
		item.shroud = true;
	} else {
		item.vision = _("none");
		item.fog = false;
		item.shroud = false;
	}
	if (game["mp_countdown"].to_bool()) {
		item.time_limit = game["mp_countdown_init_time"].str() + " / +"
			+ game["mp_countdown_turn_bonus"].str() + " "
			+ game["mp_countdown_action_bonus"].str();
	} else {
		item.time_limit = "";
	}
	item.xp = game["experience_modifier"].str() + "%";
	item.observers = game["observer"].to_bool(true);
	item.shuffle_sides = game["shuffle_sides"].to_bool(true);
	item.verified = verified;
}

void gamebrowser::set_game_items(const config& cfg, const config& game_config, const std::vector<std::string> & installed_addons)
{
	//DBG_MP << "** gamelist **\n" << cfg.debug() << "****\n";

	const bool scrolled_to_max = (has_scrollbar() && get_position() == get_max_position());
	const bool selection_visible = (selected_ >= visible_range_.first && selected_ <= visible_range_.second);
	const std::string selected_game = (selected_ < games_.size()) ? games_[selected_].id : "";

	item_height_ = 100;

	games_.clear();

	BOOST_FOREACH(const config &game, cfg.child("gamelist").child_range("game"))
	{
		games_.push_back(game_item());
		populate_game_item(games_.back(), game, game_config, installed_addons);
		// Hack...
		if (preferences::filter_lobby())
		{
			if(preferences::fi_invert() ? game_matches_filter(games_.back(), cfg) :
 			  !game_matches_filter(games_.back(), cfg))
			{
				games_.pop_back();
			}
		}
	}
	set_full_size(games_.size());
	set_shown_size(inner_location().h / row_height());

	// Try to preserve the game selection
	if (!selected_game.empty()) {
		for (unsigned int i=0; i < games_.size(); i++) {
			if (games_[i].id == selected_game) {
				selected_ = i;
				break;
			}
		}
	}
	if(selected_ >= games_.size())
		selected_ = std::max<long>(static_cast<long>(games_.size()) - 1, 0);

	if (scrolled_to_max) {
		set_position(get_max_position());
	} else {
		// Keep the selected game visible if it was visible before
		if (selection_visible && (visible_range_.first > selected_
								  || visible_range_.second < selected_)) {
			set_position(selected_);
		}
	}
	scroll(get_position());
	set_dirty();
}

void gamebrowser::select_game(const std::string& id) {
	if (id.empty()) return;

	for (unsigned int i=0; i < games_.size(); i++) {
		if (games_[i].id == id) {
			selected_ = i;
			break;
		}
	}
	adjust_position(selected_);
	set_dirty();
}

bool gamebrowser::game_matches_filter(const game_item& i, const config& cfg)
{
	if(preferences::fi_vacant_slots() && i.vacant_slots == 0) {
		return false;
	}
	if(preferences::fi_friends_in_game()) {
		bool found_friend = false;
		BOOST_FOREACH(const config &user, cfg.child_range("user")) {
			if(preferences::is_friend(user["name"]) && user["game_id"] == i.id) {
				found_friend = true;
				break;
			}
		}
		if(!found_friend) {
			return false;
		}
	}

	if(!preferences::fi_text().empty()) {
		bool found_match = true;
		BOOST_FOREACH(const std::string& search_string, utils::split(preferences::fi_text(), ' ', utils::STRIP_SPACES)) {

			if(!boost::contains(i.map_info, search_string, chars_equal_insensitive) &&
			   !boost::contains(i.name, search_string, chars_equal_insensitive) && 
			   !boost::contains(i.era_and_mod_info, search_string, chars_equal_insensitive)) {

				found_match = false;
				break;
			}
		}
		if(!found_match) {
			return false;
		}
	}

	return true;
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
	const config &list = cfg_.child("gamelist");
	if (!list) {
		return false;
	}

	size_t nb = list.child_count("game");
	if(row1.id >= nb || row2.id >= nb) {
		return false;
	}

	config::const_child_iterator gi = list.child_range("game").first, gs = gi;
	std::advance(gi, row1.id);
	const config &game1 = *gi;
	gi = gs;
	std::advance(gi, row2.id);
	const config &game2 = *gi;

	if(column == MAP_COLUMN) {
		size_t mapsize1 = game1["map_data"].str().size();
		if(mapsize1 == 0) {
			mapsize1 = game1["map"].str().size();
		}

		size_t mapsize2 = game2["map_data"].str().size();
		if(mapsize2 == 0) {
			mapsize2 = game2["map"].str().size();
		}

		return mapsize1 < mapsize2;

	} else if(column == STATUS_COLUMN) {
		int nslots1 = game1["slots"].to_int();
		int nslots2 = game2["slots"].to_int();

		int turn1 = game1["turn"].to_int();
		int turn2 = game2["turn"].to_int();

		if(nslots1 > nslots2) {
			return true;
		} else if(nslots1 < nslots2) {
			return false;
		} else {
			return turn1 < turn2;
		}
	}

	return basic_sorter::less(column,row1,row2);
}

lobby::lobby(game_display& disp, const config& cfg, chat& c, config& gamelist, const std::vector<std::string> & installed_addons) :
	mp::ui(disp, _("Game Lobby"), cfg, c, gamelist),

	current_turn(0),
	game_vacant_slots_(),
	game_observers_(),

	observe_game_(disp.video(), _("Observe Game")),
	join_game_(disp.video(), _("Join Game")),
	create_game_(disp.video(), _("Create Game")),
	replay_options_(disp, std::vector<std::string>()),
	game_preferences_(disp.video(), _("Preferences")),
	quit_game_(disp.video(), _("Quit")),
	apply_filter_(disp.video(), _("Apply filter"), gui::button::TYPE_CHECK),
	invert_filter_(disp.video(), _("Invert"), gui::button::TYPE_CHECK),
	vacant_slots_(disp.video(), _("Vacant slots"), gui::button::TYPE_CHECK),
	friends_in_game_(disp.video(), _("Friends in game"), gui::button::TYPE_CHECK),
	filter_label_(disp.video(), _("Search:")),
	filter_text_(disp.video(), 150),
	last_selected_game_(-1), sorter_(gamelist),
	games_menu_(disp.video(),cfg.child("multiplayer_hashes")),
	minimaps_(),
	search_string_(preferences::fi_text()),
	installed_addons_(installed_addons)
{
	std::vector<std::string> replay_options_strings_;
	replay_options_strings_.push_back(_("Normal Replays"));
	replay_options_strings_.push_back(_("Quick Replays"));
	replay_options_strings_.push_back(_("Enter Blindfolded"));

	replay_options_.set_items(replay_options_strings_);

	std::string help_string1 = _("Skip quickly to the active turn when observing");
	std::string help_string2 = _("Do not show the map until given control of a side");
	replay_options_.set_help_string(help_string1 + " / " + help_string2);

	replay_options_.set_selected(0);
	if (preferences::skip_mp_replay()) {
		replay_options_.set_selected(1);
	}

	if (preferences::blindfold_replay()) {
		replay_options_.set_selected(2);
	}

	apply_filter_.set_check(preferences::filter_lobby());
	apply_filter_.set_help_string(_("Enable the games filter. If unchecked all games are shown, regardless of any filter."));

	invert_filter_.set_check(preferences::fi_invert());
	invert_filter_.set_help_string(_("Show all games that do *not* match your filter. Useful for hiding games you are not interested in."));
	invert_filter_.enable(apply_filter_.checked());

	vacant_slots_.set_check(preferences::fi_vacant_slots());
	vacant_slots_.set_help_string(_("Only show games that have at least one vacant slot"));
	vacant_slots_.enable(apply_filter_.checked());

	friends_in_game_.set_check(preferences::fi_friends_in_game());
	friends_in_game_.set_help_string(_("Only show games that are played or observed by at least one of your friends"));
	friends_in_game_.enable(apply_filter_.checked());

	filter_label_.enable(apply_filter_.checked());

	filter_text_.set_text(search_string_);
	filter_text_.set_help_string(_("Only show games whose title, description, era or mods contain the entered text"));
	filter_text_.enable(apply_filter_.checked());

	gamelist_updated();
	sound::play_music_repeatedly(game_config::lobby_music);

	plugins_context_.reset(new plugins_context("Multiplayer Lobby"));

	//These structure initializers create a lobby::process_data_event
	plugins_context_->set_callback("join", 		boost::bind(&lobby::plugin_event_helper, this, process_event_data (true, false, false, false)));
	plugins_context_->set_callback("observe", 	boost::bind(&lobby::plugin_event_helper, this, process_event_data (false, true, false, false)));
	plugins_context_->set_callback("create", 	boost::bind(&lobby::plugin_event_helper, this, process_event_data (false, false, true, false)));
	plugins_context_->set_callback("quit", 		boost::bind(&lobby::plugin_event_helper, this, process_event_data (false, false, false, true)));
	plugins_context_->set_callback("chat",		boost::bind(&lobby::send_chat_message, this, boost::bind(get_str, _1, "message"), false),	true);
	plugins_context_->set_callback("select_game",	boost::bind(&gamebrowser::select_game, &(this->games_menu_), boost::bind(get_str, _1, "id")),	true);

	plugins_context_->set_accessor("game_list",	boost::bind(&lobby::gamelist, this));
	plugins_context_->set_accessor("game_config",	boost::bind(&lobby::game_config, this));
}

void lobby::hide_children(bool hide)
{
	ui::hide_children(hide);

	games_menu_.hide(hide);
	observe_game_.hide(hide);
	join_game_.hide(hide);
	create_game_.hide(hide);
	replay_options_.hide(hide);
	game_preferences_.hide(hide);
	quit_game_.hide(hide);
	apply_filter_.hide(hide);
	invert_filter_.hide(hide);
	vacant_slots_.hide(hide);
	friends_in_game_.hide(hide);
	filter_label_.hide(hide);
	filter_text_.hide(hide);
}

void lobby::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);

	int btn_space = 5;
	int xborder   = 10;
	int yborder   = 7;

	// Align to the left border
	join_game_.set_location(xscale(xborder), yscale(yborder));
	observe_game_.set_location(join_game_.location().x + join_game_.location().w + btn_space, yscale(yborder));
	create_game_.set_location(observe_game_.location().x + observe_game_.location().w + btn_space, yscale(yborder));

	// Align 'Quit' to the right border
	quit_game_.set_location(xscale(xscale_base - xborder) - quit_game_.location().w, yscale(yborder));

	// Align in the middle between the right and left buttons
	int space = (quit_game_.location().x - create_game_.location().x - create_game_.location().w
	             - replay_options_.location().w - game_preferences_.location().w - btn_space) / 2;
	if (space < btn_space) space = btn_space;
	replay_options_.set_location(create_game_.location().x + create_game_.location().w + space, yscale(yborder));
	game_preferences_.set_location(quit_game_.location().x - game_preferences_.location().w - space, yscale(yborder));

	games_menu_.set_location(client_area().x, client_area().y + title().height());
	games_menu_.set_measurements(client_area().w, client_area().h
			- title().height() - gui::ButtonVPadding
			- apply_filter_.location().h
			);

    apply_filter_.set_location(client_area().x, games_menu_.location().y + games_menu_.location().h + gui::ButtonVPadding);
    invert_filter_.set_location(client_area().x + apply_filter_.location().w + btn_space, games_menu_.location().y + games_menu_.location().h + gui::ButtonVPadding);
    vacant_slots_.set_location(client_area().w - apply_filter_.location().w, games_menu_.location().y + games_menu_.location().h + gui::ButtonVPadding);
    friends_in_game_.set_location(vacant_slots_.location().x - friends_in_game_.location().w - btn_space, games_menu_.location().y + games_menu_.location().h + gui::ButtonVPadding);
    filter_text_.set_location(friends_in_game_.location().x - filter_text_.location().w - btn_space * 4, games_menu_.location().y + games_menu_.location().h + gui::ButtonVPadding);
    filter_label_.set_location(filter_text_.location().x - filter_label_.location().w - btn_space, games_menu_.location().y + games_menu_.location().h + gui::ButtonVPadding
            + (apply_filter_.location().h - filter_label_.location().h) / 2);

}

void lobby::gamelist_updated(bool silent)
{
	ui::gamelist_updated(silent);
	const config &list = gamelist().child("gamelist");
	if (!list) {
		// No gamelist yet. Do not update anything.
		return;
	}
	games_menu_.set_game_items(gamelist(), game_config(), installed_addons_);
	join_game_.enable(games_menu_.selection_is_joinable());
	observe_game_.enable(games_menu_.selection_is_observable());
}

void lobby::process_event()
{
	process_event_data data;
	data.join 		= join_game_.pressed();
	data.observe		= observe_game_.pressed();
	data.create		= create_game_.pressed();
	data.quit		= quit_game_.pressed();

	process_event_impl(data);
}

static void handle_addon_requirements_gui(display & disp, const std::vector<required_addon> & reqs, mp::ADDON_REQ addon_outcome)
{
	if (addon_outcome == CANNOT_SATISFY) {
		std::string e_title = _("Incompatible user-made content.");
		std::string err_msg = _("This game cannot be joined because the host has out-of-date add-ons which are incompatible with your version. You might suggest to them that they update their add-ons.");

		err_msg +="\n\n";
		err_msg += _("Details:");
		err_msg += "\n";

		BOOST_FOREACH(const required_addon & a, reqs) {
			if (a.outcome == CANNOT_SATISFY) {
				err_msg += a.message;
				err_msg += "\n";
			}
		}
		gui2::show_message(disp.video(), e_title, err_msg, gui2::tmessage::auto_close);
	} else if (addon_outcome == NEED_DOWNLOAD) {
		std::string e_title = _("Missing user-made content.");
		std::string err_msg = _("This game requires one or more user-made addons to be installed or updated in order to join. Do you want to try to install them?");

		err_msg +="\n\n";
		err_msg += _("Details:");
		err_msg += "\n";

		std::vector<std::string> needs_download;
		BOOST_FOREACH(const required_addon & a, reqs) {
			if (a.outcome == NEED_DOWNLOAD) {
				err_msg += a.message;
				err_msg += "\n";

				needs_download.push_back(a.addon_id);
			} else if (a.outcome == CANNOT_SATISFY) {
				assert(false);
			}
		}
		assert(needs_download.size() > 0);

		if (gui2::show_message(disp.video(), e_title, err_msg, gui2::tmessage::yes_no_buttons) == gui2::twindow::OK) {
			ad_hoc_addon_fetch_session(disp, needs_download);
			throw lobby_reload_request_exception();
		}
	}
}

// The return value should be true if the gui result was not chnaged
void lobby::process_event_impl(const process_event_data & data)
{
	join_game_.enable(games_menu_.selection_is_joinable());
	observe_game_.enable(games_menu_.selection_is_observable());

	// check whehter to try to download addons
	if ((games_menu_.selected() || data.observe || data.join) && games_menu_.selection_needs_addons())
	{
		if (const std::vector<required_addon> * reqs = games_menu_.selection_addon_requirements()) {
			handle_addon_requirements_gui(disp(), *reqs, games_menu_.selection_addon_outcome());
		} else {
			gui2::show_error_message(video(), _("Something is wrong with the addon version check database supporting the multiplayer lobby, please report this at bugs.wesnoth.org."));
		}
		games_menu_.reset_selection();
		return ;
	}

	const bool observe = (data.observe || (games_menu_.selected() && !games_menu_.selection_is_joinable())) && games_menu_.selection_is_observable();
	const bool join = (data.join || games_menu_.selected()) && games_menu_.selection_is_joinable();

	games_menu_.reset_selection();
	preferences::set_skip_mp_replay(replay_options_.selected() == 1);
	preferences::set_blindfold_replay(replay_options_.selected() == 2);

	preferences::set_message_private(false);

	int selected_game = games_menu_.selection();
	if (selected_game != last_selected_game_) {
		if (games_menu_.empty()) {
			set_selected_game("");
		} else {
			set_selected_game(games_menu_.selected_game().id);
		}
		ui::gamelist_updated();
		last_selected_game_ = selected_game;
	}

	if(selected_user_changed()) {
		set_selected_user_changed(false);
		games_menu_.select_game(get_selected_user_game());
	}

	if(join || observe) {
		const int selected = games_menu_.selection();
		if(!games_menu_.empty() && selected >= 0) {
			gamebrowser::game_item game = games_menu_.selected_game();

			std::string password;
			if(join && game.password_required) {
				if(!gui2::tmp_join_game_password_prompt::execute(password, disp_.video())) {
					return;
				}
			}

			config response;
			config& join = response.add_child("join");
			join["id"] = game.id;
			join["observe"] = observe;

			if(!password.empty()) {
				join["password"] = password;
			}
			network::send_data(response, 0);

			if(observe) {
				this->current_turn = game.current_turn;
				set_result(OBSERVE);
			} else {
				set_result(JOIN);
			}
		}
		return;
	}

	if(data.create) {
		set_result(CREATE);
		return;
	}

	if(game_preferences_.pressed()) {
		set_result(PREFERENCES);
		return;
	}

	if(data.quit) {
		set_result(QUIT);
		return;
	}

	if(apply_filter_.pressed()) {
		preferences::set_filter_lobby(apply_filter_.checked());
		invert_filter_.enable(apply_filter_.checked());
		vacant_slots_.enable(apply_filter_.checked());
		friends_in_game_.enable(apply_filter_.checked());
		filter_label_.enable(apply_filter_.checked());

		/** @todo I am aware that the box is not grayed out even though
                  it definitely should be. This is because the textbox
                  class currently does not really have an easy way to do
                  that. I'll have to look into this.
		*/
		filter_text_.enable(apply_filter_.checked());
		gamelist_updated();
		return;
	}

	if(invert_filter_.pressed()) {
		preferences::set_fi_invert(invert_filter_.checked());
		gamelist_updated();
		return;
	}

	if(vacant_slots_.pressed()) {
		preferences::set_fi_vacant_slots(vacant_slots_.checked());
		gamelist_updated();
		return;
	}

	if(friends_in_game_.pressed()) {
		preferences::set_fi_friends_in_game(friends_in_game_.checked());
		gamelist_updated();
		return;
	}

	if(search_string_ != filter_text_.text()) {
	    search_string_ = filter_text_.text();
	    preferences::set_fi_text(search_string_);
	    gamelist_updated();
	}

}

bool lobby::plugin_event_helper(const process_event_data & data)
{
	process_event_impl(data);
	return get_result() == mp::ui::CONTINUE;
}

void lobby::process_network_data(const config& data, const network::connection sock)
{
	ui::process_network_data(data, sock);

	// Invalidate game selection for the player list
	last_selected_game_ = -1;
}

} // end namespace mp

