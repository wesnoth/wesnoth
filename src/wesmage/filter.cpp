/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "wesmage/filter.hpp"

#include "serialization/string_utils.hpp"
#include "wesmage/exit.hpp"

#include "utils/functional.hpp"

#include <iostream>
#include <cassert>

/** Contains the definition of a filter. */
struct filter
{
	/**
	 * The functor to call for the filter.
	 *
	 * @p surf                    The surface to apply the filter to.
	 * @p parameters              A string with the parameters for the
	 *                            functor. The code is expected to be
	 *                            supplied on the command line. So it should
	 *                            be validated.
	 */
	typedef std::function<void(
				  surface& surf
				, const std::string& parameters
			)>
			functor_t;

	filter(
			  const std::string& name__
			, const std::string& description__
			, const functor_t& functor__)
		: name(name__)
		, description(description__)
		, functor(functor__)
	{
	}

	/**
	 * The name of the filter.
	 *
	 * This value must be unique.
	 */
	std::string name;

	/**
	 * Description of the filter.
	 *
	 * The exact format of the string is documented at @ref filter_list.
	 */
	filter_description description;

	/** The functor to call for the filter. */
	functor_t functor;
};

/**
 * The list of the available filters.
 *
 * The map contains:
 * * @p first                     The name of the filter.
 * * @p second                    The filter itself.
 */
static std::map<std::string, filter> filters;

/** Helper structure to register a filter to the @ref filters. */
struct register_filter
{
	register_filter(const std::pair<std::string, filter>& filter)
	{
		filters.insert(filter);
	}
};

/**
 * Register macro for a filter.
 *
 * @param name                    The name of the filter.
 * @param description             A pipe-symbol separated list with the
 *                                description. The name is automatically
 *                                prefixed, so the item should start with a
 *                                pipe-symbol. When the list is splitted in
 *                                to a vector of string its contents should
 *                                be compatible with the constructor of
 *                                @ref filter_description.
 */
#define REGISTER(name, description)                                           \
	register_filter register_filter_##name(std::make_pair(                   \
			  #name                                                           \
			, filter(#name, #name description, std::bind(name, _1, _2))));

static void
scale(surface& surf, const std::string& parameters)
{
	unsigned width, height;
	const int count = sscanf(parameters.c_str(), "%u,%u", &width, &height);

	if(count != 2) {
		std::cerr << "Error: Arguments to scale »"
				<< parameters
				<< "« are not compatible.\n";
		throw exiter(EXIT_FAILURE);
	}

	surf = scale_surface(surf, width, height);
}
REGISTER(scale,
"|Scales the size of an image."
"|new_width"
	"|unsigned"
	"|The width in pixel of the image after scaling."
"|new_height"
	"|unsigned"
	"|The height in pixel of the image after scaling.")

static void
brighten(surface& surf, const std::string& parameters)
{
	float amount;
	const int count = sscanf(parameters.c_str(), "%f", &amount);

	if(count != 1) {
		std::cerr << "Error: Arguments to brighten »"
				<< parameters
				<< "« are not compatible.\n";

		throw exiter(EXIT_FAILURE);
	}

	surf = brighten_image(surf, amount);
}
REGISTER(brighten,
"|Brightens an image."
"|amount"
	"|float"
	"|The amount the image should be brightened. The value of the every "
		"color channel is multiplied by this value. Value less than zero "
		"are set to zero. The alpha channel is not modified.")

static void
blend(surface& surf, const std::string& parameters)
{
	float amount;
	unsigned color;
	const int count = sscanf(parameters.c_str(), "%f,%x", &amount, &color);

	if(count != 2) {
		std::cerr << "Error: Arguments to blend »"
				<< parameters
				<< "« are not compatible.\n";

		throw exiter(EXIT_FAILURE);
	}

	surf = blend_surface(surf, amount, color);
}
REGISTER(blend,
"|Blends an image with another color."
"|amount"
	"|float"
	"|The amount every pixel needs to be blended with its original value. "
	    "The formula is:\n"
		"result = amount * color + (1 - amount) * original\n"
		"The value needs to be in the range [0, 1]."
"|color"
	"|unsigned"
	"|The color to blend with. The value should be given as 32-bit "
		"hexadecimal value. The first fields should look like AARRGGBB, "
		"where AA is the alpha channel, RR is the red channel, GG is the "
		"green channel and BB is the blue channel. (Note the alpha channel "
		"is ignored.")

void
filter_apply(surface& surf, const std::string& filter_string)
{
	std::vector<std::string> f = utils::split(filter_string, ':', utils::STRIP_SPACES);

	if(f.size() != 2) {
		std::cerr << "Error: Filter »"
				<< filter_string
				<< "« doesn't contain the expected separator »:«\n";

		throw exiter(EXIT_FAILURE);
	}

	std::map<std::string, filter>::iterator itor = filters.find(f[0]);

	if(itor == filters.end()) {
		std::cerr << "Error: Filter »" << f[0] << "« is unknown.\n";
		throw exiter(EXIT_FAILURE);
	}

	itor->second.functor(surf, f[1]);
}

filter_description::filter_description(const std::string& fmt)
	: name()
	, description()
	, parameters()
{
	std::vector<std::string> elements(utils::split(fmt, '|'));

	/* Validate there is at least a header part. */
	assert(elements.size() >= 2);

	name = elements[0];
	description = elements[1];

	/* Validate every parameter indeed has three fields. */
	assert((elements.size() - 2) % 3 == 0);

	for(size_t i = 2; i < elements.size(); i += 3) {

		filter_description::parameter parameter =
		{
			  elements[i]
			, elements[i + 1]
			, elements[i + 2]
		};

		parameters.push_back(parameter);
	}
}

std::vector<filter_description>
filter_list()
{
	std::vector<filter_description> result;
	for(const auto& filter : filters) {
		result.push_back(filter.second.description);
	}
	return result;
}
