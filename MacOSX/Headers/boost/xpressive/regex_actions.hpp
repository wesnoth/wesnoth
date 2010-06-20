///////////////////////////////////////////////////////////////////////////////
/// \file regex_actions.hpp
/// Defines the syntax elements of xpressive's action expressions.
//
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_ACTIONS_HPP_EAN_03_22_2007
#define BOOST_XPRESSIVE_ACTIONS_HPP_EAN_03_22_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/config.hpp>
#include <boost/ref.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/int.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/core/state.hpp>
#include <boost/xpressive/detail/core/matcher/attr_matcher.hpp>
#include <boost/xpressive/detail/core/matcher/attr_end_matcher.hpp>
#include <boost/xpressive/detail/core/matcher/attr_begin_matcher.hpp>
#include <boost/xpressive/detail/core/matcher/predicate_matcher.hpp>
#include <boost/xpressive/detail/utility/ignore_unused.hpp>

// These are very often needed by client code.
#include <boost/typeof/std/map.hpp>
#include <boost/typeof/std/string.hpp>

// Doxygen can't handle proto :-(
#ifndef BOOST_XPRESSIVE_DOXYGEN_INVOKED
# include <boost/xpressive/proto/transform.hpp>
# include <boost/xpressive/detail/core/matcher/action_matcher.hpp>
#endif

/// INTERNAL ONLY
///
#define UNREF(x)    typename remove_reference<x>::type

/// INTERNAL ONLY
///
#define UNCVREF(x)  typename remove_cv<typename remove_reference<x>::type>::type

#if BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4510) // default constructor could not be generated
#pragma warning(disable : 4512) // assignment operator could not be generated
#pragma warning(disable : 4610) // can never be instantiated - user defined constructor required
#endif

namespace boost { namespace xpressive
{

    namespace detail
    {
        template<typename T, typename U>
        struct action_arg
        {
            typedef T type;
            typedef typename add_reference<T>::type reference;

            reference cast(void *pv) const
            {
                return *static_cast<UNREF(T) *>(pv);
            }
        };

        template<typename T>
        struct value_wrapper
        {
            value_wrapper()
              : value()
            {}

            value_wrapper(T const &t)
              : value(t)
            {}

            T value;
        };

        struct check_tag
        {};

        struct BindArg : proto::callable
        {
            typedef int result_type;

            template<typename Visitor, typename Expr>
            int operator ()(Visitor &visitor, Expr const &expr) const
            {
                visitor.let(expr);
                return 0;
            }
        };

        struct let_tag
        {};

        struct BindArgs
          : proto::when<
                // let(_a = b, _c = d)
                proto::function<
                    proto::terminal<let_tag>
                  , proto::vararg<proto::assign<proto::_, proto::_> >
                >
              , proto::function<
                    proto::_state   // no-op
                  , proto::vararg<proto::call<BindArg(proto::_visitor, proto::_)> >
                >
            >
        {};

        struct let_domain
          : boost::proto::domain<boost::proto::pod_generator<let_> >
        {};

        template<typename Expr>
        struct let_
        {
            BOOST_PROTO_EXTENDS(Expr, let_<Expr>, let_domain)
            BOOST_PROTO_EXTENDS_FUNCTION(Expr, let_<Expr>, let_domain)
        };

        template<typename Args, typename BidiIter>
        void bind_args(let_<Args> const &args, match_results<BidiIter> &what)
        {
            BindArgs()(args, 0, what);
        }

        template<typename BidiIter>
        struct replacement_context
          : proto::callable_context<replacement_context<BidiIter> const>
        {
            replacement_context(match_results<BidiIter> const &what)
              : what_(what)
            {}

            template<typename Sig>
            struct result;

            template<typename This>
            struct result<This(proto::tag::terminal, mark_placeholder const &)>
            {
                typedef sub_match<BidiIter> const &type;
            };

            template<typename This>
            struct result<This(proto::tag::terminal, any_matcher const &)>
            {
                typedef sub_match<BidiIter> const &type;
            };

            template<typename This, typename T>
            struct result<This(proto::tag::terminal, reference_wrapper<T> const &)>
            {
                typedef T &type;
            };

            sub_match<BidiIter> const &operator ()(proto::tag::terminal, mark_placeholder m) const
            {
                return this->what_[m.mark_number_];
            }

            sub_match<BidiIter> const &operator ()(proto::tag::terminal, any_matcher) const
            {
                return this->what_[0];
            }

            template<typename T>
            T &operator ()(proto::tag::terminal, reference_wrapper<T> r) const
            {
                return r;
            }
        private:
            match_results<BidiIter> const &what_;
        };
    }

