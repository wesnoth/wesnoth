//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_TOKEN_DEF_MAR_13_2007_0145PM)
#define BOOST_SPIRIT_LEX_TOKEN_DEF_MAR_13_2007_0145PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/lex/lexer/lexer_fwd.hpp>
#include <boost/spirit/home/lex/lexer/terminal_holder.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/qi/detail/construct.hpp>
#include <boost/spirit/home/qi/detail/assign_to.hpp>
#include <boost/mpl/if.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/type_traits/is_same.hpp>
#include <string>
#include <cstdlib>

namespace boost { namespace spirit { namespace lex
{
    ///////////////////////////////////////////////////////////////////////////
    //  create a unique token id, note this is not thread safe
    ///////////////////////////////////////////////////////////////////////////
    enum tokenids {
        // this is the first token id automatically assigned by the library
        // if needed
        min_token_id = 0x10000
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  The next_id template needs to be specialized for any non-default token 
    //  id type used by a custom token type. It need to expose a function
    //  'static Idtype get()' returning the next available token id each time 
    //  it is called.
    template <typename Idtype>
    struct next_id;
    
    ///////////////////////////////////////////////////////////////////////////
    //  Default specialization for the next_id template returning the next 
    //  available token id.
    template <>
    struct next_id<std::size_t>
    {
        static std::size_t get()
        {
            static std::size_t next_token_id = min_token_id;
            return next_token_id++;   
        }
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  This component represents a token definition
    ///////////////////////////////////////////////////////////////////////////
    template<typename Attribute, typename Char, typename Idtype>
    class token_def
      : public proto::extends<
            typename make_terminal_holder<
                token_def<Attribute, Char, Idtype>*, 
                token_def<Attribute, Char, Idtype>
            >::type,
            token_def<Attribute, Char, Idtype>
        >
    {
    private:
        // initialize proto base class
        typedef terminal_holder<token_def*, token_def> terminal_holder_;
        typedef typename proto::terminal<terminal_holder_>::type tag;
        typedef proto::extends<tag, token_def> base_type;

        tag make_tag() 
        {
            tag xpr = {{ this }};
            return xpr;
        }

    public:
        // Qi interface: metafunction calculating parser return type
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            //  The return value of the token_def is either the specified 
            //  attribute type, or the pair of iterators from the match of the 
            //  corresponding token (if no attribute type has been specified),
            //  or unused_type (if omitted has been specified).
            typedef typename Iterator::base_iterator_type iterator_type;
            typedef typename
                mpl::if_<
                    is_same<Attribute, unused_type>,
                    iterator_range<iterator_type>,
                    typename mpl::if_<
                        is_same<Attribute, omitted>,
                        unused_type,
                        Attribute
                    >::type
                >::type
            type;
        };

    private:
        // Qi interface: parse functionality
        template <typename Iterator, typename Context, typename Skipper, 
            typename Attribute1>
        bool parse(Iterator& first, Iterator const& last, 
            Context& /*context*/, Skipper const& skipper, Attribute1& attr) const
        {
            qi::skip(first, last, skipper);   // always do a pre-skip

            if (first != last) {
                typedef typename 
                    boost::detail::iterator_traits<Iterator>::value_type 
                token_type;

                //  If the following assertion fires you probably forgot to  
                //  associate this token definition with a lexer instance.
                BOOST_ASSERT((std::size_t)(~0) != token_state);

                token_type &t = *first;
                if (token_id == t.id() && token_state == t.state()) {
                    qi::detail::assign_to(t, attr);
                    ++first;
                    return true;
                }
            }
            return false;
        }
        friend struct terminal_director;
        
        std::string what() const
        {
            std::string result = "token_def(\"";
            result += def;
            result += "\")";
            return result;
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Lex interface: collect token definitions and put it into the 
        // provided lexer def
        template <typename LexerDef, typename String>
        void collect(LexerDef& lexdef, String const& state)
        {
            token_state = lexdef.add_state(state.c_str());
            if (0 == token_id)
                token_id = next_id<Idtype>::get();
            lexdef.add_token (state.c_str(), def, token_id);
        }
            
    public:
        typedef Char char_type;
        typedef Idtype id_type;
        typedef std::basic_string<char_type> string_type;
        
        // Lex interface: constructing token definitions
        token_def() 
          : base_type(make_tag()), token_id(0), token_state(~0) 
        {}
        explicit token_def(char_type def_, Idtype id_ = Idtype())
          : base_type(make_tag()), def(lex::detail::escape(def_)), 
            token_id(0 == id_ ? def_ : id_), token_state(~0) 
        {}
        explicit token_def(string_type def_, Idtype id_ = Idtype())
          : base_type(make_tag()), def(def_), token_id(id_), token_state(~0) 
        {}
        
        template <typename String>
        token_def& operator= (String const& definition)
        {
            def = definition;
            token_id = 0;
            return *this;
        }
        token_def& operator= (token_def const& rhs)
        {
            def = rhs.def;
            token_id = rhs.token_id;
            return *this;
        }
        
        // general accessors 
        Idtype id() const { return token_id; }
        void id(Idtype id) { token_id = id; }
        string_type const& definition() const { return def; }
        std::size_t state() const { return token_state; }
        
    private:
        string_type def;
        Idtype token_id;
        std::size_t token_state;
    };

}}}

#endif
