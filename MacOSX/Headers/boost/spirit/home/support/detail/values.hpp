/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser
    http://spirit.sourceforge.net/

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_VALUES_JAN_07_2007_0802PM)
#define BOOST_SPIRIT_VALUES_JAN_07_2007_0802PM

#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/and.hpp>
#include <boost/variant.hpp>

namespace boost { namespace spirit { namespace detail
{
    template <typename T>
    struct not_is_variant
      : mpl::true_ {};

    template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
    struct not_is_variant<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
      : mpl::false_ {};

    ///////////////////////////////////////////////////////////////////////////
    //  All parsers and generators have specific attribute or parameter types.
    //  Spirit parsers are passed an attribute and Spirit generators
    //  are passed a parameter; these are either references to the expected
    //  type, or an unused_type -- to flag that we do not care about the
    //  attribute/parameter. For semantic actions, however, we need to have a
    //  real value to pass to the semantic action. If the client did not
    //  provide one, we will have to synthesize the value. This class
    //  takes care of that.
    ///////////////////////////////////////////////////////////////////////////
    template <typename ValueType>
    struct make_value
    {
        static ValueType call(unused_type)
        {
            return ValueType(); // synthesize the attribute/parameter
        }

        template <typename T>
        static T& call(T& value)
        {
            return value; // just pass the one provided
        }

        template <typename T>
        static T const& call(T const& value)
        {
            return value; // just pass the one provided
        }
    };

    template <typename ValueType>
    struct make_value<ValueType&> : make_value<ValueType>
    {
    };

    template <>
    struct make_value<unused_type>
    {
        static unused_type call(unused_type)
        {
            return unused;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  pass_value determines how we pass attributes and parameters to semantic
    //  actions. Basically, all SAs receive the arguments in a tuple. So, if
    //  the argument to be passed is not a tuple, wrap it in one.
    ///////////////////////////////////////////////////////////////////////////
    template <typename ValueType>
    struct pass_value
    {
        typedef
            mpl::and_<
                fusion::traits::is_sequence<ValueType>
              , detail::not_is_variant<ValueType>
            >
        is_sequence;

        typedef typename
            mpl::if_<
                is_sequence
              , ValueType&
              , fusion::vector<ValueType&> const
            >::type
        type;

        static ValueType&
        call(ValueType& arg, mpl::true_)
        {
            // arg is a fusion sequence (except a variant) return it as-is.
            return arg;
        }

        static fusion::vector<ValueType&> const
        call(ValueType& seq, mpl::false_)
        {
            // arg is a not fusion sequence wrap it in a fusion::vector.
            return fusion::vector<ValueType&>(seq);
        }

        static type
        call(ValueType& arg)
        {
            return call(arg, is_sequence());
        }
    };
}}}

#endif
