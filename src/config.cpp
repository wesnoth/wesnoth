/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stack>
#include <sstream>
#include <vector>

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
#include "log.hpp"
#include "scoped_resource.hpp"

bool operator<(const line_source& a, const line_source& b)
{
	return a.linenum < b.linenum;
}

namespace {

bool isnewline(char c)
{
	return c == '\r' || c == '\n';
}

//make sure that we can use Mac, DOS, or Unix style text files on any system
//and they will work, by making sure the definition of whitespace is consistent
bool portable_isspace(char c)
{
	return isnewline(c) || isspace(c);
}


line_source get_line_source(const std::vector<line_source>& line_src, int line)
{
	line_source res(line,"",0);
	std::vector<line_source>::const_iterator it =
	           std::upper_bound(line_src.begin(),line_src.end(),res);
	if(it != line_src.begin()) {
		--it;
		res.file = it->file;
		res.fileline = it->fileline + (line - it->linenum);
	}

	return res;
}

struct close_FILE
{
	void operator()(FILE* f) const { if(f != NULL) { fclose(f); } }
};

void read_file_internal(const std::string& fname, std::string& res)
{
	res.resize(0);

	const util::scoped_resource<FILE*,close_FILE> file(fopen(fname.c_str(),"rb"));
	if(file == NULL) {
		return;
	}

	const int block_size = 4096;
	char buf[block_size];

	for(;;) {
		size_t nbytes = fread(buf,1,block_size,file);
		res.resize(res.size()+nbytes);
		std::copy(buf,buf+nbytes,res.end()-nbytes);

		if(nbytes < block_size) {
			return;
		}
	}
}

} //end anon namespace

std::string read_file(const std::string& fname)
{
	log_scope("reading file");

	//if we have a path to the data,
	//convert any filepath which is relative
	if(!fname.empty() && fname[0] != '/' && !game_config::path.empty()) {
		std::string res;
		read_file_internal(game_config::path + "/" + fname,res);
		if(!res.empty()) {
			return res;
		}
	}

	std::string res;
	read_file_internal(fname,res);
	return res;
}

void write_file(const std::string& fname, const std::string& data)
{
	std::ofstream file(fname.c_str());
	if(file.bad()) {
		std::cerr << "error writing to file: '" << fname << "'\n";
	}
	for(std::string::const_iterator i = data.begin(); i != data.end(); ++i) {
		file << *i;
	}
}

namespace {

void internal_preprocess_file(const std::string& fname,
                              preproc_map& defines_map,
                              int depth, std::vector<char>& res,
                              std::vector<line_source>* lines_src, int& line);

void internal_preprocess_data(const std::string& data,
                              preproc_map& defines_map,
                              int depth, std::vector<char>& res,
                              std::vector<line_source>* lines_src, int& line,
							  const std::string& fname, int srcline)
{
	bool in_quotes = false;

	for(std::string::const_iterator i = data.begin(); i != data.end(); ++i) {
		const char c = *i;
		if(c == '"') {
			in_quotes = !in_quotes;
		}

		if(c == '{') {
			std::stringstream newfile;
			for(++i; i != data.end() && *i != '}'; ++i) {
				newfile << *i;
			}

			if(i == data.end())
				break;

			const std::string newfilename = newfile.str();
			std::vector<std::string> items = config::split(newfilename,' ');
			const std::string symbol = items.front();

			//if this is a known pre-processing symbol, then we insert
			//it, otherwise we assume it's a file name to load
			if(defines_map.count(symbol) != 0) {
				items.erase(items.begin());

				const preproc_define& val = defines_map[symbol];
				if(val.arguments.size() != items.size()) {
					std::cerr << "error: preprocessor symbol '" << symbol << "' has "
					          << items.size() << " arguments, "
							  << val.arguments.size() << " expected\n";
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
							std::cerr << "macro substitution in symbol '" << symbol
								      << "' could lead to infinite recursion. Aborting.\n";
							break;
						}

						pos = new_pos;
					}
				}

				internal_preprocess_data(str,defines_map,depth,res,NULL,line,fname,srcline);
			} else if(depth < 20) {
				internal_preprocess_file("data/" + newfilename,
				                       defines_map, depth+1,res,
				                       lines_src,line);
			} else {
				const std::string& str = read_file(newfilename);
				res.insert(res.end(),str.begin(),str.end());
				line += std::count(str.begin(),str.end(),'\n');
			}

			if(lines_src != NULL) {
				lines_src->push_back(line_source(line,fname,srcline));
			}
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

				const std::string::const_iterator end = std::find_if(i,data.end(),isnewline);

				if(end == data.end())
					break;

				const std::string items(i,end);
				std::vector<std::string> args = config::split(items,' ');
				const std::string symbol = args.front();
				args.erase(args.begin());

				std::stringstream value;
				for(i = end+1; i != data.end(); ++i) {
					static const std::string hash_enddef("#enddef");
					if(size_t(data.end() - i) > hash_enddef.size() &&
					   std::equal(hash_enddef.begin(),hash_enddef.end(),i)) {
						i += hash_enddef.size();
						break;
					}

					value << *i;
				}

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
				while(i != data.end() && portable_isspace(*i))
					++i;

				const std::string::const_iterator end = std::find_if(i,data.end(),portable_isspace);

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

					i = std::find_if(i,data.end(),isnewline);
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

				i = std::find_if(i,data.end(),isnewline);
				if(i == data.end())
					break;
			}

