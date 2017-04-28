//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPIRIT_PO_DEFAULT_PLURAL_FORMS_COMPILER_HPP_INCLUDED
#define SPIRIT_PO_DEFAULT_PLURAL_FORMS_COMPILER_HPP_INCLUDED

/***
 * In GNU gettext, a language is permitted to define any number of 'plural forms'.
 * For instance, in English and most romance languages there are only two forms,
 * singular and plural. However in many other languages, there may be only one
 * form, or there may be many plural forms reserved for various numbers of items.
 *
 * In the header of a po file, as part of the metadata, translators are expected
 * to specify exactly how many plural forms there are, (how many different
 * variations of a pluralized string they will provide), and also a function that
 * computes which form (the appropriate index) should be used when the number of
 * items is a number "n".
 *
 * Traditionally, this function is specified as a single line of pseudo C code.
 *
 * Examples:
 *
 * Russian:
 *   Po header:
 *     num_plurals = 3
 *     plural=n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2;
 *   Messages:
 *
 *
 * There are many many more examples shown here:
 * http://localization-guide.readthedocs.org/en/latest/l10n/pluralforms.html
 *
 * The code in *this* file is concerned with converting these strings into
 * function objects implementing a function uint -> uint.
 *
 * These function objects are then associated to each catalog and used when
 * looking up plurals.
 *
 * In spirit-po, we provide support for the standard gettext pseudo-C language
 * using the 'default_plural_forms_compiler', which compiles these run-time
 * pseudo-C expressions into expression trees which can be evaluated.
 *
 * By using non-default template parameters and providing an appropriate
 * function object, you can make spirit-po use your favorite programming
 * language for these instead. (Or, your translators' favorite?)
 *
 * The 'plural_forms_compiler' concept must be a class/struct and provide:
 *   - The plural_forms_compiler must be default constructible.
 *   - It must have a typedef 'result_type' which is the type of the function
 *     object it produces.
 *   - An operator() overload which takes const std::string &, and return an
 *     instance of 'result_type'.
 *   - result_type must be default constructible and move constructible.
 *   - result_type must have an operator() overload which takes and yields
 *     unsigned int.
 *   - result_type must have an explicit operator bool() const overload which
 *     returns whether the function object is valid (compilation succeeded)
 *   - result_type must have a function `error()` which returns a std::string
 *     representing a compilation error message in the case of failure.
 */

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <spirit_po/default_plural_forms_expressions.hpp>
#include <spirit_po/exceptions.hpp>
#include <boost/optional/optional.hpp>
#include <boost/spirit/include/qi.hpp>
#include <string>

namespace spirit_po {

namespace qi = boost::spirit::qi;
typedef unsigned int uint;

namespace default_plural_forms {

class function_object {
  mutable stack_machine machine_;
  boost::optional<std::string> parse_error_;

public:
  function_object(const expr & _e) : machine_(_e), parse_error_() {}
  function_object(const std::string & s) : machine_(n_var()), parse_error_(s) {}
  function_object() : function_object(std::string{"uninitialized"}) {}

  uint operator()(uint n) const {
    return machine_.compute(n);
  }

  explicit operator bool() const { return !parse_error_; }
  std::string error() const { return *parse_error_; }
};

struct compiler {
  typedef function_object result_type;
  result_type operator()(const std::string & str) const {
    expr e;

    typedef std::string::const_iterator str_it;
    str_it it = str.begin();
    str_it end = str.end();
    op_grammar<str_it> grammar;

    if (qi::phrase_parse(it, end, grammar, qi::space, e) && it == end) {
      return function_object(std::move(e));
    } else {
      return function_object("Plural-Forms expression reader: Could not parse expression, stopped parsing at:\n" + string_iterator_context(str, it));
    }
  }
};

} // end namespace default_plura_forms

} // end namespace spirit_po

#endif // SPIRIT_PO_DEFAULT_PLURAL_FORMS_COMPILER_HPP_INCLUDED
