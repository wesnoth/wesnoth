#ifndef _ARROW_H
#define _ARROW_H

#include "display.hpp"

#include <vector>
#include <list>
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

typedef std::pair<map_location, surface> arrow_image;

/**
 * Arrows destined to be drawn on the map. Created for the whiteboard system.
 */
class arrow {

  public: //operations

    void set_path(const std::list<map_location> path);

    void set_color(const SDL_Color color);

    void set_layer(const display::tdrawing_layer & layer);

    /** Notifies it's arrow_observer list of the arrow's destruction */
    ~arrow() { notify_arrow_deleted(); }

    /**
     * If you want your arrow to be automatically registered and displayed, create
     * it through display::createNewArrow(). Only use this constructor directly
     * if you have a good reason to do so.
     */
    arrow();

    void add_observer(arrow_observer & observer);

    void remove_observer(arrow_observer & observer);

    std::vector<arrow_image> getImages() const;

  private: //operations

    void notify_arrow_changed();

    void notify_arrow_deleted();

  private: //properties

    display::tdrawing_layer layer_;

    SDL_Color color_;

    std::list<arrow_observer*> observers_;

    std::list<map_location> path_;

};
#endif
