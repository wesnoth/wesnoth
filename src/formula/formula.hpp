/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_HPP_INCLUDED
#define FORMULA_HPP_INCLUDED

#include "formula/debugger_fwd.hpp"
#include "formula/formula_fwd.hpp"
#include "formula/tokenizer.hpp"
#include "formula/variant.hpp"

namespace wfl
{
class formula_expression;
class function_symbol_table;

using expression_ptr = std::shared_ptr<formula_expression>;

// Namespace alias for shorter typing
namespace tk = tokenizer;

class formula
{
public:
	formula(const std::string& str, function_symbol_table* symbols = nullptr);
	formula(const tk::token* i1, const tk::token* i2, function_symbol_table* symbols = nullptr);

	~formula();

	static variant evaluate(
			const const_formula_ptr& f,
			const formula_callable& variables,
			formula_debugger* fdb = nullptr,
			variant default_res = variant(0))
	{
		if(f) {
			return f->evaluate(variables, fdb);
		} else {
			return default_res;
		}
	}

	static formula_ptr create_optional_formula(const std::string& str, function_symbol_table* symbols = nullptr);

	variant evaluate(const formula_callable& variables, formula_debugger* fdb = nullptr) const
	{
		if(fdb != nullptr) {
			return evaluate_formula_callback(*fdb, *this, variables);
		} else {
			return execute(variables, fdb);
		}
	}

	variant evaluate(formula_debugger* fdb = nullptr) const
	{
		if(fdb != nullptr) {
			return evaluate_formula_callback(*fdb,*this);
		} else {
			return execute(fdb);
		}
	}

	const std::string& str() const { return str_; }

	static const char* const id_chars;

private:
	variant execute(const formula_callable& variables, formula_debugger* fdb = nullptr) const;
	variant execute(formula_debugger* fdb) const;

	expression_ptr expr_;
	std::string str_;
	function_symbol_table* symbols_;
	bool managing_symbols;

	friend class formula_debugger;
};

struct formula_error : public game::error
{
	formula_error()
		: error()
		, type()
		, formula()
		, filename()
		, line(0)
	{}

	formula_error(const std::string& type, const std::string& formula,
			const std::string& file, int line);

	~formula_error() NOEXCEPT {}

	std::string type;
	std::string formula;
	std::string filename;
	int line;
};

} // namespace wfl

#endif
