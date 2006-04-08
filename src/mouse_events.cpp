#include "mouse_events.hpp"

#include "cursor.hpp"
#include "dialogs.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"

namespace events{

int commands_disabled = 0;

command_disabler::command_disabler()
{
	++commands_disabled;
}

command_disabler::~command_disabler()
{
	--commands_disabled;
}

bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState()&KMOD_META) != 0;
#else
	return false;
#endif
}

namespace{
	//which attack is the better one to select for the player by default
	//(the player can change the selected weapon if desired)
	class simple_attack_rating
	{
	public:
		simple_attack_rating() {}
		simple_attack_rating(const battle_stats& stats) : stats_(stats) {}

		bool operator<(const simple_attack_rating& a) const
		{
			//if our weapon can kill the enemy in one blow, the enemy does not
			//drain back and our weapon has more blows, prefer our weapon
			if(stats_.damage_defender_takes >= stats_.defender_hp &&
			   stats_.amount_defender_drains == 0 &&
			   stats_.nattacks > a.stats_.nattacks)
				{
				return false;
				}

			int this_avg_damage_dealt = stats_.chance_to_hit_defender *
					stats_.damage_defender_takes * stats_.nattacks;
			int this_avg_damage_taken = stats_.chance_to_hit_attacker *
					stats_.damage_attacker_takes * stats_.ndefends;

			int other_avg_damage_dealt = a.stats_.chance_to_hit_defender *
					a.stats_.damage_defender_takes * a.stats_.nattacks;
			int other_avg_damage_taken = a.stats_.chance_to_hit_attacker *
					a.stats_.damage_attacker_takes * a.stats_.ndefends;

			//if our weapon does less damage, it's worse
			if(this_avg_damage_dealt < other_avg_damage_dealt)
				return true;

			//if both weapons are the same but
			//ours makes the enemy retaliate for more damage, it's worse
			else if(this_avg_damage_dealt == other_avg_damage_dealt &&
				this_avg_damage_taken > other_avg_damage_taken)
				return true;

			//otherwise, ours is at least as good a default weapon
			return false;
		}
	private:
		battle_stats stats_;
	};

	class attack_calculations_displayer : public gui::dialog_button_action
	{
	public:
		typedef std::vector< battle_stats_strings > stats_vector;
		attack_calculations_displayer(display &disp, stats_vector const &stats)
			: disp_(disp), stats_(stats)
		{}

		RESULT button_pressed(int selection);
	private:
		display &disp_;
		stats_vector const &stats_;
	};

	gui::dialog_button_action::RESULT attack_calculations_displayer::button_pressed(int selection)
	{
		const size_t index = size_t(selection);
		if(index < stats_.size()) {
			battle_stats_strings const &sts = stats_[index];
			std::vector< std::string > sts_att = sts.attack_calculations,
									   sts_def = sts.defend_calculations,
									   calcs;
			unsigned sts_att_sz = sts_att.size(),
					 sts_def_sz = sts_def.size(),
					 sts_sz = maximum< unsigned >(sts_att_sz, sts_def_sz);

			std::stringstream str;
			str << _("Attacker") << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR;
			if (sts_def_sz > 0)
				str << _("Defender");
			calcs.push_back(str.str());

			for(unsigned i = 0; i < sts_sz; ++i) {
				std::stringstream str;
				if (i < sts_att_sz)
					str << sts_att[i];
				else
					str << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR << ' ';

				str << COLUMN_SEPARATOR;

				if (i < sts_def_sz)
					str << sts_def[i];
				else
					str << ' ' << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR << ' ';

				calcs.push_back(str.str());
			}

			gui::show_dialog(disp_, NULL, "", _("Damage Calculations"), gui::OK_ONLY, &calcs);
		}

		return NO_EFFECT;
	}
} //end anonymous namespace

mouse_handler::mouse_handler(display* gui, std::vector<team>& teams, unit_map& units, gamemap& map,
				gamestatus& status, const game_data& gameinfo, undo_list& undo_stack, undo_list& redo_stack):
