/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef NETWORK_NEW_HPP_INCLUDED
#define NETWORK_NEW_HPP_INCLUDED

#include "filesystem.hpp"

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/enable_shared_from_this.hpp>

class config;

namespace network_boost {

	class buffer;
	typedef boost::shared_ptr<buffer> buffer_ptr;
	class manager;

	typedef size_t connection_id;
	typedef std::string ip_address;

	/**
	 * Holds per connection data
	 * and implements connection handling
	 **/
	class connection : public boost::noncopyable {
		public:
			class impl;
			typedef boost::shared_ptr<impl> pimpl;
		private:
			pimpl impl_;
		public:
			/**
			 * Used in networking so numbers are freezed
			 * Only modes that are
			 **/
			enum mode {
				BINARY = 1,
				GZIP = 2,
				ZLIB = 3,
				PLAIN_TEXT = 4
			};
			/**
			 * Makes it possible to use custom connection
			 * implementations to make special connections
			 * like server sockets.
			 **/
			connection(pimpl implementation);

			/**
			 * connects to server and tries to use prefered mode
			 * for compression
			 **/
			void connect(const std::string& host, const size_t port, mode prefered_mode);
			/**
			 * queues buffer for sending
			 **/
			void send_buffer(buffer_ptr buffer);
			/**
			 * Flags this connection for disconnection
			 */
			void disconnect(bool force = false);
			/**
			 * Periodic checks on connection
			 * It is used to send pings to clients
			 * @return true if next connection hould be checked too
			 **/
			bool check_connection();

			/**
			 * @return remote hosts ip address
			 **/
			const ip_address& get_ip_address() const;

			connection_id get_id() const;
			void set_id(connection_id);

			class statistics {
				size_t total_send;
				size_t total_recv;
				size_t send;
				size_t recv;
				size_t send_limit;
				size_t recv_limit;
				public:
				size_t get_total_send() const;
				size_t get_total_recv() const;
				size_t get_send() const;
				size_t get_recv() const;
				size_t get_send_limit() const;
				size_t get_recv_limit() const;
				protected:
				void send_start(size_t len);
				void recv_start(size_t len);
				void send_transfered(size_t bytes);
				void recv_transfered(size_t bytes);
				friend class connection::impl;
			};
			const statistics get_statistics() const;
	};

	typedef boost::shared_ptr<connection> connection_ptr;

	class buffer {
		public:
			void set_mode(connection::mode);
			virtual ~buffer()
			{}
			/**
			 * Do actual network operations
			 **/
			virtual void process_network(connection&) = 0;
			/**
			 * Converts data to config
			 **/
			virtual void get_config(config&) = 0;
			/**
			 * Get raw network data
			 **/
			virtual void get_raw_buffer(std::vector<char>&) = 0;
	};

	class file_buffer : public buffer {
		scoped_istream filestream_;
		public:
			file_buffer(const std::string filename);
			virtual ~file_buffer();
			virtual void process_network(connection&);
			/**
			 * Converts data to config
			 **/
			virtual void get_config(config&);
			/**
			 * Get raw network data
			 **/
			virtual void get_raw_buffer(std::vector<char>&);
	};

	class config_buffer : public buffer {
		config* cfg_;
		public:
		config_buffer(const config&);
		virtual ~config_buffer();
		virtual void process_network(connection&);
		virtual void get_config(config&);
		virtual void get_raw_buffer(std::vector<char>&);
	};

	/**
	 * is public interface to network operations
	 **/
	class manager : public boost::noncopyable {
		class impl;
		impl* pimpl;
		static manager* manager_;
		public:
		manager();
		~manager();

		static manager* get_manager()
		{
			assert(manager_);
			return manager_;
		}

		/**
		 * Creates a new connection
		 **/
		connection_ptr connect(const std::string& host, const size_t port, connection::mode prefered_mode);

		/**
		 * Runs networking code that has to happen in
		 * main thread;
		 * for example sending ping to passive connection
		 **/
		void handle_network(size_t timeslice = 0);

		/**
		 * creates a server socket that aceppts connection
		 **/
		connection_ptr listen(const size_t port);

		/**
		 * Queries allready established connection
		 * @return true if there was new connection pending
		 **/
		bool accept(connection_ptr);

		/**
		 * returns queued network data
		 **/
		bool receive_data(connection_ptr, buffer_ptr);

		connection_ptr get_connection(connection_id) const;

		size_t nconnections() const;

		void disconnect_all();
	};

}
#endif
