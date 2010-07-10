/* $Id$ */

/**
* @file common.hpp
* @brief Main definitions for project ana.
*
* ana: Asynchronous Network API.
* Copyright (C) 2010 Guillermo Biset.
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

#ifndef ANA_DETAIL_INTERNAL_HPP
#error "Private file, do not include directly."
#endif

namespace ana
{
    /** @name Type and constant definitions
     *
     * Definitions of main types and relevant constants.
     */
    //@{
        typedef boost::uint32_t ana_uint32  /** Standard unsigned int, with fixed size to 32 bits.  */ ;
        typedef boost::int32_t  ana_int32   /** Standard int, with fixed size to 32 bits.           */ ;
        typedef boost::uint16_t ana_uint16  /** Standard unsigned int, with fixed size to 16 bits.  */ ;
        typedef boost::int16_t  ana_int16   /** Standard int, with fixed size to 16 bits.           */ ;


        typedef ana_uint32  message_size       /** Message size type.                               */ ;

        typedef size_t      net_id             /** IDs of connected components, unique, non zero.   */ ;

        typedef std::string port               /** Port type, a std::string (instead of a short.)   */ ;
        typedef std::string address            /** Address type, a string. Either IP of hostname.   */ ;

        typedef bool        send_type          /** Send operation type, true to copy the buffer.    */ ;

        typedef boost::system::error_code error_code /** Standard error code, can evaluate to bool. */ ;

        const send_type ZERO_COPY   = false     /** Don't copy the buffer. */ ;
        const send_type COPY_BUFFER = true      /** Copy the buffer.       */ ;

        const message_size HEADER_LENGTH = sizeof(ana_uint32) /** Length of message header, 4 bytes.    */ ;
        const message_size INITIAL_RAW_MODE_BUFFER_SIZE = 256 /** Initial length of raw message buffer. */ ;
    //@}
}

#endif