#ifndef BOOST_ARCHIVE_XML_ARCHIVE_EXCEPTION_HPP
#define BOOST_ARCHIVE_XML_ARCHIVE_EXCEPTION_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// xml_archive_exception.hpp:

// (C) Copyright 2007 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <exception>
#include <cassert>

#include <boost/archive/archive_exception.hpp>

namespace boost {
namespace archive {

//////////////////////////////////////////////////////////////////////
// exceptions thrown by xml archives
//
class xml_archive_exception : 
    public virtual archive_exception
{
public:
    typedef enum {
        xml_archive_parsing_error,    // see save_register
        xml_archive_tag_mismatch,
        xml_archive_tag_name_error
    } exception_code;
    exception_code code;
    xml_archive_exception(exception_code c)
    {}
    virtual const char *what( ) const throw( )
    {
        const char *msg;
        switch(code){
        case xml_archive_parsing_error:
            msg = "unrecognized XML syntax";
            break;
        case xml_archive_tag_mismatch:
            msg = "XML start/end tag mismatch";
            break;
        case xml_archive_tag_name_error:
            msg = "Invalid XML tag name";
            break;
        default:
            msg = archive_exception::what();
            break;
        }
        return msg;
    }
};

}// namespace archive
}// namespace boost

#endif //BOOST_XML_ARCHIVE_ARCHIVE_EXCEPTION_HPP
