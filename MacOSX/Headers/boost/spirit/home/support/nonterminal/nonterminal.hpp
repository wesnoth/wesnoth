/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_NONTERMINAL_MAR_06_2007_0236PM)
#define BOOST_SPIRIT_NONTERMINAL_MAR_06_2007_0236PM

#include <boost/xpressive/proto/proto.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/joint_view.hpp>
#include <boost/fusion/include/single_view.hpp>
#include <boost/type_traits/add_reference.hpp>

namespace boost { namespace spirit
{
    template <typename T, typename Nonterminal>
    struct nonterminal_holder
    {
        typedef Nonterminal nonterminal_type;
        T held;
    };

    template <typename T, typename Nonterminal>
    struct make_nonterminal_holder
      : proto::terminal<nonterminal_holder<T, Nonterminal> >
    {
    };

    template <typename Nonterminal, typename FSequence>
    struct parameterized_nonterminal
    {
        Nonterminal const* ptr;
        FSequence fseq;
    };

    template <typename Nonterminal>
    struct nonterminal_object
    {
        Nonterminal obj;
    };
}}

#endif