    namespace op
    {
        struct push
        {
            typedef void result_type;

            template<typename Sequence, typename Value>
            void operator()(Sequence &seq, Value const &val) const
            {
                seq.push(val);
            }
        };

        struct push_back
        {
            typedef void result_type;

            template<typename Sequence, typename Value>
            void operator()(Sequence &seq, Value const &val) const
            {
                seq.push_back(val);
            }
        };

        struct push_front
        {
            typedef void result_type;

            template<typename Sequence, typename Value>
            void operator()(Sequence &seq, Value const &val) const
            {
                seq.push_front(val);
            }
        };

        struct pop
        {
            typedef void result_type;

            template<typename Sequence>
            void operator()(Sequence &seq) const
            {
                seq.pop();
            }
        };

        struct pop_back
        {
            typedef void result_type;

            template<typename Sequence>
            void operator()(Sequence &seq) const
            {
                seq.pop_back();
            }
        };

        struct pop_front
        {
            typedef void result_type;

            template<typename Sequence>
            void operator()(Sequence &seq) const
            {
                seq.pop_front();
            }
        };

        struct front
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Sequence>
            struct result<This(Sequence)>
            {
                typedef UNREF(Sequence) sequence_type;
                typedef
                    typename mpl::if_<
                        is_const<sequence_type>
                      , typename sequence_type::const_reference
                      , typename sequence_type::reference
                    >::type
                type;
            };

            template<typename Sequence>
            typename result<front(Sequence &)>::type operator()(Sequence &seq) const
            {
                return seq.front();
            }
        };

        struct back
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Sequence>
            struct result<This(Sequence)>
            {
                typedef UNREF(Sequence) sequence_type;
                typedef
                    typename mpl::if_<
                        is_const<sequence_type>
                      , typename sequence_type::const_reference
                      , typename sequence_type::reference
                    >::type
                type;
            };

            template<typename Sequence>
            typename result<back(Sequence &)>::type operator()(Sequence &seq) const
            {
                return seq.back();
            }
        };

        struct top
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Sequence>
            struct result<This(Sequence)>
            {
                typedef UNREF(Sequence) sequence_type;
                typedef
                    typename mpl::if_<
                        is_const<sequence_type>
                      , typename sequence_type::value_type const &
                      , typename sequence_type::value_type &
                    >::type
                type;
            };

