//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boist.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_ITERATOR_OSTREAM_MAY_27_2007_0133PM)
#define BOOST_SPIRIT_ITERATOR_OSTREAM_MAY_27_2007_0133PM

#include <boost/iostreams/stream.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Char>
    struct iterator_sink
    {
        typedef boost::iostreams::sink_tag category;
        typedef Char char_type;
        
        iterator_sink (OutputIterator& sink_)
          : sink(sink_)
        {}
        
        // Write up to n characters from the buffer s to the output sequence, 
        // returning the number of characters written
        std::streamsize write (Char const* s, std::streamsize n) 
        {
            std::streamsize bytes_written = 0;
            while (n--) {
                *sink = *s;
                ++sink; ++s; ++bytes_written;
            }
            return bytes_written;
        }
        
        OutputIterator& sink;
    };

}}}}

#endif
