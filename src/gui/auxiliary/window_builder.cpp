/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/widgets/button.hpp"
#include "gui/widgets/horizontal_scrollbar.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/generator.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/scrollbar_panel.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/vertical_scrollbar.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

namespace {

unsigned get_v_align(const std::string& v_align)
{
	if(v_align == "top") {
		return tgrid::VERTICAL_ALIGN_TOP;
	} else if(v_align == "bottom") {
		return tgrid::VERTICAL_ALIGN_BOTTOM;
	} else {
		if(!v_align.empty() && v_align != "center") {
			ERR_GUI_E << "Invalid vertical alignment '"
				<< v_align << "' falling back to 'center'.\n";
		}
		return tgrid::VERTICAL_ALIGN_CENTER;
	}
}

unsigned get_h_align(const std::string& h_align)
{
	if(h_align == "left") {
		return tgrid::HORIZONTAL_ALIGN_LEFT;
	} else if(h_align == "right") {
		return tgrid::HORIZONTAL_ALIGN_RIGHT;
	} else {
		if(!h_align.empty() && h_align != "center") {
			ERR_GUI_E << "Invalid horizontal alignment '"
				<< h_align << "' falling back to 'center'.\n";
		}
		return tgrid::HORIZONTAL_ALIGN_CENTER;
	}
}

unsigned get_border(const std::vector<std::string>& border)
{
	if(std::find(border.begin(), border.end(), std::string("all")) != border.end()) {
		return tgrid::BORDER_TOP
			| tgrid::BORDER_BOTTOM | tgrid::BORDER_LEFT | tgrid::BORDER_RIGHT;
	} else {
		if(std::find(border.begin(), border.end(), std::string("top")) != border.end()) {
			return tgrid::BORDER_TOP;
		}
		if(std::find(border.begin(), border.end(), std::string("bottom")) != border.end()) {
			return tgrid::BORDER_BOTTOM;
		}
		if(std::find(border.begin(), border.end(), std::string("left")) != border.end()) {
			return tgrid::BORDER_LEFT;
		}
		if(std::find(border.begin(), border.end(), std::string("right")) != border.end()) {
			return tgrid::BORDER_RIGHT;
		}
	}

	return 0;
}

unsigned read_flags(const config& cfg)
{
	unsigned flags = 0;

	const unsigned v_flags = get_v_align(cfg["vertical_alignment"]);
	const unsigned h_flags = get_h_align(cfg["horizontal_alignment"]);
	flags |= get_border( utils::split(cfg["border"]));

	if(utils::string_bool(cfg["vertical_grow"])) {
		flags |= tgrid::VERTICAL_GROW_SEND_TO_CLIENT;

		if(! (cfg["vertical_alignment"]).empty()) {
			ERR_GUI_P << "vertical_grow and vertical_alignment "
				"can't be combined, alignment is ignored.\n";
		}
	} else {
		flags |= v_flags;
	}

	if(utils::string_bool(cfg["horizontal_grow"])) {
		flags |= tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT;

		if(! (cfg["horizontal_alignment"]).empty()) {
			ERR_GUI_P << "horizontal_grow and horizontal_alignment "
				"can't be combined, alignment is ignored.\n";
		}
	} else {
		flags |= h_flags;
	}

	return flags;
}

tmenubar::tdirection read_direction(const std::string& direction)
{
	if(direction == "vertical") {
		return tmenubar::VERTICAL;
	} else if(direction == "horizontal") {
		return tmenubar::HORIZONTAL;
	} else {
		ERR_GUI_E << "Invalid direction "
				<< direction << "' falling back to 'horizontal'.\n";
		return tmenubar::HORIZONTAL;
	}
}

tscrollbar_container::tscrollbar_mode
		get_scrollbar_mode(const std::string& scrollbar_mode)
{
	if(scrollbar_mode == "always") {
		return tscrollbar_container::always_visible;
	} else if(scrollbar_mode == "never") {
		return tscrollbar_container::always_invisible;
	} else if(scrollbar_mode == "initial_auto"
			|| (gui2::new_widgets && scrollbar_mode.empty())) {
		return tscrollbar_container::auto_visible_first_run;
	} else {
		if(!scrollbar_mode.empty() && scrollbar_mode != "auto") {
			ERR_GUI_E << "Invalid scrollbar mode '"
				<< scrollbar_mode << "' falling back to 'auto'.\n";
		}
		return tscrollbar_container::auto_visible;
	}
}

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

