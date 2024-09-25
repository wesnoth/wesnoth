/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/modal_dialog.hpp"

namespace game_config
{
struct server_info;
}

namespace gui2
{

class listbox;

namespace dialogs
{

class mp_connect : public modal_dialog
{
	/** The unit test needs to be able to test the mp_connect dialog. */
	friend modal_dialog* unit_test_mp_server_list();

public:
	mp_connect();

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(mp_connect)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	// Signal handlers

	void on_address_change();
	void on_server_add();
	void on_server_delete();
	void on_server_select();

	// Auxiliary functions

	using server_info = game_config::server_info;
	using server_list = std::vector<server_info>;

	void insert_into_server_listbox(listbox& listbox, const server_info& srv, int pos = -1);

	void select_first_match();

	class selection
	{
	public:
		selection(mp_connect* owner, int row = -1)
			: owner_(owner)
			, row_(row)
		{
		}

		bool valid() const
		{
			return owner_ && row_ >= 0;
		}

		bool user_defined() const;

		unsigned row() const;

		std::size_t relative_index() const;

		server_list& parent_list() const;

		server_info& get();

	private:
		mp_connect* owner_;
		int row_;

		void must_be_valid() const
		{
			if(!valid()) {
				throw std::out_of_range{"Invalid MP server selection"};
			}
		}
	};

	selection current_selection();

	std::array<server_list*, 2> server_lists();

	/** The host name of the selected server. */
	field_text* host_name_;

	server_list builtin_servers_;
	server_list user_servers_;
};

} // namespace dialogs
} // namespace gui2
