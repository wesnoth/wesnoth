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

#include "global.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "filesystem.hpp"
#include "log.hpp"
#include "wesconfig.h"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"

#define ERR_CF LOG_STREAM(err, config)
#define LOG_CF LOG_STREAM(info, config)

bool preproc_define::operator==(preproc_define const &v) const {
	return value == v.value && arguments == v.arguments;
}

// FIXME
struct config {
	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};
};

static int const max_recursion_levels = 100;

//this function takes a macro and parses it into the macro followed by its
//arguments. Arguments are seperated by spaces, but an argument appearing inside
//braces is treated as a single argument.
static std::vector<std::string> parse_macro_arguments(const std::string& macro)
{
	const std::vector<std::string> args = utils::split(macro, ' ');
	std::vector<std::string> res;
	if(args.empty()) {
		res.push_back("");
		return res;
	}

	res.push_back(args.front());

	bool in_braces = false;
	for(std::vector<std::string>::const_iterator i = args.begin()+1; i != args.end(); ++i) {
		size_t begin = 0, end = i->size();
		if((*i)[0] == '(') {
			++begin;
		}

		if(!in_braces) {
			res.push_back("");
		}

		if((*i)[i->size()-1] == ')') {
			in_braces = false;
			--end;
		}

		res.back() += " " + i->substr(begin,end-begin);
		utils::strip(res.back());

		if(begin == 1 && end == i->size()) {
			in_braces = true;
		}
	}

	return res;
}

static void internal_preprocess_file(const std::string& fname,
                                     preproc_map& defines_map,
                                     int depth, std::ostream &out,
                                     std::string const &included_from,
                                     bool previous_was_newline);

