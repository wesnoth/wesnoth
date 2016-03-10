/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "global.hpp"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <set>
#include <sstream>

#include "formula_callable.hpp"
#include "formula_function.hpp"
#include "map_utils.hpp"
#include "random_new.hpp"

#include <boost/foreach.hpp>

namespace game_logic
{

void formula_callable::set_value(const std::string& key, const variant& /*value*/)
{
	std::cerr << "ERROR: cannot set key '" << key << "' on object" << std::endl;
}


map_formula_callable::map_formula_callable(const formula_callable* fallback) :
	formula_callable(false),
	values_(),
	fallback_(fallback)
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
	        fallback_ ? fallback_->query_value(key) : variant());
}

void map_formula_callable::get_inputs(std::vector<formula_input>* inputs) const
{
	if(fallback_) {
		fallback_->get_inputs(inputs);
	}
	for(std::map<std::string,variant>::const_iterator i = values_.begin(); i != values_.end(); ++i) {
		inputs->push_back(formula_input(i->first, FORMULA_READ_WRITE));
	}
}

void map_formula_callable::set_value(const std::string& key, const variant& value)
{
	values_[key] = value;
}

namespace {

class function_list_expression : public formula_expression {
public:
	explicit function_list_expression(function_symbol_table *symbols)
		: symbols_(symbols)
	{}

	virtual std::string str() const
	{
		return "{function_list_expression()}";
	}
private:
	variant execute(const formula_callable& /*variables*/, formula_debugger * /*fdb*/) const {
		std::vector<variant> res;
		std::vector<std::string> function_names = builtin_function_names();
		std::vector<std::string> more_function_names = symbols_->get_function_names();
		function_names.insert(function_names.end(), more_function_names.begin(), more_function_names.end());
		for(size_t i = 0; i < function_names.size(); i++) {
			res.push_back(variant(function_names[i]));
		}
		return variant(&res);
	}

	function_symbol_table* symbols_;
};

class list_expression : public formula_expression {
public:
	explicit list_expression(const std::vector<expression_ptr>& items)
	   : items_(items)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::vector<variant> res;
		res.reserve(items_.size());
		for(std::vector<expression_ptr>::const_iterator i = items_.begin(); i != items_.end(); ++i) {
			res.push_back((*i)->evaluate(variables,fdb));
		}

		return variant(&res);
	}

	std::vector<expression_ptr> items_;

	std::string str() const
	{
		std::stringstream s;
		s << '[';
		bool first_item = true;
		BOOST_FOREACH(expression_ptr a , items_) {
			if (!first_item) {
				s << ',';
			} else {
				first_item = false;
			}
			s << a->str();
		}
		s << ']';
		return s.str();
	}


};

class map_expression : public formula_expression {
public:
	explicit map_expression(const std::vector<expression_ptr>& items)
	   : items_(items)
	{}

	virtual std::string str() const
	{
		std::stringstream s;
		s << " [";
		for(std::vector<expression_ptr>::const_iterator i = items_.begin(); ( i != items_.end() ) && ( i+1 != items_.end() ) ; i+=2) {
			s << (*i)->str();
			s << " -> ";
			s << (*(i+1))->str();
		}
		s << " ]";
		return s.str();
	}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::map<variant,variant> res;
		for(std::vector<expression_ptr>::const_iterator i = items_.begin(); ( i != items_.end() ) && ( i+1 != items_.end() ) ; i+=2) {
			variant key = (*i)->evaluate(variables,fdb);
			variant value = (*(i+1))->evaluate(variables,fdb);
			res[ key ] = value;
		}

		return variant(&res);
	}

	std::vector<expression_ptr> items_;
};

class unary_operator_expression : public formula_expression {
public:
	unary_operator_expression(const std::string& op, expression_ptr arg) :
		op_(),op_str_(op),
		operand_(arg)
	{
		if(op == "not") {
			op_ = NOT;
		} else if(op == "-") {
			op_ = SUB;
		} else {
			throw formula_error("Illegal unary operator: '" + op + "'" , "", "", 0);
		}
	}

