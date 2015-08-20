/*
   Copyright (C) 2009 - 2015 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "desktop/clipboard.hpp"
#include "game_events/manager.hpp"
#include "serialization/parser.hpp" // for write()
#include "utils/foreach.tpp"

#include "../../game_data.hpp"
#include "../../recall_list_manager.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"
#include "../../unit.hpp"
#include "../../unit_map.hpp"
#include "../../ai/manager.hpp"

#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

namespace
{

inline std::string config_to_string(const config& cfg)
{
	std::ostringstream s;
	write(s, cfg);
	return s.str();
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


/*
static void inspect_ai(twindow& window, int side)
{
	const config &ai_cfg = ai::manager::to_config(side);
	NEW_find_widget<tcontrol>(
			&window,
			"inspect",
			false).set_label(config_to_string(ai_cfg.debug));
}
*/

/**
 * Template for dialog callbacks for dialogs using model-view-controller
 * architecture pattern.
 *
 * It allows delegating of the callback to a private view class, which should
 * be accessible via dialog->get_view() (and return a pointer to the view
 * class). Example usage: widget->set_callback(dialog_callback<my_dialog_class,
 * my_dialog_class::inner_view_class
 * &my_dialog_class::inner_view_class::member_function>);
 */
template <class D, class V, void (V::*fptr)()>
void dialog_view_callback(twidget& caller)
{
	D* dialog = dynamic_cast<D*>(caller.dialog());
	assert(dialog);
	(*(dialog->get_view()).*fptr)();
}


// The model is an interface defining the data to be displayed or otherwise
// acted upon in the user interface.
class tgamestate_inspector::model
{
public:
	model(const vconfig& c)
		: cfg(c)
		, name()
		, stuff_list()
		, stuff_types_list()
		, inspect()
		, inspector_name()
		, copy_button()
		, lua_button()
	{
		name = cfg["name"].str();
	}

	vconfig cfg;
	std::string name;

	tlistbox* stuff_list;
	tlistbox* stuff_types_list;
	tcontrol* inspect;
	tcontrol* inspector_name;
	tbutton* copy_button;
	tbutton* lua_button;

	static const unsigned int max_inspect_win_len = 20000;


	void clear_stuff_list()
	{
		stuff_list->clear();
	}


	void clear_stuff_types_list()
	{
		stuff_types_list->clear();
	}

	void add_row_to_stuff_list(const std::string& id, const std::string& label)
	{
		std::map<std::string, string_map> data;
		string_map item;
		item["id"] = id;
		item["label"] = label;
		data.insert(std::make_pair("name", item));
		stuff_list->add_row(data);
	}


	void add_row_to_stuff_types_list(const std::string& id,
									 const std::string& label)
	{
		std::map<std::string, string_map> data;
		string_map item;
		item["id"] = id;
		item["label"] = label;
		data.insert(std::make_pair("typename", item));
		stuff_types_list->add_row(data);
	}


	void set_inspect_window_text(const std::string& s, unsigned int page = 0)
	{
		unsigned int reminder = (s.length() - (max_inspect_win_len * page));
		std::string s_ = s.substr(max_inspect_win_len * page, reminder > max_inspect_win_len ? max_inspect_win_len : reminder);
		inspect->set_label(s_);
	}


	unsigned int get_num_page(const std::string& s)
	{
		return (s.length() / max_inspect_win_len) + (s.length() % max_inspect_win_len > 0 ? 1 : 0);
	}
};

class single_mode_controller
{
public:
	single_mode_controller(const std::string& name,
						   tgamestate_inspector::model& m)
		: model_(m), name_(name)
	{
	}
	virtual ~single_mode_controller()
	{
	}

	std::string name() const
	{
		return name_;
	}

	virtual void show_stuff_list() = 0;
	virtual void handle_stuff_list_selection() = 0;
	virtual void update_view_from_model() = 0;

protected:
	tgamestate_inspector::model& model_;
	std::string name_;
};

class variable_mode_controller : public single_mode_controller
{
public:
	variable_mode_controller(const std::string& name,
							 tgamestate_inspector::model& m)
		: single_mode_controller(name, m)
	{
	}

