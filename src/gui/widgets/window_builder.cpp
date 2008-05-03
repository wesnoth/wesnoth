/* $Id$ */
/*
   Copyright (C) 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/widgets/window_builder.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/vertical_scrollbar.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "util.hpp"
#include "wml_exception.hpp"

#include <cassert>

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

namespace gui2 {

static unsigned get_v_align(const std::string& v_align);
static unsigned get_h_align(const std::string& h_align);
static unsigned get_border(const std::vector<std::string>& border);
static unsigned read_flags(const config& cfg);

struct tbuilder_control : public tbuilder_widget
{
private:
	tbuilder_control();
public:

	tbuilder_control(const config& cfg);

	void init_control(tcontrol* control) const;

	//! Parameters for the control.
	std::string id;
	std::string definition;
	t_string label;
	t_string tooltip;
	t_string help;
};

struct tbuilder_button : public tbuilder_control
{

private:
	tbuilder_button();
public:
	tbuilder_button(const config& cfg);

	twidget* build () const;

private:
	int retval_;
};

struct tbuilder_label : public tbuilder_control
{

private:
	tbuilder_label();
public:
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_label
 *
 * == Label ==
 *
 * A label has no special fields.
 *
 */
	tbuilder_label(const config& cfg) :
		tbuilder_control(cfg)
	{}

	twidget* build () const;

};

struct tbuilder_panel : public tbuilder_control
{

private:
	tbuilder_panel();
public:
	tbuilder_panel(const config& cfg);

	twidget* build () const;

	tbuilder_grid* grid;
};

struct tbuilder_spacer : public tbuilder_control
{

private:
	tbuilder_spacer();
public:
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_spacer
 *
 * == Spacer ==
 *
 * A spacer has no special fields.
 *
 */
	tbuilder_spacer(const config& cfg) :
		tbuilder_control(cfg)
	{}

	twidget* build () const;

};

struct tbuilder_text_box : public tbuilder_control
{
private:
	tbuilder_text_box();
	std::string history_;

public:
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_text_box
 *
 * == Text box ==
 *
 * @start_table = config
 *     label (tstring = "")            The initial text of the text box.
 *     history (string = "")           The name of the history for the text box.
 *                                     A history saves the data entered in a
 *                                     text box between the games. With the up
 *                                     and down arrow it can be accessed. To
 *                                     create a new history item just add a new
 *                                     unique name for this field and the engine
 *                                     will handle the rest.
 * @end_table
 *
 */
	tbuilder_text_box(const config& cfg) :
		tbuilder_control(cfg),
		history_(cfg["history"])
	{}

	twidget* build () const;
}
;
struct tbuilder_vertical_scrollbar : public tbuilder_control
{
private:
	tbuilder_vertical_scrollbar();

public:
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_vertical_scrollbar
 *
 * == Vertical scrollbar ==
 *
 * A vertical scrollbar has no special fields.
 *
 */
	tbuilder_vertical_scrollbar(const config& cfg) :
		tbuilder_control(cfg)
	{}

	twidget* build () const;
};

struct tbuilder_grid : public tbuilder_widget
{
private:
	tbuilder_grid();

public:
	tbuilder_grid(const config& cfg);
	unsigned rows;
	unsigned cols;

	//! The grow factor for the rows / columns.
	std::vector<unsigned> row_grow_factor;
	std::vector<unsigned> col_grow_factor;

	//! The flags per grid cell.
	std::vector<unsigned> flags;

	//! The border size per grid cell.
	std::vector<unsigned> border_size;

	//! The widgets per grid cell.
	std::vector<tbuilder_widget_ptr> widgets;

	twidget* build () const;

private:
	//! After reading the general part in the constructor read extra data.
	void read_extra(const config& cfg);
};

