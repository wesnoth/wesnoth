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
 * @file visitor.hpp
 */

#ifndef WB_VISITOR_HPP_
#define WB_VISITOR_HPP_

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace wb
{

class action;
class move;

class visitor : private boost::noncopyable
{
public:
	visitor();
	virtual ~visitor();

	virtual void visit_move(boost::shared_ptr<move> move) = 0;
};

}

#endif /* WB_VISITOR_HPP_ */
