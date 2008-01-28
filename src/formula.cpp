
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <iostream>
#include <vector>

#include "foreach.hpp"
#include "formula.hpp"
#include "formula_callable.hpp"
#include "formula_function.hpp"
#include "formula_tokenizer.hpp"
#include "map_utils.hpp"

namespace game_logic
{

void formula_callable::set_value(const std::string& key, const variant& value)
{
	std::cerr << "ERROR: cannot set key '" << key << "' on object\n";
}

map_formula_callable::map_formula_callable(
    const formula_callable* fallback) : fallback_(fallback)
{}

map_formula_callable& map_formula_callable::add(const std::string& key,
                                                const variant& value)
{
	values_[key] = value;
	return *this;
}

variant map_formula_callable::get_value(const std::string& key) const
{
	return map_get_value_default(values_, key,
	        fallback_ ? fallback_->query_value(key) : variant(0));
}

void map_formula_callable::get_inputs(std::vector<formula_input>* inputs) const
{
	if(fallback_) {
		fallback_->get_inputs(inputs);
	}
	for(std::map<std::string,variant>::const_iterator i = values_.begin(); i != values_.end(); ++i) {
		inputs->push_back(formula_input(i->first, FORMULA_READ_ONLY));
	}
}

namespace {

class list_expression : public formula_expression {
public:
	explicit list_expression(const std::vector<expression_ptr>& items)
	   : items_(items)
	{}

private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> res;
		res.reserve(items_.size());
		for(std::vector<expression_ptr>::const_iterator i = items_.begin(); i != items_.end(); ++i) {
			res.push_back((*i)->evaluate(variables));
		}

		return variant(&res);
	}

	std::vector<expression_ptr> items_;
};

class unary_operator_expression : public formula_expression {
public:
	unary_operator_expression(const std::string& op, expression_ptr arg)
	  : operand_(arg)
	{
		if(op == "not") {
			op_ = NOT;
		} else if(op == "-") {
			op_ = SUB;
		} else {
			std::cerr << "illegal unary operator: '" << op << "'\n";
			throw formula_error();
		}
	}
private:
	variant execute(const formula_callable& variables) const {
		const variant res = operand_->evaluate(variables);
		switch(op_) {
		case NOT: return res.as_bool() ? variant(0) : variant(1);
		case SUB: return -res;
		default: assert(false);
		}
	}
	enum OP { NOT, SUB };
	OP op_;
	expression_ptr operand_;
};

class dot_expression : public formula_expression {
public:
	dot_expression(expression_ptr left, expression_ptr right)
	   : left_(left), right_(right)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const variant left = left_->evaluate(variables);
		if(!left.is_callable()) {
			if(left.is_list()) {
				const variant index = right_->evaluate(variables);
				return left[index.as_int()];
			}

			return left;
		}

		return right_->evaluate(*left.as_callable());
	}

	expression_ptr left_, right_;
};

class operator_expression : public formula_expression {
public:
	operator_expression(const std::string& op, expression_ptr left,
	                             expression_ptr right)
	  : op_(OP(op[0])), left_(left), right_(right)
	{
		if(op == ">=") {
			op_ = GTE;
		} else if(op == "<=") {
			op_ = LTE;
		} else if(op == "!=") {
			op_ = NEQ;
		} else if(op == "and") {
			op_ = AND;
		} else if(op == "or") {
			op_ = OR;
		}
	}

private:
	variant execute(const formula_callable& variables) const {
		const variant left = left_->evaluate(variables);
		const variant right = right_->evaluate(variables);
		switch(op_) {
		case AND: return left.as_bool() && right.as_bool() ? variant(1) : variant(0);
		case OR: return left.as_bool() || right.as_bool() ? variant(1) : variant(0);
		case ADD: return left + right;
		case SUB: return left - right;
		case MUL: return left * right;
		case DIV: return left / right;
		case POW: return left ^ right;
		case EQ:  return left == right ? variant(1) : variant(0);
		case NEQ: return left != right ? variant(1) : variant(0);
		case LTE: return left <= right ? variant(1) : variant(0);
		case GTE: return left >= right ? variant(1) : variant(0);
		case LT:  return left < right ? variant(1) : variant(0);
		case GT:  return left > right ? variant(1) : variant(0);
		case MOD: return left % right;
		case DICE:return variant(dice_roll(left.as_int(), right.as_int()));
		default: assert(false);
		}
	}

