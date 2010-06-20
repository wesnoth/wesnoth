/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_GRAMMAR_FEB_19_2007_0236PM)
#define BOOST_SPIRIT_GRAMMAR_FEB_19_2007_0236PM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/qi/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/qi/nonterminal/grammar_fwd.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#include <boost/spirit/home/qi/nonterminal/nonterminal_director.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/is_convertible.hpp>

namespace boost { namespace spirit { namespace qi
{
    template <typename Iterator, typename T0 , typename T1 , typename T2>
    struct grammar
      : nonterminal<
            grammar<Iterator, T0, T1, T2>
          , typename qi::rule<Iterator, T0, T1, T2>::sig_type
          , typename qi::rule<Iterator, T0, T1, T2>::locals_type
        >, noncopyable
    {
        typedef Iterator iterator_type;
        typedef qi::rule<Iterator, T0, T1, T2> start_type;
        typedef typename start_type::sig_type sig_type;
        typedef typename start_type::locals_type locals_type;
        typedef typename start_type::skipper_type skipper_type;
        typedef grammar<Iterator, T0, T1, T2> base_type;

        grammar(start_type const& start, std::string const& name_ = std::string())
          : start(start), name_(name_) {}

        std::string name() const
        {
            return name_;
        }

        void name(std::string const& name__)
        {
            name_ = name__;
        }

        start_type const& start;
        std::string name_;

    private:

        template <typename Iterator_, typename Context, typename Skipper>
        bool parse(
            Iterator_& first, Iterator_ const& last
          , Context& context, Skipper const& skipper) const
        {
            return start.parse(first, last, context, skipper);
        }

        std::string what() const
        {
            if (name().empty())
            {
                return start.what();
            }
            else
            {
                return name();
            }
        }

        friend struct nonterminal_director;
    };
}}}

#endif
