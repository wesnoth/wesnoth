/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "config.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "log.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "wesconfig.h"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"

#include <sstream>
#include <stack>

#define ERR_CF lg::err(lg::config)
#define WRN_CF lg::warn(lg::config)
#define LOG_CF lg::info(lg::config)

static const int max_recursion_levels = 100;

line_source get_line_source(std::vector< line_source > const &line_src, int line)
{
	line_source res(line, "", 0);
	std::vector< line_source >::const_iterator it =
		std::upper_bound(line_src.begin(), line_src.end(), res);
	if (it != line_src.begin()) {
		--it;
		res.file = it->file;
		res.fileline = it->fileline + (line - it->linenum);
	}

	return res;
}

void read(config &cfg, std::string const &data, std::vector< line_source > const *line_sources)
{
	cfg.clear();

	std::stack< std::string > element_names;
	std::stack< int > element_locs;
	std::stack< config * > elements;
	std::stack< std::map< std::string, config * > > last_element; //allows [+element] syntax
	std::stack< std::string > textdomains;
	std::string current_textdomain = PACKAGE;
	std::string current_textdomain_location = "";
	elements.push(&cfg);
	element_names.push("");
	element_locs.push(0);
	last_element.push(std::map< std::string, config * >());

	enum { ELEMENT_NAME, IN_ELEMENT, VARIABLE_NAME, VALUE }
	state = IN_ELEMENT;
	std::string var;

	std::vector< std::pair< std::string, bool > > stored_values;
	std::string value;

	bool in_quotes = false, has_quotes = false, in_comment = false, escape_next = false,
	     translatable = false, expecting_value = false;

	int line = 0;

	for(std::string::const_iterator i = data.begin(), i_end = data.end(); i != i_end; ++i) {
		const char c = *i;
		if (c == '\r') //ignore any DOS-style newlines
			continue;

		if (c == '\n') {
			in_comment = false;
			++line;
		}

		if (*i == '#' && !in_quotes) {
			in_comment = true;
		}

		if (in_comment) {
			continue;
		}

		switch(state) {
			case ELEMENT_NAME:
				if(c == ']') {
					if(value == "end" || value.empty() == false && value[0] == '/') {
						wassert(!elements.empty());

						if(value[0] == '/' &&
						   std::string("/" + element_names.top()) != value) {
							std::stringstream err;

							if(line_sources != NULL) {
								const line_source src = get_line_source(*line_sources,line);

								err << src.file << " " << src.fileline << ": ";
							} else {
								err << "line " << line << ": ";
							}

							err << "Found illegal end tag: '" << value
							    << "', at end of '"
							    << element_names.top() << "'";

							throw config::error(err.str());
						}

						const std::string name = element_names.top();
						config* const element = elements.top();

						elements.pop();
						element_names.pop();
						element_locs.pop();
						last_element.pop();

						if(elements.empty()) {
							std::stringstream err;

							if(line_sources != NULL) {
								const line_source src =
								        get_line_source(*line_sources,line);

								err << src.file << " " << src.fileline << ": ";
							}

							err << "Unexpected terminating tag\n";
							throw config::error(err.str());
							return;
						}

						last_element.top()[name] = element;

						if(element->values.count("textdomain") != 0){
							current_textdomain = textdomains.top();
							textdomains.pop();
						}
						current_textdomain_location = "";

						state = IN_ELEMENT;

						break;
					}

					//any elements with a + sign prefix, like [+element] mean
					//that they are appending to the previous element with the same
					//name, if there is one
					if(value.empty() == false && value[0] == '+') {
						value.erase(value.begin(),value.begin()+1);
						const std::map<std::string,config*>::iterator itor = last_element.top().find(value);
						if(itor != last_element.top().end()) {
							elements.push(itor->second);
							element_names.push(value);
							element_locs.push(line);
							last_element.push(std::map<std::string,config*>());
							state = IN_ELEMENT;
							value = "";
							break;
						}
					}

					elements.push(&elements.top()->add_child(value));
					element_names.push(value);
					element_locs.push(line);
					last_element.push(std::map<std::string,config*>());

					state = IN_ELEMENT;
					value = "";
				} else {
					value.resize(value.size()+1);
					value[value.size()-1] = c;
				}

				break;

			case IN_ELEMENT:
				if(c == '[') {
					state = ELEMENT_NAME;
					value = "";
				} else if (!utils::portable_isspace(c)) {
					value.resize(1);
					value[0] = c;
					state = VARIABLE_NAME;
				}

				break;

			case VARIABLE_NAME:
				if(c == '=') {
					state = VALUE;
					var = value;
					value = "";
				} else {
					value.resize(value.size()+1);
					value[value.size()-1] = c;
				}

				break;

			case VALUE:
				if(c == '[' && in_quotes) {
					if(line_sources != NULL) {
						const line_source src = get_line_source(*line_sources,line);
						LOG_CF << src.file << " " << src.fileline << ": ";
					} else {
						LOG_CF << "line " << line << ": ";
					}

					WRN_CF << "square bracket found in string. Is this a run-away string?\n";
				}
				
				if(in_quotes && c == '"' && (i+1) != data.end() && *(i+1) == '"') {
					push_back(value, c);
					++i; // skip the next double-quote
				} else if(c == '"') {
					expecting_value = false;
					in_quotes = !in_quotes;
					has_quotes = true;

					//if we have an underscore outside of quotes in front, then
					//we strip it away, since it simply indicates that this value is translatable.
					if(value.empty() == false && std::count(value.begin(),value.end(),'_') == 1) {
						std::string val = value;
						if (utils::strip(val) == "_") {
							value = "";
							translatable = true;
						}
					}
				} else if(c == '+' && has_quotes && !in_quotes) {
					stored_values.push_back(std::make_pair(value,translatable));
					value = "";
					translatable = false;
					expecting_value = true;
				} else if(c == '\n' && !in_quotes && expecting_value) {
					//do nothing...just ignore
				} else if(c == '\n' && !in_quotes) {

					stored_values.push_back(std::make_pair(value,translatable));
					value = "";
					for(std::vector<std::pair<std::string,bool> >::const_iterator i = stored_values.begin(); i != stored_values.end(); ++i) {
						if(i->second) {
							value += dsgettext(current_textdomain.c_str(),i->first.c_str());
						} else {
							value += i->first;
						}
					}

					stored_values.clear();

					//see if this is a CSV list=CSV list style assignment (e.g. x,y=5,8)
					std::vector<std::string> vars, values;
					if(std::count(var.begin(),var.end(),',') > 0) {
						vars = utils::split(var);
						values = utils::split(value);
					} else {
						vars.push_back(var);
						values.push_back(value);
						if (var == "textdomain") {
							textdomains.push(current_textdomain);
							current_textdomain = value;
							bindtextdomain(current_textdomain.c_str(),
								       current_textdomain_location.empty() ?
								       get_intl_dir().c_str() :
								       current_textdomain_location.c_str());
							bind_textdomain_codeset (current_textdomain.c_str(), "UTF-8");
						} else if (var == "translations") {
							const std::string& location = get_binary_file_location(value, ".");
							current_textdomain_location = location;
						}
					}

					//iterate over the names and values, assigning each to its corresponding
					//element. If there are more names than values, than remaining names get
					//assigned to the last value. If there are more values than names, then
					//all the last values get concatenated onto the last name
					if(vars.empty() == false) {
						for(size_t n = 0; n != maximum<size_t>(vars.size(),values.size()); ++n) {
							std::string value;
							if(n < values.size()) {
								value = values[n];
							} else if(values.empty() == false) {
								value = values.back();
							}

							if(has_quotes == false) {
								utils::strip(value);
							}

							if(n < vars.size()) {
								elements.top()->values[vars[n]] = value;
							} else {
								elements.top()->values[vars.back()] += "," + value;
							}
						}
					}

					state = IN_ELEMENT;
					var = "";
					value = "";
					has_quotes = false;
					escape_next = false;
					translatable = false;
				} else if(in_quotes || !has_quotes) {
					expecting_value = false;
					push_back(value, c);
				} else if(expecting_value) {
					// after a +, emulate !has_quotes so we can see any _ when we encounter a " later
					push_back(value, c);
				}

				break;
		}
	}

	const std::string top = element_names.top();
	element_names.pop();
	if(!element_names.empty()) {
		throw config::error("Configuration not terminated: no closing tag to '" + top + "' (line " + str_cast(element_locs.top()) + ")");
	}
}