twindow build(CVideo& video, const std::string& type)
{
	std::vector<twindow_builder::tresolution>::const_iterator 
		definition = get_window_builder(type);

	// We set the values from the defintion since we can only determine the 
	// best size (if needed) after all widgets have been placed.
	twindow window(video, definition->x, definition->y, definition->width, definition->height);

	log_scope2(gui, "Window builder: building grid for window");

	const unsigned rows = definition->grid->rows;
	const unsigned cols = definition->grid->cols;

	window.set_rows_cols(rows, cols);

	for(unsigned x = 0; x < rows; ++x) {
		window.set_row_grow_factor(x, definition->grid->row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				window.set_col_grow_factor(y, definition->grid->col_grow_factor[y]);
			}

			twidget* widget = definition->grid->widgets[x * cols + y]->build();
			window.add_child(widget, x, y, definition->grid->flags[x * cols + y],  definition->grid->border_size[x * cols + y]);
		}
	}

	if(definition->automatic_placement) {
		
		tpoint size = window.get_best_size();
		size.x = size.x < settings::screen_width ? size.x : settings::screen_width;
		size.y = size.y < settings::screen_height ? size.y : settings::screen_height;

		tpoint position(0, 0);
		switch(definition->horizontal_placement) {
			case tgrid::HORIZONTAL_ALIGN_LEFT :
				// Do nothing
				break;
			case tgrid::HORIZONTAL_ALIGN_CENTER :
				position.x = (settings::screen_width - size.x) / 2;
				break;
			case tgrid::HORIZONTAL_ALIGN_RIGHT :
				position.x = settings::screen_width - size.x;
				break;
			default :
				assert(false);
		}
		switch(definition->vertical_placement) {
			case tgrid::VERTICAL_ALIGN_TOP :
				// Do nothing
				break;
			case tgrid::VERTICAL_ALIGN_CENTER :
				position.y = (settings::screen_height - size.y) / 2;
				break;
			case tgrid::VERTICAL_ALIGN_BOTTOM :
				position.y = settings::screen_height - size.y;
				break;
			default :
				assert(false);
		}

		window.set_size(create_rect(position, size));
	}

	return window;
}

const std::string& twindow_builder::read(const config& cfg)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 1_window
 *
 * = Window defintion =
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

	DBG_G_P << "Window builder: reading data for window " << id_ << ".\n";

	const config::child_list& cfgs = cfg.get_children("resolution");
	VALIDATE(!cfgs.empty(), _("No resolution defined."));
	for(std::vector<config*>::const_iterator itor = cfgs.begin();
			itor != cfgs.end(); ++itor) {

		resolutions.push_back(tresolution(**itor));
	}

	return id_;
}

twindow_builder::tresolution::tresolution(const config& cfg) :
	window_width(lexical_cast_default<unsigned>(cfg["window_width"])),
	window_height(lexical_cast_default<unsigned>(cfg["window_height"])),
	automatic_placement(utils::string_bool(cfg["automatic_placement"], true)),
	x(lexical_cast_default<unsigned>(cfg["x"])),
	y(lexical_cast_default<unsigned>(cfg["y"])),
	width(lexical_cast_default<unsigned>(cfg["width"])),
	height(lexical_cast_default<unsigned>(cfg["height"])),
	vertical_placement(get_v_align(cfg["vertical_placement"])),
	horizontal_placement(get_h_align(cfg["horizontal_placement"])),
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
 *     x (unsigned = 0)              X coordinate of the window to show.
 *     y (unsigned = 0)              Y coordinate of the window to show.
 *     width (unsigned = 0)          Width of the window to show.
 *     height (unsigned = 0)         Height of the window to show.
 *
 *     vertical_placement (v_align = "")
 *                                   The vertical placement of the window.
 *     horizontal_placement (h_align = "")
 *                                   The horizontal placement of the window.
 *
 *     definition (string = "default")
 *                                   Definition of the window which we want to 
 *                                   show.
 *
 *     grid (section)                The grid with the widgets to show. FIXME 
 *                                   the grid needs its own documentation page.
 * @end_table
 *
 */

	VALIDATE(cfg.child("grid"), _("No grid defined."));

	grid = new tbuilder_grid(*(cfg.child("grid")));

	if(!automatic_placement) {
		VALIDATE(width, missing_mandatory_wml_key("resulution", "width"));
		VALIDATE(height, missing_mandatory_wml_key("resulution", "height"));
	}

	DBG_G_P << "Window builder: parsing resolution " 
		<< window_width << ',' << window_height << '\n';

	if(definition.empty()) {
		definition = "default";
	}
	
}

