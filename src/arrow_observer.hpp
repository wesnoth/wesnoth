/* $Id$ */
/*
   Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file arrow_observer.hpp
 */

#ifndef ARROW_OBSERVER_HPP_
#define ARROW_OBSERVER_HPP_

/**
 * Interface implemented by any code interested of tracking an arrow's
 * changes (currently, only display implements it).
 */
class arrow_observer {
  public:

    virtual ~arrow_observer() {}

    virtual void arrow_changed(const arrow & a) = 0;

    virtual void arrow_deleted(const arrow & a) = 0;

};

#endif /* ARROW_OBSERVER_HPP_ */
