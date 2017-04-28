/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

#ifndef GUI_WIDGETS_DEBUG_HPP_INCLUDED
#define GUI_WIDGETS_DEBUG_HPP_INCLUDED

#include "gui/widgets/grid.hpp"

#include <iosfwd>
#include <string>

namespace gui2
{

class widget;
class window;

/**
 * Helper class to output the layout to dot files.
 *
 * The class will generate .dot files in the location where wesnoth is running
 * (so needs write access there). These files can be converted to images
 * containing the graphs. This is used for debugging the widget library and its
 * sizing algorithm.
 *
 * This class needs to be friends with a lot of classes so it can view the
 * private data in the class. This design is chosen so the debug info can be
 * put in a separate class instead of adding the info via virtual functions in
 * the classes themselves. Also adding 'friend class foo' doesn't need to
 * include the header declaring foo, so it avoids header cluttage.
 *
 */
class debug_layout_graph
{
public:
	/**
	 * Constructor.
	 *
	 * @param window              The window, whose information will be
	 *                            generated.
	 */
	explicit debug_layout_graph(const window* window);

	/***** ***** ***** ***** FLAGS ***** ***** ***** *****/

	// domain flags
	static const unsigned MANUAL = 0 << 0; /**<
											* Shows the info when the F12 is
											* pressed. The value 0 makes sure
											* the domain is always valid.
											*/
	static const unsigned SHOW = 1 << 0; /**<
										  * Shows the info when the dialog
										  * is shown.
										  */
	static const unsigned LAYOUT = 1 << 1; /**<
											* Shows the info in all layout
											* phases.
											*/
	/**
	 * Sets the level of wanted information.
	 *
	 * @param level               A comma separated list of levels which are
	 *                            wanted. Possible values: child, size, state
	 *                            and all.
	 */
	static void set_level(const std::string& level);

	/**
	 * Sets the domain when to show the information.
	 *
	 * @param domain              A comma separated list for domains which are
	 *                            wanted. Possible values: show, layout and all.
	 */
	static void set_domain(const std::string& domain);

	/**
	 * Generates a dot file.
	 *
	 * The file will have a fixed prefix filename but for every file a part of
	 * the name will contain where it was generated.
	 *
	 * @param generator           The location where the name was generated.
	 */
	void generate_dot_file(const std::string& generator, const unsigned domain);

private:
	/** The window whose info will be shown. */
	const window* window_;

	/** The order in which the files are generated. */
	unsigned sequence_number_;

	/** Basic part of the filename. */
	std::string filename_base_;

	/***** ***** Widget ***** *****/

	/**
	 * Generates the info about a widget.
	 *
	 * @param out                 The stream to write the info to.
	 * @param widget              The widget to write the info about.
	 * @param id                  The dof-file-id of the widget.
	 * @param embedded            Is the grid embedded in a container eg parent
	 *                            inherits from container_base.
	 */
	void widget_generate_info(std::ostream& out,
							  const widget* widget,
							  const std::string& id,
							  const bool embedded = false) const;

	/**
	 * Generates the basic info about a widget.
	 *
	 * @param out                 The stream to write the info to.
	 * @param widget              The widget to write the info about.
	 */
	void widget_generate_basic_info(std::ostream& out,
									const widget* widget) const;

	/**
	 * Generates the info about the state of the widget.
	 *
	 * @param out                 The stream to write the info to.
	 * @param widget              The widget to write the info about.
	 */
	void widget_generate_state_info(std::ostream& out,
									const widget* widget) const;

	/**
	 * Generates the info about the size and layout of the widget.
	 *
	 * @param out                 The stream to write the info to.
	 * @param widget              The widget to write the info about.
	 */
	void widget_generate_size_info(std::ostream& out,
								   const widget* widget) const;

	/***** ***** Grid ***** *****/

	/**
	 * Generates the info about a grid.
	 *
	 * @param out                 The stream to write the info to.
	 * @param grid                The grid to write the info about.
	 * @param parent_id           The dot-file-id of the parent of the widget.
	 */
	void grid_generate_info(std::ostream& out,
							const grid* grid,
							const std::string& parent_id) const;

	/**
	 * Generates the info about a grid cell.
	 *
	 * @param out                 The stream to write the info to.
	 * @param child               The grid cell to write the info about.
	 * @param id                  The dof-file-id of the child.
	 */
	void child_generate_info(std::ostream& out,
							 const grid::child& child,
							 const std::string& id) const;

	/***** ***** Helper ***** *****/

	/**
	 * Returns the control_type of a widget.
	 *
	 * This is a small wrapper around styled_widget::get_control_type() since a
	 * grid is no styled_widget and used rather frequently, so we want to give it a
	 * type.
	 *
	 * @param widget              The widget to get the type of.
	 *
	 * @returns                   If the widget is a styled_widget it returns its
	 *                            type. If the widget is a grid it returns
	 *                            'grid', otherwise 'unknown' will be returned.
	 */
	std::string get_type(const widget* widget) const;
};

} // namespace gui2

#endif
#endif
