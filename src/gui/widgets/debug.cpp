/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"


#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

#include "gui/widgets/debug.hpp"

#include "formatter.hpp"
#include "gui/widgets/generator.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/scrollbar_container.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/string_utils.hpp"

#include <fstream>
#include <iostream>

namespace gui2
{

namespace
{

/**
 * Gets the id of a grid child cell.
 *
 * @param parent_id               The id of the parent grid.
 * @param row                     Row number in the grid.
 * @param col                     Column number in the grid.
 *
 * @returns                       The id of the child cell.
 */
std::string get_child_id(const std::string& parent_id,
						 const unsigned row,
						 const unsigned col)
{
	// Originally used this formatter function but it managed to return empty
	// strings. No idea why so switched to using the good old lexical_cast
	// instead.

	// return (formatter() << parent_id << "_C_" << row << '_' << col).c_str();
	std::string result = parent_id + "_C_" + lexical_cast<std::string>(row)
						 + '_' + lexical_cast<std::string>(col);

	return result;
}

/**
 * Gets the id of a widget in a grid child cell.
 *
 * @param parent_id               The id of the parent grid.
 * @param row                     Row number in the grid.
 * @param col                     Column number in the grid.
 *
 * @returns                       The id of the widget.
 */
std::string get_child_widget_id(const std::string& parent_id,
								const unsigned row,
								const unsigned col)
{
	return get_child_id(parent_id, row, col) + "_W";
}

/** Gets the prefix of the filename. */
std::string get_base_filename()
{
	char buf[17] = { 0 };
	time_t t = time(NULL);
	tm* lt = localtime(&t);
	if(lt) {
		strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", lt);
	}
	static unsigned counter = 0;
	++counter;

	return (formatter() << buf << '_' << counter << '_').str();
}
/***** ***** ***** ***** FLAGS ***** ***** ***** *****/

const unsigned ALL = UINT_MAX; /**< All levels/domains */

const unsigned SIZE_INFO = 1 << 0; /**<
									* Shows the size info of
									* children/widgets.
									*/
const unsigned STATE_INFO = 1 << 1; /**<
									 * Shows the state info of widgets.
									 */
unsigned level_ = 0;
unsigned domain_ = 0;
} // namespace

tdebug_layout_graph::tdebug_layout_graph(const twindow* window)
	: window_(window), sequence_number_(0), filename_base_(get_base_filename())
{
}

void tdebug_layout_graph::set_level(const std::string& level)
{
	if(level.empty()) {
		level_ = ALL; /** @todo Should default to 0. */
		return;
	}

	std::vector<std::string> params = utils::split(level);

	FOREACH(const AUTO & param, params)
	{
		if(param == "all") {
			level_ = ALL;
			// No need to look further eventhough invalid items are now
			// ignored.
			return;
		} else if(param == "size") {
			level_ |= SIZE_INFO;
		} else if(param == "state") {
			level_ |= STATE_INFO;
		} else {
			// logging might not be up yet.
			std::cerr << "Unknown level '" << param << "' is ignored.\n";
		}
	}
}

void tdebug_layout_graph::set_domain(const std::string& domain)
{
	if(domain.empty()) {
		// return error and die
		domain_ = ALL; /** @todo Should default to 0. */
		return;
	}

	std::vector<std::string> params = utils::split(domain);

	FOREACH(const AUTO & param, params)
	{
		if(param == "all") {
			domain_ = ALL;
			// No need to look further eventhough invalid items are now
			// ignored.
			return;
		} else if(param == "show") {
			domain_ |= SHOW;
		} else if(param == "layout") {
			domain_ |= LAYOUT;
		} else {
			// logging might not be up yet.
			std::cerr << "Unknown domain '" << param << "' is ignored.\n";
		}
	}
}

void tdebug_layout_graph::generate_dot_file(const std::string& generator,
											const unsigned domain)
{
	// domain == 0 must also evaluate to true.
	if((domain_ & domain) != domain) {
		return;
	}

	std::string id = window_->id();
	if(!id.empty()) {
		id += '_';
	}
	const std::string filename = filename_base_ + id
								 + lexical_cast<std::string>(++sequence_number_)
								 + "-" + generator + ".dot";

	std::ofstream file(filename.c_str());

	file << "//Basic layout graph for window id '" << window_->id()
		 << "' using definition '" << window_->definition_ << "'.\n"
		 << "digraph window {\n"
		 << "\tnode [shape=record, style=filled, fillcolor=\"bisque\"];\n"
		 << "\trankdir=LR;\n";

	widget_generate_info(file, window_, "root");

	file << "}\n";
}

void tdebug_layout_graph::widget_generate_info(std::ostream& out,
											   const twidget* widget,
											   const std::string& id,
											   const bool embedded) const
{
	assert(!id.empty());

	out << "\t" << id
		<< " [label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">";

	widget_generate_basic_info(out, widget);
	if(level_ & STATE_INFO)
		widget_generate_state_info(out, widget);
	if(level_ & SIZE_INFO)
		widget_generate_size_info(out, widget);

	out << "</table>>";
	if(embedded) {
		out << ", fillcolor=\"palegoldenrod\"";
	}
	out << "];\n";

	const tgrid* grid = dynamic_cast<const tgrid*>(widget);
	if(!grid) {
		const tcontainer_* container = dynamic_cast<const tcontainer_*>(widget);

		if(container) {

			widget_generate_info(out, &container->grid(), id + "_G", true);
			out << "\t" << id << " -> " << id << "_G"
				<< " [label=\"(grid)\"];\n";
		}

		const tscrollbar_container* scrollbar_container
				= dynamic_cast<const tscrollbar_container*>(widget);

		if(scrollbar_container) {
			widget_generate_info(
					out, scrollbar_container->content_grid_, id + "_C", true);
			out << "\t" << id << " -> " << id << "_C"
				<< " [label=\"(content)\"];\n";
		}

		const tlistbox* listbox = dynamic_cast<const tlistbox*>(widget);
		if(listbox) {
			assert(listbox->generator_);
		}

		const tgenerator_* generator = dynamic_cast<const tgenerator_*>(widget);

		if(generator) {
			for(size_t i = 0; i < generator->get_item_count(); ++i) {

				const std::string child_id = id + "_I_"
											 + lexical_cast<std::string>(i);

				widget_generate_info(out, &generator->item(i), child_id, true);

				out << "\t" << id << " -> " << child_id
					<< " [label=\"(item)\"];\n";
			}
		}
	}
	if(grid) {
		grid_generate_info(out, grid, id);
	}
}

static std::string format_label(std::string label)
{
	if(label.size() > 50) {
		label = label.substr(0, 50) + "...";
	}

	// Replace characters that break the dot file/
	std::replace(label.begin(), label.end(), '>', '_');

	return label;
}

void tdebug_layout_graph::widget_generate_basic_info(std::ostream& out,
													 const twidget* widget)
		const
{
	std::string header_background
			= level_ & (SIZE_INFO | STATE_INFO) ? " bgcolor=\"gray\"" : "";
	const tcontrol* control = dynamic_cast<const tcontrol*>(widget);

	out << "<tr><td" << header_background << ">" << '\n'
		<< "type=" << get_type(widget) << '\n' << "</td></tr>" << '\n'
		<< "<tr><td" << header_background << ">" << '\n'
		<< "id=" << widget->id() << '\n' << "</td></tr>" << '\n' << "<tr><td"
		<< header_background << ">" << '\n' << "address=" << widget << '\n'
		<< "</td></tr>" << '\n' << "<tr><td" << header_background << ">" << '\n'
		<< "parent=" << widget->parent_ << '\n' << "</td></tr>" << '\n';
	if(control) {
		out << "<tr><td" << header_background << ">" << '\n'
			<< "label=" << format_label(control->label()) << '\n' << "<tr><td"
			<< header_background << ">" << '\n'
			<< "definition=" << control->definition_ << '\n' << "</td></tr>"
			<< '\n' << "</td></tr>\n";
	}
}

void tdebug_layout_graph::widget_generate_state_info(std::ostream& out,
													 const twidget* widget)
		const
{
	const tcontrol* control = dynamic_cast<const tcontrol*>(widget);
	if(!control) {
		return;
	}

	out << "<tr><td>\n"
		<< "tooltip=" << control->tooltip() << '\n' << "</td></tr>\n"
		<< "<tr><td>\n"
		<< "help message" << control->help_message() << '\n'
			// FIXME add value and other specific items
		<< "</td></tr>\n"
		<< "<tr><td>\n"
		<< "active=" << control->get_active() << '\n' << "</td></tr>\n"
		<< "<tr><td>\n"
		<< "visible=" << control->get_visible() << '\n' << "</td></tr>\n"
		<< "<tr><td>\n"
		<< "drawing action=" << control->get_drawing_action() << '\n'
		<< "</td></tr>\n"
		<< "<tr><td>\n"
		<< "clip rect=" << control->clipping_rectangle_ << '\n'
		<< "</td></tr>\n"
		<< "<tr><td>\n"
		<< "use tooltip on label overflow="
		<< control->get_use_tooltip_on_label_overflow() << '\n'
		<< "</td></tr>\n"
		<< "<tr><td>\n"
		<< "does block click dismiss=" << control->disable_click_dismiss()
		<< '\n' << "</td></tr>\n";

	const tscrollbar_container* scrollbar_container
			= dynamic_cast<const tscrollbar_container*>(widget);

	if(scrollbar_container) {
		out << "<tr><td>\n"
			<< "vertical_scrollbar_mode_="
			<< scrollbar_container->vertical_scrollbar_mode_ << '\n'
			<< "</td></tr>\n"
			<< "<tr><td>\n"
			<< "horizontal_scrollbar_mode_="
			<< scrollbar_container->horizontal_scrollbar_mode_ << '\n'
			<< "</td></tr>\n";
	}
}

void tdebug_layout_graph::widget_generate_size_info(std::ostream& out,
													const twidget* widget) const
{
	out << "<tr><td>\n"
		<< "can wrap=" << widget->can_wrap() << '\n' << "</td></tr>\n"
		<< "<tr><td>\n"
		<< "size=" << widget->get_size() << '\n' << "</td></tr>\n"
		<< "<tr><td>\n"
		<< "position=" << widget->get_origin() << '\n' << "</td></tr>\n"
		<< "<tr><td>\n"
		<< "last_best_size_=" << widget->last_best_size_ << '\n'
		<< "</td></tr>\n"
		<< "<tr><td>\n"
		<< "layout_size_=" << widget->layout_size_ << '\n' << "</td></tr>\n";


	const tcontrol* control = dynamic_cast<const tcontrol*>(widget);

	if(control) {
		out << "<tr><td>\n"
			<< "minimum config size=" << control->get_config_minimum_size()
			<< '\n' << "</td></tr>\n"
			<< "<tr><td>\n"
			<< "default config size=" << control->get_config_default_size()
			<< '\n' << "</td></tr>\n"
			<< "<tr><td>\n"
			<< "maximum config size=" << control->get_config_maximum_size()
			<< '\n' << "</td></tr>\n"
			<< "<tr><td>\n"
			<< "shrunken_=" << control->shrunken_ << '\n' << "</td></tr>\n";
	}

	const tcontainer_* container = dynamic_cast<const tcontainer_*>(widget);

	if(container) {
		out << "<tr><td>\n"
			<< "border_space=" << container->border_space() << '\n'
			<< "</td></tr>\n";
	}
}

void tdebug_layout_graph::grid_generate_info(std::ostream& out,
											 const tgrid* grid,
											 const std::string& parent_id) const
{
	assert(!parent_id.empty());

	// maybe change the order to links, child, widgets so the output of the
	// dot file might look better.

	out << "\n\n\t// The children of " << parent_id << ".\n";

	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {

			const twidget* widget = grid->child(row, col).widget();
			assert(widget);

			widget_generate_info(
					out, widget, get_child_widget_id(parent_id, row, col));
		}
	}

