#include "mouse_events.hpp"

#include "cursor.hpp"
#include "preferences_display.hpp"
#include "wassert.hpp"

namespace events{

int commands_disabled = 0;

bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState()&KMOD_META) != 0;
#else
	return false;
#endif
}

mouse_handler::mouse_handler(display* gui, std::vector<team>& teams, unit_map& units, gamemap& map, 
							 gamestatus& status, const game_data& gameinfo):
gui_(gui), teams_(teams), units_(units), map_(map), status_(status), gameinfo_(gameinfo)
{
	minimap_scrolling_ = false;
	last_nearest_ = gamemap::location::NORTH;
	last_second_nearest_ = gamemap::location::NORTH;
	enemy_paths_ = false;
	browse_ = false;
	path_turns_ = 0;
}

void mouse_handler::mouse_motion(const SDL_MouseMotionEvent& event, const int player_number)
{
	team_num_ = player_number;
	mouse_motion(event.x,event.y);
}

void mouse_handler::mouse_motion(int x, int y)
{
	if(minimap_scrolling_) {
		//if the game is run in a window, we could miss a LMB/MMB up event
		// if it occurs outside our window.
		// thus, we need to check if the LMB/MMB is still down
		minimap_scrolling_ = ((SDL_GetMouseState(NULL,NULL) & (SDL_BUTTON(1) | SDL_BUTTON(2))) != 0);
		if(minimap_scrolling_) {
			const gamemap::location& loc = (*gui_).minimap_location_on(x,y);
			if(loc.valid()) {
				if(loc != last_hex_) {
					last_hex_ = loc;
					(*gui_).scroll_to_tile(loc.x,loc.y,display::WARP,false);
				}
			} else {
				// clicking outside of the minimap will end minimap scrolling
				minimap_scrolling_ = false;
			}
		}
		if(minimap_scrolling_) return;
	}

	gamemap::location::DIRECTION nearest_hex = gamemap::location::NDIRECTIONS;
	gamemap::location::DIRECTION second_nearest_hex = gamemap::location::NDIRECTIONS;
	const gamemap::location new_hex = (*gui_).hex_clicked_on(x,y,&nearest_hex,&second_nearest_hex);

	if(new_hex != last_hex_ || nearest_hex != last_nearest_ || second_nearest_hex != last_second_nearest_) {
		if(new_hex.valid() == false) {
			current_route_.steps.clear();
			(*gui_).set_route(NULL);
		}

		(*gui_).highlight_hex(new_hex);


		//see if we should show the normal cursor, the movement cursor, or
		//the attack cursor

		const unit_map::const_iterator selected_unit = find_unit(selected_hex_);
		const unit_map::const_iterator mouseover_unit = find_unit(new_hex);

		gamemap::location attack_from;
		if(selected_unit != units_.end() && mouseover_unit != units_.end()) {
			attack_from = current_unit_attacks_from(new_hex, nearest_hex, second_nearest_hex);
		}

		if(selected_unit != units_.end() && (current_paths_.routes.count(new_hex) ||
		                                     attack_from.valid())) {
			if(mouseover_unit == units_.end()) {
				cursor::set(cursor::MOVE);
			} else if(current_team().is_enemy(mouseover_unit->second.side()) && !mouseover_unit->second.stone()) {
				cursor::set(cursor::ATTACK);
			} else {
				cursor::set(cursor::NORMAL);
			}
		} else {
			cursor::set(cursor::NORMAL);
		}

		if(enemy_paths_) {
			enemy_paths_ = false;
			current_paths_ = paths();
			(*gui_).set_paths(NULL);
		}

		const gamemap::location& dest = attack_from.valid() ? attack_from : new_hex;
		const unit_map::const_iterator dest_un = find_unit(dest);
		if(dest == selected_hex_ || dest_un != units_.end()) {
			current_route_.steps.clear();
			(*gui_).set_route(NULL);
		} else if(!current_paths_.routes.empty() && map_.on_board(selected_hex_) &&
		   map_.on_board(new_hex)) {

			unit_map::const_iterator un = find_unit(selected_hex_);

			if((new_hex != last_hex_ || attack_from.valid()) && un != units_.end() && !un->second.stone()) {
				const shortest_path_calculator calc(un->second,current_team(), visible_units(),teams_,map_,status_);
				const bool can_teleport = un->second.type().teleports();

				const std::set<gamemap::location>* teleports = NULL;

				std::set<gamemap::location> allowed_teleports;
				if(can_teleport) {
					allowed_teleports = vacant_villages(current_team().villages(),units_);
					teleports = &allowed_teleports;
					if(current_team().villages().count(un->first))
						allowed_teleports.insert(un->first);
				}

				current_route_ = a_star_search(selected_hex_, dest, 10000.0, &calc, map_.x(), map_.y(), teleports);

				current_route_.move_left = route_turns_to_complete(un->second,map_,current_route_);

				if(!browse_) {
					(*gui_).set_route(&current_route_);
				}
			}
		}

		unit_map::const_iterator un = find_unit(new_hex);

		if(un != units_.end() && /* un->second.side() != player_number && */ 
		   current_paths_.routes.empty() && !(*gui_).fogged(un->first.x,un->first.y)) {
			//Quick-Hack because of problems with passing a constant Reference
			unit un2 = un->second;
			unit_movement_resetter move_reset(un2);

			const bool ignore_zocs = un->second.type().is_skirmisher();
			const bool teleport = un->second.type().teleports();
			current_paths_ = paths(map_,status_,gameinfo_,units_,new_hex,teams_,
			                   ignore_zocs,teleport,path_turns_);
			(*gui_).set_paths(&current_paths_);
			enemy_paths_ = true;
		}
	}

	last_hex_ = new_hex;
	last_nearest_ = nearest_hex;
	last_second_nearest_ = second_nearest_hex;
}

