/*
   Copyright (C) 2016 - 2018 by Marius Spix
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

#include "utils/name_generator.hpp"
#include "config.hpp"
#include <vector>
#include <memory>

class name_generator_factory
{
public:
	/**
	 * Creates a new name generator factory
	 * @param config the WML data to be parsed for name generators
	 * @param ids a list of generator ids, e.g. genders or terrain types
	 */
	name_generator_factory(const config& config, std::vector<std::string> ids);

	/**
	 * Gets the default name generator
	 * @returns the default name generator
	 */
	std::shared_ptr<name_generator> get_name_generator();

	/**
	 * Gets a specific name generator or the default name generator, if the
	 * specific name generator is not found.
	 * @param name generator id, e.g. a gender or a terrain type
	 * @returns a name generator
	 */
	std::shared_ptr<name_generator> get_name_generator(const std::string name);

private:
	std::map<std::string, std::shared_ptr<name_generator>> name_generators_;

	/**
	 * Determines a name generator from WML data. Tries first to load a context-free generator,
	 * then falls back to Markov.
	 * @param config the WML data to be parsed for name generators
	 * @param prefix the prefix to look for
	 */
	void add_name_generator_from_config(const config& config, const std::string id, const std::string prefix);
};
