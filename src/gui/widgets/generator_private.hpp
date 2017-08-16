/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/generator.hpp"

#include <cassert>
#include "gui/widgets/grid.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp" // For widget::visibility
#include "wml_exception.hpp"

namespace gui2
{

/**
 * Contains the policies for the generator class.
 */
namespace policy
{

/***** ***** ***** ***** Minimum selection ***** ***** ***** *****/

/** Contains the policy for the minimum number of selected items. */
namespace minimum_selection
{

/** Must select at least one item. */
struct one_item : public virtual generator_base
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
	using generator_base::create_item;

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
struct no_item : public virtual generator_base
{

	/** See @ref minimum_selection::one_item::set_item_shown(). */
	void set_item_shown(const unsigned index, const bool show);

	/** See @ref minimum_selection::one_item::create_item() */
	void create_item(const unsigned /*index*/)
	{
	}

	/* Also make the overload from the generator_ visible. */
	using generator_base::create_item;

	/** See @ref minimum_selection::one_item::deselect_item() */
	bool deselect_item(const unsigned index)
	{
		do_deselect_item(index);
		return true;
	}

	/** See @ref minimum_selection::one_item::delete_item() */
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
struct one_item : public virtual generator_base
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
	void select_item(const unsigned index, const bool select) override
	{
		assert(select);

		if(get_selected_item_count() == 1) {
			// deselect current.
			do_deselect_item(get_selected_item());
		}
		// select new.
		do_select_item(index);
	}
};

/** No maximum amount of items to select. */
struct many_items : public virtual generator_base
{
	/** See one_item::select_item(). */
	void select_item(const unsigned index, const bool select) override
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
struct horizontal_list : public virtual generator_base
{
	horizontal_list();

	/**
	 * Called when an item is created.
	 *
	 * This function should place the new item.
	 *
	 * @param index                  The index of the new item.
	 */
	void create_item(const unsigned index);

	/* Also make the overload from the generator_ visible. */
	using generator_base::create_item;

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned /*maximum_width*/) override
	{
		/* DO NOTHING */
	}

	/** See @ref widget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned /*maximum_height*/) override
	{
		/* DO NOTHING */
	}

	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& origin) override;

	/**
	 * Sets the visible rectangle of the generator.
	 *
	 * @param rectangle           The visible rectangle.
	 */
	void set_visible_rectangle(const SDL_Rect& rectangle) override;

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from generator_base. */
	void handle_key_up_arrow(SDL_Keymod /*modifier*/, bool& /*handled*/) override
	{
		/* do nothing */
	}

	/** Inherited from generator_base. */
	void handle_key_down_arrow(SDL_Keymod /*modifier*/, bool& /*handled*/) override
	{
		/* do nothing */
	}

	/** Inherited from generator_base. */
	void handle_key_left_arrow(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from generator_base. */
	void handle_key_right_arrow(SDL_Keymod modifier, bool& handled) override;

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
struct vertical_list : public virtual generator_base
{
	vertical_list();

	/** See horizontal_list::create_item(). */
	void create_item(const unsigned index);

	/* Also make the overload from the generator_ visible. */
	using generator_base::create_item;

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned /*maximum_width*/) override
	{
		/* DO NOTHING */
	}

	/** See @ref widget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned /*maximum_height*/) override
	{
		/* DO NOTHING */
	}

	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& origin) override;

	/** See @ref horizontal_list::set_visible_rectangle(). */
	void set_visible_rectangle(const SDL_Rect& rectangle) override;

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from generator_base. */
	void handle_key_up_arrow(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from generator_base. */
	void handle_key_down_arrow(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from generator_base. */
	void handle_key_left_arrow(SDL_Keymod /*modifier*/, bool& /*handled*/) override
	{ /* do nothing */
	}

	/** Inherited from generator_base. */
	void handle_key_right_arrow(SDL_Keymod /*modifier*/, bool& /*handled*/) override
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
struct table : public virtual generator_base
{
	table();

	/** See horizontal_list::create_item(). */
	void create_item(const unsigned /*index*/);

	/* Also make the overload from the generator_ visible. */
	using generator_base::create_item;

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned /*maximum_width*/) override
	{
		/* DO NOTHING */
	}

	/** See @ref widget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned /*maximum_height*/) override
	{
		/* DO NOTHING */
	}

	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

	/** See @ref widget::place. */
	virtual void place(const point& /*origin*/
					   , const point& /*size*/) override;

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& /*origin*/) override;

