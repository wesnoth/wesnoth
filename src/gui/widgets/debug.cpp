/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
#include "utils/io.hpp"

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

	// return formatter() << parent_id << "_C_" << row << '_' << col;
	std::string result = parent_id + "_C_" + std::to_string(row)
						 + '_' + std::to_string(col);

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
	std::ostringstream ss;

	time_t t = time(nullptr);
	ss << utils::put_time(std::localtime(&t), "%Y%m%d_%H%M%S");

	static unsigned counter = 0;
	++counter;

	ss << '_' << counter << '_';

	return ss.str();
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

debug_layout_graph::debug_layout_graph(const window* window)
	: window_(window), sequence_number_(0), filename_base_(get_base_filename())
{
}

void debug_layout_graph::set_level(const std::string& level)
{
	if(level.empty()) {
		level_ = ALL; /** @todo Should default to 0. */
		return;
	}

	std::vector<std::string> params = utils::split(level);

	for(const auto & param : params)
	{
		if(param == "all") {
			level_ = ALL;
			// No need to look further even though invalid items are now
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

void debug_layout_graph::set_domain(const std::string& domain)
{
	if(domain.empty()) {
		// return error and die
		domain_ = ALL; /** @todo Should default to 0. */
		return;
	}

	std::vector<std::string> params = utils::split(domain);

	for(const auto & param : params)
	{
		if(param == "all") {
			domain_ = ALL;
			// No need to look further even though invalid items are now
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

void debug_layout_graph::generate_dot_file(const std::string& generator,
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
								 + std::to_string(++sequence_number_)
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

void debug_layout_graph::widget_generate_info(std::ostream& out,
											   const widget* widget,
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

	const grid* grid = dynamic_cast<const class grid*>(widget);
	if(!grid) {
		const container_base* container = dynamic_cast<const container_base*>(widget);

		if(container) {

			widget_generate_info(out, &container->get_grid(), id + "_G", true);
			out << "\t" << id << " -> " << id << "_G"
				<< " [label=\"(grid)\"];\n";
		}

		const scrollbar_container* scrollbar_container
				= dynamic_cast<const class scrollbar_container*>(widget);

		if(scrollbar_container) {
			widget_generate_info(
            out, scrollbar_container->content_grid_.get(), id + "_C", true);
			out << "\t" << id << " -> " << id << "_C"
				<< " [label=\"(content)\"];\n";
		}

		const listbox* listbox = dynamic_cast<const class listbox*>(widget);
		if(listbox) {
			assert(listbox->generator_);
		}

		const generator_base* generator = dynamic_cast<const generator_base*>(widget);

		if(generator) {
			for(size_t i = 0; i < generator->get_item_count(); ++i) {

				const std::string child_id = id + "_I_"
											 + std::to_string(i);

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

void debug_layout_graph::widget_generate_basic_info(std::ostream& out,
													 const widget* widget)
		const
{
	std::string header_background
			= level_ & (SIZE_INFO | STATE_INFO) ? " bgcolor=\"gray\"" : "";
	const styled_widget* control = dynamic_cast<const class styled_widget*>(widget);

	out << "<tr><td" << header_background << ">" << '\n'
		<< "type=" << get_type(widget) << '\n' << "</td></tr>" << '\n'
		<< "<tr><td" << header_background << ">" << '\n'
		<< "id=" << widget->id() << '\n' << "</td></tr>" << '\n' << "<tr><td"
		<< header_background << ">" << '\n' << "address=" << widget << '\n'
		<< "</td></tr>" << '\n' << "<tr><td" << header_background << ">" << '\n'
		<< "parent=" << widget->parent_ << '\n' << "</td></tr>" << '\n';
	if(control) {
		out << "<tr><td" << header_background << ">" << '\n'
			<< "label=" << format_label(control->get_label()) << '\n' << "<tr><td"
			<< header_background << ">" << '\n'
			<< "definition=" << control->definition_ << '\n' << "</td></tr>"
			<< '\n' << "</td></tr>\n";
	}
}

void debug_layout_graph::widget_generate_state_info(std::ostream& out,
													 const widget* widget)
		const
{
	const styled_widget* control = dynamic_cast<const styled_widget*>(widget);
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
        << "visible=" << static_cast<int>(control->get_visible()) << '\n' << "</td></tr>\n"
		<< "<tr><td>\n"
		<< "drawing action=" << static_cast<int>(control->get_drawing_action()) << '\n'
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

	const scrollbar_container* scrollbar_container
			= dynamic_cast<const class scrollbar_container*>(widget);

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

void debug_layout_graph::widget_generate_size_info(std::ostream& out,
													const widget* widget) const
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


	const styled_widget* control = dynamic_cast<const styled_widget*>(widget);

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

	const container_base* container = dynamic_cast<const container_base*>(widget);

	if(container) {
		out << "<tr><td>\n"
			<< "border_space=" << container->border_space() << '\n'
			<< "</td></tr>\n";
	}
}

void debug_layout_graph::grid_generate_info(std::ostream& out,
											 const grid* grid,
											 const std::string& parent_id) const
{
	assert(!parent_id.empty());

	// maybe change the order to links, child, widgets so the output of the
	// dot file might look better.

	out << "\n\n\t// The children of " << parent_id << ".\n";

	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {

            const widget* widget = grid->get_widget(row, col);
			assert(widget);

			widget_generate_info(
					out, widget, get_child_widget_id(parent_id, row, col));
		}
	}

	out << "\n\t// The grid child data of " << parent_id << ".\n";

	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {

			child_generate_info(out,
								grid->get_child(row, col),
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

void debug_layout_graph::child_generate_info(std::ostream& out,
											  const grid::child& child,
											  const std::string& id) const
{
	assert(!id.empty());

	unsigned flags = child.get_flags();

	out << "\t" << id << " [style=\"\", label=<<table border=\"0\" "
						 "cellborder=\"1\" cellspacing=\"0\">\n";
	out << "<tr><td>\n"
		<< "vertical flag=";

	switch(flags & grid::VERTICAL_MASK) {
		case grid::VERTICAL_GROW_SEND_TO_CLIENT:
			out << "send to client";
			break;
		case grid::VERTICAL_ALIGN_TOP:
			out << "align to top";
			break;
		case grid::VERTICAL_ALIGN_CENTER:
			out << "center";
			break;
		case grid::VERTICAL_ALIGN_BOTTOM:
			out << "align to bottom";
			break;
		default:
			out << "unknown value("
				<< ((flags & grid::VERTICAL_MASK) >> grid::VERTICAL_SHIFT)
				<< ")";
	}

	out << "\n</td></tr>\n"
		<< "<tr><td>\n"
		<< "horizontal flag=";

	switch(flags & grid::HORIZONTAL_MASK) {
		case grid::HORIZONTAL_GROW_SEND_TO_CLIENT:
			out << "send to client";
			break;
		case grid::HORIZONTAL_ALIGN_LEFT:
			out << "align to left";
			break;
		case grid::HORIZONTAL_ALIGN_CENTER:
			out << "center";
			break;
		case grid::HORIZONTAL_ALIGN_RIGHT:
			out << "align to right";
			break;
		default:
			out << "unknown value("
				<< ((flags & grid::HORIZONTAL_MASK) >> grid::HORIZONTAL_SHIFT)
				<< ")";
	}

	out << "\n</td></tr>\n"
		<< "<tr><td>\n"
		<< "border location=";

	if((flags & grid::BORDER_ALL) == 0) {
		out << "none";
	} else if((flags & grid::BORDER_ALL) == grid::BORDER_ALL) {
		out << "all";
	} else {
		std::string result;
		if(flags & grid::BORDER_TOP)
			result += "top, ";
		if(flags & grid::BORDER_BOTTOM)
			result += "bottom, ";
		if(flags & grid::BORDER_LEFT)
			result += "left, ";
		if(flags & grid::BORDER_RIGHT)
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

std::string debug_layout_graph::get_type(const widget* widget) const
{
	const styled_widget* control = dynamic_cast<const styled_widget*>(widget);
	if(control) {
		return control->get_control_type();
	} else {
		const grid* grid = dynamic_cast<const class grid*>(widget);
		const generator_base* generator = dynamic_cast<const generator_base*>(widget);

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