	TRY(button);
	TRY(horizontal_scrollbar);
	TRY(image);
	TRY(label);
	TRY(listbox);
	TRY(menubar);
	TRY(minimap);
	TRY(multi_page);
	TRY(panel);
	TRY(scroll_label);
	TRY(scrollbar_panel);
	TRY(slider);
	TRY(spacer);
	TRY(text_box);
	TRY(password_box);
	TRY(toggle_button);
	TRY(toggle_panel);
	TRY(vertical_scrollbar);
	TRY(grid);

#undef TRY

	std::cerr << cfg;
	ERROR_LOG(false);
}

/**
 * Returns the return value for a widget.
 *
 * If there's a valid retval_id that will be returned.
 * Else if there's a retval that's returned.
 * Else it falls back to the id.
 */
int get_retval(const std::string& retval_id,
		const int retval, const std::string& id)
{
	if(!retval_id.empty()) {
		int result = twindow::get_retval_by_id(retval_id);
		if(result) {
			return result;
		} else {
			ERR_GUI_E << "Window builder: retval_id '"
					<< retval_id << "' is unknown.\n";
		}
	}

	if(retval) {
		return retval;
	} else {
		return twindow::get_retval_by_id(id);
	}
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
	twindow* window = new twindow(video,
		definition->x, definition->y, definition->width, definition->height,
		definition->automatic_placement,
		definition->horizontal_placement, definition->vertical_placement,
		definition->definition);
	assert(window);

	log_scope2(log_gui_general, "Window builder: building grid for window");

	window->set_easy_close(definition->easy_close);

	const unsigned rows = definition->grid->rows;
	const unsigned cols = definition->grid->cols;

	window->set_rows_cols(rows, cols);

	for(unsigned x = 0; x < rows; ++x) {
		window->set_row_grow_factor(x, definition->grid->row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				window->set_col_grow_factor(y, definition->grid->col_grow_factor[y]);
			}

			twidget* widget = definition->grid->widgets[x * cols + y]->build();
			window->set_child(widget, x, y,
				definition->grid->flags[x * cols + y],
				definition->grid->border_size[x * cols + y]);
		}
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
	foreach (const config &i, cfgs) {
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
	vertical_placement(get_v_align(cfg["vertical_placement"])),
	horizontal_placement(get_h_align(cfg["horizontal_placement"])),
	easy_close(utils::string_bool(cfg["easy_close"])),
	definition(cfg["definition"]),
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
 *     easy_close (bool = false)     Does the window need easy close behaviour?
 *                                   Easy close behaviour means that any mouse
 *                                   click will close the dialog. Note certain
 *                                   widgets will automatically disable this
 *                                   behaviour since they need to process the
 *                                   clicks as well, for example buttons do need
 *                                   a click and a misclick on button shouldn't
 *                                   close the dialog. NOTE with some widgets
 *                                   this behaviour depends on their contents
 *                                   (like scrolling labels) so the behaviour
 *                                   might get changed depending on the data in
 *                                   the dialog. NOTE the default behaviour
 *                                   might be changed since it will be disabled
 *                                   when can't be used due to widgets which use
 *                                   the mouse, including buttons, so it might
 *                                   be wise to set the behaviour explicitly
 *                                   when not wanted and no mouse using widgets
 *                                   are available. This means enter, escape or
 *                                   an external source needs to be used to
 *                                   close the dialog (which is valid).
 *
 *     definition (string = "default")
 *                                   Definition of the window which we want to
 *                                   show.
 *
 *     grid (grid)                   The grid with the widgets to show.
 * @end_table
 *
 * The size variables are copied to the window and will be determined runtime.
 * This is needed since the main window can be resized and the dialog needs to
 * resize accordingly. The following variables are available:
 * @start_table = formula
 *     screen_width unsigned         The usable width of the Wesnoth main window.
 *     screen_height unsigned        The usable height of the Wesnoth main window.
 *     gamemap_width unsigned        The usable width of the Wesnoth gamemap,
 *                                   if no gamemap shown it's the same value as
 *                                   screen_width.
 *     gamemap_height unsigned       The usable height of the Wesnoth gamemap,
 *                                   if no gamemap shown it's the same value as
 *                                   screen_height.
 * @end_table
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

}

tbuilder_grid::tbuilder_grid(const config& cfg) :
	tbuilder_widget(cfg),
	id(cfg["id"]),
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

	foreach (const config &row, cfg.child_range("row"))
	{
		unsigned col = 0;

		row_grow_factor.push_back(lexical_cast_default<unsigned>(row["grow_factor"]));

		foreach (const config &c, row.child_range("column"))
		{
			flags.push_back(read_flags(c));
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

tbuilder_control::tbuilder_control(const config& cfg) :
	tbuilder_widget(cfg),
	id(cfg["id"]),
	definition(cfg["definition"]),
	label(cfg["label"]),
	tooltip(cfg["tooltip"]),
	help(cfg["help"]),
	use_tooltip_on_label_overflow(
		utils::string_bool("use_tooltip_on_label_overflow", true))
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 1_widget
 *
 * = Widget =
 *
 * All widgets placed in the cell have some values in common:
 * @start_table = config
 *     id (string = "")                This value is used for the engine to
 *                                     identify 'special' items. This means that
 *                                     for example a text_box can get the proper
 *                                     initial value. This value should be
 *                                     unique or empty. Those special values are
 *                                     documented at the window definition that
 *                                     uses them. NOTE items starting with an
 *                                     underscore are used for composed widgets
 *                                     and these should be unique per composed
 *                                     widget.
 *
 *     definition (string = "default") The id of the widget definition to use.
 *                                     This way it's possible to select a
 *                                     specific version of the widget eg a title
 *                                     label when the label is used as title.
 *
 *     label (tstring = "")            Most widgets have some text associated
 *                                     with them, this field contain the value
 *                                     of that text. Some widgets use this value
 *                                     for other purposes, this is documented
 *                                     at the widget.
 *
 *     tooptip (tstring = "")          If you hover over a widget a while (the
 *                                     time it takes can differ per widget) a
 *                                     short help can show up.This defines the
 *                                     text of that message.
 *
 *
 *     help (tstring = "")             If you hover over a widget and press F1 a
 *                                     help message can show up. This help
 *                                     message might be the same as the tooltip
 *                                     but in general (if used) this message
 *                                     should show more help. This defines the
 *                                     text of that message.
 *
 *    use_tooltip_on_label_overflow (bool = true)
 *                                     If the text on the label is truncated and
 *                                     the tooltip is empty the label can be
 *                                     used for the tooltip. If this variable is
 *                                     set to true this will happen.
 * @end_table
 *
 */

	if(definition.empty()) {
		definition = "default";
	}


	DBG_GUI_P << "Window builder: found control with id '"
		<< id << "' and definition '" << definition << "'.\n";
}

void tbuilder_control::init_control(tcontrol* control) const
{
	assert(control);

	control->set_id(id);
	control->set_definition(definition);
	control->set_label(label);
	control->set_tooltip(tooltip);
	control->set_help_message(help);
	control->set_use_tooltip_on_label_overflow(use_tooltip_on_label_overflow);
}

tbuilder_button::tbuilder_button(const config& cfg) :
	tbuilder_control(cfg),
	retval_id_(cfg["return_value_id"]),
	retval_(lexical_cast_default<int>(cfg["return_value"]))
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_button
 *
 * == Button ==
 *
 * Instance of a button. When a button has a return value it sets the
 * return value for the window. Normally this closes the window and returns
 * this value to the caller. The return value can either be defined by the
 * user or determined from the id of the button. The return value has a
 * higher precedence as the one defined by the id. (Of course it's weird to
 * give a button an id and then override it's return value.)
 *
 * When the button doesn't have a standard id, but you still want to use the
 * return value of that id, use return_value_id instead. This has a higher
 * precedence as return_value.
 *
 * List with the button specific variables:
 * @start_table = config
 *     return_value_id (string = "")   The return value id.
 *     return_value (int = 0)          The return value.
 *
 * @end_table
 *
 */
}

twidget* tbuilder_button::build() const
{
	tbutton* button = new tbutton();

	init_control(button);

	button->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed button '" << id << "' with defintion '"
		<< definition << "'.\n";

	return button;
}

twidget* tbuilder_horizontal_scrollbar::build() const
{
	thorizontal_scrollbar *horizontal_scrollbar = new thorizontal_scrollbar();

	init_control(horizontal_scrollbar);

	DBG_GUI_G << "Window builder:"
		<< " placed horizontal scrollbar '" << id
		<< "' with defintion '" << definition
		<< "'.\n";

	return horizontal_scrollbar;
}

twidget* tbuilder_image::build() const
{
	timage* widget = new timage();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed image '" << id << "' with defintion '"
		<< definition << "'.\n";

	return widget;
}

tbuilder_gridcell::tbuilder_gridcell(const config& cfg) :
	tbuilder_widget(cfg),
	flags(read_flags(cfg)),
	border_size(lexical_cast_default<unsigned>((cfg)["border_size"])),
	widget(create_builder_widget(cfg))
{
}

tbuilder_label::tbuilder_label(const config& cfg)
	: tbuilder_control(cfg)
	, wrap(utils::string_bool("wrap"))
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_label
 *
 * == Label ==
 *
 * Instance of a label.
 *
 * List with the label specific variables:
 * @start_table = config
 *     wrap (bool = false)        Is wrapping enabled for the label.
 * @end_table
 */
}

twidget* tbuilder_label::build() const
{
	tlabel* tmp_label = new tlabel();

	init_control(tmp_label);

	tmp_label->set_can_wrap(wrap);

	DBG_GUI_G << "Window builder: placed label '" << id << "' with defintion '"
		<< definition << "'.\n";

	return tmp_label;
}

tbuilder_listbox::tbuilder_listbox(const config& cfg) :
	tbuilder_control(cfg),
	vertical_scrollbar_mode(
			get_scrollbar_mode(cfg["vertical_scrollbar_mode"])),
	horizontal_scrollbar_mode(
			get_scrollbar_mode(cfg["horizontal_scrollbar_mode"])),
	header(0),
	footer(0),
	list_builder(0),
	list_data()
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_listbox
 *
 * == Listbox ==
 *
 * Instance of a listbox.
 *
 * List with the listbox specific variables:
 * @start_table = config
 *     vertical_scrollbar_mode (scrollbar_mode = auto | initial_auto)
 *                                     Determines whether or not to show the
 *                                     scrollbar. The default of initial_auto
 *                                     is used when --new-widgets is used.
 *                                     In the future the default will be
 *                                     auto.
 *     horizontal_scrollbar_mode (scrollbar_mode = auto | initial_auto)
 *                                     Determines whether or not to show the
 *                                     scrollbar. The default of initial_auto
 *                                     is used when --new-widgets is used.
 *                                     In the future the default will be
 *                                     initial_auto.
 *
 *     header (grid = [])              Defines the grid for the optional
 *                                     header. (This grid will automatically
 *                                     get the id _header_grid.)
 *     footer (grid = [])              Defines the grid for the optional
 *                                     footer. (This grid will automatically
 *                                     get the id _footer_grid.)
 *
 *     list_definition (section)       This defines how a listbox item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list.
 *
 *     list_data(section = [])         A grid alike section which stores the
 *                                     initial data for the listbox. Every row
 *                                     must have the same number of columns as
 *                                     the 'list_definition'.
 * @end_table
 *
 *
 * Inside the list section there are only the following widgets allowed
 * * grid (to nest)
 * * selectable widgets which are
 * ** toggle_button
 * ** toggle_panel
 *
 */

	if (const config &h = cfg.child("header"))
		header = new tbuilder_grid(h);

	if (const config &f = cfg.child("footer"))
		footer = new tbuilder_grid(f);

	const config &l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = new tbuilder_grid(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1, _("A 'list_definition' should contain one row."));

	const config &data = cfg.child("list_data");
	if (!data) return;

	foreach (const config &row, data.child_range("row"))
	{
		unsigned col = 0;

		foreach (const config &c, row.child_range("column"))
		{
			list_data.push_back(string_map());
			foreach (const config::attribute &i, c.attribute_range()) {
				list_data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == list_builder->cols, _("'list_data' must have "
			"the same number of columns as the 'list_definition'."));
	}
}

twidget* tbuilder_listbox::build() const
{
	tlistbox *listbox = new tlistbox(
			true, true, tgenerator_::vertical_list, true);

	init_control(listbox);

	listbox->set_list_builder(list_builder); // FIXME in finalize???

	listbox->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	listbox->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id << "' with defintion '"
		<< definition << "'.\n";

	boost::intrusive_ptr<const tlistbox_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const tlistbox_definition::tresolution>(listbox->config());
	assert(conf);


	conf->grid->build(&listbox->grid());

	listbox->finalize(header, footer, list_data);

	return listbox;
}

tbuilder_menubar::tbuilder_menubar(const config& cfg) :
	tbuilder_control(cfg),
	must_have_one_item_selected_(utils::string_bool(cfg["must_have_one_item_selected"])),
	direction_(read_direction(cfg["direction"])),
	selected_item_(lexical_cast_default<int>(
		cfg["selected_item"], must_have_one_item_selected_ ? 0 : -1)),
	cells_()
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_menubar
 *
 * == Menubar ==
 *
 * A menu bar used for menus and tab controls.
 *
 * List with the listbox specific variables:
 * @start_table = config
 *     must_have_one_item_selected (bool = false)
 *                                     Does the menu always have one item
 *                                     selected. This makes sense for tabsheets
 *                                     but not for menus.
 *     direction (direction = "")      The direction of the menubar.
 *     selected_item(int = -1)         The item to select upon creation, when
 *                                     'must_have_one_item_selected' is true the
 *                                     default value is 0 instead of -1. -1
 *                                     means no item selected.
 * @end_table
 */
	const config &data = cfg.child("data");
	if (!data) return;

	foreach (const config &cell, data.child_range("cell")) {
		cells_.push_back(tbuilder_gridcell(cell));
	}
}

