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

    virtual void set_path(const std::list<map_location> path);

    void set_color(const SDL_Color color);

    void set_layer(const display::tdrawing_layer & layer);

    /** Notifies it's arrow_observer list of the arrow's destruction */
    virtual ~arrow() { notify_arrow_deleted(); }

    /**
     * If you want your arrow to be automatically registered and displayed, create
     * it through display::createNewArrow(). Only use this constructor directly
     * if you have a good reason to do so.
     */
    arrow();

    void add_observer(arrow_observer & observer);

    void remove_observer(arrow_observer & observer);

    std::list<arrow_image> get_images() const;

    const std::list<map_location> & get_path() const;

    const std::list<map_location> & get_previous_path() const;


  private: //operations

    void notify_arrow_changed();

    void notify_arrow_deleted();

  private: //properties

    display::tdrawing_layer layer_;

    SDL_Color color_;

    std::list<arrow_observer*> observers_;

    std::list<map_location> path_;
    std::list<map_location> previous_path_;

};
#endif
