//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(SPIRIT_LEX_ACTION_NOV_18_2007_0743PM)
#define SPIRIT_LEX_ACTION_NOV_18_2007_0743PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/lex/set_state.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace lex
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Char>
        struct set_state_functor
        {
            set_state_functor (Char const* new_state_)
              : new_state(new_state_)
            {
            }
            
            template <typename Range, typename LexerContext>
            void operator()(Range const&, std::size_t, bool&, 
                LexerContext& ctx) const
            {
                ctx.set_state_name(new_state);
            }
            
            Char const* new_state;
        };
        
        ///////////////////////////////////////////////////////////////////////
        template <typename Char>
        set_state_functor<Char> 
        make_set_state(Char const* new_state)
        {
            return set_state_functor<Char>(new_state);
        }

        template <typename Char, typename Traits, typename Alloc>
        set_state_functor<Char> 
        make_set_state(std::basic_string<Char, Traits, Alloc> const& new_state)
        {
            return set_state_functor<Char>(new_state.c_str());
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename LexerDef, typename F>
        inline void add_action_helper(LexerDef& lexdef, std::size_t id, F act)
        {
            lexdef.add_action(id, act);
        }
        
        template <typename LexerDef, typename String>
        inline void add_action_helper(LexerDef& lexdef, std::size_t id, 
            spirit::tag::set_state_tag<String> t)
        {
            lexdef.add_action(id, make_set_state(t.name));
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    struct action
    {
        template <typename Component, typename LexerDef, typename String>
        static void 
        collect(Component const& component, LexerDef& lexdef, 
            String const& state)
        {
            typedef typename
                result_of::left<Component>::type::director
            director;
            
            // first collect the token definition information for the token_def 
            // this action is attached to
            director::collect(spirit::left(component), lexdef, state);

            // retrieve the id of the associated token_def and register the 
            // given semantic action with the lexer instance
            std::size_t id = director::id(spirit::left(component));
            detail::add_action_helper(lexdef, id, spirit::right(component));
        }
    };
    
}}} // namespace boost::spirit::lex

#endif