	void show_stuff_list()
	{
		model_.clear_stuff_list();

		const config& vars = resources::gamedata
									 ? resources::gamedata->get_variables()
									 : config();

		FOREACH(const AUTO & a, vars.attribute_range())
		{
			model_.add_row_to_stuff_list(a.first, a.first);
		}

		std::map<std::string, size_t> wml_array_sizes;

		FOREACH(const AUTO & c, vars.all_children_range())
		{
			if (wml_array_sizes.find(c.key) == wml_array_sizes.end()) {
				wml_array_sizes[c.key] = 0;
			} else {
				++wml_array_sizes[c.key];
			}

			unsigned int num_pages = model_.get_num_page(config_to_string(c.cfg));
			for (unsigned int i = 0; i < num_pages; i++) {
				std::ostringstream cur_str;
				cur_str << "[" << c.key << "][" << wml_array_sizes[c.key] << "]";
				if (num_pages > 1) {
					cur_str << " " << (i + 1) << "/" << num_pages;
				}
				model_.add_row_to_stuff_list(cur_str.str(), cur_str.str());
			}
		}

		model_.set_inspect_window_text("");
	}

	void handle_stuff_list_selection()
	{
		int selected = model_.stuff_list->get_selected_row();
		if(selected == -1) {
			model_.set_inspect_window_text("");
			return;
		}

		int i = 0; ///@todo replace with precached data
		const config& vars = resources::gamedata
									 ? resources::gamedata->get_variables()
									 : config();

		FOREACH(const AUTO & a, vars.attribute_range())
		{
			if(selected == i) {
				model_.set_inspect_window_text(a.second);
				return;
			}
			i++;
		}

		FOREACH(const AUTO & c, vars.all_children_range())
		{
			for (unsigned int j = 0; j < model_.get_num_page(config_to_string(c.cfg)); ++j) {
				if (selected == i) {
					model_.set_inspect_window_text(config_to_string(c.cfg), j);
					return;
				}
				i++;
			}
		}
	}

	virtual void update_view_from_model()
	{
		show_stuff_list();
		handle_stuff_list_selection();
	}
};

/**
 * Controller for WML event and menu item handler views.
 */
class event_mode_controller : public single_mode_controller
{
public:
	/** WML event handler tag identifier. */
	enum HANDLER_TYPE
	{
		EVENT_HANDLER,	/**< Standard WML event handler ([event]). */
		WMI_HANDLER		/**< WML menu item handler ([menu_item]). */
	};

	/**
	 * Constructor.
	 *
	 * @param name        View name displayed on the UI.
	 * @param m           Inspector model reference.
	 * @param wml_node    WML node key. Should be either "event" or
	 *                    "menu_item".
	 */
	event_mode_controller(const std::string& name,
						  tgamestate_inspector::model& m,
						  HANDLER_TYPE handler_type)
		: single_mode_controller(name, m)
		, handler_type_(handler_type)
		, pages_()
	{
	}

	/**
	 * Populates the list of pages for this view.
	 */
	virtual void show_stuff_list()
	{
		assert(resources::game_events);

		const std::string handler_key
				= handler_type_ == WMI_HANDLER ? "menu_item" : "event";

		model_.clear_stuff_list();
		pages_.clear();
		config events_config;
		resources::game_events->write_events(events_config);

		FOREACH(const AUTO & cfg, events_config.child_range(handler_key))
		{
			shared_string_ptr sstrp(new std::string(config_to_string(cfg)));
			const std::string& wmltext = *sstrp;
			const unsigned num_pages = model_.get_num_page(wmltext);

			for(unsigned i = 0; i < num_pages; ++i) {
				pages_.push_back(std::make_pair(sstrp, i));

				std::ostringstream o;

				if(handler_type_ == WMI_HANDLER) {
					// [menu_item]
					o << cfg["id"].str();
				} else {
					// [event]
					o << cfg["name"].str();
					if(!cfg["id"].empty()) {
						o << " [id=\"" << cfg["id"].str() << "\"]";
					}
				}

				if(num_pages > 1) {
					o << " [page" << (i + 1) << '/' << num_pages << ']';
				}

				model_.add_row_to_stuff_list(o.str(), o.str());
			}
		}

		model_.set_inspect_window_text("");
	}

	/**
	 * Updates the page display for the currently selected item in this view.
	 */
	virtual void handle_stuff_list_selection()
	{
		const int row = model_.stuff_list->get_selected_row();
		if(row < 0 || size_t(row) >= pages_.size()) {
			model_.set_inspect_window_text("");
			return;
		}

		const page_descriptor& pd = pages_[size_t(row)];
		model_.set_inspect_window_text(*pd.first, pd.second);
	}

