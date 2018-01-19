/*
   Copyright (C) 2011 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/iterator/iterator.hpp"

namespace gui2
{

namespace iteration
{


} // namespace iteration

} // namespace gui2


/**
 * @page gui2_iterator GUI2 Iterator.
 *
 * The iterator class allows the user to iterate over a group of widgets.
 * The idea is to add a visitor class later as well, where the two classes
 * can be combined.
 *
 * This page describes how the iterator class works. The iterator is build
 * from several parts:
 * - level, the part and subparts of the widget to visit.
 * - walker, iterates over a single widget at several levels.
 * - visit policy, whether a level should be visited or not.
 * - order policy, the order in which the several levels are traversed.
 * - iterator, the user interface for iteration.
 *
 *
 * @section gui2_iterator_level Level
 *
 * The levels are defined in @ref gui2::iteration::walker_base::level. The
 * level allows the user to only visit a part of the widget tree.
 *
 * @note At the moment when gui2::iteration::walker_base::widget is skipped the
 * child class also skips its children. This behavior might change.
 *
 *
 * @section gui2_iterator_walker Walker
 *
 * The is a group of classes inheriting from @ref gui2::iteration::walker_base
 * the objects are created from @ref gui2::widget::create_walker. The
 * walker allows to visit the several levels of the widget. This means
 * several widgets need to override the function in a subclass. For example
 * most @em simple widgets don't have a grid or children so they can use the
 * walker created from @ref gui2::styled_widget. But containers need to create a
 * different walker.
 *
 *
 * @section gui2_iterator_visit_policy Visit policy
 *
 * This policy simply defines whether or not to visit the widgets at a
 * certain level. There are two visit policies:
 * - @ref gui2::iteration::policy::visit::visit_level visits the widget at the level.
 * - @ref gui2::iteration::policy::visit::skip_level skips the widget at the level.
 *
 * There are no more visit policies expected for the future. These policies
 * are normally not used directly, but set from the @ref
 * gui2_iterator_order_policy.
 *
 *
 * @section gui2_iterator_order_policy Order policy
 *
 * This policy determines in which order the widgets are traversed, children
 * first, this level before diving down etc. @ref tests/gui/iterator.cpp
 * shows more information.
 * The following policies have been defined:
 * - @ref gui2::iteration::policy::order::top_down
 * - @ref gui2::iteration::policy::order::bottom_up
 *
 * The next sections describe in which order the widgets are visited. In the
 * description we use the following widget tree.
 *
 * [0]          @n
 *  \           @n
 *   [1|2|3|4]  @n
 *    \         @n
 *    [5|6|7|8] @n
 *
 * The types are:
 * - grid 0, 1
 * - styled_widget 2, 3, 4, 6, 7, 8
 *
 * The examples assume all levels will be visited.
 *
 *
 * @subsection gui2_iterator_visit_policy_top_down Top down
 *
 * The widgets visited first is the initial widget. After that it tries to go
 * down to a child widget and will continue down. Once that fails it will visit
 * the siblings at that level before going up again.
 *
 * @todo Write the entire visiting algorithm.
 *
 * The visiting order in our example is:
 * 0, 1, 5, 6, 7, 8, 2, 3, 4
 *
 *
 * @subsection gui2_iterator_visit_policy_bottom_up Bottom up
 *
 * When the iterator is created the iterator tries to go down all the child
 * widgets to get at the bottom level. That widget will be visited first. Then
 * it will first visit all siblings before going up the the next layer.
 *
 * @todo Write the entire visiting algorithm.
 *
 * The visiting order in our example is:
 * 5, 6, 7, 8, 1, 2, 3, 4, 0
 *
 *
 * @section gui2_iterator_iterator Iterator
 *
 * The iterator is the class the users should care about. The user creates the
 * iterator with the selected policy and the root widget. Then the user can
 * visit the widgets.
 *
 * When during the iteration a widget is added to or removed from the
 * widget-tree being walked the iterator becomes invalid. Using the iterator
 * when it is invalid results in Undefined Behavior.
 *
 * When it's certain there's at least one widget to visit a simple do while loop
 * can be used. It the policy visits the widget, it's certain there is at least
 * one widget to visit. Below some sample code:
@code
iterator<policy> itor(root);
assert(!itor.at_end());
do {
	...
	...
} while(itor.next());
@endcode
 *
 * When there might be no widget to visit a simple for loop can be used:
@code
for(iterator<policy> itor(root); !itor.at_end(); ++itor) {
	...
	...
}
@endcode
 *
 *
 * @subsection gui2_iterator_iterator_design Design
 *
 * As seen in the examples above the iterator doesn't look like a iterator in
 * the C++ standard library. The iterator is more designed after the iterator
 * design of the Gang of Four [GoF]. The reason for the different design is that
 * GoF design fits better to the classes as a normal C++ iterator. The rest of
 * this section explains some of the reasons why this design is chosen. The main
 * reason is simple; the iteration of the widgets feels better suited for the
 * GoF-style iteration as the C++-style iteration.
 *
 * The iterator is not lightweight like most C++ iterators, therefore the class
 * in non-copyable. (This is for example also the reason why a std::list has no
 * operator[].) Since operator++(int) should return a copy of the original
 * object it's also omitted.
 *
 * The design makes it hard to back-track the iteration (or costs more memory),
 * so the iterator is forward only. The order policy is added to allow the
 * wanted walking direction, but one-way only.
 *
 * The iterator has a begin, but it's not easy to go back to it and the
 * operation involves rewinding the state, which might be costly. Therefore no
 * begin() function.
 *
 * The end is known at the moment it's reached, but not upfront. That combined
 * with the forward only, makes implementing an end() hard and therefore it is
 * omitted.
 *
 *
 * [GoF] http://en.wikipedia.org/wiki/Design_Patterns_%28book%29
 */
