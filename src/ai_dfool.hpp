#ifndef AI_DFOOL_HPP_INCLUDED
#define AI_DFOOL_HPP_INCLUDED

#include "ai_interface.hpp"
#include "map.hpp"
#include <vector>
#include <map>
#include <string>

namespace dfool {
  class assignment{
  public:
    assignment(){};
    assignment(config cfg, unit_map my_units);
    std::string& operator[](std::string unit_id){return unit_assignment[unit_id];};
  private:
    std::map<std::string, std::string>unit_assignment;
  };

  //an ai that keeps track of what it has "seen" and does not target units 
  //that it has not "seen" or recruit based on unseen units.
  class dfool_ai : public ai_interface {
  public:
    dfool_ai(info& i) : ai_interface(i) {}
    void play_turn() {
      info info_=get_info();
      int team_num=info_.team_num;
      const config& parms = current_team().ai_parameters();
      const config::child_list& orders = parms.get_children("order");
      LOG_STREAM(info, ai)<<"dfool side:"<<team_num<<" of "<<current_team().nteams()<<std::endl;
      config ai_mem=current_team().ai_memory();
      //     LOG_STREAM(info, ai)<<"dfool memory test:"<<ai_mem["test"]<<std::endl;
      //     char buf[80];
      //     static int count=0;
      //     count++;
      //     sprintf(buf,"%d,%s",count,ai_mem["test"].c_str());
      //     ai_mem["test"]=buf;
      //     current_team().set_ai_memory(ai_mem);
      
      
      config side_filter;
      char buf[80];
      sprintf(buf,"%d",team_num);
      side_filter["side"]=buf;
      unit_map my_units=filter_units(info_.units,side_filter);
      
      //make sure previously assigned units still exist
      config::child_list order_assignments;
      assignment_list.clear();
      LOG_STREAM(info, ai)<<"\tchecking for existing assignments\n";
      config::child_list assigned_list = ai_mem.get_children("assignment_list");
      for(config::child_list::const_iterator al = assigned_list.begin(); al != assigned_list.end(); ++al) {
	std::vector<size_t> remove_list;
	LOG_STREAM(info, ai)<<"\tchecking for existing assignments\n";
	config::child_list alist = (**al).get_children("assignment");
	for(config::child_list::const_iterator a = alist.begin(); a != alist.end(); ++a) {
	  LOG_STREAM(info, ai)<<"\t\tchecking for existing assignments\n";
	  bool found=false;
	  for(unit_map::const_iterator i = my_units.begin(); i != my_units.end(); ++i) {
	    
	    LOG_STREAM(info, ai)<<"\t\tcomparing "<<i->second.underlying_description() <<" to "<<(**a)["unit_id"]  << " \n";
	    
	    if(i->second.underlying_description()==(**a)["unit_id"]){
	      found=true;
	      break;
	    }
	  } 
	  if(found){
	    LOG_STREAM(info, ai)<<"\tfound an existing assignments\n";
	    order_assignments.push_back(*a);
	  }else{//assigned unit no longer on map
	    //need to remove units no longer on map.
	  }
	}
      }
      
      
      const unit_map& v_units=visible_units(info_.units);
      LOG_STREAM(info, ai)<<"dfool sees:"<<std::endl;
      for(unit_map::const_iterator ui = v_units.begin(); ui != v_units.end(); ++ui) {
	LOG_STREAM(info, ai)<<"\t"<<ui->second.name()<<std::endl;
	LOG_STREAM(info, ai)<<"\t\t"<<ui->second.underlying_description()<<std::endl;
      }
      for(config::child_list::const_iterator o = orders.begin(); o != orders.end(); ++o) {
	std::string id=(**o)["id"];
	std::string number=(**o)["number"];
	std::string priority=(**o)["priority"];
	int num=atoi(number.c_str());
	int prior=atoi(priority.c_str());
	bool pers=(id.size()>0);
	LOG_STREAM(info, ai)<<"dfool order("<<(pers?id:"")<<"): "<<num<<(num==1?" unit":" units")<<" with priority "<<prior<<std::endl;
	const config::child_list& filter = (**o).get_children("filter");
	
	const config::child_list& clear_assign = (**o).get_children("clear_assignment");
	unit_map matching_units;
	unit_map assigned_units;
	int count=num;
	
	assigned_units.clear();
	//find units assigned to this order;
	for(config::child_list::const_iterator at = order_assignments.begin(); at != order_assignments.end(); ++at) {
	  LOG_STREAM(info, ai)<<"\tchecking for assignments\n";
	  LOG_STREAM(info, ai)<<"\t\t"<<(**at)["order_id"]<<" vs "<<id<<"\n";
	  if((**at)["order_id"]==id){
	    LOG_STREAM(info, ai)<<"\t\tchecking for existing assignments\n";
	    for(unit_map::const_iterator i = my_units.begin(); i != my_units.end(); ++i) {
	      if(i->second.underlying_description()==(**at)["unit_id"]){
		//check if assignment should be cleared
		if(clear_assignment(i->first,clear_assign,info_.map)){
		  LOG_STREAM(info, ai)<<"\tclear existing assignments\n";
		}else{
		  assigned_units.insert(*i);
		  LOG_STREAM(info, ai)<<"\t\tAssignment: "<<(**at)["unit_id"]<<" to order: "<<id<<std::endl;
		}
	      }
	    }
	  }
	}
	
	matching_units.clear();
	//find units that match any filter. If no filters then accept all units.
	if(filter.size()){
	  for(config::child_list::const_iterator f = filter.begin(); f != filter.end(); ++f) {
	    LOG_STREAM(info, ai)<<"dfool filter:"<<std::endl;
	    unit_map filtered_units=filter_units(my_units,(**f));
	    for(unit_map::const_iterator i = filtered_units.begin(); i != filtered_units.end(); ++i) {
	      LOG_STREAM(info, ai)<<"\t match: "<<i->second.underlying_description()<<std::endl;
	      //make sure matching unit isn't already assigned
	      bool found=false;
	      for(config::child_list::const_iterator at = assigned_list.begin(); at != assigned_list.end(); ++at) {
		LOG_STREAM(info, ai)<<"\talready assignments?\n";
		if(i->second.underlying_description()==(**at)["unit_id"]){
		  found=true;
		}
	      }
	      if(found){
		LOG_STREAM(info, ai)<<"\t\talready assigned: "<<i->second.underlying_description()<<std::endl;
	      }else{
		matching_units.insert(*i);
		LOG_STREAM(info, ai)<<"\t\tmatching: "<<i->second.underlying_description()<<" to order: "<<id<<std::endl;
	      }
	    } 
	  }
	}else{
	  matching_units=my_units;
	}
	
	//should add sorting functionality here in future
	//bring assigned units up to maximum number
	for(unit_map::const_iterator mu = matching_units.begin(); mu != matching_units.end() && order_assignments.size()<num; ++mu) {
	  assigned_units.insert(*mu);
	  LOG_STREAM(info, ai)<<"\tassigned unit:\t"<<mu->second.underlying_description()<<"\n";
	}
	
	if(order_assignments.size()){
	  LOG_STREAM(info, ai)<<"\t "<<assigned_units.size()<<(assigned_units.size()==1?" unit assigned":" units assigned")<<std::endl;
	  
	  const config::child_list& commands = (**o).get_children("command");
	  for(config::child_list::const_iterator com = commands.begin(); com != commands.end(); ++com) {
	    std::string type=(**com)["type"];
	    LOG_STREAM(info, ai)<<"\tcommand: "<<type<<std::endl;
	    if(type=="moveto"){
	      moveto(com,assigned_units);
	    }
	  }
	}
	
	//save assignments into memory
	if(id.size()){
	  for(unit_map::const_iterator au = assigned_units.begin(); au != assigned_units.end(); ++au) {
	    config temp;
	    temp["order_id"]=id;
	    temp["unit_id"]=au->second.underlying_description();
	    assignment_list.add_child("assignment",temp);
	  }
	}
	//save ai assignment list to ai memory
	ai_mem.clear_children("assignment_list");
	ai_mem.add_child("assignment_list",assignment_list);
	current_team().set_ai_memory(ai_mem);
      }
      return;
    }
  private:
    unit_map visible_units_;
    unit_map my_units_;
    config assignment_list;
    
