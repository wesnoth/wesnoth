//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPIRIT_PO_CATALOG_HPP_INCLUDED
#define SPIRIT_PO_CATALOG_HPP_INCLUDED

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <spirit_po/catalog_metadata.hpp>
#include <spirit_po/default_plural_forms_compiler.hpp>
#include <spirit_po/exceptions.hpp>
#include <spirit_po/po_grammar.hpp>
#include <spirit_po/po_message.hpp>

#include <boost/spirit/include/qi.hpp>
#include <functional>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

namespace spirit_po {

namespace spirit = boost::spirit;
namespace qi = spirit::qi;
typedef unsigned int uint;

typedef std::function<void(const std::string &)> warning_channel_type;
typedef std::unordered_map<std::string, po_message> default_hashmap_type;

template <typename hashmap_type = default_hashmap_type, typename pf_compiler = default_plural_forms::compiler>
class catalog {
  catalog_metadata metadata_;

  typename pf_compiler::result_type pf_function_object_;
  uint singular_index_; // cached result of pf_function_object(1)

#ifdef SPIRIT_PO_NOEXCEPT
  boost::optional<std::string> error_message_;
  // if loading failed, error_message_ contains an error
  // (rather than throwing an exception)
#endif // SPIRIT_PO_NOEXCEPT
  warning_channel_type warning_channel_;

  hashmap_type hashmap_;

public:
  static const char EOT = '\x4';
  // ASCII 4 is EOT character
  // Used to separate msg context from msgid in the hashmap, in MO files
  // We use the same formatting system, just for consistency.
  // c.f. https://www.gnu.org/software/gettext/manual/html_node/MO-Files.html

  static std::string form_context_index(const std::string & msgctxt, const std::string & id) {
    return msgctxt + EOT + id;
  }

  static std::string form_index(const po_message & msg) {
    return msg.context ? form_context_index(*msg.context, msg.id) : msg.id;
  }

private:
  /***
   * Helper for interacting with hashmap results
   * get(msg) gets the *singular* string from the message. if it's a plural message, look in singular_index_.
   * if it's not a plural message, then there is only one string. also, the po header is never a plural message
   */
  const std::string & get(const po_message & msg) const {
    if (msg.strings().size() == 1) { return msg.strings()[0]; }
    return msg.strings()[singular_index_];
  }

  const std::string & get(const po_message & msg, uint plural) const {
    uint idx = (plural == 1 ? singular_index_ : pf_function_object_(plural));
    return msg.strings()[idx];
  }

  /***
   * Emplace a message into the hashmap
   */
  void insert_message(po_message && msg) {
    if (!msg.strings().size()) { return; }
    // don't allow messages with ZERO translations into the catalog, this will cause segfaults later.
    // should perhaps throw an exception here

    if (!msg.strings()[0].size()) { return; }
    // if the (first) translated string is "", it is untranslated and message does not enter catalog

    if (msg.strings().size() > 1 && msg.strings().size() != metadata_.num_plural_forms) {
      if (warning_channel_) {
        warning_channel_("Ignoring a message with an incorrect number of plural forms: plural = " + std::to_string(msg.strings().size()) + " msgid = '" + msg.id + "'");
      }
      return;
    }

    std::string index = form_index(msg);
    // adjust the id based on context if necessary

    auto result = hashmap_.emplace(std::move(index), std::move(msg));

    // Issue a warning if emplace failed, rather than silently overwrite.
    if (!result.second) {
      if (warning_channel_) {
        std::string warning = "Overwriting a message: msgid = <<<" + msg.id + ">>>";
        if (msg.context) { warning += " msgctxt = <<<" + *msg.context + ">>>"; }
        warning_channel_(warning);
      }
      result.first->second = std::move(msg);
    }
  }

public:
#ifdef SPIRIT_PO_NOEXCEPT
  /***
   * Error checking (this is done so we don't have to throw exceptions from the ctor.
   */
  explicit operator bool() const {
    return !error_message_;
  }

  std::string error() const {
    return *error_message_; // UB if there there is not an error message
  }
#endif // SPIRIT_PO_NOEXCEPT

