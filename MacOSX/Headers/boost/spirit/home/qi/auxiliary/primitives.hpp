/*=============================================================================
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(BOOST_SPIRIT_PRIMITIVES_APR_18_2008_0751PM)
#define BOOST_SPIRIT_PRIMITIVES_APR_18_2008_0751PM

#include <boost/mpl/bool.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    //  the end_director_base is a base class for various end parsers
    ///////////////////////////////////////////////////////////////////////////
    template <typename Parser, typename StoreIterator = mpl::false_>
    struct end_director_base
    {
        typedef mpl::false_ stores_iterator;

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
            Component const& /*component*/
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& /*attr*/)
        {
            qi::skip(first, last, skipper);
            return Parser::test(first, last);
        }

        // subclasses are required to implement test:

        template <typename Iterator>
        bool test(Iterator& first, Iterator const& last);
    };

    ///////////////////////////////////////////////////////////////////////////
    //  same as end_director_base above, but stores iterator
    ///////////////////////////////////////////////////////////////////////////
    template <typename Parser>
    struct end_director_base<Parser, mpl::true_>
    {
        typedef mpl::true_ stores_iterator;

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
            Component const& /*component*/
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& /*attr*/)
        {
            qi::skip(first, last, skipper);

            Iterator it = first;
            if (!Parser::test(it, last))
                return false;

            first = it;
            return true;
        }

        // subclasses are required to implement test:

        template <typename Iterator>
        bool test(Iterator& first, Iterator const& last);
    };

    ///////////////////////////////////////////////////////////////////////////
    //  ~eoi, ~eol: 'not end of line' or 'not end of input'
    template <typename Positive>
    struct negated_end_director
      : end_director_base<
            negated_end_director<Positive>,
            typename Positive::director::stores_iterator
        >
    {
        template <typename Iterator>
        static bool test (Iterator& first, Iterator const& last)
        {
            return !Positive::director::test(first, last);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "not " +
                Positive::director::what(fusion::at_c<0>(component.elements), ctx);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  eoi: end of input
    struct eoi_director : end_director_base<eoi_director>
    {
        template <typename Iterator>
        static bool test (Iterator& first, Iterator const& last)
        {
            return first == last;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "eoi";
        }
    };


    ///////////////////////////////////////////////////////////////////////////
    //  the eol_director matches line endings
    ///////////////////////////////////////////////////////////////////////////
    struct eol_director : end_director_base<eol_director, mpl::true_>
    {
        template <typename Iterator>
        static bool test (Iterator& first, Iterator const& last)
        {
            bool matched = false;
            if (first != last && *first == '\r')    // CR
            {
                matched = true;
                ++first;
            }
            if (first != last && *first == '\n')    // LF
            {
                matched = true;
                ++first;
            }
            return matched;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            return "eol";
        }
    };

///////////////////////////////////////////////////////////////////////////////
}}}

#endif


