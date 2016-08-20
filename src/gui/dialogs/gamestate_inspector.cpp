/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "gui/dialogs/gamestate_inspector.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/iterator/walker.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "desktop/clipboard.hpp"
#include "game_events/manager.hpp"
#include "serialization/parser.hpp" // for write()

#include "game_board.hpp"
#include "game_data.hpp"
#include "recall_list_manager.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "ai/manager.hpp"

#include "display_context.hpp"
#include "filter_context.hpp"

#include <vector>
#include "utils/functional.hpp"

namespace
{

inline std::string config_to_string(const config& cfg)
{
	std::ostringstream s;
	write(s, cfg);
	return s.str();
}

inline std::string config_to_string(const config& cfg, std::string only_children)
{
	config filtered;
	for(const config& child : cfg.child_range(only_children)) {
		filtered.add_child(only_children, child);
	}
	return config_to_string(filtered);
}

}

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_gamestate_inspector
 *
 * == Gamestate inspector ==
 *
 * This shows the gamestate inspector
 *
 * @begin{table}{dialog_widgets}
 *
 * inspector_name & & control & m &
 *         Name of the inspector. $
 *
 * stuff_list & & control & m &
 *         List of various stuff that can be viewed. $
 *
 * inspect & & control & m &
 *         The state of the variable or event. $
 *
 * copy & & button & m &
 *         A button to copy the state to clipboard. $
 *
 * @end{table}
 */

class tgamestate_inspector::model
{
public:
	std::string name;

	std::string get_data_full()
	{
		return data;
	}

	std::string get_data_paged(int which_page)
	{
		if(size_t(which_page) >= pages.size()) {
			return "";
		}
		return data.substr(pages[which_page].first, pages[which_page].second);
	}

	void clear_data()
	{
		data.clear();
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
		size_t start = 0;
		while(start < data.size()) {
			size_t end = data.find_last_of('\n', start + max_inspect_win_len);
			if(end == std::string::npos) {
				end = data.size() - 1;
			}
			int len = end - start + 1;
			pages.emplace_back(start, len);
			start += len;
		}
	}
	static const unsigned int max_inspect_win_len = 20000;
	std::string data;
	std::vector<std::pair<size_t,int>> pages;
};

class stuff_list_adder
{
public:
	stuff_list_adder(ttree_view_node& stuff_list, const std::string& defn)
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
		string_map& item = data_[ref];
		item["label"] = label;
		item["use_markup"] = markup ? "true" : "false";
		return *this;
	}

private:
	ttree_view_node& stuff_list_;
	const std::string defn_;
	std::map<std::string, string_map> data_;
};

class tgamestate_inspector::view
{
public:
	view(twindow& window)
		: stuff_list_(find_widget<ttree_view>(&window, "stuff_list", false, true))
		, inspect_(find_widget<tcontrol>(&window, "inspect", false, true))
		, pages_(find_widget<tcontrol>(&window, "page_count", false, true))
		, left_(find_widget<tcontrol>(&window, "page_left", false, true))
		, right_(find_widget<tcontrol>(&window, "page_right", false, true))
	{
	}

	stuff_list_adder stuff_list_entry(ttree_view_node* parent, const std::string defn)
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
			left_->set_visible(twidget::tvisible::visible);
			right_->set_visible(twidget::tvisible::visible);
		} else {
			pages_->set_label("");
			left_->set_visible(twidget::tvisible::invisible);
			right_->set_visible(twidget::tvisible::invisible);
		}
	}

	void clear_stuff_list()
	{
		stuff_list_->clear();
		pages_->set_label("");
		left_->set_visible(twidget::tvisible::invisible);
		right_->set_visible(twidget::tvisible::invisible);
	}

	void page(int where)
	{
		current_page_ += where;
	}

private:
	int current_page_ = 0;
	ttree_view* stuff_list_;
	tcontrol* inspect_;
	tcontrol* pages_;
	tcontrol* left_;
	tcontrol* right_;
};