twidget* tbuilder_menubar::build() const
{
	tmenubar* menubar = new tmenubar(direction_);

	init_control(menubar);


	DBG_GUI_G << "Window builder: placed menubar '" << id << "' with defintion '"
		<< definition << "'.\n";

	if(direction_ == tmenubar::HORIZONTAL) {
		menubar->set_rows_cols(1, cells_.size());

		for(size_t i = 0; i < cells_.size(); ++i) {
			menubar->set_child(cells_[i].widget->build(),
				0, i, cells_[i].flags, cells_[i].border_size);
		}
	} else {
		// vertical growth
		menubar->set_rows_cols(cells_.size(), 1);

		for(size_t i = 0; i < cells_.size(); ++i) {
			menubar->set_child(cells_[i].widget->build(),
				i, 0, cells_[i].flags, cells_[i].border_size);
		}
	}

	menubar->set_selected_item(selected_item_);
	menubar->set_must_select(must_have_one_item_selected_);
	menubar->finalize_setup();

	return menubar;
}

twidget* tbuilder_minimap::build() const
{
	tminimap* minimap = new tminimap();

	init_control(minimap);

	DBG_GUI_G << "Window builder: placed minimap '" << id << "' with defintion '"
		<< definition << "'.\n";

	return minimap;
}

