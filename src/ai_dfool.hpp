/* $Id$ */
/*
   Copyright (C) 2007 - 2008
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file ai_dfool.hpp
//! The Standard-AI ??.
//! This ai uses for its decisions only units it has "seen".

#ifndef AI_DFOOL_HPP_INCLUDED
#define AI_DFOOL_HPP_INCLUDED

#include "global.hpp"

#include "ai_interface.hpp"
#include "map.hpp"
#include "unit_map.hpp"
#include "unit.hpp"

#include <vector>
#include <list>
#include <map>
#include <string>

namespace dfool {
  typedef std::vector<std::string> unit_list;

//  class target {
//  public:
//    target(config& command, unit_history u_hist, info& ai_info);
//    double value(location loc, unit& u, unit_history u_hist, info ai_info);
//   private:
//     config unit_filter_;
//     config terrain_filter_;
//     std::string hex_val_;
//     std::string number;
//     std::string id;
//   };

  class unit_memory{
  public:
    unit_memory(const game_data& gamedata, const config& cfg);
    void add_unit_sighting(unit u, gamemap::location l, size_t t);
    void remove_unit_sighting(std::string id);
    //void purge(int turn = -1); // Clean outdated entries
    void write(config& temp);
    // Create a map based upon units seen since turn
    void known_map(unit_map& units, size_t turn=0);
  private:
    void write_element(int i, config& temp);
    // Could replace these with a single vector of memory elements
    std::vector<unit> units_;
    std::vector<std::string> ids_;
    std::vector<size_t> turns_;
    std::vector<gamemap::location> locations_;
  };

  class evaluator{
  public:
    evaluator(const game_state& s, std::map<std::string, evaluator*>* m):function_map_(m),state(s){};
    virtual ~evaluator(){};
    virtual std::string value(const std::string& s);
  private:
    std::map<std::string, evaluator*>* function_map_;
    const game_state& state;
  };

  class arithmetic_evaluator : public evaluator {
  public:
    arithmetic_evaluator(const game_state& s, std::map<std::string, evaluator*>* m):evaluator(s,m){};
    std::string value(const std::string& s);
  private:
    std::list<std::string> parse_tokens(const std::string&);
    std::string evaluate_tokens(std::list<std::string>&);
  };

  class distance_evaluator : public arithmetic_evaluator {
  public:
    distance_evaluator(const game_state& s, std::map<std::string, evaluator*>* m):arithmetic_evaluator(s,m){};
    std::string value(const std::string& s);
  private:
    std::list<std::string> parse_tokens(const std::string&);
    std::string evaluate_tokens(std::list<std::string>&);
  };

  //! An ai that keeps track of what it has "seen",
  //! does not target units that it has not "seen",
  //! and does not make decisions based on unseen units.
  class dfool_ai : public ai_interface {
  public:
    dfool_ai(info& i) : ai_interface(i),unit_memory_(i.gameinfo , i.teams[i.team_num-1].ai_memory()){}
    void play_turn();
  private:
    //    std::map<std::string,target> target_map_;
    unit_list all_units();
    unit_list visible_units();
    unit_list my_units();
    unit_list filter_units(const config& filter,unit_list& ul, unit_map& um);
    bool moveto(config::child_list::const_iterator o, unit_map::const_iterator m);
    unit_map::iterator unit(std::string unit_id, unit_map& um);

    unit_memory unit_memory_;

  };

} // end namespace dfool

#endif
