//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPIRIT_PO_PO_MESSAGE_HPP_INCLUDED
#define SPIRIT_PO_PO_MESSAGE_HPP_INCLUDED

#include <boost/optional/optional.hpp>
#include <string>
#include <utility>
#include <vector>

namespace spirit_po {

typedef std::pair<std::string, std::vector<std::string>> plural_and_strings_type;

struct po_message {
  boost::optional<std::string> context;
  std::string id;
  plural_and_strings_type plural_and_strings;

  std::size_t line_no;

  // Get the 'id_plural', 'strings' fields from the pair.
  // It is arranged as a pair here to allow for simpler parsing with spirit attributes.
  std::string & id_plural() { return plural_and_strings.first; }
  const std::string & id_plural() const { return plural_and_strings.first; }

  std::vector<std::string> & strings() { return plural_and_strings.second; }
  const std::vector<std::string> & strings() const { return plural_and_strings.second; }

  // Check if message is plural. We do this for now by testing msgid_plural.size().
  // Recommended to use this method in case we change it in the future.
  bool is_plural() const { return (id_plural().size() != 0); } 
};

/***
 * Debug printer
 */
#ifdef SPIRIT_PO_DEBUG
inline std::string debug_string(const po_message & msg) {
  std::string result = "{\n";
  if (msg.context) {
    result += "  context: \"" + *msg.context + "\"\n";
  }
  result += "  id: \"" + msg.id + "\"\n";
  result += "  id_plural: \"" + msg.id_plural() + "\"\n";
  result += "  strings: { ";
  for (uint i = 0; i < msg.strings().size(); ++i) {
    if (i) { result += ", "; }
    result += '"' + msg.strings()[i] + '"';
  }
  result += " }\n";
  result += "}";
  return result;
}
#endif // SPIRIT_PO_DEBUG

} // end namespace spirit_po

#endif // SPIRIT_PO_PO_MESSAGE_HPP_INCLUDED
