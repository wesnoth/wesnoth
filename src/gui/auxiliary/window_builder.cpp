/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder_private.hpp"

#include "asserts.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/window_builder/button.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/auxiliary/window_builder/horizontal_listbox.hpp"
#include "gui/auxiliary/window_builder/horizontal_scrollbar.hpp"
#include "gui/auxiliary/window_builder/image.hpp"
#include "gui/auxiliary/window_builder/label.hpp"
#include "gui/auxiliary/window_builder/listbox.hpp"
#include "gui/auxiliary/window_builder/minimap.hpp"
#include "gui/auxiliary/window_builder/menubar.hpp"
#include "gui/auxiliary/window_builder/multi_page.hpp"
#include "gui/auxiliary/window_builder/repeating_button.hpp"
#include "gui/auxiliary/window_builder/scroll_label.hpp"
#include "gui/auxiliary/window_builder/scrollbar_panel.hpp"
#include "gui/auxiliary/window_builder/slider.hpp"
#include "gui/auxiliary/window_builder/spacer.hpp"
#include "gui/auxiliary/window_builder/stacked_widget.hpp"
#include "gui/auxiliary/window_builder/text_box.hpp"
#include "gui/auxiliary/window_builder/toggle_button.hpp"
#include "gui/auxiliary/window_builder/tree_view.hpp"
#include "gui/auxiliary/window_builder/panel.hpp"
#include "gui/auxiliary/window_builder/password_box.hpp"
#include "gui/auxiliary/window_builder/toggle_panel.hpp"
#include "gui/auxiliary/window_builder/vertical_scrollbar.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "formula_string_utils.hpp"

namespace gui2 {

namespace {

tbuilder_widget_ptr create_builder_widget(const config& cfg)
{
	config::all_children_itors children = cfg.all_children_range();
	size_t nb_children = std::distance(children.first, children.second);
	if (nb_children != 1) {
		ERR_GUI_P << "Grid cell has " << nb_children
			<< " children instead of 1, aborting. Config :\n"
			<< cfg;
		assert(false);
	}

#define TRY(name) do { \
	if (const config &c = cfg.child(#name)) \
		return new tbuilder_##name(c); \
	} while (0)

	// The widgets builders are mostly in this namespace.
	using namespace gui2::implementation;

	TRY(button);
	TRY(horizontal_listbox);
	TRY(horizontal_scrollbar);
	TRY(image);
	TRY(label);
	TRY(listbox);
	TRY(menubar);
	TRY(minimap);
	TRY(multi_page);
	TRY(panel);
	TRY(repeating_button);
	TRY(scroll_label);
	TRY(scrollbar_panel);
	TRY(slider);
	TRY(spacer);
	TRY(stacked_widget);
	TRY(text_box);
	TRY(password_box);
	TRY(toggle_button);
	TRY(toggle_panel);
	TRY(tree_view);
	TRY(vertical_scrollbar);
	TRY(grid);

#undef TRY

	std::cerr << cfg;
	ERROR_LOG(false);
}

} // namespace

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 1
 *
 * THIS PAGE IS AUTOMATICALLY GENERATED, DO NOT MODIFY DIRECTLY !!!
 *
 * = Widget instance =
 *
 * Inside a grid (which is inside all container widgets) a widget is
 * instantiated. With this instantiation some more variables of a widget can
 * be tuned. This page will describe what can be tuned.
 *
 */
twindow* build(CVideo& video, const std::string& type)
{
	std::vector<twindow_builder::tresolution>::const_iterator
		definition = get_window_builder(type);

	// We set the values from the definition since we can only determine the
	// best size (if needed) after all widgets have been placed.
	twindow* window = new twindow(video
			, definition->x
			, definition->y
			, definition->width
			, definition->height
			, definition->automatic_placement
			, definition->horizontal_placement
			, definition->vertical_placement
			, definition->maximum_width
			, definition->maximum_height
			, definition->definition);
	assert(window);
	window->set_id(type);

	BOOST_FOREACH(const twindow_builder::tresolution::tlinked_group& lg,
			definition->linked_groups) {

		if(window->has_linked_size_group(lg.id)) {
			utils::string_map symbols;
			symbols["id"] = lg.id;
			t_string msg = vgettext(
					  "Linked '$id' group has multiple definitions."
					, symbols);

			VALIDATE(false, msg);
		}

		window->init_linked_size_group(
				lg.id, lg.fixed_width, lg.fixed_height);
	}

	window->set_click_dismiss(definition->click_dismiss);

	boost::intrusive_ptr<const twindow_definition::tresolution> conf =
			boost::dynamic_pointer_cast<
				const twindow_definition::tresolution>(window->config());
	assert(conf);

	if(conf->grid) {
		window->init_grid(conf->grid);
		window->finalize(definition->grid);
	} else {
		window->init_grid(definition->grid);
	}

	window->add_to_keyboard_chain(window);

	return window;
}

const std::string& twindow_builder::read(const config& cfg)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_window
 *
 * = Window definition =
 *
 * A window defines how a window looks in the game.
 *
 * @start_table = config
 *     id (string)                   Unique id for this window.
 *     description (t_string)        Unique translatable name for this window.
 *
 *     resolution (section)          The definitions of the window in various
 *                                   resolutions.
 * @end_table
 *
 */

