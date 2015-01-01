/*
   Copyright (C) 2013 - 2015 by L.Sebilleau <l.sebilleau@free.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/*
        Yet Another Map Generator. Class Hexheap

    This class is an utility to sort dynamically hexes.
    Uses key member as sorting key (ascending).
*/

#ifndef YAMG_HEXHEAP_HPP
#define YAMG_HEXHEAP_HPP

#include "yamg_hex.hpp"
#include <stddef.h>


class yamg_hexheap
{
public:
	yamg_hexheap(size_t taille);
	virtual ~yamg_hexheap();

	//*********** Members ************
	int last_;                  ///< an index on last record
	int max_;                   ///< memory limit index

	//*********** Methods ************
    void add_hex(yamg_hex *h);   ///< add an hex to the heap
    yamg_hex *pick_hex();        ///< get and remove first hex
    int test_hex();              ///< get key value of first hex
    void update_hexes(int val);  ///< update all key values
    void clear_heap();           ///< clear the heap and reset all items 'done' flag in it.

protected:
	//*********** Members ************
	yamg_hex **table_;          ///< holds the pointers table

private:
};

#endif // YAMG_HEXHEAP_HPP