			i = std::find_if(i,data.end(),isnewline);

			if(i == data.end())
				break;

			srcline += std::count(begin,i,'\n');
			++line;

			res.push_back('\n');
		} else {
			if(c == '\n') {
				++line;
				++srcline;
			}

			res.push_back(c);
		}
	}
}

void internal_preprocess_file(const std::string& fname,
                              preproc_map& defines_map,
                              int depth, std::vector<char>& res,
                              std::vector<line_source>* lines_src, int& line)
{
	//if it's a directory, we process all files in the directory
	//that end in .cfg
	if(is_directory(fname)) {

		std::vector<std::string> files;
		get_files_in_dir(fname,&files,NULL,ENTIRE_FILE_PATH);

		for(std::vector<std::string>::const_iterator f = files.begin();
		    f != files.end(); ++f) {
			if(f->size() > 4 && std::equal(f->end()-4,f->end(),".cfg")) {
				internal_preprocess_file(*f,defines_map,depth,res,
				                         lines_src,line);
			}
		}

		return;
	}

	if(lines_src != NULL) {
		lines_src->push_back(line_source(line,fname,1));
	}

	internal_preprocess_data(read_file(fname),defines_map,depth,res,lines_src,line,fname,1);
}

} //end anonymous namespace

std::string preprocess_file(const std::string& fname,
                            const preproc_map* defines,
                            std::vector<line_source>* line_sources)
{
	log_scope("preprocessing file...");
	preproc_map defines_copy;
	if(defines != NULL)
		defines_copy = *defines;

	std::vector<char> res;
	int linenum = 0;
	internal_preprocess_file(fname,defines_copy,0,res,line_sources,linenum);
	return std::string(res.begin(),res.end());
}

config::config(const std::string& data,
               const std::vector<line_source>* line_sources)
{
	log_scope("parsing config...");
	read(data,line_sources);
}

config::config(const config& cfg) : values(cfg.values)
{
	for(std::map<std::string,std::vector<config*> >::const_iterator i =
	    cfg.children.begin(); i != cfg.children.end(); ++i) {
		for(std::vector<config*>::const_iterator j = i->second.begin();
		    j != i->second.end(); ++j) {
			add_child(i->first,**j);
		}
	}
}

config::~config()
{
	clear();
}

config& config::operator=(const config& cfg)
{
	clear();

	values = cfg.values;

	for(std::map<std::string,std::vector<config*> >::const_iterator i =
	    cfg.children.begin(); i != cfg.children.end(); ++i) {
		for(std::vector<config*>::const_iterator j = i->second.begin();
		    j != i->second.end(); ++j) {
			add_child(i->first,**j);
		}
	}

	return *this;
}

