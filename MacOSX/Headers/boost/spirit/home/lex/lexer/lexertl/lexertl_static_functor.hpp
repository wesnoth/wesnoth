//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_LEXER_STATIC_FUNCTOR_FEB_10_2008_0755PM)
#define BOOST_SPIRIT_LEX_LEXER_STATIC_FUNCTOR_FEB_10_2008_0755PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/mpl/bool.hpp>
#include <boost/function.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/detail/workaround.hpp>
#include <map>
#include <boost/spirit/home/support/detail/lexer/generator.hpp>
#include <boost/spirit/home/support/detail/lexer/rules.hpp>
#include <boost/spirit/home/support/detail/lexer/state_machine.hpp>
#include <boost/spirit/home/lex/lexer/lexertl/iterator_tokenizer.hpp>
#include <boost/spirit/home/lex/lexer/lexertl/wrap_action.hpp>

#if 0 != __COMO_VERSION__ || !BOOST_WORKAROUND(BOOST_MSVC, <= 1310)
#define BOOST_SPIRIT_STATIC_EOF 1
#define BOOST_SPIRIT_EOF_PREFIX static
#else
#define BOOST_SPIRIT_EOF_PREFIX 
#endif

namespace boost { namespace spirit { namespace lex 
{ 
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Iterator, typename HasActors, typename HasState>
        struct Data;    // no default specialization
        
        ///////////////////////////////////////////////////////////////////////
        //  doesn't support no state and no actors
        template <typename Iterator>
        struct Data<Iterator, mpl::false_, mpl::false_>
        {
            typedef std::size_t state_type;
            typedef iterator_range<Iterator> iterpair_type;
            typedef typename 
                boost::detail::iterator_traits<Iterator>::value_type 
            char_type;
            
            typedef std::size_t (*next_token_functor)(std::size_t&, 
                Iterator const&, Iterator&, Iterator const&);

            typedef unused_type semantic_actions_type;

            typedef 
                detail::wrap_action<unused_type, iterpair_type, Data>
            wrap_action_type;

            // initialize the shared data 
            template <typename IterData>
            Data (IterData const& data_, Iterator& first_, Iterator const& last_)
              : next_token(data_.next_), first(first_), last(last_)
            {}

            std::size_t next(Iterator& end)
            {
                typedef basic_iterator_tokeniser<Iterator> tokenizer;
                std::size_t state;
                return next_token(state, first, end, last);
            }

            // nothing to invoke, so this is empty
            bool invoke_actions(std::size_t, Iterator const&) 
            {
                return true;    // always accept
            }

            std::size_t get_state() const { return 0; }
            void set_state_name (char const*) {}

            next_token_functor next_token;
            Iterator& first;
            Iterator last;
        };
        
        ///////////////////////////////////////////////////////////////////////
        //  doesn't support no actors
        template <typename Iterator>
        struct Data<Iterator, mpl::false_, mpl::true_>
          : Data<Iterator, mpl::false_, mpl::false_>
        {
            typedef Data<Iterator, mpl::false_, mpl::false_> base_type;

            typedef typename base_type::state_type state_type;
            typedef typename base_type::char_type char_type;
            typedef 
                typename base_type::semantic_actions_type 
            semantic_actions_type;

            // initialize the shared data 
            template <typename IterData>
            Data (IterData const& data_, Iterator& first_, Iterator const& last_)
              : base_type(data_, first_, last_), state(0)
            {}

            std::size_t next(Iterator& end)
            {
                typedef basic_iterator_tokeniser<Iterator> tokenizer;
                return this->next_token(state, this->first, end, this->last);
            }

            std::size_t& get_state() { return state; }
            void set_state_name (char_type const* new_state) 
            { 
                std::size_t state_id = this->rules.state(new_state);
                BOOST_ASSERT(state_id != boost::lexer::npos);
                if (state_id != boost::lexer::npos)
                    state = state_id;
            }

            std::size_t state;
        };
        