namespace {
	const std::string AttributeEquals = "=\"";
	const std::string AttributePostfix = "\"\n";
	const std::string ElementPrefix = "[";
	const std::string ElementPostfix = "]\n";
	const std::string EndElementPrefix = "[/";
	const std::string EndElementPostfix = "]\n";
	const std::string ConfigPostfix = "\n";
}

static std::string escaped_string(const std::string& value) {
	std::vector<char> res;
	for(std::string::const_iterator i = value.begin(); i != value.end(); ++i) {
		//double interior quotes
		if(*i == '\"') res.push_back(*i);
		res.push_back(*i);
	}
	return std::string(res.begin(), res.end());
}

static size_t write_size(config const &cfg, size_t tab = 0)
{
	size_t res = 0;
	for(string_map::const_iterator i = cfg.values.begin(), i_end = cfg.values.end(); i != i_end; ++i) {
		if(i->second.empty() == false) {
			res += i->first.size() + AttributeEquals.size() +
			       escaped_string(i->second).size() + AttributePostfix.size() + tab;
		}
	}

	for(config::all_children_iterator j = cfg.ordered_begin(), j_end = cfg.ordered_end(); j != j_end; ++j) {
		const std::pair<const std::string*,const config*>& item = *j;
		const std::string& name = *item.first;
		const config& cfg = *item.second;
		res += ElementPrefix.size() + name.size() + ElementPostfix.size() +
		       write_size(cfg, tab + 1) + EndElementPrefix.size() + name.size() + EndElementPostfix.size() + tab * 2;
		
	}

	res += ConfigPostfix.size();

	return res;
}

