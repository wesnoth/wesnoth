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

#include <stdexcept>
#include "config.hpp"

#include "campaign_server/server_options.hpp"
#include "campaign_server/server/single_threaded/server.hpp"
#include "campaign_server/protocol/wml/wml_protocol.hpp"

int main(int argc, char *argv[])
{
  try
  {
    server_options options(argc, argv);
    if(!options.print_info())
    {
      config cfg = options.build_config();
      std::cout << "Configuration requested:\n" << cfg;

      wml_protocol protocol;
      server<wml_protocol> addon_server(cfg, protocol);
      addon_server.run();
    }
  }
  catch(std::exception &e)
  {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
