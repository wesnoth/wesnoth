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

#include "global.hpp"

#include <algorithm>
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
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/preprocessor.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "wesconfig.h"

#define ERR_CF lg::err(lg::config)
#define WRN_CF lg::warn(lg::config)
#define LOG_CF lg::info(lg::config)

bool operator<(const line_source& a, const line_source& b)
{
	return a.linenum < b.linenum;
}

namespace {

const int max_recursion_levels = 100;

bool isnewline(char c)
{
	return c == '\r' || c == '\n';
}

//make sure that we can use Mac, DOS, or Unix style text files on any system
//and they will work, by making sure the definition of whitespace is consistent
bool portable_isspace(char c)
{
	// returns true only on ASCII spaces
	if((unsigned char)c >= 128)
		return false;
	return isnewline(c) || isspace(c);
}

} //end anonymous namespace

config::config(const std::string& data,
               const std::vector<line_source>* line_sources)
{
	log_scope("parsing config...");
	read(data,line_sources);
}

config::config(const config& cfg)
{
	append(cfg);
}

config::~config()
{
	clear();
}

config& config::operator=(const config& cfg)
{
	if(this == &cfg) {
		return *this;
	}

	clear();

	append(cfg);

	return *this;
}

void config::append(const config& cfg)
{
	for(all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++i) {
		const std::pair<const std::string*,const config*>& value = *i;
		add_child(*value.first,*value.second);
	}

	for(string_map::const_iterator j = cfg.values.begin(); j != cfg.values.end(); ++j) {
		values[j->first] = j->second;
	}
}

