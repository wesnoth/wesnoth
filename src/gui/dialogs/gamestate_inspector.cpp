/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/gamestate_inspector.hpp"

#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"

#include "../../foreach.hpp"
#include "../../gamestatus.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"
#include "../../ai/manager.hpp"

#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_gamestate_inspector
 *
 * == Gamestate inspector ==
 *
 * This shows the gamestate inspector
 *
 * @start_table = grid
 *     (inspector_name) (control) ()   Name of the inspector
 *     (stuff_list) (control) ()       List of various stuff that can be viewed
 *     (inspect) (control) ()          The state of the variable or event
 * @end_table
 */


/*
static void inspect_ai(twindow& window, int side)
{
	const config &ai_cfg = ai::manager::to_config(side);
	NEW_find_widget<tcontrol>(&window, "inspect", false).set_label(ai_cfg.debug());
}
*/

template <class D, class V, void (V::*fptr)(twindow&)>
void dialog_view_callback(twidget* caller)
{
        D* dialog = dynamic_cast<D*>(caller->dialog());
        assert(dialog);
        twindow* window = dynamic_cast<twindow*>(caller->get_window());
        assert(window);
        (*(dialog->get_view()).*fptr)(*window);
}


//The model is an interface defining the data to be displayed or otherwise acted upon in the user interface.
class tgamestate_inspector::model {
public:
	model(const vconfig &cfg)
		: cfg_(cfg), name_(), stuff_list_(),stuff_types_list_(),inspect_(),inspector_name_()
	{
		name_ = cfg_["name"];
	}

	vconfig cfg_;
	std::string name_;

	tlistbox *stuff_list_;
	tlistbox *stuff_types_list_;
	tcontrol *inspect_;
	tcontrol *inspector_name_;


	void clear_stuff_list()
	{
		stuff_list_->clear();
	}


	void clear_stuff_types_list()
	{
		stuff_list_->clear();
	}

	void add_row_to_stuff_list(const std::string &id, const std::string &label)
	{
		std::map<std::string, string_map> data;
		string_map item;
		item["id"] = id;
		item["label"] = label;
		data.insert(std::make_pair("name", item));
		stuff_list_->add_row(data);
	}


	void add_row_to_stuff_types_list(const std::string &id, const std::string &label)
	{
		std::map<std::string, string_map> data;
		string_map item;
		item["id"] = id;
		item["label"] = label;
		data.insert(std::make_pair("typename", item));
		stuff_types_list_->add_row(data);
	}


	void set_inspect_window_text(const std::string &s)
	{
		std::string s_ = s;
		if (s_.length()>20000) {//workaround for known bug
			s_.resize(20000);
		}
		inspect_->set_label(s_);
	}

};

class single_mode_controller {
public:
	single_mode_controller(const std::string name, tgamestate_inspector::model &m)
		:model_(m), name_(name)
	{
	}
	virtual ~single_mode_controller()
	{
	}

	std::string name() {
		return name_;
	}

	virtual void show_stuff_list() = 0;
	virtual void handle_stuff_list_selection() = 0;
	virtual void update_view_from_model() = 0;

protected:
	tgamestate_inspector::model &model_;
	std::string name_;
};

class variable_mode_controller : public single_mode_controller{
public:
	variable_mode_controller(const std::string name, tgamestate_inspector::model &m)
		:single_mode_controller(name,m)
	{
	}

	void show_stuff_list()
	{
		model_.clear_stuff_list();

		const config &vars = resources::state_of_game->get_variables();
		foreach( const config::attribute &a, vars.attribute_range()) {
			model_.add_row_to_stuff_list(a.first,a.first);
		}

		foreach( const config::any_child &c, vars.all_children_range()) {
			model_.add_row_to_stuff_list("["+c.key+"]","["+c.key+"]");
		}

		model_.set_inspect_window_text("");

	}

	void handle_stuff_list_selection()
	{
		int selected = model_.stuff_list_->get_selected_row();
		if (selected==-1) {
			model_.set_inspect_window_text("");
			return;
		}

		int i = 0;//@todo: replace with precached data
		const config &vars = resources::state_of_game->get_variables();

		foreach( const config::attribute &a, vars.attribute_range()) {
			if (selected==i) {
				model_.set_inspect_window_text(a.second);
				return;
			}
			i++;
		}

		foreach( const config::any_child &c, vars.all_children_range()) {
			if (selected==i) {
				model_.set_inspect_window_text(c.cfg.debug());
				return;
			}
			i++;
		}
	}

	virtual void update_view_from_model()
	{
		show_stuff_list();
		handle_stuff_list_selection();
	}
};



class team_mode_controller : public single_mode_controller{
public:
	team_mode_controller(const std::string name, tgamestate_inspector::model &m, int side)
		:single_mode_controller(name,m), side_(side)
	{
	}

	void show_stuff_list()
	{
		model_.clear_stuff_list();
		//note: needs sync with handle_stuff_list_selection()
	       	model_.add_row_to_stuff_list("overview","overview");
	       	model_.add_row_to_stuff_list("ai overview","ai overview");
	       	model_.add_row_to_stuff_list("ai config","ai config");
	       	model_.add_row_to_stuff_list("recall list","recall list");
		model_.set_inspect_window_text("");

	}

