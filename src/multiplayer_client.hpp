#ifndef MULTIPLAYER_CLIENT_HPP_INCLUDED
#define MULTIPLAYER_CLIENT_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "network.hpp"
#include "unit_types.hpp"
#include "show_dialog.hpp"
#include "widgets/combo.hpp"

//function to play a game as a client, joining either another player who is
//hosting a game, or connecting to a server.
void play_multiplayer_client(display& disp, game_data& units_data, config& cfg,
                             game_state& state, std::string& host);


class leader_list_manager
{
public:
	leader_list_manager(const config::child_list& side_list, const game_data* data,
			gui::combo* combo = NULL);

	void set_combo(gui::combo* combo);
	void update_leader_list(int side);
	std::string get_leader();
	void set_leader(const std::string& leader);
	bool is_leader_ok(std::string leader);

private:
	std::vector<std::string> leaders_;
	config::child_list side_list_;
	const game_data* data_;
	gui::combo* combo_;

};

class leader_preview_pane : public gui::preview_pane
{
public:
	leader_preview_pane(display& disp, const game_data* data, 
			const config::child_list& side_list);

	bool show_above() const;
	bool left_side() const;
	void set_selection(int index);
	std::string get_selected_leader();

private:
	virtual void draw_contents();
	virtual void process_event();

	const config::child_list side_list_;
	gui::combo leader_combo_; // Must appear before the leader_list_manager
	leader_list_manager leaders_;
	int selection_;
	const game_data* data_;
};

#endif