tbuilder_multi_page::tbuilder_multi_page(const config& cfg) :
	tbuilder_control(cfg),
	builder(0),
	data()
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_multi_page
 *
 * == Multi page ==
 *
 * Instance of a multi page.
 *
 * List with the multi page specific variables:
 * @start_table = config
 *     page_definition (section)       This defines how a listbox item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list.
 *
 *     page_data(section = [])         A grid alike section which stores the
 *                                     initial data for the listbox. Every row
 *                                     must have the same number of columns as
 *                                     the 'list_definition'.
 * @end_table
 */

	const config &page = cfg.child("page_definition");

	VALIDATE(page, _("No page defined."));
	builder = new tbuilder_grid(page);
	assert(builder);

	/** @todo This part is untested. */
	const config &d = cfg.child("page_data");
	if(!d){
		return;
	}

	foreach(const config &row, d.child_range("row"))
	{
		unsigned col = 0;

		foreach (const config &column, row.child_range("column"))
		{
			data.push_back(string_map());
			foreach (const config::attribute &i, column.attribute_range()) {
				data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == builder->cols, _("'list_data' must have "
			"the same number of columns as the 'list_definition'."));
	}
}

twidget* tbuilder_multi_page::build() const
{
	tmulti_page *multi_page = new tmulti_page();

	init_control(multi_page);

	multi_page->set_page_builder(builder);

	DBG_GUI_G << "Window builder: placed multi_page '"
		<< id << "' with defintion '"
		<< definition << "'.\n";

	boost::intrusive_ptr<const tmulti_page_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const tmulti_page_definition::tresolution>(multi_page->config());
	assert(conf);

	conf->grid->build(&multi_page->grid());

	multi_page->finalize(data);

	return multi_page;
}