	/**
	 * Updates the whole view (page list and page display).
	 */
	virtual void update_view_from_model()
	{
		show_stuff_list();
		handle_stuff_list_selection();
	}

private:
	// Because of GUI2's limitations, we need to use set_inspect_window_text's
	// option to split view pages into multiple subpages. Each of those
	// subpages is built from a shared string object we cache beforehand for
	// cycle efficiency. We use a vector of page content pointers/subpage
	// number pairs matching the UI layout to keep things simple.

	typedef boost::shared_ptr<std::string> shared_string_ptr;
	typedef std::pair<shared_string_ptr, unsigned> page_descriptor;

	HANDLER_TYPE handler_type_;
	std::vector<page_descriptor> pages_;
};

class unit_mode_controller : public single_mode_controller
{
public:
	unit_mode_controller(const std::string& name,
						 tgamestate_inspector::model& m)
		: single_mode_controller(name, m)
	{
	}

	void show_stuff_list()
	{
		model_.clear_stuff_list();

		if(resources::units) {
			for(unit_map::iterator i = resources::units->begin();
				i != resources::units->end();
				++i) {

				std::stringstream s;
				s << '(' << i->get_location();
				s << ") side=" << i->side() << ' ';
				if(i->can_recruit()) {
					s << "LEADER ";
				}

				s << "\nid=\"" << i->id() << "\" (" << i->type_id() << ")\n"
				  << "L" << i->level() << "; " << i->experience() << '/'
				  << i->max_experience() << " xp; " << i->hitpoints() << '/'
				  << i->max_hitpoints() << " hp;";
				FOREACH(const AUTO & str, i->get_traits_list())
				{
					s << " " << str;
				}

				std::string key = s.str();
				model_.add_row_to_stuff_list(key, key);
			}
		}

		model_.set_inspect_window_text("");
	}

	void handle_stuff_list_selection()
	{
		int selected = model_.stuff_list->get_selected_row();
		if(selected == -1) {
			model_.set_inspect_window_text("");
			return;
		}

		if(resources::units) {
			int i = 0; ///@todo replace with precached data
			for(unit_map::iterator u = resources::units->begin();
				u != resources::units->end();
				++u) {
				if(selected == i) {
					config c_unit;
					u->write(c_unit);
					std::ostringstream cfg_str;
					write(cfg_str, c_unit);
					model_.set_inspect_window_text(cfg_str.str());
					return;
				}
				i++;
			}
		}
		model_.set_inspect_window_text("");
	}

	virtual void update_view_from_model()
	{
		show_stuff_list();
		handle_stuff_list_selection();
	}
};


class team_mode_controller : public single_mode_controller
{
public:
	team_mode_controller(const std::string& name,
						 tgamestate_inspector::model& m,
						 int side)
		: single_mode_controller(name, m), side_(side)
	{
	}

	void show_stuff_list()
	{
		model_.clear_stuff_list();
		// note: needs sync with handle_stuff_list_selection()
		model_.add_row_to_stuff_list("overview", "overview");
		model_.add_row_to_stuff_list("ai overview", "ai overview");
		model_.add_row_to_stuff_list("ai config full", "ai config full");
		model_.add_row_to_stuff_list("recall list overview",
									 "recall list overview");
		model_.add_row_to_stuff_list("recall list full", "recall list full");
		model_.add_row_to_stuff_list("ai component structure",
									 "ai component structure");
		model_.add_row_to_stuff_list("unit list overview",
									 "unit list overview");
		model_.set_inspect_window_text("");
	}

