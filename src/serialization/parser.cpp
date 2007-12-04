/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Copyright (C) 2005 - 2007 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file serialization/parser.cpp 
//! Read/Write & analyse WML- and config-files.

#include "serialization/parser.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "wesconfig.h"
#include "serialization/binary_wml.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/tokenizer.hpp"

#include <sstream>
#include <stack>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#define ERR_CF LOG_STREAM(err, config)
#define WRN_CF LOG_STREAM(warn, config)
#define LOG_CF LOG_STREAM(info, config)

static const size_t max_recursion_levels = 1000;

namespace {

class parser
{
	parser();
	parser(const parser&);
	parser& operator=(const parser&);
public:
	parser(config& cfg, std::istream& in);
	parser(config& cfg, std::string& in);
	~parser();
	void operator() (std::string* error_log=NULL);

private:
	void parse_element();
	void parse_variable();
	void parse_directive();
	std::string lineno_string(utils::string_map &map, std::string const &lineno,
			          std::string const &error_string);
	void error(const std::string& message);

	config& cfg_;
	tokenizer *tok_;

	struct element {
		element(config *cfg, std::string const &name, std::string const &start_line)
			: cfg(cfg), name(name), start_line(start_line) {}

		config* cfg;
		std::string name;

		std::map<std::string, config*> last_element_map;
		std::string start_line;
	};