	virtual std::string str() const {
		std::stringstream s;
		s << op_str_ << '('<< operand_->str() << ')';
		return s.str();
	}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant res = operand_->evaluate(variables,fdb);
		switch(op_) {
		case NOT:
			return res.as_bool() ? variant(0) : variant(1);
		case SUB:
		default:
			return -res;
		}
	}
	enum OP { NOT, SUB };
	OP op_;
	std::string op_str_;
	expression_ptr operand_;
};

class list_callable : public formula_callable {
	variant list_;
public:
	explicit list_callable(const variant& list) : list_(list)
	{}

	void get_inputs(std::vector<formula_input>* inputs) const {
		inputs->push_back(formula_input("size", FORMULA_READ_WRITE));
		inputs->push_back(formula_input("empty", FORMULA_READ_WRITE));
		inputs->push_back(formula_input("first", FORMULA_READ_WRITE));
		inputs->push_back(formula_input("last", FORMULA_READ_WRITE));
	}

	variant get_value(const std::string& key) const {
		if(key == "size") {
			return variant(list_.num_elements());
		} else if(key == "empty") {
			return variant(list_.num_elements() == 0);
		} else if(key == "first") {
			if(list_.num_elements() > 0) {
				return list_[0];
			} else {
				return variant();
			}
		} else if(key == "last") {
			if(list_.num_elements() > 0) {
				return list_[list_.num_elements()-1];
			} else {
				return variant();
			}
		} else {
			return variant();
		}
	}

};

class dot_callable : public formula_callable {
public:
	dot_callable(const formula_callable &global,
			const formula_callable& local)
		: global_(global), local_(local) { }
private:
	const formula_callable& global_, &local_;

	void get_inputs(std::vector<formula_input>* inputs) const {
		return local_.get_inputs(inputs);
	}

	variant get_value(const std::string& key) const {
		variant v = local_.query_value(key);

		if ( v == variant() )
			return global_.query_value(key);
		else
			return v;
	}
};

class dot_expression : public formula_expression {
public:
	dot_expression(expression_ptr left, expression_ptr right)
	   : left_(left), right_(right)
	{}
	std::string str() const
	{
		std::stringstream s;
		s << left_->str() << "." << right_->str();
		return s.str();
	}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant left = left_->evaluate(variables,add_debug_info(fdb,0,"left."));
		if(!left.is_callable()) {
			if(left.is_list()) {
				list_callable list_call(left);
				dot_callable callable(variables, list_call);
				return right_->evaluate(callable,fdb);
			}

			return left;
		}

		dot_callable callable(variables, *left.as_callable());
		return right_->evaluate(callable,add_debug_info(fdb,1,".right"));
	}

	expression_ptr left_, right_;
};

class square_bracket_expression : public formula_expression {
public:
	square_bracket_expression(expression_ptr left, expression_ptr key)
	   : left_(left), key_(key)
	{}

	std::string str() const
	{
		std::stringstream s;
		s << left_->str() << '[' << key_->str() << ']';
		return s.str();
	}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant left = left_->evaluate(variables,fdb);
		const variant key = key_->evaluate(variables,fdb);
		if(left.is_list() || left.is_map()) {
			return left[ key ];
		} else {
			return variant();
		}
	}

	expression_ptr left_, key_;
};

