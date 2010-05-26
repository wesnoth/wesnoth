#ifndef _ARROW_H
#define _ARROW_H

#include "display.hpp"

#include <vector>
#include <utility>

/**
 * Interface implemented by any code interested of tracking an arrow's
 * changes (currently, only display implements it).
 */
class arrow_observer {
  public:
    virtual void arrow_changed(const arrow & a) = 0;

    virtual void arrow_deleted(const arrow & a) = 0;

};

/**
 * Arrows destined to be drawn on the map. Created for the whiteboard system.
 */
class arrow {

  public:
    void set_path(const std::list<map_location> & path);

    void set_color(const SDL_Color & color);

    void set_layer(const display::tdrawing_layer & layer);

    /** Notifies it's arrow_observer list of the arrow's destruction */
    ~arrow() { notify_arrow_deleted(); }

    /**
     * If you want your arrow to be automatically registered and displayed, create
     * it through display::createNewArrow(). Only use this constructor directly
     * if you have a good reason to do so.
     */
    arrow();

    void add_observer(const arrow_observer & observer);

    void remove_observer(const arrow_observer & observer);

    std::vector<std::pair<map_location, surface> > getImages();

  private:

    void notify_arrow_changed();

    void notify_arrow_deleted();

  private:
    display::tdrawing_layer layer;

    SDL_Color color;

    std::list<arrow_observer*> observers;

    std::list<map_location> path;

};
#endif
