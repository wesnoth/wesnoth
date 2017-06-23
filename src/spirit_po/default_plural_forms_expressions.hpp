//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPIRIT_PO_DEFAULT_PLURAL_FORMS_EXPRESSIONS_HPP_INCLUDED
#define SPIRIT_PO_DEFAULT_PLURAL_FORMS_EXPRESSIONS_HPP_INCLUDED

/***
 * The namespace default_plural_forms contains all the details to implement
 * the subset of the C grammar used by standard GNU gettext po headers.
 *
 * Boolean expressions return uint 0 or 1.
 *
 * The 'compiler' is a spirit grammar which parses a string into an expression
 * object. The expressions are evaluated by a simple stack machine.
 */

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <algorithm>
#include <vector>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#ifdef SPIRIT_PO_DEBUG
#include <cassert>
#include <string>
#endif

namespace spirit_po {

namespace qi = boost::spirit::qi;
typedef unsigned int uint;

namespace default_plural_forms {

// X Macro for repetitive binary ops declarations

#define FOREACH_SPIRIT_PO_BINARY_OP(X_) \
 X_(eq_op, ==) X_(neq_op, !=) X_(ge_op, >=) X_(le_op, <=) X_(gt_op, >) X_(lt_op, <) X_(mod_op, %)

// && and || are treated slightly differently from other binary ops

#define FOREACH_SPIRIT_PO_CONJUNCTION(X_) \
 X_(and_op, &&) X_(or_op, ||)

/***
 * Declare / forward declare expr struct types
 */

struct constant { uint value; };
struct n_var { n_var() = default; explicit n_var(char) {}}; // work around a quirk in spirit
struct not_op;
struct ternary_op;

#define FWD_DECL_(name, op) \
struct name ; \

FOREACH_SPIRIT_PO_BINARY_OP(FWD_DECL_)
FOREACH_SPIRIT_PO_CONJUNCTION(FWD_DECL_)

#undef FWD_DECL_

/***
 * Define expr variant type
 */

#define WRAP_(name, op) boost::recursive_wrapper< name >, \

typedef boost::variant<constant, n_var, boost::recursive_wrapper<not_op>, 
FOREACH_SPIRIT_PO_BINARY_OP(WRAP_)
FOREACH_SPIRIT_PO_CONJUNCTION(WRAP_)
boost::recursive_wrapper<ternary_op>> expr;

#undef WRAP_

/***
 * Define structs
 */

struct not_op { expr e1; };
struct ternary_op { expr e1, e2, e3; };

#define DECL_(name, op) \
struct name { expr e1, e2; }; \

FOREACH_SPIRIT_PO_BINARY_OP(DECL_)
FOREACH_SPIRIT_PO_CONJUNCTION(DECL_)

#undef DECL_

/***
 * Visitor that naively evaluates expressions
 */
struct evaluator : public boost::static_visitor<uint> {
  uint n_value_;
  explicit evaluator(uint n) : n_value_(n) {}

  uint operator()(const constant & c) const { return c.value; }
  uint operator()(n_var) const { return n_value_; }
  uint operator()(const not_op & op) const { return !boost::apply_visitor(*this, op.e1); }

#define EVAL_OP_(name, OPERATOR) \
  uint operator()(const name & op) const { return (boost::apply_visitor(*this, op.e1)) OPERATOR (boost::apply_visitor(*this, op.e2)); } \

FOREACH_SPIRIT_PO_BINARY_OP(EVAL_OP_)
FOREACH_SPIRIT_PO_CONJUNCTION(EVAL_OP_)
#undef EVAL_OP_

  uint operator()(const ternary_op & op) const { return boost::apply_visitor(*this, op.e1) ? boost::apply_visitor(*this, op.e2) : boost::apply_visitor(*this, op.e3); }
};

} // end namespace default_plural_forms

} // end namespace spirit_po

/***
 * Adapt structs for fusion / qi
 */

BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::constant,
  (spirit_po::uint, value))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::not_op,
  (spirit_po::default_plural_forms::expr, e1))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::ternary_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2)
  (spirit_po::default_plural_forms::expr, e3))

#define ADAPT_STRUCT_(name, op) \
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms:: name, \
  (spirit_po::default_plural_forms::expr, e1) \
  (spirit_po::default_plural_forms::expr, e2)) \

