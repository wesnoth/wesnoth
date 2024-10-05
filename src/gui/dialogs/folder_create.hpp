/*
	Copyright (C) 2011 - 2024
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

class folder_create : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] folder_name
	 *                            The parameter's usage is:
	 *                            - Input: A suggested folder name.
	 *                            - Output: The folder name the user actually
	 *                              entered if the dialog returns
	 *                              retval::OK; undefined otherwise.
	 */
	folder_create(std::string& folder_name);

	/** The execute function; see @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(folder_create)

private:
	friend class bookmark_create;

	bool bookmark_mode_;

	/** Changes the dialog caption so it can be used for naming bookmarks. */
	folder_create& enable_bookmark_mode()
	{
		bookmark_mode_ = true;
		return *this;
	}

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;
};

class bookmark_create
{
public:
	/** The execute function; see @ref modal_dialog for more information. */
	static bool execute(std::string& bookmark_name)
	{
		return folder_create(bookmark_name).enable_bookmark_mode().show();
	}
};

} // namespace dialogs
