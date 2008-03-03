#ifndef FORMULA_FUNCTION_HPP_INCLUDED
#define FORMULA_FUNCTION_HPP_INCLUDED

#include <boost/shared_ptr.hpp>

#include <map>

#include "formula.hpp"
#include "variant.hpp"

namespace game_logic {

class formula_expression {
public:
	formula_expression() : name_(NULL) {}
	virtual ~formula_expression() {}
	variant evaluate(const formula_callable& variables) const {
		call_stack_manager manager(name_);
		return execute(variables);
	}
	void set_name(const char* name) { name_ = name; }
private:
	virtual variant execute(const formula_callable& variables) const = 0;
	const char* name_;
};

typedef boost::shared_ptr<formula_expression> expression_ptr;

class function_expression : public formula_expression {
public:
	typedef std::vector<expression_ptr> args_list;
	explicit function_expression(
	                    const std::string& name,
	                    const args_list& args,
	                    int min_args=-1, int max_args=-1)
	    : name_(name), args_(args)
	{
		set_name(name.c_str());
		if(min_args != -1 && args_.size() < min_args) {
			std::cerr << "too few arguments\n";
			throw formula_error();
		}

		if(max_args != -1 && args_.size() > max_args) {
			std::cerr << "too many arguments\n";
			throw formula_error();
		}
	}

protected:
	const args_list& args() const { return args_; }
private:
	std::string name_;
	args_list args_;
};

class formula_function_expression : public function_expression {
public:
	explicit formula_function_expression(const std::string& name, const args_list& args, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& arg_names);
private:
	variant execute(const formula_callable& variables) const;
	const_formula_ptr formula_;
	const_formula_ptr precondition_;
	std::vector<std::string> arg_names_;
	int star_arg_;
};

typedef boost::shared_ptr<function_expression> function_expression_ptr;

class formula_function {
	std::string name_;
	const_formula_ptr formula_;
	const_formula_ptr precondition_;
	std::vector<std::string> args_;
public:
	formula_function() {}
	formula_function(const std::string& name, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& args) : name_(name), formula_(formula), precondition_(precondition), args_(args)
	{}

	function_expression_ptr generate_function_expression(const std::vector<expression_ptr>& args) const;
};	

class function_symbol_table {
	std::map<std::string, formula_function> custom_formulas_;
public:
	virtual ~function_symbol_table() {}
	void add_formula_function(const std::string& name, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& args);
	virtual expression_ptr create_function(const std::string& fn,
					                       const std::vector<expression_ptr>& args) const;
};

expression_ptr create_function(const std::string& fn,
                               const std::vector<expression_ptr>& args,
							   const function_symbol_table* symbols);

}

#endif