class operator_expression : public formula_expression {
public:
	operator_expression(const std::string& op, expression_ptr left,
	                             expression_ptr right)
		: op_(OP(op[0])), op_str_(op), left_(left), right_(right)
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
		} else if(op == ".+") {
			op_ = ADDL;
		} else if(op == ".-") {
			op_ = SUBL;
		} else if(op == ".*") {
			op_ = MULL;
		} else if(op == "./") {
			op_ = DIVL;
		} else if(op == "..") {
			op_ = CAT;
		}
	}

	std::string str() const
	{
		std::stringstream s;
		s << left_->str() << op_str_ << right_->str();
		return s.str();
	}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant left = left_->evaluate(variables,add_debug_info(fdb,0,"left_OP"));
		const variant right = right_->evaluate(variables,add_debug_info(fdb,1,"OP_right"));
		switch(op_) {
		case AND:
			return left.as_bool() == false ? left : right;
		case OR:
			return left.as_bool() ? left : right;
		case ADD:
			return left + right;
		case SUB:
			return left - right;
		case MUL:
			return left * right;
		case DIV:
			return left / right;
		case POW:
			return left ^ right;
		case ADDL:
			return left.list_elements_add(right);
		case SUBL:
			return left.list_elements_sub(right);
		case MULL:
			return left.list_elements_mul(right);
		case DIVL:
			return left.list_elements_div(right);
		case CAT:
			return left.concatenate(right);
		case EQ:
			return left == right ? variant(1) : variant(0);
		case NEQ:
			return left != right ? variant(1) : variant(0);
		case LTE:
			return left <= right ? variant(1) : variant(0);
		case GTE:
			return left >= right ? variant(1) : variant(0);
		case LT:
			return left < right ? variant(1) : variant(0);
		case GT:
			return left > right ? variant(1) : variant(0);
		case MOD:
			return left % right;
		case RAN:
			return left.build_range(right);
		case DICE:
			return variant(dice_roll(left.as_int(), right.as_int()));
		default:
			std::cerr << "ERROR: Unimplemented operator!" << std::endl;
			return variant();
		}
	}

	static int dice_roll(int num_rolls, int faces) {
		int res = 0;
		while(faces > 0 && num_rolls-- > 0) {
			if(random_new::generator) {
				res += (random_new::generator->next_random() % faces) + 1;
			} else {
				res += (rand() % faces) + 1;
			}
		}
		return res;
	}

	enum OP { AND, OR, NEQ, LTE, GTE, CAT, GT='>', LT='<', EQ='=', RAN='~',
	          ADD='+', SUB='-', MUL='*', DIV='/', ADDL, SUBL, MULL, DIVL, DICE='d', POW='^', MOD='%' };

	OP op_;
	std::string op_str_;
	expression_ptr left_, right_;
};

typedef std::map<std::string,expression_ptr> expr_table;
typedef boost::shared_ptr<expr_table> expr_table_ptr;
typedef std::map<std::string, variant> exp_table_evaluated;

class where_variables: public formula_callable {
public:
	where_variables(const formula_callable &base,
			expr_table_ptr table )
		: formula_callable(false)
		, base_(base)
		, table_(table)
		, evaluated_table_()
	{
	}

private:
	const formula_callable& base_;
	expr_table_ptr table_;
	mutable exp_table_evaluated evaluated_table_;

	void get_inputs(std::vector<formula_input>* inputs) const {
		for(expr_table::const_iterator i = table_->begin(); i != table_->end(); ++i) {
			inputs->push_back(formula_input(i->first, FORMULA_READ_ONLY));
		}
	}