	static int dice_roll(int num_rolls, int faces) {
		int res = 0;
		while(faces > 0 && num_rolls-- > 0) {
			res += (rand()%faces)+1;
		}
		return res;
	}

	enum OP { AND, OR, NEQ, LTE, GTE, GT='>', LT='<', EQ='=',
	          ADD='+', SUB='-', MUL='*', DIV='/', DICE='d', POW='^', MOD='%' };

	OP op_;
	expression_ptr left_, right_;
};

typedef std::map<std::string,expression_ptr> expr_table;
typedef boost::shared_ptr<expr_table> expr_table_ptr;

class where_variables: public formula_callable {
public:
	where_variables(const formula_callable &base,
			expr_table_ptr table )
		: base_(base), table_(table) { }
private:
	const formula_callable& base_;
	expr_table_ptr table_;

	void get_inputs(std::vector<formula_input>* inputs) const {
		for(expr_table::const_iterator i = table_->begin(); i != table_->end(); ++i) {
			inputs->push_back(formula_input(i->first, FORMULA_READ_ONLY));
		}
	}

	variant get_value(const std::string& key) const {
		expr_table::iterator i = table_->find(key);
		if(i != table_->end()) {
			return i->second->evaluate(base_);
		}
		return base_.query_value(key);
	}
};

class where_expression: public formula_expression {
public:
	explicit where_expression(expression_ptr body,
				  expr_table_ptr clauses)
		: body_(body), clauses_(clauses)
	{}

private:
	expression_ptr body_;
	expr_table_ptr clauses_;

	variant execute(const formula_callable& variables) const {
		where_variables wrapped_variables(variables, clauses_);
		return body_->evaluate(wrapped_variables);
	}
};


class identifier_expression : public formula_expression {
public:
	explicit identifier_expression(const std::string& id) : id_(id)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variables.query_value(id_);
	}
	std::string id_;
};

class integer_expression : public formula_expression {
public:
	explicit integer_expression(int i) : i_(i)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(i_);
	}

	int i_;
};

class string_expression : public formula_expression {
public:
	explicit string_expression(std::string str)
	{
		std::string::iterator i;
		while((i = std::find(str.begin(), str.end(), '{')) != str.end()) {
			std::string::iterator j = std::find(i, str.end(), '}');
			if(j == str.end()) {
				break;
			}

			const std::string formula_str(i+1, j);
			const int pos = i - str.begin();
			str.erase(i, j+1);

			substitution sub;
			sub.pos = pos;
			sub.calculation.reset(new formula(formula_str));
			subs_.push_back(sub);
		}

		std::reverse(subs_.begin(), subs_.end());

		str_ = variant(str);
	}
private:
	variant execute(const formula_callable& variables) const {
		if(subs_.empty()) {
			return str_;
		} else {
			std::string res = str_.as_string();
			foreach(const substitution& sub, subs_) {
				const std::string str = sub.calculation->execute(variables).string_cast();
				res.insert(sub.pos, str);
			}

			return variant(res);
		}
	}

	struct substitution {
		int pos;
		const_formula_ptr calculation;
	};

	variant str_;
	std::vector<substitution> subs_;
};

using namespace formula_tokenizer;
int operator_precedence(const token& t)
{
	static std::map<std::string,int> precedence_map;
	if(precedence_map.empty()) {
		int n = 0;
		precedence_map["not"] = ++n;
		precedence_map["where"] = ++n;
		precedence_map["or"]    = ++n;
		precedence_map["and"]   = ++n;
		precedence_map["="]     = ++n;
		precedence_map["!="]    = n;
		precedence_map["<"]     = n;
		precedence_map[">"]     = n;
		precedence_map["<="]    = n;
		precedence_map[">="]    = n;
		precedence_map["+"]     = ++n;
		precedence_map["-"]     = n;
		precedence_map["*"]     = ++n;
		precedence_map["/"]     = ++n;
		precedence_map["%"]     = ++n;
		precedence_map["^"]     = ++n;
		precedence_map["d"]     = ++n;
		precedence_map["."]     = ++n;
	}

	assert(precedence_map.count(std::string(t.begin,t.end)));
	return precedence_map[std::string(t.begin,t.end)];
}