static unsigned get_v_align(const std::string& v_align)
{

	if(v_align == "top") {
		return tgrid::VERTICAL_ALIGN_TOP;
	} else if(v_align == "bottom") {
		return tgrid::VERTICAL_ALIGN_BOTTOM;
	} else {
		if(!v_align.empty() && v_align != "center") {
			ERR_G_E << "Invalid vertical alignment '" 
				<< v_align << "' falling back to 'center'.\n";
		}
		return tgrid::VERTICAL_ALIGN_CENTER;
	}
}

static unsigned get_h_align(const std::string& h_align)
{
	if(h_align == "left") {
		return tgrid::HORIZONTAL_ALIGN_LEFT;
	} else if(h_align == "right") {
		return tgrid::HORIZONTAL_ALIGN_RIGHT;
	} else {
		if(!h_align.empty() && h_align != "center") {
			ERR_G_E << "Invalid horizontal alignment '" 
				<< h_align << "' falling back to 'center'.\n";
		}
		return tgrid::HORIZONTAL_ALIGN_CENTER;
	}
}

static unsigned get_border(const std::vector<std::string>& border)
{
	if(std::find(border.begin(), border.end(), "all") != border.end()) {
		return tgrid::BORDER_TOP 
			| tgrid::BORDER_BOTTOM | tgrid::BORDER_LEFT | tgrid::BORDER_RIGHT;
	} else {
		if(std::find(border.begin(), border.end(), "top") != border.end()) {
			return tgrid::BORDER_TOP;
		}
		if(std::find(border.begin(), border.end(), "bottom") != border.end()) {
			return tgrid::BORDER_BOTTOM;
		}
		if(std::find(border.begin(), border.end(), "left") != border.end()) {
			return tgrid::BORDER_LEFT;
		}
		if(std::find(border.begin(), border.end(), "right") != border.end()) {
			return tgrid::BORDER_RIGHT;
		}
	}

	return 0;
}

static unsigned read_flags(const config& cfg)
{
	unsigned flags = 0;

	// Read the flags. FIXME document.
	flags |= get_v_align(cfg["vertical_alignment"]);
	flags |= get_h_align(cfg["horizontal_alignment"]);
	flags |= get_border( utils::split(cfg["border"]));

	if(utils::string_bool(cfg["vertical_grow"])) {
		flags |= tgrid::VERTICAL_GROW_SEND_TO_CLIENT;
	}

	if(utils::string_bool(cfg["horizontal_grow"])) {
		flags |= tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT;
	}

	return flags;
}