	std::stack<element> elements;
};

parser::parser(config &cfg, std::istream &in)
	: cfg_(cfg), tok_(new tokenizer_stream(in))
{
}

parser::parser(config &cfg, std::string &in)
	: cfg_(cfg), tok_(new tokenizer_string(in))
{
}

parser::~parser()
{
	if(tok_) {
		delete tok_;
	}
}

void parser::operator()(std::string* error_log)
{
	cfg_.clear();
	elements.push(element(&cfg_, "", ""));

	do {
		try {
			tok_->next_token();

			switch(tok_->current_token().type) {
			case token::LF:
				continue;
			case '[':
				parse_element();
				break;
			case token::STRING:
				parse_variable();
				break;
			default:
				error(_("Unexpected characters at line start"));
				break;
			case token::END:
				break;
			}
		} catch(config::error& e) {
			if(error_log == NULL)
				throw;

			// On error, dump tokens to the next LF
			while(tok_->current_token().type != token::LF &&
					tok_->current_token().type != token::END) {
				tok_->next_token();
			}

			*error_log += e.message + '\n';
		}
		increment_parser_progress();
	} while (tok_->current_token().type != token::END);

	// The main element should be there. If it is not, this is a parser error.
	wassert(!elements.empty());

	if(elements.size() != 1) {
		utils::string_map i18n_symbols;
		i18n_symbols["tag"] = elements.top().name;
		error(lineno_string(i18n_symbols, elements.top().start_line,
				N_("Missing closing tag for tag $tag at $pos")));
	}
}

void parser::parse_element()
{
	tok_->next_token();
	std::string elname;
	config* current_element = NULL;
	std::map<std::string, config*>::const_iterator last_element_itor;

	switch(tok_->current_token().type) {
	case token::STRING: // [element]
		elname = tok_->current_token().value;
		if (tok_->next_token().type != ']')
			error(_("Unterminated [element] tag"));

		// Add the element
		current_element = &(elements.top().cfg->add_child(elname));
		elements.top().last_element_map[elname] = current_element;
		elements.push(element(current_element, elname, tok_->get_line()));
		break;

	case '+': // [+element]
		if (tok_->next_token().type != token::STRING)
			error(_("Invalid tag name"));
		elname = tok_->current_token().value;
		if (tok_->next_token().type != ']')
			error(_("Unterminated [+element] tag"));

		// Find the last child of the current element whose name is
		// element
		last_element_itor = elements.top().last_element_map.find(elname);
		if(last_element_itor == elements.top().last_element_map.end()) {
			current_element = &elements.top().cfg->add_child(elname);
		} else {
			current_element = last_element_itor->second;
		}
		elements.top().last_element_map[elname] = current_element;
		elements.push(element(current_element, elname, tok_->get_line()));
		break;

	case '/': // [/element]
		if(tok_->next_token().type != token::STRING)
			error(_("Invalid closing tag name"));
		elname = tok_->current_token().value;
		if(tok_->next_token().type != ']')
			error(_("Unterminated closing tag"));
		if(elements.size() <= 1)
			error(_("Unexpected closing tag"));
		if(elname != elements.top().name) {
			utils::string_map i18n_symbols;
			i18n_symbols["tag"] = elements.top().name;
			i18n_symbols["tag2"] = elname;
			error(lineno_string(i18n_symbols, elements.top().start_line,
					N_("Found invalid closing tag $tag2 for tag $tag (opened at $pos)")));
		}

		elements.pop();
		break;
	default:
		error(_("Invalid tag name"));
	}
}

void parser::parse_variable()
{
	config& cfg = *elements.top().cfg;
	std::vector<std::string> variables;
	variables.push_back("");

	while (tok_->current_token().type != '=') {
		switch(tok_->current_token().type) {
		case token::STRING:
			if(!variables.back().empty())
				variables.back() += ' ';
			variables.back() += tok_->current_token().value;
			break;
		case ',':
			if(variables.back().empty()) {
				error(_("Empty variable name"));
			} else {
				variables.push_back("");
			}
			break;
		default:
			error(_("Unexpected characters after variable name (expected , or =)"));
			break;
		}
		tok_->next_token();
	}
	if(variables.back().empty())
		error(_("Empty variable name"));

	{
		for(std::vector<std::string>::iterator curvar = variables.begin(); curvar != variables.end(); ++curvar) {
			cfg[*curvar] = "";
		}
	}

	std::vector<std::string>::const_iterator curvar = variables.begin();

	bool ignore_next_newlines = false;
	while(1) {
		tok_->next_token();
		wassert(curvar != variables.end());

		switch (tok_->current_token().type) {
		case ',':
			if ((curvar+1) != variables.end()) {
				curvar++;
				cfg[*curvar] = "";
				continue;
			} else {
				cfg[*curvar] += ",";
			}
			break;
		case '_':
			tok_->next_token();
			switch (tok_->current_token().type) {
			case token::UNTERMINATED_QSTRING:
				error(_("Unterminated quoted string"));
				break;
			case token::QSTRING:
				cfg[*curvar] += t_string(tok_->current_token().value, tok_->textdomain());
				break;
			default:
				cfg[*curvar] += "_";
				cfg[*curvar] += tok_->current_token().value;
				break;
			case token::END:
			case token::LF:
				return;
			}
			break;
		case '+':
			// Ignore this
			break;
		default:
			cfg[*curvar] += tok_->current_token().leading_spaces + tok_->current_token().value;
			break;
		case token::QSTRING:
			cfg[*curvar] += tok_->current_token().value;
			break;
		case token::UNTERMINATED_QSTRING:
			error(_("Unterminated quoted string"));
			break;
		case token::LF:
			if(!ignore_next_newlines)
				return;
			break;
		case token::END:
			return;
		}

		if (tok_->current_token().type == '+') {
			ignore_next_newlines = true;
		} else if (tok_->current_token().type != token::LF) {
			ignore_next_newlines = false;
		}
	}
}

std::string parser::lineno_string(utils::string_map &i18n_symbols, std::string const &lineno,
			          std::string const &error_string)
{
	std::vector< std::string > pos = utils::quoted_split(lineno, ' ');
	std::vector< std::string >::const_iterator i = pos.begin(), end = pos.end();
	std::string included_from = _(" included from ");
	std::string res;
	while (i != end) {
		std::string const &line = *(i++);
		std::string const &file = i != end ? *(i++) : "<unknown>";
		if (!res.empty())
			res += included_from;
		res += file + ':' + line;
	}
	if (res.empty()) res = "???";
	i18n_symbols["pos"] = res;
	return vgettext(error_string.c_str(), i18n_symbols);
}

void parser::error(const std::string& error_type)
{
	utils::string_map i18n_symbols;
	i18n_symbols["error"] = error_type;

	throw config::error(
		lineno_string(i18n_symbols, tok_->get_line(),
		              N_("$error at $pos")));
}

} // end anon namespace

