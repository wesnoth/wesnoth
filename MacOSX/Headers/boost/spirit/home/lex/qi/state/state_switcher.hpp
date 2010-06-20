//  Copyright (c) 2001-2008 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_STATE_SWITCHER_SEP_23_2007_0714PM)
#define BOOST_SPIRIT_LEX_STATE_SWITCHER_SEP_23_2007_0714PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/lex/set_state.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/detail/to_narrow.hpp>

namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    //  parser, which switches the state of the underlying lexer component
    //  this parser gets used for the set_state(...) construct.
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename Iterator>
        inline std::size_t
        set_lexer_state(Iterator& it, std::size_t state)
        {
            return it.set_state(state);
        }

        template <typename Iterator, typename Char>
        inline std::size_t
        set_lexer_state(Iterator& it, Char const* statename)
        {
            std::size_t state = it.map_state(statename);

            //  If the following assertion fires you probably used the
            //  set_state(...) or in_state(...)[...] lexer state switcher with
            //  a lexer state name unknown to the lexer (no token definitions
            //  have been associated with this lexer state).
            BOOST_ASSERT(static_cast<std::size_t>(~0) != state);
            return it.set_state(state);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    struct state_switcher
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef unused_type type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& /*attr*/)
        {
            qi::skip(first, last, skipper);   // always do a pre-skip

            // just switch the state and return success
            detail::set_lexer_state(first, spirit::left(component).name);
            return true;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result("set_state(\"");
            result += spirit::detail::to_narrow_string(
                spirit::left(component).name);
            result += "\")";
            return result;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Parser, which switches the state of the underlying lexer component
    // for the execution of the embedded sub-parser, switching the state back
    // afterwards. This parser gets used for the in_state(...)[p] construct.
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename Iterator>
        struct reset_state_on_exit
        {
            template <typename State>
            reset_state_on_exit(Iterator& it_, State state_)
              : it(it_), state(detail::set_lexer_state(it_, state_))
            {
            }
            ~reset_state_on_exit()
            {
                // reset the state of the underlying lexer instance
                it.set_state(state);
            }

            Iterator& it;
            std::size_t state;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    struct state_switcher_context
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                traits::attribute_of<
                    qi::domain, subject_type, Context, Iterator>::type
            type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr)
        {
            qi::skip(first, last, skipper);   // always do a pre-skip

            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;

            // the state has to be reset at exit in any case
            detail::reset_state_on_exit<Iterator> guard(
                first, proto::arg_c<0>(argument1(component)).name);

            return director::parse(spirit::subject(component), first,
                last, context, skipper, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result("in_state(\"");
            result += spirit::detail::to_narrow_string(
                proto::arg_c<0>(argument1(component)).name);
            result += "\")[";

            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;

            result += director::what(subject(component), ctx);
            result += "]";
            return result;
        }
    };

}}}

#endif
