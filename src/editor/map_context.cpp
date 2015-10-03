/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "action.hpp"
#include "map_context.hpp"

#include "../display.hpp"
#include "../filesystem.hpp"
#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../map_exception.hpp"
#include "../map_label.hpp"
#include "../wml_exception.hpp"

#include "formula_string_utils.hpp"

#include <boost/regex.hpp>


namespace editor {

const size_t map_context::max_action_stack_size_ = 100;

map_context::map_context(const editor_map& map)
	: filename_()
	, map_data_key_()
	, embedded_(false)
	, map_(map)
	, undo_stack_()
	, redo_stack_()
	, actions_since_save_(0)
	, starting_position_label_locs_()
	, needs_reload_(false)
	, needs_terrain_rebuild_(false)
	, needs_labels_reset_(false)
	, changed_locations_()
	, everything_changed_(false)
{
}

map_context::map_context(const config& game_config, const std::string& filename)
	: filename_(filename)
	, map_data_key_()
	, embedded_(false)
	, map_(game_config)
	, undo_stack_()
	, redo_stack_()
	, actions_since_save_(0)
	, starting_position_label_locs_()
	, needs_reload_(false)
	, needs_terrain_rebuild_(false)
	, needs_labels_reset_(false)
	, changed_locations_()
	, everything_changed_(false)
{
	log_scope2(log_editor, "Loading map " + filename);
	if (!file_exists(filename) || is_directory(filename)) {
		throw editor_map_load_exception(filename, _("File not found"));
	}
	std::string map_string = read_file(filename);
	boost::regex re("map_data\\s*=\\s*\"(.+?)\"");
	boost::smatch m;
	if (boost::regex_search(map_string, m, re, boost::regex_constants::match_not_dot_null)) {
		boost::regex re2("\\{(.+?)\\}");
		boost::smatch m2;
		std::string m1 = m[1];
		if (boost::regex_search(m1, m2, re2)) {
			map_data_key_ = m1;
			LOG_ED << "Map looks like a scenario, trying {" << m2[1] << "}\n";
			std::string new_filename = get_wml_location(m2[1], directory_name(m2[1]));
			if (new_filename.empty()) {
				std::string message = _("The map file looks like a scenario, "
					"but the map_data value does not point to an existing file")
					+ std::string("\n") + m2[1];
				throw editor_map_load_exception(filename, message);
			}
			LOG_ED << "New filename is: " << new_filename << "\n";
			filename_ = new_filename;
			map_string = read_file(filename_);
		} else {
			LOG_ED << "Loading embedded map file\n";
			embedded_ = true;
			map_string = m[1];
		}
	}
	if (map_string.empty()) {
		std::string message = _("Empty map file");
		throw editor_map_load_exception(filename, message);
	}
	map_ = editor_map::from_string(game_config, map_string); //throws on error
}

map_context::~map_context()
{
	clear_stack(undo_stack_);
	clear_stack(redo_stack_);
}

void map_context::draw_terrain(t_translation::t_terrain terrain,
	const map_location& loc, bool one_layer_only)
{
	if (!one_layer_only) {
		terrain = map_.get_terrain_info(terrain).terrain_with_default_base();
	}
	draw_terrain_actual(terrain, loc, one_layer_only);
}

void map_context::draw_terrain_actual(t_translation::t_terrain terrain,
	const map_location& loc, bool one_layer_only)
{
	if (!map_.on_board_with_border(loc)) {
		//requests for painting off the map are ignored in set_terrain anyway,
		//but ideally we should not have any
		LOG_ED << "Attempted to draw terrain off the map (" << loc << ")\n";
		return;
	}
	t_translation::t_terrain old_terrain = map_.get_terrain(loc);
	if (terrain != old_terrain) {
		if (terrain.base == t_translation::NO_LAYER) {
			map_.set_terrain(loc, terrain, gamemap::OVERLAY);
		} else if (one_layer_only) {
			map_.set_terrain(loc, terrain, gamemap::BASE);
		} else {
			map_.set_terrain(loc, terrain);
		}
		add_changed_location(loc);
	}
}

void map_context::draw_terrain(t_translation::t_terrain terrain,
	const std::set<map_location>& locs, bool one_layer_only)
{
	if (!one_layer_only) {
		terrain = map_.get_terrain_info(terrain).terrain_with_default_base();
	}
	BOOST_FOREACH (const map_location& loc, locs) {
		draw_terrain_actual(terrain, loc, one_layer_only);
	}
}

void map_context::clear_changed_locations()
{
	everything_changed_ = false;
	changed_locations_.clear();
}

void map_context::add_changed_location(const map_location& loc)
{
	if (!everything_changed()) {
		changed_locations_.insert(loc);
	}
}

void map_context::add_changed_location(const std::set<map_location>& locs)
{
	if (!everything_changed()) {
		changed_locations_.insert(locs.begin(), locs.end());
	}
}

void map_context::set_everything_changed()
{
	everything_changed_ = true;
}

bool map_context::everything_changed() const
{
	return everything_changed_;
}

void map_context::clear_starting_position_labels(display& disp)
{
	disp.labels().clear_all();
	starting_position_label_locs_.clear();
}

void map_context::set_starting_position_labels(display& disp)
{
	std::set<map_location> new_label_locs = map_.set_starting_position_labels(disp);
	starting_position_label_locs_.insert(new_label_locs.begin(), new_label_locs.end());
}

void map_context::reset_starting_position_labels(display& disp)
{
	clear_starting_position_labels(disp);
	set_starting_position_labels(disp);
	set_needs_labels_reset(false);
}

bool map_context::save()
{
	std::string data = map_.write();
	try {
		if (!is_embedded()) {
			write_file(get_filename(), data);
		} else {
			std::string map_string = read_file(get_filename());
			boost::regex re("(.*map_data\\s*=\\s*\")(.+?)(\".*)");
			boost::smatch m;
			if (boost::regex_search(map_string, m, re, boost::regex_constants::match_not_dot_null)) {
				std::stringstream ss;
				ss << m[1];
				ss << data;
				ss << m[3];
				write_file(get_filename(), ss.str());
			} else {
				throw editor_map_save_exception(_("Could not save into scenario"));
			}
		}
		clear_modified();
	} catch (io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = vgettext("Could not save the map: $msg", symbols);
		throw editor_map_save_exception(msg);
	}
	return true;
}

void map_context::set_map(const editor_map& map)
{
	if (map_.h() != map.h() || map_.w() != map.w()) {
		set_needs_reload();
	} else {
		set_needs_terrain_rebuild();
	}
	map_ = map;
}

void map_context::perform_action(const editor_action& action)
{
	LOG_ED << "Performing action " << action.get_id() << ": " << action.get_name()
		<< ", actions count is " << action.get_instance_count() << "\n";
	editor_action* undo = action.perform(*this);
	if (actions_since_save_ < 0) {
		//set to a value that will make it impossible to get to zero, as at this point
		//it is no longer possible to get back the original map state using undo/redo
		actions_since_save_ = 1 + undo_stack_.size();
	}
	actions_since_save_++;
	undo_stack_.push_back(undo);
	trim_stack(undo_stack_);
	clear_stack(redo_stack_);
}

void map_context::perform_partial_action(const editor_action& action)
{
	LOG_ED << "Performing (partial) action " << action.get_id() << ": " << action.get_name()
		<< ", actions count is " << action.get_instance_count() << "\n";
	if (!can_undo()) {
		throw editor_logic_exception("Empty undo stack in perform_partial_action()");
	}
	editor_action_chain* undo_chain = dynamic_cast<editor_action_chain*>(last_undo_action());
	if (undo_chain == NULL) {
		throw editor_logic_exception("Last undo action not a chain in perform_partial_action()");
	}
	editor_action* undo = action.perform(*this);
	//actions_since_save_ += action.action_count();
	undo_chain->prepend_action(undo);
	clear_stack(redo_stack_);
}
bool map_context::modified() const
{
	return actions_since_save_ != 0;
}

void map_context::clear_modified()
{
	actions_since_save_ = 0;
}

bool map_context::can_undo() const
{
	return !undo_stack_.empty();
}

bool map_context::can_redo() const
{
	return !redo_stack_.empty();
}

editor_action* map_context::last_undo_action()
{
	return undo_stack_.empty() ? NULL : undo_stack_.back();
}

editor_action* map_context::last_redo_action()
{
	return redo_stack_.empty() ? NULL : redo_stack_.back();
}

const editor_action* map_context::last_undo_action() const
{
	return undo_stack_.empty() ? NULL : undo_stack_.back();
}

const editor_action* map_context::last_redo_action() const
{
	return redo_stack_.empty() ? NULL : redo_stack_.back();
}

void map_context::undo()
{
	LOG_ED << "undo() beg, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
	if (can_undo()) {
		perform_action_between_stacks(undo_stack_, redo_stack_);
		actions_since_save_--;
	} else {
		WRN_ED << "undo() called with an empty undo stack\n";
	}
	LOG_ED << "undo() end, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
}

void map_context::redo()
{
	LOG_ED << "redo() beg, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
	if (can_redo()) {
		perform_action_between_stacks(redo_stack_, undo_stack_);
		actions_since_save_++;
	} else {
		WRN_ED << "redo() called with an empty redo stack\n";
	}
	LOG_ED << "redo() end, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
}

void map_context::partial_undo()
{
	//callers should check for these conditions
	if (!can_undo()) {
		throw editor_logic_exception("Empty undo stack in partial_undo()");
	}
	editor_action_chain* undo_chain = dynamic_cast<editor_action_chain*>(last_undo_action());
	if (undo_chain == NULL) {
		throw editor_logic_exception("Last undo action not a chain in partial undo");
	}
	//a partial undo performs the first action form the current action's action_chain that would be normally performed
	//i.e. the *first* one.
	std::auto_ptr<editor_action> first_action_in_chain(undo_chain->pop_first_action());
	if (undo_chain->empty()) {
		actions_since_save_--;
		delete undo_chain;
		undo_stack_.pop_back();
	}
	redo_stack_.push_back(first_action_in_chain.get()->perform(*this));
	//actions_since_save_ -= last_redo_action()->action_count();
}

void map_context::clear_undo_redo()
{
	clear_stack(undo_stack_);
	clear_stack(redo_stack_);
}

void map_context::trim_stack(action_stack& stack)
{
	if (stack.size() > max_action_stack_size_) {
		delete stack.front();
		stack.pop_front();
	}
}

void map_context::clear_stack(action_stack& stack)
{
	BOOST_FOREACH (editor_action* a, stack) {
		delete a;
	}
	stack.clear();
}

void map_context::perform_action_between_stacks(action_stack& from, action_stack& to)
{
	assert(!from.empty());
	std::auto_ptr<editor_action> action(from.back());
	from.pop_back();
	editor_action* reverse_action = action->perform(*this);
	to.push_back(reverse_action);
	trim_stack(to);
}

} //end namespace editor