tbuilder_panel::tbuilder_panel(const config& cfg) :
	tbuilder_control(cfg),
	grid(0)
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_panel
 *
 * == Panel ==
 *
 * A panel is an item which can hold other items. The difference between a grid
 * and a panel is that it's possible to define how a panel looks. A grid in an
 * invisible container to just hold the items.
 *
 * @start_table = config
 *     grid (grid)                     Defines the grid with the widgets to
 *                                     place on the panel.
 * @end_table
 *
 */

	const config &c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = new tbuilder_grid(c);
}

twidget* tbuilder_panel::build() const
{
	tpanel* panel = new tpanel();

	init_control(panel);

	DBG_GUI_G << "Window builder: placed panel '" << id << "' with defintion '"
		<< definition << "'.\n";


	log_scope2(log_gui_general, "Window builder: building grid for panel.");

	const unsigned rows = grid->rows;
	const unsigned cols = grid->cols;

	panel->set_rows_cols(rows, cols);

	for(unsigned x = 0; x < rows; ++x) {
		panel->set_row_grow_factor(x, grid->row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				panel->set_col_grow_factor(y, grid->col_grow_factor[y]);
			}

			twidget* widget = grid->widgets[x * cols + y]->build();
			panel->set_child(widget, x, y, grid->flags[x * cols + y],  grid->border_size[x * cols + y]);
		}
	}

	return panel;
}