static std::string::iterator write_internal(config const &cfg, std::string::iterator out, size_t tab = 0)
{
	if(tab > max_recursion_levels)
		return out;

	for(string_map::const_iterator i = cfg.values.begin(), i_end = cfg.values.end(); i != i_end; ++i) {
		if(i->second.empty() == false) {
			std::fill(out,out+tab,'\t');
			out += tab;

			out = std::copy(i->first.begin(),i->first.end(),out);
			out = std::copy(AttributeEquals.begin(),AttributeEquals.end(),out);
			std::string value = escaped_string(i->second);
			out = std::copy(value.begin(),value.end(),out);
			out = std::copy(AttributePostfix.begin(),AttributePostfix.end(),out);
		}
	}

	for(config::all_children_iterator j = cfg.ordered_begin(), j_end = cfg.ordered_end(); j != j_end; ++j) {
		const std::pair<const std::string*,const config*>& item = *j;
		const std::string& name = *item.first;
		const config& cfg = *item.second;

		std::fill(out,out+tab,'\t');
		out += tab;

		out = std::copy(ElementPrefix.begin(),ElementPrefix.end(),out);
		out = std::copy(name.begin(),name.end(),out);
		out = std::copy(ElementPostfix.begin(),ElementPostfix.end(),out);
		out = write_internal(cfg, out, tab + 1);

		std::fill(out,out+tab,'\t');
		out += tab;

		out = std::copy(EndElementPrefix.begin(),EndElementPrefix.end(),out);
		out = std::copy(name.begin(),name.end(),out);
		out = std::copy(EndElementPostfix.begin(),EndElementPostfix.end(),out);
	}

	out = std::copy(ConfigPostfix.begin(),ConfigPostfix.end(),out);
	return out;
}

std::string write(config const &cfg)
{
	std::string res;

	res.resize(write_size(cfg));

	const std::string::iterator i = write_internal(cfg, res.begin());
	wassert(i == res.end());
	if(i != res.end()) {
		ERR_CF << "size of config buffer: " << (i - res.begin()) << "/" << res.size() << "\n";
	}

	return res;
}