gui_(gui), teams_(teams), units_(units), map_(map), status_(status), gameinfo_(gameinfo),
undo_stack_(undo_stack), redo_stack_(redo_stack)
{
	minimap_scrolling_ = false;
	last_nearest_ = gamemap::location::NORTH;
	last_second_nearest_ = gamemap::location::NORTH;
	enemy_paths_ = false;
	path_turns_ = 0;
	undo_ = false;
	show_menu_ = false;
}

void mouse_handler::mouse_motion(const SDL_MouseMotionEvent& event, const int player_number, const bool browse)
{
	team_num_ = player_number;
	mouse_motion(event.x,event.y, browse);
}

void mouse_handler::mouse_motion(int x, int y, const bool browse)
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
			} else if(viewing_team().is_enemy(mouseover_unit->second.side()) && mouseover_unit->second.get_state("stoned")!="yes") {
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
			gui_->unhighlight_reach();
		}

		const gamemap::location& dest = attack_from.valid() ? attack_from : new_hex;
		const unit_map::const_iterator dest_un = find_unit(dest);
		if(dest == selected_hex_ || dest_un != units_.end()) {
			current_route_.steps.clear();
			(*gui_).set_route(NULL);
		} else if(!current_paths_.routes.empty() && map_.on_board(selected_hex_) &&
		   map_.on_board(new_hex)) {

			unit_map::const_iterator un = find_unit(selected_hex_);

			if((new_hex != last_hex_ || attack_from.valid()) && un != units_.end() && un->second.get_state("stoned")!="yes") {
				const shortest_path_calculator calc(un->second,current_team(), visible_units(),teams_,map_,status_);
				const bool can_teleport = un->second.get_ability_bool("teleport",un->first);

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

				if(!browse) {
					(*gui_).set_route(&current_route_);
				}
			}
		}

		unit_map::iterator un = find_unit(new_hex);

		if(un != units_.end() && un->second.side() != team_num_ && 
			current_paths_.routes.empty() && !(*gui_).fogged(un->first.x,un->first.y)) {
			unit_movement_resetter move_reset(un->second);

			const bool ignore_zocs = un->second.get_ability_bool("skirmisher",un->first);
			const bool teleport = un->second.get_ability_bool("teleport",un->first);
			current_paths_ = paths(map_,status_,gameinfo_,units_,new_hex,teams_,
								   ignore_zocs,teleport,viewing_team(),path_turns_);
			gui_->highlight_reach(current_paths_);
			enemy_paths_ = true;
		}
	}

	last_hex_ = new_hex;
	last_nearest_ = nearest_hex;
	last_second_nearest_ = second_nearest_hex;
}

unit_map::iterator mouse_handler::selected_unit()
{
	unit_map::iterator res = find_unit(selected_hex_);
	if(res != units_.end()) {
		return res;
	} else {
		return find_unit(last_hex_);
	}
}

unit_map::iterator mouse_handler::find_unit(const gamemap::location& hex)
{
	return find_visible_unit(units_,hex,map_,status_.get_time_of_day().lawful_bonus,teams_,viewing_team());
}

