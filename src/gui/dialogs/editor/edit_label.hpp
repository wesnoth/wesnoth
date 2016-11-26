/*
   Copyright (C) 2010 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_EDIT_LABEL_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_EDIT_LABEL_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "map/label.hpp"

namespace gui2
{
namespace dialogs
{

class editor_edit_label : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param[in, out] text       The parameter's usage is:
	 *                            - Input: The initial value of the label.
	 *                            - Output: The label text the user entered if
	 *                              the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 */
	editor_edit_label(std::string& text,
					   bool& immutable,
					   bool& visible_fog,
					   bool& visible_shroud,
					   SDL_Color& color,
					   std::string& category);

	/** The execute function see @ref modal_dialog for more information. */
	static bool execute(std::string& text,
						bool& immutable,
						bool& visible_fog,
						bool& visible_shroud,
						SDL_Color& color,
						std::string& category,
						CVideo& video)
	{
		return editor_edit_label(text, immutable, visible_fog, visible_shroud, color, category)
				.show(video);
	}

private:
	SDL_Color& color_store;
	int load_color_component(Uint8 SDL_Color::* component);
	void save_color_component(Uint8 SDL_Color::* component, const int value);
	void register_color_component(std::string widget_id, Uint8 SDL_Color::* component);
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};
} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