class single_mode_controller
{
public:
	single_mode_controller(tgamestate_inspector::controller& c) : c(c)
	{
	}

	virtual ~single_mode_controller()
	{
	}

protected:
	tgamestate_inspector::model& model();
	tgamestate_inspector::view& view();
	tgamestate_inspector::controller& c;
};

class variable_mode_controller : public single_mode_controller
{
public:
	variable_mode_controller(tgamestate_inspector::controller& c)
		: single_mode_controller(c)
		, vars(resources::gamedata->get_variables())
	{
	}

	void show_list(ttree_view_node& node);
	void show_var(ttree_view_node& node);
	void show_array(ttree_view_node& node);

private:
	const config& vars;
};

class event_mode_controller : public single_mode_controller
{
public:
	event_mode_controller(tgamestate_inspector::controller& c)
		: single_mode_controller(c)
	{
		resources::game_events->write_events(events);
	}
	void show_list(ttree_view_node& node, bool is_wmi);
	void show_event(ttree_view_node& node, bool is_wmi);

private:
	config events;
};

class unit_mode_controller : public single_mode_controller
{
public:
	unit_mode_controller(tgamestate_inspector::controller& c)
		: single_mode_controller(c)
		, units(*resources::units)
	{
	}

	void show_list(ttree_view_node& node);
	void show_unit(ttree_view_node& node);

private:
	const unit_map& units;
};

class team_mode_controller : public single_mode_controller
{
public:
	team_mode_controller(tgamestate_inspector::controller& c)
		: single_mode_controller(c)
		, teams(resources::gameboard->teams())
	{
	}

	void show_list(ttree_view_node& node, int side);
	void show_ai(ttree_view_node& node, int side);
	void show_ai_components(ttree_view_node& node, int side);
	void show_ai_tree(ttree_view_node& node, int side);
	void show_recall(ttree_view_node& node, int side);
	void show_recall_unit(ttree_view_node& node, int side);
	void show_units(ttree_view_node& node, int side);
	void show_unit(ttree_view_node& node, int side);

private:
	const std::vector<team>& teams;
};

class tgamestate_inspector::controller
{
	friend class single_mode_controller;
public:
	controller(model& m, view& v) : model_(m), view_(v)
	{
	}

	void handle_stuff_list_item_clicked(twidget& tree)
	{
		ttree_view_node* selected = dynamic_cast<ttree_view&>(tree).selected_item();
		callbacks[selected->describe_path()](*selected);
		if(!selected->is_root_node()) {
			using iter_t = iterator::twalker_*;
			auto v = iterator::twalker_::child;
			for(iter_t w = selected->parent_node().create_walker(); !w->at_end(v); w->next(v)) {
				twidget* wgt = w->get(v);
				if(ttree_view_node* node = dynamic_cast<ttree_view_node*>(wgt)) {
					if(node != selected) {
						node->fold();
					} else {
						node->unfold();
					}
				}
			}
		}
		view_.update(model_);
	}

	void handle_copy_button_clicked()
	{
		desktop::clipboard::copy_to_clipboard(model_.get_data_full(), false);
	}

