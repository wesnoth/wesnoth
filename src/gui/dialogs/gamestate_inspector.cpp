/*
	Copyright (C) 2009 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/gamestate_inspector.hpp"

#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"

#include "desktop/clipboard.hpp"
#include "serialization/markup.hpp"
#include "game_events/manager.hpp"
#include "serialization/parser.hpp" // for write()
#include "serialization/markup.hpp"

#include "gettext.hpp"
#include "recall_list_manager.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "ai/manager.hpp"

#include "display_context.hpp"
#include "video.hpp"

#include <vector>
#include <functional>

namespace
{

inline std::string config_to_string(const config& cfg)
{
	std::ostringstream s;
	write(s, cfg);
	return s.str();
}

inline std::string config_to_string(const config& cfg, const std::string& only_children)
{
	config filtered;
	for(const config& child : cfg.child_range(only_children)) {
		filtered.add_child(only_children, child);
	}
	return config_to_string(filtered);
}

}

namespace gui2::dialogs
{

class gamestate_inspector::model
{
public:
	std::string name;

	std::string get_data_full() const
	{
		return data;
	}

	std::string get_data_paged(int which_page)
	{
		if(std::size_t(which_page) >= pages.size()) {
			return "";
		}
		return data.substr(pages[which_page].first, pages[which_page].second);
	}

	void clear_data()
	{
		data.clear();
		pages.clear();
	}

	void set_data(const std::string& new_data)
	{
		data = new_data;
		repaginate();
	}

	int count_pages()
	{
		return std::max<int>(pages.size(), 1);
	}

private:
	void repaginate()
	{
		pages.clear();
		std::size_t start = 0;
		while(start + page_characters < data.size()) {
			// This could search into data that's already on a previous page, which is why the result
			// is then checked for end < start.
			std::size_t end = data.find_last_of('\n', start + page_characters);
			int len;
			if(end == std::string::npos || end < start) {
				len = page_characters;
			} else {
				len = end - start + 1;
			}
			pages.emplace_back(start, len);
			start += len;
		}
		if(start < data.size()) {
			pages.emplace_back(start, data.size() - start);
		}
	}
	unsigned int page_characters = 10000 / video::get_pixel_scale();
	std::string data;
	std::vector<std::pair<std::size_t,int>> pages;
};

class stuff_list_adder
{
public:
	stuff_list_adder(tree_view_node& stuff_list, const std::string& defn)
		: stuff_list_(stuff_list)
		, defn_(defn)
	{
	}

	std::vector<int> add()
	{
		return stuff_list_.add_child(defn_, data_).describe_path();
	}

	stuff_list_adder& widget(const std::string& ref, const std::string& label, bool markup = false)
	{
		widget_item& item = data_[ref];
		item["label"] = label;
		item["use_markup"] = utils::bool_string(markup);
		return *this;
	}

private:
	tree_view_node& stuff_list_;
	const std::string defn_;
	widget_data data_;
};

class gamestate_inspector::view
{
public:
	view(window& window)
		: stuff_list_(window.find_widget<tree_view>("stuff_list", false, true))
		, inspect_(window.find_widget<styled_widget>("inspect", false, true))
		, pages_(window.find_widget<styled_widget>("page_count", false, true))
		, left_(window.find_widget<styled_widget>("page_left", false, true))
		, right_(window.find_widget<styled_widget>("page_right", false, true))
	{
	}

	stuff_list_adder stuff_list_entry(tree_view_node* parent, const std::string& defn)
	{
		return stuff_list_adder(parent ? *parent : stuff_list_->get_root_node(), defn);
	}

	void update(model& m)
	{
		int n_pages = m.count_pages();
		current_page_ = std::min(n_pages - 1, std::max(0, current_page_));
		inspect_->set_label(m.get_data_paged(current_page_));
		if(n_pages > 1) {
			std::ostringstream out;
			out << current_page_ + 1 << '/' << n_pages;
			pages_->set_label(out.str());
			left_->set_visible(widget::visibility::visible);
			right_->set_visible(widget::visibility::visible);
			left_->set_active(current_page_ > 0);
			right_->set_active(current_page_ < n_pages - 1);
		} else {
			pages_->set_label("");
			left_->set_visible(widget::visibility::invisible);
			right_->set_visible(widget::visibility::invisible);
		}
	}

	void clear_stuff_list()
	{
		stuff_list_->clear();
		pages_->set_label("");
		left_->set_visible(widget::visibility::invisible);
		right_->set_visible(widget::visibility::invisible);
	}

	void page(int where)
	{
		current_page_ += where;
	}

private:
	int current_page_ = 0;
	tree_view* stuff_list_;
	styled_widget* inspect_;
	styled_widget* pages_;
	styled_widget* left_;
	styled_widget* right_;
};

class single_mode_controller
{
public:
	single_mode_controller(gamestate_inspector::controller& c) : c(c)
	{
	}

	virtual ~single_mode_controller()
	{
	}

protected:
	gamestate_inspector::model& model();
	gamestate_inspector::view& view();
	gamestate_inspector::controller& c;
	const config& vars() const;
	const game_events::manager& events() const;
	const display_context& dc() const;
};

class variable_mode_controller : public single_mode_controller
{
public:
	variable_mode_controller(gamestate_inspector::controller& c)
		: single_mode_controller(c)
	{
	}

	void show_list(tree_view_node& node);
	void show_var(tree_view_node& node);
	void show_array(tree_view_node& node);
};

class event_mode_controller : public single_mode_controller
{
public:
	event_mode_controller(gamestate_inspector::controller& c);
	void show_list(tree_view_node& node, bool is_wmi);
	void show_event(tree_view_node& node, bool is_wmi);

private:
	config events;
};

class unit_mode_controller : public single_mode_controller
{
public:
	unit_mode_controller(gamestate_inspector::controller& c)
		: single_mode_controller(c)
	{
	}

	void show_list(tree_view_node& node);
	void show_unit(tree_view_node& node);
	void show_var(tree_view_node& node);
	void show_array(tree_view_node& node);
};

class team_mode_controller : public single_mode_controller
{
public:
	team_mode_controller(gamestate_inspector::controller& c)
		: single_mode_controller(c)
	{
	}

	void show_list(tree_view_node& node, int side);
	void show_ai(tree_view_node& node, int side);
	void show_ai_components(tree_view_node& node, int side);
	void show_ai_tree(tree_view_node& node, int side);
	void show_recall(tree_view_node& node, int side);
	void show_recall_unit(tree_view_node& node, int side);
	void show_units(tree_view_node& node, int side);
	void show_unit(tree_view_node& node, int side);
	void show_vars(tree_view_node& node, int side);
	void show_var(tree_view_node& node, int side);
	void show_array(tree_view_node& node, int side);
};

class gamestate_inspector::controller
{
	friend class single_mode_controller;
public:
	controller(model& m, view& v, const config& vars, const game_events::manager& events, const display_context& dc)
		: model_(m), view_(v)
		, vars_(vars), events_(events), dc_(dc)
	{
	}

	void handle_stuff_list_item_clicked(widget& tree)
	{
		tree_view_node* selected = dynamic_cast<tree_view&>(tree).selected_item();
		callbacks[selected->describe_path()](*selected);

		// We recursively fold, but non-recursively unfold.
		// This is because only one node on a level should be open at any given time.
		// Furthermore, there's no need to remember that a subnode was open once the parent is closed.
		if(!selected->is_root_node()) {
			for(auto& node : selected->parent_node().children()) {
				if(node.get() != selected) {
					node->fold(true);
				}
			}

			selected->unfold();
		}

		view_.update(model_);
	}

	void handle_copy_button_clicked()
	{
		desktop::clipboard::copy_to_clipboard(model_.get_data_full());
	}

	void handle_lua_button_clicked(window& window)
	{
		lua_interpreter::display(lua_interpreter::GAME);
		// The game state could've changed, so reset the dialog
		callbacks.clear();
		controllers.clear();
		view_.clear_stuff_list();
		build_stuff_list(window);
		model_.clear_data();
		view_.update(model_);
	}

	void handle_page_button_clicked(bool next)
	{
		view_.page(next ? 1 : -1);
		view_.update(model_);
	}

	template<typename T>
	std::shared_ptr<T> get_controller()
	{
		for(auto& c : controllers) {
			if(std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(c)) {
				return p;
			}
		}
		std::shared_ptr<T> p = std::make_shared<T>(*this);
		controllers.push_back(p);
		return p;
	}

	template<typename C>
	void set_node_callback(const std::vector<int>& node_path, void (C::* fcn)(tree_view_node&))
	{
		C& sub_controller = *get_controller<C>();
		callbacks.emplace(node_path, std::bind(fcn, sub_controller, std::placeholders::_1));
	}

	template<typename C, typename T>
	void set_node_callback(const std::vector<int>& node_path, void (C::* fcn)(tree_view_node&, T), T param)
	{
		C& sub_controller = *get_controller<C>();
		callbacks.emplace(node_path, std::bind(fcn, sub_controller, std::placeholders::_1, param));
	}

	void bind(window& window)
	{
		auto stuff_list = window.find_widget<tree_view>("stuff_list", false, true);
		auto copy_button = window.find_widget<button>("copy", false, true);
		auto lua_button = window.find_widget<button>("lua", false, true);
		auto left_button = window.find_widget<button>("page_left", false, true);
		auto right_button = window.find_widget<button>("page_right", false, true);

		connect_signal_notify_modified(*stuff_list,
			std::bind(&gamestate_inspector::controller::handle_stuff_list_item_clicked, this, std::placeholders::_1));

		connect_signal_mouse_left_click(
				*copy_button,
				std::bind(&gamestate_inspector::controller::handle_copy_button_clicked,
					this));

		connect_signal_mouse_left_click(
				*lua_button,
				std::bind(&gamestate_inspector::controller::handle_lua_button_clicked,
					this, std::ref(window)));

		connect_signal_mouse_left_click(
				*left_button,
				std::bind(&gamestate_inspector::controller::handle_page_button_clicked,
					this, false));

		connect_signal_mouse_left_click(
				*right_button,
				std::bind(&gamestate_inspector::controller::handle_page_button_clicked,
					this, true));

		left_button->set_visible(widget::visibility::invisible);
		right_button->set_visible(widget::visibility::invisible);
		build_stuff_list(window);
	}

	void build_stuff_list(window& window)
	{
		set_node_callback(
			view_.stuff_list_entry(nullptr, "basic")
				.widget("name", "variables")
				.add(),
			&variable_mode_controller::show_list);
		set_node_callback(
			view_.stuff_list_entry(nullptr, "basic")
				.widget("name", "events")
				.add(),
			&event_mode_controller::show_list,
			false);
		set_node_callback(
			view_.stuff_list_entry(nullptr, "basic")
				.widget("name", "menu items")
				.add(),
			&event_mode_controller::show_list,
			true);
		set_node_callback(
			view_.stuff_list_entry(nullptr, "basic")
				.widget("name", "units")
				.add(),
			&unit_mode_controller::show_list);
		int sides = dc_.teams().size();
		for(int side = 1; side <= sides; side++) {
			std::ostringstream label;
			label << "team " << side;
			const std::string& name = dc_.get_team(side).user_team_name();
			if(!name.empty()) {
				label << " (" << name << ")";
			}
			set_node_callback(
				view_.stuff_list_entry(nullptr, "basic")
					.widget("name", label.str())
					.add(),
				&team_mode_controller::show_list,
				side);
		}
		// Expand initially selected node
		callbacks[{0}](window.find_widget<tree_view>("stuff_list").get_root_node().get_child_at(0));
	}

private:
	model& model_;
	view& view_;
	using node_callback = std::function<void(tree_view_node&)>;
	using node_callback_map = std::map<std::vector<int>, node_callback>;
	std::vector<std::shared_ptr<single_mode_controller>> controllers;
	node_callback_map callbacks;
	const config& vars_;
	const game_events::manager& events_;
	const display_context& dc_;
};

gamestate_inspector::model& single_mode_controller::model() {
	return c.model_;
}

gamestate_inspector::view& single_mode_controller::view() {
	return c.view_;
}

const config& single_mode_controller::vars() const {
	return c.vars_;
}

const game_events::manager& single_mode_controller::events() const {
	return c.events_;
}

const display_context& single_mode_controller::dc() const {
	return c.dc_;
}

event_mode_controller::event_mode_controller(gamestate_inspector::controller& c)
	: single_mode_controller(c)
{
	single_mode_controller::events().write_events(events, true);
}

void variable_mode_controller::show_list(tree_view_node& node)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	for(const auto& attr : vars().attribute_range())
	{
		c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", attr.first)
				.add(),
			&variable_mode_controller::show_var);
	}

	std::map<std::string, std::size_t> wml_array_sizes;

	for(const auto [key, cfg] : vars().all_children_view())
	{
		std::ostringstream cur_str;
		cur_str << "[" << key << "][" << wml_array_sizes[key] << "]";

		this->c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", cur_str.str())
				.add(),
			&variable_mode_controller::show_array);
		wml_array_sizes[key]++;
	}
}

void variable_mode_controller::show_var(tree_view_node& node)
{
	widget* w = node.find("name", false);
	if(label* lbl = dynamic_cast<label*>(w)) {
		model().set_data(vars()[lbl->get_label().str()]);
	}
}

void variable_mode_controller::show_array(tree_view_node& node)
{
	widget* w = node.find("name", false);
	if(label* lbl = dynamic_cast<label*>(w)) {
		const std::string& var = lbl->get_label();
		std::size_t n_start = var.find_last_of('[') + 1;
		std::size_t n_len = var.size() - n_start - 1;
		int n = std::stoi(var.substr(n_start, n_len));
		model().set_data(config_to_string(vars().mandatory_child(var.substr(1, n_start - 3), n)));
	}
}

void event_mode_controller::show_list(tree_view_node& node, bool is_wmi)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	for(const auto & cfg : events.child_range(is_wmi ? "menu_item" : "event"))
	{
		std::string name = is_wmi ? cfg["id"] : cfg["name"];
		bool named_event = !is_wmi && !cfg["id"].empty();

		auto progress = view()
			.stuff_list_entry(&node, named_event ? "named_event" : "basic")
			.widget("name", name);

		if(named_event) {
			std::ostringstream out;
			out << "id=\"" << cfg["id"] << '"';
			progress.widget("id", out.str());
		}

		c.set_node_callback(progress.add(), &event_mode_controller::show_event, is_wmi);
	}

}

void event_mode_controller::show_event(tree_view_node& node, bool is_wmi)
{
	int n = node.describe_path().back();
	model().set_data(config_to_string(events.mandatory_child(is_wmi ? "menu_item" : "event", n)));
}

static stuff_list_adder add_unit_entry(stuff_list_adder& progress, const unit& u, const display_context& dc)
{

	color_t team_color = game_config::tc_info(dc.get_team(u.side()).color())[0];
	std::stringstream s;

	s << '(' << u.get_location() << ')';
	progress.widget("loc", s.str());

	s.str("");
	s << markup::span_color(team_color, "side=", u.side());
	progress.widget("side", s.str(), true);

	if(u.can_recruit()) {
		progress.widget("leader", markup::span_color("yellow", "LEADER "), true);
	}

	s.str("");
	s << "id=\"" << u.id() << '"';
	progress.widget("id", s.str());

	progress.widget("type", u.type_id());

	s.str("");
	s << "L" << u.level();
	progress.widget("level", s.str());

	s.str("");
	s << u.experience() << '/' << u.max_experience() << " xp";
	progress.widget("xp", s.str());

	s.str("");
	s << u.hitpoints() << '/' << u.max_hitpoints() << " hp";
	progress.widget("hp", s.str());

	progress.widget("traits", utils::join(u.get_traits_list(), ", "));

	return progress;
}

void unit_mode_controller::show_list(tree_view_node& node)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	for(unit_map::const_iterator i = dc().units().begin(); i != dc().units().end(); ++i) {
		auto progress = view().stuff_list_entry(&node, "unit");
		add_unit_entry(progress, *i, dc());
		c.set_node_callback(progress.add(), &unit_mode_controller::show_unit);
	}
}

void unit_mode_controller::show_unit(tree_view_node& node)
{
	int i = node.describe_path().back();
	unit_map::const_iterator u = dc().units().begin();
	std::advance(u, i);
	config c_unit;
	u->write(c_unit);
	model().set_data(config_to_string(c_unit));

	if(node.count_children() > 0) {
		return;
	}

	for(const auto& attr : u->variables().attribute_range())
	{
		c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", attr.first)
				.add(),
			&unit_mode_controller::show_var);
	}

	std::map<std::string, std::size_t> wml_array_sizes;

	for(const auto [key, cfg] : u->variables().all_children_view())
	{
		std::ostringstream cur_str;
		cur_str << "[" << key << "][" << wml_array_sizes[key] << "]";

		this->c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", cur_str.str())
				.add(),
			&unit_mode_controller::show_array);
		wml_array_sizes[key]++;
	}
}

void unit_mode_controller::show_var(tree_view_node& node)
{
	widget* w = node.find("name", false);
	int i = node.describe_path().back();
	unit_map::const_iterator u = dc().units().begin();
	std::advance(u, i);
	if(label* lbl = dynamic_cast<label*>(w)) {
		model().set_data(u->variables()[lbl->get_label().str()]);
	}
}

void unit_mode_controller::show_array(tree_view_node& node)
{
	widget* w = node.find("name", false);
	int i = node.describe_path().back();
	unit_map::const_iterator u = dc().units().begin();
	std::advance(u, i);
	if(label* lbl = dynamic_cast<label*>(w)) {
		const std::string& var = lbl->get_label();
		std::size_t n_start = var.find_last_of('[') + 1;
		std::size_t n_len = var.size() - n_start - 1;
		int n = std::stoi(var.substr(n_start, n_len));
		model().set_data(config_to_string(u->variables().mandatory_child(var.substr(1, n_start - 3), n)));
	}
}

void team_mode_controller::show_list(tree_view_node& node, int side)
{
	config&& cfg = dc().get_team(side).to_config();
	cfg.clear_children("ai");
	model().set_data(config_to_string(cfg));

	if(node.count_children() > 0) {
		return;
	}

	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "ai")
			.add(),
		&team_mode_controller::show_ai,
		side);
	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "recall list")
			.add(),
		&team_mode_controller::show_recall,
		side);
	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "units")
			.add(),
		&team_mode_controller::show_units,
		side);
	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "variables")
			.add(),
		&team_mode_controller::show_vars,
		side);
}

void team_mode_controller::show_ai(tree_view_node& node, int side)
{
	model().set_data(ai::manager::get_singleton().get_active_ai_overview_for_side(side));

	if(node.count_children() > 0) {
		return;
	}

	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "engines")
			.add(),
		&team_mode_controller::show_ai_components,
		side);
	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "stages")
			.add(),
		&team_mode_controller::show_ai_components,
		side);
	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "aspects")
			.add(),
		&team_mode_controller::show_ai_components,
		side);
	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "goals")
			.add(),
		&team_mode_controller::show_ai_components,
		side);
	c.set_node_callback(
		view().stuff_list_entry(&node, "basic")
			.widget("name", "component structure")
			.add(),
		&team_mode_controller::show_ai_tree,
		side);
}

void team_mode_controller::show_ai_components(tree_view_node& node, int side)
{
	widget* w = node.find("name", false);
	if(label* lbl = dynamic_cast<label*>(w)) {
		std::string tag = lbl->get_label();
		tag.pop_back();
		model().set_data(config_to_string(ai::manager::get_singleton().to_config(side), tag));
	}
}

void team_mode_controller::show_recall(tree_view_node& node, int side)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	for(const unit_ptr& u : dc().get_team(side).recall_list()) {
		auto progress = view().stuff_list_entry(&node, "unit");
		add_unit_entry(progress, *u, dc());
		c.set_node_callback(progress.add(), &team_mode_controller::show_recall_unit, side);
	}
}

void team_mode_controller::show_recall_unit(tree_view_node& node, int side)
{
	int i = node.describe_path().back();
	auto u = dc().get_team(side).recall_list().begin();
	std::advance(u, i);
	config c_unit;
	(*u)->write(c_unit);
	model().set_data(config_to_string(c_unit));
}

void team_mode_controller::show_ai_tree(tree_view_node&, int side)
{
	model().set_data(ai::manager::get_singleton().get_active_ai_structure_for_side(side));
}

void team_mode_controller::show_units(tree_view_node&, int side)
{
	std::ostringstream s;
	for(unit_map::const_iterator i = dc().units().begin(); i != dc().units().end();
		++i) {
		if(i->side() != side) {
			continue;
		}
		s << '(' << i->get_location() << ") ";
		if(i->can_recruit()) {
			s << "LEADER ";
		}

		s << "\nid=\"" << i->id() << "\" (" << i->type_id() << ")\n"
		  << "L" << i->level() << "; " << i->experience() << '/'
		  << i->max_experience() << " XP; " << i->hitpoints() << '/'
		  << i->max_hitpoints() << " HP\n";
		for(const auto & str : i->get_traits_list())
		{
			s << "\t" << str << std::endl;
		}
		s << std::endl;
	}
	model().set_data(s.str());
}

void team_mode_controller::show_vars(tree_view_node& node, int side)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	const team& t = dc().get_team(side);

	for(const auto& attr : t.variables().attribute_range())
	{
		c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", attr.first)
				.add(),
			&team_mode_controller::show_var,
			side);
	}

	std::map<std::string, std::size_t> wml_array_sizes;

	for(const auto [key, cfg] : t.variables().all_children_view())
	{
		std::ostringstream cur_str;
		cur_str << "[" << key << "][" << wml_array_sizes[key] << "]";

		this->c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", cur_str.str())
				.add(),
			&team_mode_controller::show_array,
			side);
		wml_array_sizes[key]++;
	}
}

void team_mode_controller::show_var(tree_view_node& node, int side)
{
	widget* w = node.find("name", false);
	const team& t = dc().get_team(side);
	if(label* lbl = dynamic_cast<label*>(w)) {
		model().set_data(t.variables()[lbl->get_label().str()]);
	}
}

void team_mode_controller::show_array(tree_view_node& node, int side)
{
	widget* w = node.find("name", false);
	const team& t = dc().get_team(side);
	if(label* lbl = dynamic_cast<label*>(w)) {
		const std::string& var = lbl->get_label();
		std::size_t n_start = var.find_last_of('[') + 1;
		std::size_t n_len = var.size() - n_start - 1;
		int n = std::stoi(var.substr(n_start, n_len));
		model().set_data(config_to_string(t.variables().mandatory_child(var.substr(1, n_start - 3), n)));
	}
}

REGISTER_DIALOG(gamestate_inspector)

gamestate_inspector::gamestate_inspector(const config& vars, const game_events::manager& events, const display_context& dc, const std::string& title)
	: modal_dialog(window_id())
	, title_(title)
	, vars_(vars)
	, events_(events)
	, dc_(dc)
{
	model_.reset(new model);
}

void gamestate_inspector::pre_show()
{
	view_.reset(new view(*this));
	controller_.reset(new controller(*model_, *view_, vars_, events_, dc_));

	if(!title_.empty()) {
		find_widget<styled_widget>("inspector_name").set_label(title_);
	}
	controller_->bind(*this);
	view_->update(*model_);
}

} // namespace dialogs
