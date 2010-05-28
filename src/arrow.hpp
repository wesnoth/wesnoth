/* $Id$ */
/*
   Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file arrow.hpp
 * Arrows destined to be drawn on the map. Created for the whiteboard project.
 */

#ifndef _ARROW_H
#define _ARROW_H

#include "display.hpp"

#include <vector>
#include <list>
#include <utility>

class arrow_observer;

typedef std::pair<map_location, surface> arrow_image;

/**
 * Arrows destined to be drawn on the map. Created for the whiteboard system.
 */
class arrow {

  public: //operations

    arrow();

    virtual ~arrow() {}

    virtual void set_path(const std::list<map_location> path);

    void set_color(const SDL_Color color);

    void set_layer(const display::tdrawing_layer & layer);

    std::list<arrow_image> get_images() const;

    const std::list<map_location> & get_path() const;

    const std::list<map_location> & get_previous_path() const;

  private: //properties

    display::tdrawing_layer layer_;

    SDL_Color color_;

    std::list<map_location> path_;
    std::list<map_location> previous_path_;

};
#endif
