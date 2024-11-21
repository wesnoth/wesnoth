/*
	Copyright (C) 2011 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
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
#include "variable.hpp"

class replay;

namespace gui2::dialogs
{

class chat_log : public modal_dialog
{
public:
	class model;
	class view;
	class controller;
	chat_log(const vconfig& cfg, const replay& replay);

	DEFINE_SIMPLE_DISPLAY_WRAPPER(chat_log)

	virtual void pre_show() override;

	std::shared_ptr<view> get_view() const;

private:
	virtual const std::string& window_id() const override;

	std::shared_ptr<view> view_;
};
} // namespace dialogs
