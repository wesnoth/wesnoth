/*
	Copyright (C) 2011 - 2024
	by Sytyi Nick <nsytyi@gmail.com>
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

#include "config_cache.hpp"
#include "serialization/parser.hpp"
#include "serialization/schema/type.hpp"
#include "serialization/schema/tag.hpp"
#include "serialization/validator.hpp"

#include <boost/graph/adjacency_list.hpp>
#include "utils/optional_fwd.hpp"
#include <stack>
#include <string>


/** @file
 *  One of the realizations of serialization/validator.hpp abstract validator.
 */
namespace schema_validation
{
/**
 * Realization of serialization/validator.hpp abstract validator.
 * Based on stack. Uses some stacks to store different info.
 */
class schema_validator : public abstract_validator
{
public:
	virtual ~schema_validator();

	/**
	 * Initializes validator from file.
	 * Throws abstract_validator::error if any error.
	 */
	schema_validator(const std::string& filename, bool validate_schema = false);

	void set_create_exceptions(bool value)
	{
		create_exceptions_ = value;
	}

	const std::vector<std::string>& get_errors() const
	{
		return errors_;
	}

	virtual void open_tag(const std::string& name, const config& parent, int start_line = 0, const std::string& file = "", bool addition = false) override;
	virtual void close_tag() override;
	virtual void validate(const config& cfg, const std::string& name, int start_line, const std::string& file) override;
	virtual void validate_key(const config& cfg, const std::string& name, const config_attribute_value& value, int start_line, const std::string& file) override;

private:
	// types section
	// Just some magic to ensure zero initialization.
	struct counter
	{
		int cnt;
		counter()
			: cnt(0)
		{
		}
	};

	/** Counters are mapped by tag name. */
	typedef std::map<std::string, counter> cnt_map;

	/** And counter maps are organize in stack. */
	typedef std::stack<cnt_map> cnt_stack;

protected:
	using message_type = int;
	enum { WRONG_TAG, EXTRA_TAG, MISSING_TAG, EXTRA_KEY, MISSING_KEY, WRONG_VALUE, MISSING_SUPER, SUPER_CYCLE, NEXT_ERROR };

	/**
	 * Messages are cached.
	 * The reason is next: in file where [tag]...[/tag][+tag]...[/tag]
	 * config object will be validated each [/tag]. So message can be as wrong
	 * (when [+tag] section contains missed elements) as duplicated.
	 *
	 * Messages are mapped by config*. That ensures uniqueness.
	 * Also message-maps are organized in stack to avoid memory leaks.
	 */
	struct message_info
	{
		message_type type;
		std::string file;
		int line;
		int n;
		std::string tag;
		std::string key;
		std::string value;
		std::string expected;

		message_info(message_type t, const std::string& file, int line = 0, int n = 0, const std::string& tag = "", const std::string& key = "", const std::string& value = "", const std::string& expected = "")
			: type(t)
			, file(file)
			, line(line)
			, n(n)
			, tag(tag)
			, key(key)
			, value(value)
			, expected(expected)
		{
		}
	};

	/** Controls the way to print errors. */
	bool create_exceptions_;

	virtual void print(message_info&);
private:
	void print_cache();

	typedef std::deque<message_info> message_list;
	typedef std::map<const config*, message_list> message_map;

	/** Reads config from input. */
	bool read_config_file(const std::string& filename);

	/** Shows, if validator is initialized with schema file. */
	bool config_read_;

	/** Root of schema information. */
	wml_tag root_;

	std::stack<const wml_tag*> stack_;

	/** Contains number of children. */
	cnt_stack counter_;

	/** Caches error messages. */
	std::stack<message_map> cache_;

	/** Type validators. */
	wml_type::map types_;

	bool validate_schema_;

	std::vector<std::string> errors_;

protected:
	template<typename... T>
	void queue_message(const config& cfg, T&&... args)
	{
		cache_.top()[&cfg].emplace_back(std::forward<T>(args)...);
	}

