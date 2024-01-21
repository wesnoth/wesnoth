/*
	Copyright (C) 2021 - 2024
	by Iris Morelle <shadowm@wesnoth.org>
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

#include "gui/dialogs/modeless_dialog.hpp"

#include <chrono>

namespace gui2::dialogs {

class file_progress : public modeless_dialog
{
public:
	file_progress(const std::string& title, const std::string& message);

	template<typename... T>
	static auto display(T&&... args)
	{
		auto instance = std::make_unique<file_progress>(std::forward<T>(args)...);
		instance->show(true);
		return instance;
	}

	void update_progress(unsigned value);

private:
	/** Inherited from modeless_dialog. */
	virtual const std::string& window_id() const override;

	std::string title_;
	std::string message_;

	using clock = std::chrono::steady_clock;

	std::chrono::time_point<clock> update_time_;
};

}
