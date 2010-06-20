//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_QI_CONFIX_AUG_26_2008_1012AM)
#define BOOST_SPIRIT_QI_CONFIX_AUG_26_2008_1012AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/auxiliary/confix.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace qi
{
    ///////////////////////////////////////////////////////////////////////////
    // the director for a confix() generated parser
    struct confix_director
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

    private:
        ///////////////////////////////////////////////////////////////////////
        template <
            typename Iterator, typename Context
          , typename Skipper, typename Expr>
        static void parse_helper(
            Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper, Expr const& e)
        {
            BOOST_MPL_ASSERT_MSG(
                (spirit::traits::is_component<qi::domain, Expr>::value), 
                expression_is_not_convertible_to_a_parser, (Context, Expr));

            typedef 
                typename result_of::as_component<qi::domain, Expr>::type 
            expr;

            expr eg = spirit::as_component(qi::domain(), e);
            typedef typename expr::director director;
            director::parse(eg, first, last, context, skipper, unused);
        }

        template <typename Context, typename Expr>
        static std::string what_helper(Expr const& e, Context& ctx)
        {
            typedef 
                typename result_of::as_component<qi::domain, Expr>::type 
            expr;

            expr eg = spirit::as_component(qi::domain(), e);
            typedef typename expr::director director;
            return director::what(eg, ctx);
        }

    public:
        ///////////////////////////////////////////////////////////////////////
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
            // parse the prefix
            parse_helper(first, last, context, skipper, 
                spirit::detail::confix_extractor::prefix(
                    proto::arg_c<0>(spirit::argument1(component))));

            // generate the embedded items
            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;
            bool result = director::parse(spirit::subject(component), 
                first, last, context, skipper, attr);

            // append the suffix 
            parse_helper(first, last, context, skipper, 
                spirit::detail::confix_extractor::suffix(
                    proto::arg_c<0>(spirit::argument1(component))));

            return result;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "confix(";

            result += what_helper(spirit::detail::confix_extractor::prefix(
                    proto::arg_c<0>(spirit::argument1(component))), ctx);
            result += ", ";

            result += what_helper(spirit::detail::confix_extractor::suffix(
                    proto::arg_c<0>(spirit::argument1(component))), ctx);
            result += ")[";

            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;
            result += director::what(spirit::subject(component), ctx);
            result += "]";

            return result;
        }
    };

}}}

#endif
