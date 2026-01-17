/*
	Copyright (C) 2003 - 2025
	by Jörg Hinrichs, David White <dave@whitevine.net>
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

#include "config.hpp"

#include <chrono>

namespace savegame
{
class save_index_class;

/** Filename and modification date for a file list */
class save_info
{
private:
	friend class create_save_info;

	save_info(const std::string& name,
		const std::shared_ptr<save_index_class>& index,
		const std::chrono::system_clock::time_point& modified)
		: name_(name)
		, save_index_(index)
		, modified_(modified)
	{
	}

public:
	const std::string& name() const
	{
		return name_;
	}

	const auto& modified() const
	{
		return modified_;
	}

	std::string format_time_summary() const;
	std::string format_time_local() const;
	const config& summary() const;

private:
	std::string name_;
	std::shared_ptr<save_index_class> save_index_;
	std::chrono::system_clock::time_point modified_;
};

/**
 * A structure for comparing to save_info objects based on their modified time.
 * If the times are equal, will order based on the name.
 */
struct save_info_less_time
{
	bool operator()(const save_info& a, const save_info& b) const;
};

/** Read the complete config information out of a savefile. */
config read_save_file(const std::string& dir, const std::string& name);

class create_save_info
{
public:
	explicit create_save_info(const std::shared_ptr<save_index_class>&);
	save_info operator()(const std::string& filename) const;
	std::shared_ptr<save_index_class> manager_;
};

class save_index_class : public std::enable_shared_from_this<save_index_class>
{
public:
	/**
	 * Constructor for a read-only instance. To get a writable instance, call default_saves_dir().
	 */
	explicit save_index_class(const std::string& dir);
	/** Syntatic sugar for choosing which constructor to use. */
	enum class create_for_default_saves_dir { yes };
	explicit save_index_class(create_for_default_saves_dir);

	/** Returns an instance for managing saves in filesystem::get_saves_dir() */
	static std::shared_ptr<save_index_class> default_saves_dir();

	std::vector<save_info> get_saves_list(const std::string* filter=nullptr);

	/** Delete a savegame, including deleting the underlying file. */
	void delete_game(const std::string& name);

	void rebuild(const std::string& name);
	void rebuild(const std::string& name, const std::chrono::system_clock::time_point& modified);

	/** Delete a savegame from the index, without deleting the underlying file. */
	void remove(const std::string& name);
	void set_modified(const std::string& name, const std::chrono::system_clock::time_point& modified);

	config& get(const std::string& name);
	const std::string& dir() const;

	/** Delete autosaves that are no longer needed (according to the autosave policy in the preferences). */
	void delete_old_auto_saves(const int autosavemax, const int infinite_auto_saves);

	/** Sync to disk, no-op if read_only_ is set */
	void write_save_index();

	/**
	 * If true, all of delete_game, delete_old_auto_saves and write_save_index will be no-ops.
	 */
	bool read_only()
	{
		return read_only_;
	}

private:
	config& data(const std::string& name);
	config& data();

	static void fix_leader_image_path(config& data);
	/** Deletes non-existent save files from the index. */
	void clean_up_index();

	config data_;
	std::map<std::string, std::chrono::system_clock::time_point> modified_;
	const std::string dir_;
	/**
	 * The instance for default_saves_dir() writes a cache file. For other instances,
	 * write_save_index() and delete() are no-ops.
	 */
	bool read_only_;
	/** Flag to only run the clean_up_index method once. */
	bool clean_up_index_;
};
} // end of namespace savegame