	/** See @ref horizontal_list::set_visible_rectangle(). */
	void set_visible_rectangle(const SDL_Rect& /*rectangle*/) override;

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& /*coordinate*/
							 , const bool /*must_be_active*/) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& /*coordinate*/
								   , const bool /*must_be_active*/) const override;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from generator_base. */
	void handle_key_up_arrow(SDL_Keymod, bool&) override;

	/** Inherited from generator_base. */
	void handle_key_down_arrow(SDL_Keymod, bool&) override;

	/** Inherited from generator_base. */
	void handle_key_left_arrow(SDL_Keymod, bool&) override;

	/** Inherited from generator_base. */
	void handle_key_right_arrow(SDL_Keymod, bool&) override;
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
 * Places the items independent of each other.
 *
 * This is mainly meant for when only one item is shown at the same time.
 *
 * @todo Implement.
 */
struct independent : public virtual generator_base
{
	/** See horizontal_list::create_item(). */
	void create_item(const unsigned /*index*/)
	{
		/* DO NOTHING */
	}

	/* Also make the overload from the generator_ visible. */
	using generator_base::create_item;

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/** See horizontal_list::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) override;

	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& origin) override;

	/** See @ref horizontal_list::set_visible_rectangle(). */
	void set_visible_rectangle(const SDL_Rect& rectangle) override;

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/** See @ref widget::find. */
	widget* find(const std::string& id, const bool must_be_active) override;

	/** See @ref widget::find. */
	const widget* find(const std::string& id,
						const bool must_be_active) const override;

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from generator_base. */
	void handle_key_up_arrow(SDL_Keymod, bool&) override
	{
		/* DO NOTHING */
	}

	/** Inherited from generator_base. */
	void handle_key_down_arrow(SDL_Keymod, bool&) override
	{
		/* DO NOTHING */
	}

	/** Inherited from generator_base. */
	void handle_key_left_arrow(SDL_Keymod, bool&) override
	{
		/* DO NOTHING */
	}

	/** Inherited from generator_base. */
	void handle_key_right_arrow(SDL_Keymod, bool&) override
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

/** Select the item, this requires the grid to contain a selectable_item. */
struct selection : public virtual generator_base
{
	void select(grid& grid, const bool select);

	/**
	 * Helper function to initialize a grid.
	 *
	 * @param grid                The grid to initialize.
	 * @param data                The data to initialize the parameters of
	 *                            the new item.
	 * @param callback            The callback function to call when an item
	 *                            in the grid is (de)selected.
	 */
	void init(grid* grid,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const std::function<void(widget&)>& callback);
};

/** Show the item. */
struct show : public virtual generator_base
{
	void select(grid& grid, const bool show)
	{
		grid.set_visible(show ? widget::visibility::visible
							  : widget::visibility::hidden);
	}

	/**
	 * Helper function to initialize a grid.
	 *
	 * @param grid                The grid to initialize.
	 * @param data                The data to initialize the parameters of
	 *                            the new item. No widgets with id == "" are
	 *                            allowed.
	 * @param callback            The callback function is not used and
	 *                            should be nullptr.
	 */
	void init(grid* grid,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const std::function<void(widget&)>& callback);
};

} // namespace select_action

} // namespace policy

/***** ***** ***** ***** generator ***** ***** ***** *****/

/**
 * Basic template class to generate new items.
 *
 * The class is policy based so the behavior can be selected.
 */
template <class minimum_selection,
		  class maximum_selection,
		  class my_placement,
		  class select_action>
