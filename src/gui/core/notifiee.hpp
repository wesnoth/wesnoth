/*
   Copyright (C) 2010 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_NOTIFIEE_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_NOTIFIEE_HPP_INCLUDED

#include <cstdlib>

namespace gui2
{

template <class T>
class notifier;

/**
 * Helper class to implement callbacks with lifetime management.
 *
 * This part manages the lifetime off the callback.
 */
template <class FUNCTOR>
class notifiee
{
public:
	typedef FUNCTOR functor_t;
	friend class notifier<functor_t>;

	notifiee() : notifier_(nullptr)
	{
	}

	~notifiee()
	{
		if(notifier_) {
			notifier_->disconnect_notifiee(*this);
		}
	}

private:
	/** Pointer the the notifier that's linked to us. */
	notifier<functor_t>* notifier_;
};

} // namespace gui2

#endif
