
#ifndef FORMULA_STRING_UTILS_HPP_INCLUDED
#define FORMULA_STRING_UTILS_HPP_INCLUDED

#include "serialization/string_utils.hpp"

namespace utils {

/**
 * Function which will interpolate variables, starting with '$' in the string
 * 'str' with the equivalent symbols in the given symbol table. If 'symbols'
 * is NULL, then game event variables will be used instead.
 */
std::string interpolate_variables_into_string(const std::string &str, const string_map * const symbols);
std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables);

}

/** Handy wrappers around interpolate_variables_into_string and gettext. */
std::string vgettext(const char*, const utils::string_map&);
std::string vngettext(const char*, const char*, int, const utils::string_map&);

#endif
