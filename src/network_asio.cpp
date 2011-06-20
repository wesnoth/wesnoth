#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include "network_asio.hpp"
#include "serialization/parser.hpp"

namespace network_asio {

connection::connection(const std::string& host, const std::string& service) :
	resolver_(io_service_), socket_(io_service_), done_(false)
{
	resolver_.async_resolve(
		boost::asio::ip::tcp::resolver::query(host, service),
		boost::bind(&connection::handle_resolve, this, _1, _2)
		);
}

void connection::handle_resolve(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		)
{
	if(ec)
		throw error(ec);

	std::cout << iterator->endpoint().address() << '\n';
	connect(iterator);
}

void connection::connect(resolver::iterator iterator)
{
	socket_.async_connect(*iterator, boost::bind(
		&connection::handle_connect, this, _1, iterator)
		);
}

void connection::handle_connect(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		)
{
	if(ec) {
		socket_.close();
		if(++iterator == resolver::iterator())
			throw error(ec);
		else
			connect(iterator);
	} else {
		std::cout << "Connected!\n";
		handshake();
	}
}

void connection::handshake()
{
	static const boost::uint32_t handshake = 0;
	boost::asio::async_write(socket_,
		boost::asio::buffer(reinterpret_cast<const char*>(&handshake), 4),
		boost::bind(&connection::handle_write, this, _1, _2)
		);
	boost::asio::async_read(socket_,
		boost::asio::buffer(&handshake_response_.binary, 4),
		boost::bind(&connection::handle_handshake, this, _1)
		);
}

void connection::handle_handshake(
		const boost::system::error_code& ec
		)
{
	if(ec)
		throw error(ec);
	done_ = true;
}

void connection::transfer(const config& request, config& /*response*/)
{
	io_service_.reset();
	done_ = false;

	std::ostream os(&write_buf_);
	write_gz(os, request);
	std::size_t size = write_buf_.size();
	size = htonl(size);
	boost::asio::write(socket_, boost::asio::buffer(reinterpret_cast<const char*>(&size), 4));
	boost::asio::async_write(socket_, write_buf_, boost::bind(&connection::handle_write, this, _1, _2));
	boost::asio::async_read(socket_, read_buf_,
		boost::bind(&connection::is_read_complete, this, _1, _2),
		boost::bind(&connection::handle_read, this, _1, _2));
}

void connection::handle_write(
	const boost::system::error_code& ec,
	std::size_t bytes_transferred
	)
{
	std::cout << "Written " << bytes_transferred << " bytes.\n";
	if(ec)
		throw error(ec);
}

std::size_t connection::is_read_complete(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred
		)
{
	if(ec)
		throw error(ec);
	std::cout << "Read: " << bytes_transferred << '\n';
	if(bytes_transferred < 4) {
		return 4;
	} else {
		if(!bytes_to_read_) {
			std::istream is(&read_buf_);
			union { char binary[4]; boost::uint32_t num; } data_size;
			is.read(data_size.binary, 4);
			bytes_to_read_ = ntohl(data_size.num) + 4;
		}
		return bytes_to_read_.get() - bytes_transferred;
	}
}

void connection::handle_read(
	const boost::system::error_code& ec,
	std::size_t bytes_transferred
	)
{
	std::cout << "Read " << bytes_transferred << " bytes.\n";
	bytes_to_read_.reset();
	done_ = true;
	if(ec && ec != boost::asio::error::eof)
		throw error(ec);
	std::istream is(&read_buf_);
	config cfg;
	read_gz(cfg, is);
	std::cout << cfg;
}

}
