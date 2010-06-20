/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_SYMBOL_MARCH_11_2007_1055AM)
#define BOOST_SPIRIT_SYMBOL_MARCH_11_2007_1055AM

#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/qi/string/tst.hpp>
#include <boost/spirit/home/support/modifier.hpp>
#include <boost/spirit/home/qi/detail/assign_to.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <boost/type_traits/add_reference.hpp>

#if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable: 4355) // 'this' : used in base member initializer list warning
#endif

namespace boost { namespace spirit { namespace qi
{
    template <typename Filter = tst_pass_through>
    struct symbols_director
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type::ptr_type::element_type::value_type
            type;
        };

        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& /*context*/, Skipper const& skipper
          , Attribute& attr)
        {
            typedef typename
                result_of::subject<Component>::type::ptr_type::element_type::value_type
            value_type;

            qi::skip(first, last, skipper);

            if (value_type* val_ptr
                = fusion::at_c<0>(component.elements)
                    .lookup->find(first, last, Filter()))
            {
                detail::assign_to(*val_ptr, attr);
                return true;
            }
            return false;
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            // perhaps we should show some of the contents?
            return "symbols";
        }
    };

    template <typename Lookup>
    struct symbols_lookup
    {
        typedef shared_ptr<Lookup> ptr_type;
        ptr_type lookup;
    };

    template <typename Char, typename T, typename Lookup = tst<Char, T> >
    struct symbols
      : proto::extends<
            typename proto::terminal<symbols_lookup<Lookup> >::type
          , symbols<Char, T>
        >
    {
        typedef Char char_type; // the character type
        typedef T value_type; // the value associated with each entry
        typedef shared_ptr<Lookup> ptr_type;

        symbols()
          : add(*this)
          , remove(*this)
        {
            proto::arg(*this).lookup = ptr_type(new Lookup());
        }

        template <typename Symbols>
        symbols(Symbols const& syms)
          : add(*this)
          , remove(*this)
        {
            proto::arg(*this).lookup = ptr_type(new Lookup());
            typename range_const_iterator<Symbols>::type si = boost::begin(syms);
            while (si != boost::end(syms))
                add(*si++);
        }

        template <typename Symbols, typename Data>
        symbols(Symbols const& syms, Data const& data)
          : add(*this)
          , remove(*this)
        {
            proto::arg(*this).lookup = ptr_type(new Lookup());
            typename range_const_iterator<Symbols>::type si = boost::begin(syms);
            typename range_const_iterator<Data>::type di = boost::begin(data);
            while (si != boost::end(syms))
                add(*si++, *di++);
        }

        symbols&
        operator=(symbols const& rhs)
        {
            proto::arg(*this) = proto::arg(rhs);
            return *this;
        }

        void clear()
        {
            lookup()->clear();
        }

        struct adder;
        struct remover;

        adder const&
        operator=(Char const* str)
        {
            lookup()->clear();
            return add(str);
        }

        adder const&
        operator+=(Char const* str)
        {
            return add(str);
        }

        remover const&
        operator-=(Char const* str)
        {
            return remove(str);
        }

        ptr_type lookup() const
        {
            return proto::arg(*this).lookup;
        }

        template <typename F>
        void for_each(F f) const
        {
            lookup()->for_each(f);
        }

        struct adder
        {
            template <typename, typename = unused_type, typename = unused_type>
            struct result { typedef adder const& type; };

            adder(symbols& sym)
              : sym(sym)
            {
            }

            template <typename Iterator>
            adder const&
            operator()(Iterator const& first, Iterator const& last, T const& val = T()) const
            {
                sym.lookup()->add(first, last, val);
                return *this;
            }

            adder const&
            operator()(Char const* s, T const& val = T()) const
            {
                Char const* last = s;
                while (*last)
                    last++;
                sym.lookup()->add(s, last, val);
                return *this;
            }

            adder const&
            operator,(Char const* s) const
            {
                Char const* last = s;
                while (*last)
                    last++;
                sym.lookup()->add(s, last, T());
                return *this;
            }

            symbols& sym;
        };

        struct remover
        {
            template <typename, typename = unused_type, typename = unused_type>
            struct result { typedef adder const& type; };

            remover(symbols& sym)
              : sym(sym)
            {
            }

            template <typename Iterator>
            remover const&
            operator()(Iterator const& first, Iterator const& last) const
            {
                sym.lookup()->remove(first, last);
                return *this;
            }

            remover const&
            operator()(Char const* s) const
            {
                Char const* last = s;
                while (*last)
                    last++;
                sym.lookup()->remove(s, last);
                return *this;
            }

            remover const&
            operator,(Char const* s) const
            {
                Char const* last = s;
                while (*last)
                    last++;
                sym.lookup()->remove(s, last);
                return *this;
            }

            symbols& sym;
        };

        adder add;
        remover remove;
    };
}}}


namespace boost { namespace spirit { namespace traits
{
    namespace detail
    {
        template <typename CharSet>
        struct no_case_filter
        {
            template <typename Char>
            Char operator()(Char ch) const
            {
                return CharSet::tolower(ch);
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // generator for no-case symbols
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain, typename Elements, typename Modifier>
    struct make_modified_component<Domain, qi::symbols_director<>, Elements, Modifier
      , typename enable_if<
            is_member_of_modifier<Modifier, spirit::char_class::no_case_base_tag>
        >::type
    >
    {
        typedef detail::no_case_filter<typename Modifier::char_set> filter;
        typedef component<qi::domain, qi::symbols_director<filter>, Elements> type;

        static type
        call(Elements const& elements)
        {
            // we return the same lookup but this time we use a director
            // with a filter that converts to lower-case.
            return elements;
        }
    };
}}}


#if defined(BOOST_MSVC)
# pragma warning(pop)
#endif

#endif
