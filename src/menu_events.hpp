#ifndef MENU_EVENTS_H_INCLUDED
#define MENU_EVENTS_H_INCLUDED

#include "global.hpp"

#include "dialogs.hpp"
#include "gamestatus.hpp"
#include "statistics.hpp"

namespace events{

class menu_handler{
public:
	void objectives(display& gui, const config& level, team& current_team);
	void show_statistics(display& gui, const game_data& gameinfo);
	void unit_list(const unit_map& units, display& gui, const gamemap& map);
	void status_table(std::vector<team>& teams, display& gui, const unit_map& units);
	void save_game(const std::string& message, gui::DIALOG_TYPE dialog_type,
		const game_state& gamestate, const gamestatus& status, display& gui,
		const config& level, std::vector<team>& teams, const unit_map& units, const gamemap& map);
	void load_game(display& gui, const config& terrain_config, const game_data& gameinfo);
	void preferences(display& gui, const config& terrain_config);
	void show_chat_log(std::vector<team>& teams, display& gui);
	void show_help(display& gui);
private:
	std::vector<std::string> create_unit_table(const statistics::stats::str_int_map& m, const game_data& gameinfo);
	void write_game_snapshot(config& start, const config& level, display& gui,
		std::vector<team>& teams, const unit_map& units, const gamestatus& status,
		const game_state& gamestate, const gamemap& map) const;
};

}
#endif