    bool clear_assignment(const location& loc, const config::child_list& clear,const gamemap& map)
    {
      for(config::child_list::const_iterator cl=clear.begin();cl<clear.end();cl++)
	{
	  const config::child_list& clear_loc_filter = (**cl).get_children("terrain_filter");
	  if(clear_loc_filter.size()){
	    for(config::child_list::const_iterator clf=clear_loc_filter.begin();clf<clear_loc_filter.end();clf++)
	      if(map.filter_location(loc,(**clf))){
		return(true);
	      }
	  }
	}
      return(false);
    }
    
    const unit_map& visible_units(const unit_map& units_)
    {
      visible_units_.clear();
      if(current_team().uses_shroud() == false && current_team().uses_fog() == false) {
	LOG_STREAM(info, ai) << "all units are visible...\n";
	visible_units_=units_;
      }else{
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
	  if(current_team().fogged(i->first.x,i->first.y) == false) {
	    visible_units_.insert(*i);
	  }
	}
      }
      
      LOG_STREAM(info, ai) << "number of visible units: " << visible_units_.size() << "\n";
      //still need to deal with invisible units not in fog.
      return visible_units_;
    }
    
    unit_map filter_units(const unit_map& units_,const config& filter)
    {
      unit_map filtered_units_;
      for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
	if(i->second.matches_filter(filter)) {
	  filtered_units_.insert(*i);
	} 
      }   
      return filtered_units_;
    }
    
    void moveto(config::child_list::const_iterator o, unit_map matching_units){
      location target(atoi((**o)["target_x"].c_str())-1,atoi((**o)["target_y"].c_str())-1);
      LOG_STREAM(info, ai)<<"\tmoving to:("<<target.x<<","<<target.y<<")"<<std::endl;
      for(unit_map::const_iterator m = matching_units.begin(); m!=matching_units.end();++m){
	if(m->second.movement_left()){
	  std::map<location,paths> possible_moves;
	  move_map srcdst, dstsrc;
	  calculate_possible_moves(possible_moves,srcdst,dstsrc,false);
	  
	  int closest_distance = -1;
	  std::pair<location,location> closest_move;
	  
	  //this undoubtedly could be done more cleanly
	  for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
	    //must restrict move_map to only unit that is moving.
	    if(i->second==m->first){
	      const int distance = distance_between(target,i->first);
	      if(closest_distance == -1 || distance < closest_distance) {
		closest_distance = distance;
		closest_move = *i;
	      }
	      //	    LOG_STREAM(info, ai)<<"\tmoving: "<<distance<<" from ("<<i->first.x<<","<<i->first.y<<")"<<" to ("<<target.x<<","<<target.y<<")"<<std::endl;
	    }
	  }
	  if(closest_distance != -1) {
	    move_unit(closest_move.second,closest_move.first,possible_moves);
	  }
	}
      }
    }
  };

}//end namespace dfool
#endif
