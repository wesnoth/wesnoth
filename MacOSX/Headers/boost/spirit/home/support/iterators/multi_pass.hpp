//  Copyright (c) 2001, Daniel C. Nuffer
//  Copyright (c) 2001-2008, Hartmut Kaiser
//  http://spirit.sourceforge.net/
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_ITERATOR_MULTI_PASS_MAR_16_2007_1124AM)
#define BOOST_SPIRIT_ITERATOR_MULTI_PASS_MAR_16_2007_1124AM

#include <boost/spirit/home/support/iterators/multi_pass_fwd.hpp>
#include <boost/spirit/home/support/iterators/detail/multi_pass.hpp>
#include <boost/spirit/home/support/iterators/detail/combine_policies.hpp>
#include <boost/limits.hpp>
#include <boost/detail/workaround.hpp>

namespace boost { namespace spirit 
{
    ///////////////////////////////////////////////////////////////////////////
    // The default multi_pass instantiation uses a ref-counted std_deque scheme.
    ///////////////////////////////////////////////////////////////////////////
    template<typename T, typename Policies>
    class multi_pass 
      : public Policies::BOOST_NESTED_TEMPLATE unique<T>
#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
      , typename iterator_base_creator<T, typename Policies::input_policy>::type
#endif
    {
    private:
        // unique and shared data types
        typedef typename Policies::BOOST_NESTED_TEMPLATE unique<T> 
            policies_base_type;
        typedef typename Policies::BOOST_NESTED_TEMPLATE shared<T> 
            shared_data_type;
                
        // define the types the standard embedded iterator typedefs are taken 
        // from
#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
        typedef typename iterator_base_creator<Input, T>::type iterator_type;
#else
        typedef typename policies_base_type::input_policy iterator_type;
#endif

    public:
        // standard iterator typedefs
        typedef std::forward_iterator_tag iterator_category;
        typedef typename iterator_type::value_type value_type;
        typedef typename iterator_type::difference_type difference_type;
        typedef typename iterator_type::distance_type distance_type;
        typedef typename iterator_type::reference reference;
        typedef typename iterator_type::pointer pointer;

        multi_pass()
          : shared(0)
        {}
        
        explicit multi_pass(T input)
          : shared(new shared_data_type(input)), policies_base_type(input)
        {}

#if BOOST_WORKAROUND(__GLIBCPP__, == 20020514)
        // The standard library shipped with gcc-3.1 has a bug in
        // bits/basic_string.tcc. It tries to use iter::iter(0) to
        // construct an iterator. Ironically, this  happens in sanity
        // checking code that isn't required by the standard.
        // The workaround is to provide an additional constructor that
        // ignores its int argument and behaves like the default constructor.
        multi_pass(int)
          : shared(0)
        {}
#endif // BOOST_WORKAROUND(__GLIBCPP__, == 20020514)

        ~multi_pass()
        {
            if (policies_base_type::release(*this)) {
                policies_base_type::destroy(*this);
                delete shared;
            }
        }

        multi_pass(multi_pass const& x)
          : shared(x.shared), policies_base_type(x)
        {
            policies_base_type::clone(*this);
        }
        
        multi_pass& operator=(multi_pass const& x)
        {
            if (this != &x) {
                multi_pass temp(x);
                temp.swap(*this);
            }
            return *this;
        }

        void swap(multi_pass& x)
        {
            spirit::detail::swap(shared, x.shared);
            this->policies_base_type::swap(x);
        }

        reference operator*() const
        {
            policies_base_type::check(*this);
            return policies_base_type::dereference(*this);
        }
        pointer operator->() const
        {
            return &(operator*());
        }

        multi_pass& operator++()
        {
            policies_base_type::check(*this);
            policies_base_type::increment(*this);
            return *this;
        }
        multi_pass operator++(int)
        {
            multi_pass tmp(*this);
            ++*this;
            return tmp;
        }

        void clear_queue()
        {
            policies_base_type::clear_queue(*this);
        }

        bool operator==(multi_pass const& y) const
        {
            if (is_eof())
                return y.is_eof();
            if (y.is_eof())
                return false;
                
            return policies_base_type::equal_to(*this, y);
        }
        bool operator<(multi_pass const& y) const
        {
            return policies_base_type::less_than(*this, y);
        }

    private: // helper functions
        bool is_eof() const
        {
            return (0 == shared) || policies_base_type::is_eof(*this);
        }
        
    public:
        shared_data_type *shared;
    };


    template <typename T, typename Policies>
    inline bool 
    operator!=(multi_pass<T, Policies> const& x, multi_pass<T, Policies> const& y)
    {
        return !(x == y);
    }

    template <typename T, typename Policies>
    inline bool 
    operator>(multi_pass<T, Policies> const& x, multi_pass<T, Policies> const& y)
    {
        return y < x;
    }

    template <typename T, typename Policies>
    inline bool 
    operator>=(multi_pass<T, Policies> const& x, multi_pass<T, Policies> const& y)
    {
        return !(x < y);
    }

    template <typename T, typename Policies>
    inline bool 
    operator<=(multi_pass<T, Policies> const& x, multi_pass<T, Policies> const& y)
    {
        return !(y < x);
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Generator function
    ///////////////////////////////////////////////////////////////////////////
    template <typename Policies, typename T>
    inline multi_pass<T, Policies>
    make_multi_pass(T i)
    {
        return multi_pass<T, Policies>(i);
    }

    template <typename T, typename Policies>
    inline void 
    swap(multi_pass<T, Policies> &x,
         multi_pass<T, Policies> &y)
    {
        x.swap(y);
    }

}} // namespace boost::spirit

#endif 


