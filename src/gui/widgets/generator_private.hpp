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

#ifndef GUI_WIDGETS_GENERATOR_PRIVATE_HPP_INCLUDED
#define GUI_WIDGETS_GENERATOR_PRIVATE_HPP_INCLUDED

#include "gui/widgets/generator.hpp"

#include "asserts.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/selectable.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "utils/foreach.tpp"

namespace gui2
{

/**
 * Contains the policies for the tgenerator class.
 */
namespace policy
{

/***** ***** ***** ***** Minimum selection ***** ***** ***** *****/

/** Contains the policy for the minimum number of selected items. */
namespace minimum_selection
{

/** Must select at least one item. */
struct tone : public virtual tgenerator_
{
	/**
	 * Called when an item is shown or hidden.
	 *
	 * @param index               The item to show or hide.
	 * @param show                If true shows the item, else hides it.
	 */
	void set_item_shown(const unsigned index, const bool show);

	/**
	 * Called when an item is created.
	 *
	 * @param index               The index of the new item.
	 */
	void create_item(const unsigned index);

	/* Also make the overload from the generator_ visible. */
	using tgenerator_::create_item;

	/**
	 * Called when the users wants to deselect an item.
	 *
	 * If the item can be deselected this function should call
	 * do_deselect_item() to make the deslection happen. If not allowed no
	 * action needs to be taken.
	 *
	 * @param index               The index of the item to deselect.
	 *
	 * @returns                   Whether the item was deselected, some
	 *                            actions might happen automatically upon
	 *                            deselecting, so if this function returns
	 *                            false the caller should make sure the
	 *                            select state is restored.
	 */
	bool deselect_item(const unsigned index);

	/**
	 * Called just before an item is deleted.
	 *
	 * This function can if needed select another items to try to obey the
	 * policy.
	 *
	 * @param index              The index of the item to be deleted.
	 */
	void delete_item(const unsigned index);
};

/** No minimum selection. */
struct tnone : public virtual tgenerator_
{

	/** See @ref minimum_selection::tone::set_item_shown(). */
	void set_item_shown(const unsigned index, const bool show);

	/** See @ref minimum_selection::tone::create_item() */
	void create_item(const unsigned /*index*/)
	{
	}

	/* Also make the overload from the generator_ visible. */
	using tgenerator_::create_item;

	/** See @ref minimum_selection::tone::deselect_item() */
	bool deselect_item(const unsigned index)
	{
		do_deselect_item(index);
		return true;
	}

	/** See @ref minimum_selection::tone::delete_item() */
	void delete_item(const unsigned index)
	{
		if(is_selected(index)) {
			do_deselect_item(index);
		}
	}
};

} // namespace minimum_selection

/***** ***** ***** ***** Maximum selection ***** ***** ***** *****/

/** Contains the policy for the maximum number of selected items. */
namespace maximum_selection
{

/** May select only one item. */
struct tone : public virtual tgenerator_
{
	/**
	 * Called when an item is selected.
	 *
	 * This function can deselect other items to obey the policy. This
	 * function should always call do_select_item() so the new item does get
	 * selected.
	 *
	 * Since this funcion controls the maximum selection count it should only
	 * be used to select items, not to deselect them.
	 *
	 * @pre                       @p select == @c true
	 *
	 * @param index               The item to select.
	 */
	void select_item(const unsigned index, const bool select) OVERRIDE
	{
		assert(select);

		if(get_selected_item_count() == 1) {
			// deselect current.
			do_deselect_item(get_selected_item());
			// select new.
			do_select_item(index);
		}
	}
};

/** No maximum amount of items to select. */
struct tinfinite : public virtual tgenerator_
{
	/** See tone::select_item(). */
	void select_item(const unsigned index, const bool select) OVERRIDE
	{
		assert(select);

		do_select_item(index);
	}
};

} // namespace maximum_selection

/***** ***** ***** ***** Placement ***** ***** ***** *****/

/** Controls how new items are placed. */
namespace placement
{

/** Places the items in a horizontal row. */
struct thorizontal_list : public virtual tgenerator_
{
	thorizontal_list();

	/**
	 * Called when an item is created.
	 *
	 * This function should place the new item.
	 *
	 * @param index                  The index of the new item.
	 */
	void create_item(const unsigned index);

	/* Also make the overload from the generator_ visible. */
	using tgenerator_::create_item;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned /*maximum_width*/) OVERRIDE
	{
		/* DO NOTHING */
	}

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned /*maximum_height*/)
			OVERRIDE
	{
		/* DO NOTHING */
	}

	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE;

