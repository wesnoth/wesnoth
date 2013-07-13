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

#ifndef UMCD_REQUEST_INFO_HPP
#define UMCD_REQUEST_INFO_HPP

#include <boost/make_shared.hpp>

#include "umcd/actions/basic_umcd_action.hpp"
#include "serialization/one_hierarchy_validator.hpp"

class request_info
{
public:
   typedef schema_validation::one_hierarchy_validator validator_type;
   typedef boost::shared_ptr<basic_umcd_action> action_ptr;
   typedef boost::shared_ptr<validator_type> validator_ptr;
private:
   action_ptr umcd_action;
   validator_ptr request_validator;

public:
   request_info(const action_ptr& action, const validator_ptr& validator);
   action_ptr action();
   validator_ptr validator();
   boost::shared_ptr<request_info> clone() const;
};

request_info::request_info(const action_ptr& action, const validator_ptr& validator)
: umcd_action(action)
, request_validator(validator)
{}

typename request_info::action_ptr request_info::action()
{
   return umcd_action;
}

typename request_info::validator_ptr request_info::validator()
{
   return request_validator;
}

boost::shared_ptr<request_info> request_info::clone() const
{
   return boost::make_shared<request_info>(
      umcd_action->clone(), 
      boost::make_shared<validator_type>(*request_validator)
   );
}

template <class Action, class Validator>
boost::shared_ptr<request_info> make_request_info(const config& server_conf, const std::string& request_name)
{
   return boost::make_shared<request_info>(
      boost::make_shared<Action>(),
      boost::make_shared<Validator>(
         server_conf["wesnoth_dir"].str() + "data/umcd/protocol_schema/"+request_name+".cfg"));
}

#endif // UMCD_REQUEST_INFO_HPP