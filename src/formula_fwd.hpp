#ifndef FORMULA_FWD_HPP_INCLUDED
#define FORMULA_FWD_HPP_INCLUDED

#include <boost/shared_ptr.hpp>

namespace game_logic {

class formula;
typedef boost::shared_ptr<formula> formula_ptr;
typedef boost::shared_ptr<const formula> const_formula_ptr;

}

#endif
