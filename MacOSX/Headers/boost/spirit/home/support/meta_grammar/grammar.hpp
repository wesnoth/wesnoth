/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_GRAMMAR_OF_JAN_28_2007_0419PM)
#define BOOST_SPIRIT_GRAMMAR_OF_JAN_28_2007_0419PM

namespace boost { namespace spirit { namespace meta_grammar
{
    ///////////////////////////////////////////////////////////////////////////
    //  Each domain has a proto meta-grammar. This is the metafunction
    //  that return the domain's meta-grammar.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Domain>
    struct grammar;

}}}

#endif