tbuilder_grid::tbuilder_grid(const config& cfg) : 
	tbuilder_widget(cfg),
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
 *                                     the grid cell.
 *     horizontal_alignment (h_align = "")
 *                                     The horizontal alignment of the widget in
 *                                     the grid cell.
 *    
 *     vertical_grow (bool = false)    Does the widget grow in vertical
 *                                     direction when the grid cell grows in the
 *                                     vertical directon. This is used if the
 *                                     grid cell is wider as the best width for
 *                                     the widget.
 *     horizontal_grow (bool = false)  Does the widget grow in horizontal
 *                                     direction when the grid cell grows in the
 *                                     horizontal directon. This is used if the
 *                                     grid cell is higher as the best width for
 *                                     the widget.
 * @end_table
 *
 * == Widget ==
 *
 * The widget is one of the following items:
 * * button a button.
 * * grid a grid, this is used to nest items.
 * * label a label.
 * * panel a panel (a grid which can be drawn on).
 * * spacer a filler item. 
 * * text_box a text box.
 * * vertical_scrollbar a vertical scrollbar.
 *
 * More details about the widgets is in the next section.
 *
 */
	log_scope2(gui_parse, "Window builder: parsing a grid");

	const config::child_list& row_cfgs = cfg.get_children("row");
	for(std::vector<config*>::const_iterator row_itor = row_cfgs.begin();
			row_itor != row_cfgs.end(); ++row_itor) {

		unsigned col = 0;

		row_grow_factor.push_back(lexical_cast_default<unsigned>((**row_itor)["grow_factor"]));

		const config::child_list& col_cfgs = (**row_itor).get_children("column");
		for(std::vector<config*>::const_iterator col_itor = col_cfgs.begin();
				col_itor != col_cfgs.end(); ++col_itor) {

			flags.push_back(read_flags(**col_itor));
			border_size.push_back(lexical_cast_default<unsigned>((**col_itor)["border_size"]));
			if(rows == 0) {
				col_grow_factor.push_back(lexical_cast_default<unsigned>((**col_itor)["grow_factor"]));
			}

			if((**col_itor).child("button")) {
				widgets.push_back(new tbuilder_button(*((**col_itor).child("button"))));
			} else if((**col_itor).child("label")) {
				widgets.push_back(new tbuilder_label(*((**col_itor).child("label"))));
			} else if((**col_itor).child("panel")) {
				widgets.push_back(new tbuilder_panel(*((**col_itor).child("panel"))));
			} else if((**col_itor).child("spacer")) {
				widgets.push_back(new tbuilder_spacer(*((**col_itor).child("spacer"))));
			} else if((**col_itor).child("text_box")) {
				widgets.push_back(new tbuilder_text_box(*((**col_itor).child("text_box"))));
			} else if((**col_itor).child("vertical_scrollbar")) {
				widgets.push_back(
					new tbuilder_vertical_scrollbar(*((**col_itor).child("vertical_scrollbar"))));
			} else if((**col_itor).child("grid")) {
				widgets.push_back(new tbuilder_grid(*((**col_itor).child("grid"))));
			} else {
				assert(false);
			}

			++col;
		}

		++rows;
		if(row_itor == row_cfgs.begin()) {
			cols = col;
		} else {
			VALIDATE(col, _("A row must have a column."));
			VALIDATE(col == cols, _("Number of columns differ."));
		}

	}

	DBG_G_P << "Window builder: grid has " 
		<< rows << " rows and " << cols << " columns.\n";
}

tbuilder_control::tbuilder_control(const config& cfg) :
	tbuilder_widget(cfg),
	id(cfg["id"]),
	definition(cfg["definition"]),
	label(cfg["label"]),
	tooltip(cfg["tooltip"]),
	help(cfg["help"])
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget
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
 *                                     uses them.
 *
 *     definition (string = "default") The id of the widget definition to use.
 *                                     This way it's possible to select a
 *                                     specific version of the widget eg a title
 *                                     label when the label is used as title.
 *
 *     label (tstring = "")            Most widgets have some text accosiated
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
 * @end_table
 *
 */

	if(definition.empty()) {
		definition = "default";
	}


	DBG_G_P << "Window builder: found control with id '" 
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
}

twidget* tbuilder_button::build() const
{
	tbutton *button = new tbutton();

	init_control(button);

	if(retval_) {
		button->set_retval(retval_);
	} else {
		button->set_retval(tbutton::get_retval_by_id(id));
	}

	DBG_G << "Window builder: placed button '" << id << "' with defintion '" 
		<< definition << "'.\n";

	return button;
}

