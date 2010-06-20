//Copyright (c) 2006-2008 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_1A590226753311DD9E4CCF6156D89593
#define UUID_1A590226753311DD9E4CCF6156D89593

#include <boost/exception/exception.hpp>
#include <boost/exception/detail/error_info_impl.hpp>
#include <boost/exception/detail/type_info.hpp>
#include <boost/shared_ptr.hpp>

namespace
boost
    {
    namespace
    exception_detail
        {
        struct
        strwrap
            {
			std::string str;
            char const * ptr;

            explicit
            strwrap( char const * s ):
                str(s),
                ptr(&str[0])
                {
                }

            private:

            strwrap( strwrap const & );
            strwrap & operator=( strwrap const & );
            };

        template <>
        struct
        get_info<throw_function>
            {
            static
            shared_ptr<char const * const>
            get( exception const & x )
                {
                if( x.throw_function_ && *x.throw_function_ )
                    {
                    shared_ptr<strwrap> s(new strwrap(x.throw_function_));
                    return shared_ptr<char const *>(s,&s->ptr);
                    }
                else
                    return shared_ptr<char const * const>();
                }
            };

        template <>
        struct
        get_info<throw_file>
            {
            static
            shared_ptr<char const * const>
            get( exception const & x )
                {
                if( x.throw_file_ && *x.throw_file_ )
                    {
                    shared_ptr<strwrap> s(new strwrap(x.throw_file_));
                    return shared_ptr<char const *>(s,&s->ptr);
                    }
                else
                    return shared_ptr<char const * const>();
                }
            };

        template <>
        struct
        get_info<throw_line>
            {
            static
            shared_ptr<int const>
            get( exception const & x )
                {
                if( x.throw_line_!=-1 )
                    return boost::shared_ptr<int>(new int(x.throw_line_));
                else
                    return shared_ptr<int const>();
                }
            };

        template <class ErrorInfo>
        struct
        get_info
            {
            static
            shared_ptr<typename ErrorInfo::value_type const>
            get( exception const & x )
                {
                if( exception_detail::error_info_container * c=x.data_.get() )
                    if( shared_ptr<exception_detail::error_info_base const> eib = c->get(BOOST_EXCEPTION_STATIC_TYPEID(ErrorInfo)) )
                        {
#ifndef BOOST_NO_RTTI
                        BOOST_ASSERT( 0!=dynamic_cast<ErrorInfo const *>(eib.get()) );
#endif
                        ErrorInfo const * w = static_cast<ErrorInfo const *>(eib.get());
                        return shared_ptr<typename ErrorInfo::value_type const>(eib,&w->value());
                        }
                return shared_ptr<typename ErrorInfo::value_type const>();
                }
            };
        }

#ifdef BOOST_NO_RTTI
    template <class ErrorInfo>
    inline
    shared_ptr<typename ErrorInfo::value_type const>
    get_error_info( boost::exception const & x )
        {
        return exception_detail::get_info<ErrorInfo>::get(x);
        }
#else
    template <class ErrorInfo,class E>
    inline
    shared_ptr<typename ErrorInfo::value_type const>
    get_error_info( E const & some_exception )
        {
        if( exception const * x = dynamic_cast<exception const *>(&some_exception) )
            return exception_detail::get_info<ErrorInfo>::get(*x);
        else
            return shared_ptr<typename ErrorInfo::value_type const>();
        }
#endif
    }

#endif
