//  (C) Copyright Gennadiy Rozental 2003-2008.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision: 49312 $
//
//  Description : implements support for test cases templates instantiated with 
//                sequence of test types
// ***************************************************************************

#ifndef BOOST_TEST_TEST_CASE_TEMPLATE_HPP_071894GER
#define BOOST_TEST_TEST_CASE_TEMPLATE_HPP_071894GER

// Boost
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type.hpp>
#include <boost/type_traits/is_const.hpp>

// STL
#include <typeinfo>

#include <boost/test/detail/suppress_warnings.hpp>

//____________________________________________________________________________//

#define BOOST_TEST_CASE_TEMPLATE( name, typelist )                          \
    boost::unit_test::ut_detail::template_test_case_gen<name,typelist >(    \
        BOOST_TEST_STRINGIZE( name ) )                                      \
/**/

//____________________________________________________________________________//

#define BOOST_TEST_CASE_TEMPLATE_FUNCTION( name, type_name )    \
template<typename type_name>                                    \
void BOOST_JOIN( name, _impl )( boost::type<type_name>* );      \
                                                                \
struct name {                                                   \
    template<typename TestType>                                 \
    static void run( boost::type<TestType>* frwrd = 0 )         \
    {                                                           \
       BOOST_JOIN( name, _impl )( frwrd );                      \
    }                                                           \
};                                                              \
                                                                \
template<typename type_name>                                    \
void BOOST_JOIN( name, _impl )( boost::type<type_name>* )       \
/**/

//____________________________________________________________________________//

namespace boost {

namespace unit_test {

namespace ut_detail {

// ************************************************************************** //
// **************          test_case_template_invoker          ************** //
// ************************************************************************** //

template<typename TestCaseTemplate,typename TestType>
class test_case_template_invoker {
public:
    void    operator()()    { TestCaseTemplate::run( (boost::type<TestType>*)0 ); }
};

//____________________________________________________________________________//

// ************************************************************************** //
// **************           generate_test_case_4_type          ************** //
// ************************************************************************** //

template<typename Generator,typename TestCaseTemplate>
struct generate_test_case_4_type {
    explicit    generate_test_case_4_type( const_string tc_name, Generator& G )
    : m_test_case_name( tc_name )
    , m_holder( G )
    {}

    template<typename TestType>
    void        operator()( mpl::identity<TestType> )
    {
        std::string full_name;
        assign_op( full_name, m_test_case_name, 0 );
        full_name += '<';
        full_name += typeid(TestType).name();
        if( boost::is_const<TestType>::value )
            full_name += " const";
        full_name += '>';

        m_holder.m_test_cases.push_back( 
            new test_case( full_name, test_case_template_invoker<TestCaseTemplate,TestType>() ) );
    }

private:
    // Data members
    const_string    m_test_case_name;
    Generator&      m_holder;
};

// ************************************************************************** //
// **************              test_case_template              ************** //
// ************************************************************************** //

template<typename TestCaseTemplate,typename TestTypesList>
class template_test_case_gen : public test_unit_generator {
public:
    // Constructor
    template_test_case_gen( const_string tc_name )
    {
        typedef generate_test_case_4_type<template_test_case_gen<TestCaseTemplate,TestTypesList>,
                                          TestCaseTemplate
        > single_test_gen;
        mpl::for_each<TestTypesList,mpl::make_identity<mpl::_> >( single_test_gen( tc_name, *this ) );
    }

    test_unit* next() const
    {
        if( m_test_cases.empty() )
            return 0;
    
        test_unit* res = m_test_cases.front();
        m_test_cases.pop_front();

        return res;
    }

    // Data members
    mutable std::list<test_unit*> m_test_cases;
};

//____________________________________________________________________________//

} // namespace ut_detail

} // unit_test

} // namespace boost

//____________________________________________________________________________//

#include <boost/test/detail/enable_warnings.hpp>

#endif // BOOST_TEST_TEST_CASE_TEMPLATE_HPP_071894GER

