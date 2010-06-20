//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_SUPPORT_CONFIX_AUG_19_2008_1103AM)
#define BOOST_SPIRIT_SUPPORT_CONFIX_AUG_19_2008_1103AM

#include <boost/spirit/home/support/placeholders.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace tag
{
    // This is the tag returned by the confix() function
    template <typename Prefix, typename Suffix>
    struct confix_tag
    {
        Prefix prefix;
        Suffix suffix;
    };

}}}

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit 
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Prefix, typename Suffix = Prefix>
    struct confix_spec
      : proto::terminal<tag::confix_tag<Prefix, Suffix> >::type
    {
    private:
        typedef typename 
            proto::terminal<tag::confix_tag<Prefix, Suffix> >::type
        base_type;

        base_type make_tag(Prefix const& prefix, Suffix const& suffix) const
        {
            base_type xpr = {{prefix, suffix}};
            return xpr;
        }

    public:
        confix_spec(Prefix const& prefix, Suffix const& suffix)
          : base_type(make_tag(prefix, suffix))
        {}
    };

    namespace detail
    {
        struct confix_extractor
        {
            template <typename Prefix, typename Suffix>
            static Prefix const& prefix(tag::confix_tag<Prefix, Suffix> const& c) 
            { return c.prefix; }

            template <typename Prefix, typename Suffix>
            static Suffix const& suffix(tag::confix_tag<Prefix, Suffix> const& c) 
            { return c.suffix; }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // construct a confix component
    ///////////////////////////////////////////////////////////////////////////
    inline confix_spec<char const*>
    confix(char const* prefix, char const* suffix)
    {
        return confix_spec<char const*>(prefix, suffix);
    }

    inline confix_spec<wchar_t const*>
    confix(wchar_t const* prefix, wchar_t const* suffix)
    {
        return confix_spec<wchar_t const*>(prefix, suffix);
    }

    template <typename Prefix, typename Suffix>
    inline confix_spec<Prefix, Suffix>
    confix(Prefix const& prefix, Suffix const& suffix)
    {
        return confix_spec<Prefix, Suffix>(prefix, suffix);
    }

}}

#endif
