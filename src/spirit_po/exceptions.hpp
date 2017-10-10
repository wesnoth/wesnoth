//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPIRIT_PO_EXCEPTIONS_HPP_INCLUDED
#define SPIRIT_PO_EXCEPTIONS_HPP_INCLUDED

#include <spirit_po/default_plural_forms_expressions.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <string>

namespace spirit_po {

// Show the next 80 characters from some iterator position.
// Intended to be used for parser error messages
template <typename Iterator>
std::string iterator_context(Iterator & it, Iterator & end) {
  std::string result;
  std::size_t line_no = boost::spirit::get_line(it);
  if (line_no != static_cast<std::size_t>(-1)) {
    result = "Line " + std::to_string(line_no) + ":\n";
  }

  uint count = 80;
  while (it != end && count) {
    result += *it;
    ++it;
    --count;
  }
  return result;
}

// When the thing being parsed is a short string, we can give
// a better context report
inline std::string string_iterator_context(const std::string & str,
                                           std::string::const_iterator it) {
  std::string result{str};
  result += "\n";

  for (auto temp = str.begin(); temp != it; ++temp) {
    result += ' ';
  }
  result += "^\n";
  return result;
}

} // end namespace spirit_po


#ifdef SPIRIT_PO_NO_EXCEPTIONS

#define SPIRIT_PO_CATALOG_FAIL(Message)                      \
do {                                                         \
  error_message_ = (Message);                                \
  return ;                                                   \
} while(0)

#else // SPIRIT_PO_NO_EXCEPTIONS

#include <stdexcept>

namespace spirit_po {

struct catalog_exception : std::runtime_error {
  explicit catalog_exception(const char * what) : runtime_error(what) {}
  explicit catalog_exception(const std::string & what) : runtime_error(what) {}
};

} // end namespace spirit_po

#define SPIRIT_PO_CATALOG_FAIL(Message)                      \
do {                                                         \
  throw spirit_po::catalog_exception(( Message ));           \
} while(0)


#endif // SPIRIT_PO_NO_EXCEPTIONS

#endif // SPIRIT_PO_EXCEPTIONS_HPP_INCLUDED
