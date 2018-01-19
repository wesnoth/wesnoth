/*
   Copyright (C) 2011 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/auxiliary/iterator/exception.hpp"
#include "gui/auxiliary/iterator/policy_visit.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/widget.hpp"

#include <iostream>

namespace gui2
{

namespace iteration
{

namespace policy
{

namespace order
{

template <bool VW, bool VG, bool VC>
class bottom_up : public visit_level<VW, walker_base::self>,
				   public visit_level<VG, walker_base::internal>,
				   public visit_level<VC, walker_base::child>
{
	typedef visit_level<VW, walker_base::self> visit_widget;
	typedef visit_level<VG, walker_base::internal> visit_grid;
	typedef visit_level<VC, walker_base::child> visit_child;

public:
	explicit bottom_up(widget& root) : root_(root.create_walker()), stack_()
	{
		TST_GUI_I << "Constructor: ";
		while(!visit_child::at_end(*root_)) {
			stack_.push_back(root_);
			root_ = visit_child::get(*root_)->create_walker();
			TST_GUI_I << " Down widget '" << operator*().id() << "'.";
		}

		if(!at_end()) {
			TST_GUI_I << " Finished at '" << operator*().id() << "'.\n";
		} else {
			TST_GUI_I << " Finished at the end.\n";
		}
	}

	~bottom_up()
	{
		delete root_;
		for(std::vector<iteration::walker_base*>::iterator itor = stack_.begin();
			itor != stack_.end();
			++itor) {

			delete *itor;
		}
	}

	bool at_end() const
	{
		return visit_widget::at_end(*root_) && visit_grid::at_end(*root_)
			   && visit_child::at_end(*root_);
	}

	bool next()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to move beyond end of the iteration range."
					  << std::endl;
			throw range_error("Tried to move beyond end of range.");
		}

		TST_GUI_I << "At '" << operator*().id() << "'.";

		/***** WIDGET *****/
		TST_GUI_I << " Iterate widget:";
		if(!visit_widget::at_end(*root_)) {
			switch(visit_widget::next(*root_)) {
				case walker_base::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case walker_base::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case walker_base::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of "
								 "the widget iteration range.\n";
					throw range_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** GRID *****/
		TST_GUI_I << " Iterate grid:";
		if(!visit_grid::at_end(*root_)) {
			switch(visit_grid::next(*root_)) {
				case walker_base::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case walker_base::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case walker_base::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of "
								 "the grid iteration range.\n";
					throw range_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** TRAVERSE CHILDREN *****/

		TST_GUI_I << " Iterate child:";
		if(visit_child::at_end(*root_)) {
			if(stack_.empty()) {
				TST_GUI_I << " Finished iteration.\n";
				return false;
			} else {
				delete root_;

				root_ = stack_.back();
				stack_.pop_back();
				TST_GUI_I << " Up '" << operator*().id() << "'.";
			}
		}
		TST_GUI_I << " Iterate child:";
		if(!visit_child::at_end(*root_)) {
			switch(visit_child::next(*root_)) {
				case walker_base::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.";
					break;
				case walker_base::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case walker_base::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of "
								 "the child iteration range.\n";
					throw range_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " already at the end.";
		}

		while(!visit_child::at_end(*root_)) {
			stack_.push_back(root_);
			root_ = visit_child::get(*root_)->create_walker();
			TST_GUI_I << " Down widget '" << operator*().id() << "'.";
		}
		TST_GUI_I << " Visit '" << operator*().id() << "'.\n";
		return true;
	}

	widget& operator*()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to defer beyond end its "
						 "iteration range iterator.\n";
			throw logic_error("Tried to defer an invalid iterator.");
		}
		if(!visit_widget::at_end(*root_)) {
			return *visit_widget::get(*root_);
		}
		if(!visit_grid::at_end(*root_)) {
			return *visit_grid::get(*root_);
		}
		if(!visit_child::at_end(*root_)) {
			return *visit_child::get(*root_);
		}
		ERR_GUI_I << "The iterator ended in an unknown "
					 "state while deferring itself.\n";
		throw logic_error("Tried to defer an invalid iterator.");
	}

private:
	iteration::walker_base* root_;

	std::vector<iteration::walker_base*> stack_;
};

template <bool VW, bool VG, bool VC>
class top_down : public visit_level<VW, walker_base::self>,
				  public visit_level<VG, walker_base::internal>,
				  public visit_level<VC, walker_base::child>
{
	typedef visit_level<VW, walker_base::self> visit_widget;
	typedef visit_level<VG, walker_base::internal> visit_grid;
	typedef visit_level<VC, walker_base::child> visit_child;

public:
	explicit top_down(widget& root) : root_(root.create_walker()), stack_()
	{
	}

	~top_down()
	{
		delete root_;
		for(std::vector<iteration::walker_base*>::iterator itor = stack_.begin();
			itor != stack_.end();
			++itor) {

			delete *itor;
		}
	}

	bool at_end() const
	{
		return visit_widget::at_end(*root_) && visit_grid::at_end(*root_)
			   && visit_child::at_end(*root_);
	}

	bool next()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to move beyond end of the iteration range."
					  << std::endl;
			throw range_error("Tried to move beyond end of range.");
		}

		TST_GUI_I << "At '" << operator*().id() << "'.";

		/***** WIDGET *****/
		TST_GUI_I << " Iterate widget:";
		if(!visit_widget::at_end(*root_)) {
			switch(visit_widget::next(*root_)) {
				case walker_base::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case walker_base::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case walker_base::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of the "
								 "widget iteration range.\n";
					throw range_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** GRID *****/
		TST_GUI_I << " Iterate grid:";
		if(!visit_grid::at_end(*root_)) {
			switch(visit_grid::next(*root_)) {
				case walker_base::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case walker_base::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case walker_base::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of the grid "
								 "iteration range.\n";
					throw range_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** TRAVERSE CHILDREN *****/

		TST_GUI_I << " Iterate child:";
		if(visit_child::at_end(*root_)) {
			TST_GUI_I << " reached the end.";
			up();
		} else {
			TST_GUI_I << " proceed.";
		}

		if(!visit_child::at_end(*root_)) {
			stack_.push_back(root_);
			root_ = visit_child::get(*root_)->create_walker();

			assert(root_);
			assert(!at_end());
			TST_GUI_I << " Down and visit '" << operator*().id() << "'.\n";
			return true;
		}

		TST_GUI_I << " Finished iteration.\n";
		return false;
	}

	widget& operator*()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to defer beyond end of the iteration "
						 "range iterator.\n";
			throw logic_error("Tried to defer an invalid iterator.");
		}
		if(!visit_widget::at_end(*root_)) {
			return *visit_widget::get(*root_);
		}
		if(!visit_grid::at_end(*root_)) {
			return *visit_grid::get(*root_);
		}
		if(!visit_child::at_end(*root_)) {
			return *visit_child::get(*root_);
		}
		ERR_GUI_I << "The iterator ended in an unknown "
					 "state while deferring iteself.\n";
		throw logic_error("Tried to defer an invalid iterator.");
	}

private:
	bool up()
	{
		while(!stack_.empty()) {
			delete root_;

			root_ = stack_.back();
			stack_.pop_back();
			TST_GUI_I << " Up widget '" << operator*().id() << "'. Iterate:";
			switch(visit_child::next(*root_)) {
				case walker_base::valid:
					TST_GUI_I << " reached '" << operator*().id() << "'.";
					return true;
				case walker_base::invalid:
					TST_GUI_I << " failed.";
					break;
				case walker_base::fail:
					throw range_error("Tried to move beyond end of range.");
			}
		}
		return true;
	}

	iteration::walker_base* root_;

	std::vector<iteration::walker_base*> stack_;
};

} // namespace order

} // namespace policy

} // namespace iteration

} // namespace gui2