	id_ = cfg["id"];
	description_ = cfg["description"];

	VALIDATE(!id_.empty(), missing_mandatory_wml_key("window", "id"));
	VALIDATE(!description_.empty(), missing_mandatory_wml_key("window", "description"));

	DBG_GUI_P << "Window builder: reading data for window " << id_ << ".\n";

	config::const_child_itors cfgs = cfg.child_range("resolution");
	VALIDATE(cfgs.first != cfgs.second, _("No resolution defined."));
	BOOST_FOREACH (const config &i, cfgs) {
		resolutions.push_back(tresolution(i));
	}

	return id_;
}

twindow_builder::tresolution::tresolution(const config& cfg) :
	window_width(lexical_cast_default<unsigned>(cfg["window_width"])),
	window_height(lexical_cast_default<unsigned>(cfg["window_height"])),
	automatic_placement(utils::string_bool(cfg["automatic_placement"], true)),
	x(cfg["x"]),
	y(cfg["y"]),
	width(cfg["width"]),
	height(cfg["height"]),
	vertical_placement(
			implementation::get_v_align(cfg["vertical_placement"])),
	horizontal_placement(
			implementation::get_h_align(cfg["horizontal_placement"])),
	maximum_width(lexical_cast_default<unsigned>(cfg["maximum_width"])),
	maximum_height(lexical_cast_default<unsigned>(cfg["maximum_height"])),
	click_dismiss(utils::string_bool(cfg["click_dismiss"])),
	definition(cfg["definition"]),
	linked_groups(),
	grid(0)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_window
 *
 * == Resolution ==
 *
 * @start_table = config
 *     window_width (unsigned = 0)   Width of the application window.
 *     window_height (unsigned = 0)  Height of the application window.
 *
 *     automatic_placement (bool = true)
 *                                   Automatically calculate the best size for
 *                                   the window and place it. If automatically
 *                                   placed ''vertical_placement'' and
 *                                   ''horizontal_placement'' can be used to
 *                                   modify the final placement. If not
 *                                   automatically placed the ''width'' and
 *                                   ''height'' are mandatory.
 *
 *     x (f_unsigned = 0)            X coordinate of the window to show.
 *     y (f_unsigned = 0)            Y coordinate of the window to show.
 *     width (f_unsigned = 0)        Width of the window to show.
 *     height (f_unsigned = 0)       Height of the window to show.
 *
 *     vertical_placement (v_align = "")
 *                                   The vertical placement of the window.
 *     horizontal_placement (h_align = "")
 *                                   The horizontal placement of the window.
 *
 *     maximum_width (unsigned = 0)  The maximum width of the window (only
 *                                   used for automatic placement).
 *     maximum_height (unsigned = 0) The maximum height of the window (only
 *                                   used for automatic placement).
 *
 *     click_dismiss (bool = false)  Does the window need click dismiss
 *                                   behaviour? Click dismiss behaviour means
 *                                   that any mouse click will close the
 *                                   dialog. Note certain widgets will
 *                                   automatically disable this behaviour since
 *                                   they need to process the clicks as well,
 *                                   for example buttons do need a click and a
 *                                   misclick on button shouldn't close the
 *                                   dialog. NOTE with some widgets this
 *                                   behaviour depends on their contents (like
 *                                   scrolling labels) so the behaviour might
 *                                   get changed depending on the data in the
 *                                   dialog. NOTE the default behaviour might
 *                                   be changed since it will be disabled when
 *                                   can't be used due to widgets which use the
 *                                   mouse, including buttons, so it might be
 *                                   wise to set the behaviour explicitly when
 *                                   not wanted and no mouse using widgets are
 *                                   available. This means enter, escape or an
 *                                   external source needs to be used to close
 *                                   the dialog (which is valid).
 *
 *     definition (string = "default")
 *                                   Definition of the window which we want to
 *                                   show.
 *
 *     linked_group (sections = [])  A group of linked widget sections.
 *
 *     grid (grid)                   The grid with the widgets to show.
 * @end_table
 *
 * A linked_group section has the following fields:
 * @start_table = config
 *     id (string)                   The unique id of the group (unique in this
 *                                   window).
 *     fixed_width (bool = false)    Should widget in this group have the same
 *                                   width.
 *     fixed_height (bool = false)   Should widget in this group have the same
 *                                   height.
 * @end_table
 *
 * A linked group needs to have at least one size fixed.
 */