expression_ptr parse_expression(const token* i1, const token* i2, const function_symbol_table* symbols);

void parse_args(const token* i1, const token* i2,
                std::vector<expression_ptr>* res,
				const function_symbol_table* symbols)
{
	int parens = 0;
	const token* beg = i1;
	while(i1 != i2) {
		if(i1->type == TOKEN_LPARENS || i1->type == TOKEN_LSQUARE) {
			++parens;
		} else if(i1->type == TOKEN_RPARENS || i1->type == TOKEN_RSQUARE) {
			--parens;
		} else if(i1->type == TOKEN_COMMA && !parens) {
			res->push_back(parse_expression(beg,i1, symbols));
			beg = i1+1;
		}

		++i1;
	}

	if(beg != i1) {
		res->push_back(parse_expression(beg,i1, symbols));
	}
}

void parse_where_clauses(const token* i1, const token * i2,
			             expr_table_ptr res, const function_symbol_table* symbols) {
	int parens = 0;
	const token *original_i1_cached = i1;
	const token *beg = i1;
	std::string var_name;
	while(i1 != i2) {
		if(i1->type == TOKEN_LPARENS) {
			++parens;
		} else if(i1->type == TOKEN_RPARENS) {
			--parens;
		} else if(!parens) {
			if(i1->type == TOKEN_COMMA) {
				if(var_name.empty()) {
					std::cerr << "There is 'where <expression>,; "
						  << "'where name=<expression>,' was needed.\n";
					throw formula_error();
				}
				(*res)[var_name] = parse_expression(beg,i1, symbols);
				beg = i1+1;
				var_name.clear();
			} else if(i1->type == TOKEN_OPERATOR) {
				std::string op_name(i1->begin, i1->end);
				if(op_name == "=") {
					if(beg->type != TOKEN_IDENTIFIER) {
						if(i1 == original_i1_cached) {
							std::cerr<< "There is 'where =<expression'; "
								 << "'where name=<expression>' was needed.\n";
						} else {
							std::cerr<< "There is 'where <expression>=<expression>'; "
								 << "'where name=<expression>' was needed.\n";
						}
						throw formula_error();
					} else if(beg+1 != i1) {
						std::cerr<<"There is 'where name <expression>=<expression>'; "
							 << "'where name=<expression>' was needed.\n";
						throw formula_error();
					} else if(!var_name.empty()) {
						std::cerr<<"There is 'where name=name=<expression>'; "
							 <<"'where name=<expression>' was needed.\n";
						throw formula_error();
					}
					var_name.insert(var_name.end(), beg->begin, beg->end);
					beg = i1+1;
				}
			}
		}
		++i1;
	}
	if(beg != i1) {
		if(var_name.empty()) {
			std::cerr << "There is 'where <expression>'; "
				  << "'where name=<expression> was needed.\n";
			throw formula_error();
		}
		(*res)[var_name] = parse_expression(beg,i1, symbols);
	}
}