twidget* tbuilder_scroll_label::build() const
{
	tscroll_label* widget = new tscroll_label();

	init_control(widget);

	boost::intrusive_ptr<const tscroll_label_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const tscroll_label_definition::tresolution>(widget->config());
	assert(conf);

	conf->grid->build(&widget->grid());
	widget->finalize_setup();

	DBG_GUI_G << "Window builder: placed scroll label '" << id << "' with defintion '"
		<< definition << "'.\n";

	return widget;
}

tbuilder_slider::tbuilder_slider(const config& cfg) :
	tbuilder_control(cfg),
	best_slider_length_(lexical_cast_default<unsigned>(cfg["best_slider_length"])),
	minimum_value_(lexical_cast_default<int>(cfg["minimum_value"])),
	maximum_value_(lexical_cast_default<int>(cfg["maximum_value"])),
	step_size_(lexical_cast_default<unsigned>(cfg["step_size"])),
	value_(lexical_cast_default<unsigned>(cfg["value"])),
	minimum_value_label_(cfg["minimum_value_label"]),
	maximum_value_label_(cfg["maximum_value_label"]),
	value_labels_()
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 3_slider
 *
 * == Slider ==
 *
 * @start_table = config
 *     best_slider_length (unsigned = 0)
 *                                    The best length for the sliding part.
 *     minimum_value (int = 0)        The minimum value the slider can have.
 *     maximum_value (int = 0)        The maximum value the slider can have.
 *
 *     step_size (unsigned = 0)       The number of items the slider's value
 *                                    increases with one step.
 *     value (int = 0)                The value of the slider.
 *
 *     minimum_value_label (t_string = "")
 *                                    If the minimum value is chosen there
 *                                    might be the need for a special value (eg
 *                                    off). When this key has a value that value
 *                                    will be shown if the minimum is selected.
 *     maximum_value_label (t_string = "")
 *                                    If the maximum value is chosen there
 *                                    might be the need for a special value (eg
 *                                    unlimited)). When this key has a value
 *                                    that value will be shown if the maximum is
 *                                    selected.
 *     value_labels ([])              It might be the labels need to be shown
 *                                    are not a linear number sequence eg (0.5,
 *                                    1, 2, 4) in that case for all items this
 *                                    section can be filled with the values,
 *                                    which should be the same number of items
 *                                    as the items in the slider. NOTE if this
 *                                    option is used, 'minimum_value_label' and
 *                                    'maximum_value_label' are ignored.
 * @end_table
 */
	const config &labels = cfg.child("value_labels");
	if (!labels) return;

	foreach (const config &label, labels.child_range("value")) {
		value_labels_.push_back(label["label"]);
	}
}