	void handle_lua_button_clicked(twindow& window)
	{
		tlua_interpreter::display(window.video(), tlua_interpreter::GAME);
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
	T& get_controller()
	{
		for(auto& c : controllers) {
			if(T* p = dynamic_cast<T*>(c)) {
				return *p;
			}
		}
		T* p = new T(*this);
		controllers.push_back(p);
		return *p;
	}

	template<typename C>
	void set_node_callback(const std::vector<int>& node_path, void (C::* fcn)(ttree_view_node&))
	{
		C& sub_controller = get_controller<C>();
		callbacks.emplace(node_path, std::bind(fcn, sub_controller, _1));
	}

	template<typename C, typename T>
	void set_node_callback(const std::vector<int>& node_path, void (C::* fcn)(ttree_view_node&, T), T param)
	{
		C& sub_controller = get_controller<C>();
		callbacks.emplace(node_path, std::bind(fcn, sub_controller, _1, param));
	}

	void bind(twindow& window)
	{
		auto stuff_list = find_widget<ttree_view>(&window, "stuff_list", false, true);
		auto copy_button = find_widget<tbutton>(&window, "copy", false, true);
		auto lua_button = find_widget<tbutton>(&window, "lua", false, true);
		auto left_button = find_widget<tbutton>(&window, "page_left", false, true);
		auto right_button = find_widget<tbutton>(&window, "page_right", false, true);

		stuff_list->set_selection_change_callback(std::bind(&tgamestate_inspector::controller::handle_stuff_list_item_clicked, this, _1));

		connect_signal_mouse_left_click(
				*copy_button,
				std::bind(&tgamestate_inspector::controller::handle_copy_button_clicked,
					this));

		connect_signal_mouse_left_click(
				*lua_button,
				std::bind(&tgamestate_inspector::controller::handle_lua_button_clicked,
					this, std::ref(window)));

		connect_signal_mouse_left_click(
				*left_button,
				std::bind(&tgamestate_inspector::controller::handle_page_button_clicked,
					this, false));

		connect_signal_mouse_left_click(
				*right_button,
				std::bind(&tgamestate_inspector::controller::handle_page_button_clicked,
					this, true));

		left_button->set_visible(twidget::tvisible::invisible);
		right_button->set_visible(twidget::tvisible::invisible);

		if (!desktop::clipboard::available()) {
			copy_button->set_active(false);
			copy_button->set_tooltip(_("Clipboard support not found, contact your packager"));
		}

		build_stuff_list(window);
	}

	void build_stuff_list(twindow& window)
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
		int sides = resources::gameboard ? resources::gameboard->teams().size() : 0;
		for(int side = 1; side <= sides; side++) {
			std::ostringstream label;
			label << "team " << side;
			const std::string& name = resources::gameboard->get_team(side).user_team_name();
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
		callbacks[{0}](find_widget<ttree_view>(&window, "stuff_list", false).get_root_node().get_child_at(0));
	}

private:
	model& model_;
	view& view_;
	using node_callback = std::function<void(ttree_view_node&)>;
	using node_callback_map = std::map<std::vector<int>, node_callback>;
	std::vector<single_mode_controller*> controllers;
	node_callback_map callbacks;
};

tgamestate_inspector::model& single_mode_controller::model() {
	return c.model_;
}

tgamestate_inspector::view& single_mode_controller::view() {
	return c.view_;
}

void variable_mode_controller::show_list(ttree_view_node& node)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	for(const auto & a : vars.attribute_range())
	{
		c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", a.first)
				.add(),
			&variable_mode_controller::show_var);
	}

	std::map<std::string, size_t> wml_array_sizes;

	for(const auto & c : vars.all_children_range())
	{

		std::ostringstream cur_str;
		cur_str << "[" << c.key << "][" << wml_array_sizes[c.key] << "]";

		this->c.set_node_callback(
			view().stuff_list_entry(&node, "basic")
				.widget("name", cur_str.str())
				.add(),
			&variable_mode_controller::show_array);
		wml_array_sizes[c.key]++;
	}
}

void variable_mode_controller::show_var(ttree_view_node& node)
{
	twidget* w = node.find("name", false);
	if(tlabel* label = dynamic_cast<tlabel*>(w)) {
		model().set_data(vars[label->label()]);
	}
}

void variable_mode_controller::show_array(ttree_view_node& node)
{
	twidget* w = node.find("name", false);
	if(tlabel* label = dynamic_cast<tlabel*>(w)) {
		const std::string& var = label->label();
		size_t n_start = var.find_last_of('[') + 1;
		size_t n_len = var.size() - n_start - 1;
		int n = std::stoi(var.substr(n_start, n_len));
		model().set_data(config_to_string(vars.child(var.substr(1, n_start - 3), n)));
	}
}