class generator : public minimum_selection,
				   public maximum_selection,
				   public my_placement,
				   public select_action
{
public:
	generator()
		: minimum_selection()
		, maximum_selection()
		, my_placement()
		, select_action()
		, selected_item_count_(0)
		, last_selected_item_(-1)
		, items_()
		, order_()
		, order_dirty_(true)
		, order_func_()
	{
	}

	~generator()
	{
		clear();
	}

	/***** ***** ***** inherited ***** ****** *****/

	/** Inherited from generator_base. */
	void delete_item(const unsigned index) override
	{
		assert(index < items_.size());

		// Might be other parts of the engine want to know about the
		// deselection, if minimum fails it gets another chance later on,
		// since it deletes the item.
		if(is_selected(index)) {
			select_item(index, false);
		}

		minimum_selection::delete_item(index);

		items_.erase(items_.begin() + index);
		order_dirty_ = true;
	}

	/** Inherited from generator_base. */
	void clear() override
	{
		items_.clear();
		order_dirty_ = true;
		selected_item_count_ = 0;
	}

	/** Inherited from generator_base. */
	void select_item(const unsigned index, const bool select = true) override
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

	/** Inherited from generator_base. */
	bool is_selected(const unsigned index) const override
	{
		assert(index < items_.size());
		return (*items_[index]).selected;
	}

	/** Inherited from generator_base. */
	void set_item_shown(const unsigned index, const bool show) override
	{
		assert(index < items_.size());
		if(items_[index]->shown != show) {

			/*** Set the proper visible state. ***/
			items_[index]->shown = show;
			items_[index]
					->child_grid.set_visible(show ? widget::visibility::visible
											: widget::visibility::invisible);

			/*** Update the selection. ***/
			minimum_selection::set_item_shown(index, show);
		}
	}

	/** Inherited from generator_base. */
	virtual bool get_item_shown(const unsigned index) const override
	{
		assert(index < items_.size());
		return items_[index]->shown && items_[index]->child_grid.get_visible() != widget::visibility::invisible;
	}


	/** Inherited from generator_base. */
	unsigned get_item_count() const override
	{
		return items_.size();
	}

	/** Inherited from generator_base. */
	unsigned get_selected_item_count() const override
	{
		return selected_item_count_;
	}

	/** Inherited from generator_base. */
	int get_selected_item() const override
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
			FAIL_WITH_DEV_MESSAGE("No item selected.",
				"selected_item_count_ was non-zero, yet no selected item was found.");
		}
	}

	/** Inherited from generator_base. */
	grid& item(const unsigned index) override
	{
		assert(index < items_.size());
		return items_[index]->child_grid;
	}

	/** Inherited from generator_base. */
	const grid& item(const unsigned index) const override
	{
		assert(index < items_.size());
		return items_[index]->child_grid;
	}

	/** Inherited from generator_base. */
	grid& item_ordered(const unsigned index) override
	{
		calculate_order();
		assert(index < items_.size());
		return items_[order_[index]]->child_grid;
	}

	/** Inherited from generator_base. */
	const grid& item_ordered(const unsigned index) const override
	{
		calculate_order();
		assert(index < items_.size());
		return items_[order_[index]]->child_grid;
	}


	/** Inherited from generator_base. */
	grid& create_item(const int index,
					   builder_grid_const_ptr list_builder,
					   const string_map& item_data,
					   const std::function<void(widget&)>& callback) override
	{
		std::map<std::string, string_map> data;

		data.emplace("", item_data);
		return create_item(index, list_builder, data, callback);
	}

	/** Inherited from generator_base. */
	grid& create_item(
			const int index,
			builder_grid_const_ptr list_builder,
			const std::map<std::string /* widget id */, string_map>& item_data,
			const std::function<void(widget&)>& callback) override
	{
		assert(list_builder);
		assert(index == -1 || static_cast<unsigned>(index) <= items_.size());

		child* item = new child;
		list_builder->build(&item->child_grid);
		init(&item->child_grid, item_data, callback);

		const unsigned item_index = index == -1 ? items_.size() : index;

		items_.emplace(items_.begin() + item_index, item);
		order_dirty_ = true;
		minimum_selection::create_item(item_index);
		my_placement::create_item(item_index);
		if(!is_selected(item_index)) {
			select_action::select(item->child_grid, false);
		}
		return item->child_grid;
	}

	/** Inherited from generator_base. */
	virtual void create_items(
			const int index,
			builder_grid_const_ptr list_builder,
			const std::vector<std::map<std::string /*widget id*/, string_map> >&
					data,
			const std::function<void(widget&)>& callback) override
	{
		impl_create_items(index, list_builder, data, callback);
	}

	/** Inherited from generator_base. */
	virtual void create_items(const int index,
							  builder_grid_const_ptr list_builder,
							  const std::vector<string_map>& data,
							  const std::function<void(widget&)>& callback) override
	{
		impl_create_items(index, list_builder, data, callback);
	}

	/** See @ref widget::layout_initialize. */
	virtual void layout_initialize(const bool full_initialization) override
	{
		for(auto& item : items_)
		{
			if(item->child_grid.get_visible() != widget::visibility::invisible
			   && item->shown) {

				item->child_grid.layout_initialize(full_initialization);
			}
		}
	}

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override
	{
		my_placement::request_reduce_width(maximum_width);
	}

	/** See @ref widget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) override
	{
		my_placement::request_reduce_height(maximum_height);
	}

	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override
	{
		return my_placement::calculate_best_size();
	}

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override
	{
		// Inherited, so we get useful debug info.
		widget::place(origin, size);

		my_placement::place(origin, size);
	}

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& origin) override
	{
		// Inherited.
		widget::set_origin(origin);

		my_placement::set_origin(origin);
	}

	/** See @ref widget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) override
	{
		my_placement::set_visible_rectangle(rectangle);
	}

	/** See @ref widget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) override
	{
		assert(this->get_visible() == widget::visibility::visible);
		calculate_order();
		for(auto index : order_)
		{
			child* item = items_[index].get();
			if(item->child_grid.get_visible() == widget::visibility::visible
			   && item->shown) {

				item->child_grid.draw_children(frame_buffer, x_offset, y_offset);
			}
		}
	}

	/** See @ref widget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(window& caller,
							  const std::vector<widget*>& call_stack) override
	{
		for(auto& item : items_)
		{
			std::vector<widget*> child_call_stack = call_stack;
			item->child_grid.populate_dirty_list(caller, child_call_stack);
		}
	}

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override
	{
		return my_placement::find_at(coordinate, must_be_active);
	}

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override
	{
		return my_placement::find_at(coordinate, must_be_active);
	}

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override
	{
		for(auto& item : items_)
		{
			if(item->child_grid.disable_click_dismiss()) {
				return true;
			}
		}
		return false;
	}

	/**
	 * See @ref widget::create_walker.
	 *
	 * @todo Implement properly.
	 */
	virtual iteration::walker_base* create_walker() override
	{
		return nullptr;
	}

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/** Inherited from generator_base. */
	void handle_key_up_arrow(SDL_Keymod modifier, bool& handled) override
	{
		my_placement::handle_key_up_arrow(modifier, handled);
	}

	/** Inherited from generator_base. */
	void handle_key_down_arrow(SDL_Keymod modifier, bool& handled) override
	{
		my_placement::handle_key_down_arrow(modifier, handled);
	}

	/** Inherited from generator_base. */
	void handle_key_left_arrow(SDL_Keymod modifier, bool& handled) override
	{
		my_placement::handle_key_left_arrow(modifier, handled);
	}

	/** Inherited from generator_base. */
	void handle_key_right_arrow(SDL_Keymod modifier, bool& handled) override
	{
		my_placement::handle_key_right_arrow(modifier, handled);
	}

