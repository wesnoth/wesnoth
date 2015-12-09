#pragma once

#include "global.hpp"
#include "events.hpp"

#include <boost/asio.hpp>

class silent_update_check : private events::pump_monitor
{
public:
	silent_update_check(void);
	~silent_update_check(void);
private:
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::resolver resolver_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;
	std::string res_;

	virtual void process(events::pump_info& info) OVERRIDE;
	void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_connect(const boost::system::error_code& err);
	void handle_write_request(const boost::system::error_code& err);
	void handle_read_status_line(const boost::system::error_code& err);
	void handle_read_headers(const boost::system::error_code& err);
	void handle_read_content(const boost::system::error_code& err);
	void do_update();
};

