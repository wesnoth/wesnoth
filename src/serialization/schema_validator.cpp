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

#include "serialization/schema_validator.hpp"

#include "filesystem.hpp"
#include "log.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "wml_exception.hpp"
#include <tuple>

namespace schema_validation
{
static lg::log_domain log_validation("validation");

#define ERR_VL LOG_STREAM(err, log_validation)
#define WRN_VL LOG_STREAM(warn, log_validation)
#define LOG_VL LOG_STREAM(info, log_validation)

static std::string at(const std::string& file, int line)
{
	std::ostringstream ss;
	ss << line << " " << file;
	return "at " + ::lineno_string(ss.str());
}

static void print_output(const std::string& message, bool flag_exception = false)
{
#ifndef VALIDATION_ERRORS_LOG
	if(flag_exception) {
		throw wml_exception("Validation error occurred", message);
	} else {
		ERR_VL << message;
	}
#else
	// dirty hack to avoid "unused" error in case of compiling with definition on
	flag_exception = true;
	if(flag_exception) {
		ERR_VL << message;
	}
#endif
}

static std::string extra_tag_error(const std::string& file,
		int line,
		const std::string& name,
		int n,
		const std::string& parent,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Extra tag [" << name << "]; there may only be " << n << " [" << name << "] in [" << parent << "]\n"
	   << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
	return ss.str();
}

static std::string wrong_tag_error(
		const std::string& file, int line, const std::string& name, const std::string& parent, bool flag_exception)
{
	std::ostringstream ss;
	ss << "Tag [" << name << "] may not be used in [" << parent << "]\n" << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
	return ss.str();
}

static std::string missing_tag_error(const std::string& file,
		int line,
		const std::string& name,
		int n,
		const std::string& parent,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Missing tag [" << name << "]; there must be " << n << " [" << name << "]s in [" << parent << "]\n"
	   << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
	return ss.str();
}

static std::string extra_key_error(
		const std::string& file, int line, const std::string& tag, const std::string& key, bool flag_exception)
{
	std::ostringstream ss;
	ss << "Invalid key '" << key << "='";
	if(!tag.empty()) {
		ss << " in tag [" << tag << "]\n";
	}
	if(!file.empty()) {
		ss << at(file, line) << "\n";
	}
	print_output(ss.str(), flag_exception);
	return ss.str();
}

static std::string missing_key_error(
		const std::string& file, int line, const std::string& tag, const std::string& key, bool flag_exception)
{
	std::ostringstream ss;
	ss << "Missing key '" << key << "='";
	if(!tag.empty()) {
		ss << " in tag [" << tag << "]\n";
	}
	if(!file.empty()) {
		ss << at(file, line) << "\n";
	}
	print_output(ss.str(), flag_exception);
	return ss.str();
}

static std::string wrong_value_error(const std::string& file,
		int line,
		const std::string& tag,
		const std::string& key,
		const std::string& value,
		const std::string& expected,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Invalid value '";
	if(value.length() > 128)
		ss << value.substr(0, 128) << "...";
	else ss << value;
	ss << "' in key '" << key << "=' in tag [" << tag << "]\n" << " (expected value of type " << expected << ") " << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
	return ss.str();
}

static void wrong_path_error(const std::string& file,
		int line,
		const std::string& tag,
		const std::string& key,
		const std::string& value,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Unknown path reference '" << value << "' in key '" << key << "=' in tag [" << tag << "]\n" << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
}

static void duplicate_tag_error(const std::string& file,
		int line,
		const std::string& tag,
		const std::string& pat,
		const std::string& value,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Duplicate or fully-overlapping tag definition '" << value << "' (which is also matched by '" << pat << "') in tag [" << tag << "]\n" << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
}

static void duplicate_key_error(const std::string& file,
		int line,
		const std::string& tag,
		const std::string& pat,
		const std::string& value,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Duplicate or fully-overlapping key definition '" << value << "' (which is also matched by '" << pat << "') in tag [" << tag << "]\n" << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
}

static void inheritance_loop_error(const std::string& file,
		int line,
		const std::string& tag,
		const std::string& key,
		const std::string& value,
		int index,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Inheritance loop " << key << "=" << value << " found (at offset " << index << ") in tag [" << tag << "]\n" << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
}

static void wrong_type_error(const std::string & file, int line,
		const std::string & tag,
		const std::string & key,
		const std::string & type,
		bool flag_exception)
{
	std::ostringstream ss;
	ss << "Invalid type '" << type << "' in key '" << key << "=' in tag [" << tag << "]\n" << at(file, line) << "\n";
	print_output(ss.str(), flag_exception);
}

schema_validator::~schema_validator()
{
}

schema_validator::schema_validator(const std::string& config_file_name, bool validate_schema)
	: abstract_validator(config_file_name)
	, create_exceptions_(strict_validation_enabled)
	, config_read_(false)
	, validate_schema_(validate_schema)
	, errors_()
{
	if(!read_config_file(config_file_name)) {
		ERR_VL << "Schema file " << config_file_name << " was not read.";
		throw abstract_validator::error("Schema file " + config_file_name + " was not read.\n");
	} else {
		stack_.push(&root_);
		counter_.emplace();
		cache_.emplace();
		root_.expand_all(root_);
		LOG_VL << "Schema file " << config_file_name << " was read.";
		LOG_VL << "Validator initialized";
	}
}

bool schema_validator::read_config_file(const std::string& filename)
{
	config cfg;
	try {
		std::unique_ptr<abstract_validator> validator;
		if(validate_schema_) {
			validator.reset(new schema_self_validator());
		}
		preproc_map preproc(game_config::config_cache::instance().get_preproc_map());
		filesystem::scoped_istream stream = preprocess_file(filename, &preproc);
		read(cfg, *stream, validator.get());
	} catch(const config::error& e) {
		ERR_VL << "Failed to read file " << filename << ":\n" << e.what();
		return false;
	}

	for(const config& g : cfg.child_range("wml_schema")) {
		for(const config& schema : g.child_range("tag")) {
			if(schema["name"].str() == "root") {
				//@NOTE Don't know, maybe merging of roots needed.
				root_ = wml_tag(schema);
			}
		}
		types_["t_string"] = std::make_shared<wml_type_tstring>();
		for(const config& type : g.child_range("type")) {
			try {
				types_[type["name"].str()] = wml_type::from_config(type);
			} catch(const std::exception&) {
				// Need to check all type values in schema-generator
			}
		}
	}

	config_read_ = true;
	return true;
}

/*
 * Please, @Note that there is some magic in pushing and poping to/from stacks.
 * assume they all are on their place due to parser algorithm
 * and validation logic
 */
void schema_validator::open_tag(const std::string& name, const config& parent, int start_line, const std::string& file, bool addition)
{
	if(name.empty()) {
		// Opened the root tag; nothing special to do here
	} else if(!stack_.empty()) {
		const wml_tag* tag = nullptr;

		if(stack_.top()) {
			tag = active_tag().find_tag(name, root_, parent);

			if(!tag) {
				errors_.emplace_back(wrong_tag_error(file, start_line, name, stack_.top()->get_name(), create_exceptions_));
			} else {
				if(!addition) {
					counter& cnt = counter_.top()[name];
					++cnt.cnt;
					counter& total_cnt = counter_.top()[""];
					++total_cnt.cnt;
				}
			}
		}

		stack_.push(tag);
	} else {
		stack_.push(nullptr);
	}

	counter_.emplace();
	cache_.emplace();
}

void schema_validator::close_tag()
{
	stack_.pop();
	counter_.pop();
	// cache_ is normally cleared in another place.
	// However, if we're closing the root tag, clear it now
	if(stack_.empty()) {
		print_cache();
	}
}

void schema_validator::print_cache()
{
	for(auto& m : cache_.top()) {
		for(auto& list : m.second) {
			print(list);
		}
	}

	cache_.pop();
}

void schema_validator::validate(const config& cfg, const std::string& name, int start_line, const std::string& file)
{
	// close previous errors and print them to output.
	print_cache();

	// clear cache
	auto cache_it = cache_.top().find(&cfg);
	if(cache_it != cache_.top().end()) {
		cache_it->second.clear();
	}

	// Please note that validating unknown tag keys the result will be false
	// Checking all elements counters.
	if(have_active_tag() && is_valid()) {
		const wml_tag& active = active_tag();
		for(const auto& tag : active.tags(cfg)) {
			int cnt = counter_.top()[tag.first].cnt;

			if(tag.second.get_min() > cnt) {
				queue_message(cfg, MISSING_TAG, file, start_line, tag.second.get_min(), tag.first, "", name);
				continue;
			}

			if(tag.second.get_max() < cnt) {
				queue_message(cfg, EXTRA_TAG, file, start_line, tag.second.get_max(), tag.first, "", name);
			}
		}

		int total_cnt = counter_.top()[""].cnt;
		if(active.get_min_children() > total_cnt) {
			queue_message(cfg, MISSING_TAG, file, start_line, active.get_min_children(), "*", "", active.get_name());
		} else if(active_tag().get_max_children() < total_cnt) {
			queue_message(cfg, EXTRA_TAG, file, start_line, active.get_max_children(), "*", "", active.get_name());
		}

		// Checking if all mandatory keys are present
		for(const auto& key : active.keys(cfg)) {
			if(key.second.is_mandatory()) {
				if(cfg.get(key.first) == nullptr) {
					queue_message(cfg, MISSING_KEY, file, start_line, 0, name, key.first);
				}
			}
		}
	}
}

void schema_validator::validate_key(
		const config& cfg, const std::string& name, const config_attribute_value& value, int start_line, const std::string& file)
{
	if(have_active_tag() && !active_tag().get_name().empty() && is_valid()) {
		// checking existing keys
		const wml_key* key = active_tag().find_key(name, cfg);
		if(key) {
			bool matched = false;
			for(auto& possible_type : utils::split(key->get_type())) {
				if(auto type = find_type(possible_type)) {
					if(type->matches(value, types_)) {
						matched = true;
						break;
					}
				}
			}
			if(!matched) {
				queue_message(cfg, WRONG_VALUE, file, start_line, 0, active_tag().get_name(), name, value, key->get_type());
			}
		} else {
			queue_message(cfg, EXTRA_KEY, file, start_line, 0, active_tag().get_name(), name);
		}
	}
}

const wml_tag& schema_validator::active_tag() const
{
	assert(have_active_tag() && "Tried to get active tag name when there was none");
	return *stack_.top();
}

wml_type::ptr schema_validator::find_type(const std::string& type) const
{
	auto it = types_.find(type);
	if(it == types_.end()) {
		return nullptr;
	}
	return it->second;
}

bool schema_validator::have_active_tag() const
{
	return !stack_.empty() && stack_.top();
}

std::string schema_validator::active_tag_path() const {
	std::stack<const wml_tag*> temp = stack_;
	std::deque<std::string> path;
	while(!temp.empty()) {
		path.push_front(temp.top()->get_name());
		temp.pop();
	}
	if(path.front() == "root") {
		path.pop_front();
	}
	return utils::join(path, "/");
}

void schema_validator::print(message_info& el)
{
	switch(el.type) {
	case WRONG_TAG:
		errors_.emplace_back(wrong_tag_error(el.file, el.line, el.tag, el.value, create_exceptions_));
		break;
	case EXTRA_TAG:
		errors_.emplace_back(extra_tag_error(el.file, el.line, el.tag, el.n, el.value, create_exceptions_));
		break;
	case MISSING_TAG:
		errors_.emplace_back(missing_tag_error(el.file, el.line, el.tag, el.n, el.value, create_exceptions_));
		break;
	case EXTRA_KEY:
		errors_.emplace_back(extra_key_error(el.file, el.line, el.tag, el.key, create_exceptions_));
		break;
	case WRONG_VALUE:
		errors_.emplace_back(wrong_value_error(el.file, el.line, el.tag, el.key, el.value, el.expected, create_exceptions_));
		break;
	case MISSING_KEY:
		errors_.emplace_back(missing_key_error(el.file, el.line, el.tag, el.key, create_exceptions_));
		break;
	}
}

schema_self_validator::schema_self_validator()
	: schema_validator(filesystem::get_wml_location("schema/schema.cfg"), false)
	, type_nesting_()
	, condition_nesting_()
{
	defined_types_.insert("t_string");
}


void schema_self_validator::open_tag(const std::string& name, const config& parent, int start_line, const std::string& file, bool addition)
{
	schema_validator::open_tag(name, parent, start_line, file, addition);
	if(name == "type") {
		type_nesting_++;
	}
	if(condition_nesting_ == 0) {
		if(name == "if" || name == "switch") {
			condition_nesting_ = 1;
		} else if(name == "tag") {
			tag_stack_.emplace();
		}
	} else {
		condition_nesting_++;
	}
}

void schema_self_validator::close_tag()
{
	if(have_active_tag()) {
		auto tag_name = active_tag().get_name();
		if(tag_name == "type") {
			type_nesting_--;
		} else if(condition_nesting_ == 0 && tag_name == "tag") {
			tag_stack_.pop();
		}
	}
	if(condition_nesting_ > 0) {
		condition_nesting_--;
	}
	schema_validator::close_tag();
}

bool schema_self_validator::tag_path_exists(const config& cfg, const reference& ref) {
	std::vector<std::string> path = utils::split(ref.value_, '/');
	std::string suffix = path.back();
	path.pop_back();
	while(!path.empty()) {
		std::string prefix = utils::join(path, "/");
		auto link = links_.find(prefix);
		if(link != links_.end()) {
			std::string new_path = link->second + "/" + suffix;
			if(defined_tag_paths_.count(new_path) > 0) {
				return true;
			}
			path = utils::split(new_path, '/');
			suffix = path.back();
			//suffix = link->second + "/" + suffix;
		} else {
			const auto supers = derivations_.equal_range(prefix);
			if(supers.first != supers.second) {
				reference super_ref = ref;
				for(auto cur = supers.first ; cur != supers.second; ++cur) {
					super_ref.value_ = cur->second + "/" + suffix;
					if(super_ref.value_.find(ref.value_) == 0) {
						continue;
					}
					if(tag_path_exists(cfg, super_ref)) {
						return true;
					}
				}
			}
			std::string new_path = prefix + "/" + suffix;
			if(defined_tag_paths_.count(new_path) > 0) {
				return true;
			}
			suffix = path.back() + "/" + suffix;
		}
		path.pop_back();
	}
	return false;
}

bool schema_self_validator::name_matches(const std::string& pattern, const std::string& name)
{
	for(const std::string& pat : utils::split(pattern)) {
		if(utils::wildcard_string_match(name, pat)) return true;
	}
	return false;
}

void schema_self_validator::check_for_duplicates(const std::string& name, std::vector<std::string>& seen, const config& cfg, message_type type, const std::string& file, int line, const std::string& tag) {
	auto split = utils::split(name);
	for(const std::string& pattern : seen) {
		for(const std::string& key : split) {
			if(name_matches(pattern, key)) {
				queue_message(cfg, type, file, line, 0, tag, pattern, name);
				continue;
			}
		}
	}
	seen.push_back(name);
}

void schema_self_validator::validate(const config& cfg, const std::string& name, int start_line, const std::string& file)
{
	if(type_nesting_ == 1 && name == "type") {
		defined_types_.insert(cfg["name"]);
	} else if(name == "tag") {
		bool first_tag = true, first_key = true;
		std::vector<std::string> tag_names, key_names;
		for(auto current : cfg.all_children_range()) {
			if(current.key == "tag" || current.key == "link") {
				std::string tag_name = current.cfg["name"];
				if(current.key == "link") {
					tag_name.erase(0, tag_name.find_last_of('/') + 1);
				}
				if(first_tag) {
					tag_names.push_back(tag_name);
					first_tag = false;
					continue;
				}
				check_for_duplicates(tag_name, tag_names, current.cfg, DUPLICATE_TAG, file, start_line, current.key);
			} else if(current.key == "key") {
				std::string key_name = current.cfg["name"];
				if(first_key) {
					key_names.push_back(key_name);
					first_key = false;
					continue;
				}
				check_for_duplicates(key_name, key_names, current.cfg, DUPLICATE_KEY, file, start_line, current.key);
			}
		}
	} else if(name == "wml_schema") {
		using namespace std::placeholders;
		std::vector<reference> missing_types = referenced_types_, missing_tags = referenced_tag_paths_;
		// Remove all the known types
		missing_types.erase(std::remove_if(missing_types.begin(), missing_types.end(), std::bind(&reference::match, std::placeholders::_1, std::cref(defined_types_))), missing_types.end());
		// Remove all the known tags. This is more complicated since links behave similar to a symbolic link.
		// In other words, the presence of links means there may be more than one way to refer to a given tag.
		// But that's not all! It's possible to refer to a tag through a derived tag even if it's actually defined in the base tag.
		auto end = std::remove_if(missing_tags.begin(), missing_tags.end(), std::bind(&reference::match, std::placeholders::_1, std::cref(defined_tag_paths_)));
		missing_tags.erase(std::remove_if(missing_tags.begin(), end, std::bind(&schema_self_validator::tag_path_exists, this, std::ref(cfg), std::placeholders::_1)), missing_tags.end());
		std::sort(missing_types.begin(), missing_types.end());
		std::sort(missing_tags.begin(), missing_tags.end());
		static const config dummy;
		for(auto& ref : missing_types) {
			std::string tag_name;
			if(ref.tag_ == "key") {
				tag_name = "type";
			} else {
				tag_name = "link";
			}
			queue_message(dummy, WRONG_TYPE, ref.file_, ref.line_, 0, ref.tag_, tag_name, ref.value_);
		}
		for(auto& ref : missing_tags) {
			std::string tag_name;
			if(ref.tag_ == "tag") {
				tag_name = "super";
			} else if(ref.tag_ == "link") {
				tag_name = "name";
			}
			queue_message(dummy, WRONG_PATH, ref.file_, ref.line_, 0, ref.tag_, tag_name, ref.value_);
		}
	}
	schema_validator::validate(cfg, name, start_line, file);
}

void schema_self_validator::validate_key(const config& cfg, const std::string& name, const config_attribute_value& value, int start_line, const std::string& file)
{
	schema_validator::validate_key(cfg, name, value, start_line, file);
	if(have_active_tag() && !active_tag().get_name().empty() && is_valid()) {
		const std::string& tag_name = active_tag().get_name();
		if(tag_name == "key" && name == "type" ) {
			for(auto& possible_type : utils::split(cfg["type"])) {
				referenced_types_.emplace_back(possible_type, file, start_line, tag_name);
			}
		} else if((tag_name == "type" || tag_name == "element") && name == "link") {
			referenced_types_.emplace_back(cfg["link"], file, start_line, tag_name);
		} else if(tag_name == "link" && name == "name") {
			referenced_tag_paths_.emplace_back(cfg["name"], file, start_line, tag_name);
			std::string link_name = utils::split(cfg["name"].str(), '/').back();
			links_.emplace(current_path() + "/" + link_name, cfg["name"]);
		} else if(tag_name == "tag" && name == "super") {
			for(auto super : utils::split(cfg["super"])) {
				referenced_tag_paths_.emplace_back(super, file, start_line, tag_name);
				if(condition_nesting_ > 0) {
					continue;
				}
				if(current_path() == super) {
					queue_message(cfg, SUPER_LOOP, file, start_line, cfg["super"].str().find(super), tag_name, "super", super);
					continue;
				}
				derivations_.emplace(current_path(), super);
			}
		} else if(condition_nesting_ == 0 && tag_name == "tag" && name == "name") {
			tag_stack_.top() = value.str();
			defined_tag_paths_.insert(current_path());
		}
	}
}

std::string schema_self_validator::current_path() const
{
	std::stack<std::string> temp = tag_stack_;
	std::deque<std::string> path;
	while(!temp.empty()) {
		path.push_front(temp.top());
		temp.pop();
	}
	if(path.front() == "root") {
		path.pop_front();
	}
	return utils::join(path, "/");
}

bool schema_self_validator::reference::operator<(const reference& other) const
{
	return std::tie(file_, line_) < std::tie(other.file_, other.line_);
}

bool schema_self_validator::reference::match(const std::set<std::string>& with)
{
	return with.count(value_) > 0;
}

bool schema_self_validator::reference::can_find(const wml_tag& root, const config& cfg)
{
	// The problem is that the schema being validated is that of the schema!!!
	return root.find_tag(value_, root, cfg) != nullptr;
}

void schema_self_validator::print(message_info& el)
{
	schema_validator::print(el);
	switch(el.type) {
	case WRONG_TYPE:
		wrong_type_error(el.file, el.line, el.tag, el.key, el.value, create_exceptions_);
		break;
	case WRONG_PATH:
		wrong_path_error(el.file, el.line, el.tag, el.key, el.value, create_exceptions_);
		break;
	case DUPLICATE_TAG:
		duplicate_tag_error(el.file, el.line, el.tag, el.key, el.value, create_exceptions_);
		break;
	case DUPLICATE_KEY:
		duplicate_key_error(el.file, el.line, el.tag, el.key, el.value, create_exceptions_);
		break;
	case SUPER_LOOP:
			inheritance_loop_error(el.file, el.line, el.tag, el.key, el.value, el.n, create_exceptions_);
		break;
	}
}

} // namespace schema_validation{