	const config &c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = new tbuilder_grid(c);

	if(!automatic_placement) {
		VALIDATE(width.has_formula() || width(),
			missing_mandatory_wml_key("resolution", "width"));
		VALIDATE(height.has_formula() || height(),
			missing_mandatory_wml_key("resolution", "height"));
	}

	DBG_GUI_P << "Window builder: parsing resolution "
		<< window_width << ',' << window_height << '\n';

	if(definition.empty()) {
		definition = "default";
	}

	BOOST_FOREACH (const config &lg, cfg.child_range("linked_group")) {
		tlinked_group linked_group;
		linked_group.id = lg["id"];
		linked_group.fixed_width = utils::string_bool(lg["fixed_width"]);
		linked_group.fixed_height = utils::string_bool(lg["fixed_height"]);

		VALIDATE(!linked_group.id.empty()
				, missing_mandatory_wml_key("linked_group", "id"));

		if(!(linked_group.fixed_width || linked_group.fixed_height)) {
			utils::string_map symbols;
			symbols["id"] = linked_group.id;
			t_string msg = vgettext(
					  "Linked '$id' group needs a 'fixed_width' or "
						"'fixed_height' key."
					, symbols);

			VALIDATE(false, msg);
		}

		linked_groups.push_back(linked_group);
	}
}

