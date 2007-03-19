#include "global.hpp"
#include "ai_dfool.hpp"

namespace dfool {
  void dfool_ai::play_turn(){
    info info_ = get_info();
    int team_num=get_info().team_num;
    const config& parms = current_team().ai_parameters();
    config ai_mem = current_team().ai_memory();
    
    const config::child_list& orders = parms.get_children("order");
    LOG_STREAM(info, ai)<<"dfool side:"<<team_num<<" of "<<current_team().nteams()<<std::endl;

    config side_filter;
    char buf[80];
    sprintf(buf,"%d",team_num);
    side_filter["side"]=buf;

    LOG_STREAM(info, ai)<<"dfool sees:"<<std::endl;
    
    //    for(unit_map::iterator ua = get_info().units.begin(); ua != get_info().units.end(); ++ua) {
      //      std::string t = ua->second.get_ai_special();
      //      LOG_STREAM(info, ai)<<"ua:"<<ua->second.underlying_description()<<"\t"<<t<<std::endl;
      //      LOG_STREAM(info, ai)<<"\t\t\t"<<ua->first.x<<","<<ua->first.y<<std::endl;
    //    }

    unit_list all = all_units();
    unit_list my_units=filter_units(side_filter, all,get_info().units);
    unit_list v_units=visible_units();
    
    //    LOG_STREAM(info, ai)<<"My Units"<<std::endl;
    //    for(unit_list::iterator ui = all.begin(); ui != all.end(); ++ui) {
    //      LOG_STREAM(info, ai)<<"\t....."<<*ui<<"........"<<std::endl;
    //      unit_map::iterator u = unit(*ui,get_info().units);
    //      if(u!=get_info().units.end()){
    //	LOG_STREAM(info, ai)<<"\t"<<u->second.name()<<std::endl;
    //	LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_description()<<std::endl;
    //	LOG_STREAM(info, ai)<<"\t\t\t"<<u->second.get_ai_special()<<std::endl;
    //	LOG_STREAM(info, ai)<<"\t\t\t"<<u->first.x<<","<<u->first.y<<std::endl;
    //      }
    //    }
    
    LOG_STREAM(info, ai)<<"Visible Units"<<std::endl;
    for(unit_list::iterator ui = v_units.begin(); ui != v_units.end(); ++ui) {
      unit_map::iterator u = unit(*ui,get_info().units);
      if(u!=get_info().units.end()){
	//	LOG_STREAM(info, ai)<<"\t"<<u->second.name()<<std::endl;
	LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_description()<<std::endl;
	//	LOG_STREAM(info, ai)<<"\t\t\t"<<u->second.get_ai_special()<<std::endl;
	//	LOG_STREAM(info, ai)<<"\t\t\t"<<u->first.x<<","<<u->first.y<<std::endl;
	
	unit_memory_.add_unit_sighting(u->second, u->first, get_info().state.turn());
      }
    }

    for(config::child_list::const_iterator o = orders.begin(); o != orders.end(); ++o) {
      std::string order_id=(**o)["id"];
      std::string number=(**o)["number"];
      size_t num=atoi(number.c_str());
      const config::child_list& filter = (**o).get_children("filter");

      LOG_STREAM(info, ai)<<"dfool order:"<<order_id<<std::endl;

      //first find units where AI_SPECIAL matches order id
      config order_filter;
      order_filter["ai_special"]=order_id;
      unit_list order_units = filter_units(order_filter,my_units,get_info().units);
      if(num > order_units.size()){
	//need to populate orders
	//find units that match any filter. If no filters then accept all units.        
	if(filter.size()){
	  for(config::child_list::const_iterator f = filter.begin(); f != filter.end(); ++f) {
	    config ff=**f;
	    //            LOG_STREAM(info, ai)<<"dfool filter:"<<std::endl;
            unit_list filtered_units=filter_units(ff,my_units,get_info().units);

	    //FIXME: add sorting
	    
            for(unit_list::iterator i = filtered_units.begin(); i != filtered_units.end() && (num > order_units.size()); ++i) {
	      unit_map::iterator ui=unit(*i,get_info().units);
	      if(ui!=get_info().units.end()){
		std::string ais=ui->second.get_ai_special();
		
		//		LOG_STREAM(info, ai)<<"\t match: "<<ui->second.underlying_description()<<"\t"<<ais<<":::"<<std::endl;	      
		
		bool used=(ais.size() > 0);
		if(used){
		  //		  LOG_STREAM(info, ai)<<"\t\talready assigned: "<<ui->second.underlying_description()<<"\t"<<ais<<std::endl;
		}else{
		  ui->second.assign_ai_special(order_id);
		  order_units.push_back(*i);
		  
		  //		  LOG_STREAM(info, ai)<<"\t\tmatching: "<<ui->second.underlying_description()<<" to order: "<<order_id<<"\t"<<ui->second.get_ai_special()<<std::endl;
		}
	      }
            }
          }
        }else{
          order_units=my_units;
        }
      }
      
      //execute commands
      for(unit_list::iterator ou = order_units.begin(); ou != order_units.end(); ou++){
        const config::child_list& commands = (**o).get_children("command");
	bool com_break=false;

        for(config::child_list::const_iterator com = commands.begin(); com != commands.end() && !com_break; ++com) {
	  unit_map::iterator u=unit(*ou,get_info().units);

	  const config::child_list& com_filter = (**com).get_children("filter");
	  bool found=true;
	  if(u!=get_info().units.end()){
	    for(config::child_list::const_iterator sf = com_filter.begin(); sf != com_filter.end(); ++sf) {
	      config ff=**sf;
	      LOG_STREAM(info, ai)<<"ff:"<<(**com)["type"]<<" "<<ff["type"]<<" "<<ff["x"]<<","<<ff["y"]<<std::endl;
	      LOG_STREAM(info, ai)<<"ff?"<<u->second.id()<<" "<<u->first.x<<","<<u->first.y<<std::endl;
	      if(! u->second.matches_filter(ff,u->first)) {
		found=false;
		break;
	      }
	    }
	  }

	  if(found){
	    std::string type=(**com)["type"];
	    LOG_STREAM(info, ai)<<"\tcommand: "<<type<<std::endl;
	    if(type=="moveto"){
	      moveto(com,u);
	    }
	    if(type=="set_order"){
	      std::string set_id=(**com)["id"];
	      std::string a=(u->second.get_ai_special());
	      LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_description()<<"\t"<<a<<"->"<<set_id<<std::endl;
	      (u->second.assign_ai_special(set_id));
	      a=(u->second.get_ai_special());
	      LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_description()<<"\t"<<a<<" =?= "<<set_id<<std::endl;
	    }
	    if(type=="break"){
	      com_break=true;
	    }
          }
        }
      }
    }

    unit_memory_.write(ai_mem);    
    current_team().set_ai_memory(ai_mem);

    return;  
  }