FOREACH_SPIRIT_PO_BINARY_OP(ADAPT_STRUCT_)
FOREACH_SPIRIT_PO_CONJUNCTION(ADAPT_STRUCT_)

#undef ADAPT_STRUCT_

namespace spirit_po {

namespace default_plural_forms {

/***
 * Pseudo-C Grammar
 *
 * Note that the grammar has been somewhat optimized by using local variables
 * and inherited attributes, in order to avoid exponential backtracking overhead.
 * This makes it a little harder to read than if we got rid of all local variables,
 * but then it is too slow to parse the expressions for certain languages.
 *
 * The main idea is that instead of parsing things like
 *
 *   BINARY_OP = LOWER_PRECENDENCE >> BINARY_OP_LITERAL >> CURRENT_PRECEDENCE
 *   CURRENT_PRECEDENCE = BINARY_OP | OTHER_OP | YET_ANOTHER_OP | LOWER_PRECEDENCE
 *
 * (which is bad because if the binary op literal is not there then we have to
 *  backtrack through an entire subexpression)
 *
 * we make BINARY_OP take the subexpression as a parameter, and in each
 * precedence level, we capture the subexpression first and store it in a local
 * variable, so that it does not get reparsed when we backtrack.
 *
 *   BINARY_OP = BINARY_OP_LITERAL >> qi::attr(parameter) >> CURRENT_PRECEDENCE
 *
 *   CURRENT_PRECEDENCE = LOWER_PRECEDENCE[local_var = result] >>
 *     (BINARY_OP(local_var) | OTHER_OP(local_var) | YET_ANOTHER_OP(local_var) | qi::attr(local_var)
 * 
 */

template <typename Iterator>
struct op_grammar : qi::grammar<Iterator, expr(), qi::space_type> {
  qi::rule<Iterator, constant(), qi::space_type> constant_;
  qi::rule<Iterator, n_var(), qi::space_type> n_;
  qi::rule<Iterator, not_op(), qi::space_type> not_;
  qi::rule<Iterator, and_op(expr), qi::space_type> and_;
  qi::rule<Iterator, or_op(expr), qi::space_type> or_;
  qi::rule<Iterator, eq_op(expr), qi::space_type> eq_;
  qi::rule<Iterator, neq_op(expr), qi::space_type> neq_;
  qi::rule<Iterator, ge_op(expr), qi::space_type> ge_;
  qi::rule<Iterator, le_op(expr), qi::space_type> le_;
  qi::rule<Iterator, gt_op(expr), qi::space_type> gt_;
  qi::rule<Iterator, lt_op(expr), qi::space_type> lt_;
  qi::rule<Iterator, mod_op(expr), qi::space_type> mod_;
  qi::rule<Iterator, ternary_op(expr), qi::space_type> ternary_;
  qi::rule<Iterator, expr(), qi::space_type> paren_expr_;

  // expression precedence levels
  qi::rule<Iterator, expr(), qi::space_type, qi::locals<expr>> ternary_level_;
  qi::rule<Iterator, expr(), qi::space_type, qi::locals<expr>> or_level_;
  qi::rule<Iterator, expr(), qi::space_type, qi::locals<expr>> and_level_;
  qi::rule<Iterator, expr(), qi::space_type, qi::locals<expr>> eq_level_;
  qi::rule<Iterator, expr(), qi::space_type, qi::locals<expr>> rel_level_;
  qi::rule<Iterator, expr(), qi::space_type, qi::locals<expr>> mod_level_;
  qi::rule<Iterator, expr(), qi::space_type> atom_level_;
  qi::rule<Iterator, expr(), qi::space_type> expr_;

  // handle optional ';' at end
  qi::rule<Iterator, expr(), qi::space_type> main_;

