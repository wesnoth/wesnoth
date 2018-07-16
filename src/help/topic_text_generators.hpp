/*
   Copyright (C) 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

class terrain_type;
class unit_type;

namespace help
{
/**
 * Abstract topic generator base class.
 */
class topic_text_generator
{
public:
	virtual ~topic_text_generator() {}

	virtual std::string generate() const = 0;
};

class plain_text_topic_generator : public topic_text_generator
{
public:
	explicit plain_text_topic_generator(const std::string& t)
		: text_(t)
	{
	}

	/** Inherited from @ref topic_text_generator. */
	virtual std::string generate() const override
	{
		return text_;
	}

private:
	std::string text_;
};

class terrain_topic_generator : public topic_text_generator
{
public:
	explicit terrain_topic_generator(const terrain_type& type)
		: type_(type)
	{
	}

	/** Inherited from @ref topic_text_generator. */
	virtual std::string generate() const override;

private:
	const terrain_type& type_;
};

class unit_topic_generator : public topic_text_generator
{
public:
	unit_topic_generator(const unit_type& t, std::string variation = "")
		: type_(t)
		, variation_(variation)
	{
		UNUSED(type_);
	}

	/** Inherited from @ref topic_text_generator. */
	virtual std::string generate() const override;

private:
	const unit_type& type_;
	const std::string variation_;

	using item = std::pair<std::string, unsigned>;
	void push_header(std::vector<item>& row, const std::string& name) const;
};

} // namespace help
