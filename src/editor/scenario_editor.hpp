#ifndef SCENARIO_EDITOR_HPP_INCLUDED
#define SCENARIO_EDITOR_HPP_INCLUDED

#include "../config.hpp"
#include "../events.hpp"
#include "../hotkeys.hpp"
#include "../scoped_resource.hpp"
#include "../team.hpp"
#include "../unit.hpp"
#include "../unit_types.hpp"
#include "../video.hpp"

class scenario_editor : public hotkey::command_executor, public events::handler
{
public:
	scenario_editor(game_data& gameinfo, const config& game_config, CVideo& video);

private:

	void handle_event(const SDL_Event& event);

	void cycle_units();
	void end_turn() {}
	void goto_leader();
	void end_unit_turn() {}
	void undo();
	void redo();
	void terrain_table();
	void attack_resistance();
	void unit_description();
	void rename_unit();
	void save_game();
	void toggle_grid();
	void status_table();
	void recall() {}
	void recruit() {}
	void repeat_recruit() {}
	void speak() {}
	void create_unit();
	void preferences();
	void objectives();
	void unit_list();
	void show_statistics() {}
	void label_terrain();
	void show_enemy_moves(bool ignore_units) {}

	void edit_set_terrain();

	game_data& gameinfo_;
	const config& game_config_;
	CVideo& video_;

	util::scoped_ptr<display> disp_;
	util::scoped_ptr<gamemap> map_;
	util::scoped_ptr<gamestatus> status_;
	config scenario_;

	unit_map units_;
	std::vector<team> teams_;
};

#endif