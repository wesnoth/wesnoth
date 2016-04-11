/*
   Copyright (C) 2016 by Ján Dugáček
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

#include "context_free_grammar_generator.hpp"
#include "../log.hpp"
#include "../random_new.hpp"

context_free_grammar_generator::context_free_grammar_generator() :
	initialized_(false)
{

}

context_free_grammar_generator::~context_free_grammar_generator()
{

}

bool context_free_grammar_generator::constructFromString(const std::string &source) {
	const char* reading = source.c_str();
	nonterminal* current = nullptr;
	std::vector<std::string>* filled = nullptr;
	std::string buf;

	while (*reading != 0) {
		if (*reading == '=') {
			current = &nonterminals_[buf];
			current->possibilities_.push_back(std::vector<std::string>());
			filled = &current->possibilities_.back();
			buf.clear();
		} else if (*reading == '\n') {
			if (filled) filled->push_back(buf);
			filled = nullptr;
			current = nullptr;
			buf.clear();
		} else if (*reading == '|') {
			if (!filled || !current) {
				lg::wml_error() << "[context_free_grammar_generator] Parsing error: misplaced | symbol";
				return false;
			}
			filled->push_back(buf);
			current->possibilities_.push_back(std::vector<std::string>());
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
					lg::wml_error() << "[context_free_grammar_generator] Parsing error: misplaced { symbol";
					return false;
				}
				filled->push_back(buf);
				buf.clear();
			}
			if (*reading == '}') {
				if (!filled) {
					lg::wml_error() << "[context_free_grammar_generator] Parsing error: misplaced } symbol";
					return false;
				}
				filled->push_back(buf);
				buf.clear();
			} else buf.push_back(*reading);
		}
		reading++;
	}
	if (filled) filled->push_back(buf);

	initialized_ = true;
	return true;
}

std::string context_free_grammar_generator::print_nonterminal(const std::string& name, uint32_t* seed, short seed_pos) const {
	std::string result;
	std::map<std::string, nonterminal>::const_iterator found = nonterminals_.find(name);
	if (found == nonterminals_.end()) {
		lg::wml_error() << "[context_free_grammar_generator] Warning: needed nonterminal " << name << " not defined";
		return "!" + name;
	}
	const context_free_grammar_generator::nonterminal& got = found->second;
	unsigned int picked = seed[seed_pos++] % got.possibilities_.size();
	if (seed_pos >= seed_size) seed_pos = 0;
	if (picked == got.last_) {
		picked = seed[seed_pos++] % got.possibilities_.size();
		if (seed_pos >= seed_size) seed_pos = 0;
	}
	const_cast<unsigned int&>(got.last_) = picked; /* The variable last_ can change, the rest must stay const */
	const std::vector<std::string>& used = got.possibilities_[picked];
	for (unsigned int i = 0; i < used.size(); i++) {
		if (used[i][0] == '{') result += print_nonterminal(used[i].substr(1), seed, seed_pos);
		else result += used[i];
	}
	return result;
}

std::string context_free_grammar_generator::generate() const {
	uint32_t seed[seed_size];
	for (unsigned short int i = 0; i < seed_size; i++) {
		seed[i] = random_new::generator->next_random();
	}
	return print_nonterminal("main", seed, 0);
}
