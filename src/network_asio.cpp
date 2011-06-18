#include "network_asio.hpp"
#include <boost/bind.hpp>
#include <iostream>

namespace network_asio {

connection::connection(const std::string& host, const std::string& service) :
	resolver_(io_service_), socket_(io_service_), connected_(false)
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
		connected_ = true;
	}
}

}
