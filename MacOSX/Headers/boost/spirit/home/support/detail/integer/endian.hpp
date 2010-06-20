//  Boost endian.hpp header file (proposed) ----------------------------------//

//  (C) Copyright Darin Adler 2000
//  (C) Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/endian

//----------------------------------------------------------------------------//

//  Original design developed by Darin Adler based on classes developed by Mark
//  Borgerding. Four original class templates combined into a single endian
//  class template by Beman Dawes, who also added the unrolled_byte_loops sign
//  partial specialization to correctly extend the sign when cover integer size
//  differs from endian representation size.

#ifndef BOOST_ENDIAN_HPP
#define BOOST_ENDIAN_HPP

#include <boost/detail/endian.hpp>
#include <boost/spirit/home/support/detail/integer/cover_operators.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>
#include <iosfwd>
#include <climits>

# if CHAR_BIT != 8
#   error Platforms with CHAR_BIT != 8 are not supported
# endif

namespace boost
{
  namespace detail
  {
    // Unrolled loops for loading and storing streams of bytes.

    template <typename T, std::size_t n_bytes,
      bool sign=boost::is_signed<T>::value >
    struct unrolled_byte_loops
    {
      typedef unrolled_byte_loops<T, n_bytes - 1, sign> next;

      static T load_big(const unsigned char* bytes)
        { return *(bytes - 1) | (next::load_big(bytes - 1) << 8); }
      static T load_little(const unsigned char* bytes)
        { return *bytes | (next::load_little(bytes + 1) << 8); }

      static void store_big(char* bytes, T value)
        {
          *(bytes - 1) = static_cast<char>(value);
          next::store_big(bytes - 1, value >> 8);
        }
      static void store_little(char* bytes, T value)
        {
          *bytes = static_cast<char>(value);
          next::store_little(bytes + 1, value >> 8);
        }
    };

    template <typename T>
    struct unrolled_byte_loops<T, 1, false>
    {
      static T load_big(const unsigned char* bytes)
        { return *(bytes - 1); }
      static T load_little(const unsigned char* bytes)
        { return *bytes; }
      static void store_big(char* bytes, T value)
        { *(bytes - 1) = static_cast<char>(value); }
      static void store_little(char* bytes, T value)
        { *bytes = static_cast<char>(value); }

    };

    template <typename T>
    struct unrolled_byte_loops<T, 1, true>
    {
      static T load_big(const unsigned char* bytes)
        { return *reinterpret_cast<const signed char*>(bytes - 1); }
      static T load_little(const unsigned char* bytes)
        { return *reinterpret_cast<const signed char*>(bytes); }
      static void store_big(char* bytes, T value)
        { *(bytes - 1) = static_cast<char>(value); }
      static void store_little(char* bytes, T value)
        { *bytes = static_cast<char>(value); }
    };

    template <typename T, std::size_t n_bytes>
    inline
    T load_big_endian(const void* bytes)
    {
      return unrolled_byte_loops<T, n_bytes>::load_big
        (static_cast<const unsigned char*>(bytes) + n_bytes);
    }

    template <typename T, std::size_t n_bytes>
    inline
    T load_little_endian(const void* bytes)
    {
      return unrolled_byte_loops<T, n_bytes>::load_little
        (static_cast<const unsigned char*>(bytes));
    }

    template <typename T, std::size_t n_bytes>
    inline
    void store_big_endian(void* bytes, T value)
    {
      unrolled_byte_loops<T, n_bytes>::store_big
        (static_cast<char*>(bytes) + n_bytes, value);
    }

    template <typename T, std::size_t n_bytes>
    inline
    void store_little_endian(void* bytes, T value)
    {
      unrolled_byte_loops<T, n_bytes>::store_little
        (static_cast<char*>(bytes), value);
    }

  } // namespace detail

  namespace integer
  {

  //  endian class template and specializations  -----------------------------//

    enum endianness { big, little, native };

    enum alignment { unaligned, aligned };

    template <endianness E, typename T, std::size_t n_bits,
      alignment A = unaligned>
    class endian;

    //  Specializations that represent unaligned bytes.
    //  Taking an integer type as a parameter provides a nice way to pass both
    //  the size and signedness of the desired integer and get the appropriate
    //  corresponding integer type for the interface.

