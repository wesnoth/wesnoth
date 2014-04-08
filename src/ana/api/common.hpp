
/**
* @file
* @brief Main definitions for project ana.
*
* ana: Asynchronous Network API.
* Copyright (C) 2010 - 2014 Guillermo Biset.
*
* This file is part of the ana project.
*
* System:         ana
* Language:       C++
*
* Author:         Guillermo Biset
* E-Mail:         billybiset AT gmail DOT com
*
* ana is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* ana is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ana.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef ANA_COMMON_HPP
#define ANA_COMMON_HPP

#include <boost/asio/detail/socket_ops.hpp>
#include <boost/cstdint.hpp>
#include <boost/system/error_code.hpp>

#include "../../global.hpp"

namespace ana
{
    /** @name Type and constant definitions
     *
     * Definitions of main types and relevant constants.
     */
    //@{
        /** Standard unsigned int, with fixed size to 32 bits.  */
        typedef boost::uint32_t ana_uint32 ;

        /** Standard int, with fixed size to 32 bits.           */
        typedef boost::int32_t  ana_int32  ;

        /** Standard unsigned int, with fixed size to 16 bits.  */
        typedef boost::uint16_t ana_uint16 ;

        /** Standard int, with fixed size to 16 bits.           */
        typedef boost::int16_t  ana_int16  ;

        /** Message size type.                                  */
        typedef ana_uint32  message_size   ;

        /** IDs of connected components, unique, non zero.      */
        typedef size_t      net_id         ;

        /** IDs of network operations, unique, zero = no-op.    */
        typedef size_t      operation_id   ;

        /** Port type, a std::string (instead of a short.)      */
        typedef std::string port           ;

        /** Address type, a string. Either IP of hostname.      */
        typedef std::string address        ;

        /** Send operation type, true to copy the buffer.       */
        typedef bool        send_type      ;

        /** Standard error code, can evaluate to bool.          */
        typedef boost::system::error_code error_code ;

        /** The default timeout error. */
        const ana::error_code operation_aborted =
                    boost::asio::error::make_error_code( boost::asio::error::operation_aborted );

        /** The default timeout error. */
        const ana::error_code timeout_error =
                    boost::asio::error::make_error_code( boost::asio::error::timed_out );

        /** A generic error. Used to describe an undefined error. */
        const ana::error_code generic_error =
                    boost::asio::error::make_error_code( boost::asio::error::fault );

        /** A network operation that didn't do anything.        */
        const operation_id no_operation = 0;

        /** An invalid net_id. Useful for error checking.       */
        const operation_id invalid_net_id = 0;


        /** Don't copy the buffer. */
        const send_type ZERO_COPY   = false;

        /** Copy the buffer.       */
        const send_type COPY_BUFFER = true ;

        /** Length of message header, 4 bytes.    */
        const message_size HEADER_LENGTH = sizeof(ana_uint32);

        /** Initial length of raw message buffer. */
        const message_size INITIAL_RAW_MODE_BUFFER_SIZE = 256 ;
    //@}
}

#endif

