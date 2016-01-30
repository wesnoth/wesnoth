/*
   Copyright (C) 2010 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_NOTIFIER_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_NOTIFIER_HPP_INCLUDED

#include "gui/auxiliary/notifiee.hpp"
#include "utils/foreach.tpp"

#include <cassert>
#include <map>

namespace gui2
{

/**
 * Helper class to implement callbacks with lifetime management.
 *
 * This part manages the connecting and disconnecting of the callbacks.
 *
 * Subclasses should implement a way to call all callback.
 */
template <class FUNCTOR>
class tnotifier
{
public:
	typedef FUNCTOR tfunctor;

	tnotifier() : notifiees_()
	{
	}

	~tnotifier()
	{
		FOREACH(AUTO & item, notifiees_)
		{
			assert(item.first);
			assert((*item.first).notifier_ == this);

			(*item.first).notifier_ = NULL;
		}
	}

	/**
	 * Connects a callback.
	 *
	 * @param notifiee               The notifiee controlling the lifetime of
	 *                               the callback.
	 * @param functor                The callback to call.
	 */
	void connect_notifiee(tnotifiee<tfunctor>& notifiee, tfunctor functor)
	{
		notifiees_.insert(std::make_pair(&notifiee, functor));

		assert(!notifiee.notifier_);

		notifiee.notifier_ = this;
	}

	/**
	 * Disconnects a callback.
	 *
	 * @param notifiee               The notifiee controlling the lifetime of
	 *                               the callback. Uses since its address is an
	 *                               unique key.
	 */
	void disconnect_notifiee(tnotifiee<tfunctor>& notifiee)
	{
		typename std::map<tnotifiee<tfunctor>*, tfunctor>::iterator itor
				= notifiees_.find(&notifiee);

		if(itor != notifiees_.end()) {

			assert(notifiee.notifier_ == this);

			notifiee.notifier_ = NULL;

			notifiees_.erase(itor);
		}
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	const std::map<tnotifiee<tfunctor>*, tfunctor>& notifiees() const
	{
		return notifiees_;
	}

private:
	/** List of registered callbacks. */
	std::map<tnotifiee<tfunctor>*, tfunctor> notifiees_;
};

} // namespace gui2

#endif