  op_grammar() : op_grammar::base_type(main_) {
    using qi::attr;
    using qi::lit;

    constant_ = qi::uint_;
    n_ = qi::char_('n');
    paren_expr_ = lit('(') >> expr_ >> lit(')');
    not_ = lit('!') >> atom_level_;
    atom_level_ = paren_expr_ | not_ | n_ | constant_;

    mod_ = lit('%') >> attr(qi::_r1) >> atom_level_; 
    mod_level_ = qi::omit[atom_level_[qi::_a = qi::_1]] >> (mod_(qi::_a) | attr(qi::_a));

    ge_ = lit(">=") >> attr(qi::_r1) >> mod_level_;
    le_ = lit("<=") >> attr(qi::_r1) >> mod_level_;
    gt_ = lit('>') >> attr(qi::_r1) >> mod_level_;
    lt_ = lit('<') >> attr(qi::_r1) >> mod_level_;
    rel_level_ = qi::omit[mod_level_[qi::_a = qi::_1]] >> (ge_(qi::_a) | le_(qi::_a) | gt_(qi::_a) | lt_(qi::_a) | attr(qi::_a));

    eq_ = lit("==") >> attr(qi::_r1) >> rel_level_;
    neq_ = lit("!=") >> attr(qi::_r1) >> rel_level_;
    eq_level_ = qi::omit[rel_level_[qi::_a = qi::_1]] >> (eq_(qi::_a) | neq_(qi::_a) | attr(qi::_a));

    and_ = lit("&&") >> attr(qi::_r1) >> and_level_;
    and_level_ = qi::omit[eq_level_[qi::_a = qi::_1]] >> (and_(qi::_a) | attr(qi::_a));

    or_ = lit("||") >> attr(qi::_r1) >> or_level_;
    or_level_ = qi::omit[and_level_[qi::_a = qi::_1]] >> (or_(qi::_a) | attr(qi::_a));

    ternary_ = lit('?') >> attr(qi::_r1) >> ternary_level_ >> lit(':') >> ternary_level_;
    ternary_level_ = qi::omit[or_level_[qi::_a = qi::_1]] >> (ternary_(qi::_a) | attr(qi::_a));

    expr_ = ternary_level_;

    main_ = expr_ >> -lit(';');
  }
};

/***
 * Now define a simple stack machine to evaluate the expressions efficiently.
 *
 * First define op_codes
 */

#define ENUMERATE(X, Y) X,

enum class op_code { n_var, FOREACH_SPIRIT_PO_BINARY_OP(ENUMERATE) not_op };

#undef ENUMERATE

/// Instruction that causes us to skip upcoming instructions
struct skip {
  uint distance;
};

/// Instructions that conditionally cause us to skip upcoming instructions
struct skip_if {
  uint distance;
};

struct skip_if_not {
  uint distance;
};

/***
 * Instruction is a variant type that represents either a push_constant, branch, jump, or arithmetic op.
 */
typedef boost::variant<constant, skip, skip_if, skip_if_not, op_code> instruction;

/***
 * Debug strings for instruction set
 */
#ifdef SPIRIT_PO_DEBUG
inline std::string op_code_string(op_code oc) {
  std::string result = "[  ";
  switch (oc) {
    case op_code::n_var: {
      result += "n ";
      break;
    }
    case op_code::not_op: {
      result += "! ";
      break;
    }
#define OP_CODE_STR_CASE_(X, Y)                  \
    case op_code::X: {                           \
      result += #Y;                              \
      break;                                     \
    }

FOREACH_SPIRIT_PO_BINARY_OP(OP_CODE_STR_CASE_)

#undef OP_CODE_STR_CASE_
  }

  if (result.size() < 5) { result += ' '; } \
  result += "  :   ]";
  return result;
}

struct instruction_debug_string_maker : boost::static_visitor<std::string> {
  std::string operator()(const constant & c) const {
    return "[ push : " + std::to_string(c.value) + " ]";
  }
  std::string operator()(const skip & s) const {
    return "[ skip : " + std::to_string(s.distance) + " ]";
  }
  std::string operator()(const skip_if & s) const {
    return "[ sif  : " + std::to_string(s.distance) + " ]";
  }
  std::string operator()(const skip_if_not & s) const {
    return "[ sifn : " + std::to_string(s.distance) + " ]";
  }
  std::string operator()(const op_code & oc) const {
    return op_code_string(oc);
  }
};

inline std::string debug_string(const instruction & i) {
  return boost::apply_visitor(instruction_debug_string_maker{}, i);
}

#endif // SPIRIT_PO_DEBUG

/***
 * Helper: Check if an expression obviously is zero-one valued
 */
struct is_boolean : public boost::static_visitor<bool> {
  bool operator()(const and_op &) const { return true; }
  bool operator()(const or_op &) const { return true; }
  bool operator()(const not_op &) const { return true; }
  bool operator()(const eq_op &) const { return true; }
  bool operator()(const neq_op &) const { return true; }
  bool operator()(const ge_op &) const { return true; }
  bool operator()(const le_op &) const { return true; }
  bool operator()(const gt_op &) const { return true; }
  bool operator()(const lt_op &) const { return true; }
  bool operator()(const n_var &) const { return false; }
  bool operator()(const constant & c) const { return (c.value == 0 || c.value == 1); }
  bool operator()(const mod_op & m) const { return boost::apply_visitor(*this, m.e1); }
  bool operator()(const ternary_op & t) const { return boost::apply_visitor(*this, t.e2) && boost::apply_visitor(*this, t.e3); }
};


/***
 * Visitor that maps expressions to instruction sequences
 */
struct emitter : public boost::static_visitor<std::vector<instruction>> {
  std::vector<instruction> operator()(const constant & c) const {
    return std::vector<instruction>{instruction{c}};
  }
  std::vector<instruction> operator()(const n_var &) const {
    return std::vector<instruction>{instruction{op_code::n_var}};
  }
  std::vector<instruction> operator()(const not_op & o) const {
    auto result = boost::apply_visitor(*this, o.e1);
    result.emplace_back(op_code::not_op);
    return result;
  }
#define EMIT_OP_(name, op) \
  std::vector<instruction> operator()(const name & o) const {        \
    auto result = boost::apply_visitor(*this, o.e1);                 \
    auto temp = boost::apply_visitor(*this, o.e2);                   \
    std::move(temp.begin(), temp.end(), std::back_inserter(result)); \
    result.emplace_back(op_code::name);                              \
    return result;                                                   \
  }

FOREACH_SPIRIT_PO_BINARY_OP(EMIT_OP_)

#undef EMIT_OP_

