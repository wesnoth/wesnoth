#include "ai_dfool.hpp"

namespace dfool {
  assignment::assignment(config cfg, unit_map my_units)
  {
    config::child_list alist = cfg.get_children("assignment");
    for(config::child_list::const_iterator a = alist.begin(); a != alist.end(); ++a) {
      LOG_STREAM(info, ai)<<"\t\tchecking for existing assignments\n";
      bool found=false;
      std::string unit_id=(**a)["unit_id"];
      for(unit_map::const_iterator i = my_units.begin(); i != my_units.end(); ++i) {

	LOG_STREAM(info, ai)<<"\t\tcomparing "<<i->second.underlying_description() <<" to "<<(**a)["unit_id"]  << " \n";

	if(i->second.underlying_description()==unit_id){
	  found=true;
	  break;
	}
      }
      if(found){
	LOG_STREAM(info, ai)<<"\tfound an existing assignments\n";
	unit_assignment[unit_id]=(**a)["order"];
      }else{//assigned unit no longer on map
	//need to remove units no longer on map.
      }
    }
  };

}//end namespace dfool