unit_map::iterator mouse_handler::find_unit(const gamemap::location& hex)
{
	if ((*gui_).fogged(hex.x,hex.y)) {
		return units_.end();
	}

	return find_visible_unit(units_,hex,map_,status_.get_time_of_day().lawful_bonus,teams_,viewing_team());
}

gamemap::location mouse_handler::current_unit_attacks_from(const gamemap::location& loc, const gamemap::location::DIRECTION preferred, const gamemap::location::DIRECTION second_preferred)
{
	const unit_map::const_iterator current = find_unit(selected_hex_);
	if(current == units_.end() || current->second.side() != team_num_) {
		return gamemap::location();
	}

	const unit_map::const_iterator enemy = find_unit(loc);
	if(enemy == units_.end() || current_team().is_enemy(enemy->second.side()) == false) {
		return gamemap::location();
	}

	int best_rating = 100;//smaller is better
	gamemap::location res;
	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if(map_.on_board(adj[n]) == false) {
			continue;
		}

		if(adj[n] == selected_hex_) {
			return selected_hex_;
		}

		if(find_unit(adj[n]) != units_.end()) {
			continue;
		}

		if(current_paths_.routes.count(adj[n])) {
			static const size_t NDIRECTIONS = gamemap::location::NDIRECTIONS;
			int difference = abs(int(preferred - n));
			if(difference > NDIRECTIONS/2) {
				difference = NDIRECTIONS - difference;
			}
			int second_difference = abs(int(second_preferred - n));
			if(second_difference > NDIRECTIONS/2) {
				second_difference = NDIRECTIONS - second_difference;
			}
			const int rating = difference * 2 + (second_difference > difference);
			if(rating < best_rating || res.valid() == false) {
				best_rating = rating;
				res = adj[n];
			}
		}
	}

	return res;
}

unit_map& mouse_handler::visible_units()
{
	if(viewing_team().uses_shroud() == false && viewing_team().uses_fog() == false) {
		LOG_STREAM(info, engine) << "all units are visible...\n";
		return units_;
	}

	visible_units_.clear();
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
		if((*gui_).fogged(i->first.x,i->first.y) == false) {
			visible_units_.insert(*i);
		}
	}

	LOG_STREAM(info, engine) << "number of visible units: " << visible_units_.size() << "\n";

	return visible_units_;
}

void mouse_handler::mouse_press(const SDL_MouseButtonEvent& event, const int player_number)
{
	team_num_ = player_number;
	mouse_motion(event.x, event.y);

	if(is_left_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_middle_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_left_click(event) && event.state == SDL_PRESSED) {
		left_click(event);
	} else if(is_right_click(event) && event.state == SDL_PRESSED) {
		if(!current_paths_.routes.empty()) {
			selected_hex_ = gamemap::location();
			gui_->select_hex(gamemap::location());
			gui_->set_paths(NULL);
			current_paths_ = paths();
			current_route_.steps.clear();
			gui_->set_route(NULL);

			cursor::set(cursor::NORMAL);
		} else {
			gui_->draw(); // redraw highlight (and maybe some more)
			const theme::menu* const m = gui_->get_theme().context_menu();
			if (m != NULL)
				;//show_menu(m->items(),event.x,event.y,true);
			else
				LOG_STREAM(warn, display) << "no context menu found...\n";
		}
	} else if(is_middle_click(event) && event.state == SDL_PRESSED) {
		// clicked on a hex on the minimap? then initiate minimap scrolling
		const gamemap::location& loc = gui_->minimap_location_on(event.x,event.y);
		minimap_scrolling_ = false;
		if(loc.valid()) {
			minimap_scrolling_ = true;
			last_hex_ = loc;
			gui_->scroll_to_tile(loc.x,loc.y,display::WARP,false);
			return;
		} else {
		const SDL_Rect& rect = gui_->map_area();
		const int centerx = (rect.x + rect.w)/2;
		const int centery = (rect.y + rect.h)/2;

		const int xdisp = event.x - centerx;
		const int ydisp = event.y - centery;

		gui_->scroll(xdisp,ydisp);
		}
	} else if((event.button == SDL_BUTTON_WHEELUP ||
		event.button == SDL_BUTTON_WHEELDOWN) && !commands_disabled) {
		const int speed = preferences::scroll_speed() *
			(event.button == SDL_BUTTON_WHEELUP ? -1:1);

		const int centerx = gui_->mapx()/2;
		const int centery = gui_->y()/2;

		const int xdisp = abs(centerx - event.x);
		const int ydisp = abs(centery - event.y);

		if(xdisp > ydisp)
			gui_->scroll(speed,0);
		else
			gui_->scroll(0,speed);
	}
}

