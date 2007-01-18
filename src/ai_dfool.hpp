#ifndef AI_DFOOL_HPP_INCLUDED
#define AI_DFOOL_HPP_INCLUDED

#include "global.hpp"

#include "ai_interface.hpp"
#include "map.hpp"
#include "unit_map.hpp"
#include "unit.hpp"
#include <vector>
#include <map>
#include <string>

namespace dfool {
  typedef std::vector<std::string> unit_list;
  //an ai that keeps track of what it has "seen", does not target units
  //that it has not "seen" and does not recruit based on unseen units.
  class dfool_ai : public ai_interface {
  public:
    dfool_ai(info& i) : ai_interface(i) {}
    void play_turn();
  private:
    unit_list all_units();
    unit_list visible_units();
    unit_list my_units();
    unit_list filter_units(const config& filter,unit_list& ul, unit_map& um);
    bool moveto(config::child_list::const_iterator o, unit_map::const_iterator m);
    unit_map::iterator unit(std::string unit_id, unit_map& um);
  };

}//end namespace dfool
#endif
