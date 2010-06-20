//  Copyright (c) 2001-2007 Joel de Guzman
//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_GRAMMAR_MAR_05_2007_0542PM)
#define BOOST_SPIRIT_KARMA_GRAMMAR_MAR_05_2007_0542PM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/karma/nonterminal/nonterminal.hpp>
#include <boost/spirit/home/karma/nonterminal/grammar_fwd.hpp>
#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/nonterminal/rule.hpp>
#include <boost/spirit/home/karma/nonterminal/nonterminal_director.hpp>
#include <boost/function_types/is_function.hpp>
#include <boost/noncopyable.hpp>

namespace boost { namespace spirit { namespace karma
{
    template <typename Iterator, typename T0 , typename T1 , typename T2>
    struct grammar
      : nonterminal<
            grammar<Iterator, T0, T1, T2>,
            typename karma::rule<Iterator, T0, T1, T2>::sig_type,
            typename karma::rule<Iterator, T0, T1, T2>::locals_type
        >, noncopyable
    {
        typedef Iterator iterator_type;
        typedef karma::rule<Iterator, T0, T1, T2> start_type;
        typedef typename start_type::sig_type sig_type;
        typedef typename start_type::locals_type locals_type;
        typedef typename start_type::delimiter_type delimiter_type;
        typedef grammar<Iterator, T0, T1, T2> base_type;

        grammar(start_type const& start, std::string const& name_ = std::string())
          : start_(start), name_(name_) 
        {}

        std::string name() const
        {
            return name_;
        }

        void name(std::string const& name__)
        {
            name_ = name__;
        }

        start_type const& start_;
        std::string name_;

    private:
        template <typename OutputIterator, typename Context, typename Delimiter>
        bool generate(OutputIterator& sink, Context& context, 
            Delimiter const& delim) const
        {
            return start_.generate(sink, context, delim);
        }

        std::string what() const
        {
            if (name().empty())
            {
                return start_.what();
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
