/*
   Copyright (C) 2008 - 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef VERSION_HPP_INCLUDED
#define VERSION_HPP_INCLUDED

#include <string>
#include <vector>

/**
 * @file
 * Interfaces for manipulating version numbers of engine,
 * add-ons, etc.
 *
 * This class assumes versions are in the format "X.Y.Z", with
 * additional components not being associated to canonical
 * names.
 */
class version_info
{
public:
	version_info();                    /**< Default constructor. */
	version_info(const std::string&);  /**< String constructor. */

	/** Simple list constructor. */
	version_info(unsigned int major, unsigned int minor, unsigned int revision_level,
	             char special_separator='\0', const std::string& special=std::string());

	bool is_canonical() const;

	// Good old setters and getters for this class. Their names should be
	// pretty self-descriptive. I couldn't use shorter names such as
	// major() or minor() because sys/sysmacros.h reserves them by defining
	// some backwards-compatibility macros for stuff, and they cause
	// conflicts in the C/C++ preprocessor on GNU/Linux (GCC).

	// Canonical version components.

	unsigned int major_version() const;  /**< Retrieves the major version number (X in "X.Y.Z"). */
	unsigned int minor_version() const;  /**< Retrieves the minor version number (Y in "X.Y.Z"). */
	unsigned int revision_level() const; /**< Retrieves the revision level (Z in "X.Y.Z"). */

	/**
	 * It is sometimes useful so append special build/distribution
	 * information to version numbers, in the form of "X.Y.Z+dev",
	 * "X.Y.Za", etcetera. This member function retrieves such if
	 * available.
	 */
	const std::string& special_version() const {
		return this->special_;
	}

	/**
	 * Retrieves the special version separator. For the "X.Y.Z+blah"
	 * string, it would be '+'. On the other hand, it would be a null
	 * (ASCII 00) character if the string was "X.Y.Za".
	 */
	char special_version_separator() const {
		return this->special_separator_;
	}

	void set_major_version(unsigned int);  /**< Set major version number. */
	void set_minor_version(unsigned int);  /**< Set minor version number. */
	void set_revision_level(unsigned int); /**< Set revision level. */

	/** Set special version string. */
	void set_special_version(const std::string& str) {
		this->special_ = str;
	}

	// Non-canonical version strings components.

	/**
	 * Returns a component from a non-canonically formatted
	 * version string (i.e. 'D' from A.B.C.D is index 3).
	 * The index may be in the [0,3) range; in such case, this
	 * function works identically to the canonical version
	 * numbers extractors.
	 *
	 * @note If the number of components is smaller than index-1,
	 *       a std::out_of_range exception may be thrown.
	 */
	unsigned int get_component(size_t index) const {
		return nums_.at(index);
	}

	/**
	 * Sets a component in a non-canonically formatted
	 * version string (i.e. 'D' from A.B.C.D is index 3).
	 * The index may be in the [0,3) range; in such case, this
	 * function works identically to the canonical version
	 * numbers setters.
	 *
	 * @note If the number of components is smaller than index-1,
	 *       new ones are added and initialized as 0 to make room.
	 */
	void set_component(size_t index, unsigned int value) {
		nums_.at(index) = value;
	}

	/** Read-only access to complete vector of components. */
	const std::vector<unsigned int>& components() const {
		return this->nums_;
	}

	std::string str() const; /**<
							   * Returns a formatted string of the form
							   * A.B.C[.x1[.x2[...]]]kS, where xN stand for
							   * non-canonical version components, k for the
							   * suffix separator, and S for the suffix string.
							   */

	/** Syntactic shortcut for str(). */
	operator std::string() const { return this->str(); }

private:
	std::vector<unsigned int> nums_;
	std::string               special_;
	char                      special_separator_;
};

/** Equality operator for version_info. */
bool operator==(const version_info&, const version_info&);
/** Inequality operator for version_info. */
bool operator!=(const version_info&, const version_info&);
/** Greater-than operator for version_info. */
bool operator>(const version_info&, const version_info&);
/** Less-than operator for version_info. */
bool operator<(const version_info&, const version_info&);
/** Greater-than-or-equal operator for version_info. */
bool operator>=(const version_info&, const version_info&);
/** Less-than-or-equal operator for version_info. */
bool operator<=(const version_info&, const version_info&);

enum VERSION_COMP_OP {
	OP_INVALID,
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_LESS,
	OP_LESS_OR_EQUAL,
	OP_GREATER,
	OP_GREATER_OR_EQUAL
};

VERSION_COMP_OP parse_version_op(const std::string& op_str);
bool do_version_check(const version_info& a, VERSION_COMP_OP op, const version_info& b);

#endif /* !VERSION_HPP_INCLUDED */
