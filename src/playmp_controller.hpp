/*
   Copyright (C) 2006 - 2018 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "playsingle_controller.hpp"
#include "syncmp_handler.hpp"

class turn_info;
struct mp_campaign_info;
class playmp_controller : public playsingle_controller, public syncmp_handler
{
public:
	playmp_controller(const config& level, saved_game& state_of_game,
		const config& game_config,
		const ter_data_cache & tdata, mp_campaign_info* mp_info);
	virtual ~playmp_controller();

	void maybe_linger() override;
	void process_oos(const std::string& err_msg) const override;

	void pull_remote_choice() override;
	void send_user_choice() override;

	class hotkey_handler;

	bool is_networked_mp() const override;
	void send_to_wesnothd(const config& cfg, const std::string& packet_type = "unknown") const override;
	bool receive_from_wesnothd(config& cfg) const override;
protected:
	virtual void handle_generic_event(const std::string& name) override;

	void start_network();
	void stop_network();

	virtual void play_side_impl() override;
	virtual void play_human_turn() override;
	virtual void play_linger_turn();
	virtual void after_human_turn() override;
	virtual void play_network_turn() override;
	virtual void do_idle_notification() override;
	virtual void play_idle_loop() override;

	void linger();
	/** Wait for the host to upload the next scenario. */
	void wait_for_upload();

	mutable bool network_processing_stopped_;

	virtual void on_not_observer() override;
	bool is_host() const;
	void remove_blindfold();

	blindfold blindfold_;
private:
	void set_end_scenario_button();
	void reset_end_scenario_button();
	void process_network_data();
	mp_campaign_info* mp_info_;
};