void config::read(const std::string& data,
                  const std::vector<line_source>* line_sources)
{
	clear();

	std::stack<std::string> element_names;
	std::stack<int> element_locs;
	std::stack<config*> elements;
	std::stack<std::map<std::string,config*> > last_element; //allows [+element] syntax
	std::stack<std::string> textdomains;
	std::string current_textdomain = PACKAGE;
	std::string current_textdomain_location = "";
	elements.push(this);
	element_names.push("");
	element_locs.push(0);
	last_element.push(std::map<std::string,config*>());

	enum { ELEMENT_NAME, IN_ELEMENT, VARIABLE_NAME, VALUE }
	state = IN_ELEMENT;
	std::string var;

	std::vector<std::pair<std::string,bool> > stored_values;
	std::string value;

	bool in_quotes = false, has_quotes = false, in_comment = false, escape_next = false, translatable = false, expecting_value = false;

	int line = 0;

	for(std::string::const_iterator i = data.begin(); i != data.end(); ++i) {
		const char c = *i;
		if(c == '\r') //ignore any DOS-style newlines
			continue;

		if(c == '\n') {
			in_comment = false;
			++line;
		}

		if(*i == '#' && !in_quotes) {
			in_comment = true;
		}

		if(in_comment) {
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

							throw error(err.str());
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
							throw error(err.str());
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
						if(strip(val) == "_") {
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
						vars = config::split(var);
						values = config::split(value);
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
								strip(value);
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
		throw error("Configuration not terminated: no closing tag to '" + top + "' (line " + str_cast(element_locs.top()) + ")");
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

std::string escaped_string(const std::string& value) {
	std::vector<char> res;
	for(std::string::const_iterator i = value.begin(); i != value.end(); ++i) {
		//double interior quotes
		if(*i == '\"') res.push_back(*i);
		res.push_back(*i);
	}
	return std::string(res.begin(), res.end());
}

}

size_t config::write_size(size_t tab) const
{
	size_t res = 0;
	for(string_map::const_iterator i = values.begin(); i != values.end(); ++i) {
		if(i->second.empty() == false) {
			res += i->first.size() + AttributeEquals.size() +
			       escaped_string(i->second).size() + AttributePostfix.size() + tab;
		}
	}

	for(all_children_iterator j = ordered_begin(); j != ordered_end(); ++j) {
		const std::pair<const std::string*,const config*>& item = *j;
		const std::string& name = *item.first;
		const config& cfg = *item.second;
		res += ElementPrefix.size() + name.size() + ElementPostfix.size() +
		       cfg.write_size(tab+1) + EndElementPrefix.size() + name.size() + EndElementPostfix.size() + tab*2;
		
	}

	res += ConfigPostfix.size();

	return res;
}

std::string::iterator config::write_internal(std::string::iterator out, size_t tab) const
{
	if(tab > max_recursion_levels)
		return out;

	for(std::map<std::string,std::string>::const_iterator i = values.begin();
					i != values.end(); ++i) {
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

	for(all_children_iterator j = ordered_begin(); j != ordered_end(); ++j) {
		const std::pair<const std::string*,const config*>& item = *j;
		const std::string& name = *item.first;
		const config& cfg = *item.second;

		std::fill(out,out+tab,'\t');
		out += tab;

		out = std::copy(ElementPrefix.begin(),ElementPrefix.end(),out);
		out = std::copy(name.begin(),name.end(),out);
		out = std::copy(ElementPostfix.begin(),ElementPostfix.end(),out);
		out = cfg.write_internal(out,tab+1);

		std::fill(out,out+tab,'\t');
		out += tab;

		out = std::copy(EndElementPrefix.begin(),EndElementPrefix.end(),out);
		out = std::copy(name.begin(),name.end(),out);
		out = std::copy(EndElementPostfix.begin(),EndElementPostfix.end(),out);
	}

	out = std::copy(ConfigPostfix.begin(),ConfigPostfix.end(),out);
	return out;
}

std::string config::write() const
{
	log_scope("config::write");

	std::string res;

	res.resize(write_size());

	const std::string::iterator i = write_internal(res.begin());
	wassert(i == res.end());
	if(i != res.end()) {
		ERR_CF << "size of config buffer: " << (i - res.begin()) << "/" << res.size() << "\n";
	}

	return res;
}

//data compression. Compression is designed for network traffic.
//assumptions compression is based on:
// - most space is taken up by element names and attribute names
// - there are relatively few element names and attribute names that are repeated many times
//
//how it works: there are some control characters:
// 'compress_open_element': signals that the next word found is an element.
// any words found that are not after this are assumed to be attributes
// 'compress_close_element': signals to close the current element
// 'compress_schema_item': signals that following is a nul-delimited string, which should
//                         be added as a word in the schema
// 'compress_literal_word': signals that following is a word stored as a nul-delimited string
//    (an attribute name, unless it was preceeded by 'compress_open_element')
//
// all other characters are mapped to words. When an item is inserted into the schema,
// it is mapped to the first available character. Any attribute found is always followed
// by a nul-delimited string which is the value for the attribute.
//
// the schema objects are designed to be persisted. That is, in a network game, both peers
// can store their schema objects, and so rather than sending schema data each time, the peers
// use and build their schemas as the game progresses, adding a new word to the schema anytime
// it is required.
namespace {
	const unsigned int compress_open_element = 0, compress_close_element = 1,
	                   compress_schema_item = 2, compress_literal_word = 3,
	                   compress_first_word = 4, compress_end_words = 256;
	const size_t compress_max_words = compress_end_words - compress_first_word;
	const size_t max_schema_item_length = 20;

	void compress_output_literal_word(const std::string& word, std::vector<char>& output)
	{
		output.resize(output.size() + word.size());
		std::copy(word.begin(),word.end(),output.end()-word.size());
		output.push_back(char(0));
	}

	compression_schema::word_char_map::const_iterator add_word_to_schema(const std::string& word, compression_schema& schema)
	{
		if(word.size() > max_schema_item_length) {
			throw config::error("Schema item is too long");
		}

		unsigned int c = compress_first_word + schema.word_to_char.size();

		schema.char_to_word.insert(std::pair<unsigned int,std::string>(c,word));
		return schema.word_to_char.insert(std::pair<std::string,unsigned int>(word,c)).first;
	}

	compression_schema::word_char_map::const_iterator get_word_in_schema(const std::string& word, compression_schema& schema, std::vector<char>& output)
	{
		if(word.size() > max_schema_item_length) {
			return schema.word_to_char.end();
		}

		//see if this word is already in the schema
		const compression_schema::word_char_map::const_iterator w = schema.word_to_char.find(word);
		if(w != schema.word_to_char.end()) {
			//in the schema. Return it
			return w;
		} else if(schema.word_to_char.size() < compress_max_words) {
			//we can add the word to the schema

			//we insert the code to add a schema item, followed by the zero-delimited word
			output.push_back(char(compress_schema_item));
			compress_output_literal_word(word,output);

			return add_word_to_schema(word,schema);
		} else {
			//it's not there, and there's no room to add it
			return schema.word_to_char.end();
		}
	}

	void compress_emit_word(const std::string& word, compression_schema& schema, std::vector<char>& res)
	{
		//get the word in the schema
		const compression_schema::word_char_map::const_iterator w = get_word_in_schema(word,schema,res);
		if(w != schema.word_to_char.end()) {
			//the word is in the schema, all we have to do is output the compression code for it.
			res.push_back(w->second);
		} else {
			//the word is not in the schema. Output it as a literal word
			res.push_back(char(compress_literal_word));
			compress_output_literal_word(word,res);
		}
	}

	std::string::const_iterator compress_read_literal_word(std::string::const_iterator i1, std::string::const_iterator i2, std::string& res)
	{
		const std::string::const_iterator end_word = std::find(i1,i2,0);
		if(end_word == i2) {
			throw config::error("Unexpected end of data in compressed config read\n");
		}

		res = std::string(i1,end_word);
		return end_word;
	}
}

void config::write_compressed_internal(compression_schema& schema, std::vector<char>& res, int level) const
{
	if(level > max_recursion_levels)
		throw config::error("Too many recursion levels in compressed config write\n");

	for(std::map<std::string,std::string>::const_iterator i = values.begin();
					i != values.end(); ++i) {
		if(i->second.empty() == false) {
			
			//output the name, using compression
			compress_emit_word(i->first,schema,res);

			//output the value, with no compression
			compress_output_literal_word(i->second,res);
		}
	}

	for(all_children_iterator j = ordered_begin(); j != ordered_end(); ++j) {
		const std::pair<const std::string*,const config*>& item = *j;
		const std::string& name = *item.first;
		const config& cfg = *item.second;

		res.push_back(compress_open_element);
		compress_emit_word(name,schema,res);
		cfg.write_compressed_internal(schema,res, level+1);
		res.push_back(compress_close_element);
	}
}

std::string config::write_compressed(compression_schema& schema) const
{
	std::vector<char> res;
	write_compressed_internal(schema,res,0);
	std::string s;
	s.resize(res.size());
	std::copy(res.begin(),res.end(),s.begin());
	return s;
}

std::string::const_iterator config::read_compressed_internal(std::string::const_iterator i1, std::string::const_iterator i2, compression_schema& schema, int level)
{
	if(level >= max_recursion_levels)
		throw config::error("Too many recursion levels in compressed config read\n");
	
	bool in_open_element = false;
	while(i1 != i2) {
		switch(*i1) {
		case compress_open_element:
			in_open_element = true;
			break;
		case compress_close_element:
			return i1;
		case compress_schema_item: {
			std::string word;
			i1 = compress_read_literal_word(i1+1,i2,word);

			add_word_to_schema(word,schema);

			break;
		}

		default: {
			std::string word;
			if(*i1 == compress_literal_word) {
				i1 = compress_read_literal_word(i1+1,i2,word);				
			} else {
				const unsigned char c = *i1;
				unsigned int code = c;

				const compression_schema::char_word_map::const_iterator itor = schema.char_to_word.find(code);
				if(itor == schema.char_to_word.end()) {
					ERR_CF << "illegal word code: " << code << "\n";
					throw error("Illegal character in compression input\n");
				}

				word = itor->second;
			}

			if(in_open_element) {
				in_open_element = false;
				config& cfg = add_child(word);
				i1 = cfg.read_compressed_internal(i1+1,i2,schema,level+1);
			} else {
				//we have a name/value pair, the value is always a literal string
				std::string value;
				i1 = compress_read_literal_word(i1+1,i2,value);
				values.insert(std::pair<std::string,std::string>(word,value));
			}
		}

		} //end switch

		if(i1 == i2)
			return i2;
		++i1;
	}

	return i1;
}

void config::read_compressed(const std::string& data, compression_schema& schema)
{
	clear();
	read_compressed_internal(data.begin(),data.end(),schema,0);
}

bool config::detect_format_and_read(const std::string& data)
{
	try {
		compression_schema schema;
		read_compressed(data,schema);
		return true;
	} catch(config::error&) {
	}

	read(data);
	return false;
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
	const child_map::const_iterator i = children.find(key);
	if(i != children.end() && i->second.empty() == false) {
		return i->second.front();
	} else {
		return NULL;
	}
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

config& config::add_child_at(const std::string& key, const config& val, size_t index)
{
	child_list& v = children[key];
	if(index > v.size()) {
		throw error("illegal index to add child at");
	}

	v.insert(v.begin()+index,new config(val));

	bool inserted = false;

	const child_pos value(children.find(key),index);

	std::vector<child_pos>::iterator ord = ordered_children.begin();
	for(; ord != ordered_children.end(); ++ord) {
		if(!inserted && ord->index == index && ord->pos->first == key) {
			ord = ordered_children.insert(ord,value);
			inserted = true;
		} else if(ord->index >= index && ord->pos->first == key) {
			ord->index++;
		}
	}

	if(!inserted) {
		ordered_children.push_back(value);
	}

	return *v[index];
}

struct remove_ordered {
	remove_ordered(const std::string& key) : key_(key) {}

	bool operator()(const config::child_pos& pos) const { return pos.pos->first == key_; }
private:
	std::string key_;
};

void config::clear_children(const std::string& key)
{
	ordered_children.erase(std::remove_if(ordered_children.begin(),ordered_children.end(),remove_ordered(key)),ordered_children.end());
	children.erase(key);
}

void config::remove_child(const std::string& key, size_t index)
{
	//remove from the ordering
	const child_pos pos(children.find(key),index);
	ordered_children.erase(std::remove(ordered_children.begin(),ordered_children.end(),pos),ordered_children.end());

	//decrement all indices in the ordering that are above this index, since everything
	//is getting shifted back by 1.
	for(std::vector<child_pos>::iterator i = ordered_children.begin(); i != ordered_children.end(); ++i) {
		if(i->pos->first == key && i->index > index) {
			i->index--;
		}
	}

	//remove from the child map
	child_list& v = children[key];
	wassert(index < v.size());
	config* const res = v[index];
	v.erase(v.begin()+index);
	delete res;
}

std::string& config::operator[](const std::string& key)
{
	return values[key];
}

const std::string& config::operator[](const std::string& key) const
{
	const std::string& str = get_attribute(key);
	//see if the value is a variable
	if(str != "" && str[0] == '$') {
		return game_events::get_variable_const(std::string(str.begin()+1,str.end()));
	} else {
		return str;
	}
}

const std::string& config::get_attribute(const std::string& key) const
{
	const string_map::const_iterator i = values.find(key);
	if(i != values.end()) {
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

std::string config::join(const std::vector<std::string>& v, char c)
{
	std::stringstream str;
	for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
		str << *i;
		if(i+1 != v.end()) {
			str << c;
		}
	}

	return str.str();
}

std::vector<std::string> config::split(const std::string& val, char c, int flags)
{
	std::vector<std::string> res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while(i2 != val.end()) {
		if(*i2 == c) {
			std::string new_val(i1,i2);
			if(flags & STRIP_SPACES)
				strip(new_val);
			if(!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if(flags & STRIP_SPACES) {
				while(i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1,i2);
	if(flags & STRIP_SPACES)
		strip(new_val);
	if(!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

//identical to split(), except it does not split when it otherwise
//would if the previous character was identical to the parameter 'quote'.
//i.e. it does not split quoted commas.
//this method was added to make it possible to quote user input,
//particularly so commas in user input will not cause visual problems in menus.
//why not change split()? that would change the methods post condition.
std::vector<std::string> config::quoted_split(const std::string& val, char c, int flags, char quote)
{
	std::vector<std::string> res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while(i2 != val.end()) {
		if(*i2 == quote) {
			// ignore quoted character
			++i2;
			if(i2 != val.end()) ++i2;
		} else if(*i2 == c) {
			std::string new_val(i1,i2);
			if(flags & STRIP_SPACES)
				strip(new_val);
			if(!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if(flags & STRIP_SPACES) {
				while(i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1,i2);
	if(flags & STRIP_SPACES)
		strip(new_val);
	if(!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

std::pair<int,int> config::parse_range(const std::string& str)
{
	const std::string::const_iterator dash = std::find(str.begin(),str.end(),'-');
	const std::string a(str.begin(),dash);
	const std::string b = dash != str.end() ? std::string(dash+1,str.end()) : a;
	std::pair<int,int> res(atoi(a.c_str()),atoi(b.c_str()));
	if(res.second < res.first)
		res.second = res.first;

	return res;
}

//make sure we regard '\r' and '\n' as a space, since Mac, Unix, and DOS
//all consider these differently.
bool config::notspace(char c) { return !portable_isspace(c); }

//prepend all special characters with a backslash
//special characters are:
//#@{}+-,\*
std::string& config::escape(std::string& str)
{
	if(!str.empty()) {
		std::string::size_type pos = 0;

		do {
			pos = str.find_first_of("#@{}+-,\\*",pos);
			if(pos != std::string::npos) {
				str.insert(pos,1,'\\');
				pos += 2;
			}
		} while(pos < str.size() && pos != std::string::npos);
	}
	return str;
}
// remove all escape characters (backslash)
std::string& config::unescape(std::string& str)
{
	std::string::size_type pos = 0;

	do {
		pos = str.find('\\',pos);
		if(pos != std::string::npos) {
			str.erase(pos,1);
			++pos;
		}
	} while(pos < str.size() && pos != std::string::npos);
	return str;
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

namespace {

bool not_id(char c)
{
	return !isdigit(c) && !isalpha(c) && c != '.' && c != '_';
}

void do_interpolation(std::string& res, size_t npos, const string_map* m)
{
	LOG_CF << "doing interpolation into '" << res << "': " << npos << "\n";
	const std::string::iterator i = std::find(res.begin()+npos,res.end(),'$');
	if(i == res.end() || i+1 == res.end()) {
		return;
	}

	npos = i - res.begin();

	const std::string::iterator end = std::find_if(i+1,res.end(),not_id);

	const std::string key(i+1,end);
	res.erase(i,end);

	if(m != NULL) {
		const string_map::const_iterator itor = m->find(key);
		if(itor != m->end()) {
			res.insert(npos,itor->second);
		}
	} else {
		res.insert(npos,game_events::get_variable_const(key));
	}

	do_interpolation(res,npos,m);
}

}

std::string config::interpolate_variables_into_string(const std::string& str, const string_map* symbols)
{
	std::string res = str;
	do_interpolation(res,0,symbols);

	//remove any pipes in the string, as they are used simply to seperate variables
	res.erase(std::remove(res.begin(),res.end(),'|'),res.end());

	return res;
}

void config::clear()
{
	for(std::map<std::string,std::vector<config*> >::iterator i = children.begin(); i != children.end(); ++i) {
		std::vector<config*>& v = i->second;
		for(std::vector<config*>::iterator j = v.begin(); j != v.end(); ++j)
			delete *j;
	}

	children.clear();
	values.clear();
	ordered_children.clear();
}

bool config::empty() const
{
	return children.empty() && values.empty();
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

config config::get_diff(const config& c) const
{
	config res;

	config* inserts = NULL;

	string_map::const_iterator i;
	for(i = values.begin(); i != values.end(); ++i) {
		const string_map::const_iterator j = c.values.find(i->first);
		if(j == c.values.end() || i->second != j->second && i->second != "") {
			if(inserts == NULL) {
				inserts = &res.add_child("insert");
			}

			(*inserts)[i->first] = i->second;
		}
	}

	config* deletes = NULL;

	for(i = c.values.begin(); i != c.values.end(); ++i) {
		const string_map::const_iterator itor = values.find(i->first);
		if(itor == values.end() || itor->second == "") {
			if(deletes == NULL) {
				deletes = &res.add_child("delete");
			}

			(*deletes)[i->first] = "x";
		}
	}

	std::vector<std::string> entities;

	child_map::const_iterator ci;
	for(ci = children.begin(); ci != children.end(); ++ci) {
		entities.push_back(ci->first);
	}

	for(ci = c.children.begin(); ci != c.children.end(); ++ci) {
		if(children.count(ci->first) == 0) {
			entities.push_back(ci->first);
		}
	}

	for(std::vector<std::string>::const_iterator itor = entities.begin(); itor != entities.end(); ++itor) {

		const child_map::const_iterator itor_a = children.find(*itor);
		const child_map::const_iterator itor_b = c.children.find(*itor);

		static const child_list dummy;

		//get the two child lists. 'b' has to be modified to look like 'a'
		const child_list& a = itor_a != children.end() ? itor_a->second : dummy;
		const child_list& b = itor_b != c.children.end() ? itor_b->second : dummy;
		
		size_t ndeletes = 0;
		size_t ai = 0, bi = 0;
		while(ai != a.size() || bi != b.size()) {
			//if the two elements are the same, nothing needs to be done
			if(ai < a.size() && bi < b.size() && *a[ai] == *b[bi]) {
				++ai;
				++bi;
			} else {
				//we have to work out what the most appropriate operation --
				//delete, insert, or change is the best to get b[bi] looking like a[ai]
				std::stringstream buf;

				//if b has more elements than a, then we assume this element is an
				//element that needs deleting
				if(b.size() - bi > a.size() - ai) {
					config& new_delete = res.add_child("delete_child");
					buf << bi - ndeletes;
					new_delete.values["index"] = buf.str();
					new_delete.add_child(*itor);

					++ndeletes;
					++bi;
				} 

				//if b has less elements than a, then we assume this element is an
				//element that needs inserting
				else if(b.size() - bi < a.size() - ai) {
					config& new_insert = res.add_child("insert_child");
					buf << ai;
					new_insert.values["index"] = buf.str();
					new_insert.add_child(*itor,*a[ai]);

					++ai;
				}

				//otherwise, they have the same number of elements, so try just
				//changing this element to match
				else {
					config& new_change = res.add_child("change_child");
					buf << bi;
					new_change.values["index"] = buf.str();
					new_change.add_child(*itor,a[ai]->get_diff(*b[bi]));

					++ai;
					++bi;
				}
			}
		}
	}


	return res;
}

void config::apply_diff(const config& diff)
{
	const config* const inserts = diff.child("insert");
	if(inserts != NULL) {
		for(string_map::const_iterator i = inserts->values.begin(); i != inserts->values.end(); ++i) {
			values[i->first] = i->second;
		}
	}

	const config* const deletes = diff.child("delete");
	if(deletes != NULL) {
		for(string_map::const_iterator i = deletes->values.begin(); i != deletes->values.end(); ++i) {
			values.erase(i->first);
		}
	}

	const child_list& child_changes = diff.get_children("change_child");
	child_list::const_iterator i;
	for(i = child_changes.begin(); i != child_changes.end(); ++i) {
		const size_t index = atoi((**i)["index"].c_str());
		for(all_children_iterator j = (*i)->ordered_begin(); j != (*i)->ordered_end(); ++j) {
			const std::pair<const std::string*,const config*> item = *j;

			if(item.first->empty()) {
				continue;
			}
			
			const child_map::iterator itor = children.find(*item.first);
			if(itor == children.end() || index >= itor->second.size()) {
				throw error("error in diff: could not find element '" + *item.first + "'");
			}

			itor->second[index]->apply_diff(*item.second);
		}
	}

	const child_list& child_inserts = diff.get_children("insert_child");
	for(i = child_inserts.begin(); i != child_inserts.end(); ++i) {
		const size_t index = atoi((**i)["index"].c_str());
		for(all_children_iterator j = (*i)->ordered_begin(); j != (*i)->ordered_end(); ++j) {
			const std::pair<const std::string*,const config*> item = *j;
			add_child_at(*item.first,*item.second,index);
		}
	}

	const child_list& child_deletes = diff.get_children("delete_child");
	for(i = child_deletes.begin(); i != child_deletes.end(); ++i) {
		const size_t index = atoi((**i)["index"].c_str());
		for(all_children_iterator j = (*i)->ordered_begin(); j != (*i)->ordered_end(); ++j) {
			const std::pair<const std::string*,const config*> item = *j;

			remove_child(*item.first,index);
		}
	}
}

bool operator==(const config& a, const config& b)
{
	if(a.values.size() != b.values.size()) {
		return false;
	}

	for(string_map::const_iterator i = a.values.begin(); i != a.values.end(); ++i) {
		const string_map::const_iterator j = b.values.find(i->first);
		if(j == b.values.end() || i->second != j->second) {
			return false;
		}
	}

	config::all_children_iterator x = a.ordered_begin(), y = b.ordered_begin();
	while(x != a.ordered_end() && y != b.ordered_end()) {
		const std::pair<const std::string*,const config*> val1 = *x;
		const std::pair<const std::string*,const config*> val2 = *y;

		if(*val1.first != *val2.first || *val1.second != *val2.second) {
			return false;
		}

		++x;
		++y;
	}

	return x == a.ordered_end() && y == b.ordered_end();
}

bool operator!=(const config& a, const config& b)
{
	return !operator==(a,b);
}

//#define TEST_CONFIG

#ifdef TEST_CONFIG

int main()
{
	config cfg(read_file("testconfig"));
	std::cout << cfg.write() << std::endl;
}

#endif
