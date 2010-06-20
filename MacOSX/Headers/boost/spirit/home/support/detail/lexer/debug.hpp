// debug.hpp
// Copyright (c) 2007 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_LEXER_DEBUG_HPP
#define BOOST_LEXER_DEBUG_HPP

#include <map>
#include <ostream>
#include "size_t.hpp"
#include "state_machine.hpp"
#include "string_token.hpp"
#include <vector>

namespace boost
{
namespace lexer
{
template<typename CharT>
class basic_debug
{
public:
    typedef std::basic_ostream<CharT> ostream;
    typedef std::basic_string<CharT> string;
    typedef std::vector<std::size_t> size_t_vector;

    static void escape_control_chars (const string &in_, string &out_)
    {
        const CharT *ptr_ = in_.c_str ();
        std::size_t size_ = in_.size ();

#if defined _MSC_VER && _MSC_VER <= 1200
        out_.erase ();
#else
        out_.clear ();
#endif

        while (size_)
        {
            switch (*ptr_)
            {
                case '\0':
                    out_ += '\\';
                    out_ += '0';
                    break;
                case '\a':
                    out_ += '\\';
                    out_ += 'a';
                    break;
                case '\b':
                    out_ += '\\';
                    out_ += 'b';
                    break;
                case 27:
                    out_ += '\\';
                    out_ += 'x';
                    out_ += '1';
                    out_ += 'b';
                    break;
                case '\f':
                    out_ += '\\';
                    out_ += 'f';
                    break;
                case '\n':
                    out_ += '\\';
                    out_ += 'n';
                    break;
                case '\r':
                    out_ += '\\';
                    out_ += 'r';
                    break;
                case '\t':
                    out_ += '\\';
                    out_ += 't';
                    break;
                case '\v':
                    out_ += '\\';
                    out_ += 'v';
                    break;
                case '\\':
                    out_ += '\\';
                    out_ += '\\';
                    break;
                case '"':
                    out_ += '\\';
                    out_ += '"';
                    break;
                default:
                {
                    if (*ptr_ < 32 && *ptr_ >= 0)
                    {
                        stringstream ss_;

                        out_ += '\\';
                        out_ += 'x';
                        ss_ << std::hex <<
                            static_cast<std::size_t> (*ptr_);
                        out_ += ss_.str ();
                    }
                    else
                    {
                        out_ += *ptr_;
                    }

                    break;
                }
            }

            ++ptr_;
            --size_;
        }
    }

    static void dump (const basic_state_machine<CharT> &state_machine_, ostream &stream_)
    {
        typename basic_state_machine<CharT>::iterator iter_ =
            state_machine_.begin ();
        typename basic_state_machine<CharT>::iterator end_ =
            state_machine_.end ();

        for (std::size_t dfa_ = 0, dfas_ = state_machine_.size ();
            dfa_ < dfas_; ++dfa_)
        {
            const std::size_t states_ = iter_->states;

            for (std::size_t i_ = 0; i_ < states_; ++i_)
            {
                state (stream_);
                stream_ << i_ << std::endl;

                if (iter_->end_state)
                {
                    end_state (stream_);
                    stream_ << iter_->id;
                    dfa (stream_);
                    stream_ << iter_->goto_dfa;
                    stream_ << std::endl;
                }

                if (iter_->bol_index != npos)
                {
                    bol (stream_);
                    stream_ << iter_->bol_index << std::endl;
                }

                if (iter_->eol_index != npos)
                {
                    eol (stream_);
                    stream_ << iter_->eol_index << std::endl;
                }

                const std::size_t transitions_ = iter_->transitions;

                if (transitions_ == 0)
                {
                    ++iter_;
                }

                for (std::size_t t_ = 0; t_ < transitions_; ++t_)
                {
                    std::size_t goto_state_ = iter_->goto_state;

                    if (iter_->token.any ())
                    {
                        any (stream_);
                    }
                    else
                    {
                        open_bracket (stream_);

                        if (iter_->token._negated)
                        {
                            negated (stream_);
                        }

                        string charset_;
                        CharT c_ = 0;

                        escape_control_chars (iter_->token._charset,
                            charset_);
                        c_ = *charset_.c_str ();

                        if (!iter_->token._negated &&
                            (c_ == '^' || c_ == ']'))
                        {
                            stream_ << '\\';
                        }

                        stream_ << charset_;
                        close_bracket (stream_);
                    }

                    stream_ << goto_state_ << std::endl;
                    ++iter_;
                }

                stream_ << std::endl;
            }
        }
    }

protected:
    typedef std::basic_stringstream<CharT> stringstream;

    static void state (std::basic_ostream<char> &stream_)
    {
        stream_ << "State: ";
    }

    static void state (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"State: ";
    }

    static void bol (std::basic_ostream<char> &stream_)
    {
        stream_ << "  BOL -> ";
    }

    static void bol (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"  BOL -> ";
    }

    static void eol (std::basic_ostream<char> &stream_)
    {
        stream_ << "  EOL -> ";
    }

    static void eol (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"  EOL -> ";
    }

    static void end_state (std::basic_ostream<char> &stream_)
    {
        stream_ << "  END STATE, Id = ";
    }

    static void end_state (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"  END STATE, Id = ";
    }

    static void any (std::basic_ostream<char> &stream_)
    {
        stream_ << "  . -> ";
    }

    static void any (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"  . -> ";
    }

    static void open_bracket (std::basic_ostream<char> &stream_)
    {
        stream_ << "  [";
    }

    static void open_bracket (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"  [";
    }

    static void negated (std::basic_ostream<char> &stream_)
    {
        stream_ << "^";
    }

    static void negated (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"^";
    }

    static void close_bracket (std::basic_ostream<char> &stream_)
    {
        stream_ << "] -> ";
    }

    static void close_bracket (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L"] -> ";
    }

    static void dfa (std::basic_ostream<char> &stream_)
    {
        stream_ << ", dfa = ";
    }

    static void dfa (std::basic_ostream<wchar_t> &stream_)
    {
        stream_ << L", dfa = ";
    }
};

typedef basic_debug<char> debug;
typedef basic_debug<wchar_t> wdebug;
}
}

#endif