expression_ptr parse_expression(const token* i1, const token* i2, const function_symbol_table* symbols)
{
	if(i1 == i2) {
		std::cerr << "empty expression\n";
		throw formula_error();
	}

	int parens = 0;
	const token* op = NULL;
	for(const token* i = i1; i != i2; ++i) {
		if(i->type == TOKEN_LPARENS || i->type == TOKEN_LSQUARE) {
			++parens;
		} else if(i->type == TOKEN_RPARENS || i->type == TOKEN_RSQUARE) {
			--parens;
		} else if(parens == 0 && i->type == TOKEN_OPERATOR) {
			if(op == NULL || operator_precedence(*op) >
			                 operator_precedence(*i)) {
				op = i;
			}
		}
	}

	if(op == NULL) {
		if(i1->type == TOKEN_LPARENS && (i2-1)->type == TOKEN_RPARENS) {
			return parse_expression(i1+1,i2-1,symbols);
		} else if(i1->type == TOKEN_LSQUARE && (i2-1)->type == TOKEN_RSQUARE) {
				std::vector<expression_ptr> args;
				parse_args(i1+1,i2-1,&args,symbols);
				return expression_ptr(new list_expression(args));
		} else if(i2 - i1 == 1) {
			if(i1->type == TOKEN_IDENTIFIER) {
				return expression_ptr(new identifier_expression(
				                 std::string(i1->begin,i1->end)));
			} else if(i1->type == TOKEN_INTEGER) {
				int n = boost::lexical_cast<int>(std::string(i1->begin,i1->end));
				return expression_ptr(new integer_expression(n));
			} else if(i1->type == TOKEN_STRING_LITERAL) {
				return expression_ptr(new string_expression(std::string(i1->begin+1,i1->end-1)));
			}
		} else if(i1->type == TOKEN_IDENTIFIER &&
		          (i1+1)->type == TOKEN_LPARENS &&
				  (i2-1)->type == TOKEN_RPARENS) {
			int nleft = 0, nright = 0;
			for(const token* i = i1; i != i2; ++i) {
				if(i->type == TOKEN_LPARENS) {
					++nleft;
				} else if(i->type == TOKEN_RPARENS) {
					++nright;
				}
			}

			if(nleft == nright) {
				std::vector<expression_ptr> args;
				parse_args(i1+2,i2-1,&args,symbols);
				return expression_ptr(
				  create_function(std::string(i1->begin,i1->end),args,symbols));
			}
		}

		std::ostringstream expr;
		while(i1 != i2) {
			expr << std::string(i1->begin,i1->end);
			++i1;
		}
		std::cerr << "could not parse expression: '" << expr.str() << "'\n";
		throw formula_error();
	}

	if(op == i1) {
		return expression_ptr(new unary_operator_expression(
		                         std::string(op->begin,op->end),
								 parse_expression(op+1,i2,symbols)));
	}

	const std::string op_name(op->begin,op->end);

	if(op_name == ".") {
		return expression_ptr(new dot_expression(
							     parse_expression(i1,op,symbols),
								 parse_expression(op+1,i2,symbols)));
	}

	if(op_name == "where") {
		expr_table_ptr table(new expr_table());
		parse_where_clauses(op+1, i2, table, symbols);
		return expression_ptr(new where_expression(parse_expression(i1, op, symbols),
							   table));
	}

	return expression_ptr(new operator_expression(
							     op_name, parse_expression(i1,op,symbols),
								 parse_expression(op+1,i2,symbols)));
}

}

formula_ptr formula::create_string_formula(const std::string& str)
{
	formula_ptr res(new formula());
	res->expr_.reset(new string_expression(str));
	return res;
}

formula_ptr formula::create_optional_formula(const std::string& str, const function_symbol_table* symbols)
{
	if(str.empty()) {
		return formula_ptr();
	}

	try {
		return formula_ptr(new formula(str, symbols));
	} catch(...) {
		std::cerr << "ERROR parsing optional formula: '" << str << "'\n";
		return formula_ptr();
	}
}

formula::formula(const std::string& str, const function_symbol_table* symbols) : str_(str)
{
	using namespace formula_tokenizer;

	std::vector<token> tokens;
	std::string::const_iterator i1 = str.begin(), i2 = str.end();
	while(i1 != i2) {
		try {
			tokens.push_back(get_token(i1,i2));
			if(tokens.back().type == TOKEN_WHITESPACE) {
				tokens.pop_back();
			}
		} catch(token_error& e) {
			throw formula_error();
		}
	}

	try {
		expr_ = parse_expression(&tokens[0],&tokens[0] + tokens.size(), symbols);
	} catch(...) {
		std::cerr << "error parsing formula '" << str << "'\n";
		throw;
	}
}

variant formula::execute(const formula_callable& variables) const
{
	try {
		return expr_->evaluate(variables);
	} catch(type_error& e) {
		std::cerr << "formula type error: " << e.message << "\n";
		return variant();
	}
}