void read(config &cfg, std::istream &in, std::string* error_log)
{
	parser(cfg, in)(error_log);
}

void read(config &cfg, std::string &in, std::string* error_log)
{
	parser(cfg, in)(error_log);
}

void read_gz(config &cfg, std::istream &file, std::string* error_log)
{

	boost::iostreams::filtering_stream<boost::iostreams::input> filter;
	filter.push(boost::iostreams::gzip_decompressor());
	filter.push(file);

	parser(cfg, filter)(error_log);
}

static char const *AttributeEquals = "=";

static char const *TranslatableAttributePrefix = "_ \"";
static char const *AttributePrefix = "\"";
static char const *AttributePostfix = "\"";

static char const* AttributeContPostfix = " + \n";
static char const* AttributeEndPostfix = "\n";

static char const* TextdomainPrefix = "#textdomain ";
static char const* TextdomainPostfix = "\n";

static char const *ElementPrefix = "[";
static char const *ElementPostfix = "]\n";
static char const *EndElementPrefix = "[/";
static char const *EndElementPostfix = "]\n";

static std::string escaped_string(const std::string& value) {
	std::vector<char> res;
	for(std::string::const_iterator i = value.begin(); i != value.end(); ++i) {
		// double interior quotes
		if(*i == '\"') res.push_back(*i);
		res.push_back(*i);
	}
	return std::string(res.begin(), res.end());
}

void write_key_val(std::ostream &out, const std::string &key, const t_string &value, unsigned int level, std::string& textdomain)
{
	bool first = true;
	if (value.empty()) {
		return;
	}

	for(t_string::walker w(value); !w.eos(); w.next()) {
		std::string part(w.begin(), w.end());

		if(w.translatable()) {
			if(w.textdomain() != textdomain) {
				out << TextdomainPrefix
					<< w.textdomain()
					<< TextdomainPostfix;
				textdomain = w.textdomain();
			}

			if(first) {
				out << std::string(level, '\t')
					<< key
					<< AttributeEquals;
			}

			out << TranslatableAttributePrefix
				<< escaped_string(part)
				<< AttributePostfix;

		} else {
			if(first) {
				out << std::string(level, '\t')
					<< key
					<< AttributeEquals;
			}

			out << AttributePrefix
				<< escaped_string(part)
				<< AttributePostfix;
		}

		if(w.last()) {
			out << AttributeEndPostfix;
		} else {
			out << AttributeContPostfix;
			out << std::string(level+1, '\t');
		}

		first = false;
	}
}

void write_open_child(std::ostream &out, const std::string &child, unsigned int level)
{
	out << std::string(level, '\t')
		<< ElementPrefix << child << ElementPostfix;
}

void write_close_child(std::ostream &out, const std::string &child, unsigned int level)
{
	out << std::string(level, '\t')
		<< EndElementPrefix << child << EndElementPostfix;
}

static void write_internal(config const &cfg, std::ostream &out, std::string& textdomain, size_t tab = 0)
{
	if (tab > max_recursion_levels)
		throw config::error("Too many recursion levels in config write");

	for(string_map::const_iterator i = cfg.values.begin(), i_end = cfg.values.end(); i != i_end; ++i) {
		write_key_val(out, i->first, i->second, tab, textdomain);
	}

	for(config::all_children_iterator j = cfg.ordered_begin(), j_end = cfg.ordered_end(); j != j_end; ++j) {
		const std::pair<const std::string*,const config*>& item = *j;
		const std::string& name = *item.first;
		const config& cfg = *item.second;

		write_open_child(out, name, tab);
		write_internal(cfg, out, textdomain, tab + 1);
		write_close_child(out, name, tab);
	}
}

void write(std::ostream &out, config const &cfg, unsigned int level)
{
	std::string textdomain = PACKAGE;
	write_internal(cfg, out, textdomain, level);
}

