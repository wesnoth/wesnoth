//  Copyright (c) 2001-2008 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_LEXERTL_ITERATOR_TOKENISER_MARCH_22_2007_0859AM)
#define BOOST_SPIRIT_LEXERTL_ITERATOR_TOKENISER_MARCH_22_2007_0859AM

#include <boost/detail/iterator.hpp>
#include <boost/spirit/home/support/detail/lexer/state_machine.hpp>
#include <boost/spirit/home/support/detail/lexer/consts.hpp>
#include <boost/spirit/home/support/detail/lexer/size_t.hpp>
#include <boost/spirit/home/support/detail/lexer/char_traits.hpp>
#include <vector>

namespace boost { namespace spirit { namespace lex 
{ 
    template<typename Iterator>
    class basic_iterator_tokeniser
    {
    public:
        typedef std::vector<std::size_t> size_t_vector;
        typedef 
            typename boost::detail::iterator_traits<Iterator>::value_type 
        char_type;

//         static std::size_t next (const std::size_t * const lookup_,
//             std::size_t const dfa_alphabet_, const std::size_t *  const dfa_,
//             Iterator const& start_, Iterator &start_token_,
//             Iterator const& end_)
//         {
//             if (start_token_ == end_) return 0;
// 
//             const std::size_t *ptr_ = dfa_ + dfa_alphabet_;
//             Iterator curr_ = start_token_;
//             bool end_state_ = *ptr_ != 0;
//             std::size_t id_ = *(ptr_ + lexer::id_index);
//             Iterator end_token_ = start_token_;
// 
//             while (curr_ != end_)
//             {
//                 std::size_t const BOL_state_ = ptr_[lexer::bol_index];
//                 std::size_t const EOL_state_ = ptr_[lexer::eol_index];
// 
//                 if (BOL_state_ && (start_token_ == start_ ||
//                     *(start_token_ - 1) == '\n'))
//                 {
//                     ptr_ = &dfa_[BOL_state_ * dfa_alphabet_];
//                 }
//                 else if (EOL_state_ && *curr_ == '\n')
//                 {
//                     ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];
//                 }
//                 else
//                 {
//                     std::size_t const state_ = ptr_[lookup_[*curr_++]];
// 
//                     if (state_ == 0)
//                     {
//                         break;
//                     }
// 
//                     ptr_ = &dfa_[state_ * dfa_alphabet_];
//                 }
// 
//                 if (*ptr_)
//                 {
//                     end_state_ = true;
//                     id_ = *(ptr_ + lexer::id_index);
//                     end_token_ = curr_;
//                 }
//             }
// 
//             const std::size_t EOL_state_ = ptr_[lexer::eol_index];
// 
//             if (EOL_state_ && curr_ == end_)
//             {
//                 ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];
// 
//                 if (*ptr_)
//                 {
//                     end_state_ = true;
//                     id_ = *(ptr_ + lexer::id_index);
//                     end_token_ = curr_;
//                 }
//             }
// 
//             if (end_state_) {
//                 // return longest match
//                 start_token_ = end_token_;
//             }
//             else {
//                 id_ = lexer::npos;
//             }
// 
//             return id_;
//         }

