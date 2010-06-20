// input.hpp
// Copyright (c) 2008 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_LEXER_INPUT
#define BOOST_LEXER_INPUT

#include "char_traits.hpp"
#include <iterator>
#include "size_t.hpp"
#include "state_machine.hpp"

namespace boost
{
namespace lexer
{
template<typename FwdIter, typename Traits =
    char_traits<typename boost::detail::iterator_traits<FwdIter>::value_type> >
class basic_input
{
public:
    class iterator
    {
    public:
#if defined _MSC_VER && _MSC_VER <= 1200
        friend basic_input;
#else
        friend class basic_input;
#endif

        struct data
        {
            std::size_t id;
            FwdIter start;
            FwdIter end;
            bool bol;
            std::size_t state;

            // Construct in end() state.
            data () :
                id (0),
                bol (false),
                state (npos)
            {
            }

            bool operator == (const data &rhs_) const
            {
                return id == rhs_.id && start == rhs_.start &&
                    end == rhs_.end && bol == rhs_.bol && state == rhs_.state;
            }
        };

        iterator () :
            _input (0)
        {
        }

        bool operator == (const iterator &rhs_) const
        {
            return _data == rhs_._data;
        }

        bool operator != (const iterator &rhs_) const
        {
            return !(*this == rhs_);
        }

        data &operator * ()
        {
            return _data;
        }

        data *operator -> ()
        {
            return &_data;
        }

        // Let compiler generate operator = ().

        // prefix version
        iterator &operator ++ ()
        {
            next_token ();
            return *this;
        }

        // postfix version
        iterator operator ++ (int)
        {
            iterator iter_ = *this;

            next_token ();
            return iter_;
        }

    private:
        // Not owner (obviously!)
        const basic_input *_input;
        data _data;

        void next_token ()
        {
            _data.start = _data.end;

            if (_input->_state_machine->_dfa->size () == 1)
            {
                if (_input->_state_machine->_seen_BOL_assertion ||
                    _input->_state_machine->_seen_EOL_assertion)
                {
                    _data.id = next
                        (&_input->_state_machine->_lookup->front ()->front (),
                        _input->_state_machine->_dfa_alphabet.front (),
                        &_input->_state_machine->_dfa->front ()->front (),
                        _input->_begin, _data.end, _input->_end);
                }
                else
                {
                    _data.id = next (&_input->_state_machine->_lookup->
                        front ()->front (), _input->_state_machine->
                        _dfa_alphabet.front (), &_input->_state_machine->
                        _dfa->front ()->front (), _data.end, _input->_end);
                }
            }
            else
            {
                if (_input->_state_machine->_seen_BOL_assertion ||
                    _input->_state_machine->_seen_EOL_assertion)
                {
                    _data.id = next (*_input->_state_machine, _data.state,
                        _input->_begin, _data.end, _input->_end);
                }
                else
                {
                    _data.id = next (*_input->_state_machine, _data.state,
                        _data.end, _input->_end);
                }
            }

            if (_data.end == _input->_end && _data.start == _data.end)
            {
                // Ensure current state matches that returned by end().
                _data.state = npos;
            }
        }

