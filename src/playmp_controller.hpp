/*
	Copyright (C) 2006 - 2024
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

struct mp_game_metadata;
class playmp_controller : public playsingle_controller
{
public:
	playmp_controller(const config& level, saved_game& state_of_game, mp_game_metadata* mp_info);
	virtual ~playmp_controller();
	void maybe_linger() override;
	void process_oos(const std::string& err_msg) const override;

	void receive_actions() override;
	void send_actions() override;
	void surrender(int side_number);

	class hotkey_handler;

	bool is_networked_mp() const override;
	void send_to_wesnothd(const config& cfg, const std::string& packet_type = "unknown") const override;
	bool receive_from_wesnothd(config& cfg) const override;

	void play_slice() override;
protected:
	virtual void handle_generic_event(const std::string& name) override;

	void start_network();
	void stop_network();

	virtual void play_human_turn() override;
	virtual void after_human_turn() override;
	virtual void play_network_turn() override;
	virtual void do_idle_notification() override;
	virtual void play_idle_loop() override;

	/** Wait for the host to upload the next scenario. */
	void wait_for_upload();

	mutable bool network_processing_stopped_;
	bool next_scenario_notified_;

	virtual void on_not_observer() override;
	virtual bool is_host() const override;
	void remove_blindfold();

	blindfold blindfold_;
private:
	enum class PROCESS_DATA_RESULT
	{
		CONTINUE,
		/** when we couldn't handle the given action currently. */
		CANNOT_HANDLE
	};
	/**
	 * @param unsync_only if false (default) this can exceute synced (gamestate changing) turn commands (recall, move, etc.)
	 */
	void process_network_data(bool unsync_only = false);

	/// Check for and Handle incoming data from the multiplayer server
	PROCESS_DATA_RESULT process_network_data_impl(const config& cfg, bool chat_only = false);

	/// Handle incoming [turn] from the multiplayer server
	PROCESS_DATA_RESULT process_network_turn_impl(const config& t, bool chat_only = false);

	/// Handle incoming [side_drop] from the multiplayer server
	void process_network_side_drop_impl(const config& t);

	/// Handle incoming [change_controller] from the multiplayer server
	void process_network_change_controller_impl(const config& );

	/// Send [change_controller] to the multiplayer server
	void send_change_side_controller(int side, const std::string& player);

	/// Helper to preprocess infoming network data.
	playturn_network_adapter network_reader_;
	/// Information about our connection to the multiplayer server.
	/// null when we are not connected to the multiplayer server
	mp_game_metadata* mp_info_;
};
