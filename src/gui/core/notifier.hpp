/*
   Copyright (C) 2010 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/notifiee.hpp"

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
class notifier
{
public:
	typedef FUNCTOR functor_t;

	notifier() : notifiees_()
	{
	}

	~notifier()
	{
		for (auto & item : notifiees_)
		{
			assert(item.first);
			assert((*item.first).notifier_ == this);

			(*item.first).notifier_ = nullptr;
		}
	}

	/**
	 * Connects a callback.
	 *
	 * @param target                 The notifiee controlling the lifetime of
	 *                               the callback.
	 * @param functor                The callback to call.
	 */
	void connect_notifiee(notifiee<functor_t>& target, functor_t functor)
	{
		notifiees_.emplace(&target, functor);

		assert(!target.notifier_);

		target.notifier_ = this;
	}

	/**
	 * Disconnects a callback.
	 *
	 * @param target                 The notifiee controlling the lifetime of
	 *                               the callback. Uses since its address is an
	 *                               unique key.
	 */
	void disconnect_notifiee(notifiee<functor_t>& target)
	{
		typename std::map<notifiee<functor_t>*, functor_t>::iterator itor
				= notifiees_.find(&target);

		if(itor != notifiees_.end()) {

			assert(target.notifier_ == this);

			target.notifier_ = nullptr;

			notifiees_.erase(itor);
		}
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	const std::map<notifiee<functor_t>*, functor_t>& notifiees() const
	{
		return notifiees_;
	}

private:
	/** List of registered callbacks. */
	std::map<notifiee<functor_t>*, functor_t> notifiees_;
};

} // namespace gui2
