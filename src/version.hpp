/*
   Copyright (C) 2008 - 2018 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <string>
#include <vector>

/**
 * @file
 * Interfaces for manipulating version numbers of engine,
 * add-ons, etc.
 */

/**
 * Represents version numbers.
 *
 * Versions are expected to be in the format <tt>x1.x2.x3[.x4[.x5[...]]]</tt>,
 * with an optional trailing special version suffix and suffix separator.
 *
 * When parsing a version string, the first three components are optional
 * and default to zero if absent. The serialized form will always have all
 * first three components, making deserialization and serialization an
 * asymmetric process in those cases (e.g. "0.1" becomes "0.1.0").
 *
 * The optional trailing suffix starts after the last digit, and may be
 * preceded by a non-alphanumeric separator character (e.g. "0.1a" has "a" as
 * its suffix and the null character as its separator, but in "0.1+dev" the
 * separator is '+' and the suffix is "dev"). Both are preserved during
 * serialization ("0.1+dev" becomes "0.1.0+dev").
 */
class version_info
{
public:
	version_info();                    /**< Default constructor. */
	version_info(const std::string&);  /**< String constructor. */

	/** Simple list constructor. */
	version_info(unsigned int major, unsigned int minor, unsigned int revision_level,
	             char special_separator='\0', const std::string& special=std::string());

	/**
	 * Whether the version number is considered canonical for mainline Wesnoth.
	 *
	 * Mainline Wesnoth version numbers have at most three components, so this
	 * check is equivalent to <tt>components() <= 3</tt>.
	 */
	bool is_canonical() const;

	/**
	 * Serializes the version number into string form.
	 *
	 * The result is in the format <tt>x1.x2.x3[.x4[.x5[...]]]</tt>, followed
	 * by the special version suffix separator (if not null) and the suffix
	 * itself (if not empty).
	 */
	std::string str() const;

	/**
	 * Syntactic shortcut for str().
	 */
	operator std::string() const { return this->str(); }

	// Good old setters and getters for this class. Their names should be
	// pretty self-descriptive. I couldn't use shorter names such as
	// major() or minor() because sys/sysmacros.h reserves them by defining
	// some backwards-compatibility macros for stuff, and they cause
	// conflicts in the C/C++ preprocessor on GNU/Linux (GCC).

	/**
	 * Retrieves the major version number (@a x1 in "x1.x2.x3").
	 */
	unsigned int major_version() const;

	/**
	 * Retrieves the minor version number (@a x2 in "x1.x2.x3").
	 */
	unsigned int minor_version() const;

	/**
	 * Retrieves the revision level (@a x3 in "x1.x2.x3").
	 */
	unsigned int revision_level() const;

	/**
	 * Retrieves the special version separator (e.g. '+' in "0.1+dev").
	 *
	 * The special version separator is the first non-alphanumerical character
	 * preceding the special version suffix and following the last numeric
	 * component. If missing, the null character is returned instead.
	 */
	char special_version_separator() const
	{
		return this->special_separator_;
	}

	/**
	 * Retrieves the special version suffix (e.g. "dev" in "0.1+dev").
	 */
	const std::string& special_version() const
	{
		return this->special_;
	}

	/**
	 * Sets the major version number.
	 */
	void set_major_version(unsigned int);

	/**
	 * Sets the minor version number.
	 */
	void set_minor_version(unsigned int);

	/**
	 * Sets the revision level.
	 */
	void set_revision_level(unsigned int);

	/**
	 * Sets the special version suffix.
	 */
	void set_special_version(const std::string& str)
	{
		this->special_ = str;
	}

	/**
	 * Returns any numeric component from a version number.
	 *
	 * The index may be in the [0,3) range, yielding the same results as
	 * major_version(), minor_version(), and revision_level().
	 *
	 * @throw std::out_of_range If the number of components is less than
	 *                          <tt>index - 1</tt>.
	 */
	unsigned int get_component(size_t index) const
	{
		return nums_.at(index);
	}

	/**
	 * Sets any numeric component from a version number.
	 *
	 * The index may be in the [0,3) range, resulting in the same effect as
	 * set_major_version(), set_minor_version(), and set_revision_level().
	 *
	 * @throw std::out_of_range If the number of components is less than
	 *                          <tt>index - 1</tt>.
	 */
	void set_component(size_t index, unsigned int value)
	{
		nums_.at(index) = value;
	}

	/**
	 * Read-only access to all numeric components.
	 */
	const std::vector<unsigned int>& components() const
	{
		return this->nums_;
	}

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