tbuilder_scrollbar_panel::tbuilder_scrollbar_panel(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, grid(0)
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_scrollbar_panel
 *
 * == Scrollbar panel ==
 *
 * Instance of a scrollbar_panel.
 *
 * List with the scrollbar_panel specific variables:
 * @start_table = config
 *     vertical_scrollbar_mode (scrollbar_mode = auto | initial_auto)
 *                                     Determines whether or not to show the
 *                                     scrollbar. The default of initial_auto
 *                                     is used when --new-widgets is used.
 *                                     In the future the default will be
 *                                     auto.
 *     horizontal_scrollbar_mode (scrollbar_mode = auto | initial_auto)
 *                                     Determines whether or not to show the
 *                                     scrollbar. The default of initial_auto
 *                                     is used when --new-widgets is used.
 *                                     In the future the default will be
 *                                     initial_auto.
 *
 *     definition (section)            This defines how a scrollbar_panel item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list.
 *
 * @end_table
 *
 */

	const config &definition = cfg.child("definition");

	VALIDATE(definition, _("No list defined."));
	grid = new tbuilder_grid(definition);
	assert(grid);
}

twidget* tbuilder_scrollbar_panel::build() const
{
	tscrollbar_panel *scrollbar_panel = new tscrollbar_panel();

	init_control(scrollbar_panel);

	scrollbar_panel->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	scrollbar_panel->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed scrollbar_panel '" << id << "' with defintion '"
		<< definition << "'.\n";

	boost::intrusive_ptr<
			const tscrollbar_panel_definition::tresolution> conf =
				boost::dynamic_pointer_cast
					<const tscrollbar_panel_definition::tresolution>
						(scrollbar_panel->config());
	assert(conf);


	conf->grid->build(&scrollbar_panel->grid());
	scrollbar_panel->finalize_setup();

	/*** Fill the content grid. ***/
	tgrid* content_grid = scrollbar_panel->content_grid();
	assert(content_grid);

	const unsigned rows = grid->rows;
	const unsigned cols = grid->cols;

	content_grid->set_rows_cols(rows, cols);

	for(unsigned x = 0; x < rows; ++x) {
		content_grid->set_row_grow_factor(x, grid->row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				content_grid->set_col_grow_factor(y
						, grid->col_grow_factor[y]);
			}

			twidget* widget = grid->widgets[x * cols + y]->build();
			content_grid->set_child(widget
					, x
					, y
					, grid->flags[x * cols + y]
					, grid->border_size[x * cols + y]);
		}
	}

	return scrollbar_panel;
}

twidget* tbuilder_slider::build() const
{
	tslider* slider = new tslider();

	init_control(slider);

	slider->set_best_slider_length(best_slider_length_);
	slider->set_maximum_value(maximum_value_);
	slider->set_minimum_value(minimum_value_);
	slider->set_step_size(step_size_);
	slider->set_value(value_);

	if(!value_labels_.empty()) {
		VALIDATE(value_labels_.size() == slider->get_item_count(),
			_("The number of value_labels and values don't match."));

		slider->set_value_labels(value_labels_);

	} else {
		slider->set_minimum_value_label(minimum_value_label_);
		slider->set_maximum_value_label(maximum_value_label_);
	}

	DBG_GUI_G << "Window builder: placed slider '" << id << "' with defintion '"
		<< definition << "'.\n";

	return slider;
}

twidget* tbuilder_spacer::build() const
{
	tspacer* spacer = new tspacer();

	init_control(spacer);

	const game_logic::map_formula_callable& size = get_screen_size_variables();
	const unsigned width = width_(size);
	const unsigned height = height_(size);

	if(width || height) {
		spacer->set_best_size(tpoint(width, height));
	}

	DBG_GUI_G << "Window builder: placed spacer '" << id << "' with defintion '"
		<< definition << "'.\n";

	return spacer;
}

