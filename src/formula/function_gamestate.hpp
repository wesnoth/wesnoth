
#include "formula/function.hpp"

namespace wfl {

class gamestate_function_symbol_table : public function_symbol_table {
public:
	gamestate_function_symbol_table(std::shared_ptr<function_symbol_table> parent = nullptr);
};

}
