/*
	Copyright (C) 2020 - 2025
	by CrawlCycle <73139676+CrawlCycle@users.noreply.github.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "wml_equivalence.hpp"
#include "serialization/parser.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>

namespace bfs = boost::filesystem;
/**
 * Self destructive temporary file at the current directory.
 * @pre An other user does not have a file with the same name.
 */
class tmp_file final
{
public:
	/** Set file to @p content and close the file */
	void set(const std::string& content)
	{
		// If any step raises an exception, tmp_file will go out of scope
		// and trigger the destructor.
		const auto mode = std::ios_base::out;
		stream.open(path, std::ios_base::trunc | mode);
		auto permission = (bfs::perms::owner_read | bfs::perms::owner_write);
		bfs::permissions(path, permission);
		stream << content;
		stream.close();
	}

	~tmp_file()
	{
		if(bfs::exists(path)) {
			bfs::remove(path);
		}
	}

	/** Path to the temporary file */
	const bfs::path path = bfs::unique_path("%%%%-%%%%-%%%%-%%%%-%%%%-%%%%.tmp.out");

private:
	bfs::ofstream stream;
};

config preprocess_and_parse(const std::string& wml_str, preproc_map* macro_map)
{
	tmp_file tmp_f;
	tmp_f.set(wml_str);
	auto b = preprocess_file(tmp_f.path.string(), macro_map);
	return io::read(*b);
}

void check_wml_equivalence(const std::string& a, const std::string& b)
{
	config config_a = preprocess_and_parse(a);
	config config_b = preprocess_and_parse(b);
	BOOST_CHECK_EQUAL(config_a, config_b);
}
