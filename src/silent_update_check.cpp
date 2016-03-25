#include "silent_update_check.hpp"

#include "config_assign.hpp"
#include "game_config.hpp"
#include "version.hpp"

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

silent_update_check::silent_update_check(void)
	: io_service_()
	, resolver_(io_service_)
	, socket_(io_service_)
	, request_()
	, response_()
{
    std::ostream request_stream(&request_);
    request_stream << "GET " << "/Download" << " HTTP/1.0\r\n";
    request_stream << "Host: " << "wiki.wesnoth.org" << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

	boost::asio::ip::tcp::resolver::query query("wiki.wesnoth.org", "http");
    resolver_.async_resolve(query,
        boost::bind(&silent_update_check::handle_resolve, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::iterator));
}


void silent_update_check::handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// Attempt a connection to each endpoint in the list until we
		// successfully establish a connection.
		boost::asio::async_connect(socket_, endpoint_iterator,
			boost::bind(&silent_update_check::handle_connect, this,
			boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}
void silent_update_check::handle_connect(const boost::system::error_code& err)
{
	if (!err)
	{
		// The connection was successful. Send the request.
		boost::asio::async_write(socket_, request_,
			boost::bind(&silent_update_check::handle_write_request, this,
			boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}


void silent_update_check::handle_write_request(const boost::system::error_code& err)
{
	if (!err)
	{
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.
		boost::asio::async_read_until(socket_, response_, "\r\n",
			boost::bind(&silent_update_check::handle_read_status_line, this,
			boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}


void silent_update_check::handle_read_status_line(const boost::system::error_code& err)
{
	if (!err)
	{
		// Check that response is OK.
		std::istream response_stream(&response_);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::cout << "Invalid response\n";
			return;
		}
		if (status_code != 200)
		{
			std::cout << "Response returned with status code ";
			std::cout << status_code << "\n";
			return;
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
			boost::bind(&silent_update_check::handle_read_headers, this,
			boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err << "\n";
	}
}

void silent_update_check::handle_read_headers(const boost::system::error_code& err)
{
	if (!err)
	{
		// Process the response headers.
		std::istream response_stream(&response_);
		std::string header;
		while (std::getline(response_stream, header) && header != "\r") { }
		// Start reading remaining data until EOF.
		boost::asio::async_read(socket_, response_,
			boost::asio::transfer_at_least(1),
			boost::bind(&silent_update_check::handle_read_content, this,
			boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err << "\n";
	}
}

void silent_update_check::handle_read_content(const boost::system::error_code& err)
{
	if (!err)
	{
		// Continue reading remaining data until EOF.
		boost::asio::async_read(socket_, response_,
			boost::asio::transfer_at_least(1),
			boost::bind(&silent_update_check::handle_read_content, this,
			boost::asio::placeholders::error));
	}
	else if (err != boost::asio::error::eof)
	{
		std::cout << "Error: " << err << "\n";
	}
	else {
		boost::regex expr("Stable \\((\\d+)\\.(\\d+) branch\\)");
		boost::smatch what;
		std::string s((std::istreambuf_iterator<char>(&response_)), std::istreambuf_iterator<char>());

		if (boost::regex_search(s, what, expr))
		{
			if(game_config::wesnoth_version.major_version() != boost::lexical_cast<int>(what[1])
			|| game_config::wesnoth_version.minor_version() != boost::lexical_cast<int>(what[2]))
			{
				std::cerr << "You are using version " << game_config::wesnoth_version.str() << " which is not the current stable version (" << what[1] << "." << what[2] << ").\n";
			}
		}
	}
}



silent_update_check::~silent_update_check(void)
{

}


void silent_update_check::process(events::pump_info&)
{
	io_service_.poll();
}