	variant get_value(const std::string& key) const {
		expr_table::iterator i = table_->find(key);
		if(i != table_->end()) {
			exp_table_evaluated::const_iterator ev = evaluated_table_.find(key);
			if( ev != evaluated_table_.end())
				return ev->second;

			variant v = i->second->evaluate(base_);
			evaluated_table_[key] = v;
			return v;
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

	std::string str() const
	{
		std::stringstream s;
		s << "{where:(";
		s << body_->str();
		BOOST_FOREACH(const expr_table::value_type &a, *clauses_) {
			s << ", [" << a.first << "] -> ["<< a.second->str()<<"]";
		}
		s << ")}";
		return s.str();
	}

private:
	expression_ptr body_;
	expr_table_ptr clauses_;

	variant execute(const formula_callable& variables,formula_debugger *fdb) const {
		where_variables wrapped_variables(variables, clauses_);
		return body_->evaluate(wrapped_variables,fdb);
	}
};


class identifier_expression : public formula_expression {
public:
	explicit identifier_expression(const std::string& id) : id_(id)
	{}
	std::string str() const
	{
		return id_;
	}
private:
	variant execute(const formula_callable& variables, formula_debugger * /*fdb*/) const {
		return variables.query_value(id_);
	}
	std::string id_;
};

class null_expression : public formula_expression {
public:
	explicit null_expression() {}
	std::string str() const {
		return "";
	}
private:
	variant execute(const formula_callable& /*variables*/, formula_debugger * /*fdb*/) const {
		return variant();
	}
};


class integer_expression : public formula_expression {
public:
	explicit integer_expression(int i) : i_(i)
	{}
	std::string str() const
	{
		std::stringstream s;
		s << i_;
		return s.str();
	}
private:
	variant execute(const formula_callable& /*variables*/, formula_debugger * /*fdb*/) const {
		return variant(i_);
	}

	int i_;
};

class decimal_expression : public formula_expression {
public:
	explicit decimal_expression(int i, int f) : i_(i), f_(f)
	{}

	std::string str() const
	{
		std::stringstream s;
		s << i_ << '.' << f_;
		return s.str();
	}
private:
	variant execute(const formula_callable& /*variables*/, formula_debugger * /*fdb*/) const {
		return variant(i_ * 1000 + f_, variant::DECIMAL_VARIANT );
	}

	int i_, f_;
};

class string_expression : public formula_expression {
public:
	explicit string_expression(std::string str) :
		str_(),
		subs_()
	{
		std::string::iterator i = str.begin();
		while((i = std::find(i, str.end(), '[')) != str.end()) {
			int bracket_depth = 0;
			std::string::iterator j = i + 1;
			while(j != str.end() && (bracket_depth > 0 || *j != ']')) {
				if(*j == '[') {
					bracket_depth++;
				} else if(*j == ']' && bracket_depth > 0) {
					bracket_depth--;
				}
				++j;
			}
			if(j == str.end()) {
				break;
			}

			const std::string formula_str(i+1, j);
			const int pos = i - str.begin();
			if(j - i == 2 && (i[1] == '(' || i[1] == '\'' || i[1] == ')')) {
				// Bracket contained nothing but a quote or parenthesis.
				// This means it was intended as a literal quote or square bracket.
				i = str.erase(i);
				if(*i == '(') *i = '[';
				else if(*i == ')') *i = ']';
				i = str.erase(i + 1);
				continue;
			} else {
				i = str.erase(i, j+1);
			}

			substitution sub;
			sub.pos = pos;
			try {
				sub.calculation.reset(new formula(formula_str));
			} catch(formula_error& e) {
				e.filename += " - string substitution";
				throw e;
			}
			subs_.push_back(sub);
		}

		std::reverse(subs_.begin(), subs_.end());

		str_ = variant(str);
	}

	std::string str() const
	{
		return str_.as_string();
	}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		if(subs_.empty()) {
			return str_;
		} else {
			std::string res = str_.as_string();
			for(size_t i=0; i < subs_.size(); ++i) {
				const substitution& sub = subs_[i];
				const std::string str = sub.calculation->evaluate(variables,fdb).string_cast();
				res.insert(sub.pos, str);
			}

			return variant(res);
		}
	}

	struct substitution {

		substitution() :
			pos(0),
			calculation()
		{
		}

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
		precedence_map["~"]     = ++n;
		precedence_map["+"]     = ++n;
		precedence_map["-"]     = n;
		precedence_map[".."]    = n;
		precedence_map["*"]     = ++n;
		precedence_map["/"]     = n;
		precedence_map["%"]     = ++n;
		precedence_map["^"]     = ++n;
		precedence_map["d"]     = ++n;
		precedence_map["."]     = ++n;
	}

	assert(precedence_map.count(std::string(t.begin,t.end)));
	return precedence_map[std::string(t.begin,t.end)];
}

expression_ptr parse_expression(const token* i1, const token* i2, function_symbol_table* symbols);


//function used when creating error reports, parses all tokens passed to parse_expression, thus there are no EOL or whitespaces
std::string tokens_to_string(const token* i1, const token* i2)
{
		std::ostringstream expr;
		while(i1 != i2) {
			expr << std::string(i1->begin,i1->end) << " ";
			++i1;
		}
		return expr.str();
}

void parse_function_args(const token* &i1, const token* i2,
		std::vector<std::string>* res)
{
	const token* begin = i1, *end = i2;	//these are used for error reporting

	if(i1->type == TOKEN_LPARENS) {
		++i1;
	} else {
		throw formula_error("Invalid function definition", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
	}

	while((i1-> type != TOKEN_RPARENS) && (i1 != i2)) {
		if(i1->type == TOKEN_IDENTIFIER) {
			if(std::string((i1+1)->begin, (i1+1)->end) == "*") {
				res->push_back(std::string(i1->begin, i1->end) + std::string("*"));
				++i1;
			} else {
				res->push_back(std::string(i1->begin, i1->end));
			}
		} else if (i1->type == TOKEN_COMMA) {
			//do nothing
		} else {
			throw formula_error("Invalid function definition", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
		}
		++i1;
	}

	if(i1->type != TOKEN_RPARENS) {
		throw formula_error("Invalid function definition", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
	}
	++i1;
}

void parse_args(const token* i1, const token* i2,
                std::vector<expression_ptr>* res,
				function_symbol_table* symbols)
{
	int parens = 0;
	const token* beg = i1;
	while(i1 != i2) {
		if(i1->type == TOKEN_LPARENS || i1->type == TOKEN_LSQUARE ) {
			++parens;
		} else if(i1->type == TOKEN_RPARENS || i1->type == TOKEN_RSQUARE ) {
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

void parse_set_args(const token* i1, const token* i2,
                std::vector<expression_ptr>* res,
				function_symbol_table* symbols)
{
	int parens = 0;
	bool check_pointer = false;
	const token* beg = i1;
	const token* begin = i1, *end = i2;	//these are used for error reporting
	while(i1 != i2) {
		if(i1->type == TOKEN_LPARENS || i1->type == TOKEN_LSQUARE) {
			++parens;
		} else if(i1->type == TOKEN_RPARENS || i1->type == TOKEN_RSQUARE) {
			--parens;
		} else if( i1->type == TOKEN_POINTER && !parens ) {
			if (!check_pointer) {
				check_pointer = true;
				res->push_back(parse_expression(beg,i1, symbols));
				beg = i1+1;
			} else {
				throw formula_error("Too many '->' operators found", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
			}
		} else if( i1->type == TOKEN_COMMA && !parens ) {
			if (check_pointer)
				check_pointer = false;
			else {
				throw formula_error("Expected comma, but '->' found", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
		}
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
			             expr_table_ptr res, function_symbol_table* symbols) {
	int parens = 0;
	const token *original_i1_cached = i1;
	const token *beg = i1;
	const token* begin = i1, *end = i2;	//these are used for error reporting
	std::string var_name;
	while(i1 != i2) {
		if(i1->type == TOKEN_LPARENS) {
			++parens;
		} else if(i1->type == TOKEN_RPARENS) {
			--parens;
		} else if(!parens) {
			if(i1->type == TOKEN_COMMA) {
				if(var_name.empty()) {
					throw formula_error("There is 'where <expression>' but 'where name=<expression>' was needed", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
				}
				(*res)[var_name] = parse_expression(beg,i1, symbols);
				beg = i1+1;
				var_name = "";
			} else if(i1->type == TOKEN_OPERATOR) {
				std::string op_name(i1->begin, i1->end);
				if(op_name == "=") {
					if(beg->type != TOKEN_IDENTIFIER) {
						if(i1 == original_i1_cached) {
							throw formula_error("There is 'where <expression' but 'where name=<expression>' was needed", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
						} else {
							throw formula_error("There is 'where <expression>=<expression>' but 'where name=<expression>' was needed", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
						}
					} else if(beg+1 != i1) {
						throw formula_error("There is 'where name <expression>=<expression>' but 'where name=<expression>' was needed", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
					} else if(!var_name.empty()) {
						throw formula_error("There is 'where name=name=<expression>' but 'where name=<expression>' was needed", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
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
			throw formula_error("There is 'where <expression>' but 'where name=<expression>' was needed", tokens_to_string(begin,end-1), *i1->filename, i1->line_number);
		}
		(*res)[var_name] = parse_expression(beg,i1, symbols);
	}
}

expression_ptr parse_expression(const token* i1, const token* i2, function_symbol_table* symbols)
{
	if(i1 == i2) {
		throw formula_error("Empty expression", "", *i1->filename, i1->line_number);
	}

	const token* begin = i1, *end = i2;	//these are used for error reporting

	if(i1->type == TOKEN_KEYWORD &&
			(i1+1)->type == TOKEN_IDENTIFIER) {
		if(std::string(i1->begin, i1->end) == "def") {
			++i1;
			const std::string formula_name = std::string(i1->begin, i1->end);
			std::vector<std::string> args;
			parse_function_args(++i1, i2, &args);
			const token* beg = i1;
			while((i1 != i2) && (i1->type != TOKEN_SEMICOLON)) {
				++i1;
			}
			const std::string precond = "";
			if(symbols == NULL) {
				throw formula_error("Function symbol table required but not present", "",*i1->filename, i1->line_number);
			}
			symbols->add_formula_function(formula_name,
					const_formula_ptr(new formula(beg, i1, symbols)),
					formula::create_optional_formula(precond, symbols),
					args);
			if((i1 == i2) || (i1 == (i2-1))) {
				return expression_ptr(new function_list_expression(symbols));
			}
			else {
				return parse_expression((i1+1), i2, symbols);
			}
		}
	}
	
	if((i2-1)->type == TOKEN_RSQUARE) { // Check if there is [ ] : a indexing operator
		const token* tok = i2-2;
		int square_parens = 0;
		bool is_index_op = true;
		while((tok->type != TOKEN_LSQUARE || square_parens) && tok != i1) {
			if (tok->type == TOKEN_RSQUARE) {
				square_parens++;
			} else if(tok->type == TOKEN_LSQUARE) {
				square_parens--;
			} else if((tok->type == TOKEN_POINTER || tok->type == TOKEN_COMMA) && !square_parens ) {
				is_index_op = false;
			}
			--tok;
		}
		if(tok == i1 || tok->type != TOKEN_LSQUARE) {
			is_index_op = false;
		}
		if(is_index_op) {
			try {
				return expression_ptr(new square_bracket_expression(
						parse_expression(i1,tok,symbols),
						parse_expression(tok+1,i2-1,symbols)));
			} catch (formula_error& e){
				throw formula_error(e.type, tokens_to_string(i1, i2-1), *i1->filename, i1->line_number);
			}
		}
	}

	int parens = 0;
	const token* op = NULL;
	bool operator_group = false;
	for(const token* i = i1; i != i2; ++i) {
		if(i->type == TOKEN_LPARENS || i->type == TOKEN_LSQUARE) {
			++parens;
		} else if(i->type == TOKEN_RPARENS || i->type == TOKEN_RSQUARE) {
			--parens;
		} else if(parens == 0 && i->type == TOKEN_OPERATOR) {
			if( ( !operator_group ) && (op == NULL || operator_precedence(*op) >=
							 operator_precedence(*i)) ) {
				// Need special exception for exponentiation to be right-associative
				if(*i->begin != '^' || op == NULL || *op->begin != '^')
					op = i;
			}
			operator_group = true;
		} else {
			operator_group = false;
		}
	}

	if(op == NULL) {
		if(i1->type == TOKEN_LPARENS && (i2-1)->type == TOKEN_RPARENS) {
			return parse_expression(i1+1,i2-1,symbols);
		} else if((i2-1)->type == TOKEN_RSQUARE) { // Check if there is [ ] : a list/map definition
			const token* tok = i2-2;
			int square_parens = 0;
			bool is_map = false;
			while ((tok->type != TOKEN_LSQUARE || square_parens) && tok != i1) {
					if (tok->type == TOKEN_RSQUARE) {
						square_parens++;
					} else if(tok->type == TOKEN_LSQUARE) {
						square_parens--;
					} else if( (tok->type == TOKEN_POINTER) && !square_parens ) {
						is_map = true;
					}
					--tok;
			}
			if (tok->type == TOKEN_LSQUARE && tok == i1) {
				//create a list or a map
				std::vector<expression_ptr> args;

				if (is_map) {
					parse_set_args(i1+1, i2-1, &args, symbols);
					return expression_ptr(new map_expression(args));
				} else {
					parse_args(i1+1,i2-1,&args,symbols);
					return expression_ptr(new list_expression(args));
				}
			}
		} else if(i2 - i1 == 1) {
			if(i1->type == TOKEN_KEYWORD) {
				if(std::string(i1->begin,i1->end) == "functions") {
					return expression_ptr(new function_list_expression(symbols));
				}
			} else if(i1->type == TOKEN_IDENTIFIER) {
				return expression_ptr(new identifier_expression(
				                 std::string(i1->begin,i1->end)));
			} else if(i1->type == TOKEN_INTEGER) {
				int n = atoi(std::string(i1->begin,i1->end).c_str());
				return expression_ptr(new integer_expression(n));
			} else if(i1->type == TOKEN_DECIMAL) {
				iterator dot = i1->begin;
				while( *dot != '.' )
					++dot;

				int n = atoi(std::string(i1->begin,dot).c_str());

				iterator end = i1->end;

				if( end - dot > 4)
					end = dot + 4;

				++dot;

				int f = 0;

				int multiplicator = 100;
				while( dot != end) {
					f += (*dot - 48)*multiplicator;
					multiplicator /= 10;
					++dot;
				}

				return expression_ptr(new decimal_expression(n, f));
			} else if(i1->type == TOKEN_STRING_LITERAL) {
				return expression_ptr(new string_expression(std::string(i1->begin+1,i1->end-1)));
			}
		} else if(i1->type == TOKEN_IDENTIFIER &&
		          (i1+1)->type == TOKEN_LPARENS &&
				  (i2-1)->type == TOKEN_RPARENS) {
			const token* begin = i1, *end = i2;	//these are used for error reporting
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
				try{
					return create_function(std::string(i1->begin,i1->end),args,symbols);
				}
				catch(formula_error& e) {
					throw formula_error(e.type, tokens_to_string(begin,end), *i1->filename, i1->line_number);
				}
			}
		}

		throw formula_error("Could not parse expression", tokens_to_string(i1, i2), *i1->filename, i1->line_number);
	}
	if (op + 1 == end) {
		throw formula_error("Expected another token", tokens_to_string(begin,end-1), *op->filename, op->line_number);
	}
	if(op == i1) {
		try{
			return expression_ptr(new unary_operator_expression(
		                         std::string(op->begin,op->end),
								 parse_expression(op+1,i2,symbols)));
		}
		catch(formula_error& e)	{
			throw formula_error( e.type, tokens_to_string(begin,end-1), *op->filename, op->line_number);
		}
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

formula::~formula()
{
	if(managing_symbols) {
		delete symbols_;
	}
}

formula_ptr formula::create_optional_formula(const std::string& str, function_symbol_table* symbols)
{
	if(str.empty()) {
		return formula_ptr();
	}

	return formula_ptr(new formula(str, symbols));
}

formula::formula(const std::string& str, function_symbol_table* symbols) :
	expr_(),
	str_(str),
	symbols_(symbols),
	managing_symbols(symbols == NULL)
{
	using namespace formula_tokenizer;
	
	if(symbols == NULL) {
		symbols_ = new function_symbol_table;
	}

	std::vector<token> tokens;
	std::string::const_iterator i1 = str.begin(), i2 = str.end();

	//set true when 'fai' keyword is found
	bool fai_keyword = false;
	//set true when 'wfl' keyword is found
	bool wfl_keyword = false;
	//used to locally keep the track of which file we parse actually and in which line we are
	std::vector< std::pair< std::string, int> > files;
	//used as a source of strings - we point to these strings from tokens
	std::set< std::string > filenames;
	files.push_back( std::make_pair( "formula", 1 ) );
	filenames.insert( "formula" );
	std::set< std::string >::iterator filenames_it = filenames.begin();

	while(i1 != i2) {
		try {

			tokens.push_back( get_token(i1,i2) );

			formula_tokenizer::TOKEN_TYPE current_type = tokens.back().type;

			if(current_type == TOKEN_WHITESPACE)  {
				tokens.pop_back();
			} else
			if( current_type == TOKEN_COMMENT) {
				//since we can have multiline comments, let's see how many EOL are within it
				int counter = 0;
				std::string comment = std::string(tokens.back().begin,tokens.back().end);
				for( std::string::iterator str_it = comment.begin(); str_it != comment.end(); ++str_it)
					if( *str_it == '\n' )
						counter++;

				files.back().second+=counter;
				tokens.pop_back();
			} else
			if( current_type == TOKEN_EOL) {
				files.back().second++;
				tokens.pop_back();
			} else
			if( ( current_type == TOKEN_KEYWORD) && ( std::string(tokens.back().begin,tokens.back().end)  == "fai") ) {
				fai_keyword = true;
				tokens.pop_back();
			} else
			if( ( current_type == TOKEN_KEYWORD) && ( std::string(tokens.back().begin,tokens.back().end)  == "wfl") ) {
				wfl_keyword = true;
				tokens.pop_back();
			} else
			if( ( current_type == TOKEN_KEYWORD) && ( std::string(tokens.back().begin,tokens.back().end) == "faiend") ) {
				if (files.size() > 1) {
					files.pop_back();
					filenames_it = filenames.find( files.back().first );
					tokens.pop_back();
				} else {
					throw formula_error("Unexpected 'faiend' found", "", "", 0);
				}
			} else
			if( ( current_type == TOKEN_KEYWORD) && ( std::string(tokens.back().begin,tokens.back().end) == "wflend") ) {
				if (files.size() > 1) {
					files.pop_back();
					filenames_it = filenames.find( files.back().first );
					tokens.pop_back();
				} else {
					throw formula_error("Unexpected 'wflend' found", "", "", 0);
				}
			} else
			if (fai_keyword || wfl_keyword) {
				if(current_type == TOKEN_STRING_LITERAL) {
					std::string str = std::string(tokens.back().begin,tokens.back().end);
					files.push_back( std::make_pair( str , 1 ) );
					std::pair < std::set< std::string>::iterator , bool > ret;
					ret = filenames.insert( str );
					if(ret.second==true) {
						filenames_it = ret.first;
					} else {
						if (fai_keyword)
							throw formula_error("Faifile already included", "fai" + str, "", 0);
						else throw formula_error("Wflfile already included", "wfl" + str, "", 0);
					}
					tokens.pop_back();
					fai_keyword = false;
					wfl_keyword = false;
				} else {
					if (fai_keyword)
						throw formula_error("Expected string after the 'fai'", "fai", "", 0);
					else throw formula_error("Expected string after the 'wfl'", "wfl", "", 0);
				}
			} else {
				//in every token not specified above, store line number and name of file it came from
				tokens.back().filename = &(*filenames_it);
				tokens.back().line_number = files.back().second;
			}
		} catch(token_error& e) {
			//when we catch token error, we should write whole line in which error occurred, so we merge info from token and everything we had in the line so far
			std::string str = "";
			if(!tokens.empty()) {
				token* tok_it = &tokens[0] + tokens.size()-1;
				while( ( tok_it != &tokens[0] ) && (tok_it->line_number == tokens.back().line_number) )
					--tok_it;

				if( tok_it != &tokens[0] && tok_it != &tokens[0] + tokens.size() -1)
					++tok_it;

				str = tokens_to_string( tok_it, &tokens[0] + tokens.size() );
			}

			throw formula_error(e.description_, str + e.formula_, *filenames_it, files.back().second);
		}
	}

	if(files.size() > 1) {
		throw formula_error("Missing 'wflend', make sure each .wfl file ends with it", "", "", 0);
	}

	if(!tokens.empty()) {
		expr_ = parse_expression(&tokens[0],&tokens[0] + tokens.size(), symbols_);
	} else {
		expr_ = expression_ptr(new null_expression());
	}
}
formula::formula(const token* i1, const token* i2, function_symbol_table* symbols) :
	expr_(),
	str_(),
	symbols_(symbols),
	managing_symbols(symbols == NULL)
{
	if(symbols == NULL) {
		symbols_ = new function_symbol_table;
	}

	if(i1 != i2) {
		expr_ = parse_expression(i1,i2, symbols);
	} else {
		expr_ = expression_ptr(new null_expression());
	}
}

variant formula::execute(const formula_callable& variables, formula_debugger *fdb) const
{
	try {
		return expr_->evaluate(variables, fdb);
	} catch(type_error& e) {
		std::cerr << "formula type error: " << e.message << "\n";
		return variant();
	}
}

variant formula::execute(formula_debugger *fdb) const
{
	static map_formula_callable null_callable;
	return execute(null_callable,fdb);
}

formula_error::formula_error(const std::string& type, const std::string& formula,
		const std::string& file, int line)
	: error()
	, type(type)
	, formula(formula)
	, filename(file)
	, line(line)
{
	std::stringstream ss;
	ss << "Formula error in " << filename << ":" << line
	   << "\nIn formula " << formula
	   << "\nError: " << type;
	message = ss.str();
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
	try {
		mock_char c;
		mock_party p;

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