tbuilder_grid::tbuilder_grid(const config& cfg) :
	tbuilder_widget(cfg),
	id(cfg["id"]),
	linked_group(cfg["linked_group"]),
	rows(0),
	cols(0),
	row_grow_factor(),
	col_grow_factor(),
	flags(),
	border_size(),
	widgets()
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_cell
 *
 * = Cell =
 *
 * Every grid cell has some cell configuration values and one widget in the grid
 * cell. Here we describe the what is available more information about the usage
 * can be found here [[GUILayout]].
 *
 * == Row values ==
 *
 * For every row the following variables are available:
 *
 * @start_table = config
 *     grow_factor (unsigned = 0)      The grow factor for a row.
 * @end_table
 *
 * == Cell values ==
 *
 * For every column the following variables are available:
 * @start_table = config
 *     grow_factor (unsigned = 0)      The grow factor for a column, this value
 *                                     is only read for the first row.
 *
 *     border_size (unsigned = 0)      The border size for this grid cell.
 *     border (border = "")            Where to place the border in this grid
 *                                     cell.
 *
 *     vertical_alignment (v_align = "")
 *                                     The vertical alignment of the widget in
 *                                     the grid cell. (This value is ignored if
 *                                     vertical_grow is true.)
 *     horizontal_alignment (h_align = "")
 *                                     The horizontal alignment of the widget in
 *                                     the grid cell.(This value is ignored if
 *                                     horizontal_grow is true.)
 *
 *     vertical_grow (bool = false)    Does the widget grow in vertical
 *                                     direction when the grid cell grows in the
 *                                     vertical direction. This is used if the
 *                                     grid cell is wider as the best width for
 *                                     the widget.
 *     horizontal_grow (bool = false)  Does the widget grow in horizontal
 *                                     direction when the grid cell grows in the
 *                                     horizontal direction. This is used if the
 *                                     grid cell is higher as the best width for
 *                                     the widget.
 * @end_table
 *
 */
	log_scope2(log_gui_parse, "Window builder: parsing a grid");

	BOOST_FOREACH (const config &row, cfg.child_range("row"))
	{
		unsigned col = 0;

		row_grow_factor.push_back(lexical_cast_default<unsigned>(row["grow_factor"]));

		BOOST_FOREACH (const config &c, row.child_range("column"))
		{
			flags.push_back(implementation::read_flags(c));
			border_size.push_back(lexical_cast_default<unsigned>(c["border_size"]));
			if(rows == 0) {
				col_grow_factor.push_back(lexical_cast_default<unsigned>(c["grow_factor"]));
			}

			widgets.push_back(create_builder_widget(c));

			++col;
		}

		++rows;
		if (rows == 1) {
			cols = col;
		} else {
			VALIDATE(col, _("A row must have a column."));
			VALIDATE(col == cols, _("Number of columns differ."));
		}

	}

	DBG_GUI_P << "Window builder: grid has "
		<< rows << " rows and " << cols << " columns.\n";
}

tbuilder_gridcell::tbuilder_gridcell(const config& cfg) :
	tbuilder_widget(cfg),
	flags(implementation::read_flags(cfg)),
	border_size(lexical_cast_default<unsigned>((cfg)["border_size"])),
	widget(create_builder_widget(cfg))
{
}

twidget* tbuilder_grid::build() const
{
	return build(new tgrid());
}

twidget* tbuilder_grid::build (tgrid* grid) const
{
	grid->set_id(id);
	grid->set_linked_group(linked_group);
	grid->set_rows_cols(rows, cols);

	log_scope2(log_gui_general, "Window builder: building grid");

	DBG_GUI_G << "Window builder: grid '" << id
		<< "' has " << rows << " rows and "
		<< cols << " columns.\n";

	for(unsigned x = 0; x < rows; ++x) {
		grid->set_row_grow_factor(x, row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				grid->set_column_grow_factor(y, col_grow_factor[y]);
			}

			DBG_GUI_G << "Window builder: adding child at " << x << ',' << y << ".\n";

			twidget* widget = widgets[x * cols + y]->build();
			grid->set_child(widget, x, y, flags[x * cols + y],  border_size[x * cols + y]);
		}
	}

	return grid;
}

} // namespace gui2
/*WIKI
 * @page = GUIToolkitWML
 * @order = ZZZZZZ_footer
 *
 * [[Category: WML Reference]]
 * [[Category: GUI WML Reference]]
 * [[Category: Generated]]
 *
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = ZZZZZZ_footer
 *
 * [[Category: WML Reference]]
 * [[Category: GUI WML Reference]]
 * [[Category: Generated]]
 *
 */

