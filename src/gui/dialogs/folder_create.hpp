/*
   Copyright (C) 2011 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_FOLDER_CREATE_HPP_INCLUDED
#define GUI_DIALOGS_FOLDER_CREATE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{

class tfolder_create : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] folder_name
	 *                            The parameter's usage is:
	 *                            - Input: A suggested folder name.
	 *                            - Output: The folder name the user actually
	 *                              entered if the dialog returns @ref
	 *                              twindow::OK; undefined otherwise.
	 */
	tfolder_create(std::string& folder_name);

	/** The execute function; see @ref tdialog for more information. */
	static bool execute(std::string& folder_name, CVideo& video)
	{
		return tfolder_create(folder_name).show(video);
	}

private:
	friend class tbookmark_create;

	bool bookmark_mode_;

	/** Changes the dialog caption so it can be used for naming bookmarks. */
	tfolder_create& enable_bookmark_mode()
	{
		bookmark_mode_ = true;
		return *this;
	}

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);
};

class tbookmark_create
{
public:
	/** The execute function; see @ref tdialog for more information. */
	static bool execute(std::string& bookmark_name, CVideo& video)
	{
		return tfolder_create(bookmark_name).enable_bookmark_mode().show(video);
	}
};

}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