	void handle_stuff_list_selection()
	{
		int selected = model_.stuff_list->get_selected_row();
		if(selected == -1) {
			model_.set_inspect_window_text("");
			return;
		}

		if(selected == 0) {
			config c = resources::teams
							   ? resources::teams->at(side_ - 1).to_config()
							   : config();
			c.clear_children("ai");
			c.clear_children("village");
			model_.set_inspect_window_text(config_to_string(c));
			return;
		}

		if(selected == 1) {
			model_.set_inspect_window_text(
					ai::manager::get_active_ai_overview_for_side(side_));
			return;
		}

		if(selected == 2) {
			model_.set_inspect_window_text(
					config_to_string(ai::manager::to_config(side_)));
			return;
		}

		if(selected == 3) {
			std::stringstream s;
			if (resources::teams) {
				FOREACH(const AUTO & u, resources::teams->at(side_ - 1).recall_list())
				{
					s << "id=\"" << u->id() << "\" (" << u->type_id() << ")\nL"
					  << u->level() << "; " << u->experience() << "/"
					  << u->max_experience() << " xp; " << u->hitpoints() << "/"
					  << u->max_hitpoints() << " hp\n";
					FOREACH(const AUTO & str, u->get_traits_list())
					{
						s << "\t" << str << std::endl;
					}
					s << std::endl;
				}
			}
			model_.set_inspect_window_text(s.str());
			return;
		}

		if(selected == 4) {
			config c;
			if (resources::teams) {
				FOREACH(const AUTO & u, resources::teams->at(side_ - 1).recall_list())
				{
					config c_unit;
					u->write(c_unit);
					c.add_child("unit", c_unit);
				}
			}
			model_.set_inspect_window_text(config_to_string(c));
			return;
		}

		if(selected == 5) {
			model_.set_inspect_window_text(
					ai::manager::get_active_ai_structure_for_side(side_));
			return;
		}


		if(selected == 6) {
			std::stringstream s;
			if(resources::units) {
				for(unit_map::iterator i = resources::units->begin();
					i != resources::units->end();
					++i) {
					if(i->side() != side_) {
						continue;
					}
					s << '(' << i->get_location() << ") ";
					if(i->can_recruit()) {
						s << "LEADER ";
					}

					s << "\nid=\"" << i->id() << "\" (" << i->type_id() << ")\n"
					  << "L" << i->level() << "; " << i->experience() << '/'
					  << i->max_experience() << " xp; " << i->hitpoints() << '/'
					  << i->max_hitpoints() << " hp\n";
					FOREACH(const AUTO & str, i->get_traits_list())
					{
						s << "\t" << str << std::endl;
					}
					s << std::endl;
				}
			}
			model_.set_inspect_window_text(s.str());
			return;
		}
	}

	virtual void update_view_from_model()
	{
		show_stuff_list();
		handle_stuff_list_selection();
	}

private:
	int side_;
};


// The controller acts upon the model. It retrieves data from repositories,
// persists it, manipulates it, and determines how it will be displayed in the
// view.
class tgamestate_inspector::controller
{
public:
	typedef std::vector<boost::shared_ptr<single_mode_controller> >
	sm_controller_ptr_vector;
	controller(model& m) : model_(m), sm_controllers_()
	{
		sm_controllers_.push_back(boost::shared_ptr<single_mode_controller>(
				new variable_mode_controller("variables", model_)));
		sm_controllers_.push_back(boost::shared_ptr<single_mode_controller>(
				new event_mode_controller(
						"events", model_, event_mode_controller::EVENT_HANDLER)));
		sm_controllers_.push_back(boost::shared_ptr<single_mode_controller>(
				new event_mode_controller(
						"menu items", model_, event_mode_controller::WMI_HANDLER)));
		sm_controllers_.push_back(boost::shared_ptr<single_mode_controller>(
				new unit_mode_controller("units", model_)));
		// BOOST_FOREACHteam
		int sides = resources::teams
							? static_cast<int>((*resources::teams).size())
							: 0;
		for(int side = 1; side <= sides; ++side) {
			std::string side_str = str_cast(side);
			sm_controllers_.push_back(boost::shared_ptr<single_mode_controller>(
					new team_mode_controller(
							std::string("team ") + side_str, model_, side)));
		}
	}

	boost::shared_ptr<single_mode_controller> get_sm_controller()
	{
		int selected = model_.stuff_types_list->get_selected_row();
		if(selected == -1) {
			// TODO: select row (maybe remember last selected row?...)
			selected = 0;
		}
		return sm_controllers_.at(selected);
	}

	void show_stuff_types_list()
	{
		model_.clear_stuff_types_list();
		FOREACH(AUTO sm_controller, sm_controllers_)
		{
			model_.add_row_to_stuff_types_list(sm_controller->name(),
											   sm_controller->name());
		}
	}


	void show_title()
	{
		model_.inspector_name->set_label(model_.name);
	}