        std::size_t next (const basic_state_machine
            <typename Traits::char_type> &state_machine_,
            std::size_t &start_state_, const FwdIter &start_,
            FwdIter &start_token_, const FwdIter &end_)
        {
            if (start_token_ == end_) return 0;

        again:
            bool bol_ = _data.bol;
            const std::size_t * lookup_ = &state_machine_._lookup[start_state_]->
                front ();
            std::size_t dfa_alphabet_ = state_machine_._dfa_alphabet[start_state_];
            const std::size_t *dfa_ = &state_machine_._dfa[start_state_]->front ();
            const std::size_t *ptr_ = dfa_ + dfa_alphabet_;
            FwdIter curr_ = start_token_;
            bool end_state_ = *ptr_ != 0;
            std::size_t id_ = *(ptr_ + id_index);
            bool end_bol_ = bol_;
            FwdIter end_token_ = start_token_;

            while (curr_ != end_)
            {
                const std::size_t BOL_state_ = ptr_[bol_index];
                const std::size_t EOL_state_ = ptr_[eol_index];

                if (BOL_state_ && bol_)
                {
                    ptr_ = &dfa_[BOL_state_ * dfa_alphabet_];
                }
                else if (EOL_state_ && *curr_ == '\n')
                {
                    ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];
                }
                else
                {
                    typename Traits::char_type prev_char_ = *curr_++;

                    bol_ = prev_char_ == '\n';

                    const std::size_t state_ =
                        ptr_[lookup_[static_cast<typename Traits::index_type>
                        (prev_char_)]];

                    if (state_ == 0)
                    {
                        break;
                    }

                    ptr_ = &dfa_[state_ * dfa_alphabet_];
                }

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + id_index);
                    start_state_ = *(ptr_ + state_index);
                    end_bol_ = bol_;
                    end_token_ = curr_;
                }
            }

            const std::size_t EOL_state_ = ptr_[eol_index];

            if (EOL_state_ && curr_ == end_)
            {
                ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + id_index);
                    start_state_ = *(ptr_ + state_index);
                    end_bol_ = bol_;
                    end_token_ = curr_;
                }
            }

            if (end_state_)
            {
                // return longest match
                _data.bol = end_bol_;
                start_token_ = end_token_;

                if (id_ == 0) goto again;
            }
            else
            {
                // No match causes char to be skipped
                _data.bol = *start_token_ == '\n';
                ++start_token_;
                id_ = npos;
            }

            return id_;
        }

        std::size_t next (const basic_state_machine
            <typename Traits::char_type> &state_machine_,
            std::size_t &start_state_, FwdIter &start_token_,
            FwdIter const &end_)
        {
            if (start_token_ == end_) return 0;

        again:
            const std::size_t * lookup_ = &state_machine_._lookup[start_state_]->
                front ();
            std::size_t dfa_alphabet_ = state_machine_._dfa_alphabet[start_state_];
            const std::size_t *dfa_ = &state_machine_._dfa[start_state_]->front ();
            const std::size_t *ptr_ = dfa_ + dfa_alphabet_;
            FwdIter curr_ = start_token_;
            bool end_state_ = *ptr_ != 0;
            std::size_t id_ = *(ptr_ + id_index);
            FwdIter end_token_ = start_token_;

            while (curr_ != end_)
            {
                const std::size_t state_ = ptr_[lookup_[static_cast
                    <typename Traits::index_type>(*curr_++)]];

                if (state_ == 0)
                {
                    break;
                }

                ptr_ = &dfa_[state_ * dfa_alphabet_];

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + id_index);
                    start_state_ = *(ptr_ + state_index);
                    end_token_ = curr_;
                }
            }

            if (end_state_)
            {
                // return longest match
                start_token_ = end_token_;

                if (id_ == 0) goto again;
            }
            else
            {
                // No match causes char to be skipped
                ++start_token_;
                id_ = npos;
            }

            return id_;
        }

        std::size_t next (const std::size_t * const lookup_,
            const std::size_t dfa_alphabet_, const std::size_t * const dfa_,
            FwdIter const &start_, FwdIter &start_token_, FwdIter const &end_)
        {
            if (start_token_ == end_) return 0;

            bool bol_ = _data.bol;
            const std::size_t *ptr_ = dfa_ + dfa_alphabet_;
            FwdIter curr_ = start_token_;
            bool end_state_ = *ptr_ != 0;
            std::size_t id_ = *(ptr_ + id_index);
            bool end_bol_ = bol_;
            FwdIter end_token_ = start_token_;

            while (curr_ != end_)
            {
                const std::size_t BOL_state_ = ptr_[bol_index];
                const std::size_t EOL_state_ = ptr_[eol_index];

                if (BOL_state_ && bol_)
                {
                    ptr_ = &dfa_[BOL_state_ * dfa_alphabet_];
                }
                else if (EOL_state_ && *curr_ == '\n')
                {
                    ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];
                }
                else
                {
                    typename Traits::char_type prev_char_ = *curr_++;

                    bol_ = prev_char_ == '\n';

                    const std::size_t state_ =
                        ptr_[lookup_[static_cast<typename Traits::index_type>
                        (prev_char_)]];

                    if (state_ == 0)
                    {
                        break;
                    }

                    ptr_ = &dfa_[state_ * dfa_alphabet_];
                }

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + id_index);
                    end_bol_ = bol_;
                    end_token_ = curr_;
                }
            }

            const std::size_t EOL_state_ = ptr_[eol_index];

            if (EOL_state_ && curr_ == end_)
            {
                ptr_ = &dfa_[EOL_state_ * dfa_alphabet_];

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + id_index);
                    end_bol_ = bol_;
                    end_token_ = curr_;
                }
            }

            if (end_state_)
            {
                // return longest match
                start_token_ = end_token_;
                _data.bol = end_bol_;
            }
            else
            {
                // No match causes char to be skipped
                _data.bol = *start_token_ == '\n';
                ++start_token_;
                id_ = npos;
            }

            return id_;
        }

        std::size_t next (const std::size_t * const lookup_,
            const std::size_t dfa_alphabet_, const std::size_t * const dfa_,
            FwdIter &start_token_, FwdIter const &end_)
        {
            if (start_token_ == end_) return 0;

            const std::size_t *ptr_ = dfa_ + dfa_alphabet_;
            FwdIter curr_ = start_token_;
            bool end_state_ = *ptr_ != 0;
            std::size_t id_ = *(ptr_ + id_index);
            FwdIter end_token_ = start_token_;

            while (curr_ != end_)
            {
                const std::size_t state_ = ptr_[lookup_[static_cast
                    <typename Traits::index_type>(*curr_++)]];

                if (state_ == 0)
                {
                    break;
                }

                ptr_ = &dfa_[state_ * dfa_alphabet_];

                if (*ptr_)
                {
                    end_state_ = true;
                    id_ = *(ptr_ + id_index);
                    end_token_ = curr_;
                }
            }

            if (end_state_)
            {
                // return longest match
                start_token_ = end_token_;
            }
            else
            {
                // No match causes char to be skipped
                ++start_token_;
                id_ = npos;
            }

            return id_;
        }
    };