	void handle_stuff_list_selection()
	{
		int selected = model_.stuff_list_->get_selected_row();
		if (selected==-1) {
			model_.set_inspect_window_text("");
			return;
		}

		if (selected==0) {
			model_.set_inspect_window_text("team overview here");
			return;
		}

		if (selected==1) {
			model_.set_inspect_window_text("ai overview here");
			return;
		}

		if (selected==2) {
			model_.set_inspect_window_text(ai::manager::to_config(side_).debug());
			return;
		}

		if (selected==3) {
			model_.set_inspect_window_text("recall list here");
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


//The controller acts upon the model. It retrieves data from repositories, persists it, manipulates it, and determines how it will be displayed in the view.
class tgamestate_inspector::controller {
public:
	typedef std::vector< boost::shared_ptr<single_mode_controller> > sm_controller_ptr_vector;
	controller(model &m)
		: model_(m)
	{
		sm_controllers_.push_back( boost::shared_ptr<single_mode_controller>(new variable_mode_controller("variables",model_)));
		//foreach team
		int sides = static_cast<int>((*resources::teams).size());
		for( int side = 1; side<=sides; ++side) {
			std::string side_str = str_cast(side);
			sm_controllers_.push_back( boost::shared_ptr<single_mode_controller>(new team_mode_controller(std::string("team ")+side_str,model_,side)));
		}
	}

	boost::shared_ptr<single_mode_controller> get_sm_controller()
	{
		int selected = model_.stuff_types_list_->get_selected_row();
		if (selected==-1) {
			//TODO: select row
			selected=0;
		}
		return sm_controllers_.at(selected);
	}

	void show_stuff_types_list()
	{
		model_.clear_stuff_types_list();
		foreach (boost::shared_ptr<single_mode_controller> sm_controller, sm_controllers_ ) {
			model_.add_row_to_stuff_types_list(sm_controller->name(),sm_controller->name());
		}
	}


	void show_title()
	{
		model_.inspector_name_->set_label(model_.name_);
	}


	void update_view_from_model()
	{
		boost::shared_ptr<single_mode_controller> c = get_sm_controller();

		show_title();
		c->update_view_from_model();
	}


	void handle_stuff_list_item_clicked()
	{	
		boost::shared_ptr<single_mode_controller> c = get_sm_controller();
		c->handle_stuff_list_selection();
	}


	void handle_stuff_types_list_item_clicked()
	{
		boost::shared_ptr<single_mode_controller> c = get_sm_controller();
		c->update_view_from_model();//TODO: 'activate'
	}


private:
	model &model_;
	sm_controller_ptr_vector sm_controllers_;
};


//The view is an interface that displays data (the model) and routes user commands to the controller to act upon that data.
class tgamestate_inspector::view {
public:
	view(const vconfig &cfg)
		: model_(cfg),controller_(model_)
	{
	}

	void pre_show(CVideo& /*video*/, twindow& window)
	{
		controller_.show_stuff_types_list();
		controller_.update_view_from_model();
		window.invalidate_layout();//workaround for assertion failure
	}


	void handle_stuff_list_item_clicked(twindow &window)
	{
		controller_.handle_stuff_list_item_clicked();
		window.invalidate_layout();//workaround for assertion failure
	}

	void handle_stuff_types_list_item_clicked(twindow &window)
	{
		controller_.handle_stuff_types_list_item_clicked();
		window.invalidate_layout();//workaround for assertion failure
	}


	void bind(twindow &window)
	{
		model_.stuff_list_ = &find_widget<tlistbox>(&window, "stuff_list", false);
		model_.stuff_types_list_ = &find_widget<tlistbox>(&window, "stuff_types_list", false);
		model_.inspect_ = find_widget<tcontrol>(&window, "inspect", false,true);
		model_.inspector_name_ = &find_widget<tcontrol>(&window, "inspector_name", false);

		model_.stuff_list_->set_callback_value_change(
			dialog_view_callback<tgamestate_inspector, tgamestate_inspector::view, &tgamestate_inspector::view::handle_stuff_list_item_clicked>);

		model_.stuff_types_list_->set_callback_value_change(
			dialog_view_callback<tgamestate_inspector, tgamestate_inspector::view, &tgamestate_inspector::view::handle_stuff_types_list_item_clicked>);

	}

private:
	model model_;
	controller controller_;
};


tgamestate_inspector::tgamestate_inspector(const vconfig &cfg)
	: view_()
{

	view_ = boost::shared_ptr<view>(new view(cfg));
}


twindow* tgamestate_inspector::build_window(CVideo& video)
{
	return build(video, get_id(GAMESTATE_INSPECTOR));
}


boost::shared_ptr<tgamestate_inspector::view> tgamestate_inspector::get_view()
{
	return view_;
}


void tgamestate_inspector::pre_show(CVideo& video, twindow& window)
{
	view_->bind(window);
	view_->pre_show(video,window);
}

} //end of namespace gui2
