//  Copyright (c) 2001-2008 Hartmut Kaiser
//  Copyright (c) 2001-2007 Joel de Guzman
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_TERMINAL_DIRECTOR_MAR_22_2007_0846PM)
#define BOOST_SPIRIT_LEX_TERMINAL_DIRECTOR_MAR_22_2007_0846PM

#include <boost/spirit/home/lex/lexer/terminal_holder.hpp>
#include <boost/spirit/home/lex/domain.hpp>
#include <boost/spirit/home/support/component.hpp>

namespace boost { namespace spirit { namespace lex
{
    // this is the director for all lexer related proto terminals
    struct terminal_director
    {
        // Qi interface: return value of the parser
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            terminal_holder;
            typedef typename terminal_holder::terminal_type terminal_type;

            typedef typename
                terminal_type::template attribute<
                    terminal_holder, Context, Iterator
                >::type
            type;
        };

        // Qi interface: parse functionality, delegates back to the
        // corresponding lexer terminal
        template <typename Component, typename Iterator, typename Context,
            typename Skipper, typename Attribute>
        static bool parse(Component const& component,
            Iterator& first, Iterator const& last, Context& context,
            Skipper const& skipper, Attribute& attr)
        {
            // main entry point, just forward to the lexer terminal
            return subject(component).held->parse(
                first, last, context, skipper, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return subject(component).held->what();
        }

        // Lex interface: collect functionality, delegates back to the
        // corresponding lexer terminal
        template <typename Component, typename LexerDef, typename String>
        static void collect (Component const& component, LexerDef& lexdef,
            String const& state)
        {
            subject(component).held->collect(lexdef, state);
        }

        // Lex interface: return the token id of the associated token_def
        template <typename Component>
        static std::size_t id(Component const& component)
        {
            return subject(component).held->id();
        }
    };

}}}

#endif