	void update_view_from_model()
	{
		boost::shared_ptr<single_mode_controller> c = get_sm_controller();

		show_title();
		c->update_view_from_model();
	}


	void handle_stuff_list_item_clicked()
	{
		model_.set_inspect_window_text("");
		boost::shared_ptr<single_mode_controller> c = get_sm_controller();
		c->handle_stuff_list_selection();
	}


	void handle_stuff_types_list_item_clicked()
	{
		model_.clear_stuff_list();
		model_.set_inspect_window_text("");
		boost::shared_ptr<single_mode_controller> c = get_sm_controller();
		c->update_view_from_model(); // TODO: 'activate'
	}

	void handle_copy_button_clicked()
	{
		desktop::clipboard::copy_to_clipboard(model_.inspect->label(), false);
	}

	void handle_lua_button_clicked(CVideo & video)
	{
		tlua_interpreter::display(video, tlua_interpreter::GAME);
	}

private:
	model& model_;
	sm_controller_ptr_vector sm_controllers_;
};


// The view is an interface that displays data (the model) and routes user
// commands to the controller to act upon that data.
class tgamestate_inspector::view
{
public:
	view(const vconfig& cfg) : model_(cfg), controller_(model_)
	{
	}

	void pre_show(CVideo& /*video*/, twindow& /*window*/)
	{
		controller_.show_stuff_types_list();
		controller_.update_view_from_model();
	}


	void handle_stuff_list_item_clicked()
	{
		controller_.handle_stuff_list_item_clicked();
	}

	void handle_stuff_types_list_item_clicked()
	{
		controller_.handle_stuff_types_list_item_clicked();
	}

	void handle_copy_button_clicked(twindow& /*window*/)
	{
		controller_.handle_copy_button_clicked();
	}

	void handle_lua_button_clicked(twindow& window)
	{
		controller_.handle_lua_button_clicked(window.video());
	}

	void bind(twindow& window)
	{
		model_.stuff_list
				= &find_widget<tlistbox>(&window, "stuff_list", false);
		model_.stuff_types_list
				= &find_widget<tlistbox>(&window, "stuff_types_list", false);
		model_.inspect = find_widget<tcontrol>(&window, "inspect", false, true);
		model_.inspector_name
				= &find_widget<tcontrol>(&window, "inspector_name", false);
		model_.copy_button
				= &find_widget<tbutton>(&window, "copy", false);
		model_.lua_button
				= &find_widget<tbutton>(&window, "lua", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
		connect_signal_notify_modified(
				*model_.stuff_list,
				boost::bind(&tgamestate_inspector::view::
									 handle_stuff_list_item_clicked,
							this));

		connect_signal_notify_modified(
				*model_.stuff_types_list,
				boost::bind(&tgamestate_inspector::view::
									 handle_stuff_list_item_clicked,
							this));

#else
		model_.stuff_list->set_callback_value_change(
				dialog_view_callback<tgamestate_inspector,
									 tgamestate_inspector::view,
									 &tgamestate_inspector::view::
											  handle_stuff_list_item_clicked>);

		model_.stuff_types_list->set_callback_value_change(
				dialog_view_callback<tgamestate_inspector,
									 tgamestate_inspector::view,
									 &tgamestate_inspector::view::
											  handle_stuff_types_list_item_clicked>);
#endif

		connect_signal_mouse_left_click(
				*model_.copy_button,
				boost::bind(&tgamestate_inspector::view::handle_copy_button_clicked,
							this,
							boost::ref(window)));

		connect_signal_mouse_left_click(
				*model_.lua_button,
				boost::bind(&tgamestate_inspector::view::handle_lua_button_clicked,
							this,
							boost::ref(window)));

		if (!desktop::clipboard::available()) {
			model_.copy_button->set_active(false);
			model_.copy_button->set_tooltip(_("Clipboard support not found, contact your packager"));
		}
	}

private:
	model model_;
	controller controller_;
};


REGISTER_DIALOG(gamestate_inspector)

tgamestate_inspector::tgamestate_inspector(const vconfig& cfg)
	: view_(new view(cfg))
{
}

boost::shared_ptr<tgamestate_inspector::view> tgamestate_inspector::get_view()
{
	return view_;
}

void tgamestate_inspector::pre_show(CVideo& video, twindow& window)
{
	view_->bind(window);
	view_->pre_show(video, window);
}

} // end of namespace gui2
