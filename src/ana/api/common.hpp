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

        typedef ana_uint32  client_id          /** Type of IDs of connected components, unique.     */ ;
        typedef ana_uint32  message_size       /** Message size type.                               */ ;

        typedef std::string port               /** Port type, a std::string (instead of a short.)   */ ;
        typedef std::string address            /** Address type, a string. Either IP of hostname.   */ ;

        typedef bool        send_type          /** Send operation type, true to copy the buffer.    */ ;

        typedef boost::system::error_code error_code /** Standard error code, can evaluate to bool. */ ;

        const send_type ZeroCopy   = false     /** Don't copy the buffer. */ ;
        const send_type CopyBuffer = true      /** Copy the buffer.       */ ;

        const message_size HeaderLength = sizeof(ana_uint32) /** Length of message header. */ ;

        const client_id ServerID = 0  /** The ID of the server application. */ ;

}

#endif