	/**
	 * Sets the visible rectangle of the generator.
	 *
	 * @param rectangle           The visible rectangle.
	 */
	void set_visible_rectangle(const SDL_Rect& rectangle);

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from tgenerator_. */
	void handle_key_up_arrow(SDLMod /*modifier*/, bool& /*handled*/)
	{
		/* do nothing */
	}

	/** Inherited from tgenerator_. */
	void handle_key_down_arrow(SDLMod /*modifier*/, bool& /*handled*/)
	{
		/* do nothing */
	}

	/** Inherited from tgenerator_. */
	void handle_key_left_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tgenerator_. */
	void handle_key_right_arrow(SDLMod modifier, bool& handled);

private:
	/**
	 * Has the grid already been placed?
	 *
	 * If the grid is placed it's no problem set the location of the new
	 * item,it hasn't been placed, there's no information about its location
	 * so do nothing.
	 */
	bool placed_;
};

/** Places the items in a vertical column. */
struct tvertical_list : public virtual tgenerator_
{
	tvertical_list();

	/** See thorizontal_list::create_item(). */
	void create_item(const unsigned index);

	/* Also make the overload from the generator_ visible. */
	using tgenerator_::create_item;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned /*maximum_width*/) OVERRIDE
	{
		/* DO NOTHING */
	}

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned /*maximum_height*/)
			OVERRIDE
	{
		/* DO NOTHING */
	}

	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE;

	/** See @ref thorizontal_list::set_visible_rectangle(). */
	void set_visible_rectangle(const SDL_Rect& rectangle);

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from tgenerator_. */
	void handle_key_up_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tgenerator_. */
	void handle_key_down_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tgenerator_. */
	void handle_key_left_arrow(SDLMod /*modifier*/, bool& /*handled*/)
	{ /* do nothing */
	}

	/** Inherited from tgenerator_. */
	void handle_key_right_arrow(SDLMod /*modifier*/, bool& /*handled*/)
	{ /* do nothing */
	}

	// FIXME we need a delete handler as well,
	// when deleting the last item we need to remove the placed flag.

	// FIXME we also need a clear function, called when
	// clear is called.
private:
	/**
	 * Has the grid already been placed?
	 *
	 * If the grid is placed it's no problem set the location of the new
	 * item,it hasn't been placed, there's no information about its location
	 * so do nothing.
	 */
	bool placed_;
};

/**
 * Places the items in a grid.
 *
 * The items will be placed in rows and columns. It has to be determined
 * whether the number of columns will be fixed or variable.
 *
 * @todo Implement.
 */
struct tmatrix : public virtual tgenerator_
{
	/** See thorizontal_list::create_item(). */
	void create_item(const unsigned /*index*/)
	{
		ERROR_LOG(false);
	}

	/* Also make the overload from the generator_ visible. */
	using tgenerator_::create_item;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned /*maximum_width*/) OVERRIDE
	{
		/* DO NOTHING */
	}

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned /*maximum_height*/)
			OVERRIDE
	{
		/* DO NOTHING */
	}

	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE
	{
		ERROR_LOG(false);
	}

	/** See @ref twidget::place. */
	virtual void place(const tpoint& /*origin*/
					   ,
					   const tpoint& /*size*/) OVERRIDE
	{
		ERROR_LOG(false);
	}

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& /*origin*/) OVERRIDE
	{
		ERROR_LOG(false);
	}

	/** See @ref thorizontal_list::set_visible_rectangle(). */
	void set_visible_rectangle(const SDL_Rect& /*rectangle*/)
	{
		ERROR_LOG(false);
	}

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& /*coordinate*/
							 ,
							 const bool /*must_be_active*/) OVERRIDE
	{
		ERROR_LOG(false);
	}

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& /*coordinate*/
								   ,
								   const bool /*must_be_active*/) const OVERRIDE
	{
		ERROR_LOG(false);
	}

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from tgenerator_. */
	void handle_key_up_arrow(SDLMod, bool&)
	{
		ERROR_LOG(false);
	}

	/** Inherited from tgenerator_. */
	void handle_key_down_arrow(SDLMod, bool&)
	{
		ERROR_LOG(false);
	}

	/** Inherited from tgenerator_. */
	void handle_key_left_arrow(SDLMod, bool&)
	{
		ERROR_LOG(false);
	}

	/** Inherited from tgenerator_. */
	void handle_key_right_arrow(SDLMod, bool&)
	{
		ERROR_LOG(false);
	}
};

