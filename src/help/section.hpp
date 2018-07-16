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

#include "help/topic.hpp"

#include <memory>
#include <string>
#include <vector>

namespace help
{
class help_manager;

/**
 * A help section.
 *
 * Sections serve as the branches of the topic tree. Each section has a set of topics,
 * and can have additional sub-sections.
 *
 * A section on its own doesn't have any text. That's the exclusive domain of topics.
 */
class section
{
public:
	using section_ptr = std::unique_ptr<section>;
	using section_list = std::vector<section_ptr>;

	/**
	 * Creates a section owned by the given manager. This is considered the toplevel node.
	 *
	 * The manager argument is mandatory; it's only a pointer since this class keeps a
	 * pointer to the manager and there's no reason to go ptr->ref->ptr.
	 */
	section(const config& section_cfg, const help_manager* manager);

	/**
	 * Creates a new sub-section owned by the given parent section.
	 */
	section(const config& section_cfg, const section& parent);

	section(const section&) = delete;
	section& operator=(const section&) = delete;

	bool operator==(const section& s) const
	{
		return id == s.id;
	}

	/**
	 * Adds a new sub-section to this section.
	 *
	 * @param new_section_cfg    The config data for the new section.
	 *
	 * @returns                  A pointer to the newly-added section.
	 */
	section* add_section(const config& new_section_cfg);

	/**
	 * Adds a new topic to this section.
	 *
	 * @tparam T                 The text generator type.
	 * @tparam Args              The values to forward to the text generator's constructor.
	 *
	 * @param id                 The id of the new section.
	 * @param title              The title of the new section.
	 *
	 * @todo Might make sense for this to return a pointer-to-topic like @ref add_section
	 * does, but we don't need that right now.
	 */
	template<typename T, typename... Args>
	void add_topic(const std::string& id, const std::string& title, Args&&... generator_args)
	{
		topics_.emplace_back(id, title, std::make_unique<T>(std::forward<Args>(generator_args)...));
	}

	/**
	 * Returns a pointer to the section with the given id, or nullptr if none is found.
	 * Sub-sections' sub-sections are also searched.
	 */
	const section* find_section(const std::string& s_id) const;

	/**
	 * Returns a pointer to the topic with the given id, or nullptr if none is found.
	 * Sub-sections' topics are also searched.
	 */
	const topic* find_topic(const std::string& t_id) const;

	/** Deletes this section's sub-sections and topics. */
	void clear()
	{
		topics_.clear();
		sections_.clear();
	}

	void sort_topics()
	{
		topics_.sort();
	}

	const topic_list& topics() const
	{
		return topics_;
	}

	const section_list& sections() const
	{
		return sections_;
	}

	std::string id;
	std::string title;

private:
	/** Constructor implementation detail. */
	void initialize(const config& section_cfg);

	void generate_sections(const std::string& generator_type);
	void generate_topics(const std::string& generator_type, const bool sort_generated);

	std::string generate_table_of_contents(const std::string& generator);

	std::string print_table_of_contents() const;

	std::string print_table_of_contents_for(const std::string& section_id) const;

	/** All topics this section owns. */
	topic_list topics_;

	/** All sub-sections. */
	section_list sections_;

	/** Tracks how many levels deep this section is in a topic tree. */
	unsigned recursion_level_;

	/** The manager that owns the toplevel node. This the same for all sections in a tree */
	const help_manager* manager_;
};

} // namespace help
