
/**
 * @file
 * @brief Implementation details for the ana project dealing with client predicates.
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

#ifndef ANA_PREDICATES_HPP
#define ANA_PREDICATES_HPP

#include "common.hpp"

namespace ana
{
    /** @name Predicates over client ids.
    *
    * Declaration of types used in conditional send operations, e.g. send_if.
    * \sa send_if
    */
    //@{
        /** A boolean predicate of client IDs. Used for conditional send operations. */
        struct client_predicate
        {
			virtual ~client_predicate() {}

            /**
            * Decides if a given condition applies to a client.
            *
            * @param client ID of the queried client.
            * @returns true if the condition holds for this client.
            */
            virtual bool selects(net_id) const = 0;
        };
        //@}

    /// @cond false
    /**
     * Intended for private use, should be created with create_predicate.
     *
     * \sa create_predicate
     */
    template<class predicate>
    class _generic_client_predicate : public client_predicate
    {
        public:
            /** Construct via a generic object with overloaded operator(). */
            _generic_client_predicate(const predicate& pred)
                : pred(pred)
            {
            }

            /** Copy constructor. */
            _generic_client_predicate(const _generic_client_predicate<predicate>& other )
                : client_predicate(), pred(other.pred)
            {
            }
        private:
            const predicate pred /** The predicate object. */ ;

            /**
             * Implementation of the selection method using operator() from the
             * predicate object.
             *
             * \sa client_predicate
             */
            virtual bool selects(net_id cid) const
            {
                return pred(cid);
            }
    };
    /// @endcond

    /**
     * Creates a client predicate to be used in send operations.
     *
     * This function can be used to create predicate objects from the standard library's
     * bind1st objects and from boost::bind too.
     *
     * Examples:
     *    - server_->send_if(boost::asio::buffer( str ), this, create_predicate(
     *                       boost::bind( std::not_equal_to<net_id>(), client, _1) ) );
     *
     * @param pred Predicate of the queried client.
     * @returns Predicate for client selection.
     */
    template<class predicate>
    _generic_client_predicate<predicate> create_predicate(const predicate& pred)
    {
        return _generic_client_predicate<predicate>(pred);
    }
}

#endif
