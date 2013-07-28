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

#include "umcd/otl/otl.hpp"
#include "filesystem.hpp"
#include "umcd/actions/basic_umcd_action.hpp"

class request_umc_upload_action 
   : public basic_umcd_action
   , public boost::enable_shared_from_this<request_umc_upload_action>
{
public:
   typedef basic_umcd_action base;
   
   request_umc_upload_action(const config& server_config)
   : server_config_(server_config)
   {}

   const config& get_info(const config& metadata)
   {
      return metadata.child("request_umc_upload").child("umc_configuration").child("info");
   }

   bool umc_exists(const config& metadata)
   {
      return get_info(metadata).has_attribute("id");
   }

   void update_umc()
   {
   }

   void create_umc()
   {
   }
   
   virtual void execute(boost::shared_ptr<umcd_protocol> p)
   {
      protocol_ = p;
      config& metadata = protocol_->get_metadata();
      if(umc_exists(metadata))
      {
         update_umc();
      }
      else
      {
         create_umc();
      }
   }

   virtual boost::shared_ptr<base> clone() const
   {
      return boost::shared_ptr<base>(new request_umc_upload_action(*this));
   }

private:
   const config& server_config_;
   boost::shared_ptr<umcd_protocol> protocol_;
};

#endif // UMCD_REQUEST_UMC_UPLOAD_ACTION_HPP