	const wml_tag& active_tag() const;
	std::string active_tag_path() const;
	bool have_active_tag() const;
	bool is_valid() const {return config_read_;}
	wml_type::ptr find_type(const std::string& type) const;

private:
	using derivation_graph_t = boost::adjacency_list<boost::vecS,
		boost::vecS,
		boost::directedS,
		std::pair<const wml_tag*, std::string>,
		std::tuple<config, std::string, int>>;

	derivation_graph_t derivation_graph_;
	std::map<const wml_tag*, derivation_graph_t::vertex_descriptor> derivation_map_;

	void detect_derivation_cycles();

	/**
	 * Collects all mandatory keys for a tag, including the super keys and overrides.
	 *
	 * The returned map can contain non-mandatory keys if they are overriden, please check the is_mandatory() result.
	 */
	utils::optional<std::map<std::string, wml_key>> find_mandatory_keys(const wml_tag* tag, const config& cfg) const;
	utils::optional<std::map<std::string, wml_key>> find_mandatory_keys(
		const wml_tag* tag, const config& cfg, std::vector<const wml_tag*>& visited) const;

	/**
	 * Validates that all mandatory keys for a tag are present.
	 */
	void validate_mandatory_keys(
		const wml_tag* tag, const config& cfg, const std::string& name, int start_line, const std::string& file);
	void validate_mandatory_keys(const std::map<std::string, wml_key>& mandatory_keys,
		const wml_tag* tag,
		const config& cfg,
		const std::string& name,
		int start_line,
		const std::string& file,
		std::vector<const wml_tag*>& visited);

	using link_graph_t = boost::adjacency_list<boost::vecS,
		boost::vecS,
		boost::directedS,
		std::string>;

	using link_graph_map_t = std::map<const wml_type_alias*,
		link_graph_t::vertex_descriptor>;

	void detect_link_cycles(const std::string& filename);
	void collect_link_source(link_graph_t& link_graph, link_graph_map_t& link_map, const std::string& type_name, const wml_type* type);
	void collect_link_target(link_graph_t& link_graph, link_graph_map_t& link_map, const std::string& type_name, const wml_type* type, const wml_type_alias* alias);
};

// A validator specifically designed for validating a schema
// In addition to the usual, it verifies that references within the schema are valid, for example via the [link] tag
class schema_self_validator : public schema_validator
{
public:
	schema_self_validator();
	virtual void open_tag(const std::string& name, const config& parent, int start_line = 0, const std::string& file = "", bool addition = false) override;
	virtual void close_tag() override;
	virtual void validate(const config& cfg, const std::string& name, int start_line, const std::string& file) override;
	virtual void validate_key(const config& cfg, const std::string& name, const config_attribute_value& value, int start_line, const std::string& file) override;
private:
	struct reference {
		reference(const std::string& value, const std::string& file, int line, const std::string& tag)
			: value_(value)
			, file_(file)
			, tag_(tag)
			, line_(line)
		{}
		std::string value_, file_, tag_;
		int line_;
		bool match(const std::set<std::string>& with) const;
		bool can_find(const wml_tag& root, const config& cfg) const;
		bool operator<(const reference& other) const;
	};
	std::string current_path() const;
	std::set<std::string> defined_types_, defined_tag_paths_;
	std::vector<reference> referenced_types_, referenced_tag_paths_;
	std::stack<std::string> tag_stack_;
	std::map<std::string, std::string> links_;
	std::multimap<std::string, std::string> derivations_;
	int type_nesting_, condition_nesting_;
	bool tag_path_exists(const config& cfg, const reference& ref);
	void check_for_duplicates(const std::string& name, std::vector<std::string>& seen, const config& cfg, message_type type, const std::string& file, int line, const std::string& tag);
	static bool name_matches(const std::string& pattern, const std::string& name);

	void print(message_info& message) override;
	enum { WRONG_TYPE = NEXT_ERROR, WRONG_PATH, DUPLICATE_TAG, DUPLICATE_KEY, SUPER_LOOP, NEXT_ERROR };

	using schema_derivation_graph_t
		= boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, std::string, std::pair<config, reference>>;

	schema_derivation_graph_t schema_derivation_graph_;
	std::map<std::string, schema_derivation_graph_t::vertex_descriptor> schema_derivation_map_;

	void detect_schema_derivation_cycles();
};

} // namespace schema_validation{
