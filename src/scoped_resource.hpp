/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  scoped_resource: class template, functions, helper policies etc.\ for
 *  resource management.
 */

#ifndef SCOPED_RESOURCE_H_INCLUDED
#define SCOPED_RESOURCE_H_INCLUDED

#include "global.hpp"

namespace util
{
/**
* A class template, scoped_resource, designed to implement
* the Resource Acquisition Is Initialization (RAII) approach
* to resource management.
* scoped_resource is designed to be used when a resource
* is initialized at the beginning or middle of a scope,
* and released at the end of the scope.
* The template argument ReleasePolicy is a functor
* which takes an argument of the type of the resource,
* and releases it.
*
* Usage example, for working with files:
*
* @code
* struct close_file { void operator()(int fd) const {close(fd);} };
* ...
* {
*    const scoped_resource<int,close_file> file(open("file.txt",O_RDONLY));
*    read(file, buf, 1000);
* } // file is automatically closed here
* @endcode
*
* Note that scoped_resource has an explicit constructor,
* and prohibits copy-construction, and thus the initialization syntax.
* The assignment syntax must be used when initializing.
*
* I.e. using scoped_resource<int,close_file> file = open("file.txt",O_RDONLY);
* in the above example is illegal.
*
*/
template<typename T,typename ReleasePolicy>
class scoped_resource
{
	T resource;

	//prohibited operations
	scoped_resource(const scoped_resource&);
	scoped_resource& operator=(const scoped_resource&);
public:
	typedef T resource_type;
	typedef ReleasePolicy release_type;

  /**
  * Constructor
  *
  * @param res	This is the resource to be managed
  */
	scoped_resource(resource_type res = resource_type())
			: resource(res) {}

  /**
  * The destructor is the main point in this class.
  * It takes care of proper deletion of the resource,
  * using the provided release policy.
  */
	virtual ~scoped_resource()
	{
		release_type()(resource);
	}

  /**
  * This operator makes sure you can access and use the scoped_resource
  * just like you were using the resource itself.
  *
  * @return the underlying resource
  */
	operator resource_type() const { return resource; }

  /**
  * This function provides explicit access to the resource.
  * Its behavior is identical to operator resource_type()
  *
  * @return the underlying resource
  */
	resource_type get() const { return resource; }

  /**
  * This function provides convenient direct access to the -> operator
  * if the underlying resource is a pointer.
  * Only call this function if resource_type is a pointer type.
  */
	resource_type operator->() const { return resource; }

	void assign(const resource_type& o) {
		release_type()(resource);
		resource = o;
	}
};

}

#endif