	out << "\n\t// The grid child data of " << parent_id << ".\n";

	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {

			child_generate_info(out,
								grid->child(row, col),
								get_child_id(parent_id, row, col));
		}
	}


	out << "\n\t// The links of " << parent_id << ".\n";

	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {

			// grid -> child
			out << "\t" << parent_id << " -> "
				<< get_child_id(parent_id, row, col) << " [label=\"(" << row
				<< ',' << col << ")\"];\n";

			// child -> widget
			out << "\t" << get_child_id(parent_id, row, col) << " -> "
				<< get_child_widget_id(parent_id, row, col) << ";\n";
		}
	}
}

void tdebug_layout_graph::child_generate_info(std::ostream& out,
											  const tgrid::tchild& child,
											  const std::string& id) const
{
	assert(!id.empty());

	unsigned flags = child.get_flags();

	out << "\t" << id << " [style=\"\", label=<<table border=\"0\" "
						 "cellborder=\"1\" cellspacing=\"0\">\n";
	out << "<tr><td>\n"
		<< "vertical flag=";

	switch(flags & tgrid::VERTICAL_MASK) {
		case tgrid::VERTICAL_GROW_SEND_TO_CLIENT:
			out << "send to client";
			break;
		case tgrid::VERTICAL_ALIGN_TOP:
			out << "align to top";
			break;
		case tgrid::VERTICAL_ALIGN_CENTER:
			out << "center";
			break;
		case tgrid::VERTICAL_ALIGN_BOTTOM:
			out << "align to bottom";
			break;
		default:
			out << "unknown value("
				<< ((flags & tgrid::VERTICAL_MASK) >> tgrid::VERTICAL_SHIFT)
				<< ")";
	}

	out << "\n</td></tr>\n"
		<< "<tr><td>\n"
		<< "horizontal flag=";

	switch(flags & tgrid::HORIZONTAL_MASK) {
		case tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT:
			out << "send to client";
			break;
		case tgrid::HORIZONTAL_ALIGN_LEFT:
			out << "align to left";
			break;
		case tgrid::HORIZONTAL_ALIGN_CENTER:
			out << "center";
			break;
		case tgrid::HORIZONTAL_ALIGN_RIGHT:
			out << "align to right";
			break;
		default:
			out << "unknown value("
				<< ((flags & tgrid::HORIZONTAL_MASK) >> tgrid::HORIZONTAL_SHIFT)
				<< ")";
	}

	out << "\n</td></tr>\n"
		<< "<tr><td>\n"
		<< "border location=";

	if((flags & tgrid::BORDER_ALL) == 0) {
		out << "none";
	} else if((flags & tgrid::BORDER_ALL) == tgrid::BORDER_ALL) {
		out << "all";
	} else {
		std::string result;
		if(flags & tgrid::BORDER_TOP)
			result += "top, ";
		if(flags & tgrid::BORDER_BOTTOM)
			result += "bottom, ";
		if(flags & tgrid::BORDER_LEFT)
			result += "left, ";
		if(flags & tgrid::BORDER_RIGHT)
			result += "right, ";

		if(!result.empty()) {
			result.resize(result.size() - 2);
		}

		out << result;
	}

	out << "\n</td></tr>\n"
		<< "<tr><td>\n"
		<< "border_size=" << child.get_border_size() << "\n</td></tr>\n";

	out << "</table>>];\n";
}

std::string tdebug_layout_graph::get_type(const twidget* widget) const
{
	const tcontrol* control = dynamic_cast<const tcontrol*>(widget);
	if(control) {
		return control->get_control_type();
	} else {
		const tgrid* grid = dynamic_cast<const tgrid*>(widget);
		const tgenerator_* generator = dynamic_cast<const tgenerator_*>(widget);

		if(grid) {
			return "grid";
		} else if(generator) {
			return "generator";
		} else {
			return "unknown";
		}
	}
}

} // namespace gui2
#endif
