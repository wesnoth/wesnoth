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

#include "global.hpp"
#include "config.hpp"
#include "network_boost.hpp"
#include "time.hpp"
#include "filesystem.hpp"
#include "scoped_resource.hpp"

#include <vector>
#include <deque>
#include <string>
#include <sys/types.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#define DBG_NW LOG_STREAM(debug, network)
#define LOG_NW LOG_STREAM(info, network)
#define WRN_NW LOG_STREAM(warn, network)
#define ERR_NW LOG_STREAM(err, network)

namespace  network_boost {


	typedef boost::shared_ptr<buffer> buffer_ptr;
	typedef std::deque<buffer_ptr> buffer_queue;


	class connection::impl : public boost::noncopyable {
		connection_id id_;
		protected:
		manager* manager_;
		statistics stats_;

		public:
		impl(manager* manager) : manager_(manager) {}
		virtual ~impl()
		{
		}

		virtual void connect(const std::string&, const size_t, mode) = 0;
		virtual const connection::statistics get_statistics() const
		{
			return stats_;
		}

		virtual void send_buffer(buffer_ptr) = 0;
		virtual void disconnect(bool) = 0;
		virtual bool check_connection() = 0;
		virtual const std::string& get_ip_address() const = 0;

		connection_id get_id() const
		{
			return id_;
		}

		void set_id(connection_id id)
		{
			id_ = id;
		}

	};

	class old_data_connection : public connection::impl {
		ip_address address_;
		size_t port_;
		enum connection_state {
			RESOLVE_HOST,
			CONNECT,
			CONNECTED,
			OPEN,
			DISCONNECT,
			LOST
		};
		connection_state state_;
		time_t last_activity_;
		buffer_queue send_queue;
		public:
		old_data_connection(manager* manager) : connection::impl(manager)
		{
		}

		virtual ~old_data_connection()
		{
		}

		virtual void connect(const std::string&, const size_t, connection::mode)
		{
		}

		virtual void send_buffer(buffer_ptr)
		{
		}
		virtual void disconnect(bool)
		{
		}
		virtual bool check_connection()
		{
			return false;
		}
		virtual const ip_address& get_ip_address() const
		{
			return address_;
		}

	};

	class data_connection : public  old_data_connection {
	};


	// It could be possible to implement administrative
	// connection to server that use special client to
	// monitore and command server thought tcp connection
//	class admin_data_connection : public data_connection {
//	};

	class listen_connection : public connection::impl {
	};

	connection::connection(pimpl implementation) : impl_(implementation)
	{
	}

	void connection::connect(const std::string& host, const size_t port, mode prefered_mode)
	{
		assert(impl_);
		impl_->connect(host, port, prefered_mode);
	}

	void connection::send_buffer(buffer_ptr buffer)
	{
		assert(impl_);
		impl_->send_buffer(buffer);
	}

	void connection::disconnect(bool force)
	{
		assert(impl_);
		impl_->disconnect(force);
	}

	bool connection::check_connection()
	{
		assert(impl_);
		return impl_->check_connection();
	}

	const ip_address& connection::get_ip_address() const
	{
		assert(impl_);
		return impl_->get_ip_address();
	}

	connection_id connection::get_id() const
	{
		assert(impl_);
		return impl_->get_id();
	}

	void connection::set_id(connection_id id)
	{
		assert(impl_);
		impl_->set_id(id);
	}

	const connection::statistics connection::get_statistics() const
	{
		assert(impl_);
		return impl_->get_statistics();
	}

	size_t connection::statistics::get_total_send() const
	{
		return total_send;
	}

	size_t connection::statistics::get_total_recv() const
	{
		return total_recv;
	}

	size_t connection::statistics::get_send() const
	{
		return send;
	}

	size_t connection::statistics::get_recv() const
	{
		return recv;
	}

	size_t connection::statistics::get_send_limit() const
	{
		return send_limit;
	}

	size_t connection::statistics::get_recv_limit() const
	{
		return recv_limit;
	}

	void connection::statistics::send_start(size_t len)
	{
		send = 0;
		send_limit = len;
	}

	void connection::statistics::recv_start(size_t len)
	{
		total_recv += recv;
		recv = 0;
		recv_limit = len;
	}

