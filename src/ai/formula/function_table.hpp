/* 
 * File:   function_table.hpp
 * Author: dk
 *
 * Created on 17 lipiec 2009, 11:43
 */

#ifndef FORMULA_AI_FUNCTION_TABLE_HPP_INCLUDED
#define	FORMULA_AI_FUNCTION_TABLE_HPP_INCLUDED


#include "../../formula_function.hpp"

class formula_ai;

namespace game_logic {

class ai_function_symbol_table : public function_symbol_table {

public:
	explicit ai_function_symbol_table(formula_ai& ai) :
		ai_(ai),
		move_functions()
	{}

	expression_ptr create_function(const std::string& fn,
	                               const std::vector<expression_ptr>& args) const;

private:
	formula_ai& ai_;
	std::set<std::string> move_functions;
};

}

#endif	/* _FUNCTION_TABLE_HPP */

