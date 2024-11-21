/*
	Copyright (C) 2008 - 2024
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

namespace gui2::dialogs
{

class mp_report : public modal_dialog
{
public:
	mp_report(std::string& report_text_);

	DEFINE_SIMPLE_EXECUTE_WRAPPER(mp_report)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	void reportee_changed(const std::string& text);
	void report_reason_changed(const std::string& text);

	std::string& report_text_;
	bool reportee_empty_;
	bool report_reason_empty_;
	const static std::array<std::string, 3> occurrence_locations;
};

} // namespace gui2::dialogs