unit_map::const_iterator mouse_handler::find_unit(const gamemap::location& hex) const
{
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
			unsigned int difference = abs(int(preferred - n));
			if(difference > NDIRECTIONS/2) {
				difference = NDIRECTIONS - difference;
			}
			unsigned int second_difference = abs(int(second_preferred - n));
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

const unit_map& mouse_handler::visible_units()
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

void mouse_handler::mouse_press(const SDL_MouseButtonEvent& event, const int player_number, const bool browse)
{
	show_menu_ = false;
	team_num_ = player_number;
	mouse_motion(event.x, event.y, browse);

	if(is_left_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_middle_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_left_click(event) && event.state == SDL_PRESSED) {
		left_click(event, browse);
	} else if(is_right_click(event) && event.state == SDL_PRESSED) {
		// FIXME: when it's not our turn, movement gets highlighted
		// merely by mousing over.  This hack means we don't require a
		// two clicks to access right menu.
		if (gui_->viewing_team() == team_num_-1 && !current_paths_.routes.empty()) {
			selected_hex_ = gamemap::location();
			gui_->select_hex(gamemap::location());
			gui_->unhighlight_reach();
			current_paths_ = paths();
			current_route_.steps.clear();
			gui_->set_route(NULL);

			cursor::set(cursor::NORMAL);
		} else {
			gui_->draw(); // redraw highlight (and maybe some more)
			const theme::menu* const m = gui_->get_theme().context_menu();
			if (m != NULL)
				show_menu_ = true;
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

void mouse_handler::left_click(const SDL_MouseButtonEvent& event, const bool browse)
{
	undo_ = false;
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
	if(u != units_.end() && !browse && selected_hex_ == hex && u->second.side() == team_num_) {
		((unit) u->second).set_goto(gamemap::location());
	}

	//if we can move to that tile
	std::map<gamemap::location,paths::route>::const_iterator
			route = enemy_paths_ ? current_paths_.routes.end() :
	                               current_paths_.routes.find(hex);

	unit_map::iterator enemy = find_unit(hex);

	const gamemap::location src = selected_hex_;
	paths orig_paths = current_paths_;

	//see if we're trying to do a move-and-attack
	if(!browse && u != units_.end() && enemy != units_.end() && !current_route_.steps.empty()) {
		const gamemap::location& attack_from = current_unit_attacks_from(hex, nearest_hex, second_nearest_hex);
		if(attack_from.valid()) {
			if(move_unit_along_current_route(false)) { //move the unit without updating shroud
				u = find_unit(attack_from);
				// enemy = find_unit(hex);
				if(u != units_.end() && u->second.side() == team_num_ &&
					enemy != units_.end() && current_team().is_enemy(enemy->second.side()) && enemy->second.get_state("stoned")!="yes") {
					if(attack_enemy(u,enemy) == false) {
						undo_ = true;
						selected_hex_ = src;
						gui_->select_hex(src);
						current_paths_ = orig_paths;
						gui_->highlight_reach(current_paths_);
						return;
					}
				}
			}

			if(clear_shroud(*gui_, status_, map_, gameinfo_, units_, teams_, team_num_ - 1)) {
				clear_undo_stack();
			}

			return;
		}
	}

	//see if we're trying to attack an enemy
	if(u != units_.end() && route != current_paths_.routes.end() && enemy != units_.end() &&
	   hex != selected_hex_ && !browse &&
	   enemy->second.side() != u->second.side() &&
	   current_team().is_enemy(enemy->second.side())) {
		attack_enemy(u,enemy);
	}

	//otherwise we're trying to move to a hex
	else if(!browse && selected_hex_.valid() && selected_hex_ != hex &&
		     units_.count(selected_hex_) && !enemy_paths_ &&
		     enemy == units_.end() && !current_route_.steps.empty() &&
		     current_route_.steps.front() == selected_hex_) {
		move_unit_along_current_route();
		if(clear_shroud(*gui_, status_, map_, gameinfo_, units_, teams_, team_num_ - 1)) {
			clear_undo_stack();
		}
	} else {
		gui_->unhighlight_reach();
		current_paths_ = paths();

		selected_hex_ = hex;
		gui_->select_hex(hex);
		current_route_.steps.clear();
		gui_->set_route(NULL);

		const unit_map::iterator it = find_unit(hex);

		if(it != units_.end() && it->second.side() == team_num_ && !gui_->fogged(it->first.x,it->first.y)) {
			const bool ignore_zocs = it->second.get_ability_bool("skirmisher",it->first);
			const bool teleport = it->second.get_ability_bool("teleport",it->first);
			current_paths_ = paths(map_,status_,gameinfo_,units_,hex,teams_,
								   ignore_zocs,teleport,viewing_team(),path_turns_);

			next_unit_ = it->first;

			show_attack_options(it);

			gui_->highlight_reach(current_paths_);

			unit u = it->second;
			const gamemap::location go_to = u.get_goto();
			if(map_.on_board(go_to)) {
				const shortest_path_calculator calc(u,current_team(),
				                                    visible_units(),teams_,map_,status_);

				const std::set<gamemap::location>* teleports = NULL;

				std::set<gamemap::location> allowed_teleports;
				if(u.get_ability_bool("teleport",it->first)) {
					allowed_teleports = vacant_villages(current_team().villages(),units_);
					teleports = &allowed_teleports;
					if(current_team().villages().count(it->first))
						allowed_teleports.insert(it->first);

				}

				paths::route route = a_star_search(it->first, go_to, 10000.0, &calc, map_.x(), map_.y(), teleports);
				route.move_left = route_turns_to_complete(it->second,map_,route);
				gui_->set_route(&route);
			}
			game_events::fire("select",hex);
		}
	}
}

void mouse_handler::clear_undo_stack()
{
	if(teams_[team_num_ - 1].auto_shroud_updates() == false)
		apply_shroud_changes(undo_stack_,gui_,status_,map_,gameinfo_,units_,teams_,team_num_-1);
	undo_stack_.clear();
}

bool mouse_handler::move_unit_along_current_route(bool check_shroud)
{
	const std::vector<gamemap::location> steps = current_route_.steps;
	if(steps.empty()) {
		return false;
	}

	const size_t moves = ::move_unit(gui_,gameinfo_,status_,map_,units_,teams_,
	                   steps,&recorder,&undo_stack_,&next_unit_,false,check_shroud);

	cursor::set(cursor::NORMAL);

	gui_->invalidate_game_status();

	selected_hex_ = gamemap::location();
	gui_->select_hex(gamemap::location());

	gui_->set_route(NULL);
	gui_->unhighlight_reach();
	current_paths_ = paths();

	if(moves == 0)
		return false;

	redo_stack_.clear();

	wassert(moves <= steps.size());
	const gamemap::location& dst = steps[moves-1];
	const unit_map::const_iterator u = units_.find(dst);

	//u may be equal to units_.end() in the case of e.g. a [teleport]
	if(u != units_.end()) {
		//Reselect the unit if the move was interrupted
		if(dst != steps.back()) {
			selected_hex_ = dst;
			gui_->select_hex(dst);
		}

		current_route_.steps.clear();
		show_attack_options(u);

		if(current_paths_.routes.empty() == false) {
			current_paths_.routes[dst] = paths::route();
			selected_hex_ = dst;
			gui_->select_hex(dst);
			gui_->highlight_reach(current_paths_);
		}
	}

	return moves == steps.size();
}

bool mouse_handler::attack_enemy(unit_map::iterator attacker, unit_map::iterator defender)
{
	//we must get locations by value instead of by references, because the iterators
	//may become invalidated later
	const gamemap::location attacker_loc = attacker->first;
	const gamemap::location defender_loc = defender->first;

	const std::vector<attack_type>& attacks = attacker->second.attacks();
	std::vector<std::string> items;
	std::vector<int> weapons;

	int best_weapon_index = -1;
	simple_attack_rating best_weapon_rating;

	attack_calculations_displayer::stats_vector stats;

	for(size_t a = 0; a != attacks.size(); ++a) {
		// skip weapons with attack_weight=0
		if (attacks[a].attack_weight() > 0){
			weapons.push_back(a);
			battle_stats_strings sts;
			battle_stats st = evaluate_battle_stats(map_, teams_, attacker_loc, defender_loc,
		                                        a, units_, status_, gameinfo_, 0, &sts);
			stats.push_back(sts);

			simple_attack_rating weapon_rating(st);

			if (best_weapon_index < 0 || best_weapon_rating < weapon_rating) {
				best_weapon_index = items.size();
				best_weapon_rating = weapon_rating;
			}

			//if there is an attack special or defend special, we output a single space for the other unit, to make sure
			//that the attacks line up nicely.
			std::string special_pad = (sts.attack_special.empty() && sts.defend_special.empty()) ? "" : " ";

			int damage_defender_takes;
			damage_defender_takes = st.damage_defender_takes;
			int damage_attacker_takes;
			damage_attacker_takes = st.damage_attacker_takes;
			std::stringstream att;
			att << IMAGE_PREFIX << sts.attack_icon << COLUMN_SEPARATOR
			    << font::BOLD_TEXT << sts.attack_name << "\n" << damage_defender_takes << "-"
			    << st.nattacks << " " << sts.range << " (" << st.chance_to_hit_defender << "%)\n"
			    << sts.attack_special << special_pad
			    << COLUMN_SEPARATOR << _("vs") << COLUMN_SEPARATOR
			    << font::BOLD_TEXT << sts.defend_name << "\n" << damage_attacker_takes << "-"
			    << st.ndefends << " " << sts.range << " (" << st.chance_to_hit_attacker << "%)\n"
			    << sts.defend_special << special_pad << COLUMN_SEPARATOR
			    << IMAGE_PREFIX << sts.defend_icon;

			items.push_back(att.str());
		}
	}

	if (best_weapon_index >= 0) {
		items[best_weapon_index] = DEFAULT_ITEM + items[best_weapon_index];
	}

	//make it so that when we attack an enemy, the attacking unit
	//is again shown in the status bar, so that we can easily
	//compare between the attacking and defending unit
	gui_->highlight_hex(gamemap::location());
	gui_->draw(true,true);

	attack_calculations_displayer calc_displayer(*gui_,stats);
	std::vector<gui::dialog_button> buttons;
	buttons.push_back(gui::dialog_button(&calc_displayer,_("Damage Calculations")));

	int res = 0;

	{
		const events::event_context dialog_events_context;
		dialogs::unit_preview_pane attacker_preview(*gui_,&map_,attacker->second,dialogs::unit_preview_pane::SHOW_BASIC,true);
		dialogs::unit_preview_pane defender_preview(*gui_,&map_,defender->second,dialogs::unit_preview_pane::SHOW_BASIC,false);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&attacker_preview);
		preview_panes.push_back(&defender_preview);

		res = gui::show_dialog(*gui_,NULL,_("Attack Enemy"),
				_("Choose weapon:")+std::string("\n"),
				gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,NULL,NULL,-1,-1,
				NULL,&buttons);
	}

	cursor::set(cursor::NORMAL)
;
	if(size_t(res) < weapons.size()) {

		attacker->second.set_goto(gamemap::location());
		clear_undo_stack();
		redo_stack_.clear();

		current_paths_ = paths();
		gui_->unhighlight_reach();

		gui_->invalidate_all();
		gui_->draw();

		const bool defender_human = teams_[defender->second.side()-1].is_human();

		recorder.add_attack(attacker_loc,defender_loc,weapons[res]);

		//MP_COUNTDOWN grant time bonus for attacking
		current_team().set_action_bonus_count(1 + current_team().action_bonus_count());

		try {
			attack(*gui_,map_,teams_,attacker_loc,defender_loc,weapons[res],units_,status_,gameinfo_);
		} catch(end_level_exception&) {
			//if the level ends due to a unit being killed, still see if
			//either the attacker or defender should advance
			dialogs::advance_unit(gameinfo_,map_,units_,attacker_loc,*gui_);
			dialogs::advance_unit(gameinfo_,map_,units_,defender_loc,*gui_,!defender_human);
			throw;
		}

		dialogs::advance_unit(gameinfo_,map_,units_,attacker_loc,*gui_);
		dialogs::advance_unit(gameinfo_,map_,units_,defender_loc,*gui_,!defender_human);

		selected_hex_ = gamemap::location();
		current_route_.steps.clear();
		gui_->set_route(NULL);

		check_victory(units_,teams_);

		gui_->invalidate_all();
		gui_->draw(); //clear the screen

		return true;
	} else {
		return false;
	}
}

void mouse_handler::show_attack_options(unit_map::const_iterator u)
{
	team& current_team = teams_[team_num_-1];

	if(u == units_.end() || u->second.attacks_left() == 0)
		return;

	for(unit_map::const_iterator target = units_.begin(); target != units_.end(); ++target) {
		if(current_team.is_enemy(target->second.side()) &&
			distance_between(target->first,u->first) == 1 && target->second.get_state("stoned")!="yes") {
			current_paths_.routes[target->first] = paths::route();
		}
	}
}

bool mouse_handler::unit_in_cycle(unit_map::const_iterator it)
{
	if(it->second.side() == team_num_ && unit_can_move(it->first,units_,map_,teams_) && it->second.user_end_turn() == false && !gui_->fogged(it->first.x,it->first.y)) {
		bool is_enemy = current_team().is_enemy(int(gui_->viewing_team()+1));
		return is_enemy == false || it->second.invisible(map_.underlying_union_terrain(it->first),status_.get_time_of_day().lawful_bonus,it->first,units_,teams_) == false;
	}

	return false;

}

void mouse_handler::cycle_units()
{
	unit_map::const_iterator it = units_.find(next_unit_);
	if(it != units_.end()) {
		for(++it; it != units_.end(); ++it) {
			if(unit_in_cycle(it)) {
				break;
			}
		}
	}

	if(it == units_.end()) {
		for(it = units_.begin(); it != units_.end(); ++it) {
			if(unit_in_cycle(it)) {
				break;
			}
		}
	}

	if(it != units_.end() && !gui_->fogged(it->first.x,it->first.y)) {
		const bool ignore_zocs = it->second.get_ability_bool("skirmisher",it->first);
		const bool teleport = it->second.get_ability_bool("teleport",it->first);
		current_paths_ = paths(map_,status_,gameinfo_,units_,it->first,teams_,ignore_zocs,teleport,viewing_team(),path_turns_);
		gui_->highlight_reach(current_paths_);

		gui_->scroll_to_tile(it->first.x,it->first.y,display::WARP);
	}

	if(it != units_.end()) {
		next_unit_ = it->first;
		selected_hex_ = next_unit_;
		gui_->select_hex(selected_hex_);
		gui_->highlight_hex(selected_hex_);
		current_route_.steps.clear();
		gui_->set_route(NULL);
		last_hex_=gamemap::location(-1,-1);
		int mousex, mousey;
		SDL_GetMouseState(&mousex, &mousey);
		mouse_motion(mousex, mousey, true);

		show_attack_options(it);
	} else {
		next_unit_ = gamemap::location();
	}
}

void mouse_handler::cycle_back_units()
{
	unit_map::const_iterator it = units_.find(next_unit_);
	if(it != units_.begin()) {
		for(--it; it != units_.begin(); --it) {
			if(unit_in_cycle(it)) {
				break;
			}
		}
	}

	if(it == units_.begin()) {
		for(it = units_.end(); it != units_.begin(); --it) {
			if(unit_in_cycle(it)) {
				break;
			}
		}
	}

	if(it != units_.begin() && !gui_->fogged(it->first.x,it->first.y)) {
		const bool ignore_zocs = it->second.get_ability_bool("skirmisher",it->first);
		const bool teleport = it->second.get_ability_bool("teleport",it->first);
		current_paths_ = paths(map_,status_,gameinfo_,units_,it->first,teams_,ignore_zocs,teleport,viewing_team(),path_turns_);
		gui_->highlight_reach(current_paths_);

		gui_->scroll_to_tile(it->first.x,it->first.y,display::WARP);
	}

	if(it != units_.begin()) {
		next_unit_ = it->first;
		selected_hex_ = next_unit_;
		gui_->select_hex(selected_hex_);
		gui_->highlight_hex(selected_hex_);
		current_route_.steps.clear();
		gui_->set_route(NULL);
		show_attack_options(it);
	} else {
		next_unit_ = gamemap::location();
	}
}

void mouse_handler::set_current_paths(paths new_paths) { 
	gui_->unhighlight_reach();
	current_paths_ = new_paths; 
	current_route_.steps.clear();
	gui_->set_route(NULL);
}

}
