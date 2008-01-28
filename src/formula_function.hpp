#ifndef FORMULA_FUNCTION_HPP_INCLUDED
#define FORMULA_FUNCTION_HPP_INCLUDED

#include <boost/shared_ptr.hpp>

#include <map>

#include "formula.hpp"
#include "variant.hpp"

namespace game_logic {

class formula_expression {
public:
	virtual ~formula_expression() {}
	variant evaluate(const formula_callable& variables) const {
		return execute(variables);
	}
private:
	virtual variant execute(const formula_callable& variables) const = 0;
};

typedef boost::shared_ptr<formula_expression> expression_ptr;

class function_expression : public formula_expression {
public:
	typedef std::vector<expression_ptr> args_list;
	explicit function_expression(const args_list& args,
	                    int min_args=-1, int max_args=-1)
	    : args_(args)
	{
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
	args_list args_;
};

class formula_function_expression : public function_expression {
public:
	explicit formula_function_expression(const args_list& args, const_formula_ptr formula, const std::vector<std::string>& arg_names)
	   : function_expression(args, arg_names.size(), arg_names.size()),
		 formula_(formula), arg_names_(arg_names)
	{}
private:
	variant execute(const formula_callable& variables) const;
	const_formula_ptr formula_;
	std::vector<std::string> arg_names_;
};

typedef boost::shared_ptr<function_expression> function_expression_ptr;

class formula_function {
	const_formula_ptr formula_;
	std::vector<std::string> args_;
public:
	formula_function() {}
	formula_function(const_formula_ptr formula, const std::vector<std::string>& args) : formula_(formula), args_(args)
	{}

	function_expression_ptr generate_function_expression(const std::vector<expression_ptr>& args) const;
};	

class function_symbol_table {
	std::map<std::string, formula_function> custom_formulas_;
public:
	virtual ~function_symbol_table() {}
	void add_formula_function(const std::string& name, const_formula_ptr formula, const std::vector<std::string>& args);
	virtual expression_ptr create_function(const std::string& fn,
					                       const std::vector<expression_ptr>& args) const;
};

expression_ptr create_function(const std::string& fn,
                               const std::vector<expression_ptr>& args,
							   const function_symbol_table* symbols);

}

#endif
