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

#ifndef UMCD_REQUEST_UMC_UPLOAD_ACTION_HPP
#define UMCD_REQUEST_UMC_UPLOAD_ACTION_HPP

#include <boost/shared_ptr.hpp>
#include "filesystem.hpp"
#include "umcd/actions/basic_wml_action.hpp"

template <class NetworkStream>
class request_umc_upload_action : public basic_wml_action<NetworkStream>::type
{
public:
   typedef typename basic_wml_action<NetworkStream>::type base;

   virtual wml_reply execute(wml_request<NetworkStream>&, const config& server_config)
   {
      config reply("request_license");
      reply.child("request_license")["text"] = read_file(server_config["wesnoth_dir"].str() + "COPYING");
      return make_reply(reply);
   }

   virtual boost::shared_ptr<base> clone() const
   {
      return boost::shared_ptr<base>(new request_umc_upload_action(*this));
   }
};

#endif // UMCD_REQUEST_UMC_UPLOAD_ACTION_HPP
