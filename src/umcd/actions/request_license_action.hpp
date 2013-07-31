/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/* Design rational:
The license is not shipped with the Wesnoth client because this server can be re-use with different licenses on other server than Wesnoth ones.
*/

#ifndef UMCD_REQUEST_LICENSE_ACTION_HPP
#define UMCD_REQUEST_LICENSE_ACTION_HPP

#include <boost/shared_ptr.hpp>
#include "config.hpp"
#include "umcd/actions/basic_umcd_action.hpp"

class request_license_action : public basic_umcd_action
{
public:
	typedef basic_umcd_action base;

	request_license_action(const config& server_config);
	virtual void execute(boost::shared_ptr<umcd_protocol> protocol);
	virtual boost::shared_ptr<base> clone() const;

private:
	const config& server_config_;
};

#endif // UMCD_REQUEST_LICENSE_ACTION_HPP
