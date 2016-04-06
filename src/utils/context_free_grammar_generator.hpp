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

#ifndef CONTEXT_FREE_GRAMMAR_GENERATOR_INCLUDED
#define CONTEXT_FREE_GRAMMAR_GENERATOR_INCLUDED

#include <string>
#include <map>
#include <list>
#include <vector>

class context_free_grammar_generator
{
private:

	struct nonterminal {
		nonterminal() : last_(1) {}
		std::vector<std::vector<std::string> > possibilities_;
		unsigned int last_;
	};

	std::map<std::string, nonterminal> nonterminals_;
	bool initialized_;
	std::string print_nonterminal(const std::string& name, uint32_t* seed, short int seed_pos) const;
	static const short unsigned int seed_size = 20;

public:
	/** Default constructor */
	context_free_grammar_generator();

	/** Initialisation
	 * @param source the definition of the context-free grammar to use
	 * @returns if the operation was successful
	 */
	bool constructFromString(const std::string& source);

	/** Generates a possible word in the grammar set before
	 * @returns the word
	 */
	std::string generate() const;

	~context_free_grammar_generator();

	/** Checks if the object is initialized
	 * @returns if it is initialized
	 */
	bool is_initialized() const {return initialized_; }
};

#endif
