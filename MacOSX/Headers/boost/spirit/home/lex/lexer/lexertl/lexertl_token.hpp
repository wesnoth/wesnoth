//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEX_LEXERTL_TOKEN_FEB_10_2008_0751PM)
#define BOOST_SPIRIT_LEX_LEXERTL_TOKEN_FEB_10_2008_0751PM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once      // MS compatible compilers support #pragma once
#endif

#include <boost/spirit/home/qi/detail/assign_to.hpp>
#include <boost/spirit/home/support/placeholders.hpp>
#include <boost/spirit/home/support/detail/lexer/generator.hpp>
#include <boost/spirit/home/support/detail/lexer/rules.hpp>
#include <boost/spirit/home/support/detail/lexer/consts.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/range/iterator_range.hpp>

namespace boost { namespace spirit { namespace lex 
{ 
    ///////////////////////////////////////////////////////////////////////////
    //
    //  The lexertl_token is the type of the objects returned by default by the 
    //  lexertl_iterator.
    //
    //    template parameters:
    //        Iterator        The type of the iterator used to access the
    //                        underlying character stream.
    //        AttributeTypes  A mpl sequence containing the types of all 
    //                        required different token values to be supported 
    //                        by this token type.
    //        HasState        A mpl::bool_ indicating, whether this token type
    //                        should support lexer states.
    //
    //  It is possible to use other token types with the spirit::lex 
    //  framework as well. If you plan to use a different type as your token 
    //  type, you'll need to expose the following things from your token type 
    //  to make it compatible with spirit::lex:
    //
    //    typedefs
    //        iterator_type   The type of the iterator used to access the
    //                        underlying character stream.
    //
    //        id_type         The type of the token id used.
    //
    //    methods
    //        default constructor
    //                        This should initialize the token as an end of 
    //                        input token.
    //        constructors    The prototype of the other required 
    //                        constructors should be:
    //
    //              token(int)
    //                        This constructor should initialize the token as 
    //                        an invalid token (not carrying any specific 
    //                        values)
    //
    //              where:  the int is used as a tag only and its value is 
    //                      ignored
    //
    //                        and:
    //
    //              token(std::size_t id, std::size_t state, 
    //                    iterator_type first, iterator_type last);
    //
    //              where:  id:           token id
    //                      state:        lexer state this token was matched in
    //                      first, last:  pair of iterators marking the matched 
    //                                    range in the underlying input stream 
    //
    //        accessors
    //              id()      return the token id of the matched input sequence
    //
    //              state()   return the lexer state this token was matched in
    //
    //              value()   return the token value
    //
    //  Additionally, you will have to implement a couple of helper functions
    //  in the same namespace as the token type: a comparison operator==() to 
    //  compare your token instances, a token_is_valid() function and different 
    //  construct() function overloads as described below.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Iterator = char const*, 
        typename AttributeTypes = mpl::vector0<>, 
        typename HasState = mpl::true_
    >
    struct lexertl_token;