    template <typename T, std::size_t n_bits>
    class endian< big, T, n_bits, unaligned >
      : cover_operators< endian< big, T, n_bits >, T >
    {
        BOOST_STATIC_ASSERT( (n_bits/8)*8 == n_bits );
      public:
        typedef T value_type;
        endian() {}
        endian(T i) { detail::store_big_endian<T, n_bits/8>(bytes, i); }
        operator T() const 
          { return detail::load_big_endian<T, n_bits/8>(bytes); }
      private:
        char bytes[n_bits/8];
    };

    template <typename T, std::size_t n_bits>
    class endian< little, T, n_bits, unaligned >
      : cover_operators< endian< little, T, n_bits >, T >
    {
        BOOST_STATIC_ASSERT( (n_bits/8)*8 == n_bits );
      public:
        typedef T value_type;
        endian() {}
        endian(T i) { detail::store_little_endian<T, n_bits/8>(bytes, i); }
        operator T() const
          { return detail::load_little_endian<T, n_bits/8>(bytes); }
      private:
        char bytes[n_bits/8];
    };

    template <typename T, std::size_t n_bits>
    class endian< native, T, n_bits, unaligned >
      : cover_operators< endian< native, T, n_bits >, T >
    {
        BOOST_STATIC_ASSERT( (n_bits/8)*8 == n_bits );
      public:
        typedef T value_type;
        endian() {}
#     ifdef BOOST_BIG_ENDIAN
        endian(T i) { detail::store_big_endian<T, n_bits/8>(bytes, i); }
        operator T() const
          { return detail::load_big_endian<T, n_bits/8>(bytes); }
#     else
        endian(T i) { detail::store_little_endian<T, n_bits/8>(bytes, i); }
        operator T() const
          { return detail::load_little_endian<T, n_bits/8>(bytes); }
#     endif
      private:
        char bytes[n_bits/8];
    };

    //  Specializations that mimic built-in integer types.
    //  These typically have the same alignment as the underlying types.

    template <typename T, std::size_t n_bits>
    class endian< big, T, n_bits, aligned  >
      : cover_operators< endian< big, T, n_bits, aligned >, T >
    {
        BOOST_STATIC_ASSERT( (n_bits/8)*8 == n_bits );
        BOOST_STATIC_ASSERT( sizeof(T) == n_bits/8 );
      public:
        typedef T value_type;
        endian() {}
    #ifdef BOOST_BIG_ENDIAN
        endian(T i) : integer(i) { }
        operator T() const { return integer; }
    #else
        endian(T i) { detail::store_big_endian<T, sizeof(T)>(&integer, i); }
        operator T() const
          { return detail::load_big_endian<T, sizeof(T)>(&integer); }
    #endif
      private:
       T integer;
    };

    template <typename T, std::size_t n_bits>
    class endian< little, T, n_bits, aligned  >
      : cover_operators< endian< little, T, n_bits, aligned >, T >
    {
        BOOST_STATIC_ASSERT( (n_bits/8)*8 == n_bits );
        BOOST_STATIC_ASSERT( sizeof(T) == n_bits/8 );
      public:
        typedef T value_type;
        endian() {}
    #ifdef BOOST_LITTLE_ENDIAN
        endian(T i) : integer(i) { }
        operator T() const { return integer; }
    #else
        endian(T i)
          { detail::store_little_endian<T, sizeof(T)>(&integer, i); }
        operator T() const
          { return detail::load_little_endian<T, sizeof(T)>(&integer); }
    #endif
      private:
        T integer;
    };

  //  naming convention typedefs  --------------------------------------------//

    // unaligned big endian signed integer types
    typedef endian< big, int_least8_t, 8 >           big8_t;
    typedef endian< big, int_least16_t, 16 >         big16_t;
    typedef endian< big, int_least32_t, 24 >         big24_t;
    typedef endian< big, int_least32_t, 32 >         big32_t;
    typedef endian< big, int_least64_t, 40 >         big40_t;
    typedef endian< big, int_least64_t, 48 >         big48_t;
    typedef endian< big, int_least64_t, 56 >         big56_t;
    typedef endian< big, int_least64_t, 64 >         big64_t;

    // unaligned big endian unsigned integer types
    typedef endian< big, uint_least8_t, 8 >          ubig8_t;
    typedef endian< big, uint_least16_t, 16 >        ubig16_t;
    typedef endian< big, uint_least32_t, 24 >        ubig24_t;
    typedef endian< big, uint_least32_t, 32 >        ubig32_t;
    typedef endian< big, uint_least64_t, 40 >        ubig40_t;
    typedef endian< big, uint_least64_t, 48 >        ubig48_t;
    typedef endian< big, uint_least64_t, 56 >        ubig56_t;
    typedef endian< big, uint_least64_t, 64 >        ubig64_t;

