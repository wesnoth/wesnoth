//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_CONFIX_AUG_19_2008_1041AM)
#define BOOST_SPIRIT_KARMA_CONFIX_AUG_19_2008_1041AM

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/delimit.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/auxiliary/confix.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma
{
    ///////////////////////////////////////////////////////////////////////////
    // the director for a confix() generated generator
    struct confix_director
    {
        template <typename Component, typename Context, typename Unused>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                traits::attribute_of<karma::domain, subject_type, Context>::type
            type;
        };

    private:
        ///////////////////////////////////////////////////////////////////////
        template <typename OutputIterator, typename Context, 
            typename Delimiter, typename Expr>
        static void
        generate_helper(OutputIterator& sink, Context& ctx, Delimiter const& d,
            Expr const& e)
        {
            BOOST_MPL_ASSERT_MSG(
                (spirit::traits::is_component<karma::domain, Expr>::value), 
                expression_is_not_convertible_to_a_generator, (Context, Expr));

            typedef 
                typename result_of::as_component<karma::domain, Expr>::type 
            expr;

            expr eg = spirit::as_component(karma::domain(), e);
            typedef typename expr::director director;
            director::generate(eg, sink, ctx, d, unused);
        }

        template <typename Context, typename Expr>
        static std::string what_helper(Expr const& e, Context& ctx)
        {
            typedef 
                typename result_of::as_component<karma::domain, Expr>::type 
            expr;

            expr eg = spirit::as_component(karma::domain(), e);
            typedef typename expr::director director;
            return director::what(eg, ctx);
        }

    public:
        ///////////////////////////////////////////////////////////////////////
        template <typename Component, typename OutputIterator, 
            typename Context, typename Delimiter, typename Parameter>
        static bool 
        generate(Component const& component, OutputIterator& sink, 
            Context& ctx, Delimiter const& d, Parameter const& param) 
        {
            // generate the prefix
            generate_helper(sink, ctx, d, 
                spirit::detail::confix_extractor::prefix(
                    proto::arg_c<0>(spirit::argument1(component))));

            // generate the embedded items
            typedef typename
                spirit::result_of::subject<Component>::type::director
            director;
            bool result = director::generate(spirit::subject(component), sink, 
                ctx, d, param);

            // append the suffix 
            generate_helper(sink, ctx, d, 
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