void config::read(const std::string& data,
                  const std::vector<line_source>* line_sources)
{
	clear();

	std::stack<std::string> element_names;
	std::stack<config*> elements;
	elements.push(this);
	element_names.push("");

	enum { ELEMENT_NAME, IN_ELEMENT, VARIABLE_NAME, VALUE }
	state = IN_ELEMENT;
	std::string var;
	std::string value;

	bool in_quotes = false, has_quotes = false;

	int line = 0;

	for(std::string::const_iterator i = data.begin(); i != data.end(); ++i) {
		const char c = *i;
		if(c == '\r') //ignore any DOS-style newlines
			continue;

		if(c == '\n')
			++line;

		switch(state) {
			case ELEMENT_NAME:
				if(c == ']') {
					if(value == "end" || !value.empty() && value[0] == '/') {
						assert(!elements.empty());

						if(value[0] == '/' &&
						   std::string("/" + element_names.top()) != value) {
							std::stringstream err;

							if(line_sources != NULL) {
								const line_source src =
								        get_line_source(*line_sources,line);

								err << src.file << " " << src.fileline << ": ";
							}

							err << "Found illegal end tag: '" << value
							    << "', at end of '"
							    << element_names.top() << "'";

							throw error(err.str());
						}

						elements.pop();
						element_names.pop();

						if(elements.empty()) {
							std::stringstream err;

							if(line_sources != NULL) {
								const line_source src =
								        get_line_source(*line_sources,line);

								err << src.file << " " << src.fileline << ": ";
							}

							err << "Unexpected terminating tag\n";
							throw error(err.str());
							return;
						}


						state = IN_ELEMENT;

						break;
					}
					;
					elements.push(&elements.top()->add_child(value));
					element_names.push(value);
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
				} else if(!portable_isspace(c)) {
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
				if(c == '"') {
					in_quotes = !in_quotes;
					has_quotes = true;
				} else if(c == '\n' && !in_quotes) {
					state = IN_ELEMENT;
					elements.top()->values[var] = has_quotes ? value : strip(value);
					var = "";
					value = "";
					has_quotes = false;
				} else if(in_quotes || !has_quotes) {
					value.resize(value.size()+1);
					value[value.size()-1] = c;
				}

				break;
		}
	}

	assert(!element_names.empty());
	const std::string top = element_names.top();
	element_names.pop();
	if(!element_names.empty()) {
		throw error("Configuration not terminated: no closing tag to '" +
		            top + "'");
	}
}

std::string config::write() const
{
	std::string res;
	for(std::map<std::string,std::string>::const_iterator i = values.begin();
					i != values.end(); ++i) {
		if(i->second.empty() == false) {
			res += i->first + "=\"" + i->second + "\"\n";
		}
	}

	for(std::map<std::string,std::vector<config*> >::const_iterator j =
					children.begin(); j != children.end(); ++j) {
		const std::vector<config*>& v = j->second;
		for(std::vector<config*>::const_iterator it = v.begin();
						it != v.end(); ++it) {
			res += "[" + j->first + "]\n";
			res += (*it)->write();
			res += "[/" + j->first + "]\n";
		}
	}

	res += "\n";

	return res;
}

config::child_itors config::child_range(const std::string& key)
{
	child_map::iterator i = children.find(key);
	if(i != children.end()) {
		return child_itors(i->second.begin(),i->second.end());
	} else {
		static std::vector<config*> dummy;
		return child_itors(dummy.begin(),dummy.end());
	}
}

config::const_child_itors config::child_range(const std::string& key) const
{
	child_map::const_iterator i = children.find(key);
	if(i != children.end()) {
		return const_child_itors(i->second.begin(),i->second.end());
	} else {
		static const std::vector<config*> dummy;
		return const_child_itors(dummy.begin(),dummy.end());
	}
}

const config::child_list& config::get_children(const std::string& key) const
{
	const child_map::const_iterator i = children.find(key);
	if(i != children.end()) {
		return i->second;
	} else {
		static const child_list dummy;
		return dummy;
	}
}

const config::child_map& config::all_children() const { return children; }

config* config::child(const std::string& key)
{
	const child_map::const_iterator i = children.find(key);
	if(i != children.end() && i->second.empty() == false) {
		return i->second.front();
	} else {
		return NULL;
	}
}

const config* config::child(const std::string& key) const
{
	return const_cast<config*>(this)->child(key);
}

config& config::add_child(const std::string& key)
{
	std::vector<config*>& v = children[key];
	v.push_back(new config());
	ordered_children.push_back(child_pos(children.find(key),v.size()-1));
	return *v.back();
}

config& config::add_child(const std::string& key, const config& val)
{
	std::vector<config*>& v = children[key];
	v.push_back(new config(val));
	ordered_children.push_back(child_pos(children.find(key),v.size()-1));
	return *v.back();
}

std::string& config::operator[](const std::string& key)
{
	return values[key];
}

const std::string& config::operator[](const std::string& key) const
{
	const string_map::const_iterator i = values.find(key);
	if(i != values.end()) {
		//see if the value is a variable
		if(i->second[0] == '$') {
			return game_events::get_variable(std::string(i->second.begin()+1,i->second.end()));
		}

		return i->second;
	} else {
		static const std::string empty_string;
		return empty_string;
	}
}

config* config::find_child(const std::string& key,
                           const std::string& name,
                           const std::string& value)
{
	const child_map::iterator i = children.find(key);
	if(i == children.end())
		return NULL;

	const child_list::iterator j = std::find_if(i->second.begin(),
	                                            i->second.end(),
	                                            config_has_value(name,value));
	if(j != i->second.end())
		return *j;
	else
		return NULL;
}

const config* config::find_child(const std::string& key,
                                 const std::string& name,
                                 const std::string& value) const
{
	const child_map::const_iterator i = children.find(key);
	if(i == children.end())
		return NULL;

	const child_list::const_iterator j = std::find_if(
	                                            i->second.begin(),
	                                            i->second.end(),
	                                            config_has_value(name,value));
	if(j != i->second.end())
		return *j;
	else
		return NULL;
}

std::vector<std::string> config::split(const std::string& val, char c)
{
	std::vector<std::string> res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while(i2 != val.end()) {
		if(*i2 == c) {
			std::string new_val(i1,i2);
			strip(new_val);
			if(!new_val.empty())
				res.push_back(new_val);
			++i2;
			while(i2 != val.end() && *i2 == ' ')
				++i2;

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1,i2);
	strip(new_val);
	if(!new_val.empty())
		res.push_back(new_val);

	return res;
}

namespace {
//make sure we regard '\r' and '\n' as a space, since Mac, Unix, and DOS
//all consider these differently.
bool notspace(char c) { return !portable_isspace(c); }
}

std::string& config::strip(std::string& str)
{
	//if all the string contains is whitespace, then the whitespace may
	//have meaning, so don't strip it
	const std::string::iterator it=std::find_if(str.begin(),str.end(),notspace);
	if(it == str.end())
		return str;

	str.erase(str.begin(),it);
	str.erase(std::find_if(str.rbegin(),str.rend(),notspace).base(),str.end());

	return str;
}

bool config::has_value(const std::string& values, const std::string& val)
{
	const std::vector<std::string>& vals = split(values);
	return std::count(vals.begin(),vals.end(),val) > 0;
}

void config::clear()
{
	for(std::map<std::string,std::vector<config*> >::iterator i =
					children.begin(); i != children.end(); ++i) {
		std::vector<config*>& v = i->second;
		for(std::vector<config*>::iterator j = v.begin(); j != v.end(); ++j)
			delete *j;
	}

	children.clear();
	values.clear();
	ordered_children.clear();
}

config::all_children_iterator::all_children_iterator(config::all_children_iterator::Itor i) : i_(i)
{}

config::all_children_iterator config::all_children_iterator::operator++()
{
	++i_;
	return *this;
}

config::all_children_iterator config::all_children_iterator::operator++(int)
{
	config::all_children_iterator i = *this;
	++i_;
	return i;
}

std::pair<const std::string*,const config*> config::all_children_iterator::operator*() const
{
	return std::pair<const std::string*,const config*>(&(i_->pos->first),i_->pos->second[i_->index]);
}

bool config::all_children_iterator::operator==(all_children_iterator i) const
{
	return i_ == i.i_;
}

bool config::all_children_iterator::operator!=(all_children_iterator i) const
{
	return i_ != i.i_;
}

config::all_children_iterator config::ordered_begin() const
{
	return all_children_iterator(ordered_children.begin());
}

config::all_children_iterator config::ordered_end() const
{
	return all_children_iterator(ordered_children.end());
}

//#define TEST_CONFIG

#ifdef TEST_CONFIG

int main()
{
	config cfg(read_file("testconfig"));
	std::cout << cfg.write() << std::endl;
}

#endif
