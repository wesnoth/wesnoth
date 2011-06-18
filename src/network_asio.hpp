#ifndef NETWORK_ASIO_HPP_INCLUDED
#define NETWORK_ASIO_HPP_INCLUDED

#include <boost/asio.hpp>
#include "exceptions.hpp"

namespace network_asio {

struct error : public game::error
{
	error(const boost::system::error_code& error) : game::error(error.message()) {}
};

/** A class that represents a TCP/IP connection. */
class connection
{
	boost::asio::io_service io_service_;
	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	typedef boost::asio::ip::tcp::socket socket;
	socket socket_;

	bool connected_;

	void handle_resolve(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		);
	
	void connect(resolver::iterator iterator);
	void handle_connect(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		);


	public:
	/**
	 * Constructor.
	 *
	 * @param host    Name of the host to connect to
	 * @param service Service identifier such as "80" or "http"
	 */
	connection(const std::string& host, const std::string& service);

	/** Handle all pending asynchonous events and return */
	std::size_t poll()
	{
		try {
			return io_service_.poll();
		} catch(const boost::system::system_error& err) {
			throw error(err.code());
		}
	}
	/** Run asio's event loop
	 *
	 * Handle asynchronous events blocking until all asynchronous
	 * operations have finished
	 */
	void run() { io_service_.run(); }

	bool connected() const { return connected_; }
};

}

#endif
