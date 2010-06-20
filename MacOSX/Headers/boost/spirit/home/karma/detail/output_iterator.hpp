//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_OUTPUT_ITERATOR_MAY_26_2007_0506PM)
#define BOOST_SPIRIT_KARMA_OUTPUT_ITERATOR_MAY_26_2007_0506PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <iterator>
#include <vector>
#include <algorithm>

#include <boost/noncopyable.hpp>
#include <boost/spirit/home/karma/detail/ostream_iterator.hpp>

namespace boost { namespace spirit { namespace karma { namespace detail 
{
    ///////////////////////////////////////////////////////////////////////////
    //  This class is used to keep track of the current position in the output.
    ///////////////////////////////////////////////////////////////////////////
    class position_sink 
    {
    public:
        position_sink() : count(0), line(1), column(0) {}
        void tidy() { count = 0; line = 1; column = 0; }
        
        template <typename T>
        void output(T const& value) 
        {
            ++count; 
            if (value == '\n') {
                ++line;
                column = 1;
            }
            else {
                ++column;
            }
        }
        std::size_t get_count() const { return count; }
        std::size_t get_line() const { return line; }
        std::size_t get_column() const { return column; }

    private:
        std::size_t count;
        std::size_t line;
        std::size_t column;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  This class is used to count the umber of characters streamed into the 
    //  output.
    ///////////////////////////////////////////////////////////////////////////
    class counting_sink 
    {
    public:
        counting_sink() : count(0) {}
        
        void init(std::size_t count_) { count = count_; }
        void tidy() { count = 0; }
        
        void output() { ++count; }
        std::size_t get_count() const { return count; }

    private:
        std::size_t count;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  The following classes are used to intercept the output into a buffer
    //  allowing to do things like alignment, character escaping etc.
    //
    //  We need to use virtual functions because output_iterators do not have
    //  an associated value_type. The type of the buffer elements is available
    //  at insertion time only (and not at buffer creation time).
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator>
    struct abstract_container
    {
        virtual ~abstract_container() {}
        virtual void output(void const *item) = 0;
        virtual void copy(OutputIterator& sink) = 0;
        virtual std::size_t buffer_size() = 0;
    };
    
    template <typename OutputIterator, typename T>
    class concrete_container : public abstract_container<OutputIterator>
    {
    public:
        concrete_container(std::size_t size)
        { 
            buffer.reserve(size); 
        }
        ~concrete_container() {}

        void output(void const *item)
        {
            buffer.push_back(*static_cast<T const*>(item));
        }
        void copy(OutputIterator& sink)
        {
            std::copy(buffer.begin(), buffer.end(), sink);
        }
        std::size_t buffer_size()
        {
            return buffer.size();
        }
    
    private:
        std::vector<T> buffer;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator>
    class buffer_sink : boost::noncopyable
    {
    public:
        buffer_sink()
          : width(0), buffer(0) 
        {}
        
        ~buffer_sink() 
        { 
            delete buffer; 
        }
        
        void init(std::size_t width_) { width = width_; }
        void tidy() { delete buffer; buffer = 0; width = 0; }
        
        template <typename T>
        void output(T const& value)
        {
            if (0 == buffer)
            {
                typedef concrete_container<OutputIterator, T> container;
                buffer = new container(width);
            }
            buffer->output(&value);
        }
        
        void copy(OutputIterator& sink) const 
        { 
            if (buffer) 
                buffer->copy(sink); 
        }
        
        std::size_t buffer_size() const 
        { 
            return buffer ? buffer->buffer_size() : 0; 
        }

    private:
        std::size_t width;
        abstract_container<OutputIterator> *buffer;
    };

    ///////////////////////////////////////////////////////////////////////////
    //  forward declaration only
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator> struct enable_counting;
    template <typename OutputIterator> struct enable_buffering;

    ///////////////////////////////////////////////////////////////////////////
    //  Karma uses a output iterator wrapper for all output operations. This
    //  is necessary to avoid the dreaded 'scanner business' problem, i.e. the
    //  dependency of rules and grammars on the used output iterator. 
    //
    //  By default the user supplied output iterator is wrapped inside an 
    //  instance of this internal output_iterator class. 
    //
    //  This output_iterator class normally just forwards to the embedded user
    //  supplied iterator. But it is possible to enable additional functionality
    //  on demand, such as counting, buffering, and position tracking.
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator, typename Enable = void>
    class output_iterator : boost::noncopyable
    {
    private:
        enum output_mode 
        {
            output_characters = 0,    // just hand through character
            count_characters = 1,     // additionally count characters
            buffer_characters = 2     // buffer all characters, no output
        };
        