    // unaligned little endian signed integer types
    typedef endian< little, int_least8_t, 8 >        little8_t;
    typedef endian< little, int_least16_t, 16 >      little16_t;
    typedef endian< little, int_least32_t, 24 >      little24_t;
    typedef endian< little, int_least32_t, 32 >      little32_t;
    typedef endian< little, int_least64_t, 40 >      little40_t;
    typedef endian< little, int_least64_t, 48 >      little48_t;
    typedef endian< little, int_least64_t, 56 >      little56_t;
    typedef endian< little, int_least64_t, 64 >      little64_t;

    // unaligned little endian unsigned integer types
    typedef endian< little, uint_least8_t, 8 >       ulittle8_t;
    typedef endian< little, uint_least16_t, 16 >     ulittle16_t;
    typedef endian< little, uint_least32_t, 24 >     ulittle24_t;
    typedef endian< little, uint_least32_t, 32 >     ulittle32_t;
    typedef endian< little, uint_least64_t, 40 >     ulittle40_t;
    typedef endian< little, uint_least64_t, 48 >     ulittle48_t;
    typedef endian< little, uint_least64_t, 56 >     ulittle56_t;
    typedef endian< little, uint_least64_t, 64 >     ulittle64_t;

    // unaligned native endian signed integer types
    typedef endian< native, int_least8_t, 8 >        native8_t;
    typedef endian< native, int_least16_t, 16 >      native16_t;
    typedef endian< native, int_least32_t, 24 >      native24_t;
    typedef endian< native, int_least32_t, 32 >      native32_t;
    typedef endian< native, int_least64_t, 40 >      native40_t;
    typedef endian< native, int_least64_t, 48 >      native48_t;
    typedef endian< native, int_least64_t, 56 >      native56_t;
    typedef endian< native, int_least64_t, 64 >      native64_t;

    // unaligned native endian unsigned integer types
    typedef endian< native, uint_least8_t, 8 >       unative8_t;
    typedef endian< native, uint_least16_t, 16 >     unative16_t;
    typedef endian< native, uint_least32_t, 24 >     unative24_t;
    typedef endian< native, uint_least32_t, 32 >     unative32_t;
    typedef endian< native, uint_least64_t, 40 >     unative40_t;
    typedef endian< native, uint_least64_t, 48 >     unative48_t;
    typedef endian< native, uint_least64_t, 56 >     unative56_t;
    typedef endian< native, uint_least64_t, 64 >     unative64_t;

#define BOOST_HAS_INT16_T
#define BOOST_HAS_INT32_T
#define BOOST_HAS_INT64_T
  
  //  These types only present if platform has exact size integers:
  //     aligned big endian signed integer types
  //     aligned big endian unsigned integer types
  //     aligned little endian signed integer types
  //     aligned little endian unsigned integer types

  //     aligned native endian typedefs are not provided because
  //     <cstdint> types are superior for this use case

# if defined(BOOST_HAS_INT16_T)
    typedef endian< big, int16_t, 16, aligned >      aligned_big16_t;
    typedef endian< big, uint16_t, 16, aligned >     aligned_ubig16_t;
    typedef endian< little, int16_t, 16, aligned >   aligned_little16_t;
    typedef endian< little, uint16_t, 16, aligned >  aligned_ulittle16_t;
# endif

# if defined(BOOST_HAS_INT32_T)
    typedef endian< big, int32_t, 32, aligned >      aligned_big32_t;
    typedef endian< big, uint32_t, 32, aligned >     aligned_ubig32_t;
    typedef endian< little, int32_t, 32, aligned >   aligned_little32_t;
    typedef endian< little, uint32_t, 32, aligned >  aligned_ulittle32_t;
# endif

# if defined(BOOST_HAS_INT64_T)
    typedef endian< big, int64_t, 64, aligned >      aligned_big64_t;
    typedef endian< big, uint64_t, 64, aligned >     aligned_ubig64_t;
    typedef endian< little, int64_t, 64, aligned >   aligned_little64_t;
    typedef endian< little, uint64_t, 64, aligned >  aligned_ulittle64_t;
# endif

  } // namespace integer
} // namespace boost

#endif // BOOST_ENDIAN_HPP