	void connection::statistics::send_transfered(size_t bytes)
	{
		total_send += bytes;
		send += bytes;
	}

	void connection::statistics::recv_transfered(size_t bytes)
	{
		total_recv += bytes;
		recv += bytes;
	}

	file_buffer::file_buffer(const std::string filename) :
		filestream_(istream_file(filename))
	{
	}

	file_buffer::~file_buffer() {
	}

	void file_buffer::process_network(connection&) {
	}

	void file_buffer::get_config(config& cfg) {
	}

	void file_buffer::get_raw_buffer(std::vector<char>& buf) {
	}

	config_buffer::config_buffer(const config& cfg) : cfg_(new config(cfg))
	{
	}

	config_buffer::~config_buffer() {
		delete cfg_;
	}

	void config_buffer::process_network(connection&) {
	}

	void config_buffer::get_config(config& cfg) {
	}

	void config_buffer::get_raw_buffer(std::vector<char>& buf) {
	}


	class connection_array :
		public std::vector<connection_ptr>
	{
		typedef std::vector<size_t> connection_ids;
		connection_ids free_ids;
		public:
		size_t insert(const connection_ptr& val)
		{
			if (!free_ids.empty())
			{
				size_t id = free_ids.back();
				free_ids.pop_back();
				operator[](id) = val;
				return id;
			}
			else
				push_back(val);
			return size() - 1;
		}

		void erase(size_t index)
		{
			if (index == size() - 1)
				pop_back();
			else
				free_ids.push_back(index);
		}

		size_t active_size() const
		{
			return size() - free_ids.size();
		}
	};


	class manager::impl : public boost::noncopyable {
		connection_array connections_;
		connection_array new_connections_;
		buffer_queue recv_queue_;
		manager* manager_;

		connection_ptr create_connection()
		{
			connection::pimpl new_conn_details(new old_data_connection(manager_));
			connection_ptr new_conn(new connection(new_conn_details));

			return new_conn;
		}

		public:
		impl(manager* manager) : manager_(manager)
		{
		}

		connection_ptr connect(const std::string& host, const size_t port, connection::mode prefered_mode)
		{
			connection_ptr new_conn(create_connection());

			new_conn->set_id(connections_.insert(new_conn));

			new_conn->connect(host, port, prefered_mode);

			return new_conn;;
		}

		void handle_network(size_t)
		{

		}

		bool accept(connection_ptr conn)
		{
			if (new_connections_.empty())
				return false;

			conn = new_connections_.back();
			conn->set_id(connections_.insert(conn));
			new_connections_.pop_back();
			return true;
		}

		connection_ptr listen(size_t port)
		{
			return connection_ptr();
		}

		bool receive_data(connection_ptr con, buffer_ptr buffer)
		{
			return false;
		}
		connection_ptr get_connection(connection_id id) const
		{
			return connections_[id];
		}
		struct call_disconnect {
			void operator()(connection_ptr& con)
			{
				con->disconnect();
			}
		};

		void disconnect_all()
		{
			std::for_each(connections_.begin(), connections_.end(), call_disconnect());
		}
		size_t nconnections() const
		{
			return connections_.active_size();
		}
	};

	manager* manager::manager_ = 0;

	manager::manager() : pimpl(new manager::impl(this))
	{
		manager_ = this;
	}

	manager::~manager()
	{
		manager_ = 0;
		delete pimpl;
	}

	connection_ptr manager::connect(const std::string& host, const size_t port, connection::mode prefered_mode)
	{
		return pimpl->connect(host, port, prefered_mode);
	}

	void manager::handle_network(size_t timeslice)
	{
		pimpl->handle_network(timeslice);
	}

	connection_ptr manager::listen(const size_t port)
	{
		return pimpl->listen(port);
	}

	bool manager::accept(connection_ptr con)
	{
		return pimpl->accept(con);
	}

	bool manager::receive_data(connection_ptr con, buffer_ptr buffer)
	{
		return pimpl->receive_data(con,buffer);
	}
	connection_ptr manager::get_connection(connection_id id) const
	{
		return pimpl->get_connection(id );
	}

	size_t manager::nconnections() const
	{
		return pimpl->nconnections();
	}
	void manager::disconnect_all()
	{
		pimpl->disconnect_all();
	}
}