static void internal_preprocess_data(std::istream &data_in,
                                     preproc_map& defines_map,
                                     int depth, std::ostream &out,
                                     std::string const &included_from,
                                     bool need_linenum,
                                     std::string const &current_directory)
{
	std::string data_str;
	{
		//temporary, only here to accomodate the old preprocessor
		std::stringstream tmp_in;
		tmp_in << data_in.rdbuf();
		data_str = tmp_in.str();
	}
	std::string const &data = data_str;

	bool in_quotes = false;
	int current_line = 1;
	bool previous_was_newline = !need_linenum;

	for(std::string::const_iterator i = data.begin(); i != data.end(); ++i) {
		const char c = *i;
		if(c == '"') {
			in_quotes = !in_quotes;
		}

		if(c == '{') {
			int bracket_depth = 1;
			std::stringstream newfile;
			for(++i; i != data.end(); ++i) {
				if(*i == '{') {
					bracket_depth++;
				} else if(*i == '}') {
					bracket_depth--;
					if(bracket_depth == 0) {
						break;
					}
				}

				newfile << *i;
			}

			if(i == data.end())
				break;

			const std::string newfilename = newfile.str();
			std::vector<std::string> items = parse_macro_arguments(newfilename);
			const std::string symbol = items.front();

			//if this is a known pre-processing symbol, then we insert
			//it, otherwise we assume it's a file name to load
			if(defines_map.count(symbol) != 0) {
				items.erase(items.begin());

				const preproc_define& val = defines_map[symbol];
				if(val.arguments.size() != items.size()) {
					ERR_CF << "preprocessor symbol '" << symbol << "' has "
					       << items.size() << " arguments, "
					       << val.arguments.size() << " expected: '" << newfilename << "'\n";
				}

				std::string str = val.value;

				//substitute in given arguments
				for(size_t n = 0; n != val.arguments.size(); ++n) {
					const std::string& replace_with = (n < items.size()) ? items[n] : "";

					int subs = 0;

					const std::string item = "{" + val.arguments[n] + "}";
					std::string::size_type pos = str.find(item);
					while(pos != std::string::npos) {
						++subs;
						str.replace(pos,item.size(),replace_with);
						const std::string::size_type new_pos = str.find(item);
						if(new_pos < pos+replace_with.size()) {
							ERR_CF << "macro substitution in symbol '" << symbol
							       << "' could lead to infinite recursion. Aborting.\n";
							break;
						}

						pos = new_pos;
					}
				}

				std::ostringstream from;
				if (!in_quotes && !included_from.empty()) {
					from << " {" << symbol << "} " << current_line << included_from;
					if (previous_was_newline)
						out << "#line 0" << from.str();
				}
				std::istringstream stream(str);
				internal_preprocess_data(stream, defines_map, depth, out,
				                         from.str(), !previous_was_newline, "");
			} else if(depth < 20) {
				std::string prefix;
				std::string nfname;

#ifdef USE_ZIPIOS
				if(newfilename != "" && newfilename[0] == '~') {
					// I do not know of any valid use of {~xxx} when {xxx} is
					// not used, and zipios takes care of both
					LOG_CF << "ignoring reference to '" << newfilename << "'\n";
				} else
#endif
				{
#ifndef USE_ZIPIOS
					//if the filename begins with a '~', then look
					//in the user's data directory. If the filename begins with
					//a '@' then we look in the user's data directory,
					//but default to the standard data directory if it's not found
					//there.
					if(newfilename != "" && (newfilename[0] == '~' || newfilename[0] == '@')) {
						nfname = newfilename;
						nfname.erase(nfname.begin(),nfname.begin()+1);
						nfname = get_user_data_dir() + "/data/" + nfname;

						LOG_CF << "got relative name '" << newfilename << "' -> '" << nfname << "'\n";

						if(newfilename[0] == '@' && file_exists(nfname) == false && is_directory(nfname) == false) {
							nfname = "data/" + newfilename.substr(1);
						}
					} else
#endif
					if(newfilename.size() >= 2 && newfilename[0] == '.' &&
						newfilename[1] == '/' ) {
						//if the filename begins with a "./", then look
						//in the same directory as the file currrently
						//being preprocessed
						nfname = newfilename;
						nfname.erase(nfname.begin(),nfname.begin()+2);
						nfname = current_directory + nfname;
					
					} else {
#ifdef USE_ZIPIOS
						if(newfilename != "" && newfilename[0] == '@') {
							nfname = newfilename;
							nfname.erase(nfname.begin(),nfname.begin()+1);
							nfname = "data/" + nfname;
						} else
#endif

							nfname = "data/" + newfilename;
					}

					std::ostringstream from;
					if (!in_quotes && !included_from.empty())
						from << ' ' << current_line << included_from;
					internal_preprocess_file(nfname, defines_map, depth + 1, out,
					                         from.str(), previous_was_newline);
				}
			} else {
				scoped_istream stream = istream_file(newfilename);
				out << stream->rdbuf();
			}

			previous_was_newline = false;
			need_linenum = true;
		} else if(c == '#' && !in_quotes) {
			//we are about to skip some things, so keep track of
			//the start of where we're skipping, so we can count
			//the number of newlines, so we can track the line number
			//in the source file
			const std::string::const_iterator begin = i;

			//if this is the beginning of a pre-processing definition
			static const std::string hash_define("#define");
			if(size_t(data.end() - i) > hash_define.size() &&
			   std::equal(hash_define.begin(),hash_define.end(),i)) {

				i += hash_define.size();

				i = std::find_if(i,data.end(),isgraph);

				const std::string::const_iterator end = std::find_if(i, data.end(), utils::isnewline);

				if(end == data.end())
					break;

				const std::string items(i,end);
				std::vector<std::string> args = utils::split(items, ' ');
				const std::string symbol = args.front();
				args.erase(args.begin());

				std::stringstream value;
				static const std::string hash_enddef("#enddef");
				for(i = end+1; i <= data.end() - hash_enddef.size(); ++i) {
					if(std::equal(hash_enddef.begin(),hash_enddef.end(),i)) {
						break;
					}

					value << *i;
				}

				if(i > data.end() - hash_enddef.size()) {
					throw config::error("pre-processing condition unterminated " +
					                    included_from + ": '" + items + "'");
				}

				i += hash_enddef.size();

				defines_map.insert(std::pair<std::string,preproc_define>(
				                    symbol,preproc_define(value.str(),args)));
			}

			//if this is a pre-processing conditional
			static const std::string hash_ifdef("#ifdef");
			static const std::string hash_else("#else");
			static const std::string hash_endif("#endif");

			if(size_t(data.end() - i) > hash_ifdef.size() &&
			   std::equal(hash_ifdef.begin(),hash_ifdef.end(),i)) {
				i += hash_ifdef.size();
				while(i != data.end() && utils::portable_isspace(*i))
					++i;

				const std::string::const_iterator end = std::find_if(i, data.end(),
				                                                     utils::portable_isspace);

				if(end == data.end())
					break;

				//if the symbol is not defined, then we want to skip
				//to the #endif or #else . Otherwise, continue processing
				//as normal. The #endif will just be treated as a comment
				//anyway.
				const std::string symbol(i,end);
				if(defines_map.count(symbol) == 0) {
					while(size_t(data.end() - i) > hash_endif.size() &&
					      !std::equal(hash_endif.begin(),hash_endif.end(),i) &&
					      !std::equal(hash_else.begin(),hash_else.end(),i)) {
						++i;
					}

					i = std::find_if(i,data.end(), utils::isnewline);
					if(i == data.end())
						break;
				} else {
					i = end;
				}
			}

			//if we come across a #else, it must mean that we found a #ifdef
			//earlier, and we should ignore until #endif
			if(size_t(data.end() - i) > hash_else.size() &&
			   std::equal(hash_else.begin(),hash_else.end(),i)) {
				while(size_t(data.end() - i) > hash_endif.size() &&
				      !std::equal(hash_endif.begin(),hash_endif.end(),i)) {
					++i;
				}

				i = std::find_if(i, data.end(), utils::isnewline);
				if(i == data.end())
					break;
			}

			static const std::string hash_textdomain("#textdomain");
			//if we find a #textdomain directive, pass it untouched
			if(size_t(data.end() - i) > hash_textdomain.size() &&
					std::equal(hash_textdomain.begin(),hash_textdomain.end(),i)) {

				i += hash_textdomain.size();
				while(i != data.end() && utils::portable_isspace(*i))
					++i;

				const std::string::const_iterator end = std::find_if(i, data.end(),
						utils::portable_isspace);
				if(end == data.end())
					break;
				const std::string symbol(i,end);
				//put the textdomain to the output stream
				out << hash_textdomain << " " << symbol;
			}

			i = std::find_if(i, data.end(), utils::isnewline);

			if(i == data.end())
				break;

			out.put('\n');
			current_line += std::count(begin, i, '\n');
			need_linenum = true;
			goto linenum_output;
		} else {
			if (c == '\n') {
				linenum_output:
				if (need_linenum && !in_quotes && !included_from.empty()) {
					out << "#line " << current_line << included_from;
					need_linenum = false;
				} else
					out.put('\n');
				++current_line;
				previous_was_newline = true;
			} else {
				out.put(c);
				if ((unsigned)c > 32)
					previous_was_newline = false;
			}
		}
	}
}

