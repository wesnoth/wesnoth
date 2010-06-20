/*=============================================================================
    Copyright (c) 2001-2008 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser
    http://spirit.sourceforge.net/

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_ACTION_DISPATCH_APR_18_2008_0720AM)
#define BOOST_SPIRIT_ACTION_DISPATCH_APR_18_2008_0720AM

#include <boost/spirit/home/support/detail/values.hpp>
#include <boost/spirit/home/phoenix/core/actor.hpp>

namespace boost { namespace spirit { namespace detail
{
    // general handler for everything not explicitly specialized below
    template <typename F, typename Attribute, typename Context, bool IsSequence>
    bool action_dispatch(F const& f, Attribute& attr, Context& context
      , mpl::bool_<IsSequence>)
    {
        bool pass = true;
        f(attr, context, pass);
        return pass;
    }

    // handler for phoenix actors

    // If the component this action has to be invoked for is a sequence, we 
    // wrap any non-fusion sequence into a fusion sequence (done by pass_value)
    // and pass through any fusion sequence.
    template <typename Eval, typename Attribute, typename Context>
    bool action_dispatch(phoenix::actor<Eval> const& f
      , Attribute& attr, Context& context, mpl::true_)
    {
        bool pass = true;
        f (pass_value<Attribute>::call(attr), context, pass);
        return pass;
    }

    // If this action has to be invoked for anything but a sequence, we always 
    // need to wrap the attribute into a fusion sequence, because the attribute
    // has to be treated as being a single value in any case (even if it 
    // actually already is a fusion sequence on its own).
    template <typename Eval, typename Attribute, typename Context>
    bool action_dispatch(phoenix::actor<Eval> const& f
      , Attribute& attr, Context& context, mpl::false_)
    {
        bool pass = true;
        fusion::vector<Attribute&> wrapped_attr(attr);
        f (wrapped_attr, context, pass);
        return pass;
    }

    // specializations for plain function pointers taking a different number of
    // arguments
    template <typename RT, typename A0, typename A1, typename A2
      , typename Attribute, typename Context, bool IsSequence>
    bool action_dispatch(RT(*f)(A0, A1, A2)
      , Attribute& attr, Context& context, mpl::bool_<IsSequence>)
    {
        bool pass = true;
        f(attr, context, pass);
        return pass;
    }

    template <typename RT, typename A0, typename A1
      , typename Attribute, typename Context, bool IsSequence>
    bool action_dispatch(RT(*f)(A0, A1)
      , Attribute& attr, Context& context, mpl::bool_<IsSequence>)
    {
        f(attr, context);
        return true;
    }

    template <typename RT, typename A0
      , typename Attribute, typename Context, bool IsSequence>
    bool action_dispatch(RT(*f)(A0)
      , Attribute& attr, Context&, mpl::bool_<IsSequence>)
    {
        f(attr);
        return true;
    }

    template <typename RT
      , typename Attribute, typename Context, bool IsSequence>
    bool action_dispatch(RT(*f)()
      , Attribute&, Context&, mpl::bool_<IsSequence>)
    {
        f();
        return true;
    }
    
}}}

#endif