        static std::size_t next (
            boost::lexer::basic_state_machine<char_type> const& state_machine_,
            std::size_t &dfa_state_, Iterator const& start_,
            Iterator &start_token_, Iterator const& end_)
        {
            if (start_token_ == end_) return 0;

        again:
            std::size_t const* lookup_ = &state_machine_._lookup[dfa_state_]->
                front ();
            std::size_t dfa_alphabet_ = state_machine_._dfa_alphabet[dfa_state_];
            std::size_t const* dfa_ = &state_machine_._dfa[dfa_state_]->front ();
            std::size_t const* ptr_ = dfa_ + dfa_alphabet_;
            Iterator curr_ = start_token_;
            bool end_state_ = *ptr_ != 0;
            std::size_t id_ = *(ptr_ + boost::lexer::id_index);
            Iterator end_token_ = start_token_;

            while (curr_ != end_)
            {
                std::size_t const BOL_state_ = ptr_[boost::lexer::bol_index];
                std::size_t const EOL_state_ = ptr_[boost::lexer::eol_index];

                if (BOL_state_ && (start_token_ == start_ ||
                    *(start_token_ - 1) == '\n'))
                {
                    ptr_ = &dfa_[BOL_state_ * dfa_alphabet_];
                }
                else if (EOL_state_ && *curr_ == '\n')
                {
                    ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];
                }
                else
                {
                    typedef typename 
                        boost::detail::iterator_traits<Iterator>::value_type 
                    value_type;
                    typedef typename 
                        boost::lexer::char_traits<value_type>::index_type 
                    index_type;
                    
                    index_type index = 
                        boost::lexer::char_traits<value_type>::call(*curr_++);
                    std::size_t const state_ = ptr_[
                        lookup_[static_cast<std::size_t>(index)]];

                    if (state_ == 0)
                    {
                        break;
                    }

                    ptr_ = &dfa_[state_ * dfa_alphabet_];
                }

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + boost::lexer::id_index);
                    dfa_state_ = *(ptr_ + boost::lexer::state_index);
                    end_token_ = curr_;
                }
            }

            std::size_t const EOL_state_ = ptr_[boost::lexer::eol_index];

            if (EOL_state_ && curr_ == end_)
            {
                ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + boost::lexer::id_index);
                    dfa_state_ = *(ptr_ + boost::lexer::state_index);
                    end_token_ = curr_;
                }
            }

            if (end_state_) {
                // return longest match
                start_token_ = end_token_;

                if (id_ == 0) 
                    goto again;
            }
            else {
                id_ = boost::lexer::npos;
            }
            
            return id_;
        }

        ///////////////////////////////////////////////////////////////////////
        static 
        std::size_t next (
            boost::lexer::basic_state_machine<char_type> const& state_machine_,
            Iterator const& start_, Iterator &start_token_, Iterator const& end_)
        {
            if (start_token_ == end_) return 0;

            std::size_t const* lookup_ = &state_machine_._lookup[0]->front();
            std::size_t dfa_alphabet_ = state_machine_._dfa_alphabet[0];
            std::size_t const* dfa_ = &state_machine_._dfa[0]->front ();
            std::size_t const* ptr_ = dfa_ + dfa_alphabet_;
            Iterator curr_ = start_token_;
            bool end_state_ = *ptr_ != 0;
            std::size_t id_ = *(ptr_ + boost::lexer::id_index);
            Iterator end_token_ = start_token_;

            while (curr_ != end_)
            {
                std::size_t const BOL_state_ = ptr_[boost::lexer::bol_index];
                std::size_t const EOL_state_ = ptr_[boost::lexer::eol_index];

                if (BOL_state_ && (start_token_ == start_ ||
                    *(start_token_ - 1) == '\n'))
                {
                    ptr_ = &dfa_[BOL_state_ * dfa_alphabet_];
                }
                else if (EOL_state_ && *curr_ == '\n')
                {
                    ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];
                }
                else
                {
                    typedef typename 
                        boost::detail::iterator_traits<Iterator>::value_type 
                    value_type;
                    typedef typename 
                        boost::lexer::char_traits<value_type>::index_type 
                    index_type;
                    
                    index_type index = 
                        boost::lexer::char_traits<value_type>::call(*curr_++);
                    std::size_t const state_ = ptr_[
                        lookup_[static_cast<std::size_t>(index)]];

                    if (state_ == 0)
                    {
                        break;
                    }

                    ptr_ = &dfa_[state_ * dfa_alphabet_];
                }

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + boost::lexer::id_index);
                    end_token_ = curr_;
                }
            }

            std::size_t const EOL_state_ = ptr_[boost::lexer::eol_index];

            if (EOL_state_ && curr_ == end_)
            {
                ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + boost::lexer::id_index);
                    end_token_ = curr_;
                }
            }

            if (end_state_) {
                // return longest match
                start_token_ = end_token_;
            }
            else {
                id_ = boost::lexer::npos;
            }
            
            return id_;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    typedef basic_iterator_tokeniser<char const *> tokeniser;
    typedef basic_iterator_tokeniser<wchar_t const *> wtokeniser;

}}}

#endif