        ///////////////////////////////////////////////////////////////////////
        //  does support actors, but may have no state
        template <typename Iterator, typename HasState>
        struct Data<Iterator, mpl::true_, HasState> 
          : Data<Iterator, mpl::false_, HasState>
        {
            typedef Data<Iterator, mpl::false_, HasState> base_type;

            typedef iterator_range<Iterator> iterpair_type;
            typedef typename base_type::state_type state_type;
            typedef typename base_type::char_type char_type;

            typedef void functor_type(iterpair_type, std::size_t, bool&, Data&);
            typedef boost::function<functor_type> functor_wrapper_type;
            typedef std::multimap<std::size_t, functor_wrapper_type> 
                semantic_actions_type;

            typedef 
                detail::wrap_action<functor_wrapper_type, iterpair_type, Data>
            wrap_action_type;

            template <typename IterData>
            Data (IterData const& data_, Iterator& first_, Iterator const& last_)
              : base_type(data_, first_, last_),
                actions(data_.actions_)
            {}

            // invoke attached semantic actions, if defined
            bool invoke_actions(std::size_t id, Iterator const& end)
            {
                if (actions.empty()) 
                    return true;  // nothing to invoke, continue with 'match'

                iterpair_type itp(this->first, end);
                bool match = true;
                
                typedef typename 
                    semantic_actions_type::const_iterator 
                iterator_type;
                
                std::pair<iterator_type, iterator_type> p = actions.equal_range(id);
                while (p.first != p.second)
                {
                    ((*p.first).second)(itp, id, match, *this);
                    if (!match)
                        return false;   // return a 'no-match'
                    ++p.first;
                }
                return true;    // normal execution
            }
            
            semantic_actions_type const& actions;
        };
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  lexertl_static_functor is a template usable as the functor object for 
    //  the multi_pass iterator allowing to wrap a lexertl based dfa into a 
    //  iterator based interface.
    //  
    //    Iterator:   the type of the underlying iterator
    //    Token:      the type of the tokens produced by this functor
    //                this needs to expose a constructor with the following
    //                prototype:
    //
    //                Token(std::size_t id, std::size_t state, 
    //                      Iterator start, Iterator end)
    //
    //                where 'id' is the token id, state is the lexer state,
    //                this token has been matched in, and 'first' and 'end'  
    //                mark the start and the end of the token with respect 
    //                to the underlying character stream.
    //    SupportsActors:
    //                this is expected to be a mpl::bool_, if mpl::true_ the
    //                lexertl_static_functor invokes functors which 
    //                (optionally) have been attached to the token definitions.
    //    SupportState:
    //                this is expected to be a mpl::bool_, if mpl::true_ the
    //                lexertl_functor supports different lexer states, 
    //                otherwise no lexer state is supported.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename Token, 
        typename Iterator = typename Token::iterator_type, 
        typename SupportsActors = mpl::false_,
        typename SupportsState = typename Token::has_state>
    class lexertl_static_functor
    {
    public:
        typedef typename 
            boost::detail::iterator_traits<Iterator>::value_type 
        char_type;

    private:
        // Needed by compilers not implementing the resolution to DR45. For
        // reference, see
        // http://www.open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#45.
        template <typename Iterator_, typename HasActors, typename HasState> 
        friend struct detail::Data;

        // Helper template allowing to assign a value on exit
        template <typename T>
        struct assign_on_exit
        {
            assign_on_exit(T& dst_, T const& src_)
              : dst(dst_), src(src_)
            {}
            ~assign_on_exit()
            {
                dst = src;
            }
            
            T& dst;
            T const& src;
        };
        
    public:
        lexertl_static_functor()
#if defined(__PGI)
          : eof()
#endif 
        {}

#if BOOST_WORKAROUND(BOOST_MSVC, <= 1310)
        // somehow VC7.1 needs this (meaningless) assignment operator
        lexertl_functor& operator=(lexertl_functor const& rhs)
        {
            return *this;
        }
#endif

        ///////////////////////////////////////////////////////////////////////
        // interface to the multi_pass_policies::split_functor_input policy
        typedef Token result_type;
        typedef lexertl_static_functor unique;
        typedef detail::Data<Iterator, SupportsActors, SupportsState> shared;
        
