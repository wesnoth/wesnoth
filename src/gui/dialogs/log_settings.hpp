/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "gui/widgets/group.hpp"
#include <map>

namespace gui2
{
namespace dialogs
{

class log_settings : public modal_dialog
{
public:
	/** Constructor. */
	log_settings();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(log_settings)

private:
	void set_logger(const std::basic_string<char> log_domain);

	std::map<std::string, group<std::string>> groups_;
	std::vector<std::string> domain_list_, widget_id_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

};

} // namespace dialogs
} // end namespace gui2
