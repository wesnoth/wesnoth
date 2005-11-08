#include "mouse_events.hpp"

#include "cursor.hpp"

namespace events{

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
	mouse_motion(event.x,event.y, player_number);
}

void mouse_handler::mouse_motion(int x, int y, const int player_number)
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

		const unit_map::const_iterator selected_unit = find_unit(selected_hex_, player_number);
		const unit_map::const_iterator mouseover_unit = find_unit(new_hex, player_number);

		gamemap::location attack_from;
		if(selected_unit != units_.end() && mouseover_unit != units_.end()) {
			attack_from = current_unit_attacks_from(new_hex, nearest_hex, second_nearest_hex, player_number);
		}

		team& current_team = teams_[player_number - 1];
		if(selected_unit != units_.end() && (current_paths_.routes.count(new_hex) ||
		                                     attack_from.valid())) {
			if(mouseover_unit == units_.end()) {
				cursor::set(cursor::MOVE);
			} else if(current_team.is_enemy(mouseover_unit->second.side()) && !mouseover_unit->second.stone()) {
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
		const unit_map::const_iterator dest_un = find_unit(dest, player_number);
		if(dest == selected_hex_ || dest_un != units_.end()) {
			current_route_.steps.clear();
			(*gui_).set_route(NULL);
		} else if(!current_paths_.routes.empty() && map_.on_board(selected_hex_) &&
		   map_.on_board(new_hex)) {

			unit_map::const_iterator un = find_unit(selected_hex_, player_number);

			if((new_hex != last_hex_ || attack_from.valid()) && un != units_.end() && !un->second.stone()) {
				const shortest_path_calculator calc(un->second,current_team,
				                                    visible_units(player_number),teams_,map_,status_);
				const bool can_teleport = un->second.type().teleports();

				const std::set<gamemap::location>* teleports = NULL;

				std::set<gamemap::location> allowed_teleports;
				if(can_teleport) {
					allowed_teleports = vacant_villages(current_team.villages(),units_);
					teleports = &allowed_teleports;
					if(current_team.villages().count(un->first))
						allowed_teleports.insert(un->first);
				}

				current_route_ = a_star_search(selected_hex_, dest, 10000.0, &calc, map_.x(), map_.y(), teleports);

				current_route_.move_left = route_turns_to_complete(un->second,map_,current_route_);

				if(!browse_) {
					(*gui_).set_route(&current_route_);
				}
			}
		}

		unit_map::const_iterator un = find_unit(new_hex, player_number);

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

unit_map::const_iterator mouse_handler::find_unit(const gamemap::location& hex, const int player_number)
{
	if ((*gui_).fogged(hex.x,hex.y)) {
		return units_.end();
	}

	return find_visible_unit(units_,hex,map_,status_.get_time_of_day().lawful_bonus,teams_,viewing_team(player_number));
}

gamemap::location mouse_handler::current_unit_attacks_from(const gamemap::location& loc, const gamemap::location::DIRECTION preferred, const gamemap::location::DIRECTION second_preferred, const int player_number)
{
	const unit_map::const_iterator current = find_unit(selected_hex_, player_number);
	if(current == units_.end() || current->second.side() != player_number) {
		return gamemap::location();
	}

	team& current_team = teams_[player_number - 1];
	const unit_map::const_iterator enemy = find_unit(loc, player_number);
	if(enemy == units_.end() || current_team.is_enemy(enemy->second.side()) == false) {
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

		if(find_unit(adj[n], player_number) != units_.end()) {
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

unit_map& mouse_handler::visible_units(const int player_number)
{
	if(viewing_team(player_number).uses_shroud() == false && viewing_team(player_number).uses_fog() == false) {
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

}