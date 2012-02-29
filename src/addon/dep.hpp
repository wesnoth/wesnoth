/* $Id$ */
/*
   Copyright (C) 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ADDON_DEP_HPP_INCLUDED
#define ADDON_DEP_HPP_INCLUDED

#include "version.hpp"

#include <set>

class config;

/**
 * Type for definining add-on dependencies.
 */
struct addon_dep
{
	/** Representation of valid dependency levels. */
	enum dep_type {
		requires,		/**< The add-on must be installed for dependants to work properly. */
		conflicts,		/**< The add-on must not be installed for dependants to work properly. */
		recommends		/**< The add-on is suggested to be installed for dependants to work better. */
	};

	/** Representation of valid version requirements. */
	enum version_dep_type {
		version_any,					/**< Any @a version is accepted. */
		version_equals,					/**< The version must be exactly the specified @a version. */
		version_not_equals,				/**< The version must differ from the specified @a version. */
		version_less_than,				/**< The version must be less than the specified @a version. */
		version_less_than_or_equals,	/**< The version must be less than or equal to the specified @a version. */
		version_greater_than,			/**< The version must be greater than the specified @a version. */
		version_greater_than_or_equals	/**< The version must be greater than or equal to the specified @a version. */
	};

	//
	// Fields
	//

	/** Add-on id that is depended upon. */
	std::string id;

	/** Dependency level. */
	dep_type type;

	/** List of known dependants, used in UI reports. */
	std::set<std::string> dependants;

	/** Add-on version that is depended upon. */
	version_info version;

	/** Dependency version class. */
	version_dep_type version_type;

	//
	// Methods
	//

	/** Default constructor. */
	addon_dep() :
		id(), type(), dependants(), version(), version_type()
	{}

	/** Copy constructor. */
	explicit addon_dep(const addon_dep& o);

	/**
	 * Compatibility constructor used with the old .dependencies attribute.
	 *
	 * @todo Remove this once the new syntax is finalized and the server protocol changed.
	 */
	explicit addon_dep(const std::string& dep);
	

	/**
	 * WML constructor using the @a read method.
	 *
	 * Sets attributes to reflect the specified WML node object.
	 *
	 * @param cfg      The node object with attributes to read.
	 * @param type_str The original node key, e.g. "depends", "requires".
	 */
	addon_dep(const config& cfg, const std::string& type_str);

	/** Copy operator. */
	addon_dep& operator=(const addon_dep& o);

	/**
	 * Sets attributes to reflect the specified WML node object.
	 *
	 * @param cfg      The node object with attributes to read.
	 * @param type_str The original node key, e.g. "depends", "requires".
	 */
	void read(const config& cfg, const std::string& type_str);
};

inline bool operator<(const addon_dep& a, const addon_dep& b) {
	return a.id < b.id;
}

inline bool operator==(const addon_dep& a, const addon_dep& b) {
	return a.id == b.id;
}

inline bool operator>(const addon_dep& a, const addon_dep& b) {
	return a.id > b.id;
}

inline bool operator<=(const addon_dep& a, const addon_dep& b) {
	return a.id <= b.id;
}

inline bool operator!=(const addon_dep& a, const addon_dep& b) {
	return a.id != b.id;
}

inline bool operator>=(const addon_dep& a, const addon_dep& b) {
	return a.id >= b.id;
}

#endif
