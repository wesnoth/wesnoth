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

#ifndef UMCD_REQUEST_LICENSE_ACTION_HPP
#define UMCD_REQUEST_LICENSE_ACTION_HPP

#include <boost/shared_ptr.hpp>
#include "umcd/actions/basic_wml_action.hpp"

class request_license_action : public basic_wml_action
{
public:
   virtual wml_reply execute(wml_request& request)
   {
      return wml_reply(request.get_data());
   }

   virtual boost::shared_ptr<basic_wml_action> clone() const
   {
      return boost::shared_ptr<basic_wml_action>(new request_license_action(*this));
   }
};

#endif // UMCD_REQUEST_LICENSE_ACTION_HPP