  /***
   * Ctors
   */
  template <typename Iterator>
  catalog(spirit::line_pos_iterator<Iterator> & it, spirit::line_pos_iterator<Iterator> & end, warning_channel_type warn_channel = warning_channel_type(), pf_compiler compiler = pf_compiler())
    : metadata_()
    , pf_function_object_()
    , warning_channel_(warn_channel)
    , hashmap_()
  {
    typedef spirit::line_pos_iterator<Iterator> iterator_type;
    po_grammar<iterator_type> grammar;

    po_message msg;
    std::size_t line_no = 0;

    // Parse header first
    {
      // must be able to parse first message
      qi::parse(it, end, grammar.skipped_block); // first parse any comments
      if (!qi::parse(it, end, grammar, msg)) {   // now parse the main grammar target
        int err_line = it.position();
        SPIRIT_PO_CATALOG_FAIL("Failed to parse po header, stopped at line " + std::to_string(err_line) + ": " + iterator_context(it, end));
      }

      // first message must have empty MSGID (po format says so)
      if (msg.id.size()) {
        SPIRIT_PO_CATALOG_FAIL("Failed to parse po header, first msgid must be empty string \"\", found: " + msg.id);
      }

      // Now parse the header string itself
      if (msg.strings().size()) {
        std::string maybe_error = metadata_.parse_header(msg.strings()[0]);
        if (maybe_error.size()) {
          SPIRIT_PO_CATALOG_FAIL("Failed to parse po header: " + maybe_error);
        }
      }

      if (!metadata_.num_plural_forms) {
        SPIRIT_PO_CATALOG_FAIL("Invalid metadata in po header, found num_plurals = 0");
      }

      // Try to compile the plural forms function string
      pf_function_object_ = compiler(metadata_.plural_forms_function_string);
      if (!pf_function_object_) {
        SPIRIT_PO_CATALOG_FAIL(("Failed to read plural forms function. "
                                "Input: '" + metadata_.plural_forms_function_string + "', "
                                "error message: " + pf_function_object_.error()));
      } 

      // Cache the 'singular' form index since it is most common
      singular_index_ = pf_function_object_(1);
      if (singular_index_ >= metadata_.num_plural_forms) {
        SPIRIT_PO_CATALOG_FAIL(("Invalid plural forms function. "
                                "On input n = 1, returned plural = " + std::to_string(singular_index_) + ", "
                                "while num_plurals = " + std::to_string(metadata_.num_plural_forms)));
      }

      msg.line_no = line_no;
      insert_message(std::move(msg)); // for compatibility, need to insert the header message at msgid ""
    }

    // Now parse non-fuzzy messages
    while (it != end) {
      // this parse rule cannot fail, it can be a zero length match
      qi::parse(it, end, grammar.ignored_comments);

      bool fuzzy = false;
      // this parse rule cannot fail, it can be a zero length match
      qi::parse(it, end, grammar.message_preamble, fuzzy);

      // check if we exhausted the file by comments
      if (it != end) {
        msg = po_message{};
        msg.strings().reserve(metadata_.num_plural_forms); // try to prevent frequent vector reallocations
        line_no = it.position();
        // actually parse a message
        if (!qi::parse(it, end, grammar, msg)) {
          int err_line = it.position();
          SPIRIT_PO_CATALOG_FAIL(("Failed to parse po file, "
                                  "started at " + std::to_string(line_no) + ": , stopped at " + std::to_string(err_line) + ":\n"
                                  + iterator_context(it, end)));
        }
        // cannot overwrite header
        if (!msg.id.size()) {
          int err_line = it.position();
          SPIRIT_PO_CATALOG_FAIL(("Malformed po file: Cannot overwrite the header entry later in the po file."
                                  "Started at " + std::to_string(line_no) + ": , stopped at " + std::to_string(err_line) + ":\n" 
                                  + iterator_context(it, end)));
        }
        msg.line_no = line_no;
        // only insert it if it wasn't marked fuzzy
        if (!fuzzy) { insert_message(std::move(msg)); }
      }
    }

#ifdef SPIRIT_PO_DEBUG
    // validate resulting hashmap
    for (const auto & p : hashmap_) {
      if (!p.second.strings().size()) { SPIRIT_PO_CATALOG_FAIL(("Internal catalog error: found a message id with no strings, msgid='" + p.first + "'")); }
      if (p.second.strings().size() != 1 && p.second.strings().size() != metadata_.num_plural_forms) {
        SPIRIT_PO_CATALOG_FAIL(("Internal catalog error: found a message id with wrong number of strings, msgid='" + p.first + "' num msgstr = " + std::to_string(p.second.strings().size()) + ", catalog num_plural_forms = " + std::to_string(metadata_.num_plural_forms) + "\nWhole message: " + debug_string(p.second) ));
      }
    }
#endif // SPIRIT_PO_DEBUG
  }