tbuilder_button::tbuilder_button(const config& cfg) :
	tbuilder_control(cfg),
	retval_(lexical_cast_default<int>(cfg["return_value"]))
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_button
 *
 * == Button ==
 *
 * Definition of a button. When a button has a return value it sets the retour
 * value for the window. Normally this closes the window and returns this value
 * to the caller. The return value can either be defined by the user or
 * determined from the id of the button. The return value has a higher
 * precedence as the one defined by the id. (Of course it's weird to give a
 * button an id and then override it's return value.)
 *
 * List with the button specific variables:
 * @start_table = config
 *     return_value (int = 0)          The return value.
 *
 * @end_table
 *
 */
}

twidget* tbuilder_label::build() const
{
	tlabel *tmp_label = new tlabel();

	init_control(tmp_label);

	DBG_G << "Window builder: placed label '" << id << "' with defintion '" 
		<< definition << "'.\n";

	return tmp_label;
}

tbuilder_panel::tbuilder_panel(const config& cfg) :
	tbuilder_control(cfg),
	grid(0)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_panel
 *
 * == Panel ==
 *
 * A panel is an item which can hold other items. The difference between a grid
 * and a panel is that it's possible to define how a panel looks. A grid in an
 * invisible container to just hold the items.
 *
 * @start_table = config
 *     grid (section)                  Defines the grid with the widgets to
 *                                     place on the panel.
 * @end_table                                   
 *
 */
	VALIDATE(cfg.child("grid"), _("No grid defined."));

	grid = new tbuilder_grid(*(cfg.child("grid")));
}

twidget* tbuilder_panel::build() const
{
	tpanel *panel = new tpanel();

	init_control(panel);

	DBG_G << "Window builder: placed panel '" << id << "' with defintion '" 
		<< definition << "'.\n";


	log_scope2(gui, "Window builder: building grid for panel.");

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
			panel->add_child(widget, x, y, grid->flags[x * cols + y],  grid->border_size[x * cols + y]);
		}
	}

	return panel;
}

twidget* tbuilder_spacer::build() const
{
	tspacer *spacer = new tspacer();

	init_control(spacer);

	DBG_G << "Window builder: placed spacer '" << id << "' with defintion '" 
		<< definition << "'.\n";

	return spacer;
}

twidget* tbuilder_text_box::build() const
{
	ttext_box *text_box = new ttext_box();

	init_control(text_box);

	// A textbox doesn't have a label but a text
	text_box->set_text(label);

	if (!history_.empty()) {
		text_box->set_history(history_);		
	}

	DBG_G << "Window builder: placed text box '" << id << "' with defintion '" 
		<< definition << "'.\n";

	return text_box;
}

twidget* tbuilder_vertical_scrollbar::build() const
{
	tvertical_scrollbar *vertical_scrollbar = new tvertical_scrollbar();

	init_control(vertical_scrollbar);

	DBG_G << "Window builder: placed text box '" << id << "' with defintion '" 
		<< definition << "'.\n";

	return vertical_scrollbar;
}

twidget* tbuilder_grid::build() const
{
	tgrid *grid = new tgrid();

	grid->set_rows_cols(rows, cols);

	log_scope2(gui, "Window builder: building grid");

	DBG_G << "Window builder: grid has " << rows << " rows and "
		<< cols << " columns.\n";

	for(unsigned x = 0; x < rows; ++x) {
		grid->set_row_grow_factor(x, row_grow_factor[x]);
		for(unsigned y = 0; y < cols; ++y) {

			if(x == 0) {
				grid->set_col_grow_factor(y, col_grow_factor[y]);
			}

			DBG_G << "Window builder: adding child at " << x << ',' << y << ".\n";

			twidget* widget = widgets[x * cols + y]->build();
			grid->add_child(widget, x, y, flags[x * cols + y],  border_size[x * cols + y]);
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
 * [[Category: Generated]]
 *
 */