        struct output_proxy 
        {
            output_proxy(output_iterator& parent) 
              : parent(parent) 
            {}
            
            template <typename T> 
            output_proxy& operator=(T const& value) 
            {
                parent.output(value);
                return *this; 
            }

        private:
            output_iterator& parent;

            // suppress warning about assignment operator not being generated
            output_proxy& operator=(output_proxy const&);
        };
        
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
private:
        friend struct enable_counting<output_iterator>;
        friend struct enable_buffering<output_iterator>;
#else
public:
#endif
        // functions related to counting
        void enable_counting(std::size_t count = 0)
        {
            count_data.init(count);
            mode = output_mode(mode | count_characters);
        }
        void disable_counting()
        {
            mode = output_mode(mode & ~count_characters);
        }
        void reset_counting()
        {
            count_data.tidy();
        }
        
        // functions related to buffering
        void enable_buffering(std::size_t width = 0)
        {
            buffer_data.init(width);
            mode = output_mode(mode | buffer_characters);
        }
        void disable_buffering()
        {
            mode = output_mode(mode & ~buffer_characters);
        }
        void reset_buffering()
        {
            buffer_data.tidy();
        }
        
    public:
        typedef std::output_iterator_tag iterator_category;
        typedef void value_type;
        typedef void difference_type;
        typedef void pointer;
        typedef void reference;

        output_iterator(OutputIterator& sink_)
          : sink(sink_), mode(output_characters)
        {}

        output_proxy operator*() { return output_proxy(*this); }
        output_iterator& operator++() { ++sink; return *this; } 
        output_iterator& operator++(int) { sink++; return *this; }

        template <typename T> 
        void output(T const& value) 
        { 
            if (mode & count_characters)    // count characters, if appropriate
                count_data.output();

            // always track position in the output (this is needed by different 
            // generators, such as indent, pad, etc.)
            track_position_data.output(value);

            if (mode & buffer_characters)   // buffer output, if appropriate
                buffer_data.output(value);
            else
                *sink = value; 
        }

        // functions related to counting
        std::size_t count() const
        {
            return count_data.get_count();
        }
        
        // functions related to buffering
        std::size_t buffer_size() const
        {
            return buffer_data.buffer_size();
        }
        void buffer_copy()
        {
            buffer_data.copy(sink);
        }
        
        // return the current count in the output
        std::size_t get_out_count() const
        {
            return track_position_data.get_count();
        }
        
    protected:
        // this is the wrapped user supplied output iterator
        OutputIterator& sink;

    private:
        // these are the hooks providing optional functionality
        counting_sink count_data;                   // for counting
        buffer_sink<OutputIterator> buffer_data;    // for buffering
        position_sink track_position_data;          // for position tracking
        int mode;
        
        // suppress warning about assignment operator not being generated
        output_iterator& operator=(output_iterator const&);
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Elem, typename Traits>
    class output_iterator<ostream_iterator<T, Elem, Traits> >
      : public output_iterator<ostream_iterator<T, Elem, Traits>, int>
    {
    private:
        typedef 
            output_iterator<ostream_iterator<T, Elem, Traits>, int> 
        base_type;
        typedef ostream_iterator<T, Elem, Traits> base_iterator_type;
        typedef std::basic_ostream<Elem, Traits> ostream_type;

    public:
        output_iterator(base_iterator_type& sink)
          : base_type(sink)
        {}

        ostream_type& get_ostream() { return this->sink.get_ostream(); }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  Helper class for exception safe enabling of character counting in the
    //  output iterator
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator>
    struct enable_counting
    {
        enable_counting(OutputIterator& sink_, std::size_t count = 0)
          : sink(sink_)
        {
            sink.enable_counting(count);
        }
        ~enable_counting()
        {
            sink.disable_counting();
            sink.reset_counting();
        }
        
        void disable()
        {
            sink.disable_counting();
        }

        OutputIterator& sink;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  Helper class for exception safe enabling of character buffering in the
    //  output iterator
    ///////////////////////////////////////////////////////////////////////////
    template <typename OutputIterator>
    struct enable_buffering
    {
        enable_buffering(OutputIterator& sink_, std::size_t width = 0)
          : sink(sink_)
        {
            sink.enable_buffering(width);
        }
        ~enable_buffering()
        {
            sink.disable_buffering();
            sink.reset_buffering();
        }
        
        void disable()
        {
            sink.disable_buffering();
        }
        
        OutputIterator& sink;
    };
    
}}}}

#endif 

