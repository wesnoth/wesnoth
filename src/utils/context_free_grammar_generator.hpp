/*
   Copyright (C) 2016 - 2018 by Ján Dugáček
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "utils/name_generator.hpp"

#include <list>
#include <vector>
#include <cstdint>

class context_free_grammar_generator : public name_generator
{
private:

	struct nonterminal {
		nonterminal() : last_(1) {}
		std::vector<std::vector<std::string> > possibilities_;
		mutable unsigned int last_;
	};

	void init_seed(uint32_t seed[]) const;
	std::map<std::string, nonterminal> nonterminals_;
	std::string print_nonterminal(const std::string& name, uint32_t seed[], short int seed_pos) const;
	static const short unsigned int seed_size = 20;

public:
	/** Initialisation
	 * @param source the definition of the context-free grammar to use
	 */
	context_free_grammar_generator(const std::string& source);

	/** Initialisation
	 * @param source A map of nonterminals to lists of possibilities
	 */
	context_free_grammar_generator(const std::map<std::string, std::vector<std::string>>& source);

	/** Generates a possible word in the grammar set before
	 * @return the word
	 */
	std::string generate() const override;

	~context_free_grammar_generator();
};