  unit_list dfool_ai::filter_units(const config& filter, unit_list& ul, unit_map& um)
  {
    //    LOG_STREAM(info, ai)<<"filter1:"<<std::endl;
    unit_list filtered_units_;
    for(unit_list::const_iterator i = ul.begin(); i != ul.end(); ++i) {
      //      LOG_STREAM(info, ai)<<"filter2:"<<std::endl;
      unit_map::iterator j = unit(*i,um);
      //      LOG_STREAM(info, ai)<<"j:"<<j->second.underlying_description()<<":"<<j->first.x<<","<<j->first.y<<std::endl;
      if(j->second.underlying_description().size()>0){
	//	LOG_STREAM(info, ai)<<"filter3:"<<std::endl;
	if(j->second.matches_filter(filter,j->first)) {
	  //	  LOG_STREAM(info, ai)<<"filter4:"<<std::endl;
	  filtered_units_.push_back(*i);
	}
      }
    }
    //    LOG_STREAM(info, ai)<<"filter:"<<std::endl;
    return filtered_units_;
  }

  unit_list dfool_ai::visible_units()
  {
    unit_list visible_units;
    bool no_fog=current_team().uses_shroud() == false && current_team().uses_fog() == false;

    unit_map um=get_info().units;
    for(unit_map::iterator i = um.begin(); i != um.end(); ++i) {
      bool hidden_by_fog = current_team().fogged(i->first.x,i->first.y);
      bool hidden = i->second.invisible(i->first, um, get_info().teams);
      if((no_fog || !hidden_by_fog) && !hidden) {
	  visible_units.push_back(i->second.underlying_description());
	
      }
    }
    
    LOG_STREAM(info, ai) << "number of visible units: " << visible_units.size() << "\n";
    return visible_units;
  }

  unit_list dfool_ai::all_units(){
    unit_list all;
    unit_map um=get_info().units;
    for(unit_map::const_iterator i = um.begin(); i != um.end(); ++i) {
      //      LOG_STREAM(info, ai)<<"all:"<<i->second.underlying_description()<<std::endl;
      all.push_back(i->second.underlying_description());
    }    
    return(all); 
  }