protected:
	/** Inherited from generator_base. */
	void do_select_item(const unsigned index) override
	{
		assert(index < items_.size());

		++selected_item_count_;
		set_item_selected(index, true);
	}

	/** Inherited from generator_base. */
	void do_deselect_item(const unsigned index) override
	{
		assert(index < items_.size());

		--selected_item_count_;
		set_item_selected(index, false);
	}

private:
	/** Definition of an item. */
	struct child
	{

		child() : child_grid(), selected(false), shown(true), ordered_index(0)
		{
		}

		/** The grid containing the widgets. */
		grid child_grid;

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

		size_t ordered_index;
	};

	/** The number of selected items. */
	unsigned selected_item_count_;

	/** The last item selected. */
	int last_selected_item_;

	/** The items in the generator. */
	typedef std::vector<std::unique_ptr<child>> child_list;
	child_list items_;

	/** the elements of order_ are indexes to items_ */
	mutable std::vector<size_t> order_;
	/** whether need to recalculate order_dirty_ */
	mutable bool order_dirty_;

	typedef std::function<bool (unsigned, unsigned)> torder_func;
	torder_func order_func_;


	virtual void set_order(const torder_func& order) override
	{
		order_func_ = order;
		order_dirty_ = true;
		this->set_is_dirty(true);
	}

	struct calculate_order_helper
	{
		const torder_func& order_func_;
		const child_list& items_;

		calculate_order_helper(const torder_func& order_func, const child_list& items)
			: order_func_(order_func)
			, items_(items)
		{
		}

		bool operator()(size_t a, size_t b)
		{
			return order_func_(a, b);
		}
	};

	virtual unsigned get_ordered_index(unsigned index) const override
	{
		assert(index < items_.size());
		calculate_order();
		return items_[index]->ordered_index;
	}

	virtual unsigned get_item_at_ordered(unsigned index_ordered) const override
	{
		assert(index_ordered < items_.size());
		calculate_order();
		return order_[index_ordered];
	}

	void calculate_order() const
	{
		if(order_dirty_) {
			if(order_.size() != items_.size()) {
				order_.resize(items_.size());
				for(size_t i = 0; i < items_.size(); ++i) {
					order_[i] = i;
				}
			}
			if(order_func_) {
				std::stable_sort(order_.begin(), order_.end(), calculate_order_helper(order_func_, items_));
			}
			for(size_t i = 0; i < order_.size(); ++i) {
				items_[order_[i]]->ordered_index = i;
			}

			order_dirty_ = false;
		}
		else {
			assert(order_.size() == items_.size());
		}
	}
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
		select_action::select((*items_[index]).child_grid, selected);
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
						   builder_grid_const_ptr list_builder,
						   const std::vector<T>& data,
						   const std::function<void(widget&)>& callback)
	{
		int i = index;
		for(const auto & item_data : data)
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
	void init(grid* grid,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const std::function<void(widget&)>& callback)
	{
		assert(grid);
		grid->set_parent(this);

		select_action::init(grid, data, callback);
	}
};

} // namespace gui2