#if defined _MSC_VER && _MSC_VER <= 1200
    friend iterator;
#else
    friend class iterator;
#endif

    // Make it explict that we are NOT taking a copy of state_machine_!
    basic_input (const basic_state_machine<typename Traits::char_type>
        *state_machine_, const FwdIter &begin_, const FwdIter &end_) :
        _state_machine (state_machine_),
        _begin (begin_),
        _end (end_)
    {
    }

    iterator begin () const
    {
        iterator iter_;

        iter_._input = this;
        iter_._data.id = npos;
        iter_._data.start = _begin;
        iter_._data.end = _begin;
        iter_._data.bol = _state_machine->_seen_BOL_assertion;
        iter_._data.state = 0;
        ++iter_;
        return iter_;
    }

    iterator end () const
    {
        iterator iter_;

        iter_._input = this;
        iter_._data.start = _end;
        iter_._data.end = _end;
        return iter_;
    }

private:
    const basic_state_machine<typename Traits::char_type> *_state_machine;
    FwdIter _begin;
    FwdIter _end;
};

typedef basic_input<std::string::iterator> iter_input;
typedef basic_input<std::wstring::iterator> iter_winput;
typedef basic_input<const char *> ptr_input;
typedef basic_input<const wchar_t *> ptr_winput;
}
}

#endif