  // Upgrade an iterator pair to spirit::line_pos_iterators
  template <typename Iterator>
  static catalog from_iterators(Iterator & b, Iterator & e, warning_channel_type w = warning_channel_type()) {
    spirit::line_pos_iterator<Iterator> it{b};
    spirit::line_pos_iterator<Iterator> end{e};
    return catalog(it, end, w);
  }

  template <typename Iterator>
  static catalog from_iterators(spirit::line_pos_iterator<Iterator> & b, spirit::line_pos_iterator<Iterator> & e, warning_channel_type w = warning_channel_type()) {
    return catalog(b, e, w);
  }

  // Construct a catalog from a range using one expression
  template <typename Range>
  static catalog from_range(const Range & range, warning_channel_type w = warning_channel_type()) {
    auto it = boost::begin(range);
    auto end = boost::end(range);
    return from_iterators(it, end, w);
  }

  static catalog from_istream(std::istream & is, warning_channel_type w = warning_channel_type()) {
    // no white space skipping in the stream!
    is.unsetf(std::ios::skipws);
    spirit::istream_iterator it(is);
    spirit::istream_iterator end;
    return from_iterators(it, end, w);
  }

  ///////////////
  // ACCESSORS //
  ///////////////

  /***
   * Lookup strings from the catalog
   *
   * When using string literals as the parameters, these versions are safe and
   * are maximally efficient.
   * (The returned pointer is either the input pointer, having static storage
   * duration, or has lifetime as long as the catalog.)
   *
   * Chosen to behave in the same manner as corresponding gettext functions.
   */
  const char * gettext(const char * msgid) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end()) {
      return get(it->second).c_str();
    } else {
      return msgid;
    }
  }

  const char * ngettext(const char * msgid, const char * msgid_plural, uint plural) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end() && it->second.is_plural()) {
      return get(it->second, plural).c_str();
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  const char * pgettext(const char * context, const char * msgid) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end()) {
      return get(it->second).c_str();
    } else {
      return msgid;
    }
  }

  const char * npgettext(const char * context, const char * msgid, const char * msgid_plural, uint plural) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end() && it->second.is_plural()) {
      return get(it->second, plural).c_str();
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  /***
   * Lookup strings from catalog, return std::string.
   *
   * When, for whatever reason, it is more comfortable to use idiomatic C++.
   */
  std::string gettext_str(const std::string & msgid) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end()) {
      return get(it->second);
    } else {
      return msgid;
    }
  }

  std::string ngettext_str(const std::string & msgid, const std::string & msgid_plural, uint plural) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end() && it->second.is_plural()) {
      return get(it->second, plural);
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  std::string pgettext_str(const std::string & context, const std::string & msgid) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end()) {
      return get(it->second);
    } else {
      return msgid;
    }
  }

  std::string npgettext_str(const std::string & context, const std::string & msgid, const std::string & msgid_plural, uint plural) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end() && it->second.is_plural()) {
      return get(it->second, plural);
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  /***
   * Get line numbers of messages
   */
  std::size_t gettext_line_no(const std::string & msgid) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end()) {
      return it->second.line_no;
    } else {
      return 0;
    }
  }

  std::size_t pgettext_line_no(const std::string & context, const std::string & msgid) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end()) {
      return it->second.line_no;
    } else {
      return 0;
    }
  }

  /***
   * Access metadata
   */
  const catalog_metadata & get_metadata() const { return metadata_; }

  /***
   * Catalog size
   */
  uint size() const {
    // exclude po header from the count, this is how msgfmt reports size also
    return hashmap_.size() - hashmap_.count("");
  }

  /***
   * Debugging output
   */
  const hashmap_type & get_hashmap() const { return hashmap_; }

  /***
   * Set warning channel (for msgid overwrites)
   */
  void set_warning_channel(const warning_channel_type & w) { warning_channel_ = w; }

  /***
   * Merge a different catalog into this one
   */
  template <typename H, typename P>
  void merge(catalog<H, P> && other) {
    std::string maybe_error = metadata_.check_compatibility(other.metadata_);
    if (maybe_error.size()) {
      SPIRIT_PO_CATALOG_FAIL(("Cannot merge catalogs: " + maybe_error));
    }
    for (auto & p : other.hashmap_) {
      if (p.first.size()) { // don't copy over the header, keep our original header
        insert_message(std::move(p.second));
      }
    }
  }
};

} // end namespace spirit_po

#endif // SPIRIT_PO_CATALOG_HPP_INCLUDED
