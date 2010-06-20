//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_LEXER_MAR_13_2007_0145PM)
#define BOOST_SPIRIT_LEX_LEXER_MAR_13_2007_0145PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/support/safe_bool.hpp>
#include <boost/spirit/home/lex/lexer/lexer_fwd.hpp>
#include <boost/spirit/home/lex/lexer/terminal_holder.hpp>
#include <boost/spirit/home/lex/lexer/token_def.hpp>
#include <boost/noncopyable.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/assert.hpp>
#include <boost/mpl/assert.hpp>
#include <string>
#include <boost/range/iterator_range.hpp>

namespace boost { namespace spirit { namespace lex
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename LexerDef>
        struct lexer_def_
          : public proto::extends<
                typename make_terminal_holder<
                    lexer_def_<LexerDef> const*, lexer_def_<LexerDef>
                >::type,
                lexer_def_<LexerDef>
            >
        {
        private:
            // avoid warnings about using 'this' in constructor
            lexer_def_& this_() { return *this; }    

            // initialize proto base class
            typedef 
                terminal_holder<lexer_def_ const*, lexer_def_> 
            terminal_holder_;
            typedef typename proto::terminal<terminal_holder_>::type tag;
            typedef proto::extends<tag, lexer_def_> base_type;

            typedef typename LexerDef::id_type id_type;
            
            tag make_tag() const
            {
                tag xpr = {{ this }};
                return xpr;
            }
            
            typedef typename LexerDef::char_type char_type;
            typedef typename LexerDef::string_type string_type;

        public:
            // Qi interface: metafunction calculating parser return type
            template <typename Component, typename Context, typename Iterator>
            struct attribute
            {
                //  the return value of a token set contains the matched token 
                //  id, and the corresponding pair of iterators
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

                    token_type &t = *first;
                    if (token_is_valid(t)) {
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
                return "lexer";
            }

            // allow to use the lexer.self.add("regex1", id1)("regex2", id2);
            // syntax
            struct adder
            {
                adder(lexer_def_& def_) 
                : def(def_)
                {}

                adder const&
                operator()(char_type c, id_type token_id = 0) const
                {
                    if (0 == token_id)
                        token_id = static_cast<id_type>(c);
                    def.def.add_token (def.state.c_str(), lex::detail::escape(c), 
                        token_id);
                    return *this;
                }
                adder const&
                operator()(string_type const& s, id_type token_id = id_type()) const
                {
                    if (0 == token_id)
                        token_id = next_id<id_type>::get();
                    def.def.add_token (def.state.c_str(), s, token_id);
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
                    
                    def.define(tokdef);
                    return *this;
                }
                template <typename TokenSet>
                adder const&
                operator()(token_set<TokenSet>& tokset) const
                {
                    def.define(tokset);
                    return *this;
                }

                lexer_def_& def;
            };
            friend struct adder;
            
            // allow to use lexer.self.add_pattern("pattern1", "regex1")(...);
            // syntax
            struct pattern_adder
            {
                pattern_adder(lexer_def_& def_) 
                : def(def_)
                {}

                pattern_adder const&
                operator()(string_type const& p, string_type const& s) const
                {
                    def.def.add_pattern (def.state.c_str(), p, s);
                    return *this;
                }

                lexer_def_& def;
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
                director::collect(c, def, state);
            }

            lexer_def_(LexerDef& def_, string_type const& state_)
              : base_type(make_tag()), add(this_()), add_pattern(this_()),
                def(def_), state(state_)
            {
            }

            // allow to switch states
            lexer_def_ operator()(char_type const* state) const
            {
                return lexer_def_(def, state);
            }
            lexer_def_ operator()(string_type const& state) const
            {
                return lexer_def_(def, state);
            }
            
            // allow to assign a token definition expression
            template <typename Expr>
            lexer_def_& operator= (Expr const& xpr)
            {
                typedef 
                    spirit::traits::is_component<lex::domain, Expr> 
                is_component;

                // report invalid expression error as early as possible
                BOOST_MPL_ASSERT_MSG(
                    is_component::value,
                    xpr_is_not_convertible_to_a_token_definition, ());

                def.clear(state.c_str());
                define(xpr);
                return *this;
            }

            adder add;
            pattern_adder add_pattern;
            
        private:
            LexerDef& def;
            string_type state;
        };
    
        // allow to assign a token definition expression
        template <typename LexerDef, typename Expr>
        inline lexer_def_<LexerDef>&
        operator+= (lexer_def_<LexerDef>& lexdef, Expr& xpr)
        {
            typedef 
                spirit::traits::is_component<lex::domain, Expr> 
            is_component;

            // report invalid expression error as early as possible
            BOOST_MPL_ASSERT_MSG(
                is_component::value,
                xpr_is_not_convertible_to_a_token_definition, ());

            lexdef.define(xpr);
            return lexdef;
        }
        
        template <typename LexerDef, typename Expr>
        inline lexer_def_<LexerDef>& 
        operator+= (lexer_def_<LexerDef>& lexdef, Expr const& xpr)
        {
            typedef 
                spirit::traits::is_component<lex::domain, Expr> 
            is_component;

            // report invalid expression error as early as possible
            BOOST_MPL_ASSERT_MSG(
                is_component::value,
                xpr_is_not_convertible_to_a_token_definition, ());

            lexdef.define(xpr);
            return lexdef;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    //  This represents a lexer definition (helper for token and token set 
    //  definitions
    ///////////////////////////////////////////////////////////////////////////
    template <typename Lexer>
    class lexer_def : private noncopyable, public Lexer
    {
    private:
        typedef lexer_def self_type;
        
        // avoid warnings about using 'this' in constructor
        lexer_def& this_() { return *this; }    

    public:        
        typedef Lexer lexer_type;
        typedef typename Lexer::id_type id_type;
        typedef detail::lexer_def_<self_type> token_set;
        typedef typename Lexer::char_type char_type;
        typedef std::basic_string<char_type> string_type;
        
        lexer_def() 
          : self(this_(), Lexer::initial_state())  
        {
        }

        token_set self;  // allow for easy token definition
        
        // this is just a dummy implementation to allow to use lexer_def 
        // directly, without having to derive a separate class
        void def(token_set& /*self*/) {}
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  This represents a lexer object
    ///////////////////////////////////////////////////////////////////////////
    template <typename Definition>
    class lexer : public safe_bool<lexer<Definition> >
    {
    public:
        // operator_bool() is needed for the safe_bool base class
        bool operator_bool() const { return token_def; }

        typedef typename Definition::lexer_type lexer_type;
        typedef typename Definition::char_type char_type;
        typedef typename Definition::iterator_type iterator_type;
        typedef typename Definition::id_type id_type;

        lexer(Definition& token_def_)
          : token_def(token_def_) 
        {
            // call initialization routine supplied by the target lexer
            token_def.def(token_def.self);
        }

        // access iterator interface
        template <typename Iterator>
        iterator_type begin(Iterator& first, Iterator const& last) const
            { return token_def.begin(first, last); }
        iterator_type end() const { return token_def.end(); }
    
        std::size_t map_state(char_type const* state)
            { return token_def.add_state(state); }
        
        Definition& get_definition() { return token_def; }
        Definition  const& get_definition() const { return token_def; }
        
    private:
        Definition& token_def;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  Metafunction returning the iterator type of the lexer given the token 
    //  definition type.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Definition>
    struct lexer_iterator
    {
        typedef typename lexer<Definition>::iterator_type type;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  Generator function helping to construct a proper lexer object 
    //  instance
    ///////////////////////////////////////////////////////////////////////////
    template <typename Definition>
    inline lexer<Definition> 
    make_lexer(Definition& def)
    {
        return lexer<Definition>(def);
    }
    
}}}

#endif