variant formula::execute() const
{
	static map_formula_callable null_callable;
	return execute(null_callable);
}

}

#ifdef UNIT_TEST_FORMULA
using namespace game_logic;
class mock_char : public formula_callable {
	variant get_value(const std::string& key) const {
		if(key == "strength") {
			return variant(15);
		} else if(key == "agility") {
			return variant(12);
		}

		return variant(10);
	}
};
class mock_party : public formula_callable {
	variant get_value(const std::string& key) const {
		if(key == "members") {
			i_[0].add("strength",variant(12));
			i_[1].add("strength",variant(16));
			i_[2].add("strength",variant(14));
			std::vector<variant> members;
			for(int n = 0; n != 3; ++n) {
				members.push_back(variant(&i_[n]));
			}

			return variant(&members);
		} else if(key == "char") {
			return variant(&c_);
		} else {
			return variant(0);
		}
	}

	mock_char c_;
	mutable map_formula_callable i_[3];

};

#include <time.h>

int main()
{
	srand(time(NULL));
	mock_char c;
	mock_party p;
	try {
		assert(formula("strength").execute(c).as_int() == 15);
		assert(formula("17").execute(c).as_int() == 17);
		assert(formula("strength/2 + agility").execute(c).as_int() == 19);
		assert(formula("(strength+agility)/2").execute(c).as_int() == 13);
		assert(formula("strength > 12").execute(c).as_int() == 1);
		assert(formula("strength > 18").execute(c).as_int() == 0);
		assert(formula("if(strength > 12, 7, 2)").execute(c).as_int() == 7);
		assert(formula("if(strength > 18, 7, 2)").execute(c).as_int() == 2);
		assert(formula("2 and 1").execute(c).as_int() == 1);
		assert(formula("2 and 0").execute(c).as_int() == 0);
		assert(formula("2 or 0").execute(c).as_int() == 1);
		assert(formula("-5").execute(c).as_int() == -5);
		assert(formula("not 5").execute(c).as_int() == 0);
		assert(formula("not 0").execute(c).as_int() == 1);
		assert(formula("abs(5)").execute(c).as_int() == 5);
		assert(formula("abs(-5)").execute(c).as_int() == 5);
		assert(formula("min(3,5)").execute(c).as_int() == 3);
		assert(formula("min(5,2)").execute(c).as_int() == 2);
		assert(formula("max(3,5)").execute(c).as_int() == 5);
		assert(formula("max(5,2)").execute(c).as_int() == 5);
		assert(formula("max(4,5,[2,18,7])").execute(c).as_int() == 18);
		assert(formula("char.strength").execute(p).as_int() == 15);
		assert(formula("choose(members,strength).strength").execute(p).as_int() == 16);
		assert(formula("4^2").execute().as_int() == 16);
		assert(formula("2+3^3").execute().as_int() == 29);
		assert(formula("2*3^3+2").execute().as_int() == 56);
		assert(formula("9^3").execute().as_int() == 729);
		assert(formula("x*5 where x=1").execute().as_int() == 5);
		assert(formula("x*(a*b where a=2,b=1) where x=5").execute().as_int() == 10);
		assert(formula("char.strength * ability where ability=3").execute(p).as_int() == 45);
		assert(formula("'abcd' = 'abcd'").execute(p).as_bool() == true);
		assert(formula("'abcd' = 'acd'").execute(p).as_bool() == false);
		assert(formula("'strength, agility: {strength}, {agility}'").execute(c).as_string() ==
		               "strength, agility: 15, 12");
		const int dice_roll = formula("3d6").execute().as_int();
		assert(dice_roll >= 3 && dice_roll <= 18);

		assert(formula::create_string_formula("Your strength is {strength}")->execute(c).as_string() ==
						"Your strength is 15");
		variant myarray = formula("[1,2,3]").execute();
		assert(myarray.num_elements() == 3);
		assert(myarray[0].as_int() == 1);
		assert(myarray[1].as_int() == 2);
		assert(myarray[2].as_int() == 3);
	} catch(formula_error& e) {
		std::cerr << "parse error\n";
	}
}
#endif