    ///////////////////////////////////////////////////////////////////////////
    //  This specialization of the token type doesn't contain any item data and
    //  doesn't support working with lexer states.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    struct lexertl_token<Iterator, omitted, mpl::false_>
    {
        typedef Iterator iterator_type;
        typedef mpl::false_ has_state;
        typedef std::size_t id_type;
        
        //  default constructed tokens correspond to EOI tokens
        lexertl_token() 
          : id_(boost::lexer::npos) 
        {}
        
        //  construct an invalid token
        lexertl_token(int)
          : id_(0) 
        {}
        
        lexertl_token(std::size_t id, std::size_t)
          : id_(id)
        {}
        
        lexertl_token(std::size_t id, std::size_t, 
                Iterator const&, Iterator const&)
          : id_(id)
        {}
        
        //  this default conversion operator is needed to allow the direct 
        //  usage of tokens in conjunction with the primitive parsers defined 
        //  in Qi
        operator std::size_t() const { return id_; }
        
        std::size_t id() const { return id_; }
        std::size_t state() const { return 0; }   // always '0' (INITIAL state)

    protected:
        std::size_t id_;         // token id, 0 if nothing has been matched
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  This specialization of the token type doesn't contain any item data but
    //  supports working with lexer states.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    struct lexertl_token<Iterator, omitted, mpl::true_>
      : lexertl_token<Iterator, omitted, mpl::false_>
    {
    private:
        typedef lexertl_token<Iterator, omitted, mpl::false_> base_type;

    public:
        typedef Iterator iterator_type;
        typedef mpl::true_ has_state;

        //  default constructed tokens correspond to EOI tokens
        lexertl_token() 
          : state_(boost::lexer::npos) 
        {}
        
        //  construct an invalid token
        lexertl_token(int)
          : base_type(0), state_(boost::lexer::npos) 
        {}
        
        lexertl_token(std::size_t id, std::size_t state)
          : base_type(id, boost::lexer::npos), state_(state)
        {}
        
        lexertl_token(std::size_t id, std::size_t state, 
                Iterator const&, Iterator const&)
          : base_type(id, boost::lexer::npos), state_(state)
        {}
        
        std::size_t state() const { return state_; }

    protected:
        std::size_t state_;      // lexer state this token was matched in
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  The generic version of the lexertl_token type derives from the 
    //  specialization above and adds a single data member holding the item 
    //  data carried by the token instance.
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        //  Metafunction to calculate the type of the variant data item to be 
        //  stored with each token instance.
        //
        //  Note: The iterator pair needs to be the first type in the list of 
        //        types supported by the generated variant type (this is being 
        //        used to identify whether the stored data item in a particular 
        //        token instance needs to be converted from the pair of 
        //        iterators (see the first of the construct() functions below).
        ///////////////////////////////////////////////////////////////////////
        template <typename IteratorPair, typename AttributeTypes>
        struct token_value_typesequence
        {
            typedef typename 
                mpl::insert<
                    AttributeTypes, 
                    typename mpl::begin<AttributeTypes>::type, 
                    IteratorPair
                >::type
            sequence_type;
            typedef typename make_variant_over<sequence_type>::type type;
        };
        
        ///////////////////////////////////////////////////////////////////////
        //  The type of the data item stored with a token instance is defined 
        //  by the template parameter 'AttributeTypes' and may be:
        //  
        //     omitted:           no data item is stored with the token 
        //                        instance (this is handled by the 
        //                        specializations of the lexertl_token class
        //                        below)
        //     mpl::vector0<>:    each token instance stores a pair of 
        //                        iterators pointing to the matched input 
        //                        sequence
        //     mpl::vector<...>:  each token instance stores a variant being 
        //                        able to store the pair of iterators pointing 
        //                        to the matched input sequence, or any of the 
        //                        types a specified in the mpl::vector<>
        //
        //  All this is done to ensure the token type is as small (in terms 
        //  of its byte-size) as possible.
        ///////////////////////////////////////////////////////////////////////
        template <typename IteratorPair, typename AttributeTypes>
        struct token_value_type
        {
            typedef 
                typename mpl::eval_if<
                    is_same<AttributeTypes, mpl::vector0<> >,
                    mpl::identity<IteratorPair>,
                    token_value_typesequence<IteratorPair, AttributeTypes>
                >::type 
            type;
        };
    }

    template <typename Iterator, typename AttributeTypes, typename HasState>
    struct lexertl_token : lexertl_token<Iterator, omitted, HasState>
    {
    private: // precondition assertions
#if !BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
        BOOST_STATIC_ASSERT((mpl::is_sequence<AttributeTypes>::value || 
                            is_same<AttributeTypes, omitted>::value));
#endif
        typedef lexertl_token<Iterator, omitted, HasState> base_type;
        
    protected: 
        //  If no additional token value types are given, the the token will 
        //  hold the plain pair of iterators pointing to the matched range
        //  in the underlying input sequence. Otherwise the token value is 
        //  stored as a variant and will again hold the pair of iterators but
        //  is able to hold any of the given data types as well. The conversion 
        //  from the iterator pair to the required data type is done when it is
        //  accessed for the first time.
        typedef iterator_range<Iterator> iterpair_type;
        typedef 
            typename detail::token_value_type<iterpair_type, AttributeTypes>::type 
        token_value_type;

    public:
        typedef Iterator iterator_type;

        //  default constructed tokens correspond to EOI tokens
        lexertl_token() 
          : value_(iterpair_type(iterator_type(), iterator_type())) 
        {}
        
        //  construct an invalid token
        lexertl_token(int)
          : base_type(0), value_(iterpair_type(iterator_type(), iterator_type())) 
        {}
        
        lexertl_token(std::size_t id, std::size_t state, 
                Iterator first, Iterator last)
          : base_type(id, state), value_(iterpair_type(first, last))
        {}

        token_value_type& value() { return value_; }
        token_value_type const& value() const { return value_; }
        
    protected:
        token_value_type value_; // token value, by default a pair of iterators
    };
    
    ///////////////////////////////////////////////////////////////////////////
    //  tokens are considered equal, if their id's match (these are unique)
    template <typename Iterator, typename AttributeTypes, typename HasState>
    inline bool 
    operator== (lexertl_token<Iterator, AttributeTypes, HasState> const& lhs, 
                lexertl_token<Iterator, AttributeTypes, HasState> const& rhs)
    {
        return lhs.id() == rhs.id();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //  This overload is needed by the multi_pass/functor_input_policy to 
    //  validate a token instance. It has to be defined in the same namespace 
    //  as the token class itself to allow ADL to find it.
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator, typename AttributeTypes, typename HasState>
    inline bool 
    token_is_valid(lexertl_token<Iterator, AttributeTypes, HasState> const& t)
    {
        return 0 != t.id() && std::size_t(boost::lexer::npos) != t.id();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //  We have to provide overloads for the construct() function allowing
    //  to extract the needed value from the token. These overloads have to be
    //  defined in the same namespace as the token class itself to allow ADL to 
    //  find them.
    ///////////////////////////////////////////////////////////////////////////
    
    //  This is called from the parse function of token_def if the token_def
    //  has been defined to carry a special attribute type
    template <typename Attribute, typename Iterator, typename AttributeTypes,
        typename HasState>
    inline void construct(Attribute& attr, 
        lexertl_token<Iterator, AttributeTypes, HasState>& t)
    {
    //  The goal of this function is to avoid the conversion of the pair of
    //  iterators (to the matched character sequence) into the token value 
    //  of the required type being done more than once. For this purpose it 
    //  checks whether the stored value type is still the default one (pair 
    //  of iterators) and if yes, replaces the pair of iterators with the 
    //  converted value to be returned from subsequent calls.
        
        if (0 == t.value().which()) {
        //  first access to the token value
            typedef iterator_range<Iterator> iterpair_type;
            iterpair_type const& ip = get<iterpair_type>(t.value());
            
        // Interestingly enough we use the assign_to() framework defined in 
        // Spirit.Qi allowing to convert the pair of iterators to almost any 
        // required type (assign_to(), if available, uses the standard Spirit 
        // parsers to do the conversion, and falls back to boost::lexical_cast
        // otherwise).
            qi::detail::assign_to(ip.begin(), ip.end(), attr);

        //  If you get an error during the compilation of the following 
        //  assignment expression, you probably forgot to list one or more 
        //  types used as token value types (in your token_def<...> 
        //  definitions) in your definition of the token class. I.e. any token 
        //  value type used for a token_def<...> definition has to be listed 
        //  during the declaration of the token type to use. For instance let's 
        //  assume we have two token_def's:
        //
        //      token_def<int> number; number = "...";
        //      token_def<std::string> identifier; identifier = "...";
        //
        //  Then you'll have to use the following token type definition 
        //  (assuming you are using the lexertl_token class):
        //
        //      typedef mpl::vector<int, std::string> token_values;
        //      typedef lexertl_token<base_iter_type, token_values> token_type;
        //
        //  where: base_iter_type is the iterator type used to expose the 
        //         underlying input stream.
        //
        //  This token_type has to be used as the second template parameter 
        //  to the lexer class:
        //
        //      typedef lexertl_lexer<base_iter_type, token_type> lexer_type;
        //
        //  again, assuming you're using the lexertl_lexer<> template for your 
        //  tokenization.
        
            t.value() = attr;   // re-assign value
        }
        else {
        // reuse the already assigned value
            qi::detail::assign_to(get<Attribute>(t.value()), attr);
        }
    }
    
    //  This is called from the parse function of token_def if the token type
    //  has no special attribute type assigned 
    template <typename Attribute, typename Iterator, typename HasState>
    inline void construct(Attribute& attr, 
        lexertl_token<Iterator, mpl::vector0<>, HasState>& t)
    {
    //  The default type returned by the token_def parser component (if it
    //  has no token value type assigned) is the pair of iterators to the 
    //  matched character sequence.
        
        qi::detail::assign_to(t.value().begin(), t.value().end(), attr);
    }
    
    //  This is called from the parse function of token_def if the token type
    //  has been explicitly omitted (i.e. no attribute value is used), which
    //  essentially means that every attribute gets initialized using default 
    //  constructed values.
    template <typename Attribute, typename Iterator, typename HasState>
    inline void
    construct(Attribute& attr, lexertl_token<Iterator, omitted, HasState>& t)
    {
    }
    
    //  This is called from the parse function of token_set or lexer_def_
    template <typename Iterator, typename AttributeTypes, typename HasState>
    inline void
    construct(fusion::vector<std::size_t, iterator_range<Iterator> >& attr,
        lexertl_token<Iterator, AttributeTypes, HasState> const& t)
    {
    //  The type returned by the token_set and lexer_def_ parser components
    //  is a fusion::vector containing the token id of the matched token 
    //  and the pair of iterators to the matched character sequence.
        
        typedef iterator_range<Iterator> iterpair_type;
        typedef 
            fusion::vector<std::size_t, iterator_range<Iterator> > 
        attribute_type;
        
        iterpair_type const& ip = get<iterpair_type>(t.value());
        attr = attribute_type(t.id(), get<iterpair_type>(t.value()));
    }

}}}

#endif