/**
 * Places the items independent of each other.
 *
 * This is mainly meant for when only one item is shown at the same time.
 *
 * @todo Implement.
 */
struct tindependent : public virtual tgenerator_
{
	/** See thorizontal_list::create_item(). */
	void create_item(const unsigned /*index*/)
	{
		/* DO NOTHING */
	}

	/* Also make the overload from the generator_ visible. */
	using tgenerator_::create_item;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) OVERRIDE;

	/** See thorizontal_list::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height);

	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE;

	/** See @ref thorizontal_list::set_visible_rectangle(). */
	void set_visible_rectangle(const SDL_Rect& rectangle);

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/** See @ref twidget::find. */
	twidget* find(const std::string& id, const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find. */
	const twidget* find(const std::string& id,
						const bool must_be_active) const OVERRIDE;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from tgenerator_. */
	void handle_key_up_arrow(SDLMod, bool&)
	{
		/* DO NOTHING */
	}

	/** Inherited from tgenerator_. */
	void handle_key_down_arrow(SDLMod, bool&)
	{
		/* DO NOTHING */
	}

	/** Inherited from tgenerator_. */
	void handle_key_left_arrow(SDLMod, bool&)
	{
		/* DO NOTHING */
	}

	/** Inherited from tgenerator_. */
	void handle_key_right_arrow(SDLMod, bool&)
	{
		/* DO NOTHING */
	}
};

} // namespace placement

/***** ***** ***** ***** Select action ***** ***** ***** *****/

/**
 * Contains the policy for which action to take when an item is selected or
 * deselected.
 */
namespace select_action
{

/** Select the item, this requires the grid to contain a tselectable_. */
struct tselect : public virtual tgenerator_
{
	void select(tgrid& grid, const bool select);

	/**
	 * Helper function to initialize a grid.
	 *
	 * @param grid                The grid to initialize.
	 * @param data                The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	void init(tgrid* grid,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const boost::function<void(twidget&)>& callback);
};

/** Show the item. */
struct tshow : public virtual tgenerator_
{
	void select(tgrid& grid, const bool show)
	{
		grid.set_visible(show ? twidget::tvisible::visible
							  : twidget::tvisible::hidden);
	}

	/**
	 * Helper function to initialize a grid.
	 *
	 * @param grid                The grid to initialize.
	 * @param data                The data to initialize the parameters of
	 *                            the new item. No widgets with id == "" are
	 *                            allowed.
	 * @param callback            The callback function is not used and
	 *                            should be NULL.
	 */
	void init(tgrid* grid,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const boost::function<void(twidget&)>& callback);
};

} // namespace select_action

} // namespace policy

/***** ***** ***** ***** tgenerator ***** ***** ***** *****/

/**
 * Basic template class to generate new items.
 *
 * The class is policy based so the behavior can be selected.
 */
template <class minimum_selection,
		  class maximum_selection,
		  class placement,
		  class select_action>