  bool dfool_ai::moveto(config::child_list::const_iterator o, unit_map::const_iterator m){
      location target(atoi((**o)["target_x"].c_str())-1,atoi((**o)["target_y"].c_str())-1);
      LOG_STREAM(info, ai)<<"\tmoving to:("<<target.x<<","<<target.y<<")"<<std::endl;
      if(m->second.movement_left()){
	std::map<location,paths> possible_moves;
	move_map srcdst, dstsrc;
	unit_map known_units;
	
	unit_memory_.known_map(known_units,0);

	calculate_moves(known_units,possible_moves,srcdst,dstsrc,false);
	
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
	    //            LOG_STREAM(info, ai)<<"\tmoving: "<<distance<<" from ("<<i->first.x<<","<<i->first.y<<")"<<" to ("<<target.x<<","<<target.y<<")"<<std::endl;
	  }
	}
	if(closest_distance != -1) {
	  gamemap::location to = move_unit_partial(closest_move.second,closest_move.first,possible_moves);
	  if(to != closest_move.second)
	    return(false); //something unexpected happened
	}
      }
      return(true);
  }

  unit_map::iterator dfool_ai::unit(std::string unit_id, unit_map& um){
    //    LOG_STREAM(info, ai)<<"unit start:"<<unit_id<<std::endl;
    for(unit_map::iterator u = um.begin(); u != um.end(); u++){
      if(unit_id == u->second.underlying_description()){
	//	LOG_STREAM(info, ai)<<"unit:"<<unit_id<<" , "<< u->second.underlying_description()<<std::endl;
	return u;
      }
    }
    //    LOG_STREAM(info, ai)<<"nounit:"<<std::endl;
    return(um.end());
  }

  unit_memory::unit_memory(const game_data& gamedata, const config& cfg){
    const config::child_list mem_list=cfg.get_children("unit_memory");
    for(config::child_list::const_iterator mem = mem_list.begin(); mem != mem_list.end(); ++mem) {
      config unit_cfg = *((*mem)->child("unit"));

      unit u(gamedata , unit_cfg);

      int t = atoi((**mem)["turn"].c_str());

      gamemap::location l(atoi((**mem)["x"].c_str())-1,atoi((**mem)["y"].c_str())-1);
      add_unit_sighting(u,l,t);
    }
  }

  void unit_memory::add_unit_sighting(unit u, gamemap::location l, size_t t){
    std::string unit_id= u.underlying_description();
    //check if this unit has already been seen 
    size_t i;
    for(i=0; i < ids_.size();i++){
      if(unit_id == ids_[i]){break;}
    }

    if(i == ids_.size()){    
      //unit has not been seen
      units_.push_back(u);
      ids_.push_back(unit_id);
      turns_.push_back(t);
      locations_.push_back(l);
    }else{
      //update unit info
      units_[i]=u;
      turns_[i]=t;
      locations_[i]=l;
    }

  }

  void unit_memory::remove_unit_sighting(std::string id){
    size_t i;
    for(i=0;i<ids_.size();i++){
      if(id == ids_[i]){break;}
    }

    if(i == ids_.size()){    
      //unit not in memory
    }else{
      //remove unit info
      units_.erase(units_.begin()+i);
      ids_.erase(ids_.begin()+i);
      locations_.erase(locations_.begin()+i);
      turns_.erase(turns_.begin()+i);
    }
  }

  void unit_memory::write(config& temp){
    //    std::cout<<"ai_write:\n";
    for(size_t i = 0; i < units_.size(); i++){
      config element;
      write_element(i, element);
      temp.add_child("unit_memory",element);
    }
  }

  void unit_memory::write_element(int i, config &temp){
    config temp_unit;
    std::stringstream ts,xs,ys;
    ts << turns_[i];;
    temp["turn"] = ts.str();
    xs << locations_[i].x;
    temp["x"] = xs.str();
    ys << locations_[i].y;
    temp["y"] = ys.str();
    units_[i].write(temp_unit); 
    temp.add_child("unit",temp_unit);
    //    std::cout<<"ai write: "<<temp_unit["description"]<<"\n";
  }

  void unit_memory::known_map(unit_map& u, size_t turn){
    size_t i;
    std::map<gamemap::location,size_t> turn_used;
    for(i=0;i<units_.size();i++){
      gamemap::location l = locations_[i];
      size_t t = turn_used[l]; 
      //      std::cout<<"turn: "<< t <<"\n";
      if(turns_[i] >= turn && turns_[i] >= t){
	  turn_used[l] = t;
	  if(t != 0){
	    u.replace(new std::pair<gamemap::location,unit>(l,units_[i]));
	  }else{
	    u.add(new std::pair<gamemap::location,unit>(l,units_[i]));
	  }
      }
    }
  }
}//end namespace dfool
