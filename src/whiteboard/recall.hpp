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
 * @file
 */

#ifndef WB_RECALL_HPP_
#define WB_RECALL_HPP_

#include "action.hpp"

#include "typedefs.hpp"

#include "map_location.hpp"

namespace wb
{

class recall: public wb::action
{
public:
	recall(size_t team_index, const unit& unit, const map_location& recall_hex);
	virtual ~recall();

private:
	boost::scoped_ptr<unit> unit_;
	map_location recall_hex_;
};

}

#endif /* WB_RECALL_HPP_ */