class tgenerator : public minimum_selection,
				   public maximum_selection,
				   public placement,
				   public select_action
{
public:
	tgenerator()
		: minimum_selection()
		, maximum_selection()
		, placement()
		, select_action()
		, selected_item_count_(0)
		, last_selected_item_(-1)
		, items_()
	{
	}

	~tgenerator()
	{
		clear();
	}

	/***** ***** ***** inherited ***** ****** *****/

	/** Inherited from tgenerator_. */
	void delete_item(const unsigned index)
	{
		assert(index < items_.size());

		// Might be other parts of the engine want to know about the
		// deselection, if minimum fails it gets another chance later on,
		// since it deletes the item.
		if(is_selected(index)) {
			select_item(index, false);
		}

		minimum_selection::delete_item(index);

		delete items_[index];
		items_.erase(items_.begin() + index);
	}

	/** Inherited from tgenerator_. */
	void clear()
	{
		FOREACH(AUTO item, items_)
		{
			delete item;
		}
		selected_item_count_ = 0;
	}

	/** Inherited from tgenerator_. */
	void select_item(const unsigned index, const bool select = true)
	{
		assert(index < items_.size());

		if(select && !is_selected(index)) {
			maximum_selection::select_item(index, true);
			last_selected_item_ = index;
		} else if(is_selected(index)) {
			if(!minimum_selection::deselect_item(index)) {
				// Some items might have deseleted themselves so
				// make sure they do get selected again.
				select_action::select(item(index), true);
			}
		}
	}

	/** Inherited from tgenerator_. */
	bool is_selected(const unsigned index) const
	{
		assert(index < items_.size());
		return (*items_[index]).selected;
	}

	/** Inherited from tgenerator_. */
	void set_item_shown(const unsigned index, const bool show)
	{
		assert(index < items_.size());
		if(items_[index]->shown != show) {

			/*** Set the proper visible state. ***/
			items_[index]->shown = show;
			items_[index]
					->grid.set_visible(show ? twidget::tvisible::visible
											: twidget::tvisible::invisible);

			/*** Update the selection. ***/
			minimum_selection::set_item_shown(index, show);
		}
	}

	/** Inherited from tgenerator_. */
	virtual bool get_item_shown(const unsigned index) const
	{
		assert(index < items_.size());
		return items_[index]->shown;
	}


	/** Inherited from tgenerator_. */
	unsigned get_item_count() const
	{
		return items_.size();
	}

	/** Inherited from tgenerator_. */
	unsigned get_selected_item_count() const
	{
		return selected_item_count_;
	}

	/** Inherited from tgenerator_. */
	int get_selected_item() const
	{

		if(selected_item_count_ == 0) {
			return -1;
		} else if(last_selected_item_ != -1
				  && last_selected_item_ < static_cast<int>(items_.size())
				  && (*items_[last_selected_item_]).selected) {

			return last_selected_item_;

		} else {
			for(size_t i = 0; i < items_.size(); ++i) {
				if((*items_[i]).selected) {
					return i;
				}
			}
			ERROR_LOG("No item selected.");
		}
	}

	/** Inherited from tgenerator_. */
	tgrid& item(const unsigned index)
	{
		assert(index < items_.size());
		return items_[index]->grid;
	}

	/** Inherited from tgenerator_. */
	const tgrid& item(const unsigned index) const
	{
		assert(index < items_.size());
		return items_[index]->grid;
	}


	/** Inherited from tgenerator_. */
	tgrid& create_item(const int index,
					   tbuilder_grid_const_ptr list_builder,
					   const string_map& item_data,
					   const boost::function<void(twidget&)>& callback)
	{
		std::map<std::string, string_map> data;

		data.insert(std::make_pair("", item_data));
		return create_item(index, list_builder, data, callback);
	}

	/** Inherited from tgenerator_. */
	tgrid& create_item(
			const int index,
			tbuilder_grid_const_ptr list_builder,
			const std::map<std::string /* widget id */, string_map>& item_data,
			const boost::function<void(twidget&)>& callback)
	{
		assert(list_builder);
		assert(index == -1 || static_cast<unsigned>(index) < items_.size());

		titem* item = new titem;
		list_builder->build(&item->grid);
		init(&item->grid, item_data, callback);

		const unsigned item_index = index == -1 ? items_.size() : index;

		items_.insert(items_.begin() + item_index, item);
		minimum_selection::create_item(item_index);
		placement::create_item(item_index);
		if(!is_selected(item_index)) {
			select_action::select(item->grid, false);
		}
		return item->grid;
	}

	/** Inherited from tgenerator_. */
	virtual void create_items(
			const int index,
			tbuilder_grid_const_ptr list_builder,
			const std::vector<std::map<std::string /*widget id*/, string_map> >&
					data,
			const boost::function<void(twidget&)>& callback)
	{
		impl_create_items(index, list_builder, data, callback);
	}

	/** Inherited from tgenerator_. */
	virtual void create_items(const int index,
							  tbuilder_grid_const_ptr list_builder,
							  const std::vector<string_map>& data,
							  const boost::function<void(twidget&)>& callback)
	{
		impl_create_items(index, list_builder, data, callback);
	}

	/** See @ref twidget::layout_initialise. */
	virtual void layout_initialise(const bool full_initialisation) OVERRIDE
	{
		FOREACH(AUTO item, items_)
		{
			if(item->grid.get_visible() != twidget::tvisible::invisible
			   && item->shown) {

				item->grid.layout_initialise(full_initialisation);
			}
		}
	}

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) OVERRIDE
	{
		placement::request_reduce_width(maximum_width);
	}

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) OVERRIDE
	{
		placement::request_reduce_height(maximum_height);
	}

	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE
	{
		return placement::calculate_best_size();
	}

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE
	{
		// Inherited, so we get useful debug info.
		twidget::place(origin, size);

		placement::place(origin, size);
	}

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE
	{
		// Inherited.
		twidget::set_origin(origin);

		placement::set_origin(origin);
	}

	/** See @ref twidget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) OVERRIDE
	{
		placement::set_visible_rectangle(rectangle);
	}

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer) OVERRIDE
	{
		assert(this->get_visible() == twidget::tvisible::visible);

		FOREACH(AUTO item, items_)
		{
			if(item->grid.get_visible() == twidget::tvisible::visible
			   && item->shown) {

				item->grid.draw_children(frame_buffer);
			}
		}
	}

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) OVERRIDE
	{
		assert(this->get_visible() == twidget::tvisible::visible);

		FOREACH(AUTO item, items_)
		{
			if(item->grid.get_visible() == twidget::tvisible::visible
			   && item->shown) {

				item->grid.draw_children(frame_buffer, x_offset, y_offset);
			}
		}
	}

	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) OVERRIDE
	{
		FOREACH(AUTO item, items_)
		{
			std::vector<twidget*> child_call_stack = call_stack;
			item->grid.populate_dirty_list(caller, child_call_stack);
		}
	}

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE
	{
		return placement::find_at(coordinate, must_be_active);
	}

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE
	{
		return placement::find_at(coordinate, must_be_active);
	}

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const OVERRIDE
	{
		FOREACH(AUTO item, items_)
		{
			if(item->grid.disable_click_dismiss()) {
				return true;
			}
		}
		return false;
	}

	/**
	 * See @ref twidget::create_walker.
	 *
	 * @todo Implement properly.
	 */
	virtual iterator::twalker_* create_walker() OVERRIDE
	{
		return NULL;
	}

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from tgenerator_. */
	void handle_key_up_arrow(SDLMod modifier, bool& handled)
	{
		placement::handle_key_up_arrow(modifier, handled);
	}

	/** Inherited from tgenerator_. */
	void handle_key_down_arrow(SDLMod modifier, bool& handled)
	{
		placement::handle_key_down_arrow(modifier, handled);
	}

	/** Inherited from tgenerator_. */
	void handle_key_left_arrow(SDLMod modifier, bool& handled)
	{
		placement::handle_key_left_arrow(modifier, handled);
	}

	/** Inherited from tgenerator_. */
	void handle_key_right_arrow(SDLMod modifier, bool& handled)
	{
		placement::handle_key_right_arrow(modifier, handled);
	}

