//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_TOKEN_SET_MAR_13_2007_0145PM)
#define BOOST_SPIRIT_LEX_TOKEN_SET_MAR_13_2007_0145PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/lex/lexer/lexer_fwd.hpp>
#include <boost/spirit/home/lex/lexer/terminal_holder.hpp>
#include <boost/spirit/home/lex/lexer/token_def.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/range/iterator_range.hpp>
#include <cstdlib>

namespace boost { namespace spirit { namespace lex
{
    ///////////////////////////////////////////////////////////////////////////
    //  token_set
    ///////////////////////////////////////////////////////////////////////////
    template <typename TokenSet>
    class token_set
      : public TokenSet,
        public proto::extends<
            typename make_terminal_holder<
                token_set<TokenSet>*, token_set<TokenSet>
            >::type,
            token_set<TokenSet>
        >
    {
    protected:
        typedef typename TokenSet::char_type char_type;
        typedef typename TokenSet::string_type string_type;

    private:
        // avoid warnings about using 'this' in constructor
        token_set& this_() { return *this; }

        typedef token_set self_type;
        typedef TokenSet base_token_set;

        // initialize proto base class
        typedef terminal_holder<token_set*, token_set> terminal_holder_;
        typedef typename proto::terminal<terminal_holder_>::type tag;
        typedef proto::extends<tag, token_set> base_type;

        tag make_tag()
        {
            tag xpr = {{ this }};
            return xpr;
        }

    public:
        typedef typename TokenSet::id_type id_type;
        
        // Qi interface: metafunction calculating parser return type
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            //  the return value of a token set contains the matched token id,
            //  and the corresponding pair of iterators
            typedef typename Iterator::base_iterator_type iterator_type;
            typedef
                fusion::vector<id_type, iterator_range<iterator_type> >
            type;
        };

    private:
        // Qi interface: parse functionality
        template <typename Iterator, typename Context, typename Skipper,
            typename Attribute>
        bool parse(Iterator& first, Iterator const& last,
            Context& context, Skipper const& skipper, Attribute& attr) const
        {
            qi::skip(first, last, skipper);   // always do a pre-skip

            if (first != last) {
                typedef typename
                    boost::detail::iterator_traits<Iterator>::value_type
                token_type;

                //  If the following assertion fires you probably forgot to
                //  associate this token set definition with a lexer instance.
                BOOST_ASSERT(~0 != token_state);

                token_type &t = *first;
                if (token_is_valid(t) && token_state == t.state()) {
                // any of the token definitions matched
                    qi::detail::assign_to(t, attr);
                    ++first;
                    return true;
                }
            }
            return false;
        }
        friend struct terminal_director;

        static std::string what() 
        {
            return "token_set";
        }

        ///////////////////////////////////////////////////////////////////////
        // Lex interface: collect token definitions and put it into the
        // provided lexer def
        template <typename LexerDef, typename String>
        void collect(LexerDef& lexdef, String const& state)
        {
            token_state = lexdef.add_state(state.c_str());
            lexdef.add_token (state.c_str(), *this);
        }

        // allow to use the tokset.add("regex1", id1)("regex2", id2);
        // syntax
        struct adder
        {
            adder(token_set& def_)
            : def(def_)
            {}

            adder const&
            operator()(char_type c, id_type token_id = id_type()) const
            {
                if (0 == token_id)
                    token_id = static_cast<std::size_t>(c);
                def.add_token (def.initial_state().c_str(),
                    lex::detail::escape(c), token_id);
                return *this;
            }
            adder const&
            operator()(string_type const& s, id_type token_id = id_type()) const
            {
                if (0 == token_id)
                    token_id = next_id<id_type>::get();
                def.add_token (def.initial_state().c_str(), s, token_id);
                return *this;
            }
            template <typename Attribute>
            adder const&
            operator()(token_def<Attribute, char_type, id_type>& tokdef,
                id_type token_id = id_type()) const
            {
                // make sure we have a token id
                if (0 == token_id) {
                    if (0 == tokdef.id()) {
                        token_id = next_id<id_type>::get();
                        tokdef.id(token_id);
                    }
                    else {
                        token_id = tokdef.id();
                    }
                }
                else { 
                // the following assertion makes sure, that the token_def
                // instance has not been assigned a different id earlier
                    BOOST_ASSERT(0 == tokdef.id() || token_id == tokdef.id());
                    tokdef.id(token_id);
                }

                def.add_token (def.initial_state().c_str(), tokdef.definition(),
                    token_id);
                return *this;
            }
            template <typename TokenSet_>
            adder const&
            operator()(token_set<TokenSet_> const& tokset) const
            {
                def.add_token (def.initial_state().c_str(), tokset);
                return *this;
            }

            token_set& def;
        };
        friend struct adder;

        // allow to use lexer.self.add_pattern("pattern1", "regex1")(...);
        // syntax
        struct pattern_adder
        {
            pattern_adder(token_set& def_) 
            : def(def_)
            {}

            pattern_adder const&
            operator()(string_type const& p, string_type const& s) const
            {
                def.add_pattern (def.state.c_str(), p, s);
                return *this;
            }

            token_set& def;
        };
        friend struct pattern_adder;
            
    public:
        ///////////////////////////////////////////////////////////////////
        template <typename Expr>
        void define(Expr const& xpr)
        {
            typedef typename
                result_of::as_component<lex::domain, Expr>::type
            component;
            typedef typename component::director director;

            component c = spirit::as_component(lex::domain(), xpr);
            director::collect(c, *this, base_token_set::initial_state());
        }

        token_set()
          : base_type(make_tag()), add(this_()), add_pattern(this_()), 
            token_state(~0)
        {}

        // allow to assign a token definition expression
        template <typename Expr>
        token_set& operator= (Expr const& xpr)
        {
            typedef
                spirit::traits::is_component<lex::domain, Expr>
            is_component;

            // report invalid expression error as early as possible
            BOOST_MPL_ASSERT_MSG(
                is_component::value,
                xpr_is_not_convertible_to_a_token_definition, ());

            this->clear();
            define(xpr);
            return *this;
        }

        adder add;
        pattern_adder add_pattern;

    private:
        std::size_t token_state;
    };

    // allow to assign a token definition expression
    template <typename TokenSet, typename Expr>
    inline token_set<TokenSet>&
    operator+= (token_set<TokenSet>& tokset, Expr& xpr)
    {
        typedef
            spirit::traits::is_component<lex::domain, Expr>
        is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(
            is_component::value,
            xpr_is_not_convertible_to_a_token_definition, ());

        tokset.define(xpr);
        return tokset;
    }
    template <typename TokenSet, typename Expr>
    inline token_set<TokenSet>&
    operator+= (token_set<TokenSet>& tokset, Expr const& xpr)
    {
        typedef
            spirit::traits::is_component<lex::domain, Expr>
        is_component;

        // report invalid expression error as early as possible
        BOOST_MPL_ASSERT_MSG(
            is_component::value,
            xpr_is_not_convertible_to_a_token_definition, ());

        tokset.define(xpr);
        return tokset;
    }

}}}

#endif