void event_mode_controller::show_list(ttree_view_node& node, bool is_wmi)
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

void event_mode_controller::show_event(ttree_view_node& node, bool is_wmi)
{
	int n = node.describe_path().back();
	model().set_data(config_to_string(events.child(is_wmi ? "menu_item" : "event", n)));
}

static stuff_list_adder add_unit_entry(stuff_list_adder& progress, const unit& u)
{

	Uint32 team_color = game_config::tc_info(resources::gameboard->get_team(u.side()).color())[0];
	std::stringstream s;

	s << '(' << u.get_location() << ')';
	progress.widget("loc", s.str());

	s.str("");
	s << "<span color='#" << std::hex << team_color << std::dec;
	s << "'>side=" << u.side() << "</span>";
	progress.widget("side", s.str(), true);

	if(u.can_recruit()) {
		progress.widget("leader", "<span color='yellow'>LEADER</span> ", true);
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

void unit_mode_controller::show_list(ttree_view_node& node)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	for(unit_map::iterator i = resources::units->begin(); i != resources::units->end(); ++i) {
		auto progress = view().stuff_list_entry(&node, "unit");
		add_unit_entry(progress, *i);
		c.set_node_callback(progress.add(), &unit_mode_controller::show_unit);
	}
}

void unit_mode_controller::show_unit(ttree_view_node& node)
{
	int i = node.describe_path().back();
	unit_map::const_iterator u = units.begin();
	std::advance(u, i);
	config c_unit;
	u->write(c_unit);
	model().set_data(config_to_string(c_unit));
}

void team_mode_controller::show_list(ttree_view_node& node, int side)
{
	config&& cfg = resources::gameboard->get_team(side).to_config();
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
}

void team_mode_controller::show_ai(ttree_view_node& node, int side)
{
	model().set_data(ai::manager::get_active_ai_overview_for_side(side));

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

void team_mode_controller::show_ai_components(ttree_view_node& node, int side)
{
	twidget* w = node.find("name", false);
	if(tlabel* label = dynamic_cast<tlabel*>(w)) {
		std::string tag = label->label();
		tag.pop_back();
		model().set_data(config_to_string(ai::manager::to_config(side), tag));
	}
}

void team_mode_controller::show_recall(ttree_view_node& node, int side)
{
	model().clear_data();

	if(node.count_children() > 0) {
		return;
	}

	for(const unit_ptr u : resources::gameboard->get_team(side).recall_list()) {
		auto progress = view().stuff_list_entry(&node, "unit");
		add_unit_entry(progress, *u);
		c.set_node_callback(progress.add(), &team_mode_controller::show_recall_unit, side);
	}
}

void team_mode_controller::show_recall_unit(ttree_view_node& node, int side)
{
	int i = node.describe_path().back();
	auto u = resources::gameboard->get_team(side).recall_list().begin();
	std::advance(u, i);
	config c_unit;
	(*u)->write(c_unit);
	model().set_data(config_to_string(c_unit));
}

void team_mode_controller::show_ai_tree(ttree_view_node&, int side)
{
	model().set_data(ai::manager::get_active_ai_structure_for_side(side));
}

void team_mode_controller::show_units(ttree_view_node&, int side)
{
	std::ostringstream s;
	for(unit_map::iterator i = resources::units->begin();
		i != resources::units->end();
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

REGISTER_DIALOG(gamestate_inspector)

tgamestate_inspector::tgamestate_inspector(const std::string& title) : title_(title)
{
	model_.reset(new model);
}

void tgamestate_inspector::pre_show(twindow& window)
{
	view_.reset(new view(window));
	controller_.reset(new controller(*model_, *view_));

	if(!title_.empty()) {
		find_widget<tcontrol>(&window, "inspector_name", false).set_label(title_);
	}
	controller_->bind(window);
	view_->update(*model_);
}

} // end of namespace gui2