bool mouse_handler::is_left_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_LEFT && !command_active();
}

bool mouse_handler::is_middle_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_MIDDLE;
}

bool mouse_handler::is_right_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_RIGHT || event.button == SDL_BUTTON_LEFT && command_active();
}

void mouse_handler::left_click(const SDL_MouseButtonEvent& event)
{
	if(commands_disabled) {
		return;
	}

	// clicked on a hex on the minimap? then initiate minimap scrolling
	const gamemap::location& loc = gui_->minimap_location_on(event.x,event.y);
	minimap_scrolling_ = false;
	if(loc.valid()) {
		minimap_scrolling_ = true;
		last_hex_ = loc;
		gui_->scroll_to_tile(loc.x,loc.y,display::WARP,false);
		return;
	}

	gamemap::location::DIRECTION nearest_hex, second_nearest_hex;
	gamemap::location hex = gui_->hex_clicked_on(event.x,event.y,&nearest_hex,&second_nearest_hex);

	unit_map::iterator u = find_unit(selected_hex_);

	//if the unit is selected and then itself clicked on,
	//any goto command is cancelled
	if(u != units_.end() && !browse_ && selected_hex_ == hex && u->second.side() == team_num_) {
		u->second.set_goto(gamemap::location());
	}

	//if we can move to that tile
	std::map<gamemap::location,paths::route>::const_iterator
			route = enemy_paths_ ? current_paths_.routes.end() :
	                               current_paths_.routes.find(hex);

	unit_map::iterator enemy = find_unit(hex);

	const gamemap::location src = selected_hex_;
	paths orig_paths = current_paths_;

	{
		gui_->set_paths(NULL);
		current_paths_ = paths();

		selected_hex_ = hex;
		gui_->select_hex(hex);
		current_route_.steps.clear();
		gui_->set_route(NULL);

		const unit_map::iterator it = find_unit(hex);

		if(it != units_.end() && it->second.side() == team_num_ && !gui_->fogged(it->first.x,it->first.y)) {
			const bool ignore_zocs = it->second.type().is_skirmisher();
			const bool teleport = it->second.type().teleports();
			current_paths_ = paths(map_,status_,gameinfo_,units_,hex,teams_,
			                   ignore_zocs,teleport,path_turns_);

			next_unit_ = it->first;

			show_attack_options(it);

			gui_->set_paths(&current_paths_);

			unit u = it->second;
			const gamemap::location go_to = u.get_goto();
			if(map_.on_board(go_to)) {
				const shortest_path_calculator calc(u,current_team(),
				                                    visible_units(),teams_,map_,status_);

				const std::set<gamemap::location>* teleports = NULL;

				std::set<gamemap::location> allowed_teleports;
				if(u.type().teleports()) {
					allowed_teleports = vacant_villages(current_team().villages(),units_);
					teleports = &allowed_teleports;
					if(current_team().villages().count(it->first))
						allowed_teleports.insert(it->first);

				}

				paths::route route = a_star_search(it->first, go_to, 10000.0, &calc, map_.x(), map_.y(), teleports);
				route.move_left = route_turns_to_complete(it->second,map_,route);
				gui_->set_route(&route);
			}
		}
	}
}

void mouse_handler::show_attack_options(unit_map::const_iterator u)
{
	team& current_team = teams_[team_num_-1];

	if(u == units_.end() || u->second.can_attack() == false)
		return;

	for(unit_map::const_iterator target = units_.begin(); target != units_.end(); ++target) {
		if(current_team.is_enemy(target->second.side()) &&
			distance_between(target->first,u->first) == 1 && !target->second.stone()) {
			current_paths_.routes[target->first] = paths::route();
		}
	}
}

}
