/*=============================================================================
    Copyright (c) 2001-2007 Joel de Guzman
    Copyright (c) 2001-2008 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_PLACEHOLDERS_NOV_18_2006_0326PM)
#define BOOST_SPIRIT_PLACEHOLDERS_NOV_18_2006_0326PM

#include <boost/xpressive/proto/proto.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_enum.hpp>

namespace boost { namespace spirit
{
    //  This file contains the common placeholders. If you have a placeholder
    //  that can be (re)used in different spirit domains. This is the place
    //  to put them in.

    namespace tag
    {
        struct char_ {};
        struct wchar {};
        struct lit {};
        struct wlit {};
        struct eol {};
        struct eoi {};

        struct bin {};
        struct oct {};
        struct hex {};

        struct byte {};
        struct word {};
        struct dword {};
        struct big_word {};
        struct big_dword {};
        struct little_word {};
        struct little_dword {};
#ifdef BOOST_HAS_LONG_LONG
        struct qword {};
        struct big_qword {};
        struct little_qword {};
#endif
        struct pad {};

        struct ushort {};
        struct ulong {};
        struct uint {};
        struct short_ {};
        struct long_ {};
        struct int_ {};
#ifdef BOOST_HAS_LONG_LONG
        struct ulong_long {};
        struct long_long {};
#endif
        struct float_ {};
        struct double_ {};
        struct long_double {};

        struct left_align {};
        struct right_align {};
        struct center {};

        struct delimit {};
        struct verbatim {};

        struct none {};
        struct eps {};
        struct lexeme {};
        struct lazy {};
        struct omit {};
        struct raw {};

        struct stream {};
        struct wstream {};

        struct token {};
    }

    ////////////////////////////////////////////////////////////////////////////
    typedef proto::terminal<tag::char_>::type char_type;
    typedef proto::terminal<tag::wchar>::type wchar_type;
    typedef proto::terminal<tag::lit>::type lit_type;
    typedef proto::terminal<tag::wlit>::type wlit_type;
    typedef proto::terminal<tag::eol>::type eol_type;
    typedef proto::terminal<tag::eoi>::type eoi_type;
    
    typedef proto::terminal<tag::bin>::type bin_type;
    typedef proto::terminal<tag::oct>::type oct_type;
    typedef proto::terminal<tag::hex>::type hex_type;

    typedef proto::terminal<tag::byte>::type byte_type;
    typedef proto::terminal<tag::word>::type word_type;
    typedef proto::terminal<tag::dword>::type dword_type;
    typedef proto::terminal<tag::big_word>::type big_word_type;
    typedef proto::terminal<tag::big_dword>::type big_dword_type;
    typedef proto::terminal<tag::little_word>::type little_word_type;
    typedef proto::terminal<tag::little_dword>::type little_dword_type;
#ifdef BOOST_HAS_LONG_LONG
    typedef proto::terminal<tag::qword>::type qword_type;
    typedef proto::terminal<tag::big_qword>::type big_qword_type;
    typedef proto::terminal<tag::little_qword>::type little_qword_type;
#endif
    typedef proto::terminal<tag::pad>::type pad_type;

    typedef proto::terminal<tag::ushort>::type ushort_type;
    typedef proto::terminal<tag::ulong>::type ulong_type;
    typedef proto::terminal<tag::uint>::type uint_type;
    typedef proto::terminal<tag::short_>::type short_type;
    typedef proto::terminal<tag::long_>::type long_type;
    typedef proto::terminal<tag::int_>::type int_type;
#ifdef BOOST_HAS_LONG_LONG
    typedef proto::terminal<tag::ulong_long>::type ulong_long_type;
    typedef proto::terminal<tag::long_long>::type long_long_type;
#endif
    typedef proto::terminal<tag::float_>::type float_type;
    typedef proto::terminal<tag::double_>::type double_type;
    typedef proto::terminal<tag::long_double>::type long_double_type;

    typedef proto::terminal<tag::left_align>::type left_align_type;
    typedef proto::terminal<tag::right_align>::type right_align_type;
    typedef proto::terminal<tag::center>::type center_type;

    typedef proto::terminal<tag::delimit>::type delimit_type;
    typedef proto::terminal<tag::verbatim>::type verbatim_type;

    typedef proto::terminal<tag::none>::type none_type;
    typedef proto::terminal<tag::eps>::type eps_type;
    typedef proto::terminal<tag::lexeme>::type lexeme_type;
    typedef proto::terminal<tag::lazy>::type lazy_type;
    typedef proto::terminal<tag::omit>::type omitted;
    typedef proto::terminal<tag::raw>::type raw_type;

    typedef proto::terminal<tag::stream>::type stream_type;
    typedef proto::terminal<tag::wstream>::type wstream_type;

    typedef proto::terminal<tag::token>::type token_type;

    ////////////////////////////////////////////////////////////////////////////
    proto::terminal<tag::char_>::type const char_ = {{}};
    proto::terminal<tag::wchar>::type const wchar = {{}};
    proto::terminal<tag::lit>::type const lit = {{}};
    proto::terminal<tag::wlit>::type const wlit = {{}};
    proto::terminal<tag::eol>::type const eol = {{}};
    proto::terminal<tag::eoi>::type const eoi = {{}};
    
    proto::terminal<tag::bin>::type const bin = {{}};
    proto::terminal<tag::oct>::type const oct = {{}};
    proto::terminal<tag::hex>::type const hex = {{}};

    proto::terminal<tag::byte>::type const byte = {{}};
    proto::terminal<tag::word>::type const word = {{}};
    proto::terminal<tag::dword>::type const dword = {{}};
    proto::terminal<tag::big_word>::type const big_word = {{}};
    proto::terminal<tag::big_dword>::type const big_dword = {{}};
    proto::terminal<tag::little_word>::type const little_word = {{}};
    proto::terminal<tag::little_dword>::type const little_dword = {{}};
#ifdef BOOST_HAS_LONG_LONG
    proto::terminal<tag::qword>::type const qword = {{}};
    proto::terminal<tag::big_qword>::type const big_qword = {{}};
    proto::terminal<tag::little_qword>::type const little_qword = {{}};
#endif
    proto::terminal<tag::pad>::type const pad = {{}};

    proto::terminal<tag::ushort>::type const ushort = {{}};
    proto::terminal<tag::ulong>::type const ulong = {{}};
    proto::terminal<tag::uint>::type const uint = {{}};
    proto::terminal<tag::short_>::type const short_ = {{}};
    proto::terminal<tag::long_>::type const long_ = {{}};
    proto::terminal<tag::int_>::type const int_ = {{}};
#ifdef BOOST_HAS_LONG_LONG
    proto::terminal<tag::ulong_long>::type const ulong_long = {{}};
    proto::terminal<tag::long_long>::type const long_long = {{}};
#endif
    proto::terminal<tag::float_>::type const float_ = {{}};
    proto::terminal<tag::double_>::type const double_ = {{}};
    proto::terminal<tag::long_double>::type const long_double = {{}};

    proto::terminal<tag::left_align>::type const left_align = {{}};
    proto::terminal<tag::right_align>::type const right_align = {{}};
    proto::terminal<tag::center>::type const center = {{}};

    proto::terminal<tag::delimit>::type const delimit = {{}};
    proto::terminal<tag::verbatim>::type const verbatim = {{}};

    proto::terminal<tag::none>::type const none = {{}};
    proto::terminal<tag::eps>::type const eps = {{}};
    proto::terminal<tag::lexeme>::type const lexeme = {{}};
    proto::terminal<tag::lazy>::type const lazy = {{}};
    proto::terminal<tag::omit>::type const omit = {{}};
    proto::terminal<tag::raw>::type const raw = {{}};

    proto::terminal<tag::stream>::type const stream = {{}};
    proto::terminal<tag::wstream>::type const wstream = {{}};

    proto::terminal<tag::token>::type const token = {{}};

//  Some platforms/compilers have conflict with these terminals below
//  we'll provide variations for them with trailing underscores as
//  substitutes.

    proto::terminal<tag::uint>::type const uint_ = {{}};

#if defined(__GNUC__)
    inline void silence_unused_warnings__placeholders()
    {
        (void) char_; (void) wchar; (void) lit; (void) wlit;
        (void) eol; (void) eoi;
        (void) bin; (void) oct; (void) hex;
        (void) byte; (void) word; (void) dword; 
        (void) big_word; (void) big_dword; 
        (void) little_word; (void) little_dword; 
        (void) ushort; (void) uint; (void) ulong;
        (void) short_; (void) int_; (void) long_;
#ifdef BOOST_HAS_LONG_LONG
        (void) qword; (void) little_qword; (void) big_qword;
        (void) ulong_long; (void) long_long;
#endif
        (void) pad;
        (void) float_; (void) double_; (void) long_double;
        (void) left_align; (void) right_align; (void) center;
        (void) delimit; (void) verbatim;
        (void) none; (void) eps; (void) lazy; (void) lexeme; 
        (void) omit; (void) raw;
        (void) stream; (void) wstream;
        
        (void) token;
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is an int tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_int_tag : mpl::false_ {};

    template <typename Domain>
    struct is_int_tag<tag::bin, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::oct, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::hex, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::ushort, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::ulong, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::uint, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::short_, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::long_, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::int_, Domain> : mpl::true_ {};

#ifdef BOOST_HAS_LONG_LONG
    template <typename Domain>
    struct is_int_tag<tag::ulong_long, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_tag<tag::long_long, Domain> : mpl::true_ {};
#endif

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is an integer type
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_int_lit_tag : is_enum<T> {};

    template <typename Domain>
    struct is_int_lit_tag<short, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_int_lit_tag<unsigned short, Domain> : mpl::true_ {};
        
    template <typename Domain>
    struct is_int_lit_tag<int, Domain> : mpl::true_ {};
        
    template <typename Domain>
    struct is_int_lit_tag<unsigned int, Domain> : mpl::true_ {};
        
    template <typename Domain>
    struct is_int_lit_tag<long, Domain> : mpl::true_ {};
        
    template <typename Domain>
    struct is_int_lit_tag<unsigned long, Domain> : mpl::true_ {};
    
#ifdef BOOST_HAS_LONG_LONG
    template <typename Domain>
    struct is_int_lit_tag<boost::ulong_long_type, Domain> : mpl::true_ {};
        
    template <typename Domain>
    struct is_int_lit_tag<boost::long_long_type, Domain> : mpl::true_ {};
#endif

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is an floating point tag
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_real_tag : mpl::false_ {};

    template <typename Domain>
    struct is_real_tag<tag::float_, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_real_tag<tag::double_, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_real_tag<tag::long_double, Domain> : mpl::true_ {};

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is a floating type
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_real_lit_tag : mpl::false_ {};

    template <typename Domain>
    struct is_real_lit_tag<float, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_real_lit_tag<double, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_real_lit_tag<long double, Domain> : mpl::true_ {};

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is a character literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_char_tag : mpl::false_ {};

    template <typename Domain>
    struct is_char_tag<tag::char_, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_char_tag<tag::wchar, Domain> : mpl::true_ {};

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is a character literal type
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_lit_tag : mpl::false_ {};

    template <typename Domain>
    struct is_lit_tag<tag::lit, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_lit_tag<tag::wlit, Domain> : mpl::true_ {};

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is a binary type
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_binary_tag : mpl::false_ {};

    template <typename Domain>
    struct is_binary_tag<tag::byte, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::word, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::dword, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::big_word, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::big_dword, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::little_word, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::little_dword, Domain> : mpl::true_ {};

#ifdef BOOST_HAS_LONG_LONG
    template <typename Domain>
    struct is_binary_tag<tag::qword, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::big_qword, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_binary_tag<tag::little_qword, Domain> : mpl::true_ {};
#endif

    ///////////////////////////////////////////////////////////////////////////
    // test if a tag is a stream terminal 
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Domain>
    struct is_stream_tag : mpl::false_ {};

    template <typename Domain>
    struct is_stream_tag<tag::stream, Domain> : mpl::true_ {};

    template <typename Domain>
    struct is_stream_tag<tag::wstream, Domain> : mpl::true_ {};

}}

#endif