            template<typename Sequence>
            typename result<top(Sequence &)>::type operator()(Sequence &seq) const
            {
                return seq.top();
            }
        };

        struct first
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Pair>
            struct result<This(Pair)>
            {
                typedef UNREF(Pair)::first_type type;
            };

            template<typename Pair>
            typename Pair::first_type operator()(Pair const &p) const
            {
                return p.first;
            }
        };

        struct second
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Pair>
            struct result<This(Pair)>
            {
                typedef UNREF(Pair)::second_type type;
            };

            template<typename Pair>
            typename Pair::second_type operator()(Pair const &p) const
            {
                return p.second;
            }
        };

        struct matched
        {
            typedef bool result_type;

            template<typename Sub>
            bool operator()(Sub const &sub) const
            {
                return sub.matched;
            }
        };

        struct length
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Sub>
            struct result<This(Sub)>
            {
                typedef UNREF(Sub)::difference_type type;
            };

            template<typename Sub>
            typename Sub::difference_type operator()(Sub const &sub) const
            {
                return sub.length();
            }
        };

        struct str
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Sub>
            struct result<This(Sub)>
            {
                typedef UNREF(Sub)::string_type type;
            };

            template<typename Sub>
            typename Sub::string_type operator()(Sub const &sub) const
            {
                return sub.str();
            }
        };

        // This codifies the return types of the various insert member
        // functions found in sequence containers, the 2 flavors of
        // associative containers, and strings.
        struct insert
        {
            template<typename Sig, typename EnableIf = void>
            struct result
            {};

            // assoc containers
            template<typename This, typename Cont, typename Value>
            struct result<This(Cont, Value), void>
            {
                typedef UNREF(Cont) cont_type;
                typedef UNREF(Value) value_type;
                static cont_type &scont_;
                static value_type &svalue_;
                typedef char yes_type;
                typedef char (&no_type)[2];
                static yes_type check_insert_return(typename cont_type::iterator);
                static no_type check_insert_return(std::pair<typename cont_type::iterator, bool>);
                BOOST_STATIC_CONSTANT(bool, is_iterator = (sizeof(yes_type) == sizeof(check_insert_return(scont_.insert(svalue_)))));
                typedef
                    typename mpl::if_c<
                        is_iterator
                      , typename cont_type::iterator
                      , std::pair<typename cont_type::iterator, bool>
                    >::type
                type;
            };

            // sequence containers, assoc containers, strings
            template<typename This, typename Cont, typename It, typename Value>
            struct result<This(Cont, It, Value),
                typename disable_if<mpl::or_<is_integral<UNCVREF(It)>, is_same<UNCVREF(It), UNCVREF(Value)> > >::type>
            {
                typedef UNREF(Cont)::iterator type;
            };

            // strings
            template<typename This, typename Cont, typename Size, typename T>
            struct result<This(Cont, Size, T),
                typename enable_if<is_integral<UNCVREF(Size)> >::type>
            {
                typedef UNREF(Cont) &type;
            };

            // assoc containers
            template<typename This, typename Cont, typename It>
            struct result<This(Cont, It, It), void>
            {
                typedef void type;
            };

            // sequence containers, strings
            template<typename This, typename Cont, typename It, typename Size, typename Value>
            struct result<This(Cont, It, Size, Value),
                typename disable_if<is_integral<UNCVREF(It)> >::type>
            {
                typedef void type;
            };

            // strings
            template<typename This, typename Cont, typename Size, typename A0, typename A1>
            struct result<This(Cont, Size, A0, A1),
                typename enable_if<is_integral<UNCVREF(Size)> >::type>
            {
                typedef UNREF(Cont) &type;
            };

            /// operator()
            ///
            template<typename Cont, typename A0>
            typename result<insert(Cont &, A0 const &)>::type
            operator()(Cont &cont, A0 const &a0) const
            {
                return cont.insert(a0);
            }

            /// \overload
            ///
            template<typename Cont, typename A0, typename A1>
            typename result<insert(Cont &, A0 const &, A1 const &)>::type
            operator()(Cont &cont, A0 const &a0, A1 const &a1) const
            {
                return cont.insert(a0, a1);
            }

            /// \overload
            ///
            template<typename Cont, typename A0, typename A1, typename A2>
            typename result<insert(Cont &, A0 const &, A1 const &, A2 const &)>::type
            operator()(Cont &cont, A0 const &a0, A1 const &a1, A2 const &a2) const
            {
                return cont.insert(a0, a1, a2);
            }
        };

        struct make_pair
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename First, typename Second>
            struct result<This(First, Second)>
            {
                typedef std::pair<UNCVREF(First), UNCVREF(Second)> type;
            };

            template<typename First, typename Second>
            std::pair<First, Second> operator()(First const &first, Second const &second) const
            {
                return std::make_pair(first, second);
            }
        };

        template<typename T>
        struct as
        {
            typedef T result_type;

            template<typename Value>
            T operator()(Value const &val) const
            {
                return lexical_cast<T>(val);
            }
        };

        template<typename T>
        struct static_cast_
        {
            typedef T result_type;

            template<typename Value>
            T operator()(Value const &val) const
            {
                return static_cast<T>(val);
            }
        };

        template<typename T>
        struct dynamic_cast_
        {
            typedef T result_type;

            template<typename Value>
            T operator()(Value const &val) const
            {
                return dynamic_cast<T>(val);
            }
        };

        template<typename T>
        struct const_cast_
        {
            typedef T result_type;

            template<typename Value>
            T operator()(Value const &val) const
            {
                return const_cast<T>(val);
            }
        };

        template<typename T>
        struct construct
        {
            typedef T result_type;

            T operator()() const
            {
                return T();
            }

            template<typename A0>
            T operator()(A0 const &a0) const
            {
                return T(a0);
            }

            template<typename A0, typename A1>
            T operator()(A0 const &a0, A1 const &a1) const
            {
                return T(a0, a1);
            }

            template<typename A0, typename A1, typename A2>
            T operator()(A0 const &a0, A1 const &a1, A2 const &a2) const
            {
                return T(a0, a1, a2);
            }
        };

        template<typename Except>
        struct throw_
        {
            typedef void result_type;

            void operator()() const
            {
                boost::throw_exception(Except());
            }

            template<typename A0>
            void operator()(A0 const &a0) const
            {
                boost::throw_exception(Except(a0));
            }

            template<typename A0, typename A1>
            void operator()(A0 const &a0, A1 const &a1) const
            {
                boost::throw_exception(Except(a0, a1));
            }

            template<typename A0, typename A1, typename A2>
            void operator()(A0 const &a0, A1 const &a1, A2 const &a2) const
            {
                boost::throw_exception(Except(a0, a1, a2));
            }
        };
    }

    template<typename Fun>
    struct function
    {
        typedef typename proto::terminal<Fun>::type type;
    };

    function<op::push>::type const push = {{}};
    function<op::push_back>::type const push_back = {{}};
    function<op::push_front>::type const push_front = {{}};
    function<op::pop>::type const pop = {{}};
    function<op::pop_back>::type const pop_back = {{}};
    function<op::pop_front>::type const pop_front = {{}};
    function<op::top>::type const top = {{}};
    function<op::back>::type const back = {{}};
    function<op::front>::type const front = {{}};
    function<op::first>::type const first = {{}};
    function<op::second>::type const second = {{}};
    function<op::matched>::type const matched = {{}};
    function<op::length>::type const length = {{}};
    function<op::str>::type const str = {{}};
    function<op::insert>::type const insert = {{}};
    function<op::make_pair>::type const make_pair = {{}};

    template<typename T>
    struct value
      : proto::extends<typename proto::terminal<T>::type, value<T> >
    {
        typedef proto::extends<typename proto::terminal<T>::type, value<T> > base_type;

        value()
          : base_type()
        {}

        explicit value(T const &t)
          : base_type(base_type::proto_base_expr::make(t))
        {}

        using base_type::operator =;

        T &get()
        {
            return proto::arg(*this);
        }

        T const &get() const
        {
            return proto::arg(*this);
        }
    };

    template<typename T>
    struct reference
      : proto::extends<typename proto::terminal<reference_wrapper<T> >::type, reference<T> >
    {
        typedef proto::extends<typename proto::terminal<reference_wrapper<T> >::type, reference<T> > base_type;

        explicit reference(T &t)
          : base_type(base_type::proto_base_expr::make(boost::ref(t)))
        {}

        using base_type::operator =;

        T &get() const
        {
            return proto::arg(*this).get();
        }
    };

    template<typename T>
    struct local
      : private noncopyable
      , detail::value_wrapper<T>
      , proto::terminal<reference_wrapper<T> >::type
    {
        typedef typename proto::terminal<reference_wrapper<T> >::type base_type;

        local()
          : noncopyable()
          , detail::value_wrapper<T>()
          , base_type(base_type::make(boost::ref(detail::value_wrapper<T>::value)))
        {}

        explicit local(T const &t)
          : noncopyable()
          , detail::value_wrapper<T>(t)
          , base_type(base_type::make(boost::ref(detail::value_wrapper<T>::value)))
        {}

        using base_type::operator =;

        T &get()
        {
            return proto::arg(*this);
        }

        T const &get() const
        {
            return proto::arg(*this);
        }
    };

    /// as (a.k.a., lexical_cast)
    ///
    BOOST_PROTO_DEFINE_FUNCTION_TEMPLATE(
        1
      , as
      , boost::proto::default_domain
      , (boost::proto::tag::function)
      , ((op::as)(typename))
    )

    /// static_cast_
    ///
    BOOST_PROTO_DEFINE_FUNCTION_TEMPLATE(
        1
      , static_cast_
      , boost::proto::default_domain
      , (boost::proto::tag::function)
      , ((op::static_cast_)(typename))
    )

    /// dynamic_cast_
    ///
    BOOST_PROTO_DEFINE_FUNCTION_TEMPLATE(
        1
      , dynamic_cast_
      , boost::proto::default_domain
      , (boost::proto::tag::function)
      , ((op::dynamic_cast_)(typename))
    )

    /// const_cast_
    ///
    BOOST_PROTO_DEFINE_FUNCTION_TEMPLATE(
        1
      , const_cast_
      , boost::proto::default_domain
      , (boost::proto::tag::function)
      , ((op::const_cast_)(typename))
    )

    /// val()
    ///
    template<typename T>
    value<T> const val(T const &t)
    {
        return value<T>(t);
    }

    /// ref()
    ///
    template<typename T>
    reference<T> const ref(T &t)
    {
        return reference<T>(t);
    }

    /// cref()
    ///
    template<typename T>
    reference<T const> const cref(T const &t)
    {
        return reference<T const>(t);
    }

    /// check(), for testing custom assertions
    ///
    proto::terminal<detail::check_tag>::type const check = {{}};

    /// let(), for binding references to non-local variables
    ///
    detail::let_<proto::terminal<detail::let_tag>::type> const let = {{{}}};

    /// placeholder<T>, for defining a placeholder to stand in fo
    /// a variable of type T in a semantic action.
    ///
    template<typename T, int I, typename Dummy>
    struct placeholder
    {
        typedef placeholder<T, I, Dummy> this_type;
        typedef typename proto::terminal<detail::action_arg<T, mpl::int_<I> > >::type action_arg_type;

        BOOST_PROTO_EXTENDS(action_arg_type, this_type, proto::default_domain)
        BOOST_PROTO_EXTENDS_ASSIGN(action_arg_type, this_type, proto::default_domain)
        BOOST_PROTO_EXTENDS_SUBSCRIPT(action_arg_type, this_type, proto::default_domain)
        BOOST_PROTO_EXTENDS_FUNCTION(action_arg_type, this_type, proto::default_domain)
    };

    /// Usage: construct\<Type\>(arg1, arg2)
    ///
    BOOST_PROTO_DEFINE_VARARG_FUNCTION_TEMPLATE(
        construct
      , boost::proto::default_domain
      , (boost::proto::tag::function)
      , ((op::construct)(typename))
    )

    /// Usage: throw_\<Exception\>(arg1, arg2)
    ///
    BOOST_PROTO_DEFINE_VARARG_FUNCTION_TEMPLATE(
        throw_
      , boost::proto::default_domain
      , (boost::proto::tag::function)
      , ((op::throw_)(typename))
    )

    namespace detail
    {
        inline void ignore_unused_regex_actions()
        {
            ignore_unused(xpressive::push);
            ignore_unused(xpressive::push_back);
            ignore_unused(xpressive::push_front);
            ignore_unused(xpressive::pop);
            ignore_unused(xpressive::pop_back);
            ignore_unused(xpressive::pop_front);
            ignore_unused(xpressive::top);
            ignore_unused(xpressive::back);
            ignore_unused(xpressive::front);
            ignore_unused(xpressive::first);
            ignore_unused(xpressive::second);
            ignore_unused(xpressive::matched);
            ignore_unused(xpressive::length);
            ignore_unused(xpressive::str);
            ignore_unused(xpressive::insert);
            ignore_unused(xpressive::make_pair);
            ignore_unused(xpressive::check);
            ignore_unused(xpressive::let);
        }
    }

}}

#undef UNREF
#undef UNCVREF

#if BOOST_MSVC
#pragma warning(pop)
#endif

#endif // BOOST_XPRESSIVE_ACTIONS_HPP_EAN_03_22_2007
