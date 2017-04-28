/*
   Copyright (C) 2016 - 2017 by Ján Dugáček
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Algorithm to generate names using a context-free grammar, which allows more control
 *  than the usual Markov chain generator
 */

#include "utils/context_free_grammar_generator.hpp"

#include "log.hpp"
#include "random.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>

#include <boost/algorithm/string.hpp>

context_free_grammar_generator::~context_free_grammar_generator()
{
}

context_free_grammar_generator::context_free_grammar_generator(const std::string& source)
{
	const char* reading = source.c_str();
	nonterminal* current = nullptr;
	std::vector<std::string>* filled = nullptr;
	std::string buf;

	while (*reading != 0) {
		if (*reading == '=') {
			// Leading and trailing whitespace is not significant, but internal whitespace is
			std::string key = boost::trim_copy(buf);
			if(key == "!" || key =="(" || key == ")") {
				throw name_generator_invalid_exception("[context_free_grammar_generator] Parsing error: nonterminals (, ! and ) may not be overridden");
			}
			current = &nonterminals_[key];
			current->possibilities_.emplace_back();
			filled = &current->possibilities_.back();
			buf.clear();
		} else if (*reading == '\n') {
			// All whitespace is significant after the =
			if (filled) filled->push_back(buf);
			filled = nullptr;
			current = nullptr;
			buf.clear();
		} else if (*reading == '|') {
			if (!filled || !current) {
				throw name_generator_invalid_exception("[context_free_grammar_generator] Parsing error: misplaced | symbol");
			}
			filled->push_back(buf);
			current->possibilities_.emplace_back();
			filled = &current->possibilities_.back();
			buf.clear();
		} else if (*reading == '\\' && reading[1] == 'n') {
			reading++;
			buf.push_back('\n');
		} else if (*reading == '\\' && reading[1] == 't') {
			reading++;
			buf.push_back('\t');
		} else {
			if (*reading == '{') {
				if (!filled) {
					throw name_generator_invalid_exception("[context_free_grammar_generator] Parsing error: misplaced { symbol");
				}
				filled->push_back(buf);
				buf.clear();
			}
			else if (*reading == '}') {
				if (!filled) {
					throw name_generator_invalid_exception("[context_free_grammar_generator] Parsing error: misplaced } symbol");
				}
				filled->push_back('{' + boost::trim_copy(buf));
				buf.clear();
			} else buf.push_back(*reading);
		}
		reading++;
	}
	if (filled) filled->push_back(buf);
}

context_free_grammar_generator::context_free_grammar_generator(const std::map<std::string, std::vector<std::string>>& source)
{
	for(auto rule : source) {
		std::string key = rule.first; // Need to do this because utils::strip is mutating
		boost::trim(key);
		if(key == "!" || key =="(" || key == ")") {
			throw name_generator_invalid_exception("[context_free_grammar_generator] Parsing error: nonterminals (, ! and ) may not be overridden");
		}
		for(std::string str : rule.second) {
			nonterminals_[key].possibilities_.emplace_back();
			std::vector<std::string>* filled = &nonterminals_[key].possibilities_.back();
			std::string buf;
			// A little code duplication here...
			for(char c : str) {
				if (c == '{') {
					if (!filled) {
						throw  name_generator_invalid_exception("[context_free_grammar_generator] Parsing error: misplaced { symbol");
					}
					filled->push_back(buf);
					buf.clear();
				}
				else if (c == '}') {
					if (!filled) {
						throw  name_generator_invalid_exception("[context_free_grammar_generator] Parsing error: misplaced } symbol");
					}

					filled->push_back('{' + boost::trim_copy(buf));
					buf.clear();
				} else buf.push_back(c);
			}
			if(!buf.empty()) {
				filled->push_back(buf);
			}
		}
	}
}

std::string context_free_grammar_generator::print_nonterminal(const std::string& name, uint32_t* seed, short seed_pos) const {
	if (name == "!") {
		return "|";
	}
	else if (name == "(" ) {
		return "{";
	}
	else if (name == ")" ) {
		return "}";
	}
	else {
		std::string result = "";

		std::map<std::string,nonterminal>::const_iterator found = nonterminals_.find(name);
		if (found == nonterminals_.end()) {
			lg::wml_error() << "[context_free_grammar_generator] Warning: needed nonterminal" << name << " not defined";
			return "!" + name;
		}
		const context_free_grammar_generator::nonterminal& got = found->second;
		unsigned int picked = seed[seed_pos++] % got.possibilities_.size();
		if (seed_pos >= seed_size) seed_pos = 0;
		if (picked == got.last_) {
			picked = seed[seed_pos++] % got.possibilities_.size();
			if (seed_pos >= seed_size) seed_pos = 0;
		}
		got.last_ = picked;
		const std::vector<std::string>& used = got.possibilities_[picked];
		for (unsigned int i = 0; i < used.size(); i++) {
			if (used[i][0] == '{') result += print_nonterminal(used[i].substr(1), seed, seed_pos);
			else result += used[i];
		}
		return result;
	}
}

std::string context_free_grammar_generator::generate() const {
	uint32_t seed[seed_size];
	init_seed(seed);
	return print_nonterminal("main", seed, 0);
}

void context_free_grammar_generator::init_seed(uint32_t seed[]) const {
	for (unsigned short int i = 0; i < seed_size; i++) {
		seed[i] = randomness::generator->next_random();
	}
}