twidget* tbuilder_toggle_button::build() const
{
	ttoggle_button *toggle_button = new ttoggle_button();

	init_control(toggle_button);

	toggle_button->set_icon_name(icon_name_);
	toggle_button->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed toggle button '"
			<< id << "' with defintion '" << definition << "'.\n";

	return toggle_button;
}

tbuilder_toggle_panel::tbuilder_toggle_panel(const config& cfg) :
	tbuilder_control(cfg),
	grid(0),
	retval_id_(cfg["return_value_id"]),
	retval_(lexical_cast_default<int>(cfg["return_value"]))
{
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_toggle_panel
 *
 * == Toggle panel ==
 *
 * A panel is an item which can hold other items. The difference between a grid
 * and a panel is that it's possible to define how a panel looks. A grid in an
 * invisible container to just hold the items. The toggle panel is a
 * combination of the panel and a toggle button, it allows a toggle button with
 * its own grid.
 *
 * @start_table = config
 *     grid (grid)                     Defines the grid with the widgets to
 *                                     place on the panel.
 *     return_value_id (string = "")   The return value id, see
 *                                     [[GUIToolkitWML#Button]] for more info.
 *     return_value (int = 0)          The return value, see
 *                                     [[GUIToolkitWML#Button]] for more info.
 * @end_table
 *
 */

	const config &c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = new tbuilder_grid(c);
}

twidget* tbuilder_toggle_panel::build() const
{
	ttoggle_panel* toggle_panel = new ttoggle_panel();

	init_control(toggle_panel);

	toggle_panel->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed toggle panel '"
			<< id << "' with defintion '" << definition << "'.\n";

	log_scope2(log_gui_general, "Window builder: building grid for toggle panel.");

	const unsigned rows = grid->rows;
	const unsigned cols = grid->cols;

	toggle_panel->set_rows_cols(rows, cols);

	for(unsigned x = 0; x < rows; ++x) {
		toggle_panel->set_row_grow_factor(x, grid->row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				toggle_panel->set_col_grow_factor(y, grid->col_grow_factor[y]);
			}

			twidget* widget = grid->widgets[x * cols + y]->build();
			toggle_panel->set_child(widget, x, y, grid->flags[x * cols + y],  grid->border_size[x * cols + y]);
		}
	}

	return toggle_panel;
}

twidget* tbuilder_text_box::build() const
{
	ttext_box* text_box = new ttext_box();

	init_control(text_box);

	// A textbox doesn't have a label but a text
	text_box->set_value(label);

	if (!history_.empty()) {
		text_box->set_history(history_);
	}

	DBG_GUI_G << "Window builder: placed text box '" << id << "' with defintion '"
		<< definition << "'.\n";

	return text_box;
}

twidget* tbuilder_password_box::build() const
{
	tpassword_box* password_box = new tpassword_box();

	init_control(password_box);

	// A textbox doesn't have a label but a text
	password_box->set_value(label);

	if (!history_.empty()) {
		password_box->set_history(history_);
	}

	DBG_GUI_G << "Window builder: placed password box '" << id << "' with defintion '"
		<< definition << "'.\n";

	return password_box;
}

twidget* tbuilder_vertical_scrollbar::build() const
{
	tvertical_scrollbar *vertical_scrollbar = new tvertical_scrollbar();

	init_control(vertical_scrollbar);

	DBG_GUI_G << "Window builder:"
		<< " placed vertical scrollbar '" << id
		<< "' with defintion '" << definition
		<< "'.\n";

	return vertical_scrollbar;
}

twidget* tbuilder_grid::build() const
{
	return build(new tgrid());
}

twidget* tbuilder_grid::build (tgrid* grid) const
{
	grid->set_id(id);
	grid->set_rows_cols(rows, cols);

	log_scope2(log_gui_general, "Window builder: building grid");

	DBG_GUI_G << "Window builder: grid '" << id
		<< "' has " << rows << " rows and "
		<< cols << " columns.\n";

	for(unsigned x = 0; x < rows; ++x) {
		grid->set_row_grow_factor(x, row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				grid->set_col_grow_factor(y, col_grow_factor[y]);
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

