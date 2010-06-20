//Copyright (c) 2006-2008 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_0552D49838DD11DD90146B8956D89593
#define UUID_0552D49838DD11DD90146B8956D89593

#include <boost/exception/get_error_info.hpp>
#include <exception>
#include <sstream>
#include <string>

namespace
boost
    {
    namespace
    exception_detail
        {
        inline
        char const *
        get_diagnostic_information( exception const & x )
            {
            if( error_info_container * c=x.data_.get() )
                try
                    {
                    return c->diagnostic_information();
                    }
                catch(...)
                    {
                    }
            return 0;
            }
        }

    inline
    std::string
    diagnostic_information( exception const & x )
        {
        std::ostringstream tmp;
        if( boost::shared_ptr<char const * const> f=get_error_info<throw_file>(x) )
            {
            tmp << *f;
            if( boost::shared_ptr<int const> l=get_error_info<throw_line>(x) )
                tmp << '(' << *l << "): ";
            }
        tmp << "Throw in function ";
        if( boost::shared_ptr<char const * const> fn=get_error_info<throw_function>(x) )
            tmp << *fn;
        else
            tmp << "(unknown)";
#ifndef BOOST_NO_RTTI
        tmp << "\nDynamic exception type: " << BOOST_EXCEPTION_DYNAMIC_TYPEID(x).name();
        if( std::exception const * e=dynamic_cast<std::exception const *>(&x) )
            tmp << "\nstd::exception::what: " << e->what();
#endif
        if( char const * s=exception_detail::get_diagnostic_information(x) )
            if( *s )
                tmp << '\n' << s;
        return tmp.str();
        }
    }

#endif
