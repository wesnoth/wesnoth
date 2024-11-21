/*
	Copyright (C) 2014 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

class wml_error : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param summary      Leading summary line for the report.
	 * @param post_summary Additional line with instructions for the user, may
	 *                     be empty.
	 * @param files        List of WML files on which errors were detected.
	 * @param details      Detailed WML preprocessor/parser error report.
	 */
	wml_error(const std::string& summary,
			   const std::string& post_summary,
			   const std::vector<std::string>& files,
			   const std::string& details);

	/** The display function; see @ref modal_dialog for more information. */
	static void display(const std::string& summary,
						const std::string& post_summary,
						const std::vector<std::string>& files,
						const std::string& details)
	{
		wml_error(summary, post_summary, files, details).show();
	}

	/** The display function; see @ref modal_dialog for more information. */
	static void display(const std::string& summary,
						const std::string& details)
	{
		display(summary, "", std::vector<std::string>(), details);
	}

private:
	bool have_files_;
	bool have_post_summary_;
	std::string report_; // Plain text report for copying to clipboard.

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	void copy_report_callback();
};

} // end namespace dialogs
