//Copyright (c) 2006-2008 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_8D22C4CA9CC811DCAA9133D256D89593
#define UUID_8D22C4CA9CC811DCAA9133D256D89593

#include <boost/exception/exception.hpp>
#include <boost/exception/to_string_stub.hpp>
#include <boost/exception/detail/error_info_impl.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

namespace
boost
    {
    template <class Tag,class T>
    inline
    error_info<Tag,T>::
    error_info( value_type const & value ):
        value_(value)
        {
        }

    template <class Tag,class T>
    inline
    error_info<Tag,T>::
    ~error_info() throw()
        {
        }

    template <class Tag,class T>
    inline
    char const *
    error_info<Tag,T>::
    tag_typeid_name() const
        {
        return tag_type_name<Tag>();
        }

    template <class Tag,class T>
    inline
    std::string
    error_info<Tag,T>::
    value_as_string() const
        {
        return to_string_stub(value_);
        }

    namespace
    exception_detail
        {
        class
        error_info_container_impl:
            public error_info_container
            {
            public:

            error_info_container_impl():
                count_(0)
                {
                }

            ~error_info_container_impl() throw()
                {
                }

            void
            set( shared_ptr<error_info_base const> const & x, type_info_ const & typeid_ )
                {
                BOOST_ASSERT(x);
                info_[typeid_] = x;
                diagnostic_info_str_.clear();
                }

            shared_ptr<error_info_base const>
            get( type_info_ const & ti ) const
                {
                error_info_map::const_iterator i=info_.find(ti);
                if( info_.end()!=i )
                    {
                    shared_ptr<error_info_base const> const & p = i->second;
#ifndef BOOST_NO_RTTI
                    BOOST_ASSERT( BOOST_EXCEPTION_DYNAMIC_TYPEID(*p)==ti );
#endif
                    return p;
                    }
                return shared_ptr<error_info_base const>();
                }

            char const *
            diagnostic_information() const
                {
                if( diagnostic_info_str_.empty() )
                    {
                    std::string tmp;
                    for( error_info_map::const_iterator i=info_.begin(),end=info_.end(); i!=end; ++i )
                        {
                        shared_ptr<error_info_base const> const & x = i->second;
                        tmp += '[';
                        tmp += x->tag_typeid_name();
                        tmp += "] = ";
                        tmp += x->value_as_string();
                        tmp += '\n';
                        }
                    diagnostic_info_str_.swap(tmp);
                    }
                return diagnostic_info_str_.c_str();
                }

            private:

            friend class boost::exception;

            typedef std::map< type_info_, shared_ptr<error_info_base const> > error_info_map;
            error_info_map info_;
            mutable std::string diagnostic_info_str_;
            mutable int count_;

            void
            add_ref() const
                {
                ++count_;
                }

            void
            release() const
                {
                if( !--count_ )
                    delete this;
                }
            };
        }

    template <class E,class Tag,class T>
    inline
    E const &
    operator<<( E const & x, error_info<Tag,T> const & v )
        {
        typedef error_info<Tag,T> error_info_tag_t;
        shared_ptr<error_info_tag_t> p( new error_info_tag_t(v) );
        exception_detail::error_info_container * c;
        if( !(c=x.data_.get()) )
            x.data_.adopt(c=new exception_detail::error_info_container_impl);
        c->set(p,BOOST_EXCEPTION_STATIC_TYPEID(error_info_tag_t));
        return x;
        }
    }

#endif
