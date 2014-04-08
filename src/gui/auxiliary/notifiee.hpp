/*
   Copyright (C) 2010 - 2014 by Mark de Wever <koraq@xs4all.nl>
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
class tnotifier;

/**
 * Helper class to implement callbacks with lifetime management.
 *
 * This part manages the lifetime off the callback.
 */
template <class FUNCTOR>
class tnotifiee
{
public:
	typedef FUNCTOR tfunctor;
	friend class tnotifier<tfunctor>;

	tnotifiee() : notifier_(NULL)
	{
	}

	~tnotifiee()
	{
		if(notifier_) {
			notifier_->disconnect_notifiee(*this);
		}
	}

private:
	/** Pointer the the tnotifier that's linked to us. */
	tnotifier<tfunctor>* notifier_;
};

} // namespace gui2

#endif