static void internal_preprocess_file(const std::string& fname,
                                     preproc_map& defines_map,
                                     int depth, std::ostream &out,
                                     std::string const &included_from,
                                     bool previous_was_newline)
{
	//if it's a directory, we process all files in the directory
	//that end in .cfg
	if(is_directory(fname)) {

		std::vector<std::string> files;
		get_files_in_dir(fname,&files,NULL,ENTIRE_FILE_PATH);

		for(std::vector<std::string>::const_iterator f = files.begin();
		    f != files.end(); ++f) {
			if(is_directory(*f) || f->size() > 4 && std::equal(f->end()-4,f->end(),".cfg")) {
				internal_preprocess_file(*f, defines_map, depth, out, included_from, true);
			}
		}

		return;
	}

	std::string from;
	if (!included_from.empty()) {
		from = ' ' + fname + included_from;
		if (previous_was_newline)
			out << "#line 0" << from;
	}
	scoped_istream stream = istream_file(fname);
	internal_preprocess_data(*stream, defines_map, depth, out, from, !previous_was_newline,
	                         directory_name(fname));
}

std::istream *preprocess_file(std::string const &fname,
                              preproc_map const *defines)
{
	log_scope("preprocessing file...");
	preproc_map defines_copy;
	if(defines != NULL)
		defines_copy = *defines;

	std::stringstream *stream = new std::stringstream;
	internal_preprocess_file(fname, defines_copy, 0, *stream, "\n", true);
	return stream;
}
