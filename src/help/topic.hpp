/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "help/topic_text_generators.hpp"

#include <list>
#include <memory>
#include <string>

namespace help
{

/** A topic contains a title, an id and some text. */
struct topic
{
	using text_gen_ptr_t = std::unique_ptr<topic_text_generator>;

	topic(const std::string& t_id, const std::string& t_title)
		: id(t_id)
		, title(t_title)
		, text_generator_(nullptr)
		, parsed_text_()
	{
	}

	topic(const std::string& t_id, const std::string& t_title, const std::string& t_text)
		: id(t_id)
		, title(t_title)
		, text_generator_(std::make_unique<plain_text_topic_generator>(t_text))
		, parsed_text_()
	{
	}

	topic(const std::string& t_id, const std::string& t_title, text_gen_ptr_t g)
		: id(t_id)
		, title(t_title)
		, text_generator_(std::move(g))
		, parsed_text_()
	{
	}

	bool operator==(const topic& t) const
	{
		return id == t.id;
	}

	bool operator!=(const topic& t) const
	{
		return !operator==(t);
	}

	/** Case-sensitive, locale-dependent sort by title. */
	bool operator<(const topic& t) const;

	std::string id;
	std::string title;

	const config& parsed_text() const;

private:
	mutable std::unique_ptr<topic_text_generator> text_generator_;
	mutable config parsed_text_;
};

using topic_list = std::list<topic>;

} // namespace help