protected:
	/** Inherited from tgenerator_. */
	void do_select_item(const unsigned index) // fixme rename to impl
	{
		assert(index < items_.size());

		++selected_item_count_;
		set_item_selected(index, true);
	}

	/** Inherited from tgenerator_. */
	void do_deselect_item(const unsigned index)
	{
		assert(index < items_.size());

		--selected_item_count_;
		set_item_selected(index, false);
	}

private:
	/** Definition of an item. */
	struct titem
	{

		titem() : grid(), selected(false), shown(true)
		{
		}

		/** The grid containing the widgets. */
		tgrid grid;

		/** Is the item selected or not. */
		bool selected;

		/**
		 * Is the row shown or not.
		 *
		 * This flag is used the help to set the visible flag, it's preferred to
		 * test this flag for external functions.
		 *
		 * @todo functions now test for visible and shown, that can use some
		 * polishing.
		 */
		bool shown;
	};

	/** The number of selected items. */
	unsigned selected_item_count_;

	/** The last item selected. */
	int last_selected_item_;

	/** The items in the generator. */
	std::vector<titem*> items_;

	/**
	 * Sets the selected state of an item.
	 *
	 * @param index               The item to modify.
	 * @param selected            Select or deselect.
	 */
	void set_item_selected(const unsigned index, const bool selected)
	{
		assert(index < items_.size());

		(*items_[index]).selected = selected;
		select_action::select((*items_[index]).grid, selected);
	}

	/**
	 * Helper function for create_items().
	 *
	 * @tparam T                  Type of the data, this should be one of the
	 *                            valid parameters for create_item().
	 *
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 * @param list_builder        A grid builder that's will build the
	 *                            contents of the new item.
	 * @param data                The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	template <class T>
	void impl_create_items(const int index,
						   tbuilder_grid_const_ptr list_builder,
						   const std::vector<T>& data,
						   const boost::function<void(twidget&)>& callback)
	{
		int i = index;
		FOREACH(const AUTO & item_data, data)
		{
			create_item(i, list_builder, item_data, callback);
			if(i != -1) {
				++i;
			}
		}
	}

	/**
	 * Helper function to initialize a grid.
	 *
	 * The actual part is implemented in select_action, see those
	 * implementations for more information.
	 *
	 * @param grid                The grid to initialize.
	 * @param data                The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	void init(tgrid* grid,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const boost::function<void(twidget&)>& callback)
	{
		assert(grid);
		grid->set_parent(this);

		select_action::init(grid, data, callback);
	}
};

} // namespace gui2

#endif
