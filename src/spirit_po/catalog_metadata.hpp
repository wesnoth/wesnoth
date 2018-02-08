//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPIRIT_PO_CATALOG_METADATA_HPP_INCLUDED
#define SPIRIT_PO_CATALOG_METADATA_HPP_INCLUDED

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <spirit_po/exceptions.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <string>

namespace spirit_po {

namespace qi = boost::spirit::qi;

typedef unsigned int uint;
typedef std::pair<uint, std::string> num_plurals_info;

struct catalog_metadata {
  std::string project_id;
  std::string language;
  std::string language_team;
  std::string last_translator;

  uint num_plural_forms;
  std::string plural_forms_function_string;

  std::string charset;

  catalog_metadata()
    : project_id()
    , language()
    , language_team()
    , last_translator()
    , num_plural_forms(0)
    , plural_forms_function_string()
  {}

private:
  std::string find_header_line(const std::string & header, const std::string & label) {
    size_t idx = header.find(label);
    if (idx == std::string::npos) {
      return "";
    }
    auto it = header.begin() + idx + label.size();
    while (it != header.end() && *it == ' ') { ++it; }

    auto e = it;
    while (e != header.end() && *e != '\n') { ++e; }
    return std::string(it, e);
  }

  template <typename Iterator>
  struct num_plurals_grammar : qi::grammar<Iterator, num_plurals_info()> {
    qi::rule<Iterator, num_plurals_info()> main;
    num_plurals_grammar() : num_plurals_grammar::base_type(main) {
      using qi::lit;
      main = qi::skip(' ') [ lit("nplurals=") >> qi::uint_ >> lit(';') >> lit("plural=") ] >> (*qi::char_);
    }
  };

#define SPIRIT_PO_DEFAULT_CHARSET "UTF-8"

  template <typename Iterator>
  struct content_type_grammar : qi::grammar<Iterator, std::string()> {
    qi::rule<Iterator, std::string()> main;
    content_type_grammar() : content_type_grammar::base_type(main) {
      using qi::lit;
      using qi::omit;
      using qi::skip;
      main = skip(' ')[ omit[ *(qi::char_ - ';') >> lit(';') ] >> ((lit("charset=") >> *(qi::char_)) | qi::attr(SPIRIT_PO_DEFAULT_CHARSET)) ];
    }
  };

public:
  // nonempty return is an error message
  std::string parse_header(const std::string & header) {
    const char * const default_charset = SPIRIT_PO_DEFAULT_CHARSET;
#undef SPIRIT_PO_DEFAULT_CHARSET

    project_id = find_header_line(header, "Project-Id-Version:");
    language = find_header_line(header, "Language:");
    language_team = find_header_line(header, "Language-Team:");
    last_translator = find_header_line(header, "Last-Translator:");

    std::string content_type_line = find_header_line(header, "Content-Type:");
    if (content_type_line.size()) {
      auto it = content_type_line.begin();
      auto end = content_type_line.end();
      content_type_grammar<decltype(it)> gram;
      std::string ct;
      if (qi::parse(it, end, gram, ct)) {
        charset = ct;
        if (charset != "ASCII" && charset != "UTF-8") {
          return "PO file declared charset of '" + charset + "', but spirit_po only supports UTF-8 and ASCII for this.";
        }
      }
    } else {
      // Assume defaults for charset
      charset = default_charset;
    }

    std::string content_transfer_encoding = find_header_line(header, "Content-Transfer-Encoding:");
    if (content_transfer_encoding.size()) {
      auto it = content_transfer_encoding.begin();
      auto end = content_transfer_encoding.end();
      if (!qi::phrase_parse(it, end, qi::lit("8bit"), qi::ascii::space)) {
        return "PO header 'Content-Transfer-Encoding' must be '8bit' if specified, but PO file declared '" + content_transfer_encoding + "'";
      }
    }

    std::string num_plurals_line = find_header_line(header, "Plural-Forms:");

    if (num_plurals_line.size()) {
      auto it = num_plurals_line.begin();
      auto end = num_plurals_line.end();

      num_plurals_grammar<decltype(it)> gram;
      num_plurals_info info;
      if (qi::parse(it, end, gram, info)) {
        num_plural_forms = info.first;
        plural_forms_function_string = info.second;
      } else {
        num_plural_forms = 0;
        plural_forms_function_string = "";
        return "Failed to parse Plural-Forms entry -- stopped at:\n" + string_iterator_context(num_plurals_line, it);
      }
    } else {
      num_plural_forms = 2;
      plural_forms_function_string = "n != 1";
    }
    return "";
  }

  // check if this metadata is compatible with another metadata (number of plural forms, maybe other criteria)
  // return a nonempty string containing error message if they are not compatible.
  std::string check_compatibility(const catalog_metadata & other) const {
    if (num_plural_forms != other.num_plural_forms) {
      return std::string{"Num plural forms mismatch. this = "} + std::to_string(num_plural_forms) + " other = " + std::to_string(other.num_plural_forms);
    }
    return "";
  }
};

} // end namespace spirit_po

#endif // SPIRIT_PO_CATALOG_METADATA_HPP_INCLUDED
