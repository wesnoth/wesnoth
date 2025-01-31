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

namespace gui2
{
class field_text;

namespace dialogs
{

class mp_login : public modal_dialog
{
public:
	mp_login(const std::string& host, const std::string& label, const bool focus_password);

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	void load_password();
	void save_password();

	const std::string host_;
	field_text* username_;
	bool focus_password_;
};

} // namespace dialogs
} // namespace gui2
