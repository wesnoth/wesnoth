/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_MODIFIER_FEB_05_2007_0259PM)
#define BOOST_SPIRIT_MODIFIER_FEB_05_2007_0259PM

#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/component.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace boost { namespace spirit
{
    ///////////////////////////////////////////////////////////////////////////
    //  The modifier is like a set of types. Types can be added (but not 
    //  removed). The unique feature of the modifier is that addition of
    //  types is done using inheritance. Thus, checking for membership
    //  involves checking for inheritance. More importantly, because the
    //  modifier inherits from a type, the type's members (typedefs,
    //  nested structs, etc.), are all visible; unless, of course, if the
    //  member is hidden (newer types take priority) or there's ambiguity.
    //
    //      to add:                     add_modifier<Modifier, T>
    //      to test for membership:     is_member_of_modifier<Modifier, T>
    //
    //  The modifier is used as the "Visitor" in proto transforms to
    //  modify the behavior of the expression template building.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Set = unused_type, typename New = unused_type>
    struct modifier : Set, New {};

    template <typename Set>
    struct modifier<Set, unused_type> : Set {};

    template <typename New>
    struct modifier<unused_type, New> : New {};

    template <>
    struct modifier<unused_type, unused_type> {};
        
    template <typename Modifier, typename New>
    struct add_modifier
    {
        typedef typename // add only if New is not a member
            mpl::if_<
                is_base_of<New, Modifier>
              , Modifier
              , modifier<Modifier, New>
            >::type
        type;
    };
    
    template <typename Modifier, typename T>
    struct is_member_of_modifier : is_base_of<T, Modifier> {};

    ///////////////////////////////////////////////////////////////////////////
    //  This is the main customization point for hooking into the 
    //  make_component mechanism for building /modified/ components.
    //  The make_component specialization detects modifier Visitors
    //  and dispatches to the secondary template make_modified_component
    //  for clients to specialize. By default, the modifier is ignored
    //  and the control goes back to make_component.
    //
    //  (see also: component.hpp)
    ///////////////////////////////////////////////////////////////////////////
    namespace traits
    {
        template <
            typename Domain, typename Director, typename Elements
          , typename Modifier, typename Enable = void>
        struct make_modified_component : 
            make_component<Domain, Director, Elements, unused_type>
        {
        };
    
        template <
            typename Domain, typename Director
          , typename Elements, typename Set, typename New>
        struct make_component<Domain, Director, Elements, modifier<Set, New> >
          : make_modified_component<Domain, Director, Elements, modifier<Set, New> >
        {
        };
    }
}}

#endif