        BOOST_SPIRIT_EOF_PREFIX result_type const eof;
    
        ///////////////////////////////////////////////////////////////////////
        typedef Iterator iterator_type;
        typedef typename shared::semantic_actions_type semantic_actions_type;
        typedef typename shared::next_token_functor next_token_functor;

        // this is needed to wrap the semantic actions in a proper way
        typedef typename shared::wrap_action_type wrap_action_type;

        ///////////////////////////////////////////////////////////////////////
        template <typename MultiPass>
        static result_type& get_next(MultiPass& mp, result_type& result)
        {
            shared& data = mp.shared->ftor;
            if (data.first == data.last) 
#if defined(BOOST_SPIRIT_STATIC_EOF)
                return result = eof;
#else
                return result = mp.ftor.eof;
#endif

            Iterator end = data.first;
            std::size_t id = data.next(end);
            
            if (boost::lexer::npos == id) {   // no match
#if defined(BOOST_SPIRIT_LEXERTL_DEBUG)
                std::string next;
                Iterator it = data.first;
                for (std::size_t i = 0; i < 10 && it != data.last; ++it, ++i)
                    next += *it;

                std::cerr << "Not matched, in state: " << data.state 
                          << ", lookahead: >" << next << "<" << std::endl;
#endif
                return result = result_type(0);
            }
            else if (0 == id) {         // EOF reached
#if defined(BOOST_SPIRIT_STATIC_EOF)
                return result = eof;
#else
                return result = mp.ftor.eof;
#endif
            }
            
#if defined(BOOST_SPIRIT_LEXERTL_DEBUG)
            {
                std::string next;
                Iterator it = data.first;
                for (std::size_t i = 0; i < 10 && it != data.last; ++it, ++i)
                    next += *it;

                std::cerr << "Matched: " << id << ", in state: " 
                          << data.state << ", string: >" 
                          << std::basic_string<char_type>(data.first, end) << "<"
                          << ", lookahead: >" << next << "<" << std::endl;
            }
#endif
            // invoke_actions might change state
            std::size_t state = data.get_state();

            // invoke attached semantic actions, if there are any defined
            if (!data.invoke_actions(id, end))
            {
                // one of the semantic actions signaled no-match
                return result = result_type(0); 
            }
            
            // return matched token, advancing 'data.first' past the matched 
            // sequence
            assign_on_exit<Iterator> on_exit(data.first, end);
            return result = result_type(id, state, data.first, end);
        }

        // set_state is propagated up to the iterator interface, allowing to 
        // manipulate the current lexer state through any of the exposed 
        // iterators.
        template <typename MultiPass>
        static std::size_t set_state(MultiPass& mp, std::size_t state_) 
        { 
            std::size_t oldstate = mp.shared->ftor.state;
            mp.shared->ftor.state = state_;

#if defined(BOOST_SPIRIT_LEXERTL_DEBUG)
            std::cerr << "Switching state from: " << oldstate 
                      << " to: " << state_
                      << std::endl;
#endif
            return oldstate; 
        }
        
        template <typename MultiPass>
        static std::size_t 
        map_state(MultiPass const& mp, char_type const* statename)  
        { 
            return mp.shared->ftor.rules.state(statename);
        }
        
        // we don't need this, but it must be there
        template <typename MultiPass>
        static void destroy(MultiPass const&)
        {}  
    };

#if defined(BOOST_SPIRIT_STATIC_EOF)
    ///////////////////////////////////////////////////////////////////////////
    //  eof token
    ///////////////////////////////////////////////////////////////////////////
    template <typename Token, typename Iterator, typename SupportsActors, 
        typename SupportsState>
    typename lexertl_static_functor<
            Token, Iterator, SupportsActors, SupportsState>::result_type const
        lexertl_static_functor<
                Token, Iterator, SupportsActors, SupportsState>::eof = 
            typename lexertl_static_functor<
                Token, Iterator, SupportsActors, SupportsState>::result_type();
#endif

}}}

#undef BOOST_SPIRIT_EOF_PREFIX
#undef BOOST_SPIRIT_STATIC_EOF

#endif