  /***
   * We make &&, ||, and ? shortcut
   */
  std::vector<instruction> operator()(const and_op & o) const {
    auto result = boost::apply_visitor(*this, o.e1);
    auto second = boost::apply_visitor(*this, o.e2);
    bool second_is_boolean = boost::apply_visitor(is_boolean{}, o.e2);

    uint sec_size = static_cast<uint>(second.size());
    if (!second_is_boolean) { sec_size += 2; }

    result.emplace_back(skip_if{2});
    result.emplace_back(constant{0});
    result.emplace_back(skip{sec_size});

    std::move(second.begin(), second.end(), std::back_inserter(result));
    if (!second_is_boolean) {
      result.emplace_back(op_code::not_op);
      result.emplace_back(op_code::not_op);
    }

    return result;
  }

  std::vector<instruction> operator()(const or_op & o) const {
    auto result = boost::apply_visitor(*this, o.e1);
    auto second = boost::apply_visitor(*this, o.e2);
    bool second_is_boolean = boost::apply_visitor(is_boolean{}, o.e2);

    uint sec_size = static_cast<uint>(second.size());
    if (!second_is_boolean) { sec_size += 2; }

    result.emplace_back(skip_if_not{2});
    result.emplace_back(constant{1});
    result.emplace_back(skip{sec_size});

    std::move(second.begin(), second.end(), std::back_inserter(result));
    if (!second_is_boolean) {
      result.emplace_back(op_code::not_op);
      result.emplace_back(op_code::not_op);
    }

    return result;
  }

