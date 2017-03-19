/*
   Copyright (C) 2014 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_GAME_CACHE_OPTIONS_HPP_INCLUDED
#define GUI_DIALOGS_GAME_CACHE_OPTIONS_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2
{
class label;
class button;
namespace dialogs
{

class game_cache_options : public modal_dialog
{
public:
	/** Constructor. */
	game_cache_options();

	/**
     * The display function.
	 *
	 * See @ref modal_dialog for more information.
     */
	static void display(CVideo& video)
	{
		game_cache_options().show(video);
	}

private:
	std::string cache_path_;

	button* clean_button_;
	button* purge_button_;
	label* size_label_;

	void clean_cache_callback(CVideo& video);
	bool clean_cache();

	void purge_cache_callback(CVideo& video);
	bool purge_cache();

	void copy_to_clipboard_callback();

	void browse_cache_callback();

	void update_cache_size_display();

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);
};

} // namespace dialogs
} // namespace gui2

#endif
