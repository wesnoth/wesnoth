//Copyright (c) 2006-2008 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_FA5836A2CADA11DC8CD47C8555D89593
#define UUID_FA5836A2CADA11DC8CD47C8555D89593

#include <boost/exception/exception.hpp>
#include <boost/exception/detail/type_info.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <new>

namespace
boost
    {
    class exception_ptr;
    exception_ptr current_exception();
    void rethrow_exception( exception_ptr const & );

    class
    exception_ptr
        {
        typedef bool exception_ptr::*unspecified_bool_type;
        friend exception_ptr current_exception();
        friend void rethrow_exception( exception_ptr const & );

        shared_ptr<exception_detail::clone_base const> c_;
        bool bad_alloc_;

        struct
        bad_alloc_tag
            {
            };

        explicit
        exception_ptr( bad_alloc_tag ):
            bad_alloc_(true)
            {
            }

        explicit
        exception_ptr( shared_ptr<exception_detail::clone_base const> const & c ):
            c_(c),
            bad_alloc_(false)
            {
            BOOST_ASSERT(c);
            }

        public:

        exception_ptr():
            bad_alloc_(false)
            {
            }

        operator unspecified_bool_type() const
            {
            return (bad_alloc_ || c_) ? &exception_ptr::bad_alloc_ : 0;
            }

        friend
        bool
        operator==( exception_ptr const & a, exception_ptr const & b )
            {
            return a.c_==b.c_ && a.bad_alloc_==b.bad_alloc_;
            }

        friend
        bool
        operator!=( exception_ptr const & a, exception_ptr const & b )
            {
            return !(a==b);
            }
        };

    class
    unknown_exception:
        public exception,
        public std::exception,
        public exception_detail::clone_base
        {
        public:

        unknown_exception()
            {
            }

        explicit
        unknown_exception( boost::exception const & e ):
            boost::exception(e)
            {
            }

        ~unknown_exception() throw()
            {
            }

        private:

		exception_detail::clone_base const *
        clone() const
            {
            return new unknown_exception(*this);
            }

        void
        rethrow() const
            {
            throw*this;
            }
        };

    namespace
    exception_detail
        {
        template <class T>
        class
        current_exception_std_exception_wrapper:
            public T,
            public boost::exception,
            public clone_base
            {
            public:

            explicit
            current_exception_std_exception_wrapper( T const & e1 ):
                T(e1)
                {
                }

            current_exception_std_exception_wrapper( T const & e1, boost::exception const & e2 ):
                T(e1),
                boost::exception(e2)
                {
                }

            ~current_exception_std_exception_wrapper() throw()
                {
                }

            private:

            clone_base const *
            clone() const
                {
                return new current_exception_std_exception_wrapper(*this);
                }

            void
            rethrow() const
                {
                throw *this;
                }
            };

#ifdef BOOST_NO_RTTI
        template <class T>
        exception const *
        get_boost_exception( T const * )
            {
            try
                {
                throw;
                }
            catch(
            exception & x )
                {
                return &x;
                }
            catch(...)
                {
                return 0;
                }
            }
#else
        template <class T>
        exception const *
        get_boost_exception( T const * x )
            {
            return dynamic_cast<exception const *>(x);
            }
#endif

        template <class T>
        inline
        shared_ptr<clone_base const>
        current_exception_std_exception( T const & e1 )
            {
            if( boost::exception const * e2 = get_boost_exception(&e1) )
                return shared_ptr<clone_base const>(new current_exception_std_exception_wrapper<T>(e1,*e2));
            else
                return shared_ptr<clone_base const>(new current_exception_std_exception_wrapper<T>(e1));
            }

        inline
        shared_ptr<clone_base const>
        current_exception_unknown_exception()
            {
            return shared_ptr<clone_base const>(new unknown_exception());
            }

        inline
        shared_ptr<clone_base const>
        current_exception_unknown_boost_exception( boost::exception const & e )
            {
            return shared_ptr<clone_base const>(new unknown_exception(e));
            }

        inline
        shared_ptr<clone_base const>
        current_exception_unknown_std_exception( std::exception const & e )
            {
            if( boost::exception const * be = get_boost_exception(&e) )
                return current_exception_unknown_boost_exception(*be);
            else
                return current_exception_unknown_exception();
            }

        inline
        shared_ptr<clone_base const>
        current_exception_impl()
            {
            try
                {
                throw;
                }
            catch(
            exception_detail::clone_base & e )
                {
                return shared_ptr<exception_detail::clone_base const>(e.clone());
                }
            catch(
            std::invalid_argument & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::out_of_range & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::logic_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::bad_alloc & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
#ifndef BOOST_NO_TYPEID
            catch(
            std::bad_cast & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::bad_typeid & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
#endif
            catch(
            std::bad_exception & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::exception & e )
                {
                return exception_detail::current_exception_unknown_std_exception(e);
                }
            catch(
            boost::exception & e )
                {
                return exception_detail::current_exception_unknown_boost_exception(e);
                }
            catch(
            ... )
                {
                return exception_detail::current_exception_unknown_exception();
                }
            }
        }

    inline
    exception_ptr
    current_exception()
        {
        try
            {
            return exception_ptr(exception_detail::current_exception_impl());
            }
        catch(
        std::bad_alloc & )
            {
            }
        catch(
        ... )
            {
            try
                {
                return exception_ptr(exception_detail::current_exception_std_exception(std::bad_exception()));
                }
            catch(
            std::bad_alloc & )
                {
                }
            catch(
            ... )
                {
                BOOST_ASSERT(0);
                }
            }
        return exception_ptr(exception_ptr::bad_alloc_tag());
        }

    template <class T>
    inline
    exception_ptr
    copy_exception( T const & e )
        {
        try
            {
            throw enable_current_exception(e);
            }
        catch(
        ... )
            {
            return current_exception();
            }
        }

    inline
    void
    rethrow_exception( exception_ptr const & p )
        {
        BOOST_ASSERT(p);
        if( p.bad_alloc_ )
            throw enable_current_exception(std::bad_alloc());
        else
            p.c_->rethrow();
        }
    }

#endif