  std::vector<instruction> operator()(const ternary_op & o) const {
    auto result = boost::apply_visitor(*this, o.e1);
    auto tbranch = boost::apply_visitor(*this, o.e2);
    auto fbranch = boost::apply_visitor(*this, o.e3);

    uint tsize = static_cast<uint>(tbranch.size());
    uint fsize = static_cast<uint>(fbranch.size());

    // We use jump if / jump if not in the way that will let us put the shorter branch first.
    if (tbranch.size() > fbranch.size()) {
       // + 1 to size because we have to put a jump at end of this branch also
      result.emplace_back(skip_if{fsize + 1});
      std::move(fbranch.begin(), fbranch.end(), std::back_inserter(result));
      result.emplace_back(skip{tsize});
      std::move(tbranch.begin(), tbranch.end(), std::back_inserter(result));
    } else {
      result.emplace_back(skip_if_not{tsize + 1});
      std::move(tbranch.begin(), tbranch.end(), std::back_inserter(result));
      result.emplace_back(skip{fsize});
      std::move(fbranch.begin(), fbranch.end(), std::back_inserter(result));
    }
    return result;
  }
};

/***
 * Actual stack machine
 */

class stack_machine : public boost::static_visitor<uint> {
  std::vector<instruction> instruction_seq_;
  std::vector<uint> stack_;
  uint n_value_;

#ifdef SPIRIT_PO_DEBUG
public:
  void debug_print_instructions() const {
    std::cerr << "Instruction sequence:\n";
    for (const auto & i : instruction_seq_) {
      std::cerr << debug_string(i) << std::endl;
    }
  }
private:

#define MACHINE_ASSERT(X)                       \
  do {                                          \
    if (!(X)) {                                 \
      std::cerr << "Stack machine failure:\n";  \
      debug_print_instructions();               \
      assert(false && #X);                      \
    }                                           \
  } while(0)

#else // SPIRIT_PO_DEBUG

#define MACHINE_ASSERT(...)  do {} while(0)

#endif // SPIRIT_PO_DEBUG

  uint pop_one() {
    MACHINE_ASSERT(stack_.size());

    uint result = stack_.back();
    stack_.resize(stack_.size() - 1);
    return result;
  }

public:
  explicit stack_machine(const expr & e)
    : instruction_seq_(boost::apply_visitor(emitter{}, e))
    , stack_()
    , n_value_()
  {}

  /***
   * operator() takes the instruction that we should execute
   * It should perform the operation adjusting the stack
   * It returns the amount by which we should increment the
   * program counter.
   */
  uint operator()(const constant & c) {
    stack_.emplace_back(c.value);
    return 1;
  }

  uint operator()(const skip & s) {
    return 1 + s.distance;
  }

  uint operator()(const skip_if & s) {
   return 1 + (pop_one() ? s.distance : 0);
  }

  uint operator()(const skip_if_not & s) {
   return 1 + (pop_one() ? 0 : s.distance);
  }

  uint operator()(op_code oc) {
    switch (oc) {
      case op_code::n_var: {
        stack_.emplace_back(n_value_);
        return 1;
      }
      case op_code::not_op: {
        MACHINE_ASSERT(stack_.size());
        stack_.back() = !stack_.back();
        return 1;
      }
#define STACK_MACHINE_CASE_(name, op)                                                                  \
      case op_code::name: {                                                                            \
        MACHINE_ASSERT(stack_.size() >= 2);                                                            \
        uint parm2 = pop_one();                                                                        \
                                                                                                       \
        if (op_code::name == op_code::mod_op) {                                                        \
          MACHINE_ASSERT(parm2 && "Division by zero when evaluating gettext plural form expression");  \
        }                                                                                              \
                                                                                                       \
        stack_.back() = (stack_.back() op parm2);                                                      \
        return 1;                                                                                      \
      }

FOREACH_SPIRIT_PO_BINARY_OP(STACK_MACHINE_CASE_)

#undef STACK_MACHINE_CASE_
    }
    MACHINE_ASSERT(false);
    return 1;
  }

  uint compute(uint arg) {
    n_value_ = arg;
    stack_.resize(0);
    uint pc = 0;
    while (pc < instruction_seq_.size()) {
      pc += boost::apply_visitor(*this, instruction_seq_[pc]);
    }
    MACHINE_ASSERT(pc == instruction_seq_.size());
    MACHINE_ASSERT(stack_.size() == 1);
    return stack_[0];
  }
};

#undef MACHINE_ASSERT

// X macros not used anymore
#undef FOREACH_SPIRIT_PO_BINARY_OP
#undef FOREACH_SPIRIT_PO_CONJUNCTION

} // end namespace default_plural_forms

} // end namespace spirit_po

#endif // SPIRIT_PO_DEFAULT_PLURAL_FORMS_EXPRESSIONS_HPP_INCLUDED
