/* $Id$ */
/*
   Copyright (C) 2007 - 2009
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/dfool/ai.hpp
 * This ai uses for its decisions only units it has "seen".
 */

#ifndef AI_DFOOL_AI_HPP_INCLUDED
#define AI_DFOOL_AI_HPP_INCLUDED

#include "../../global.hpp"

#include "../interface.hpp"
#include "../contexts.hpp"

#include "../../map_location.hpp"
#include "../../unit_map.hpp"
#include "../../unit.hpp"

#include <vector>
#include <list>
#include <map>
#include <string>

namespace ai {

namespace dfool {
  typedef std::vector<size_t> unit_list;

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
    unit_memory(const config& cfg);
    void add_unit_sighting(const unit& u, const map_location& l, size_t t);
    void remove_unit_sighting(size_t id);
    //void purge(int turn = -1); // Clean outdated entries
    void write(config& temp);
    // Create a map based upon units seen since turn
    void known_map(unit_map& units, size_t turn=0);
  private:
    void write_element(int i, config& temp);
    // Could replace these with a single vector of memory elements
    std::vector<unit> units_;
    unit_list ids_;
    std::vector<size_t> turns_;
    std::vector<map_location> locations_;
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

  /**
   * An ai that keeps track of what it has "seen",
   * does not target units that it has not "seen",
   * and does not make decisions based on unseen units.
   */
  class dfool_ai : public default_ai_context_proxy, public interface {
  public:
	dfool_ai(default_ai_context &context)
		: recursion_counter_(context.get_recursion_count()), unit_memory_(context.current_team().ai_memory())
	{
		init_default_ai_context_proxy(context);
	}
	void play_turn();
	virtual std::string describe_self();
	virtual int get_recursion_count() const{
		return recursion_counter_.get_count();
	}
  private:
	recursion_counter recursion_counter_;
    //    std::map<std::string,target> target_map_;
    unit_list all_units();
    unit_list visible_units();
	void switch_side(side_number /*side*/)
	{}
    unit_list my_units();
    unit_list filter_units(const config& filter,unit_list& ul, unit_map& um);
	bool moveto(const config &o, unit_map::const_iterator m);
    unit_map::iterator unit(size_t unit_id, unit_map& um);

    unit_memory unit_memory_;

  };

} // end of namespace dfool

} // end of namespace ai

#endif
