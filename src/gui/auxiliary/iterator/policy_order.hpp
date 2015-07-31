/*
   Copyright (C) 2011 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_ITERATOR_POLICY_ORDER_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_ITERATOR_POLICY_ORDER_HPP_INCLUDED

#include "gui/auxiliary/iterator/exception.hpp"
#include "gui/auxiliary/iterator/policy_visit.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/widget.hpp"

#include <iostream>

namespace gui2
{

namespace iterator
{

namespace policy
{

namespace order
{

template <bool visit_widget, bool visit_grid, bool visit_child>
class tbottom_up : public tvisit<visit_widget, twalker_::widget>,
				   public tvisit<visit_grid, twalker_::grid>,
				   public tvisit<visit_child, twalker_::child>
{
	typedef tvisit<visit_widget, twalker_::widget> tvisit_widget;
	typedef tvisit<visit_grid, twalker_::grid> tvisit_grid;
	typedef tvisit<visit_child, twalker_::child> tvisit_child;

public:
	explicit tbottom_up(twidget& root) : root_(root.create_walker()), stack_()
	{
		TST_GUI_I << "Constructor: ";
		while(!tvisit_child::at_end(*root_)) {
			stack_.push_back(root_);
			root_ = tvisit_child::get(*root_)->create_walker();
			TST_GUI_I << " Down widget '" << operator*().id() << "'.";
		}

		if(!at_end()) {
			TST_GUI_I << " Finished at '" << operator*().id() << "'.\n";
		} else {
			TST_GUI_I << " Finished at the end.\n";
		}
	}

	~tbottom_up()
	{
		delete root_;
		for(std::vector<iterator::twalker_*>::iterator itor = stack_.begin();
			itor != stack_.end();
			++itor) {

			delete *itor;
		}
	}

	bool at_end() const
	{
		return tvisit_widget::at_end(*root_) && tvisit_grid::at_end(*root_)
			   && tvisit_child::at_end(*root_);
	}

	bool next()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to move beyond end of the iteration range."
					  << std::endl;
			throw trange_error("Tried to move beyond end of range.");
		}

		TST_GUI_I << "At '" << operator*().id() << "'.";

		/***** WIDGET *****/
		TST_GUI_I << " Iterate widget:";
		if(!tvisit_widget::at_end(*root_)) {
			switch(tvisit_widget::next(*root_)) {
				case twalker_::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case twalker_::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case twalker_::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of "
								 "the widget iteration range.\n";
					throw trange_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** GRID *****/
		TST_GUI_I << " Iterate grid:";
		if(!tvisit_grid::at_end(*root_)) {
			switch(tvisit_grid::next(*root_)) {
				case twalker_::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case twalker_::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case twalker_::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of "
								 "the grid iteration range.\n";
					throw trange_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** TRAVERSE CHILDREN *****/

		TST_GUI_I << " Iterate child:";
		if(tvisit_child::at_end(*root_)) {
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
		if(!tvisit_child::at_end(*root_)) {
			switch(tvisit_child::next(*root_)) {
				case twalker_::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.";
					break;
				case twalker_::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case twalker_::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of "
								 "the child iteration range.\n";
					throw trange_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " already at the end.";
		}

		while(!tvisit_child::at_end(*root_)) {
			stack_.push_back(root_);
			root_ = tvisit_child::get(*root_)->create_walker();
			TST_GUI_I << " Down widget '" << operator*().id() << "'.";
		}
		TST_GUI_I << " Visit '" << operator*().id() << "'.\n";
		return true;
	}

	twidget& operator*()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to defer beyond end its "
						 "iteration range iterator.\n";
			throw tlogic_error("Tried to defer an invalid iterator.");
		}
		if(!tvisit_widget::at_end(*root_)) {
			return *tvisit_widget::get(*root_);
		}
		if(!tvisit_grid::at_end(*root_)) {
			return *tvisit_grid::get(*root_);
		}
		if(!tvisit_child::at_end(*root_)) {
			return *tvisit_child::get(*root_);
		}
		ERR_GUI_I << "The iterator ended in an unknown "
					 "state while deferring itself.\n";
		throw tlogic_error("Tried to defer an invalid iterator.");
	}

private:
	iterator::twalker_* root_;

	std::vector<iterator::twalker_*> stack_;
};

template <bool visit_widget, bool visit_grid, bool visit_child>
class ttop_down : public tvisit<visit_widget, twalker_::widget>,
				  public tvisit<visit_grid, twalker_::grid>,
				  public tvisit<visit_child, twalker_::child>
{
	typedef tvisit<visit_widget, twalker_::widget> tvisit_widget;
	typedef tvisit<visit_grid, twalker_::grid> tvisit_grid;
	typedef tvisit<visit_child, twalker_::child> tvisit_child;

public:
	explicit ttop_down(twidget& root) : root_(root.create_walker()), stack_()
	{
	}

	~ttop_down()
	{
		delete root_;
		for(std::vector<iterator::twalker_*>::iterator itor = stack_.begin();
			itor != stack_.end();
			++itor) {

			delete *itor;
		}
	}

	bool at_end() const
	{
		return tvisit_widget::at_end(*root_) && tvisit_grid::at_end(*root_)
			   && tvisit_child::at_end(*root_);
	}

	bool next()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to move beyond end of the iteration range."
					  << std::endl;
			throw trange_error("Tried to move beyond end of range.");
		}

		TST_GUI_I << "At '" << operator*().id() << "'.";

		/***** WIDGET *****/
		TST_GUI_I << " Iterate widget:";
		if(!tvisit_widget::at_end(*root_)) {
			switch(tvisit_widget::next(*root_)) {
				case twalker_::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case twalker_::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case twalker_::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of the "
								 "widget iteration range.\n";
					throw trange_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** GRID *****/
		TST_GUI_I << " Iterate grid:";
		if(!tvisit_grid::at_end(*root_)) {
			switch(tvisit_grid::next(*root_)) {
				case twalker_::valid:
					TST_GUI_I << " visit '" << operator*().id() << "'.\n";
					return true;
				case twalker_::invalid:
					TST_GUI_I << " reached the end.";
					break;
				case twalker_::fail:
					TST_GUI_I << "\n";
					ERR_GUI_E << "Tried to move beyond end of the grid "
								 "iteration range.\n";
					throw trange_error("Tried to move beyond end of range.");
			}
		} else {
			TST_GUI_I << " failed.";
		}

		/***** TRAVERSE CHILDREN *****/

		TST_GUI_I << " Iterate child:";
		if(tvisit_child::at_end(*root_)) {
			TST_GUI_I << " reached the end.";
			up();
		} else {
			TST_GUI_I << " proceed.";
		}

		if(!tvisit_child::at_end(*root_)) {
			stack_.push_back(root_);
			root_ = tvisit_child::get(*root_)->create_walker();

			assert(root_);
			assert(!at_end());
			TST_GUI_I << " Down and visit '" << operator*().id() << "'.\n";
			return true;
		}

		TST_GUI_I << " Finished iteration.\n";
		return false;
	}

	twidget& operator*()
	{
		if(at_end()) {
			ERR_GUI_I << "Tried to defer beyond end of the iteration "
						 "range iterator.\n";
			throw tlogic_error("Tried to defer an invalid iterator.");
		}
		if(!tvisit_widget::at_end(*root_)) {
			return *tvisit_widget::get(*root_);
		}
		if(!tvisit_grid::at_end(*root_)) {
			return *tvisit_grid::get(*root_);
		}
		if(!tvisit_child::at_end(*root_)) {
			return *tvisit_child::get(*root_);
		}
		ERR_GUI_I << "The iterator ended in an unknown "
					 "state while deferring iteself.\n";
		throw tlogic_error("Tried to defer an invalid iterator.");
	}

private:
	bool up()
	{
		while(!stack_.empty()) {
			delete root_;

			root_ = stack_.back();
			stack_.pop_back();
			TST_GUI_I << " Up widget '" << operator*().id() << "'. Iterate:";
			switch(tvisit_child::next(*root_)) {
				case twalker_::valid:
					TST_GUI_I << " reached '" << operator*().id() << "'.";
					return true;
				case twalker_::invalid:
					TST_GUI_I << " failed.";
					break;
				case twalker_::fail:
					throw trange_error("Tried to move beyond end of range.");
			}
		}
		return true;
	}

	iterator::twalker_* root_;

	std::vector<iterator::twalker_*> stack_;
};

} // namespace order

} // namespace policy

} // namespace iterator

} // namespace gui2

#endif
