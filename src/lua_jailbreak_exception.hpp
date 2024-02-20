/*
	Copyright (C) 2011 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

/**
 * Base class for exceptions that want to be thrown 'through' lua.
 *
 * Classes derived from this class <b>must call `this->store()` in all of their
 * constructors</b> and use the @ref IMPLEMENT_LUA_JAILBREAK_EXCEPTION macro
 * in the class definition. No other classes may derive from classes derived
 * from this class.
 */
class lua_jailbreak_exception
{
public:
	virtual ~lua_jailbreak_exception() noexcept {}

	/** Depth of recursive luaW_pcall_internal() function calls. */
	static int jail_depth;

	/** Stores a copy the current exception to be rethrown. */
	void store() const noexcept;

	/**
	 * Rethrows the stored exception.
	 *
	 * It is safe to call this function is no exception is stored.
	 */
	static void rethrow();

protected:

	/** The exception to be rethrown. */
	static lua_jailbreak_exception* jailbreak_exception;

private:

	/** Clears the current exception. */
	static void clear() noexcept;

	/**
	 * Creates a copy of the current exception.
	 *
	 * The copy is allocated with @p new and is implemented by the @ref
	 * IMPLEMENT_LUA_JAILBREAK_EXCEPTION macro.
	 *
	 * @note it's implemented by the subclass to avoid slicing.
	 *
	 * @returns                   A pointer to a copy of the class on the heap.
	 */
	virtual lua_jailbreak_exception* clone() const = 0;

	/**
	 * Executes the exception.
	 *
	 * Throws a copy of the stored @ref jailbreak_exception. The caller is
	 * responsible for clearing @ref jailbreak_exception after the throw.
	 * The function is implemented by the @ref
	 * IMPLEMENT_LUA_JAILBREAK_EXCEPTION macro.
	 *
	 * @note it's implemented by the subclass to avoid slicing.
	 *
	 * @pre                       jailbreak_exception != nullptr
	 */
	virtual void execute() = 0;
};

/**
 * Helper macro for classes deriving from @ref lua_jailbreak_exception.
 *
 * @ref lua_jailbreak_exception has several pure virtual functions, this
 * macro implements them properly. This macro needs to be placed in the
 * definition of the most derived class, which uses @ref
 * lua_jailbreak_exception as baseclass.
 *
 * @param type                    The type of the class whc
 */
#define IMPLEMENT_LUA_JAILBREAK_EXCEPTION(type)                      \
	                                                                 \
	virtual type* clone() const final { return new type(*this); }    \
	                                                                 \
	virtual void execute() final                                     \
	{                                                                \
		type exception(dynamic_cast<type&>(*jailbreak_exception));   \
		throw exception;                                             \
	}
