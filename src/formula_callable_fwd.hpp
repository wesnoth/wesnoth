#ifndef FORMULA_CALLABLE_FWD_HPP_INCLUDED
#define FORMULA_CALLABLE_FWD_HPP_INCLUDED

#include <boost/intrusive_ptr.hpp>

namespace game_logic {

class formula_callable;
typedef boost::intrusive_ptr<formula_callable> formula_callable_ptr;
typedef boost::intrusive_ptr<const formula_callable> const_formula_callable_ptr;

}

#endif
